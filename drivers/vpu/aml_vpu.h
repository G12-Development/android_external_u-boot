
/*
 * drivers/vpu/aml_vpu.h
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

#ifndef __VPU_PARA_H__
#define __VPU_PARA_H__

//#define VPU_DEBUG_PRINT
#define VPUPR(fmt, args...)     printf("vpu: "fmt"", ## args)
#define VPUERR(fmt, args...)    printf("vpu: error: "fmt"", ## args)

enum vpu_chip_e {
	VPU_CHIP_M8 = 0,
	VPU_CHIP_M8B, /* 1 */
	VPU_CHIP_M8M2, /* 2 */
	VPU_CHIP_G9TV, /* 3 */
	VPU_CHIP_G9BB, /* 4 */
	VPU_CHIP_GXBB, /* 5 */
	VPU_CHIP_GXTVBB, /* 6 */
	VPU_CHIP_GXL, /* 7 */
	VPU_CHIP_GXM, /* 8 */
	VPU_CHIP_TXL, /* 9 */
	VPU_CHIP_TXLX, /* 10 */
	VPU_CHIP_AXG, /* 11 */
	VPU_CHIP_TXHD, /* 12 */
	VPU_CHIP_G12A, /* 13 */
	VPU_CHIP_MAX,
};

struct fclk_div_s {
	unsigned int fclk_id;
	unsigned int fclk_mux;
	unsigned int fclk_div;
};

struct vpu_clk_s {
	unsigned int freq;
	unsigned int mux;
	unsigned int div;
};

struct vpu_conf_s {
	unsigned int clk_level_dft;
	unsigned int clk_level_max;
	unsigned int clk_level;
	unsigned int fclk_freq; /* unit: MHz */
	struct fclk_div_s *fclk_div_table;
	struct vpu_clk_s  *vpu_clk_table;
};

/* #define LIMIT_VPU_CLK_LOW */

/* ************************************************ */
/* VPU frequency table, important. DO NOT modify!! */
/* ************************************************ */
/* fixed pll frequency */
#define FCLK_2550M    2550 /* unit: MHz */
#define FCLK_2000M    2000 /* unit: MHz */

/* M8: */
/* freq Max=364M, default=255M */
#define CLK_LEVEL_DFT_M8       3
#define CLK_LEVEL_MAX_M8       5
#define FCLK_TYPE_M8           FCLK_2550M
/* M8M2: */
/* freq Max=364M, default=255M */
#define CLK_LEVEL_DFT_M8M2     3
#define CLK_LEVEL_MAX_M8M2     5
#define FCLK_TYPE_M8M2         FCLK_2550M
/* M8baby */
/* freq max=212MHz, default=212MHz. */
#define CLK_LEVEL_DFT_M8B      2
#define CLK_LEVEL_MAX_M8B      3
#define FCLK_TYPE_M8B          FCLK_2550M
/* G9TV */
/* freq max=696M, default=637M */
#define CLK_LEVEL_DFT_G9TV     7
#define CLK_LEVEL_MAX_G9TV     9
#define FCLK_TYPE_G9TV         FCLK_2550M
/* G9BB */
/* freq max=212M, default=212M */
#define CLK_LEVEL_DFT_G9BB     2
#define CLK_LEVEL_MAX_G9BB     3
#define FCLK_TYPE_G9BB         FCLK_2550M
/* GXBB */
/* freq max=666M, default=666M */
#define CLK_LEVEL_DFT_GXBB     7
#define CLK_LEVEL_MAX_GXBB     8
#define FCLK_TYPE_GXBB         FCLK_2000M
/* GXTVBB */
/* freq max=666M, default=666M */
#define CLK_LEVEL_DFT_GXTVBB   7
#define CLK_LEVEL_MAX_GXTVBB   8
#define FCLK_TYPE_GXTVBB       FCLK_2000M
/* GXL */
/* freq max=666M, default=666M */
#define CLK_LEVEL_DFT_GXL      7
#define CLK_LEVEL_MAX_GXL      8
#define FCLK_TYPE_GXL          FCLK_2000M
/* GXM */
/* freq max=666M, default=666M */
#define CLK_LEVEL_DFT_GXM      7
#define CLK_LEVEL_MAX_GXM      8
#define FCLK_TYPE_GXM          FCLK_2000M
/* TXL */
/* freq max=666M, default=666M */
#define CLK_LEVEL_DFT_TXL      7
#define CLK_LEVEL_MAX_TXL      8
#define FCLK_TYPE_TXL          FCLK_2000M

/* TXLX */
/* freq max=666M, default=666M */
#define CLK_LEVEL_DFT_TXLX     7
#define CLK_LEVEL_MAX_TXLX     8
#define FCLK_TYPE_TXLX         FCLK_2000M

/* AXG */
/* freq max=250M, default=250M */
#define CLK_LEVEL_DFT_AXG      3
#define CLK_LEVEL_MAX_AXG      4
#define FCLK_TYPE_AXG          FCLK_2000M

/* TXHD */
/* freq max=400M, default=400M */
#define CLK_LEVEL_DFT_TXHD     5
#define CLK_LEVEL_MAX_TXHD     6
#define FCLK_TYPE_TXHD         FCLK_2000M

/* G12A */
/* freq max=666M, default=666M */
#define CLK_LEVEL_DFT_G12A     7
#define CLK_LEVEL_MAX_G12A     8
#define FCLK_TYPE_G12A         FCLK_2000M

/* vpu clk setting */
enum vpu_mux_e {
	FCLK_DIV4 = 0,
	FCLK_DIV3,
	FCLK_DIV5,
	FCLK_DIV7,
	MPLL_CLK1,
	VID_PLL_CLK,
	VID2_PLL_CLK,
	GPLL_CLK,
	FCLK_DIV_MAX,
};

static struct fclk_div_s fclk_div_table_gxb[] = {
	/* id,         mux,  div */
	{FCLK_DIV4,    0,    4},
	{FCLK_DIV3,    1,    3},
	{FCLK_DIV5,    2,    5},
	{FCLK_DIV7,    3,    7},
	{FCLK_DIV_MAX, 8,    1},
};

static struct fclk_div_s fclk_div_table_g12a[] = {
	/* id,         mux,  div */
	{FCLK_DIV3,    0,    3},
	{FCLK_DIV4,    1,    4},
	{FCLK_DIV5,    2,    5},
	{FCLK_DIV7,    3,    7},
	{FCLK_DIV_MAX, 8,    1},
};

static struct vpu_clk_s vpu_clk_table_m8[] = {
	/* m8, m8m2, g9tv, g9bb, fpll=2550M */
	/* frequency   clk_mux       div */
	{106250000,    FCLK_DIV3,    7}, /* 0 */
	{159375000,    FCLK_DIV4,    3}, /* 1 */
	{212500000,    FCLK_DIV3,    3}, /* 2 */
	{255000000,    FCLK_DIV5,    1}, /* 3 */
	{364300000,    FCLK_DIV7,    0}, /* 4 */ /* M8M2 use gp_pll */
	{425000000,    FCLK_DIV3,    1}, /* 5 */
	{510000000,    FCLK_DIV5,    0}, /* 6 */
	{637500000,    FCLK_DIV4,    0}, /* 7 */
	{696000000,    GPLL_CLK,     0}, /* 8 */ /* G9TV use gp1_pll */
	{850000000,    FCLK_DIV3,    0}, /* 9 */
};

static struct vpu_clk_s vpu_clk_table_gxb[] = {
	/* gxbb, gxtvbb, gxl, txl, txlx, axg fpll=2000M */
	/* frequency   clk_mux       div */
	{100000000,    FCLK_DIV5,    3}, /* 0 */
	{166667000,    FCLK_DIV3,    3}, /* 1 */
	{200000000,    FCLK_DIV5,    1}, /* 2 */
	{250000000,    FCLK_DIV4,    1}, /* 3 */
	{333333000,    FCLK_DIV3,    1}, /* 4 */
	{400000000,    FCLK_DIV5,    0}, /* 5 */
	{500000000,    FCLK_DIV4,    0}, /* 6 */
	{666667000,    FCLK_DIV3,    0}, /* 7 */
	{696000000,    GPLL_CLK,     0}, /* 8 */ /* invalid */
	{850000000,    GPLL_CLK,     0}, /* 9 */ /* invalid */
};

/* ************************************************ */
/* extern function */
/* ************************************************ */
extern void udelay(unsigned long usec);
extern int printf(const char *fmt, ...);
/* ************************************************ */

#endif
