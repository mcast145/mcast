/*
 *  Copyright (C) 2012 Altera Corporation <www.altera.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/arch/hardware.h>
#include <asm/arch/clock_manager.h>

#include "../../board/cl/mitysom-5csx/build.h"
#include "../../board/cl/mitysom-5csx/pinmux_config.h"
#include "../../board/cl/mitysom-5csx/pll_config.h"
#include "../../board/cl/mitysom-5csx/sdram/sdram_config.h"
#include "../../board/cl/mitysom-5csx/reset_config.h"

#include "socfpga_common.h"
#ifdef CONFIG_SPL_BUILD
#include "../../board/cl/mitysom-5csx/iocsr_config_cyclone5.h"
#endif

/* Define machine type for Cyclone 5 */
#define CONFIG_MACH_TYPE 4251

#define COMPANY_NAME "Critical Link"
#define PRODUCT_NAME "MitySOM-5CSx"
#define CONFIG_IDENT_STRING " " COMPANY_NAME " " PRODUCT_NAME

#define CONFIG_SYS_PROMPT		"MitySOM-5CSx # "

/* Need I2C support for factory config eeprom */
#define CONFIG_CMD_I2C
#define CONFIG_I2C_MULTI_BUS
#undef CONFIG_SYS_I2C_SPEED
#define CONFIG_SYS_I2C_SPEED                10000
#define CONFIG_DW_I2C
#undef CONFIG_SYS_I2C_BASE
#define CONFIG_SYS_I2C_BASE		SOCFPGA_I2C0_ADDRESS
#define CONFIG_SYS_I2C_BASE1	SOCFPGA_I2C1_ADDRESS
#define CONFIG_SYS_I2C_BASE2	SOCFPGA_I2C2_ADDRESS
#define CONFIG_SYS_I2C_BASE3	SOCFPGA_I2C3_ADDRESS
#undef CONFIG_SYS_I2C_SLAVE
#define CONFIG_SYS_I2C_SLAVE            1
#define CONFIG_HARD_I2C
#define CONFIG_SYS_I2C_BUS_MAX 4

#define CONFIG_AUTO_COMPLETE

/* use "hush" command parser */
#define CONFIG_CMDLINE_EDITING
#define CONFIG_CMD_RUN

/* look for preboot env command to run before boot */
#define CONFIG_PREBOOT

#define CONFIG_OVERWRITE_ETHADDR_ONCE
#define CONFIG_ETHADDR "00:00:00:00:00:00"

/*
 * Hardware drivers
 */
#define CONFIG_SOCFPGA_GPIO		1
#define CONFIG_CMD_GPIO			1

/*
 * QSPI support
 */
/* We are running the QSPI slow until we firgure out the issue with high speed use */
#undef CONFIG_SF_DEFAULT_SPEED
#define CONFIG_SF_DEFAULT_SPEED		(10000000)

#define CONFIG_EMAC_BASE CONFIG_EMAC1_BASE
#define CONFIG_EPHY_PHY_ADDR		CONFIG_EPHY1_PHY_ADDR
#define CONFIG_PHY_INTERFACE_MODE	SOCFPGA_PHYSEL_ENUM_RGMII

#define CONFIG_CMD_UNZIP

#endif	/* __CONFIG_H */
