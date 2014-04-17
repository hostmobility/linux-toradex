/*
 * Copyright 2012 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <linux/kernel.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <asm/io.h>
#include <mach/hardware.h>
#include <mach/clock.h>
#include <asm/proc-fns.h>
#include <asm/system.h>
#include "crm_regs.h"
#include "regs-anadig.h"
#include "regs-pm.h"

/* PTB19, GPIO 41, SO-DIMM 45 */
#define SW1_WAKEUP_GPIO		41
#define SW1_WAKEUP_PIN		12
#define	SW1_PORT1_PCR9_ADDR	MVF_IO_ADDRESS(0x4004a024)

/* PTB20, GPIO 42, SO-DIMM 43 */
#define SW2_WAKEUP_GPIO		42
#define SW2_WAKEUP_PIN		13
#define	SW2_PORT1_PCR10_ADDR	MVF_IO_ADDRESS(0x4004a028)

static void __iomem *gpc_base = MVF_GPC_BASE;

void gpc_set_wakeup(void)
{
	__raw_writel(0xffffffff, gpc_base + GPC_IMR1_OFFSET);
	/* unmask UART0 */
	__raw_writel(0xdfffffff, gpc_base + GPC_IMR2_OFFSET);
	/* unmask WKPU12/13 interrupt */
	__raw_writel(0xefffffff, gpc_base + GPC_IMR3_OFFSET);
	/* unmask GPIO4 interrupt */
	__raw_writel(0xffff7fff, gpc_base + GPC_IMR4_OFFSET);

	return;
}

void enable_wkpu(u32 source, u32 rise_fall)
{
	u32 tmp;
	tmp = __raw_readl(MVF_WKPU_BASE + WKPU_IRER_OFFSET);
	__raw_writel(tmp | 1 << source, MVF_WKPU_BASE + WKPU_IRER_OFFSET);

	tmp = __raw_readl(MVF_WKPU_BASE + WKPU_WRER_OFFSET);
	__raw_writel(tmp | 1 << source, MVF_WKPU_BASE + WKPU_WRER_OFFSET);

	if (rise_fall == RISING_EDGE_ENABLED) {
		tmp = __raw_readl(MVF_WKPU_BASE + WKPU_WIREER_OFFSET);
		tmp |= 1 << source;
		__raw_writel(tmp, MVF_WKPU_BASE + WKPU_WIREER_OFFSET);
	} else {
		tmp = __raw_readl(MVF_WKPU_BASE + WKPU_WIFEER_OFFSET);
		tmp |= 1 << source;
		__raw_writel(tmp, MVF_WKPU_BASE + WKPU_WIFEER_OFFSET);
	}
}

static irqreturn_t wkpu_irq(int irq, void *dev_id)
{
	u32 wisr;

	wisr = __raw_readl(MVF_WKPU_BASE + WKPU_WISR_OFFSET);
	if (wisr)
		__raw_writel(wisr, MVF_WKPU_BASE + WKPU_WISR_OFFSET);

	return IRQ_NONE;
}

static void mvf_configure_wakeup_pin(int gpio, int wkup_pin,
		const volatile void __iomem *pinctl_addr)
{
	u32 tmp;

	/* config SW1 for waking up system */
	gpio_request_one(gpio, GPIOF_IN, "SW wakeup");
	gpio_set_value(gpio, 0);

	/* Disable IRQC interrupt/dma request in pin contorl register */
	tmp = __raw_readl(pinctl_addr);
	tmp &= ~0x000f0000;
	__raw_writel(tmp, pinctl_addr);

	/* enable WKPU interrupt for this wakeup pin */
	enable_wkpu(wkup_pin, FALLING_EDGE_ENABLED);
}

/* set cpu multiple modes before WFI instruction */
void mvf_cpu_lp_set(enum mvf_cpu_pwr_mode mode)
{
	u32 ccm_ccsr, ccm_clpcr, ccm_ccr;
	int ret;

	if ((mode == LOW_POWER_STOP) || (mode == STOP_MODE)) {
		ret = request_irq(MVF_INT_WKPU0, wkpu_irq,
			IRQF_DISABLED, "wkpu irq", NULL);

		if (ret)
			printk(KERN_ERR "Request wkpu IRQ failed\n");

		mvf_configure_wakeup_pin(SW1_WAKEUP_GPIO, SW1_WAKEUP_PIN,
				SW1_PORT1_PCR9_ADDR);
		mvf_configure_wakeup_pin(SW2_WAKEUP_GPIO, SW2_WAKEUP_PIN,
				SW2_PORT1_PCR10_ADDR);
	}

	ccm_ccr = __raw_readl(MXC_CCM_CCR);
	ccm_ccsr = __raw_readl(MXC_CCM_CCSR);

	switch (mode) {
	case WAIT_MODE:
		break;
	case LOW_POWER_RUN:
		ccm_ccr |= MXC_CCM_CCR_FIRC_EN;
		__raw_writel(ccm_ccr, MXC_CCM_CCR);
		ccm_ccsr &= ~MXC_CCM_CCSR_FAST_CLK_SEL_MASK;
		ccm_ccsr &= ~MXC_CCM_CCSR_SLOW_CLK_SEL_MASK;
		__raw_writel(ccm_ccsr, MXC_CCM_CCSR);

		/* switch system clock to FIRC 24MHz */
		ccm_ccsr &= ~MXC_CCM_CCSR_SYS_CLK_SEL_MASK;
		__raw_writel(ccm_ccsr, MXC_CCM_CCSR);

		__raw_writel(__raw_readl(PLL3_480_USB1_BASE_ADDR) &
			~ANADIG_PLL_ENABLE, PLL3_480_USB1_BASE_ADDR);
		__raw_writel(__raw_readl(PLL3_480_USB2_BASE_ADDR) &
			~ANADIG_PLL_ENABLE, PLL3_480_USB2_BASE_ADDR);
		__raw_writel(__raw_readl(PLL4_AUDIO_BASE_ADDR) &
			~ANADIG_PLL_ENABLE, PLL4_AUDIO_BASE_ADDR);
		__raw_writel(__raw_readl(PLL6_VIDEO_BASE_ADDR) &
			~ANADIG_PLL_ENABLE, PLL6_VIDEO_BASE_ADDR);
		__raw_writel(__raw_readl(PLL5_ENET_BASE_ADDR) &
			~ANADIG_PLL_ENABLE, PLL5_ENET_BASE_ADDR);
		__raw_writel(__raw_readl(PLL1_SYS_BASE_ADDR) &
			~ANADIG_PLL_ENABLE, PLL1_SYS_BASE_ADDR);
		break;
	case STOP_MODE:
	case LOW_POWER_STOP:
		gpc_set_wakeup();

		/* unmask UART1 in low-power mode */
		__raw_writel(0xfffeffff, MXC_CCM_CCGR0);
		/* unmask WKUP and GPC in low-power mode */
		__raw_writel(0xfeefffff, MXC_CCM_CCGR4);
		/* unmask DDRC in low-power mode */
		__raw_writel(0xefffffff, MXC_CCM_CCGR6);

		ccm_ccr |= MXC_CCM_CCR_FIRC_EN;
		__raw_writel(ccm_ccr, MXC_CCM_CCR);
		ccm_ccsr &= ~MXC_CCM_CCSR_FAST_CLK_SEL_MASK;
		ccm_ccsr &= ~MXC_CCM_CCSR_SLOW_CLK_SEL_MASK;
		__raw_writel(ccm_ccsr, MXC_CCM_CCSR);

		/* switch system clock to FIRC 24MHz */
		ccm_ccsr &= ~MXC_CCM_CCSR_SYS_CLK_SEL_MASK;
		__raw_writel(ccm_ccsr, MXC_CCM_CCSR);

		/* on-chip oscillator will not be
		 * powered down in stop mode */
		ccm_clpcr = __raw_readl(MXC_CCM_CLPCR);
		ccm_clpcr &= ~MXC_CCM_CLPCR_SBYOS;
		ccm_clpcr &= ~MXC_CCM_CLPCR_M_CORE1_WFI;
		ccm_clpcr &= ~MXC_CCM_CLPCR_M_CORE0_WFI;
		ccm_clpcr &= ~MXC_CCM_CLPCR_ARM_CLK_LPM;
		__raw_writel(ccm_clpcr, MXC_CCM_CLPCR);

		/* mask stop mode to Anatop */
		__raw_writel(ccm_clpcr & ~MXC_CCM_CLPCR_ANATOP_STOP_MODE,
			MXC_CCM_CLPCR);

		break;
	default:
		printk(KERN_WARNING "UNKNOW cpu power mode: %d\n", mode);
		return;
	}
}

void arch_idle(void)
{
	cpu_do_idle();
}
