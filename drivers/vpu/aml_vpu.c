
/*
 * drivers/vpu/aml_vpu.c
 *
 * Copyright (C) 2015 Amlogic, Inc. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <asm/cpu_id.h>
#include <config.h>
#include <linux/kernel.h>
#ifdef CONFIG_OF_LIBFDT
#include <libfdt.h>
#endif
#include <vpu.h>
#include "aml_vpu_reg.h"
#include "aml_vpu.h"

/* v04: add txlx support */
/* v05: add axg support */
/* v06: add g12a support */
#define VPU_VERION	"v05"

#ifdef CONFIG_OF_LIBFDT
static char *dt_addr;
#endif

static int dts_ready = 0;
enum vpu_chip_e vpu_chip_type = VPU_CHIP_MAX;

//#define VPU_DEBUG_PRINT

static struct vpu_conf_s vpu_conf = {
	.clk_level_dft = 0,
	.clk_level_max = 1,
	.clk_level = 0,
	.fclk_freq = 0,
	.fclk_div_table = NULL,
	.vpu_clk_table = NULL,
};

static void vpu_chip_detect(void)
{
#if 1
	unsigned int cpu_type;

	cpu_type = get_cpu_id().family_id;
	switch (cpu_type) {
	case MESON_CPU_MAJOR_ID_M8:
		vpu_chip_type = VPU_CHIP_M8;
#ifdef CONFIG_VPU_CLK_LEVEL_DFT
		vpu_conf.clk_level_dft = CONFIG_VPU_CLK_LEVEL_DFT;
#else
		vpu_conf.clk_level_dft = CLK_LEVEL_DFT_M8;
#endif
		vpu_conf.clk_level_max = CLK_LEVEL_MAX_M8;
		vpu_conf.fclk_freq = FCLK_TYPE_M8;
		vpu_conf.fclk_div_table = fclk_div_table_gxb;
		vpu_conf.vpu_clk_table = vpu_clk_table_m8;
		break;
	case MESON_CPU_MAJOR_ID_M8B:
		vpu_chip_type = VPU_CHIP_M8B;
#ifdef CONFIG_VPU_CLK_LEVEL_DFT
		vpu_conf.clk_level_dft = CONFIG_VPU_CLK_LEVEL_DFT;
#else
		vpu_conf.clk_level_dft = CLK_LEVEL_DFT_M8B;
#endif
		vpu_conf.clk_level_max = CLK_LEVEL_MAX_M8B;
		vpu_conf.fclk_freq = FCLK_TYPE_M8B;
		vpu_conf.fclk_div_table = fclk_div_table_gxb;
		vpu_conf.vpu_clk_table = vpu_clk_table_m8;
		break;
	case MESON_CPU_MAJOR_ID_M8M2:
		vpu_chip_type = VPU_CHIP_M8M2;
#ifdef CONFIG_VPU_CLK_LEVEL_DFT
		vpu_conf.clk_level_dft = CONFIG_VPU_CLK_LEVEL_DFT;
#else
		vpu_conf.clk_level_dft = CLK_LEVEL_DFT_M8M2;
#endif
		vpu_conf.clk_level_max = CLK_LEVEL_MAX_M8M2;
		vpu_conf.fclk_freq = FCLK_TYPE_M8M2;
		vpu_conf.fclk_div_table = fclk_div_table_gxb;
		vpu_conf.vpu_clk_table = vpu_clk_table_m8;
		break;
	case MESON_CPU_MAJOR_ID_MG9TV:
		vpu_chip_type = VPU_CHIP_G9TV;
#ifdef CONFIG_VPU_CLK_LEVEL_DFT
		vpu_conf.clk_level_dft = CONFIG_VPU_CLK_LEVEL_DFT;
#else
		vpu_conf.clk_level_dft = CLK_LEVEL_DFT_G9TV;
#endif
		vpu_conf.clk_level_max = CLK_LEVEL_MAX_G9TV;
		vpu_conf.fclk_freq = FCLK_TYPE_G9TV;
		vpu_conf.fclk_div_table = fclk_div_table_gxb;
		vpu_conf.vpu_clk_table = vpu_clk_table_m8;
		break;
	/* case MESON_CPU_MAJOR_ID_MG9BB:
		vpu_chip_type = VPU_CHIP_G9BB;
#ifdef CONFIG_VPU_CLK_LEVEL_DFT
		vpu_conf.clk_level_dft = CONFIG_VPU_CLK_LEVEL_DFT;
#else
		vpu_conf.clk_level_dft = CLK_LEVEL_DFT_G9BB;
#endif
		vpu_conf.clk_level_max = CLK_LEVEL_MAX_G9BB;
		vpu_conf.fclk_freq = FCLK_TYPE_G9BB;
		vpu_conf.fclk_div_table = fclk_div_table_gxb;
		vpu_conf.vpu_clk_table = vpu_clk_table_m8;
		break; */
	case MESON_CPU_MAJOR_ID_GXBB:
		vpu_chip_type = VPU_CHIP_GXBB;
#ifdef CONFIG_VPU_CLK_LEVEL_DFT
		vpu_conf.clk_level_dft = CONFIG_VPU_CLK_LEVEL_DFT;
#else
		vpu_conf.clk_level_dft = CLK_LEVEL_DFT_GXBB;
#endif
		vpu_conf.clk_level_max = CLK_LEVEL_MAX_GXBB;
		vpu_conf.fclk_freq = FCLK_TYPE_GXBB;
		vpu_conf.fclk_div_table = fclk_div_table_gxb;
		vpu_conf.vpu_clk_table = vpu_clk_table_gxb;
		break;
	case MESON_CPU_MAJOR_ID_GXTVBB:
		vpu_chip_type = VPU_CHIP_GXTVBB;
#ifdef CONFIG_VPU_CLK_LEVEL_DFT
		vpu_conf.clk_level_dft = CONFIG_VPU_CLK_LEVEL_DFT;
#else
		vpu_conf.clk_level_dft = CLK_LEVEL_DFT_GXTVBB;
#endif
		vpu_conf.clk_level_max = CLK_LEVEL_MAX_GXTVBB;
		vpu_conf.fclk_freq = FCLK_TYPE_GXTVBB;
		vpu_conf.fclk_div_table = fclk_div_table_gxb;
		vpu_conf.vpu_clk_table = vpu_clk_table_gxb;
		break;
	case MESON_CPU_MAJOR_ID_GXL:
		vpu_chip_type = VPU_CHIP_GXL;
#ifdef CONFIG_VPU_CLK_LEVEL_DFT
		vpu_conf.clk_level_dft = CONFIG_VPU_CLK_LEVEL_DFT;
#else
		vpu_conf.clk_level_dft = CLK_LEVEL_DFT_GXL;
#endif
		vpu_conf.clk_level_max = CLK_LEVEL_MAX_GXL;
		vpu_conf.fclk_freq = FCLK_TYPE_GXL;
		vpu_conf.fclk_div_table = fclk_div_table_gxb;
		vpu_conf.vpu_clk_table = vpu_clk_table_gxb;
		break;
	case MESON_CPU_MAJOR_ID_GXM:
		vpu_chip_type = VPU_CHIP_GXM;
#ifdef CONFIG_VPU_CLK_LEVEL_DFT
		vpu_conf.clk_level_dft = CONFIG_VPU_CLK_LEVEL_DFT;
#else
		vpu_conf.clk_level_dft = CLK_LEVEL_DFT_GXM;
#endif
		vpu_conf.clk_level_max = CLK_LEVEL_MAX_GXM;
		vpu_conf.fclk_freq = FCLK_TYPE_GXM;
		vpu_conf.fclk_div_table = fclk_div_table_gxb;
		vpu_conf.vpu_clk_table = vpu_clk_table_gxb;
		break;
	case MESON_CPU_MAJOR_ID_TXL:
		vpu_chip_type = VPU_CHIP_TXL;
#ifdef CONFIG_VPU_CLK_LEVEL_DFT
		vpu_conf.clk_level_dft = CONFIG_VPU_CLK_LEVEL_DFT;
#else
		vpu_conf.clk_level_dft = CLK_LEVEL_DFT_TXL;
#endif
		vpu_conf.clk_level_max = CLK_LEVEL_MAX_TXL;
		vpu_conf.fclk_freq = FCLK_TYPE_TXL;
		vpu_conf.fclk_div_table = fclk_div_table_gxb;
		vpu_conf.vpu_clk_table = vpu_clk_table_gxb;
		break;
	case MESON_CPU_MAJOR_ID_TXLX:
		vpu_chip_type = VPU_CHIP_TXLX;
#ifdef CONFIG_VPU_CLK_LEVEL_DFT
		vpu_conf.clk_level_dft = CONFIG_VPU_CLK_LEVEL_DFT;
#else
		vpu_conf.clk_level_dft = CLK_LEVEL_DFT_TXLX;
#endif
		vpu_conf.clk_level_max = CLK_LEVEL_MAX_TXLX;
		vpu_conf.fclk_freq = FCLK_TYPE_TXLX;
		vpu_conf.fclk_div_table = fclk_div_table_gxb;
		vpu_conf.vpu_clk_table = vpu_clk_table_gxb;
		break;
	case MESON_CPU_MAJOR_ID_AXG:
		vpu_chip_type = VPU_CHIP_AXG;
#ifdef CONFIG_VPU_CLK_LEVEL_DFT
		vpu_conf.clk_level_dft = CONFIG_VPU_CLK_LEVEL_DFT;
#else
		vpu_conf.clk_level_dft = CLK_LEVEL_DFT_AXG;
#endif
		vpu_conf.clk_level_max = CLK_LEVEL_MAX_AXG;
		vpu_conf.fclk_freq = FCLK_TYPE_AXG;
		vpu_conf.fclk_div_table = fclk_div_table_gxb;
		vpu_conf.vpu_clk_table = vpu_clk_table_gxb;
		break;
	case MESON_CPU_MAJOR_ID_TXHD:
		vpu_chip_type = VPU_CHIP_TXHD;
#ifdef CONFIG_VPU_CLK_LEVEL_DFT
		vpu_conf.clk_level_dft = CONFIG_VPU_CLK_LEVEL_DFT;
#else
		vpu_conf.clk_level_dft = CLK_LEVEL_DFT_TXHD;
#endif
		vpu_conf.clk_level_max = CLK_LEVEL_MAX_TXHD;
		vpu_conf.fclk_freq = FCLK_TYPE_TXHD;
		vpu_conf.fclk_div_table = fclk_div_table_gxb;
		vpu_conf.vpu_clk_table = vpu_clk_table_gxb;
		break;
	case MESON_CPU_MAJOR_ID_G12A:
		vpu_chip_type = VPU_CHIP_G12A;
#ifdef CONFIG_VPU_CLK_LEVEL_DFT
		vpu_conf.clk_level_dft = CONFIG_VPU_CLK_LEVEL_DFT;
#else
		vpu_conf.clk_level_dft = CLK_LEVEL_DFT_G12A;
#endif
		vpu_conf.clk_level_max = CLK_LEVEL_MAX_G12A;
		vpu_conf.fclk_freq = FCLK_TYPE_G12A;
		vpu_conf.fclk_div_table = fclk_div_table_g12a;
		vpu_conf.vpu_clk_table = vpu_clk_table_gxb;
		break;
	default:
		vpu_chip_type = VPU_CHIP_G12A;
#ifdef CONFIG_VPU_CLK_LEVEL_DFT
		vpu_conf.clk_level_dft = CONFIG_VPU_CLK_LEVEL_DFT;
#else
		vpu_conf.clk_level_dft = CLK_LEVEL_DFT_G12A;
#endif
		vpu_conf.clk_level_max = CLK_LEVEL_MAX_G12A;
		vpu_conf.fclk_freq = FCLK_TYPE_G12A;
		vpu_conf.fclk_div_table = fclk_div_table_g12a;
		vpu_conf.vpu_clk_table = vpu_clk_table_gxb;
	}
#else
	vpu_chip_type = VPU_CHIP_G12A;
#ifdef CONFIG_VPU_CLK_LEVEL_DFT
	vpu_conf.clk_level_dft = CONFIG_VPU_CLK_LEVEL_DFT;
#else
	vpu_conf.clk_level_dft = CLK_LEVEL_DFT_G12A;
#endif
	vpu_conf.clk_level_max = CLK_LEVEL_MAX_G12A;
	vpu_conf.fclk_freq = FCLK_TYPE_G12A;
	vpu_conf.fclk_div_table = fclk_div_table_g12a;
	vpu_conf.vpu_clk_table = vpu_clk_table_gxb;
#endif

#ifdef VPU_DEBUG_PRINT
	VPUPR("detect chip type: %d\n", vpu_chip_type);
	VPUPR("clk_level default: %d(%dHz), max: %d(%dHz)\n",
		vpu_conf.clk_level_dft,
		(vpu_conf.vpu_clk_table + vpu_conf.clk_level_dft)->freq,
		(vpu_conf.clk_level_max - 1),
		(vpu_conf.vpu_clk_table + (vpu_conf.clk_level_max - 1))->freq);
#endif
}

static int vpu_check(void)
{
	int ret = -1;

	switch (vpu_chip_type) {
	case VPU_CHIP_M8:
	case VPU_CHIP_M8B:
	case VPU_CHIP_M8M2:
	case VPU_CHIP_G9TV:
	case VPU_CHIP_G9BB:
	case VPU_CHIP_GXBB:
	case VPU_CHIP_GXTVBB:
	case VPU_CHIP_GXL:
	case VPU_CHIP_GXM:
	case VPU_CHIP_TXL:
	case VPU_CHIP_TXLX:
	case VPU_CHIP_AXG:
	case VPU_CHIP_TXHD:
	case VPU_CHIP_G12A:
		if ((vpu_conf.fclk_div_table == NULL) ||
			(vpu_conf.vpu_clk_table == NULL)) {
			VPUERR("invalid vpu table\n");
		} else {
			ret = 0;
		}
		break;
	default:
		VPUERR("invalid vpu for current chip\n");
		break;
	}

	return ret;
}

static unsigned int get_vpu_clk_level(unsigned int video_clk)
{
	unsigned clk_level;
	int i;

	for (i = 0; i < vpu_conf.clk_level_max; i++) {
		if (video_clk <= (vpu_conf.vpu_clk_table + i)->freq)
			break;
	}
	clk_level = i;

	return clk_level;
}

static unsigned long get_fclk_div_freq(unsigned int mux_id)
{
	struct fclk_div_s *fclk_div;
	unsigned long fclk, div, clk_source = 0;
	unsigned int i;

	fclk = vpu_conf.fclk_freq * 1000000;

	for (i = 0; i < FCLK_DIV_MAX; i++) {
		fclk_div = vpu_conf.fclk_div_table + i;
		if (fclk_div->fclk_id == mux_id) {
			div = fclk_div->fclk_div;
			clk_source = ((fclk * 100 / div) + 99) / 100;
			break;
		}
		if (fclk_div->fclk_id == FCLK_DIV_MAX)
			break;
	}

	return clk_source;
}

static unsigned int get_vpu_clk_mux_id(void)
{
	struct fclk_div_s *fclk_div;
	unsigned int i, mux, mux_id;

	mux = vpu_hiu_getb(HHI_VPU_CLK_CNTL, 9, 3);
	mux_id = mux;
	for (i = 0; i < FCLK_DIV_MAX; i++) {
		fclk_div = vpu_conf.fclk_div_table + i;
		if (fclk_div->fclk_mux == mux) {
			mux_id = fclk_div->fclk_id;
			break;
		}
		if (fclk_div->fclk_id == FCLK_DIV_MAX)
			break;
	}

	return mux_id;
}

static unsigned int get_vpu_clk_mux(unsigned int mux_id)
{
	struct fclk_div_s *fclk_div;
	unsigned int i, mux;

	mux = mux_id;
	for (i = 0; i < FCLK_DIV_MAX; i++) {
		fclk_div = vpu_conf.fclk_div_table + i;
		if (fclk_div->fclk_id == mux_id) {
			mux = fclk_div->fclk_mux;
			break;
		}
		if (fclk_div->fclk_id == FCLK_DIV_MAX)
			break;
	}

	return mux;
}

static unsigned int get_vpu_clk(void)
{
	unsigned long clk_freq;
	unsigned long clk_source, div;
	unsigned int mux_id;

	mux_id = get_vpu_clk_mux_id();
	switch (mux_id) {
	case FCLK_DIV4:
	case FCLK_DIV3:
	case FCLK_DIV5:
	case FCLK_DIV7:
		clk_source = get_fclk_div_freq(mux_id);
		break;
	case GPLL_CLK:
		clk_source = (vpu_conf.vpu_clk_table + 8)->freq;
		break;
	default:
		clk_source = 0;
		break;
	}

	div = vpu_hiu_getb(HHI_VPU_CLK_CNTL, 0, 7) + 1;
	clk_freq = ((clk_source * 100 / div) + 99) / 100;

	return (unsigned int)clk_freq;
}

static int switch_gp_pll_m8m2(int flag)
{
	int cnt = 100;
	int ret = 0;

	if (flag) { /* enable gp_pll */
		/* M=182, N=3, OD=2. gp_pll=24*M/N/2^OD=364M */
		/* set gp_pll frequency fixed to 364M */
		vpu_hiu_write(HHI_GP_PLL_CNTL, 0x206b6);
		vpu_hiu_setb(HHI_GP_PLL_CNTL, 1, 30, 1); /* enable */
		do {
			udelay(10);
			vpu_hiu_setb(HHI_GP_PLL_CNTL, 1, 29, 1); /* reset */
			udelay(50);
			/* release reset */
			vpu_hiu_setb(HHI_GP_PLL_CNTL, 0, 29, 1);
			udelay(50);
			cnt--;
			if (cnt == 0)
				break;
		} while (vpu_hiu_getb(HHI_GP_PLL_CNTL, 31, 1) == 0);
		if (cnt == 0) {
			ret = -1;
			vpu_hiu_setb(HHI_GP_PLL_CNTL, 0, 30, 1);
			VPUERR("GP_PLL lock failed\n");
		}
	} else { /* disable gp_pll */
		vpu_hiu_setb(HHI_GP_PLL_CNTL, 0, 30, 1);
	}

	return ret;
}

static int switch_gp1_pll_g9tv(int flag)
{
	int cnt = 100;
	int ret = 0;

	if (flag) { /* enable gp1_pll */
		/* GP1 DPLL 696MHz output*/
		vpu_hiu_write(HHI_GP1_PLL_CNTL, 0x6a01023a);
		vpu_hiu_write(HHI_GP1_PLL_CNTL2, 0x69c80000);
		vpu_hiu_write(HHI_GP1_PLL_CNTL3, 0x0a5590c4);
		vpu_hiu_write(HHI_GP1_PLL_CNTL4, 0x0000500d);
		vpu_hiu_write(HHI_GP1_PLL_CNTL, 0x4a01023a);
		do {
			udelay(10);
			vpu_hiu_setb(HHI_GP1_PLL_CNTL, 1, 29, 1); /* reset */
			udelay(50);
			/* release reset */
			vpu_hiu_setb(HHI_GP1_PLL_CNTL, 0, 29, 1);
			udelay(50);
			cnt--;
			if (cnt == 0)
				break;
		} while (vpu_hiu_getb(HHI_GP1_PLL_CNTL, 31, 1) == 0);
		if (cnt == 0) {
			ret = -1;
			vpu_hiu_setb(HHI_GP1_PLL_CNTL, 0, 30, 1);
			VPUERR("GP_PLL lock failed\n");
		}
	} else { /* disable gp1_pll */
		vpu_hiu_setb(HHI_GP1_PLL_CNTL, 0, 30, 1);
	}

	return ret;
}

static int switch_gp1_pll_gxtvbb(int flag)
{
	int cnt = 100;
	int ret = 0;

	if (flag) { /* enable gp1_pll */
		/* GP1 DPLL 696MHz output*/
		vpu_hiu_write(HHI_GP1_PLL_CNTL2, 0x69c80000);
		vpu_hiu_write(HHI_GP1_PLL_CNTL3, 0x0a5590c4);
		vpu_hiu_write(HHI_GP1_PLL_CNTL4, 0x0000500d);
		vpu_hiu_write(HHI_GP1_PLL_CNTL, 0x6a01023a);
		vpu_hiu_write(HHI_GP1_PLL_CNTL, 0x4a01023a);
		do {
			udelay(10);
			vpu_hiu_setb(HHI_GP1_PLL_CNTL, 1, 29, 1); /* reset */
			udelay(50);
			/* release reset */
			vpu_hiu_setb(HHI_GP1_PLL_CNTL, 0, 29, 1);
			udelay(50);
			cnt--;
			if (cnt == 0)
				break;
		} while (vpu_hiu_getb(HHI_GP1_PLL_CNTL, 31, 1) == 0);
		if (vpu_hiu_getb(HHI_GP1_PLL_CNTL, 31, 1) == 0) {
			ret = -1;
			vpu_hiu_setb(HHI_GP1_PLL_CNTL, 0, 30, 1);
			VPUERR("GP1_PLL lock failed\n");
		}
	} else { /* disable gp1_pll */
		vpu_hiu_setb(HHI_GP1_PLL_CNTL, 0, 30, 1);
	}

	return ret;
}

static int switch_gp_pll(int flag)
{
	int ret = 0;

	switch (vpu_chip_type) {
	case VPU_CHIP_M8M2:
		ret = switch_gp_pll_m8m2(flag);
		break;
	case VPU_CHIP_G9TV:
		ret = switch_gp1_pll_g9tv(flag);
		break;
	case VPU_CHIP_GXTVBB:
		ret = switch_gp1_pll_gxtvbb(flag);
		break;
	default:
		ret = -1;
		break;
	}

	return ret;
}

static int adjust_vpu_clk_m8_g9(unsigned int clk_level)
{
	struct vpu_clk_s *clk_table;
	unsigned int mux, div;
	int ret = 0;

	clk_table = vpu_conf.vpu_clk_table + clk_level;
	mux = clk_table->mux;
	if (mux == GPLL_CLK) {
		ret = switch_gp_pll(1);
		if (ret) {
			clk_level = vpu_conf.clk_level_dft;
			VPUERR("adjust_vpu_clk: gp pll error, use default clk\n");
		}
	}
	vpu_conf.clk_level = clk_level;
	clk_table = vpu_conf.vpu_clk_table + clk_level;

	mux = clk_table->mux;
	div = clk_table->div;
	vpu_hiu_write(HHI_VPU_CLK_CNTL, ((mux << 9) | (div << 0) | (1<<8)));

	VPUPR("set clk: %uHz, readback: %uHz(0x%x)\n",
		clk_table->freq, get_vpu_clk(),
		(vpu_hiu_read(HHI_VPU_CLK_CNTL)));
	return ret;
}

/* unit: MHz */
#define VPU_CLKB_MAX    350
static int adjust_vpu_clk_gx(unsigned int clk_level)
{
	struct vpu_clk_s *clk_table;
	unsigned int vpu_clk;
	unsigned int mux, div;
	int ret = 0;

	/* vpu clk */
	clk_table = vpu_conf.vpu_clk_table + clk_level;
	mux = get_vpu_clk_mux(clk_table->mux);
	if (mux == GPLL_CLK) {
		ret = switch_gp_pll(1);
		if (ret) {
			clk_level = vpu_conf.clk_level_dft;
			VPUERR("adjust_vpu_clk: gp pll error, use default clk\n");
		}
	}
	vpu_conf.clk_level = clk_level;
	clk_table = vpu_conf.vpu_clk_table + clk_level;

	vpu_clk = clk_table->freq;
	mux = get_vpu_clk_mux(clk_table->mux);
	div = clk_table->div;

	vpu_hiu_write(HHI_VPU_CLK_CNTL, ((mux << 9) | (div << 0)));
	vpu_hiu_setb(HHI_VPU_CLK_CNTL, 1, 8, 1);

	/* vpu clkb if needed */
	switch (vpu_chip_type) {
	case VPU_CHIP_GXBB:
	case VPU_CHIP_GXTVBB:
	case VPU_CHIP_GXL:
	case VPU_CHIP_GXM:
	case VPU_CHIP_TXL:
		if (vpu_clk >= (VPU_CLKB_MAX * 1000000))
			vpu_hiu_write(HHI_VPU_CLKB_CNTL, ((1 << 8) | (1 << 0)));
		else
			vpu_hiu_write(HHI_VPU_CLKB_CNTL, ((1 << 8) | (0 << 0)));
		break;
	default:
		break;
	}

	/* vapb clk */
	if (vpu_clk >= 250000000) {
		vpu_hiu_write(HHI_VAPBCLK_CNTL, (1 << 30) | /* turn on ge2d clock */
						(0 << 9)  | /* clk_sel    //250Mhz */
						(1 << 0));  /* clk_div */
	} else {
		vpu_hiu_write(HHI_VAPBCLK_CNTL, (1 << 30) | /* turn on ge2d clock */
						(clk_table->mux << 9)  | /* clk_sel */
						(div << 0));  /* clk_div */
	}
	vpu_hiu_setb(HHI_VAPBCLK_CNTL, 1, 8, 1);

	VPUPR("set clk: %uHz, readback: %uHz(0x%x)\n",
		vpu_clk, get_vpu_clk(), (vpu_hiu_read(HHI_VPU_CLK_CNTL)));
	return ret;
}

static int set_vpu_clk(unsigned int vclk)
{
	int ret = 0;
	unsigned int clk_level;

#ifdef VPU_DEBUG_PRINT
	VPUPR("%s\n", __func__);
#endif

	if (vclk >= 100) /* regard as vpu_clk */
		clk_level = get_vpu_clk_level(vclk);
	else /* regard as clk_level */
		clk_level = vclk;

	if (clk_level >= vpu_conf.clk_level_max) {
		ret = 1;
		clk_level = vpu_conf.clk_level_dft;
		VPUPR("clk out of supported range, set to default\n");
	}

	switch (vpu_chip_type) {
	case VPU_CHIP_M8:
	case VPU_CHIP_M8B:
	case VPU_CHIP_M8M2:
	case VPU_CHIP_G9TV:
	case VPU_CHIP_G9BB:
		ret = adjust_vpu_clk_m8_g9(clk_level);
		break;
	default:
		ret = adjust_vpu_clk_gx(clk_level);
		break;
	}

#ifdef VPU_DEBUG_PRINT
	VPUPR("%s finish\n", __func__);
#endif

	return ret;
}

static void vpu_mem_pd_init_off(void)
{
	return;
#ifdef VPU_DEBUG_PRINT
	VPUPR("%s\n", __func__);
#endif
}

static void vpu_clk_gate_init_off(void)
{
#ifdef VPU_DEBUG_PRINT
	VPUPR("%s\n", __func__);
#endif

	switch (vpu_chip_type) {
	case VPU_CHIP_TXLX:
		/* dolby core1 */
		vpu_vcbus_setb(DOLBY_TV_CLKGATE_CTRL, 1, 10, 2);
		vpu_vcbus_setb(DOLBY_TV_CLKGATE_CTRL, 1, 2, 2);
		vpu_vcbus_setb(DOLBY_TV_CLKGATE_CTRL, 1, 4, 2);
		/* dolby core2 */
		vpu_vcbus_setb(DOLBY_CORE2A_CLKGATE_CTRL, 1, 10, 2);
		vpu_vcbus_setb(DOLBY_CORE2A_CLKGATE_CTRL, 1, 2, 2);
		vpu_vcbus_setb(DOLBY_CORE2A_CLKGATE_CTRL, 1, 4, 2);
		/* dolby core3 */
		vpu_vcbus_setb(DOLBY_CORE3_CLKGATE_CTRL, 0, 1, 1);
		vpu_vcbus_setb(DOLBY_CORE3_CLKGATE_CTRL, 1, 2, 2);
		break;
	case VPU_CHIP_GXM:
		vpu_vcbus_write(DOLBY_CORE1_CLKGATE_CTRL, 0x55555555);
		vpu_vcbus_write(DOLBY_CORE2A_CLKGATE_CTRL, 0x55555555);
		vpu_vcbus_write(DOLBY_CORE3_CLKGATE_CTRL, 0x55555555);
		break;
	default:
		break;
	}

	vpu_hiu_setb(HHI_VID_CLK_CNTL2, 0, 0, 8);

	VPUPR("%s finish\n", __func__);
}

static void vpu_module_init_config(void)
{
#ifdef VPU_DEBUG_PRINT
	VPUPR("%s\n", __func__);
#endif

	/* dmc_arb_config */
	vpu_vcbus_write(VPU_RDARB_MODE_L1C1, 0x0); //0x210000
	vpu_vcbus_write(VPU_RDARB_MODE_L1C2, 0x10000);
	vpu_vcbus_write(VPU_RDARB_MODE_L2C1, 0x900000);
	vpu_vcbus_write(VPU_WRARB_MODE_L2C1, 0x20000);

#ifdef VPU_DEBUG_PRINT
	VPUPR("%s finish\n", __func__);
#endif
}

static void vpu_power_on_m8_g9(void)
{
	unsigned int i;

	vpu_ao_setb(AO_RTI_GEN_PWR_SLEEP0, 0, 8, 1); /* [8] power on */
	udelay(20);

	/* power up memories */
	for (i = 0; i < 32; i+=2) {
		vpu_hiu_setb(HHI_VPU_MEM_PD_REG0, 0, i, 2);
		udelay(5);
	}
	for (i = 0; i < 32; i+=2) {
		vpu_hiu_setb(HHI_VPU_MEM_PD_REG1, 0, i, 2);
		udelay(5);
	}
	for (i = 8; i < 16; i++) {
		vpu_hiu_setb(HHI_MEM_PD_REG0, 0, i, 1);
		udelay(5);
	}
	udelay(20);

	/* Reset VIU + VENC */
	/* Reset VENCI + VENCP + VADC + VENCL */
	/* Reset HDMI-APB + HDMI-SYS + HDMI-TX + HDMI-CEC */
	vpu_cbus_clr_mask(RESET0_MASK, ((1 << 5) | (1<<10)));
	vpu_cbus_clr_mask(RESET4_MASK, ((1 << 6) | (1<<7) | (1<<9) | (1<<13)));
	vpu_cbus_clr_mask(RESET2_MASK, ((1 << 2) | (1<<3) | (1<<11) | (1<<15)));
	vpu_cbus_write(RESET2_REGISTER, ((1 << 2) | (1<<3) | (1<<11) | (1<<15)));
	/* reset this will cause VBUS reg to 0 */
	vpu_cbus_write(RESET4_REGISTER, ((1 << 6) | (1<<7) | (1<<9) | (1<<13)));
	vpu_cbus_write(RESET0_REGISTER, ((1 << 5) | (1<<10)));
	vpu_cbus_write(RESET4_REGISTER, ((1 << 6) | (1<<7) | (1<<9) | (1<<13)));
	vpu_cbus_write(RESET2_REGISTER, ((1 << 2) | (1<<3) | (1<<11) | (1<<15)));
	vpu_cbus_set_mask(RESET0_MASK, ((1 << 5) | (1<<10)));
	vpu_cbus_set_mask(RESET4_MASK, ((1 << 6) | (1<<7) | (1<<9) | (1<<13)));
	vpu_cbus_set_mask(RESET2_MASK, ((1 << 2) | (1<<3) | (1<<11) | (1<<15)));

	/* Remove VPU_HDMI ISO */
	vpu_ao_setb(AO_RTI_GEN_PWR_SLEEP0, 0, 9, 1); /* [9] VPU_HDMI */
}

static void vpu_power_off_m8_g9(void)
{
	unsigned int i;

	/* Power down VPU_HDMI */
	/* Enable Isolation */
	vpu_ao_setb(AO_RTI_GEN_PWR_SLEEP0, 1, 9, 1); /* ISO */
	udelay(20);

	/* power down memories */
	for (i = 0; i < 32; i+=2) {
		vpu_hiu_setb(HHI_VPU_MEM_PD_REG0, 0x3, i, 2);
		udelay(5);
	}
	for (i = 0; i < 32; i+=2) {
		vpu_hiu_setb(HHI_VPU_MEM_PD_REG1, 0x3, i, 2);
		udelay(5);
	}
	for (i = 8; i < 16; i++) {
		vpu_hiu_setb(HHI_MEM_PD_REG0, 0x1, i, 1);
		udelay(5);
	}
	udelay(20);

	/* Power down VPU domain */
	vpu_ao_setb(AO_RTI_GEN_PWR_SLEEP0, 1, 8, 1); /* PDN */

	vpu_hiu_setb(HHI_VPU_CLK_CNTL, 0, 8, 1);
}

static void vpu_power_on_gx(void)
{
	unsigned int i;

#ifdef VPU_DEBUG_PRINT
	VPUPR("%s\n", __func__);
#endif

	vpu_ao_setb(AO_RTI_GEN_PWR_SLEEP0, 0, 8, 1); /* [8] power on */
	udelay(20);

	/* power up memories */
	for (i = 0; i < 32; i+=2) {
		vpu_hiu_setb(HHI_VPU_MEM_PD_REG0, 0, i, 2);
		udelay(5);
	}
	for (i = 0; i < 32; i+=2) {
		vpu_hiu_setb(HHI_VPU_MEM_PD_REG1, 0, i, 2);
		udelay(5);
	}
	switch (vpu_chip_type) {
	case VPU_CHIP_TXHD:
		vpu_hiu_setb(HHI_VPU_MEM_PD_REG2, 0, 2, 2);
		udelay(5);
		break;
	case VPU_CHIP_G12A:
		vpu_hiu_setb(HHI_VPU_MEM_PD_REG2, 0, 0, 2);
		udelay(5);
		for (i = 4; i < 18; i+=2) {
			vpu_hiu_setb(HHI_VPU_MEM_PD_REG2, 0, i, 2);
			udelay(5);
		}
		vpu_hiu_setb(HHI_VPU_MEM_PD_REG2, 0, 30, 2);
		udelay(5);
		break;
	default:
		break;
	}
	for (i = 8; i < 16; i++) {
		vpu_hiu_setb(HHI_MEM_PD_REG0, 0, i, 1);
		udelay(5);
	}
	udelay(20);

	/* Reset VIU + VENC */
	/* Reset VENCI + VENCP + VADC + VENCL */
	/* Reset HDMI-APB + HDMI-SYS + HDMI-TX + HDMI-CEC */
	vpu_cbus_clr_mask(RESET0_LEVEL, ((1<<5) | (1<<10) | (1<<19) | (1<<13)));
	vpu_cbus_clr_mask(RESET1_LEVEL, (1<<5));
	vpu_cbus_clr_mask(RESET2_LEVEL, (1<<15));
	vpu_cbus_clr_mask(RESET4_LEVEL, ((1<<6) | (1<<7) | (1<<13) | (1<<5) | (1<<9) | (1<<4) | (1<<12)));
	switch (vpu_chip_type) {
	case VPU_CHIP_TXHD:
		vpu_cbus_clr_mask(RESET7_LEVEL, ((1<<7) | (1<<12)));
		break;
	default:
		vpu_cbus_clr_mask(RESET7_LEVEL, (1<<7));
		break;
	}

	/* Remove VPU_HDMI ISO */
	vpu_ao_setb(AO_RTI_GEN_PWR_SLEEP0, 0, 9, 1); /* [9] VPU_HDMI */

	/* release Reset */
	vpu_cbus_set_mask(RESET0_LEVEL, ((1 << 5) | (1<<10) | (1<<19) | (1<<13)));
	vpu_cbus_set_mask(RESET1_LEVEL, (1<<5));
	vpu_cbus_set_mask(RESET2_LEVEL, (1<<15));
	vpu_cbus_set_mask(RESET4_LEVEL, ((1<<6) | (1<<7) | (1<<13) | (1<<5) | (1<<9) | (1<<4) | (1<<12)));
	switch (vpu_chip_type) {
	case VPU_CHIP_TXHD:
		vpu_cbus_set_mask(RESET7_LEVEL, ((1<<7) | (1<<12)));
		break;
	default:
		vpu_cbus_set_mask(RESET7_LEVEL, (1<<7));
		break;
	}

#ifdef VPU_DEBUG_PRINT
	VPUPR("%s finish\n", __func__);
#endif
}

static void vpu_power_off_gx(void)
{
	unsigned int i;

	/* Power down VPU_HDMI */
	/* Enable Isolation */
	vpu_ao_setb(AO_RTI_GEN_PWR_SLEEP0, 1, 9, 1); /* ISO */
	udelay(20);

	/* power down memories */
	for (i = 0; i < 32; i+=2) {
		vpu_hiu_setb(HHI_VPU_MEM_PD_REG0, 0x3, i, 2);
		udelay(5);
	}
	for (i = 0; i < 32; i+=2) {
		vpu_hiu_setb(HHI_VPU_MEM_PD_REG1, 0x3, i, 2);
		udelay(5);
	}
	switch (vpu_chip_type) {
	case VPU_CHIP_TXHD:
		vpu_hiu_setb(HHI_VPU_MEM_PD_REG2, 0x3, 2, 2);
		udelay(5);
		break;
	case VPU_CHIP_G12A:
		vpu_hiu_setb(HHI_VPU_MEM_PD_REG2, 0x3, 0, 2);
		udelay(5);
		for (i = 4; i < 18; i+=2) {
			vpu_hiu_setb(HHI_VPU_MEM_PD_REG2, 0x3, i, 2);
			udelay(5);
		}
		vpu_hiu_setb(HHI_VPU_MEM_PD_REG2, 0x3, 30, 2);
		udelay(5);
		break;
	default:
		break;
	}
	for (i = 8; i < 16; i++) {
		vpu_hiu_setb(HHI_MEM_PD_REG0, 0x1, i, 1);
		udelay(5);
	}
	udelay(20);

	/* Power down VPU domain */
	vpu_ao_setb(AO_RTI_GEN_PWR_SLEEP0, 1, 8, 1); /* PDN */

	vpu_hiu_setb(HHI_VAPBCLK_CNTL, 0, 8, 1);
	vpu_hiu_setb(HHI_VPU_CLK_CNTL, 0, 8, 1);
}

static void vpu_power_on_axg(void)
{
	unsigned int i;

	/* power up memories */
	for (i = 8; i < 16; i++) {
		vpu_hiu_setb(HHI_MEM_PD_REG0, 0, i, 1);
		udelay(5);
	}
	for (i = 0; i < 32; i+=2) {
		vpu_hiu_setb(HHI_VPU_MEM_PD_REG0, 0, i, 2);
		udelay(5);
	}
	udelay(20);
	vpu_ao_setb(AO_RTI_GEN_PWR_SLEEP0, 0, 8, 1); /* [8] power on */
	udelay(20);

	/* Reset VIU + VENC */
	/* Reset VENCI + VENCP + VADC + VENCL */
	/* Reset HDMI-APB + HDMI-SYS + HDMI-TX + HDMI-CEC */
	vpu_cbus_clr_mask(RESET0_LEVEL, ((1<<5) | (1<<10) | (1<<19) | (1<<13)));
	vpu_cbus_clr_mask(RESET1_LEVEL, (1<<5));
	vpu_cbus_clr_mask(RESET2_LEVEL, (1<<15));
	vpu_cbus_clr_mask(RESET4_LEVEL, ((1<<6) | (1<<7) | (1<<13) | (1<<5) | (1<<9) | (1<<4) | (1<<12)));
	vpu_cbus_clr_mask(RESET7_LEVEL, (1<<7));

	/* release Reset */
	vpu_cbus_set_mask(RESET0_LEVEL, ((1 << 5) | (1<<10) | (1<<19) | (1<<13)));
	vpu_cbus_set_mask(RESET1_LEVEL, (1<<5));
	vpu_cbus_set_mask(RESET2_LEVEL, (1<<15));
	vpu_cbus_set_mask(RESET4_LEVEL, ((1<<6) | (1<<7) | (1<<13) | (1<<5) | (1<<9) | (1<<4) | (1<<12)));
	vpu_cbus_set_mask(RESET7_LEVEL, (1<<7));

	/* Remove VPU_HDMI ISO */
	vpu_ao_setb(AO_RTI_GEN_PWR_SLEEP0, 0, 9, 1); /* [9] VPU_HDMI */
}

static void vpu_power_off_axg(void)
{
	unsigned int i;

	/* Power down VPU_HDMI */
	/* Enable Isolation */
	vpu_ao_setb(AO_RTI_GEN_PWR_SLEEP0, 1, 9, 1); /* ISO */
	udelay(20);

	/* power down memories */
	for (i = 0; i < 32; i+=2) {
		vpu_hiu_setb(HHI_VPU_MEM_PD_REG0, 0x3, i, 2);
		udelay(5);
	}
	for (i = 8; i < 16; i++) {
		vpu_hiu_setb(HHI_MEM_PD_REG0, 0x1, i, 1);
		udelay(5);
	}
	udelay(20);

	/* Power down VPU domain */
	vpu_ao_setb(AO_RTI_GEN_PWR_SLEEP0, 1, 8, 1); /* PDN */

	vpu_hiu_setb(HHI_VAPBCLK_CNTL, 0, 8, 1);
	vpu_hiu_setb(HHI_VPU_CLK_CNTL, 0, 8, 1);
}

static void vpu_power_on(void)
{
#ifdef VPU_DEBUG_PRINT
	VPUPR("%s\n", __func__);
#endif

	switch (vpu_chip_type) {
	case VPU_CHIP_M8:
	case VPU_CHIP_M8B:
	case VPU_CHIP_M8M2:
	case VPU_CHIP_G9TV:
	case VPU_CHIP_G9BB:
		vpu_power_on_m8_g9();
		break;
	case VPU_CHIP_AXG:
		vpu_power_on_axg();
		break;
	default:
		vpu_power_on_gx();
		break;
	}

#ifdef VPU_DEBUG_PRINT
	VPUPR("%s finish\n", __func__);
#endif
}

static void vpu_power_off(void)
{
#ifdef VPU_DEBUG_PRINT
	VPUPR("%s\n", __func__);
#endif

	switch (vpu_chip_type) {
	case VPU_CHIP_M8:
	case VPU_CHIP_M8B:
	case VPU_CHIP_M8M2:
	case VPU_CHIP_G9TV:
	case VPU_CHIP_G9BB:
		vpu_power_off_m8_g9();
		break;
	case VPU_CHIP_AXG:
		vpu_power_off_axg();
		break;
	default:
		vpu_power_off_gx();
		break;
	}

#ifdef VPU_DEBUG_PRINT
	VPUPR("%s finish\n", __func__);
#endif
}

static int get_vpu_config(void)
{
#ifdef CONFIG_OF_LIBFDT
	int nodeoffset;
	char * propdata;
#endif

	if (dts_ready == 1) {
#ifdef CONFIG_OF_LIBFDT
		nodeoffset = fdt_path_offset(dt_addr, "/vpu");
		if (nodeoffset < 0) {
			VPUERR("not find /vpu node in dts %s\n", fdt_strerror(nodeoffset));
			return -1;
		}

		propdata = (char *)fdt_getprop(dt_addr, nodeoffset, "clk_level", NULL);
		if (propdata == NULL) {
			vpu_conf.clk_level = vpu_conf.clk_level_dft;
			VPUERR("don't find clk_level in dts, set to default\n");
		} else {
			vpu_conf.clk_level = (unsigned short)(be32_to_cpup((u32*)propdata));
			if (vpu_conf.clk_level >= vpu_conf.clk_level_max) {
				VPUERR("clk_level in dts is out of support, set to default\n");
				vpu_conf.clk_level = vpu_conf.clk_level_dft;
			}
		}
		VPUPR("clk_level in dts: %u\n", vpu_conf.clk_level);
#endif
	} else {
		vpu_conf.clk_level = vpu_conf.clk_level_dft;
		VPUPR("clk_level = %u\n", vpu_conf.clk_level);
	}

	return 0;
}

int vpu_probe(void)
{
	int ret;

	dts_ready = 0;
#ifdef CONFIG_OF_LIBFDT
#ifdef CONFIG_DTB_MEM_ADDR
	dt_addr = (char *)CONFIG_DTB_MEM_ADDR;
#else
	dt_addr = (char *)0x01000000;
#endif
	ret = fdt_check_header((const void *)dt_addr);
	if (ret < 0) {
		VPUERR("vpu: check dts: %s, load default parameters\n",
			fdt_strerror(ret));
	} else {
		dts_ready = 1;
	}
#endif

	vpu_chip_detect();
	if (vpu_check())
		return -1;

	ret = get_vpu_config();
	vpu_power_on();
	set_vpu_clk(vpu_conf.clk_level);
	//vpu_power_on();

	/* vpu module init off, for power save */
	vpu_mem_pd_init_off();
	vpu_clk_gate_init_off();

	/* vpu special module init */
	vpu_module_init_config();

	return ret;
}

int vpu_remove(void)
{
	if (vpu_check())
		return -1;

	VPUPR("vpu remove\n");
	vpu_power_off();
	return 0;
}

static void vpu_clk_switch(void)
{
	struct vpu_clk_s *clk_table;
	unsigned int mux, div;

	/* step 1: switch to 2nd vpu clk patch */
	clk_table = vpu_conf.vpu_clk_table + vpu_conf.clk_level_dft;
	mux = get_vpu_clk_mux(clk_table->mux);
	vpu_hiu_setb(HHI_VPU_CLK_CNTL, mux, 25, 3);
	div = clk_table->div;
	vpu_hiu_setb(HHI_VPU_CLK_CNTL, div, 16, 7);
	vpu_hiu_setb(HHI_VPU_CLK_CNTL, 1, 24, 1);
	vpu_hiu_setb(HHI_VPU_CLK_CNTL, 1, 31, 1);
	udelay(10);
	/* step 2: adjust 1st vpu clk frequency */
	clk_table = vpu_conf.vpu_clk_table + vpu_conf.clk_level;
	vpu_hiu_setb(HHI_VPU_CLK_CNTL, 0, 8, 1);
	mux = get_vpu_clk_mux(clk_table->mux);
	vpu_hiu_setb(HHI_VPU_CLK_CNTL, mux, 9, 3);
	div = clk_table->div;
	vpu_hiu_setb(HHI_VPU_CLK_CNTL, div, 0, 7);
	vpu_hiu_setb(HHI_VPU_CLK_CNTL, 1, 8, 1);
	udelay(20);
	/* step 3: switch back to 1st vpu clk patch */
	vpu_hiu_setb(HHI_VPU_CLK_CNTL, 0, 31, 1);
	vpu_hiu_setb(HHI_VPU_CLK_CNTL, 0, 24, 1);
}

int vpu_clk_change(int level)
{
	struct vpu_clk_s *clk_table;
	unsigned int vpu_clk;
	unsigned int mux, div;

	if (vpu_check())
		return -1;

	if (level >= 100) /* regard as vpu_clk */
		level = get_vpu_clk_level(level);

	if (level >= vpu_conf.clk_level_max) {
		clk_table = vpu_conf.vpu_clk_table + (vpu_conf.clk_level_max - 1);
		VPUPR("clk out of supported range\n");
		VPUPR("clk max level: %u(&uHz)\n",
			(vpu_conf.clk_level_max - 1),
			clk_table->freq);
		return -1;
	}

	vpu_conf.clk_level = level;
	clk_table = vpu_conf.vpu_clk_table + vpu_conf.clk_level;
	vpu_clk = clk_table->freq;

	switch (vpu_chip_type) {
	case VPU_CHIP_M8:
		mux = get_vpu_clk_mux(clk_table->mux);
		div = clk_table->div;
		vpu_hiu_write(HHI_VPU_CLK_CNTL, ((mux << 9) | (div << 0) | (1<<8)));
		break;
	default:
		vpu_clk_switch();
		break;
	}

	VPUPR("set clk: %uHz, readback: %uHz(0x%x)\n",
		vpu_clk, get_vpu_clk(), vpu_hiu_read(HHI_VPU_CLK_CNTL));
	return 0;
}

void vpu_clk_get(void)
{
	if (vpu_check())
		return;

	VPUPR("clk_level: %u, clk: %uHz, reg: 0x%x\n",
		vpu_conf.clk_level, get_vpu_clk(),
		vpu_hiu_read(HHI_VPU_CLK_CNTL));
}

void vpu_info_print(void)
{
	if (vpu_check())
		return;

	printf("detect chip type: %d\n", vpu_chip_type);
	printf("clk_level:         %d(%dHz)\n"
		"clk_level default: %d(%dHz)\n"
		"clk_level max:     %d(%dHz)\n"
		"fclk_freq:         %dHz\n",
		vpu_conf.clk_level,
		(vpu_conf.vpu_clk_table + vpu_conf.clk_level)->freq,
		vpu_conf.clk_level_dft,
		(vpu_conf.vpu_clk_table + vpu_conf.clk_level_dft)->freq,
		(vpu_conf.clk_level_max - 1),
		(vpu_conf.vpu_clk_table + (vpu_conf.clk_level_max - 1))->freq,
		(vpu_conf.fclk_freq * 1000000));
}

static unsigned int vcbus_reg[] = {
	0x1b7f, /* VENC_VDAC_TST_VAL */
	0x1c30, /* ENCP_DVI_HSO_BEGIN */
	0x1d00, /* VPP_DUMMY_DATA */
	0x2730, /* VPU_VPU_PWM_V0 */
};

void vcbus_test(void)
{
	unsigned int val;
	unsigned int temp;
	int i,j;

	if (vpu_check())
		return;

	VPUPR("vcbus test:\n");
	for (i = 0; i < ARRAY_SIZE(vcbus_reg); i++) {
		for (j = 0; j < 24; j++) {
			val = vpu_vcbus_read(vcbus_reg[i]);
			printf("%02d read 0x%04x=0x%08x\n", j, vcbus_reg[i], val);
		}
		printf("\n");
	}
	temp = 0x5a5a5a5a;
	for (i = 0; i < ARRAY_SIZE(vcbus_reg); i++) {
		vpu_vcbus_write(vcbus_reg[i], temp);
		val = vpu_vcbus_read(vcbus_reg[i]);
		printf("write 0x%04x=0x%08x, readback: 0x%08x\n", vcbus_reg[i], temp, val);
	}
	for (i = 0; i < ARRAY_SIZE(vcbus_reg); i++) {
		for (j = 0; j < 24; j++) {
			val = vpu_vcbus_read(vcbus_reg[i]);
			printf("%02d read 0x%04x=0x%08x\n", j, vcbus_reg[i], val);
		}
		printf("\n");
	}
}
