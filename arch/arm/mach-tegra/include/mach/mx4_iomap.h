#ifndef __MACH_MX4_IOMAP_H
#define __MACH_MX4_IOMAP_H

#include <asm/sizes.h>
#include <../gpio-names.h>

/* Host Mobility MX-4 MAP*/
#define GMI_CS_BASE_TEGRA 0xd0000000 /* Nor base addr*/

#define CS_PIN				4

/* In 16bit mode the external address pin A[0] correlates to the internal memory address bit 1,
   external address pin A[1] to internal memory address bit 2, and so on. Found in Colibri_T20_Datasheet 5.6.1*/
#define PXA_ADDR_TO_T20_ADDR(x) (x << 1)

#define TEGRA_CAN_BASE 			GMI_CS_BASE_TEGRA + PXA_ADDR_TO_T20_ADDR(0x180)
#define TEGRA_CAN_SIZE			0x3f
#define TEGRA_CAN_INT			TEGRA_GPIO_PB5			
 
#define TEGRA_CAN2_BASE 		(GMI_CS_BASE_TEGRA + PXA_ADDR_TO_T20_ADDR(0x1A0))
#define TEGRA_CAN2_SIZE			0x3f
#define TEGRA_CAN2_INT			TEGRA_GPIO_PA6

#define TEGRA_EXT_UARTA_BASE 		(GMI_CS_BASE_TEGRA)
#define TEGRA_EXT_UARTA_INT		TEGRA_GPIO_PT2

#define TEGRA_EXT_UARTB_BASE 		(GMI_CS_BASE_TEGRA + PXA_ADDR_TO_T20_ADDR(0x80))
#define TEGRA_EXT_UARTB_INT		TEGRA_GPIO_PBB2

#define TEGRA_EXT_UARTC_BASE 		(GMI_CS_BASE_TEGRA + PXA_ADDR_TO_T20_ADDR(0x100))
#define TEGRA_EXT_UARTC_INT		TEGRA_GPIO_PK5

/* End Host Mobility MX-4 MAP*/
#endif
