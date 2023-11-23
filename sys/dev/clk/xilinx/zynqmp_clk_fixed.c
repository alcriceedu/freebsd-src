/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2023 Beckhoff Automation GmbH & Co. KG
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
 */

#include <sys/cdefs.h>

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/bus.h>

#include <dev/extres/clk/clk.h>

#include <dev/clk/xilinx/zynqmp_clk_fixed.h>

#include "clkdev_if.h"
#include "zynqmp_firmware_if.h"

struct zynqmp_clk_fixed_softc {
	device_t	firmware;
	uint32_t	id;
};

static int
zynqmp_clk_fixed_init(struct clknode *clk, device_t dev)
{

	clknode_init_parent_idx(clk, 0);
	return (0);
}

static int
zynqmp_clk_fixed_recalc(struct clknode *clk, uint64_t *freq)
{
	struct zynqmp_clk_fixed_softc *sc;
	uint32_t mult, div;
	int rv;

	sc = clknode_get_softc(clk);
	rv = ZYNQMP_FIRMWARE_CLOCK_GET_FIXEDFACTOR(sc->firmware, sc->id, &mult, &div);
	if (rv != 0) {
		printf("%s: Error while getting fixed factor for %s\n",
		    __func__,
		    clknode_get_name(clk));
		return (EINVAL);
	}

	*freq = (*freq * mult) / div;
	return (0);
}

static clknode_method_t zynqmp_clk_fixed_clknode_methods[] = {
	/* Device interface */
	CLKNODEMETHOD(clknode_init,		zynqmp_clk_fixed_init),
	CLKNODEMETHOD(clknode_recalc_freq,	zynqmp_clk_fixed_recalc),
	CLKNODEMETHOD_END
};

DEFINE_CLASS_1(zynqmp_clk_fixed_clknode, zynqmp_clk_fixed_clknode_class,
    zynqmp_clk_fixed_clknode_methods, sizeof(struct zynqmp_clk_fixed_softc), clknode_class);

int
zynqmp_clk_fixed_register(struct clkdom *clkdom, device_t fw, struct clknode_init_def *clkdef)
{
	struct clknode *clk;
	struct zynqmp_clk_fixed_softc *sc;
	uint32_t fw_clk_id;

	fw_clk_id = clkdef->id - 1;
	clkdef->id = 0;
	clk = clknode_create(clkdom, &zynqmp_clk_fixed_clknode_class, clkdef);
	if (clk == NULL)
		return (1);
	sc = clknode_get_softc(clk);
	sc->id = fw_clk_id;
	sc->firmware = fw;
	clknode_register(clkdom, clk);
	return (0);
}
