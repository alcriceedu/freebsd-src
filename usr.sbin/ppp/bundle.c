/*-
 * Copyright (c) 1998 Brian Somers <brian@Awfulhak.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	$Id: bundle.c,v 1.1.2.6 1998/02/08 19:29:43 brian Exp $
 */

#include <sys/param.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <net/route.h>
#include <net/if_dl.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include "command.h"
#include "mbuf.h"
#include "log.h"
#include "id.h"
#include "defs.h"
#include "timer.h"
#include "fsm.h"
#include "iplist.h"
#include "hdlc.h"
#include "throughput.h"
#include "ipcp.h"
#include "link.h"
#include "bundle.h"
#include "loadalias.h"
#include "vars.h"
#include "arp.h"
#include "systems.h"
#include "route.h"
#include "lcp.h"
#include "ccp.h"
#include "async.h"
#include "descriptor.h"
#include "physical.h"
#include "modem.h"
#include "main.h"
#include "auth.h"
#include "lcpproto.h"
#include "pap.h"
#include "chap.h"
#include "tun.h"

static const char *PhaseNames[] = {
  "Dead", "Establish", "Authenticate", "Network", "Terminate"
};

const char *
bundle_PhaseName(struct bundle *bundle)
{
  return bundle->phase <= PHASE_TERMINATE ?
    PhaseNames[bundle->phase] : "unknown";
}

void
bundle_NewPhase(struct bundle *bundle, struct physical *physical, u_int new)
{
  if (new <= PHASE_NETWORK)
    LogPrintf(LogPHASE, "bundle_NewPhase: %s\n", PhaseNames[new]);

  switch (new) {
  case PHASE_DEAD:
    bundle->phase = new;
    if (CleaningUp || (mode & MODE_DIRECT) ||
        ((mode & MODE_BACKGROUND) && reconnectState != RECON_TRUE))
      Cleanup(EX_DEAD);
    break;

  case PHASE_ESTABLISH:
    bundle->phase = new;
    break;

  case PHASE_AUTHENTICATE:
    LcpInfo.auth_ineed = LcpInfo.want_auth;
    LcpInfo.auth_iwait = LcpInfo.his_auth;
    if (LcpInfo.his_auth || LcpInfo.want_auth) {
      LogPrintf(LogPHASE, " his = %s, mine = %s\n",
                Auth2Nam(LcpInfo.his_auth), Auth2Nam(LcpInfo.want_auth));
       /* XXX-ML AuthPapInfo and AuthChapInfo must be allocated! */
      if (LcpInfo.his_auth == PROTO_PAP)
	StartAuthChallenge(&AuthPapInfo, physical);
      if (LcpInfo.want_auth == PROTO_CHAP)
	StartAuthChallenge(&AuthChapInfo, physical);
      bundle->phase = new;
      Prompt(bundle);
    } else
      bundle_NewPhase(bundle, physical, PHASE_NETWORK);
    break;

  case PHASE_NETWORK:
    tun_configure(bundle, LcpInfo.his_mru, modem_Speed(physical));
    IpcpUp();
    IpcpOpen();
    CcpUp();
    CcpOpen();
    /* Fall through */

  case PHASE_TERMINATE:
    bundle->phase = new;
    Prompt(bundle);
    break;
  }
}

static int
bundle_CleanInterface(const struct bundle *bundle)
{
  int s;
  struct ifreq ifrq;
  struct ifaliasreq ifra;

  s = ID0socket(AF_INET, SOCK_DGRAM, 0);
  if (s < 0) {
    LogPrintf(LogERROR, "bundle_CleanInterface: socket(): %s\n",
              strerror(errno));
    return (-1);
  }
  strncpy(ifrq.ifr_name, bundle->ifname, sizeof ifrq.ifr_name - 1);
  ifrq.ifr_name[sizeof ifrq.ifr_name - 1] = '\0';
  while (ID0ioctl(s, SIOCGIFADDR, &ifrq) == 0) {
    memset(&ifra.ifra_mask, '\0', sizeof ifra.ifra_mask);
    strncpy(ifra.ifra_name, bundle->ifname, sizeof ifra.ifra_name - 1);
    ifra.ifra_name[sizeof ifra.ifra_name - 1] = '\0';
    ifra.ifra_addr = ifrq.ifr_addr;
    if (ID0ioctl(s, SIOCGIFDSTADDR, &ifrq) < 0) {
      if (ifra.ifra_addr.sa_family == AF_INET)
        LogPrintf(LogERROR,
                  "bundle_CleanInterface: Can't get dst for %s on %s !\n",
                  inet_ntoa(((struct sockaddr_in *)&ifra.ifra_addr)->sin_addr),
                  bundle->ifname);
      return 0;
    }
    ifra.ifra_broadaddr = ifrq.ifr_dstaddr;
    if (ID0ioctl(s, SIOCDIFADDR, &ifra) < 0) {
      if (ifra.ifra_addr.sa_family == AF_INET)
        LogPrintf(LogERROR,
                  "bundle_CleanInterface: Can't delete %s address on %s !\n",
                  inet_ntoa(((struct sockaddr_in *)&ifra.ifra_addr)->sin_addr),
                  bundle->ifname);
      return 0;
    }
  }

  return 1;
}

void
bundle_LayerStart(struct bundle *bundle, struct fsm *fp)
{
  if (fp == &LcpInfo.fsm)
    bundle_NewPhase(bundle, link2physical(fp->link), PHASE_ESTABLISH);
}

void
bundle_LayerUp(struct bundle *bundle, struct fsm *fp)
{
  /* The given fsm is now up */
  if (fp == &LcpInfo.fsm) {
    reconnectState = RECON_UNKNOWN;
    bundle_NewPhase(bundle, link2physical(fp->link), PHASE_AUTHENTICATE);
  }

  if (fp == &IpcpInfo.fsm)
    if (mode & MODE_BACKGROUND && BGFiledes[1] != -1) {
      char c = EX_NORMAL;

      if (write(BGFiledes[1], &c, 1) == 1)
	LogPrintf(LogPHASE, "Parent notified of success.\n");
      else
	LogPrintf(LogPHASE, "Failed to notify parent of success.\n");
      close(BGFiledes[1]);
      BGFiledes[1] = -1;
    }
}

int
bundle_LinkIsUp(const struct bundle *bundle)
{
  return IpcpInfo.fsm.state == ST_OPENED;
}

void
bundle_Close(struct bundle *bundle, struct fsm *fp)
{
  /*
   * Please close the given FSM.
   *
   * If fp is any CCP, just FsmClose that CCP.
   *
   * If fp == NULL or fp is the last NCP or the last LCP, enter TERMINATE phase.
   *
   * If fp == NULL, FsmClose all NCPs.
   *
   * If fp is an NCP, just FsmClose that.  When the NCPs TLF happens,
   * and if it's the last NCP, bundle_LayerFinish will enter TERMINATE
   * phase, FsmDown the top level CCP and FsmClose each of the LCPs.
   *
   * If fp is the last LCP, FsmClose all NCPs for the same
   * reasons as above.
   *
   * If fp isn't an NCP and isn't the last LCP, just FsmClose that LCP.
   */

  if (fp == &CcpInfo.fsm) {
    FsmClose(&CcpInfo.fsm);
    return;
  }

  bundle_NewPhase(bundle, NULL, PHASE_TERMINATE);

  FsmClose(&IpcpInfo.fsm);
  FsmClose(&CcpInfo.fsm);
}

/*
 *  Open tunnel device and returns its descriptor
 */

#define MAX_TUN 256
/*
 * MAX_TUN is set at 256 because that is the largest minor number
 * we can use (certainly with mknod(1) anyway.  The search for a
 * device aborts when it reaches the first `Device not configured'
 * (ENXIO) or the third `No such file or directory' (ENOENT) error.
 */
struct bundle *
bundle_Create(const char *prefix)
{
  int s, enoentcount, err;
  struct ifreq ifrq;
  static struct bundle bundle;		/* there can be only one */

  if (bundle.ifname != NULL) {	/* Already allocated ! */
    LogPrintf(LogERROR, "bundle_Create:  There's only one BUNDLE !\n");
    return NULL;
  }

  err = ENOENT;
  enoentcount = 0;
  for (bundle.unit = 0; bundle.unit <= MAX_TUN; bundle.unit++) {
    snprintf(bundle.dev, sizeof bundle.dev, "%s%d", prefix, bundle.unit);
    bundle.tun_fd = ID0open(bundle.dev, O_RDWR);
    if (bundle.tun_fd >= 0)
      break;
    if (errno == ENXIO) {
      bundle.unit = MAX_TUN;
      err = errno;
    } else if (errno == ENOENT) {
      if (++enoentcount > 2)
	bundle.unit = MAX_TUN;
    } else
      err = errno;
  }

  if (bundle.unit > MAX_TUN) {
    if (VarTerm)
      fprintf(VarTerm, "No tunnel device is available (%s).\n", strerror(err));
    return NULL;
  }

  LogSetTun(bundle.unit);

  s = socket(AF_INET, SOCK_DGRAM, 0);
  if (s < 0) {
    LogPrintf(LogERROR, "bundle_Create: socket(): %s\n", strerror(errno));
    close(bundle.tun_fd);
    return NULL;
  }

  bundle.ifname = strrchr(bundle.dev, '/');
  if (bundle.ifname == NULL)
    bundle.ifname = bundle.dev;
  else
    bundle.ifname++;

  /*
   * Now, bring up the interface.
   */
  memset(&ifrq, '\0', sizeof ifrq);
  strncpy(ifrq.ifr_name, bundle.ifname, sizeof ifrq.ifr_name - 1);
  ifrq.ifr_name[sizeof ifrq.ifr_name - 1] = '\0';
  if (ID0ioctl(s, SIOCGIFFLAGS, &ifrq) < 0) {
    LogPrintf(LogERROR, "OpenTunnel: ioctl(SIOCGIFFLAGS): %s\n",
	      strerror(errno));
    close(s);
    close(bundle.tun_fd);
    bundle.ifname = NULL;
    return NULL;
  }
  ifrq.ifr_flags |= IFF_UP;
  if (ID0ioctl(s, SIOCSIFFLAGS, &ifrq) < 0) {
    LogPrintf(LogERROR, "OpenTunnel: ioctl(SIOCSIFFLAGS): %s\n",
	      strerror(errno));
    close(s);
    close(bundle.tun_fd);
    bundle.ifname = NULL;
    return NULL;
  }

  close(s);

  if ((bundle.ifIndex = GetIfIndex(bundle.ifname)) < 0) {
    LogPrintf(LogERROR, "OpenTunnel: Can't find ifindex.\n");
    close(bundle.tun_fd);
    bundle.ifname = NULL;
    return NULL;
  }

  if (VarTerm)
    fprintf(VarTerm, "Using interface: %s\n", bundle.ifname);
  LogPrintf(LogPHASE, "Using interface: %s\n", bundle.ifname);

  bundle.routing_seq = 0;
  bundle.phase = 0;

  /* Clean out any leftover crud */
  bundle_CleanInterface(&bundle);

  bundle.physical = modem_Create("Modem");
  if (bundle.physical == NULL) {
    LogPrintf(LogERROR, "Cannot create modem device: %s\n", strerror(errno));
    return NULL;
  }

  IpcpDefAddress();
  LcpInit(&bundle, bundle.physical);
  IpcpInit(&bundle, physical2link(bundle.physical));
  CcpInit(&bundle, physical2link(bundle.physical));

  return &bundle;
}

static void
bundle_DownInterface(struct bundle *bundle)
{
  struct ifreq ifrq;
  int s;

  DeleteIfRoutes(bundle, 1);

  s = ID0socket(AF_INET, SOCK_DGRAM, 0);
  if (s < 0) {
    LogPrintf(LogERROR, "bundle_DownInterface: socket: %s\n", strerror(errno));
    return;
  }

  memset(&ifrq, '\0', sizeof ifrq);
  strncpy(ifrq.ifr_name, bundle->ifname, sizeof ifrq.ifr_name - 1);
  ifrq.ifr_name[sizeof ifrq.ifr_name - 1] = '\0';
  if (ID0ioctl(s, SIOCGIFFLAGS, &ifrq) < 0) {
    LogPrintf(LogERROR, "bundle_DownInterface: ioctl(SIOCGIFFLAGS): %s\n",
       strerror(errno));
    close(s);
    return;
  }
  ifrq.ifr_flags &= ~IFF_UP;
  if (ID0ioctl(s, SIOCSIFFLAGS, &ifrq) < 0) {
    LogPrintf(LogERROR, "bundle_DownInterface: ioctl(SIOCSIFFLAGS): %s\n",
       strerror(errno));
    close(s);
    return;
  }
  close(s);
}

void
bundle_Destroy(struct bundle *bundle)
{
  if (mode & MODE_AUTO) {
    IpcpCleanInterface(&IpcpInfo.fsm);
    bundle_DownInterface(bundle);
  }
  link_Destroy(&bundle->physical->link);
  bundle->ifname = NULL;
}

struct rtmsg {
  struct rt_msghdr m_rtm;
  char m_space[64];
};

void
bundle_SetRoute(struct bundle *bundle, int cmd, struct in_addr dst,
                struct in_addr gateway, struct in_addr mask, int bang)
{
  struct rtmsg rtmes;
  int s, nb, wb;
  char *cp;
  const char *cmdstr;
  struct sockaddr_in rtdata;

  if (bang)
    cmdstr = (cmd == RTM_ADD ? "Add!" : "Delete!");
  else
    cmdstr = (cmd == RTM_ADD ? "Add" : "Delete");
  s = ID0socket(PF_ROUTE, SOCK_RAW, 0);
  if (s < 0) {
    LogPrintf(LogERROR, "bundle_SetRoute: socket(): %s\n", strerror(errno));
    return;
  }
  memset(&rtmes, '\0', sizeof rtmes);
  rtmes.m_rtm.rtm_version = RTM_VERSION;
  rtmes.m_rtm.rtm_type = cmd;
  rtmes.m_rtm.rtm_addrs = RTA_DST;
  rtmes.m_rtm.rtm_seq = ++bundle->routing_seq;
  rtmes.m_rtm.rtm_pid = getpid();
  rtmes.m_rtm.rtm_flags = RTF_UP | RTF_GATEWAY | RTF_STATIC;

  memset(&rtdata, '\0', sizeof rtdata);
  rtdata.sin_len = 16;
  rtdata.sin_family = AF_INET;
  rtdata.sin_port = 0;
  rtdata.sin_addr = dst;

  cp = rtmes.m_space;
  memcpy(cp, &rtdata, 16);
  cp += 16;
  if (cmd == RTM_ADD)
    if (gateway.s_addr == INADDR_ANY) {
      /* Add a route through the interface */
      struct sockaddr_dl dl;
      const char *iname;
      int ilen;

      iname = Index2Nam(bundle->ifIndex);
      ilen = strlen(iname);
      dl.sdl_len = sizeof dl - sizeof dl.sdl_data + ilen;
      dl.sdl_family = AF_LINK;
      dl.sdl_index = bundle->ifIndex;
      dl.sdl_type = 0;
      dl.sdl_nlen = ilen;
      dl.sdl_alen = 0;
      dl.sdl_slen = 0;
      strncpy(dl.sdl_data, iname, sizeof dl.sdl_data);
      memcpy(cp, &dl, dl.sdl_len);
      cp += dl.sdl_len;
      rtmes.m_rtm.rtm_addrs |= RTA_GATEWAY;
    } else {
      rtdata.sin_addr = gateway;
      memcpy(cp, &rtdata, 16);
      cp += 16;
      rtmes.m_rtm.rtm_addrs |= RTA_GATEWAY;
    }

  if (dst.s_addr == INADDR_ANY)
    mask.s_addr = INADDR_ANY;

  if (cmd == RTM_ADD || dst.s_addr == INADDR_ANY) {
    rtdata.sin_addr = mask;
    memcpy(cp, &rtdata, 16);
    cp += 16;
    rtmes.m_rtm.rtm_addrs |= RTA_NETMASK;
  }

  nb = cp - (char *) &rtmes;
  rtmes.m_rtm.rtm_msglen = nb;
  wb = ID0write(s, &rtmes, nb);
  if (wb < 0) {
    LogPrintf(LogTCPIP, "bundle_SetRoute failure:\n");
    LogPrintf(LogTCPIP, "bundle_SetRoute:  Cmd = %s\n", cmd);
    LogPrintf(LogTCPIP, "bundle_SetRoute:  Dst = %s\n", inet_ntoa(dst));
    LogPrintf(LogTCPIP, "bundle_SetRoute:  Gateway = %s\n", inet_ntoa(gateway));
    LogPrintf(LogTCPIP, "bundle_SetRoute:  Mask = %s\n", inet_ntoa(mask));
failed:
    if (cmd == RTM_ADD && (rtmes.m_rtm.rtm_errno == EEXIST ||
                           (rtmes.m_rtm.rtm_errno == 0 && errno == EEXIST)))
      if (!bang)
        LogPrintf(LogWARN, "Add route failed: %s already exists\n",
                  inet_ntoa(dst));
      else {
        rtmes.m_rtm.rtm_type = cmd = RTM_CHANGE;
        if ((wb = ID0write(s, &rtmes, nb)) < 0)
          goto failed;
      }
    else if (cmd == RTM_DELETE &&
             (rtmes.m_rtm.rtm_errno == ESRCH ||
              (rtmes.m_rtm.rtm_errno == 0 && errno == ESRCH))) {
      if (!bang)
        LogPrintf(LogWARN, "Del route failed: %s: Non-existent\n",
                  inet_ntoa(dst));
    } else if (rtmes.m_rtm.rtm_errno == 0)
      LogPrintf(LogWARN, "%s route failed: %s: errno: %s\n", cmdstr,
                inet_ntoa(dst), strerror(errno));
    else
      LogPrintf(LogWARN, "%s route failed: %s: %s\n",
		cmdstr, inet_ntoa(dst), strerror(rtmes.m_rtm.rtm_errno));
  }
  LogPrintf(LogDEBUG, "wrote %d: cmd = %s, dst = %x, gateway = %x\n",
            wb, cmdstr, dst.s_addr, gateway.s_addr);
  close(s);
}

void
bundle_LinkLost(struct bundle *bundle, struct link *link)
{
  /*
   * Locate the appropriate LCP and its associated CCP, and FsmDown
   * them both.
   * The LCP TLF will notify bundle_LayerFinish() which will
   * slam the top level CCP and all NCPs down.
   */

  FsmDown(&LcpInfo.fsm);
  if (CleaningUp || reconnectState == RECON_FALSE)
    FsmClose(&LcpInfo.fsm);
}

void
bundle_LayerDown(struct bundle *bundle, struct fsm *fp)
{
  /*
   * The given FSM has been told to come down.
   * We don't do anything here, as the FSM will eventually
   * come up or down and will call LayerUp or LayerFinish.
   */
}

void
bundle_LayerFinish(struct bundle *bundle, struct fsm *fp)
{
  /* The given fsm is now down (fp cannot be NULL)
   *
   * If it's a CCP, just bring it back to STARTING in case we get more REQs
   * If it's an LCP, FsmDown the corresponding CCP and link (if open).  The
   * link_Close causes the LCP to be FsmDown()d, so make sure we only close
   * open links. XXX Not if the link is ok to come up again.
   * If it's the last LCP, FsmDown all NCPs
   * If it's the last NCP, FsmClose all LCPs and enter TERMINATE phase.
   */

  if (fp == &CcpInfo.fsm) {
    FsmDown(&CcpInfo.fsm);
    FsmOpen(&CcpInfo.fsm);
  } else if (fp == &LcpInfo.fsm) {
    FsmDown(&CcpInfo.fsm);

    FsmDown(&IpcpInfo.fsm);		/* You've lost your underlings */
    FsmClose(&IpcpInfo.fsm);		/* ST_INITIAL please */

    if (link_IsActive(fp->link)) 
      link_Close(fp->link, bundle, 0);	/* clean shutdown */

    if (!(mode & MODE_AUTO))
      bundle_DownInterface(bundle);
    bundle_NewPhase(bundle, NULL, PHASE_DEAD);
  } else if (fp == &IpcpInfo.fsm) {
    FsmClose(&LcpInfo.fsm);
    if (fp->bundle->phase != PHASE_TERMINATE)
      bundle_NewPhase(bundle, NULL, PHASE_TERMINATE);
  }
}
