/*
 * arch/arm/mach-tegra/mx4-pm.c
 *
 * Custom suspend action for mx4 board.
 *
 * Copyright (c) 2013-2016, Host Mobility AB. All rights reserved.
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
#include <asm/mach/arch.h>
#include <linux/io.h>
#include <asm/mach-types.h>

#include <linux/kernel.h>
#include <linux/syscore_ops.h>

#include <mach/gpio.h>

#include "gpio-names.h"

#define __BITMASK0(len)				((1 << (len)) - 1)
#define __BITMASK(start, len)		(__BITMASK0(len) << (start))
#define REG_BIT(bit)				(1 << (bit))
#define REG_FIELD(val, start, len)	(((val) & __BITMASK0(len)) << (start))
#define REG_FIELD_MASK(start, len)	(~(__BITMASK((start), (len))))
#define REG_GET_FIELD(val, start, len)	(((val) >> (start)) & __BITMASK0(len))
#define TEGRA_GMI_PHYS				0x70009000
#define TEGRA_GMI_BASE				IO_TO_VIRT(TEGRA_GMI_PHYS)
#define CONFIG_REG					(TEGRA_GMI_BASE + 0x00)
#define STATUS_REG					(TEGRA_GMI_BASE + 0x04)
#define CONFIG_SNOR_CS(val) 		REG_FIELD((val), 4, 3)
#define CONFIG_GO					REG_BIT(31)
#define POWER_DOWN					REG_BIT(19)


static struct gpio gpios_to_handle[] = {	
	{TEGRA_GPIO_PY4,	GPIOF_OUT_INIT_LOW,	"P35 - FF-TXD"},
	{TEGRA_GPIO_PY5,	GPIOF_OUT_INIT_LOW,	"P33 - FF-RXD"},
	{TEGRA_GPIO_PC2,	GPIOF_OUT_INIT_LOW,	"P21 - STD-TXD"},
	{TEGRA_GPIO_PK7,	GPIOF_OUT_INIT_LOW,	"P32 - BT-RTS"},
	{TEGRA_GPIO_PJ7,	GPIOF_OUT_INIT_LOW,	"P38 - BT-TXD"},

	
	{TEGRA_GPIO_PI0,	GPIOF_OUT_INIT_LOW,	"P89 - nWE"},
	{TEGRA_GPIO_PI1,	GPIOF_OUT_INIT_LOW,	"P91 - nOE"},		
	{TEGRA_GPIO_PW0,	GPIOF_OUT_INIT_LOW,	"P93 - RDnWR"},	
	{TEGRA_GPIO_PK2,	GPIOF_OUT_INIT_LOW,	"P105 - nCS"},	
	
	{TEGRA_GPIO_PJ6,	GPIOF_OUT_INIT_LOW,	"P111 - MA00"},
	{TEGRA_GPIO_PJ5,	GPIOF_OUT_INIT_LOW,	"P113 - MA01"},
	{TEGRA_GPIO_PW6,	GPIOF_OUT_INIT_LOW,	"P115 - MA02"},
	{TEGRA_GPIO_PW7,	GPIOF_OUT_INIT_LOW,	"P117 - MA03"},
	{TEGRA_GPIO_PC0,	GPIOF_OUT_INIT_LOW,	"P119 - MA04"},
	{TEGRA_GPIO_PA1,	GPIOF_OUT_INIT_LOW,	"P121 - MA05"},
	{TEGRA_GPIO_PU0,	GPIOF_OUT_INIT_LOW,	"P123 - MA06"},
	{TEGRA_GPIO_PU1,	GPIOF_OUT_INIT_LOW,	"P125 - MA07"},
	{TEGRA_GPIO_PU2,	GPIOF_OUT_INIT_LOW,	"P110 - MA08"},
	{TEGRA_GPIO_PU3,	GPIOF_OUT_INIT_LOW,	"P112 - MA09"},
	{TEGRA_GPIO_PU4,	GPIOF_OUT_INIT_LOW,	"P113 - MA10"},
	

	{TEGRA_GPIO_PG0,	GPIOF_OUT_INIT_LOW,	"P149 - MD00"},
	{TEGRA_GPIO_PG1,	GPIOF_OUT_INIT_LOW,	"P151 - MD01"},
	{TEGRA_GPIO_PG2,	GPIOF_OUT_INIT_LOW,	"P153 - MD02"},
	{TEGRA_GPIO_PG3,	GPIOF_OUT_INIT_LOW,	"P155 - MD03"},
	{TEGRA_GPIO_PG4,	GPIOF_OUT_INIT_LOW,	"P157 - MD04"},
	{TEGRA_GPIO_PG5,	GPIOF_OUT_INIT_LOW,	"P169 - MD05"},
	{TEGRA_GPIO_PG6,	GPIOF_OUT_INIT_LOW,	"P161 - MD06"},
	{TEGRA_GPIO_PG7,	GPIOF_OUT_INIT_LOW,	"P163 - MD07"},
	{TEGRA_GPIO_PH0,	GPIOF_OUT_INIT_LOW,	"P165 - MD08"},
	{TEGRA_GPIO_PH1,	GPIOF_OUT_INIT_LOW,	"P167 - MD09"},
	{TEGRA_GPIO_PH2,	GPIOF_OUT_INIT_LOW,	"P169 - MD10"},
	{TEGRA_GPIO_PH3,	GPIOF_OUT_INIT_LOW,	"P171 - MD11"},
	{TEGRA_GPIO_PH4,	GPIOF_OUT_INIT_LOW,	"P173 - MD12"},
	{TEGRA_GPIO_PH5,	GPIOF_OUT_INIT_LOW,	"P175 - MD13"},
	{TEGRA_GPIO_PH6,	GPIOF_OUT_INIT_LOW,	"P177 - MD14"},
	{TEGRA_GPIO_PH7,	GPIOF_OUT_INIT_LOW,	"P179 - MD15"},
};

#if 0
static struct tegra_pingroup_config mx4_pinconfig[] = {
	{TEGRA_PINGROUP_SDIO1,	TEGRA_MUX_UARTE,	TEGRA_PUPD_PULL_UP,	TEGRA_TRI_NORMAL},
	{TEGRA_PINGROUP_UAD,	TEGRA_MUX_IRDA,		TEGRA_PUPD_NORMAL,	TEGRA_TRI_NORMAL},
	{TEGRA_PINGROUP_GMC,	TEGRA_MUX_GMI, 		TEGRA_PUPD_NORMAL, 	TEGRA_TRI_NORMAL},
};
#endif

static int tegra_mx4_custom_suspend(void)
{
	int length = sizeof(gpios_to_handle) / sizeof(struct gpio);
	int err = 0, i;

	printk( KERN_INFO "Entering custom mx4 suspend rutine!");

	for (i = 0; i < length; i++) {
		err = gpio_request_one(gpios_to_handle[i].gpio, gpios_to_handle[i].flags, "function tri-stated");
		if (err) {
			pr_warning("gpio_request(%d) failed, err = %d",
				   gpios_to_handle[i].gpio, err);
		}
		tegra_gpio_enable(gpios_to_handle[i].gpio);	
	}
#if 0
	/* tri-stating GMI_WR_N on SODIMM pin 99 nPWE */
	gpio_request(TEGRA_GPIO_PT5, "no GMI_WR_N on 99");
	gpio_direction_output(TEGRA_GPIO_PT5, 1);

	/* tri-stating GMI_WR_N on SODIMM pin 93 RDnWR */
	gpio_request(TEGRA_GPIO_PT6, "GMI_WR_N on 93 RDnWR");
	gpio_direction_output(TEGRA_GPIO_PT6, 1);

	writel(CONFIG_GO | POWER_DOWN, CONFIG_REG);	
#endif
	return 0;
}

static void tegra_mx4_custom_resume(void)
{
	int length = sizeof(gpios_to_handle) / sizeof(struct gpio);
	int i, err = 0;

	printk( KERN_INFO "Entering custom mx4 resume rutine!");

	for (i = 0; i < length; i++) {		
		err = gpio_request(gpios_to_handle[i].gpio, "function tri-stated");
		if (err) {
			pr_warning("gpio_request(%d) failed, err = %d",
				   gpios_to_handle[i].gpio, err);
		}
		tegra_gpio_disable(gpios_to_handle[i].gpio);
		gpio_free(gpios_to_handle[i].gpio);
	}
#if 0
	/* not tri-stating GMI_WR_N on SODIMM pin 99 nPWE */
	gpio_request(TEGRA_GPIO_PT5, "no GMI_WR_N on 99");
	gpio_direction_output(TEGRA_GPIO_PT5, 1);

	/* not tri-stating GMI_WR_N on SODIMM pin 93 RDnWR */
	gpio_request(TEGRA_GPIO_PT6, "GMI_WR_N on 93 RDnWR");
	gpio_direction_output(TEGRA_GPIO_PT6, 1);		
#endif
}

static struct syscore_ops tegra_mx4_custom_syscore_ops = {
	.suspend = tegra_mx4_custom_suspend,
	.resume = tegra_mx4_custom_resume,
};

static int tegra_mx4_custom_syscore_init(void){
	register_syscore_ops(&tegra_mx4_custom_syscore_ops);
	return 0;
}
arch_initcall(tegra_mx4_custom_syscore_init);