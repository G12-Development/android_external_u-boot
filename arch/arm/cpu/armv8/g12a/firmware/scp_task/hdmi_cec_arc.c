/*
* Copyright (C) 2017 Amlogic, Inc. All rights reserved.
* *
This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
* *
This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
* more details.
* *
You should have received a copy of the GNU General Public License along
* with this program; if not, write to the Free Software Foundation, Inc.,
* 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
* *
Description:
*/

/**************************************************
 *           HDMI CEC uboot code                  *
 *                                                *
 **************************************************/
#ifdef CONFIG_CEC_WAKEUP
#include <cec_tx_reg.h>
#include <amlogic/aml_cec.h>
#include "hdmi_cec_arc.h"
#include "secure_apb.h"

#ifndef NULL
#define NULL ((void *)0)
#endif

#define CEC_DBG_PRINT
#define CEC_REG_DEBUG				0

cec_msg_t cec_msg;
unsigned char hdmi_cec_func_config;
cec_wakeup_t cec_wakup;
static unsigned int cec_wait_addr;
struct st_cec_mailbox_data cec_mailbox;


#ifdef CEC_DBG_PRINT
static void cec_dbg_print(char *s, int v)
{
	uart_puts(s);
	uart_put_hex(v,8);
	_udelay(100);
}

static void cec_dbg_print32(char *s, int v)
{
	uart_puts(s);
	uart_put_hex(v,32);
	_udelay(100);
}

static void cec_dbg_prints(char *s)
{
	uart_puts(s);
	_udelay(100);
}
#if CEC_REG_DEBUG
static void cec_print_reg(char *s, int v, int l)
{
	uart_puts(s);
	uart_put_hex(v, l);
	uart_puts("\n");
	_udelay(1000);
}
#endif
#else
	#define cec_dbg_print(s,v)
	#define cec_dbg_print32(s,v)
	#define cec_dbg_prints(s)
	#define cec_print_reg(s, v, l)
#endif

static void cec_reset_addr(void);
struct cec_tx_msg_t {
	unsigned char buf[16];
	unsigned char retry;
	unsigned char len;
};

#define CEX_TX_MSG_BUF_NUM	  4
#define CEC_TX_MSG_BUF_MASK	 (CEX_TX_MSG_BUF_NUM - 1)

struct cec_tx_msg {
	struct cec_tx_msg_t msg[CEX_TX_MSG_BUF_NUM];
	unsigned char send_idx;
	unsigned char queue_idx;
};

struct cec_tx_msg cec_tx_msgs = {};


static int cec_strlen(char *p)
{
	int i=0;

	while (*p++)
		i++;
	return i;
}

static void *cec_memcpy(void *memto, const void *memfrom, unsigned int size)
{
	char *tempfrom = (char *)memfrom;
	char *tempto = (char *)memto;

	if ((memto == NULL) || (memfrom == NULL))
		return NULL;
	while (size -- > 0)
		*tempto++ = *tempfrom++;
	return memto;
}

void cec_update_config_data(void *data, unsigned int size)
{
	unsigned int i;

	memcpy((void *)&cec_mailbox, data, size);

	if (cec_mailbox.cec_config & CEC_CFG_DBG_EN) {
		cec_dbg_print32("size:", size);
		cec_dbg_prints("\n");
		cec_dbg_print32("cec_config:", cec_mailbox.cec_config);
		cec_dbg_prints("\n");
		cec_dbg_print32("phy_addr:", cec_mailbox.phy_addr);
		cec_dbg_prints("\n");
		cec_dbg_print32("vendor_id:", cec_mailbox.vendor_id);
		cec_dbg_prints("\n");
		cec_dbg_prints("osd_name:");
		for (i = 0; i < 16; i++) {
			cec_dbg_print(" ", cec_mailbox.osd_name[i]);
		}
		cec_dbg_prints("\n");
	}
}

void cec_update_phyaddress(unsigned int phyaddr)
{
	unsigned int data;

	cec_mailbox.phy_addr = cec_mailbox.phy_addr & 0xffff0000;
	cec_mailbox.phy_addr |= phyaddr & 0xffff;

	data = (readl(P_AO_DEBUG_REG1) & 0xffff0000) | (phyaddr & 0xffff);
	writel(data, P_AO_DEBUG_REG1);
	cec_dbg_print32("update phyaddr:", phyaddr);
	cec_dbg_prints("\n");
}

void cec_update_func_cfg(unsigned int cfg)
{
	cec_mailbox.cec_config = cfg;

	if (hdmi_cec_func_config & CEC_CFG_DBG_EN) {
		cec_dbg_print("cec_config:", cfg);
		cec_dbg_prints("\n");
	}
}

static void waiting_aocec_free(void) {
	do {
		unsigned long cnt = 0;
		while (readl(P_AO_CEC_RW_REG) & (1<<23))
		{
			if (5000 == cnt++)
			{
				break;
			}
		}
	} while(0);
}

static unsigned long cec_rd_reg(unsigned long addr)
{
	unsigned long data32;
	waiting_aocec_free();
	data32  = 0;
	data32 |= 0    << 16;  /* [16]   cec_reg_wr */
	data32 |= 0    << 8;   /* [15:8] cec_reg_wrdata */
	data32 |= addr << 0;   /* [7:0]  cec_reg_addr */
	writel(data32, P_AO_CEC_RW_REG);
	waiting_aocec_free();
	data32 = ((readl(P_AO_CEC_RW_REG)) >> 24) & 0xff;
	return (data32);
} /* cec_rd_reg */

static void cec_wr_reg (unsigned long addr, unsigned long data)
{
	unsigned long data32;
	waiting_aocec_free();
	data32  = 0;
	data32 |= 1    << 16;  /* [16]   cec_reg_wr */
	data32 |= data << 8;   /* [15:8] cec_reg_wrdata */
	data32 |= addr << 0;   /* [7:0]  cec_reg_addr */
	writel(data32, P_AO_CEC_RW_REG);
} /* aocec_wr_only_reg */

static inline void cec_set_bits_dwc(uint32_t reg, uint32_t bits,
				       uint32_t start, uint32_t len)
{
	unsigned int tmp;
	tmp = cec_rd_reg(reg);
	tmp &= ~(((1 << len) - 1) << start);
	tmp |=  (bits << start);
	cec_wr_reg(reg, tmp);
}

void cec_set_reg_bits(unsigned long addr, unsigned int value,
	unsigned int offset, unsigned int len)
{
	unsigned int data32 = 0;

	data32 = readl(addr);
	data32 &= ~(((1 << len) - 1) << offset);
	data32 |= (value & ((1 << len) - 1)) << offset;
	writel(data32, addr);
}

static void cec_rx_read_pos_plus(void)
{
	(cec_msg.rx_read_pos ==  cec_msg.rx_buf_size - 1) ?
				(cec_msg.rx_read_pos = 0) :
				(cec_msg.rx_read_pos++);
}

static void cec_arbit_bit_time_set(unsigned bit_set, unsigned time_set)
{
	//11bit:bit[10:0]
	switch (bit_set) {
	case 3:
		//3 bit
		cec_wr_reg(AO_CEC_TXTIME_4BIT_BIT7_0, time_set & 0xff);
		cec_wr_reg(AO_CEC_TXTIME_4BIT_BIT10_8, (time_set >> 8) & 0x7);
		break;
		//5 bit
	case 5:
		cec_wr_reg(AO_CEC_TXTIME_2BIT_BIT7_0, time_set & 0xff);
		cec_wr_reg(AO_CEC_TXTIME_2BIT_BIT10_8, (time_set >> 8) & 0x7);
		//7 bit
	case 7:
		cec_wr_reg(AO_CEC_TXTIME_17MS_BIT7_0, time_set & 0xff);
		cec_wr_reg(AO_CEC_TXTIME_17MS_BIT10_8, (time_set >> 8) & 0x7);
		break;
	default:
		break;
	}
}

static void cec_hw_buf_clear(void)
{
	cec_wr_reg(CEC_RX_MSG_CMD, RX_DISABLE);
	cec_wr_reg(CEC_TX_MSG_CMD, TX_ABORT);
	cec_wr_reg(CEC_RX_CLEAR_BUF, 1);
	cec_wr_reg(CEC_TX_CLEAR_BUF, 1);
	_udelay(100);
	cec_wr_reg(CEC_RX_CLEAR_BUF, 0);
	cec_wr_reg(CEC_TX_CLEAR_BUF, 0);
	_udelay(100);
	cec_wr_reg(CEC_RX_MSG_CMD, RX_NO_OP);
	cec_wr_reg(CEC_TX_MSG_CMD, TX_NO_OP);
}

void cec_hw_reset(void)
{
	unsigned int reg;

	cec_dbg_prints("cec hw reset\n");

	reg = (0 << 31) |
		(0 << 30) |
		(1 << 28) | /* clk_div0/clk_div1 in turn */
		((732-1) << 12) |/* Div_tcnt1 */
		((733-1) << 0); /* Div_tcnt0 */
	writel(reg, P_AO_CEC_CLK_CNTL_REG0);

	reg = (0 << 13) |
		((11-1)  << 12) |
		((8-1)	<<	0);
	writel(reg, P_AO_CEC_CLK_CNTL_REG1);

	/*enable clk in*/
	reg = readl(P_AO_CEC_CLK_CNTL_REG0);
	reg |= (1 << 31);
	writel(reg, P_AO_CEC_CLK_CNTL_REG0);
	/*enable clk out*/
	_udelay(100);
	reg |= (1 << 30);
	writel(reg, P_AO_CEC_CLK_CNTL_REG0);

	reg = readl(P_AO_RTI_PWR_CNTL_REG0);
	reg |=	(0x01 << 14);/* enable the crystal clock*/
	writel(reg, P_AO_RTI_PWR_CNTL_REG0);

	// Assert SW reset AO_CEC
	writel(0x1, P_AO_CEC_GEN_CNTL);
	// Enable gated clock (Normal mode).
	writel(readl(P_AO_CEC_GEN_CNTL) | (1<<1), P_AO_CEC_GEN_CNTL);

	// Release SW reset
	writel(readl(P_AO_CEC_GEN_CNTL) & ~(1<<0), P_AO_CEC_GEN_CNTL);

	/*tx , rx interrupt*/
	writel(((readl(P_AO_CEC_INTR_MASKN) & 0xfffffff8) | (6)),
		P_AO_CEC_INTR_MASKN);

	/* set up pinmux */
	writel(((readl(P_PERIPHS_PIN_MUX_B) & 0xffff0fff) | (4 << 12)),
		P_PERIPHS_PIN_MUX_B);

	cec_arbit_bit_time_set(3, 0x118);
	cec_arbit_bit_time_set(5, 0x000);
	cec_arbit_bit_time_set(7, 0x2aa);
}

static unsigned char remote_cec_ll_rx(void)
{
	int i;
	int print = 1;
	unsigned char rx_msg_length = cec_rd_reg(CEC_RX_MSG_LENGTH) + 1;

	cec_dbg_prints("cec R:");
	for (i = 0; i < rx_msg_length; i++) {
		cec_msg.buf[cec_msg.rx_write_pos].msg[i] = cec_rd_reg(CEC_RX_MSG_0_HEADER + i);
		if (print) {
			cec_dbg_print(" ", cec_msg.buf[cec_msg.rx_write_pos].msg[i]);
		}
		if (i == 1 && cec_msg.buf[cec_msg.rx_write_pos].msg[i] == CEC_OC_VENDOR_COMMAND_WITH_ID) {
			/* do not print command with ID */
			print = 0;
		}
	}
	cec_msg.buf[cec_msg.rx_write_pos].msg_len = rx_msg_length;
	cec_dbg_prints("\n");

	return 0;
}
static void cec_buf_clear(void)
{
	int i;

	for (i = 0; i < 16; i++)
		cec_msg.buf[cec_msg.rx_read_pos].msg[i] = 0;
}

static void cec_tx_buf_init(void)
{
	int i, j;
	for (j = 0; j < CEX_TX_MSG_BUF_NUM; j++) {
		for (i = 0; i < 16; i++) {
			cec_tx_msgs.msg[j].buf[i] = 0;
		}
		cec_tx_msgs.msg[j].retry = 0;
		cec_tx_msgs.msg[j].len = 0;
	}
}

static int cec_queue_tx_msg(unsigned char *msg, unsigned char len)
{
	int s_idx, q_idx;

	s_idx = cec_tx_msgs.send_idx;
	q_idx = cec_tx_msgs.queue_idx;
	if (((q_idx + 1) & CEC_TX_MSG_BUF_MASK) == s_idx) {
		cec_dbg_prints("tx buffer full, abort msg\n");
		cec_reset_addr();
		return -1;
	}
	if (len && msg) {
		cec_memcpy(cec_tx_msgs.msg[q_idx].buf, msg, len);
		cec_tx_msgs.msg[q_idx].len = len;
		cec_tx_msgs.queue_idx = (q_idx + 1) & CEC_TX_MSG_BUF_MASK;
	}
	return 0;
}

static int cec_triggle_tx(unsigned char *msg, unsigned char len)
{
	int i;

	if ((TX_IDLE == cec_rd_reg(CEC_TX_MSG_STATUS)) ||
	    (TX_DONE == cec_rd_reg(CEC_TX_MSG_STATUS))) {
		cec_dbg_prints("cec T:");
		for (i = 0; i < len; i++) {
			cec_wr_reg(CEC_TX_MSG_0_HEADER + i, msg[i]);
			cec_dbg_print(" ", msg[i]);
		}
		cec_dbg_prints("\n");
		cec_wr_reg(CEC_TX_MSG_LENGTH, len-1);
		cec_wr_reg(CEC_TX_MSG_CMD, TX_REQ_CURRENT); //TX_REQ_NEXT
		return 0;
	}
	return -1;
}

static int remote_cec_ll_tx(unsigned char *msg, unsigned char len)
{
	cec_queue_tx_msg(msg, len);
	cec_triggle_tx(msg, len);

	return 0;
}

static int ping_cec_ll_tx(unsigned char *msg, unsigned char len)
{
	/*int i;*/
	int ret = TX_ERROR;
	unsigned int n = 800;
	unsigned int reg;

	cec_triggle_tx(msg, len);

	while (cec_rd_reg(CEC_TX_MSG_STATUS) == TX_BUSY) {
		/*
		 * waiting tx to idle if it is busy, other device may in tx state
		 */
		_udelay(500);
		if (n-- <= 1)
			break;
	}

	while (1) {
		reg = cec_rd_reg(CEC_TX_MSG_STATUS);
		if ( reg == TX_DONE ) {
			ret = TX_DONE;
			cec_wr_reg(CEC_TX_MSG_CMD, TX_NO_OP);
			cec_dbg_prints("ping_cec_ll_tx:TX_DONE\n");
			break;
		}

		if (reg == TX_ERROR) {
			ret = TX_ERROR;
			cec_wr_reg(CEC_TX_MSG_CMD, TX_NO_OP);
			cec_dbg_prints("ping_cec_ll_tx:TX_NACK\n");
			break;
		}

		if (reg == TX_BUSY) {
			cec_dbg_prints("ping_cec_ll_tx:TX_BUSY\n");
			ret = TX_BUSY;
			cec_wr_reg(CEC_TX_MSG_CMD, TX_NO_OP);
			break;
		}

		break;
	}

	return ret;
}

#define DEVICE_TV		0
#define DEVICE_RECORDER		1
#define DEVICE_RESERVED		2
#define DEVICE_TUNER		3
#define DEVICE_PLAYBACK		4
#define DEVICE_AUDIO_SYSTEM	5
#define DEVICE_PURE_CEC_SWITCH	6
#define DEVICE_VIDEO_PROCESSOR	7

static unsigned char log_addr_to_devtye(unsigned int addr)
{
	static unsigned char addr_map[] = {
		DEVICE_TV,
		DEVICE_RECORDER,
		DEVICE_RECORDER,
		DEVICE_TUNER,
		DEVICE_PLAYBACK,
		DEVICE_AUDIO_SYSTEM,
		DEVICE_TUNER,
		DEVICE_TUNER,
		DEVICE_PLAYBACK,
		DEVICE_RECORDER,
		DEVICE_TUNER,
		DEVICE_PLAYBACK,
		DEVICE_RESERVED,
		DEVICE_RESERVED,
		DEVICE_TV
	};
	return addr_map[addr & 0xf];
}

static void cec_report_physical_address(void)
{
	unsigned char msg[5];
	/*unsigned char phy_addr_ab = (readl(AO_DEBUG_REG1) >> 8) & 0xff;*/
	/*unsigned char phy_addr_cd = readl(AO_DEBUG_REG1) & 0xff;*/

	msg[0] = ((cec_msg.log_addr & 0xf) << 4)| CEC_BROADCAST_ADDR;
	msg[1] = CEC_OC_REPORT_PHYSICAL_ADDRESS;
	msg[2] = (cec_mailbox.phy_addr >> 8) & 0xff;
	msg[3] = cec_mailbox.phy_addr & 0xff;
	msg[4] = log_addr_to_devtye(cec_msg.log_addr);

	remote_cec_ll_tx(msg, 5);
}

static void cec_report_device_power_status(int dst)
{
	unsigned char msg[3];

	msg[0] = ((cec_msg.log_addr & 0xf) << 4) | (dst & 0xf);
	msg[1] = CEC_OC_REPORT_POWER_STATUS;
	msg[2] = cec_msg.power_status;

	remote_cec_ll_tx(msg, 3);
}

static void cec_set_stream_path(void)
{
	unsigned char phy_addr_ab = (cec_mailbox.phy_addr >> 8) & 0xff;
	unsigned char phy_addr_cd = cec_mailbox.phy_addr & 0xff;

	if ((hdmi_cec_func_config >> CEC_FUNC_MASK) & 0x1) {
		if ((hdmi_cec_func_config >> AUTO_POWER_ON_MASK) & 0x1) {
			if ((phy_addr_ab == cec_msg.buf[cec_msg.rx_read_pos].msg[2]) &&
			    (phy_addr_cd == cec_msg.buf[cec_msg.rx_read_pos].msg[3]))  {
				cec_msg.cec_power = 0x1;
				cec_msg.active_source = 1;
				uart_puts("stream path power on\n");
			}
		}
	}
}

void cec_routing_change(void)
{
	unsigned char phy_addr_ab = (cec_mailbox.phy_addr >> 8) & 0xff;
	unsigned char phy_addr_cd = cec_mailbox.phy_addr & 0xff;

	if ((hdmi_cec_func_config >> CEC_FUNC_MASK) & 0x1) {
		if ((hdmi_cec_func_config >> AUTO_POWER_ON_MASK) & 0x1) {
			/* wake up if routing destination is self */
			if ((phy_addr_ab == cec_msg.buf[cec_msg.rx_read_pos].msg[4]) &&
				(phy_addr_cd == cec_msg.buf[cec_msg.rx_read_pos].msg[5])) {
				cec_msg.cec_power = 0x1;
				cec_msg.active_source = 1;
				uart_puts("routing chg power on\n");
			}
		}
	}
}

static void cec_device_vendor_id(void)
{
	unsigned char msg[5];
	unsigned int vendor_id = cec_mailbox.vendor_id;

	msg[0] = ((cec_msg.log_addr & 0xf) << 4)| CEC_BROADCAST_ADDR;
	msg[1] = CEC_OC_DEVICE_VENDOR_ID;
	msg[2] = (vendor_id >> 16) & 0xff;
	msg[3] = (vendor_id >> 8) & 0xff;
	msg[4] = (vendor_id >> 0) & 0xff;

	remote_cec_ll_tx(msg, 5);
}

static void cec_menu_status_smp(int menu_status, int dst)
{
	unsigned char msg[3];

	msg[0] = ((cec_msg.log_addr & 0xf) << 4) | (dst & 0xf);
	msg[1] = CEC_OC_MENU_STATUS;
	msg[2] = menu_status;

	remote_cec_ll_tx(msg, 3);
}

static void cec_give_deck_status(int dst)
{
	unsigned char msg[3];

	msg[0] = ((cec_msg.log_addr & 0xf) << 4) | (dst & 0xf);
	msg[1] = CEC_OC_DECK_STATUS;
	msg[2] = 0x1a;

	remote_cec_ll_tx(msg, 3);
}

/*static void cec_standby(void)
{
	unsigned char msg[2];

	msg[0] = ((cec_msg.log_addr & 0xf) << 4) | CEC_BROADCAST_ADDR;
	msg[1] = CEC_OC_STANDBY;

	remote_cec_ll_tx(msg, 2);
}*/

static void cec_set_osd_name(int dst)
{
	unsigned char msg[16];
	unsigned char osd_len = cec_mailbox.osd_name[15];

	msg[0] = ((cec_msg.log_addr & 0xf) << 4) | (dst & 0xf);
	msg[1] = CEC_OC_SET_OSD_NAME;
	if (osd_len > 0 && osd_len <= 14) {
		cec_memcpy(&msg[2], cec_mailbox.osd_name, osd_len);
	} else {
		osd_len = cec_strlen(CONFIG_CEC_OSD_NAME);
		cec_memcpy(&msg[2], CONFIG_CEC_OSD_NAME, osd_len);
	}

	remote_cec_ll_tx(msg, osd_len + 2);
}

static void cec_get_version(int dst)
{
	unsigned char dest_log_addr = cec_msg.log_addr & 0xf;
	unsigned char msg[3];

	if (0xf != dest_log_addr) {
		msg[0] = ((cec_msg.log_addr & 0xf) << 4) | (dst & 0xf);
		msg[1] = CEC_OC_CEC_VERSION;
		msg[2] = CEC_VERSION_14A;
		remote_cec_ll_tx(msg, 3);
	}
}

static int check_addr(int phy_addr)
{
	unsigned int local_addr = (cec_mailbox.phy_addr) & 0xffff;
	unsigned int i, mask = 0xf000, a, b;

	for (i = 0; i < 4; i++) {
		if (!(local_addr & mask)) {
			break;
		}
		a = local_addr & mask;
		b = phy_addr & mask;
		if (a != b)	{/* node is not same */
			cec_dbg_prints("addr fail 1\n");
			return 0;
		}
		mask >>= 4;
	}
	cec_dbg_prints("addr ok\n");
	return 1;
}

static int is_playback_dev(int addr)
{
	if (addr != CEC_PLAYBACK_DEVICE_1_ADDR &&
	    addr != CEC_PLAYBACK_DEVICE_2_ADDR &&
	    addr != CEC_PLAYBACK_DEVICE_3_ADDR) {
		return 0;
	}
	return 1;
}

int is_phy_addr_ready(cec_msg_t *msg)
{
	/*unsigned int source_addr = readl(AO_RTI_STATUS_REG1);*/

	if (msg == NULL)
		return 0;
	if (((hdmi_cec_func_config >> CEC_FUNC_MASK) & 0x1) &&
	    ((hdmi_cec_func_config >> AUTO_POWER_ON_MASK) & 0x1) &&
	    msg->cec_power) {
		if (cec_wakup.wk_phy_addr &&
		    cec_wakup.wk_logic_addr &&
			(cec_wakup.wk_phy_addr != 0xFFFF)) {
				uart_puts("is_phy_addr_ready 0x");
				uart_put_hex(cec_wakup.wk_phy_addr, 16);
				uart_puts("\n");
				return 1;
		}
	}

	return 0;
}

void cec_save_port_id(void)
{
	unsigned int phy_addr;
	int i, port_id;

	phy_addr = cec_wakup.wk_phy_addr;
	uart_puts("save port id\n");
	if ((phy_addr == 0xFFFF) || ((phy_addr & 0xF000) == 0)) {
		cec_wakup.wk_port_id = 0xFF;
		set_cec_val1(*((unsigned int *)&cec_wakup));
		return;
	}

	for (i = 0; i < PHY_ADDR_LEN; i++) {
		port_id = (phy_addr >> (PHY_ADDR_LEN - i - 1)*4) & 0xF;
		if (port_id == 0) {
			 port_id = (phy_addr >> (PHY_ADDR_LEN - i)*4) & 0xF;
			 break;
		}
	}
	cec_wakup.wk_port_id = port_id;
	set_cec_val1(*((unsigned int *)&cec_wakup));
}

static unsigned int cec_handle_message(void)
{
	unsigned char opcode;
	unsigned char source;
	unsigned int  phy_addr;

	source = (cec_msg.buf[cec_msg.rx_read_pos].msg[0] >> 4) & 0xf;
	if (((hdmi_cec_func_config>>CEC_FUNC_MASK) & 0x1) &&
		(cec_msg.buf[cec_msg.rx_read_pos].msg_len > 1)) {
		opcode = cec_msg.buf[cec_msg.rx_read_pos].msg[1];
		switch (opcode) {
		case CEC_OC_GET_CEC_VERSION:
			cec_get_version(source);
			break;
		case CEC_OC_GIVE_DECK_STATUS:
			cec_give_deck_status(source);
			break;
		case CEC_OC_GIVE_PHYSICAL_ADDRESS:
			cec_report_physical_address();
			break;
		case CEC_OC_GIVE_DEVICE_VENDOR_ID:
			cec_device_vendor_id();
			break;
		case CEC_OC_GIVE_OSD_NAME:
			cec_set_osd_name(source);
			break;
		case CEC_OC_SET_STREAM_PATH:
			cec_set_stream_path();
			break;
		case CEC_OC_ROUTING_CHANGE:
			cec_routing_change();
			break;
		case CEC_OC_GIVE_DEVICE_POWER_STATUS:
			cec_report_device_power_status(source);
			break;
		case CEC_OC_USER_CONTROL_PRESSED:
			if (((hdmi_cec_func_config >> CEC_FUNC_MASK) & 0x1) &&
			    ((hdmi_cec_func_config >> AUTO_POWER_ON_MASK) & 0x1) &&
			     (cec_msg.buf[cec_msg.rx_read_pos].msg_len == 3) &&
			    ((0x40 == cec_msg.buf[cec_msg.rx_read_pos].msg[2]) ||
			     (0x6d == cec_msg.buf[cec_msg.rx_read_pos].msg[2]) ||
			     (0x09 == cec_msg.buf[cec_msg.rx_read_pos].msg[2]) )) {
				cec_msg.cec_power = 0x1;
				uart_puts("user power on\n");
			}
			break;
		case CEC_OC_MENU_REQUEST:
			cec_menu_status_smp(DEVICE_MENU_INACTIVE, source);
			break;

		/* TV Wake up by image/text view on */
		case CEC_OC_IMAGE_VIEW_ON:
		case CEC_OC_TEXT_VIEW_ON:
			if (((hdmi_cec_func_config >> CEC_FUNC_MASK) & 0x1) &&
			    ((hdmi_cec_func_config >> AUTO_POWER_ON_MASK) & 0x1) &&
			    (!is_playback_dev(cec_msg.log_addr))) {
				/* request active source needed */
				phy_addr = 0xffff;
				cec_msg.cec_power = 0x1;
				/*writel(wake, P_AO_RTI_STATUS_REG1);*/
				cec_wakup.wk_logic_addr = source;
				cec_wakup.wk_phy_addr = phy_addr;
				set_cec_val1(*((unsigned int *)&cec_wakup));
				uart_puts("otp power on\n");
			}
			break;

		/* TV Wake up by active source*/
		case CEC_OC_ACTIVE_SOURCE:
			phy_addr = (cec_msg.buf[cec_msg.rx_read_pos].msg[2] << 8) |
				   (cec_msg.buf[cec_msg.rx_read_pos].msg[3] << 0);
			if (((hdmi_cec_func_config >> CEC_FUNC_MASK) & 0x1) &&
			    ((hdmi_cec_func_config >> AUTO_POWER_ON_MASK) & 0x1) &&
			    (!is_playback_dev(cec_msg.log_addr) && check_addr(phy_addr))) {
				cec_msg.cec_power = 0x1;
				cec_msg.active_source = 1;
				uart_puts("source : 0x");
				uart_put_hex(source, 8);
				uart_puts("\n");
				/*writel(wake, P_AO_RTI_STATUS_REG1);*/
				cec_wakup.wk_logic_addr = source;
				cec_wakup.wk_phy_addr = phy_addr;
				set_cec_val1(*((unsigned int *)&cec_wakup));
				uart_puts("active src power on\n");
			}
			break;
		case CEC_OC_REPORT_PHYSICAL_ADDRESS:

			break;
		default:
			break;
		}
	}
	cec_rx_read_pos_plus();
	return 0;
}

void cec_set_log_addr(int l_add)
{
	cec_wr_reg(CEC_LOGICAL_ADDR0, 0);
	cec_hw_buf_clear();
	cec_wr_reg(CEC_LOGICAL_ADDR0, (l_add & 0xf));
	_udelay(100);
	cec_wr_reg(CEC_LOGICAL_ADDR0, (0x1 << 4) | (l_add & 0xf));
}

unsigned int cec_get_log_addr(void)
{
	return cec_rd_reg(CEC_LOGICAL_ADDR0);
}

static void cec_reset_addr(void)
{
	cec_hw_reset();
	cec_wr_reg(CEC_LOGICAL_ADDR0, 0);
	cec_hw_buf_clear();
	cec_set_log_addr(cec_msg.log_addr & 0x0f);
}

unsigned int cec_handler(void)
{
	unsigned char s_idx;
	static int busy_count = 0;

	if (0xf == cec_rd_reg(CEC_RX_NUM_MSG)) {
		cec_wr_reg(CEC_RX_CLEAR_BUF, 0x1);
		cec_wr_reg(CEC_RX_CLEAR_BUF, 0x0);
		cec_wr_reg(CEC_RX_MSG_CMD,  RX_ACK_CURRENT);
		cec_wr_reg(CEC_RX_MSG_CMD, RX_NO_OP);
		cec_dbg_prints("error:hw_buf overflow\n");
	}

	switch (cec_rd_reg(CEC_RX_MSG_STATUS)) {
	case RX_DONE:
		if (1 == cec_rd_reg(CEC_RX_NUM_MSG)) {
			remote_cec_ll_rx();
			(cec_msg.rx_write_pos == cec_msg.rx_buf_size - 1) ? (cec_msg.rx_write_pos = 0) : (cec_msg.rx_write_pos++);
		}
		cec_wr_reg(CEC_RX_MSG_CMD, RX_ACK_CURRENT);
		cec_wr_reg(CEC_RX_MSG_CMD, RX_NO_OP);
		cec_dbg_prints("RX_OK\n");
		break;
	case RX_ERROR:
		cec_dbg_prints("RX_ERROR\n");
		if (TX_ERROR == cec_rd_reg(CEC_TX_MSG_STATUS)) {
			cec_dbg_prints("TX_ERROR\n");
			cec_reset_addr();
		} else {
			cec_dbg_prints("TX_other\n");
			cec_wr_reg(CEC_RX_MSG_CMD,  RX_ACK_CURRENT);
			cec_wr_reg(CEC_RX_MSG_CMD, RX_NO_OP);
		}
		break;
	default:
		break;
	}

	switch (cec_rd_reg(CEC_TX_MSG_STATUS)) {
	case TX_DONE:
		cec_wr_reg(CEC_TX_MSG_CMD, TX_NO_OP);
		cec_tx_msgs.send_idx = (cec_tx_msgs.send_idx + 1) & CEC_TX_MSG_BUF_MASK;
		s_idx = cec_tx_msgs.send_idx;
		if (cec_tx_msgs.send_idx != cec_tx_msgs.queue_idx) {
			cec_dbg_prints("TX_OK\n");
			cec_triggle_tx(cec_tx_msgs.msg[s_idx].buf,
				       cec_tx_msgs.msg[s_idx].len);
		} else {
			cec_dbg_prints("TX_END\n");
		}
		busy_count = 0;
		break;

	case TX_ERROR:
		cec_dbg_prints("@TX_ERROR\n");
		if (RX_ERROR == cec_rd_reg(CEC_RX_MSG_STATUS)) {
			cec_dbg_prints("@RX_ERROR\n");
			cec_reset_addr();
		} else {
			cec_wr_reg(CEC_TX_MSG_CMD, TX_NO_OP);
			s_idx = cec_tx_msgs.send_idx;
			if (cec_tx_msgs.msg[s_idx].retry < 5) {
				cec_tx_msgs.msg[s_idx].retry++;
				cec_triggle_tx(cec_tx_msgs.msg[s_idx].buf,
					       cec_tx_msgs.msg[s_idx].len);
			} else {
				cec_dbg_prints("TX retry too much, abort msg\n");
				cec_tx_msgs.send_idx = (cec_tx_msgs.send_idx + 1) & CEC_TX_MSG_BUF_MASK;
			}
		}
		busy_count = 0;
		break;

	 case TX_IDLE:
		s_idx = cec_tx_msgs.send_idx;
		if (cec_tx_msgs.send_idx != cec_tx_msgs.queue_idx) { // triggle tx if idle
			cec_triggle_tx(cec_tx_msgs.msg[s_idx].buf,
				       cec_tx_msgs.msg[s_idx].len);
		}
		busy_count = 0;
		break;

	case TX_BUSY:
		busy_count++;
		if (busy_count >= 2000) {
			uart_puts("busy too long, reset hw\n");
			cec_reset_addr();
			busy_count = 0;
		}
		break;

	 default:
		break;
	}
	if (cec_msg.rx_read_pos != cec_msg.rx_write_pos) {
		cec_handle_message();
	}

	return 0;
}

/*static void check_standby(void)
{
	if (((cec_msg.log_addr & 0xf) == 0) &&
	    ((hdmi_cec_func_config >> CEC_FUNC_MASK) & 0x1) &&
	    ((hdmi_cec_func_config >> ONE_TOUCH_STANDBY_MASK) & 0x1)) {
		cec_standby();
	}
}*/

static unsigned int retry = 0;
static int i = 0;
static int *probe = NULL;
void cec_node_init(void)
{
	static int regist_devs = 0;
	int tx_stat;
	unsigned char msg[8];
	unsigned int kern_log_addr = (readl(P_AO_DEBUG_REG1) >> 16) & 0xf;
	int player_dev[3][3] =
		{{CEC_PLAYBACK_DEVICE_1_ADDR, CEC_PLAYBACK_DEVICE_2_ADDR, CEC_PLAYBACK_DEVICE_3_ADDR},
		 {CEC_PLAYBACK_DEVICE_2_ADDR, CEC_PLAYBACK_DEVICE_3_ADDR, CEC_PLAYBACK_DEVICE_1_ADDR},
		 {CEC_PLAYBACK_DEVICE_3_ADDR, CEC_PLAYBACK_DEVICE_1_ADDR, CEC_PLAYBACK_DEVICE_2_ADDR}};

	cec_wait_addr = 0;
	if (retry >= 12) {  /* retry all device addr */
		cec_msg.log_addr = 0x0f;
		uart_puts("failed on retried all possible address\n");
		return;
	}
	writel(0, P_AO_RTI_STATUS_REG1);
	memset(&cec_wakup, 0, sizeof(cec_wakup));
	if (probe == NULL) {
		cec_msg.rx_read_pos = 0;
		cec_msg.rx_write_pos = 0;
		cec_msg.rx_buf_size = 2;

		cec_msg.power_status = 1;
		cec_msg.active_source = 0;
		cec_msg.cec_power = 0;
		cec_tx_msgs.send_idx = 0;
		cec_tx_msgs.queue_idx = 0;
		cec_tx_buf_init();
		cec_buf_clear();
		_udelay(100);
		/*
		 * use kernel cec logic address to detect which logic address is the
		 * started one to allocate.
		 */
		uart_puts("\n");
		/* we don't need probe TV address */
		if (!is_playback_dev(kern_log_addr)) {
			cec_set_log_addr(kern_log_addr);
			msg[0] = (kern_log_addr << 4) | kern_log_addr;
			ping_cec_ll_tx(msg, 1);
			cec_msg.log_addr = 0x10 | kern_log_addr;
			_udelay(100);
			cec_dbg_print("log_addr:0x", cec_msg.log_addr);
			cec_dbg_print("regAddr:", cec_rd_reg(CEC_LOGICAL_ADDR0));
			uart_puts("\n");
			probe = NULL;
			regist_devs = 0;
			i = 0;
			retry = 0;
			/* check_standby(); */
			return;
		}
		for (i = 0; i < 3; i++) {
			if (kern_log_addr == player_dev[i][0]) {
				probe = player_dev[i];
				break;
			}
		}
		if (probe == NULL) {
			probe = player_dev[0];
		}
		i = 0;
	}

	cec_set_log_addr(probe[i]);
	msg[0] = (probe[i]<<4) | probe[i];
	tx_stat = ping_cec_ll_tx(msg, 1);
	if (tx_stat == TX_BUSY) {   /* can't get cec bus */
		retry++;
		cec_hw_reset();
		cec_set_log_addr(probe[i]);
		if (!(retry & 0x03)) {
			cec_dbg_print("retry too much, log_addr:0x", probe[i]);
			uart_puts("\n");
		} else {
			i -= 1;
		}
	} else if (tx_stat == TX_ERROR) {
		_udelay(100);
		cec_msg.log_addr = probe[i];
		cec_set_log_addr(cec_msg.log_addr & 0xf);
		cec_dbg_print("log_addr:0x", cec_msg.log_addr);
		cec_dbg_print("reg-addr:", cec_rd_reg(CEC_LOGICAL_ADDR0));
		uart_puts("\n");
		probe = NULL;
		regist_devs = 0;
		i = 0;
		retry = 0;
		/*remote_cec_ll_tx(msg, 3);*/
		return;
	} else if (tx_stat == TX_DONE) {
		cec_dbg_print("sombody takes cec log_addr:0x", probe[i]);
		uart_puts("\n");
		regist_devs |= (1 << i);
		retry += (4 - (retry & 0x03));
		if (regist_devs == 0x07) {
			/* No avilable logical address */
			cec_msg.log_addr = 0x0f;
			cec_set_log_addr(15);
			uart_puts("CEC allocate logic address failed\n");
		}
	}
	i++;
	if (i == 3) {
		i = 0;
	}
}

int cec_suspend_wakeup_chk(void)
{
	int exit_reason = 0;

	if ((cec_msg.cec_power == 0x1) &&
		(hdmi_cec_func_config & 0x1)) {
		if (cec_wait_addr++ < 100) {
			if (cec_msg.active_source) {
				cec_save_port_id();
				exit_reason = CEC_WAKEUP;
				uart_puts("check wakeup\n");
			} else {
				uart_puts(".");
			}
		} else {
			exit_reason = CEC_WAKEUP;
			uart_puts("timeout wakeup\n");
		}
	}

	if (exit_reason == CEC_WAKEUP)
		return 1;
	else
		return 0;
}

int cec_suspend_handle(void)
{
	int exit_reason = 0;

	if (cec_msg.log_addr) {
		if (hdmi_cec_func_config & 0x1) {
			cec_handler();
			if (cec_msg.cec_power == 0x1) {
				if (cec_msg.active_source) {
					cec_save_port_id();
					/*cec power key*/
					exit_reason = CEC_WAKEUP;
					uart_puts("message wakeup\n");
				}
			}
		}
	} else if (hdmi_cec_func_config & 0x1)
		cec_node_init();

	if (exit_reason == CEC_WAKEUP) {
		uart_puts("active source:");
		uart_put_hex(cec_msg.active_source, 8);
		uart_puts("\n");

		uart_puts("wk_logic_addr:");
		uart_put_hex(cec_wakup.wk_logic_addr, 8);
		uart_puts("\n");

		uart_puts("wk_phy_addr:0x");
		uart_put_hex(cec_wakup.wk_phy_addr, 16);
		uart_puts("\n");

		uart_puts("wk_port_id:0x");
		uart_put_hex(cec_wakup.wk_port_id, 8);
		uart_puts("\n");
		return 1;
	} else
		return 0;
}

void cec_start_config(void)
{
	retry = 0;
	probe = NULL;
	cec_msg.active_source = 0;
	cec_msg.cec_power = 0;
	cec_wait_addr = 0;
	i = 0;
	hdmi_cec_func_config = readl(P_AO_DEBUG_REG0);

	if (cec_mailbox.cec_config & CEC_CFG_DBG_EN) {
		uart_puts(CEC_VERSION);
		uart_puts("cfg0:0x");
		uart_put_hex(hdmi_cec_func_config, 32);
		uart_puts("\n");
		uart_puts("cfg1:0x");
		uart_put_hex(readl(P_AO_DEBUG_REG1), 32);
		uart_puts("\n");
	}

	cec_mailbox.phy_addr = readl(P_AO_DEBUG_REG1);

	/*cec is enable*/
	if (hdmi_cec_func_config & CEC_CFG_FUNC_EN) {
		cec_hw_reset();
		cec_node_init();
	}
}
#endif
