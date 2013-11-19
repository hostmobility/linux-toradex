#ifndef __MACH_MX4_IOMAP_H
#define __MACH_MX4_IOMAP_H

#include <asm/sizes.h>
#include <../gpio-names.h>

/* Host Mobility MX-4 MAP*/
#define GMI_CS_BASE_TEGRA 0xd0000000 /* Nor base addr*/

#define SNOR_CS_PIN				4

/* In 16bit mode the external address pin A[0] correlates to the internal memory address bit 1,
   external address pin A[1] to internal memory address bit 2, and so on. Found in Colibri_T20_Datasheet 5.6.1*/
#define PXA_ADDR_TO_T20_ADDR(x) (x << 1)

#ifndef CONFIG_HM_GTT_CAN /* MX4 GTT uses different addressing on CAN */
#define TEGRA_CAN_BASE 			GMI_CS_BASE_TEGRA + PXA_ADDR_TO_T20_ADDR(0x180)
#define TEGRA_CAN_SIZE			0x3f
#define TEGRA_CAN_INT			TEGRA_GPIO_PB5			
 
#define TEGRA_CAN2_BASE 		(GMI_CS_BASE_TEGRA + PXA_ADDR_TO_T20_ADDR(0x1A0))
#define TEGRA_CAN2_SIZE			0x3f
#define TEGRA_CAN2_INT			TEGRA_GPIO_PA6
#else
#define TEGRA_CAN_BASE 			GMI_CS_BASE_TEGRA + PXA_ADDR_TO_T20_ADDR(0x000)
#define TEGRA_CAN_SIZE			0x3f
#define TEGRA_CAN_INT			TEGRA_GPIO_PB5			
 
#define TEGRA_CAN2_BASE 		(GMI_CS_BASE_TEGRA + PXA_ADDR_TO_T20_ADDR(0x020))
#define TEGRA_CAN2_SIZE			0x3f
#define TEGRA_CAN2_INT			TEGRA_GPIO_PA6

#define TEGRA_CAN3_BASE         (GMI_CS_BASE_TEGRA + PXA_ADDR_TO_T20_ADDR(0x040))
#define TEGRA_CAN3_SIZE			0x3f
#define TEGRA_CAN3_INT			TEGRA_GPIO_PN0

#define TEGRA_CAN4_BASE         (GMI_CS_BASE_TEGRA + PXA_ADDR_TO_T20_ADDR(0x060))
#define TEGRA_CAN4_SIZE			0x3f
#define TEGRA_CAN4_INT			TEGRA_GPIO_PN1

#define TEGRA_CAN5_BASE         (GMI_CS_BASE_TEGRA + PXA_ADDR_TO_T20_ADDR(0x080))
#define TEGRA_CAN5_SIZE			0x3f
#define TEGRA_CAN5_INT			TEGRA_GPIO_PN2

#define TEGRA_CAN6_BASE         (GMI_CS_BASE_TEGRA + PXA_ADDR_TO_T20_ADDR(0x0A0))
#define TEGRA_CAN6_SIZE			0x3f
#define TEGRA_CAN6_INT			TEGRA_GPIO_PN3
#endif /* CONFIG_HM_GTT_CAN */

#define TEGRA_EXT_UARTA_BASE 		(GMI_CS_BASE_TEGRA)
#define TEGRA_EXT_UARTA_INT			TEGRA_GPIO_PT2

#define TEGRA_EXT_UARTB_BASE 		(GMI_CS_BASE_TEGRA + PXA_ADDR_TO_T20_ADDR(0x80))
#define TEGRA_EXT_UARTB_INT			TEGRA_GPIO_PBB2

#define TEGRA_EXT_UARTC_BASE 		(GMI_CS_BASE_TEGRA + PXA_ADDR_TO_T20_ADDR(0x100))
#define TEGRA_EXT_UARTC_INT			TEGRA_GPIO_PK5

#define GPIO_WAKEUP_PIN				TEGRA_GPIO_PC7

/* End Host Mobility MX-4 MAP*/
#endif
