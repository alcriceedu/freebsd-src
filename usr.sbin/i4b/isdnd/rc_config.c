/*
 * Copyright (c) 1997, 1998 Hellmuth Michaelis. All rights reserved.
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
 *---------------------------------------------------------------------------
 *
 *	i4b daemon - config file processing
 *	-----------------------------------
 *
 * $FreeBSD: src/usr.sbin/i4b/isdnd/rc_config.c,v 1.1.2.1 1999/08/29 15:41:48 peter Exp $ 
 *
 *      last edit-date: [Mon Dec 14 13:41:41 1998]
 *
 *---------------------------------------------------------------------------*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "isdnd.h"
#include "y.tab.h"

#include "monitor.h"
#include "vararray.h"

extern int entrycount;
extern int lineno;
extern char *yytext;

extern FILE *yyin;
extern int yyparse();

static void set_config_defaults(void);
static void check_config(void);
static void print_config(void);

static int nregexpr = 0;
static int nregprog = 0;

/*---------------------------------------------------------------------------*
 *	called from main to read and process config file
 *---------------------------------------------------------------------------*/
void
configure(char *filename, int reread)
{
	extern void reset_scanner(FILE *inputfile);
	
	set_config_defaults();

	yyin = fopen(filename, "r");

	if(reread)
	{
		reset_scanner(yyin);
	}
	
	if (yyin == NULL)
	{
		log(LL_ERR, "cannot fopen file [%s]", filename);
		exit(1);
	}

	yyparse();
	
	monitor_fixup_rights();

	check_config();		/* validation and consistency check */

	fclose(yyin);

	if(do_print)
	{
		if(config_error_flag)
		{
			log(LL_ERR, "there were %d error(s) in the configuration file, terminating!", config_error_flag);
			exit(1);
		}
		print_config();
		do_exit(0);
	}
}

/*---------------------------------------------------------------------------*
 *	yacc error routine
 *---------------------------------------------------------------------------*/
void
yyerror(const char *msg)
{
	log(LL_ERR, "configuration error: %s at line %d, token \"%s\"", msg, lineno+1, yytext);
	config_error_flag++;
}

/*---------------------------------------------------------------------------*
 *	fill all config entries with default values
 *---------------------------------------------------------------------------*/
static void
set_config_defaults(void)
{
	cfg_entry_t *cep = &cfg_entry_tab[0];	/* ptr to config entry */
	int i;

	/* system section cleanup */
	
	nregprog = nregexpr = 0;

	rt_prio = RTPRIO_NOTUSED;
	
	/* clean regular expression table */
	
	for(i=0; i < MAX_RE; i++)
	{
		if(rarr[i].re_expr)
			free(rarr[i].re_expr);
		rarr[i].re_expr = NULL;
		
		if(rarr[i].re_prog)
			free(rarr[i].re_prog);
		rarr[i].re_prog = NULL;

		rarr[i].re_flg = 0;
	}

	/* entry section cleanup */
	
	for(i=0; i < CFG_ENTRY_MAX; i++, cep++)
	{
		bzero(cep, sizeof(cfg_entry_t));

		/* ====== filled in at startup configuration, then static */

		sprintf(cep->name, "ENTRY%d", i);	

		cep->isdncontroller = INVALID;
		cep->isdnchannel = CHAN_ANY;

		cep->usrdevicename = INVALID;
		cep->usrdeviceunit = INVALID;
		
		cep->remote_numbers_handling = RNH_LAST;

		cep->dialin_reaction = REACT_IGNORE;

		cep->b1protocol = BPROT_NONE;

		cep->unitlength = UNITLENGTH_DEFAULT;

		cep->earlyhangup = EARLYHANGUP_DEFAULT;
		
		cep->ratetype = INVALID_RATE;
		
	 	cep->unitlengthsrc = ULSRC_NONE;

		cep->answerprog = ANSWERPROG_DEF;	 	

		cep->callbackwait = CALLBACKWAIT_MIN;

		cep->calledbackwait = CALLEDBACKWAIT_MIN;		

		cep->dialretries = DIALRETRIES_DEF;

		cep->recoverytime = RECOVERYTIME_MIN;
	
		cep->dialouttype = DIALOUT_NORMAL;
		
		cep->inout = DIR_INOUT;
		
		/* ======== filled in after start, then dynamic */

		cep->cdid = CDID_UNUSED;

		cep->state = ST_IDLE;

		cep->aoc_valid = AOC_INVALID;
 	}
}

/*---------------------------------------------------------------------------*
 *	extract values from config and fill table
 *---------------------------------------------------------------------------*/
void
cfg_setval(int keyword)
{
	int i;
	
	switch(keyword)
	{
		case ACCTALL:
			acct_all = yylval.booln;
			log(LL_DBG, "system: acctall = %d", yylval.booln);
			break;
			
		case ACCTFILE:
			strcpy(acctfile, yylval.str);
			log(LL_DBG, "system: acctfile = %s", yylval.str);
			break;

		case ALERT:
			if(yylval.num < MINALERT)
			{
				yylval.num = MINALERT;
				log(LL_DBG, "entry %d: alert < %d, min = %d", entrycount, MINALERT, yylval.num);
			}
			else if(yylval.num > MAXALERT)
			{
				yylval.num = MAXALERT;
				log(LL_DBG, "entry %d: alert > %d, min = %d", entrycount, MAXALERT, yylval.num);
			}
				
			log(LL_DBG, "entry %d: alert = %d", entrycount, yylval.num);
			cfg_entry_tab[entrycount].alert = yylval.num;
			break;

		case ALIASING:
			log(LL_DBG, "system: aliasing = %d", yylval.booln);
			aliasing = yylval.booln;
			break;

		case ALIASFNAME:
			strcpy(aliasfile, yylval.str);
			log(LL_DBG, "system: aliasfile = %s", yylval.str);
			break;

		case ANSWERPROG:
			if((cfg_entry_tab[entrycount].answerprog = malloc(strlen(yylval.str)+1)) == NULL)
			{
				log(LL_ERR, "entry %d: answerstring, malloc failed!", entrycount);
				do_exit(1);
			}
			strcpy(cfg_entry_tab[entrycount].answerprog, yylval.str);
			log(LL_DBG, "entry %d: answerprog = %s", entrycount, yylval.str);
			break;
			
		case B1PROTOCOL:
			log(LL_DBG, "entry %d: b1protocol = %s", entrycount, yylval.str);
			if(!(strcmp(yylval.str, "raw")))
				cfg_entry_tab[entrycount].b1protocol = BPROT_NONE;
			else if(!(strcmp(yylval.str, "hdlc")))
				cfg_entry_tab[entrycount].b1protocol = BPROT_RHDLC;
			else
			{
				log(LL_ERR, "ERROR parsing config file: unknown parameter for keyword \"b1protocol\" at line %d!", lineno);
				config_error_flag++;
			}
			break;

		case CALLBACKWAIT:
			if(yylval.num < CALLBACKWAIT_MIN)
			{
				yylval.num = CALLBACKWAIT_MIN;
				log(LL_DBG, "entry %d: callbackwait < %d, min = %d", entrycount, CALLBACKWAIT_MIN, yylval.num);
			}
				
			log(LL_DBG, "entry %d: callbackwait = %d", entrycount, yylval.num);
			cfg_entry_tab[entrycount].callbackwait = yylval.num;
			break;
			
		case CALLEDBACKWAIT:
			if(yylval.num < CALLEDBACKWAIT_MIN)
			{
				yylval.num = CALLEDBACKWAIT_MIN;
				log(LL_DBG, "entry %d: calledbackwait < %d, min = %d", entrycount, CALLEDBACKWAIT_MIN, yylval.num);
			}

			log(LL_DBG, "entry %d: calledbackwait = %d", entrycount, yylval.num);
			cfg_entry_tab[entrycount].calledbackwait = yylval.num;
			break;

		case CONNECTPROG:
			if((cfg_entry_tab[entrycount].connectprog = malloc(strlen(yylval.str)+1)) == NULL)
			{
				log(LL_ERR, "entry %d: connectprog, malloc failed!", entrycount);
				do_exit(1);
			}
			strcpy(cfg_entry_tab[entrycount].connectprog, yylval.str);
			log(LL_DBG, "entry %d: connectprog = %s", entrycount, yylval.str);
			break;
			
		case DIALOUTTYPE:
			log(LL_DBG, "entry %d: dialouttype = %s", entrycount, yylval.str);
			if(!(strcmp(yylval.str, "normal")))
				cfg_entry_tab[entrycount].dialouttype = DIALOUT_NORMAL;
			else if(!(strcmp(yylval.str, "calledback")))
				cfg_entry_tab[entrycount].dialouttype = DIALOUT_CALLEDBACK;
			else
			{
				log(LL_ERR, "ERROR parsing config file: unknown parameter for keyword \"dialout-type\" at line %d!", lineno);
				config_error_flag++;
			}
			break;

		case DIALRETRIES:
			log(LL_DBG, "entry %d: dialretries = %d", entrycount, yylval.num);
			cfg_entry_tab[entrycount].dialretries = yylval.num;
			break;

		case DIALRANDINCR:
			log(LL_DBG, "entry %d: dialrandincr = %d", entrycount, yylval.booln);
			cfg_entry_tab[entrycount].dialrandincr = yylval.booln;
			break;

		case DIRECTION:
			log(LL_DBG, "entry %d: direction = %s", entrycount, yylval.str);

			if(!(strcmp(yylval.str, "inout")))
				cfg_entry_tab[entrycount].inout = DIR_INOUT;
			else if(!(strcmp(yylval.str, "in")))
				cfg_entry_tab[entrycount].inout = DIR_INONLY;
			else if(!(strcmp(yylval.str, "out")))
				cfg_entry_tab[entrycount].inout = DIR_OUTONLY;
			else
			{
				log(LL_ERR, "ERROR parsing config file: unknown parameter for keyword \"direction\" at line %d!", lineno);
				config_error_flag++;
			}
			break;

		case DISCONNECTPROG:
			if((cfg_entry_tab[entrycount].disconnectprog = malloc(strlen(yylval.str)+1)) == NULL)
			{
				log(LL_ERR, "entry %d: disconnectprog, malloc failed!", entrycount);
				do_exit(1);
			}
			strcpy(cfg_entry_tab[entrycount].disconnectprog, yylval.str);
			log(LL_DBG, "entry %d: disconnectprog = %s", entrycount, yylval.str);
			break;

		case DOWNTRIES:
			if(yylval.num > DOWN_TRIES_MAX)
				yylval.num = DOWN_TRIES_MAX;
			else if(yylval.num < DOWN_TRIES_MIN)
				yylval.num = DOWN_TRIES_MIN;
		
			log(LL_DBG, "entry %d: downtries = %d", entrycount, yylval.num);
			cfg_entry_tab[entrycount].downtries = yylval.num;
			break;

		case DOWNTIME:
			if(yylval.num > DOWN_TIME_MAX)
				yylval.num = DOWN_TIME_MAX;
			else if(yylval.num < DOWN_TIME_MIN)
				yylval.num = DOWN_TIME_MIN;
		
			log(LL_DBG, "entry %d: downtime = %d", entrycount, yylval.num);
			cfg_entry_tab[entrycount].downtime = yylval.num;
			break;

		case EARLYHANGUP:
			log(LL_DBG, "entry %d: earlyhangup = %d", entrycount, yylval.num);
			cfg_entry_tab[entrycount].earlyhangup = yylval.num;
			break;

		case IDLETIME_IN:
			log(LL_DBG, "entry %d: idle_time_in = %d", entrycount, yylval.num);
			cfg_entry_tab[entrycount].idle_time_in = yylval.num;
			break;
			
		case IDLETIME_OUT:
			log(LL_DBG, "entry %d: idle_time_out = %d", entrycount, yylval.num);
			cfg_entry_tab[entrycount].idle_time_out = yylval.num;
			break;

		case ISDNCONTROLLER:
			cfg_entry_tab[entrycount].isdncontroller = yylval.num;
			log(LL_DBG, "entry %d: isdncontroller = %d", entrycount, yylval.num);
			break;

		case ISDNCHANNEL:
			switch(yylval.num)
			{
				case 0:
				case -1:
					cfg_entry_tab[entrycount].isdnchannel = CHAN_ANY;
					log(LL_DBG, "entry %d: isdnchannel = any", entrycount);
					break;
				case 1:
					cfg_entry_tab[entrycount].isdnchannel = CHAN_B1;
					log(LL_DBG, "entry %d: isdnchannel = one", entrycount);
					break;
				case 2:
					cfg_entry_tab[entrycount].isdnchannel = CHAN_B2;
					log(LL_DBG, "entry %d: isdnchannel = two", entrycount);
					break;
				default:
					log(LL_DBG, "entry %d: isdnchannel value out of range", entrycount);
					config_error_flag++;
					break;
			}
			break;

		case ISDNTIME:
			log(LL_DBG, "system: isdntime = %d", yylval.booln);
			isdntime = yylval.booln;
			break;

		case ISDNTXDELIN:
			cfg_entry_tab[entrycount].isdntxdelin = yylval.num;
			log(LL_DBG, "entry %d: isdntxdel-incoming = %d", entrycount, yylval.num);
			break;

		case ISDNTXDELOUT:
			cfg_entry_tab[entrycount].isdntxdelout = yylval.num;
			log(LL_DBG, "entry %d: isdntxdel-outgoing = %d", entrycount, yylval.num);
			break;

		case LOCAL_PHONE_DIALOUT:
			log(LL_DBG, "entry %d: local_phone_dialout = %s", entrycount, yylval.str);
			strcpy(cfg_entry_tab[entrycount].local_phone_dialout, yylval.str);
			break;

		case LOCAL_PHONE_INCOMING:
			log(LL_DBG, "entry %d: local_phone_incoming = %s", entrycount, yylval.str);
			strcpy(cfg_entry_tab[entrycount].local_phone_incoming, yylval.str);
			break;

		case MONITORPORT:
			monitorport = yylval.num;
			log(LL_DBG, "system: monitorport = %d", yylval.num);
			break;

		case MONITORSW:
			if (yylval.booln && inhibit_monitor) {
				do_monitor = 0;
				log(LL_DBG, "system: monitor-enable overriden by command line flag");
			} else {
			do_monitor = yylval.booln;
				log(LL_DBG, "system: monitor-enable = %d", yylval.booln);
			}
			break;

		case NAME:
			log(LL_DBG, "entry %d: name = %s", entrycount, yylval.str);
			strcpy(cfg_entry_tab[entrycount].name, yylval.str);
			break;

		case REACTION:
			log(LL_DBG, "entry %d: dialin_reaction = %s", entrycount, yylval.str);
			if(!(strcmp(yylval.str, "accept")))
				cfg_entry_tab[entrycount].dialin_reaction = REACT_ACCEPT;
			else if(!(strcmp(yylval.str, "reject")))
				cfg_entry_tab[entrycount].dialin_reaction = REACT_REJECT;
			else if(!(strcmp(yylval.str, "ignore")))
				cfg_entry_tab[entrycount].dialin_reaction = REACT_IGNORE;
			else if(!(strcmp(yylval.str, "answer")))
				cfg_entry_tab[entrycount].dialin_reaction = REACT_ANSWER;
			else if(!(strcmp(yylval.str, "callback")))
				cfg_entry_tab[entrycount].dialin_reaction = REACT_CALLBACK;
			else
			{
				log(LL_ERR, "ERROR parsing config file: unknown parameter for keyword \"dialin_reaction\" at line %d!", lineno);
				config_error_flag++;
			}
			break;

		case REMOTE_PHONE_DIALOUT:
			if(cfg_entry_tab[entrycount].remote_numbers_count >= MAXRNUMBERS)
			{
				log(LL_ERR, "ERROR parsing config file: too many remote numbers at line %d!", lineno);
				config_error_flag++;
				break;
			}				
			
			log(LL_DBG, "entry %d: remote_phone_dialout #%d = %s",
				entrycount, cfg_entry_tab[entrycount].remote_numbers_count, yylval.str);

			strcpy(cfg_entry_tab[entrycount].remote_numbers[cfg_entry_tab[entrycount].remote_numbers_count].number, yylval.str);
			cfg_entry_tab[entrycount].remote_numbers[cfg_entry_tab[entrycount].remote_numbers_count].flag = 0;

			cfg_entry_tab[entrycount].remote_numbers_count++;
			
			break;

		case REMOTE_NUMBERS_HANDLING:			
			log(LL_DBG, "entry %d: remdial_handling = %s", entrycount, yylval.str);
			if(!(strcmp(yylval.str, "next")))
				cfg_entry_tab[entrycount].remote_numbers_handling = RNH_NEXT;
			else if(!(strcmp(yylval.str, "last")))
				cfg_entry_tab[entrycount].remote_numbers_handling = RNH_LAST;
			else if(!(strcmp(yylval.str, "first")))
				cfg_entry_tab[entrycount].remote_numbers_handling = RNH_FIRST;
			else
			{
				log(LL_ERR, "ERROR parsing config file: unknown parameter for keyword \"remdial_handling\" at line %d!", lineno);
				config_error_flag++;
			}
			break;

		case REMOTE_PHONE_INCOMING:
			{
				int n;
				n = cfg_entry_tab[entrycount].incoming_numbers_count;
				if (n >= MAX_INCOMING)
				{
					log(LL_ERR, "ERROR parsing config file: too many \"remote_phone_incoming\" entries at line %d!", lineno);
					config_error_flag++;
					break;
				}
				log(LL_DBG, "entry %d: remote_phone_incoming #%d = %s", entrycount, n, yylval.str);
				strcpy(cfg_entry_tab[entrycount].remote_phone_incoming[n].number, yylval.str);
				cfg_entry_tab[entrycount].incoming_numbers_count++;
			}
			break;

		case RATESFILE:
			strcpy(ratesfile, yylval.str);
			log(LL_DBG, "system: ratesfile = %s", yylval.str);
			break;

		case RATETYPE:
			log(LL_DBG, "entry %d: ratetype = %d", entrycount, yylval.num);
			cfg_entry_tab[entrycount].ratetype = yylval.num;
			break;
		
		case RECOVERYTIME:
			if(yylval.num < RECOVERYTIME_MIN)
			{
				yylval.num = RECOVERYTIME_MIN;
				log(LL_DBG, "entry %d: recoverytime < %d, min = %d", entrycount, RECOVERYTIME_MIN, yylval.num);
			}

			log(LL_DBG, "entry %d: recoverytime = %d", entrycount, yylval.num);
			cfg_entry_tab[entrycount].recoverytime = yylval.num;
			break;
		
		case REGEXPR:
			if(nregexpr >= MAX_RE)
			{
				log(LL_DBG, "system: regexpr #%d >= MAX_RE", nregexpr);
				config_error_flag++;
				break;
			}

			if((i = regcomp(&(rarr[nregexpr].re), yylval.str, REG_EXTENDED|REG_NOSUB)) != 0)
		        {
                		char buf[256];
                		regerror(i, &(rarr[nregexpr].re), buf, sizeof(buf));
				log(LL_DBG, "system: regcomp error for %s: [%s]", yylval.str, buf);
				config_error_flag++;
                		break;
			}
			else
			{
				if((rarr[nregexpr].re_expr = malloc(strlen(yylval.str)+1)) == NULL)
				{
					log(LL_DBG, "system: regexpr malloc error error for %s", yylval.str);
					config_error_flag++;
					break;
				}
				strcpy(rarr[nregexpr].re_expr, yylval.str);

				log(LL_DBG, "system: regexpr %s stored into slot %d", yylval.str, nregexpr);
				
				if(rarr[nregexpr].re_prog != NULL)
					rarr[nregexpr].re_flg = 1;
				
				nregexpr++;
				
			}
			break;

		case REGPROG:
			if(nregprog >= MAX_RE)
			{
				log(LL_DBG, "system: regprog #%d >= MAX_RE", nregprog);
				config_error_flag++;
				break;
			}
			if((rarr[nregprog].re_prog = malloc(strlen(yylval.str)+1)) == NULL)
			{
				log(LL_DBG, "system: regprog malloc error error for %s", yylval.str);
				config_error_flag++;
				break;
			}
			strcpy(rarr[nregprog].re_prog, yylval.str);

			log(LL_DBG, "system: regprog %s stored into slot %d", yylval.str, nregprog);
			
			if(rarr[nregprog].re_expr != NULL)
				rarr[nregprog].re_flg = 1;

			nregprog++;
			break;

		case RTPRIO:
#ifdef USE_RTPRIO
			rt_prio = yylval.num;
			if(rt_prio < RTP_PRIO_MIN || rt_prio > RTP_PRIO_MAX)
			{
				config_error_flag++;
				log(LL_DBG, "system: error, rtprio (%d) out of range!", yylval.num);
			}
			else
			{
				log(LL_DBG, "system: rtprio = %d", yylval.num);
			}
#else
			rt_prio = RTPRIO_NOTUSED;
#endif
			break;

		case UNITLENGTH:
			log(LL_DBG, "entry %d: unitlength = %d", entrycount, yylval.num);
			cfg_entry_tab[entrycount].unitlength = yylval.num;
			break;

		case UNITLENGTHSRC:
			log(LL_DBG, "entry %d: unitlengthsrc = %s", entrycount, yylval.str);
			if(!(strcmp(yylval.str, "none")))
				cfg_entry_tab[entrycount].unitlengthsrc = ULSRC_NONE;
			else if(!(strcmp(yylval.str, "cmdl")))
				cfg_entry_tab[entrycount].unitlengthsrc = ULSRC_CMDL;
			else if(!(strcmp(yylval.str, "conf")))
				cfg_entry_tab[entrycount].unitlengthsrc = ULSRC_CONF;
			else if(!(strcmp(yylval.str, "rate")))
				cfg_entry_tab[entrycount].unitlengthsrc = ULSRC_RATE;
			else if(!(strcmp(yylval.str, "aocd")))
				cfg_entry_tab[entrycount].unitlengthsrc = ULSRC_DYN;
			else
			{
				log(LL_ERR, "ERROR parsing config file: unknown parameter for keyword \"unitlengthsrc\" at line %d!", lineno);
				config_error_flag++;
			}
			break;

		case USRDEVICENAME:
			log(LL_DBG, "entry %d: usrdevicename = %s", entrycount, yylval.str);
			if(!strcmp(yylval.str, "rbch"))
				cfg_entry_tab[entrycount].usrdevicename = BDRV_RBCH;
			else if(!strcmp(yylval.str, "tel"))
				cfg_entry_tab[entrycount].usrdevicename = BDRV_TEL;
			else if(!strcmp(yylval.str, "ipr"))
				cfg_entry_tab[entrycount].usrdevicename = BDRV_IPR;
			else if(!strcmp(yylval.str, "isp"))
				cfg_entry_tab[entrycount].usrdevicename = BDRV_ISPPP;
			else
			{
				log(LL_ERR, "ERROR parsing config file: unknown parameter for keyword \"usrdevicename\" at line %d!", lineno);
				config_error_flag++;
			}
			break;

		case USRDEVICEUNIT:
			log(LL_DBG, "entry %d: usrdeviceunit = %d", entrycount, yylval.num);
			cfg_entry_tab[entrycount].usrdeviceunit = yylval.num;
			break;

		case USEACCTFILE:
			useacctfile = yylval.booln;
			log(LL_DBG, "system: useacctfile = %d", yylval.booln);
			break;

		case USEDOWN:
			log(LL_DBG, "entry %d: usedown = %d", entrycount, yylval.booln);
			cfg_entry_tab[entrycount].usedown = yylval.booln;
			break;

		default:
			log(LL_ERR, "ERROR parsing config file: unknown keyword at line %d!", lineno);
			config_error_flag++;
			break;			
	}
}

/*---------------------------------------------------------------------------*
 *	configuration validation and consistency check
 *---------------------------------------------------------------------------*/
static void
check_config(void)
{
	cfg_entry_t *cep = &cfg_entry_tab[0];	/* ptr to config entry */
	int i;
	int error = 0;

	/* regular expression table */
	
	for(i=0; i < MAX_RE; i++)
	{
		if((rarr[i].re_expr != NULL) && (rarr[i].re_prog == NULL))
		{
			log(LL_ERR, "check_config: regular expression %d without program!", i);
			error++;
		}
		if((rarr[i].re_prog != NULL) && (rarr[i].re_expr == NULL))
		{
			log(LL_ERR, "check_config: regular expression program %d without expression!", i);
			error++;
		}
	}

	/* entry sections */
	
	for(i=0; i <= entrycount; i++, cep++)
	{
		/* isdn controller number */

		if((cep->isdncontroller < 0) || (cep->isdncontroller > (ncontroller-1)))
		{
			log(LL_ERR, "check_config: isdncontroller out of range in entry %d!", i);
			error++;
		}

		/* numbers used for dialout */
		
		if((cep->inout != DIR_INONLY) && (cep->dialin_reaction != REACT_ANSWER))
		{
			if(cep->remote_numbers_count == 0)
			{
				log(LL_ERR, "check_config: remote-phone-dialout not set in entry %d!", i);
				error++;
			}
			if(strlen(cep->local_phone_dialout) == 0)
			{
				log(LL_ERR, "check_config: local-phone-dialout not set in entry %d!", i);
				error++;
			}
		}

		/* numbers used for incoming calls */
		
		if(cep->inout != DIR_OUTONLY)
		{
			if(strlen(cep->local_phone_incoming) == 0)
			{
				log(LL_ERR, "check_config: local-phone-incoming not set in entry %d!", i);
				error++;
			}
			if(cep->incoming_numbers_count == 0)
			{
				log(LL_ERR, "check_config: remote-phone-incoming not set in entry %d!", i);
				error++;
			}
		}

		if((cep->dialin_reaction == REACT_ANSWER) && (cep->b1protocol != BPROT_NONE))
		{
			log(LL_ERR, "check_config: b1protocol not raw for telephony in entry %d!", i);
			error++;
		}
	}
	if(error)
	{
		log(LL_ERR, "check_config: %d error(s) in configuration file, exit!", error);
		do_exit(1);
	}
}

/*---------------------------------------------------------------------------*
 *	print the configuration
 *---------------------------------------------------------------------------*/
static void
print_config(void)
{
#define PFILE stdout

	cfg_entry_t *cep = &cfg_entry_tab[0];	/* ptr to config entry */
	int i, j;
	extern VARA_DECL(struct monitor_rights) rights;
	time_t clock;
	char mytime[64];

	time(&clock);
	strcpy(mytime, ctime(&clock));
	mytime[strlen(mytime)-1] = '\0';

	fprintf(PFILE, "#---------------------------------------------------------------------------\n");
	fprintf(PFILE, "# system section (generated %s)\n", mytime);
	fprintf(PFILE, "#---------------------------------------------------------------------------\n");
	fprintf(PFILE, "system\n");
	fprintf(PFILE, "useacctfile     = %s\n", useacctfile ? "on\t\t\t\t# update accounting information file" : "off\t\t\t\t# don't update accounting information file");
	fprintf(PFILE, "acctall         = %s\n", acct_all ? "on\t\t\t\t# put all events into accounting file" : "off\t\t\t\t# put only charged events into accounting file");
	fprintf(PFILE, "acctfile        = %s\t\t# accounting information file\n", acctfile);
	fprintf(PFILE, "ratesfile       = %s\t\t# charging rates database file\n", ratesfile);

#ifdef USE_RTPRIO
	if(rt_prio == RTPRIO_NOTUSED)
		fprintf(PFILE, "# rtprio is unused\n");
	else
		fprintf(PFILE, "rtprio          = %d\t\t\t\t# isdnd runs at realtime priority\n", rt_prio);
#endif

	/* regular expression table */
	
	for(i=0; i < MAX_RE; i++)
	{
		if(rarr[i].re_expr != NULL)
		{
			fprintf(PFILE, "regexpr         = \"%s\"\t\t# scan logfile for this expression\n", rarr[i].re_expr);
		}
		if(rarr[i].re_prog != NULL)
		{
			fprintf(PFILE, "regprog         = %s\t\t# program to run when expression is matched\n", rarr[i].re_prog);
		}
	}

#ifdef I4B_EXTERNAL_MONITOR

	fprintf(PFILE, "monitor-allowed = %s\n", do_monitor ? "on\t\t\t\t# remote isdnd monitoring allowed" : "off\t\t\t\t# remote isdnd monitoring disabled");
	fprintf(PFILE, "monitor-port    = %d\t\t\t\t# TCP/IP port number used for remote monitoring\n", monitorport);

	if(VARA_NUM(rights))
	{
		char *s = "error\n";
		char b[512];
		
		VARA_FOREACH(rights, i)
		{
			if(VARA_AT(rights, i).local)
			{
				fprintf(PFILE, "monitor         = \"%s\"\t\t# local socket name for monitoring\n", VARA_AT(rights, i).name);
			}
			else
			{
				struct in_addr ia;
				ia.s_addr = ntohl(VARA_AT(rights, i).net);

				switch(VARA_AT(rights, i).mask)
				{
					case 0xffffffff:
						s = "32";
						break;
					case 0xfffffffe:
						s = "31";
						break;
					case 0xfffffffc:
						s = "30";
						break;
					case 0xfffffff8:
						s = "29";
						break;
					case 0xfffffff0:
						s = "28";
						break;
					case 0xffffffe0:
						s = "27";
						break;
					case 0xffffffc0:
						s = "26";
						break;
					case 0xffffff80:
						s = "25";
						break;
					case 0xffffff00:
						s = "24";
						break;
					case 0xfffffe00:
						s = "23";
						break;
					case 0xfffffc00:
						s = "22";
						break;
					case 0xfffff800:
						s = "21";
						break;
					case 0xfffff000:
						s = "20";
						break;
					case 0xffffe000:
						s = "19";
						break;
					case 0xffffc000:
						s = "18";
						break;
					case 0xffff8000:
						s = "17";
						break;
					case 0xffff0000:
						s = "16";
						break;
					case 0xfffe0000:
						s = "15";
						break;
					case 0xfffc0000:
						s = "14";
						break;
					case 0xfff80000:
						s = "13";
						break;
					case 0xfff00000:
						s = "12";
						break;
					case 0xffe00000:
						s = "11";
						break;
					case 0xffc00000:
						s = "10";
						break;
					case 0xff800000:
						s = "9";
						break;
					case 0xff000000:
						s = "8";
						break;
					case 0xfe000000:
						s = "7";
						break;
					case 0xfc000000:
						s = "6";
						break;
					case 0xf8000000:
						s = "5";
						break;
					case 0xf0000000:
						s = "4";
						break;
					case 0xe0000000:
						s = "3";
						break;
					case 0xc0000000:
						s = "2";
						break;
					case 0x80000000:
						s = "1";
						break;
					case 0x00000000:
						s = "0";
						break;
				}
				fprintf(PFILE, "monitor         = \"%s/%s\"\t\t# host (net/mask) allowed to connect for monitoring\n", inet_ntoa(ia), s);
			}
			b[0] = '\0';
			
			if((VARA_AT(rights, i).rights) & I4B_CA_COMMAND_FULL)
				strcat(b, "fullcmd,");
			if((VARA_AT(rights, i).rights) & I4B_CA_COMMAND_RESTRICTED)
				strcat(b, "restrictedcmd,");
			if((VARA_AT(rights, i).rights) & I4B_CA_EVNT_CHANSTATE)
				strcat(b, "channelstate,");
			if((VARA_AT(rights, i).rights) & I4B_CA_EVNT_CALLIN)
				strcat(b, "callin,");
			if((VARA_AT(rights, i).rights) & I4B_CA_EVNT_CALLOUT)
				strcat(b, "callout,");
			if((VARA_AT(rights, i).rights) & I4B_CA_EVNT_I4B)
				strcat(b, "logevents,");

			if(b[strlen(b)-1] == ',')
				b[strlen(b)-1] = '\0';
				
			fprintf(PFILE, "monitor-access  = %s\t\t# monitor access rights\n", b);
		}
	}
	
#endif
	/* entry sections */
	
	for(i=0; i <= entrycount; i++, cep++)
	{
		fprintf(PFILE, "\n");
		fprintf(PFILE, "#---------------------------------------------------------------------------\n");
		fprintf(PFILE, "# entry section %d\n", i);
		fprintf(PFILE, "#---------------------------------------------------------------------------\n");
		fprintf(PFILE, "entry\n");

		fprintf(PFILE, "name                  = %s\t\t# name for this entry section\n", cep->name);

		fprintf(PFILE, "isdncontroller        = %d\t\t# ISDN card number used for this entry\n", cep->isdncontroller);
		fprintf(PFILE, "isdnchannel           = ");
		switch(cep->isdnchannel)
		{
				case CHAN_ANY:
					fprintf(PFILE, "-1\t\t# any ISDN B-channel may be used\n");
					break;
				case CHAN_B1:
					fprintf(PFILE, "1\t\t# only ISDN B-channel 1 may be used\n");
					break;
				case CHAN_B2:
					fprintf(PFILE, "2\t\t# only ISDN B-channel 2 ay be used\n");
					break;
		}

		fprintf(PFILE, "usrdevicename         = %s\t\t# name of userland ISDN B-channel device\n", bdrivername(cep->usrdevicename));
		fprintf(PFILE, "usrdeviceunit         = %d\t\t# unit number of userland ISDN B-channel device\n", cep->usrdeviceunit);

		fprintf(PFILE, "b1protocol            = %s\n", cep->b1protocol ? "hdlc\t\t# B-channel layer 1 protocol is HDLC" : "raw\t\t# No B-channel layer 1 protocol used");

		if(!(cep->usrdevicename == BDRV_TEL))
		{
			fprintf(PFILE, "direction             = ");
			switch(cep->inout)
			{
				case DIR_INONLY:
					fprintf(PFILE, "in\t\t# only incoming connections allowed\n");
					break;
				case DIR_OUTONLY:
					fprintf(PFILE, "out\t\t# only outgoing connections allowed\n");
					break;
				case DIR_INOUT:
					fprintf(PFILE, "inout\t\t# incoming and outgoing connections allowed\n");
					break;
			}
		}
		
		if(!((cep->usrdevicename == BDRV_TEL) || (cep->inout == DIR_INONLY)))
		{
			if(cep->remote_numbers_count > 1)
			{
				for(j=0; j<cep->remote_numbers_count; j++)
					fprintf(PFILE, "remote-phone-dialout  = %s\t\t# telephone number %d for dialing out to remote\n", cep->remote_numbers[j].number, j+1);

				fprintf(PFILE, "remdial-handling      = ");
		
				switch(cep->remote_numbers_handling)
				{
					case RNH_NEXT:
						fprintf(PFILE, "next\t\t# use next number after last successfull for new dial\n");
						break;
					case RNH_LAST:
						fprintf(PFILE, "last\t\t# use last successfull number for new dial\n");
						break;
					case RNH_FIRST:
						fprintf(PFILE, "first\t\t# always start with first number for new dial\n");
						break;
				}
			}
			else
			{
				fprintf(PFILE, "remote-phone-dialout  = %s\t\t# telephone number for dialing out to remote\n", cep->remote_numbers[0].number);
			}

			fprintf(PFILE, "local-phone-dialout   = %s\t\t# show this number to remote when dialling out\n", cep->local_phone_dialout);
			fprintf(PFILE, "dialout-type          = %s\n", cep->dialouttype ? "calledback\t\t# i am called back by remote" : "normal\t\t# i am not called back by remote");
		}

		if(!(cep->inout == DIR_OUTONLY))
		{
			int n;
			
			fprintf(PFILE, "local-phone-incoming  = %s\t\t# incoming calls must match this (mine) telephone number\n", cep->local_phone_incoming);
			for (n = 0; n < cep->incoming_numbers_count; n++)
				fprintf(PFILE, "remote-phone-incoming = %s\t\t# this is a valid remote number to call me\n",
					cep->remote_phone_incoming[n].number);

			fprintf(PFILE, "dialin-reaction       = ");
			switch(cep->dialin_reaction)
			{
				case REACT_ACCEPT:
					fprintf(PFILE, "accept\t\t# i accept a call from remote and connect\n");
					break;
				case REACT_REJECT:
					fprintf(PFILE, "reject\t\t# i reject the call from remote\n");
					break;
				case REACT_IGNORE:
					fprintf(PFILE, "ignore\t\t# i ignore the call from remote\n");
					break;
				case REACT_ANSWER:
					fprintf(PFILE, "answer\t\t# i will start telephone answering when remote calls in\n");
					break;
				case REACT_CALLBACK:
					fprintf(PFILE, "callback\t\t# when remote calls in, i will hangup and call back\n");
					break;
			}
		}

		if(!((cep->inout == DIR_INONLY) || (cep->usrdevicename == BDRV_TEL)))
			fprintf(PFILE, "idletime-outgoing     = %d\t\t# outgoing call idle timeout\n", cep->idle_time_out);

		if(!(cep->inout == DIR_OUTONLY))
			fprintf(PFILE, "idletime-incoming     = %d\t\t# incoming call idle timeout\n", cep->idle_time_in);

		if(!(cep->usrdevicename == BDRV_TEL))
		{		
	 		fprintf(PFILE, "unitlengthsrc         = ");
			switch(cep->unitlengthsrc)
			{
				case ULSRC_NONE:
					fprintf(PFILE, "none\t\t# no unit length specified, using default\n");
					break;
				case ULSRC_CMDL:
					fprintf(PFILE, "cmdl\t\t# using unit length specified on commandline\n");
					break;
				case ULSRC_CONF:
					fprintf(PFILE, "conf\t\t# using unitlength specified by unitlength-keyword\n");
					fprintf(PFILE, "unitlength            = %d\t\t# fixed unitlength\n", cep->unitlength);
					break;
				case ULSRC_RATE:
					fprintf(PFILE, "rate\t\t# using unitlength specified in rate database\n");
					fprintf(PFILE, "ratetype              = %d\t\t# type of rate from rate database\n", cep->ratetype);
					break;
				case ULSRC_DYN:
					fprintf(PFILE, "aocd\t\t# using dynamically calculated unitlength based on AOCD subscription\n");
					fprintf(PFILE, "ratetype              = %d\t\t# type of rate from rate database\n", cep->ratetype);
					break;
			}

			fprintf(PFILE, "earlyhangup           = %d\t\t# early hangup safety time\n", cep->earlyhangup);

		}
		
		if(cep->usrdevicename == BDRV_TEL)
		{
			fprintf(PFILE, "answerprog            = %s\t\t# program used to answer incoming telephone calls\n", cep->answerprog);
			fprintf(PFILE, "alert                 = %d\t\t# number of seconds to wait before accepting a call\n", cep->alert);
		}

		if(!(cep->usrdevicename == BDRV_TEL))
		{		
			if(cep->dialin_reaction == REACT_CALLBACK)
				fprintf(PFILE, "callbackwait          = %d\t\t# i am waiting this time before calling back remote\n", cep->callbackwait);
	
			if(cep->dialouttype == DIALOUT_CALLEDBACK)
				fprintf(PFILE, "calledbackwait        = %d\t\t# i am waiting this time for a call back from remote\n", cep->calledbackwait);
	
			if(!(cep->inout == DIR_INONLY))
			{
				fprintf(PFILE, "dialretries           = %d\t\t# number of dialing retries\n", cep->dialretries);
				fprintf(PFILE, "recoverytime          = %d\t\t# time to wait between dialling retries\n", cep->recoverytime);
				fprintf(PFILE, "dialrandincr          = %s\t\t# use random dialing time addon\n", cep->dialrandincr ? "on" : "off");

				fprintf(PFILE, "usedown               = %s\n", cep->usedown ? "on\t\t# ISDN device switched off on excessive dial failures" : "off\t\t# no device switchoff on excessive dial failures");
				if(cep->usedown)
				{
					fprintf(PFILE, "downtries             = %d\t\t# number of dialretries failures before switching off\n", cep->downtries);
					fprintf(PFILE, "downtime              = %d\t\t# time device is switched off\n", cep->downtime);
				}
			}
		}		
	}
	fprintf(PFILE, "\n");	
}

/* EOF */
