/*
 *  Copyright (C) 2013 Critical Link LLC <www.criticallink.com>
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

#include <common.h>
#include <asm/arch/reset_manager.h>
#include <asm/arch/system_manager.h>
#include <asm/io.h>

#include <netdev.h>
#include <mmc.h>
#include <asm/arch/interrupts.h>
#include <asm/arch/sdram.h>
#include <phy.h>
#include <micrel.h>
#include <miiphy.h>
#include <../drivers/net/designware.h>
#include <environment.h>

#include "config_block.h"

#define I2C_EEPROM_BUS 0
#define I2C_EEPROM_ADDR 0x50

DECLARE_GLOBAL_DATA_PTR;

/*
 * Print Board information
 */
int checkboard(void)
{
	puts("BOARD : Critical Link MityARM-5CSX Module\n");
	return 0;
}
#ifndef NUM_EEPROM_RETRIES
#define NUM_EEPROM_RETRIES	2
#endif

/*
 * Read header information from EEPROM into global structure.
 */
int read_eeprom(void)
{

	int retries, rv = 0;

	for(retries = 0; retries <= NUM_EEPROM_RETRIES; ++retries)
	{
		if(retries) {
			printf("Retrying [%d] ...\n", retries);
		}

		/* Check if baseboard eeprom is available */
#ifdef CONFIG_I2C_MULTI_BUS
		i2c_set_bus_num(I2C_EEPROM_BUS);
#endif
		if (rv) {
			printf("Could not probe the EEPROM; something fundamentally "
				"wrong on the I2C bus.\n");
			continue; /* retry */
		}

		/* try and read our configuration block */
		rv = get_factory_config_block();
		if (rv < 0) {
			printf("I2C Error reading factory config block\n");
			continue; /* retry */
		}
		/* No I2C issues, data was either OK or not entered... */
		if (rv > 0)
			printf("Bad I2C configuration found\n");
		break;
	}
	factory_config_block.ModelNumber[31] = '\0';

#if defined(CONFIG_SPL_BUILD)
	if (!rv)
		printf("%s - Model  No: %32s Serial No: %d\n", PRODUCT_NAME, 
			factory_config_block.ModelNumber, factory_config_block.SerialNumber);
#endif

	return rv;
}

/*
 * Initialization function which happen at early stage of c code
 */
int board_early_init_f(void)
{
#ifdef CONFIG_HW_WATCHDOG
	/* disable the watchdog when entering U-Boot */
	watchdog_disable();
#endif
	return 0;
}

/*
 * Miscellaneous platform dependent initialisations
 */
int board_init(void)
{
	int rv = 0;
	uint8_t eth_addr[10];
	/* adress of boot parameters (ATAG or FDT blob) */
	gd->bd->bi_boot_params = 0x00000100;

	rv = read_eeprom();
	if ( 0 != rv) {
		memset(&factory_config_block, '\0', sizeof(factory_config_block));
	}

        memcpy(eth_addr,factory_config_block.MACADDR, 6);

        if(is_valid_ether_addr(eth_addr)) {
		char tmp[24];
		char* env_ethaddr = getenv("ethaddr");
                sprintf((char *)tmp, "%02x:%02x:%02x:%02x:%02x:%02x", eth_addr[0],
                        eth_addr[1], eth_addr[2], eth_addr[3], eth_addr[4], eth_addr[5]);

		debug("FC Ethernet MAC: %s Env: %s\n",tmp, env_ethaddr);
		if(!env_ethaddr || (0 != strncmp(tmp, env_ethaddr,18)))
			setenv("ethaddr", (char *)tmp);
        }

	return rv;
}

/* EMAC related setup */
#define MICREL_KSZ9021_EXTREG_CTRL 11
#define MICREL_KSZ9021_EXTREG_DATA_WRITE 12

/*
 * Write the extended registers in the PHY
 */
static int eth_emdio_write(struct eth_device *dev, u8 addr, u16 reg, u16 val,
		int (*mii_write)(struct eth_device *, u8, u8, u16))

{
	int ret = (*mii_write)(dev, addr,
		MICREL_KSZ9021_EXTREG_CTRL, 0x8000|reg);

	if (0 != ret) {
		printf("eth_emdio_read write0 failed %d\n", ret);
		return ret;
	}
	ret = (*mii_write)(dev, addr, MICREL_KSZ9021_EXTREG_DATA_WRITE, val);
	if (0 != ret) {
		printf("eth_emdio_read write1 failed %d\n", ret);
		return ret;
	}

	return 0;
}

static int eth_set_ksz9021_skew(struct eth_device *dev, int phy_addr,
		int (*mii_write)(struct eth_device *, u8, u8, u16))
{
	/*  RXC PAD Skew andTXC PAD Skew */
	if (eth_emdio_write(dev, phy_addr,
		MII_KSZ9021_EXT_RGMII_CLOCK_SKEW,
		getenv_ulong(CONFIG_KSZ9021_CLK_SKEW_ENV, 16,
			CONFIG_KSZ9021_CLK_SKEW_VAL),
		mii_write) < 0)
		return -1;

	/* set no PAD skew for data */
	if (eth_emdio_write(dev, phy_addr,
		MII_KSZ9021_EXT_RGMII_RX_DATA_SKEW,
		getenv_ulong(CONFIG_KSZ9021_DATA_SKEW_ENV, 16,
			CONFIG_KSZ9021_DATA_SKEW_VAL),
		mii_write) < 0)
		return -1;

	return 0;
}

/*
 * DesignWare Ethernet initialization
 * This function overrides the __weak  version in the driver proper.
 * Our Micrel Phy needs slightly non-conventional setup
 */
int designware_board_phy_init(struct eth_device *dev, int phy_addr,
		int (*mii_write)(struct eth_device *, u8, u8, u16),
		int (*dw_reset_phy)(struct eth_device *))
{
	struct dw_eth_dev *priv = dev->priv;
	struct phy_device *phydev;
	struct mii_dev *bus;

	if ((*dw_reset_phy)(dev) < 0)
		return -1;

	bus = mdio_get_current_dev();
	phydev = phy_connect(bus, phy_addr, dev,
		priv->interface);

	/* Micrel PHY is connected to EMAC1 */
	if (strcasecmp(phydev->drv->name, "Micrel ksz9021") == 0 &&
		((phydev->drv->uid & phydev->drv->mask) ==
		(phydev->phy_id & phydev->drv->mask))) {

		printf("Configuring PHY skew timing for %s\n",
			phydev->drv->name);

		/* min rx data delay */
		if (ksz9021_phy_extended_write(phydev,
			MII_KSZ9021_EXT_RGMII_RX_DATA_SKEW,
			getenv_ulong(CONFIG_KSZ9021_DATA_SKEW_ENV, 16,
				CONFIG_KSZ9021_DATA_SKEW_VAL)) < 0)
			return -1;
		/* min tx data delay */
		if (ksz9021_phy_extended_write(phydev,
			MII_KSZ9021_EXT_RGMII_TX_DATA_SKEW,
			getenv_ulong(CONFIG_KSZ9021_DATA_SKEW_ENV, 16,
				CONFIG_KSZ9021_DATA_SKEW_VAL)) < 0)
			return -1;
		/* max rx/tx clock delay, min rx/tx control */
		if (ksz9021_phy_extended_write(phydev,
			MII_KSZ9021_EXT_RGMII_CLOCK_SKEW,
			getenv_ulong(CONFIG_KSZ9021_CLK_SKEW_ENV, 16,
				CONFIG_KSZ9021_CLK_SKEW_VAL)) < 0)
			return -1;

		if (phydev->drv->config)
			phydev->drv->config(phydev);
	}
	return 0;
}

/* We know all the init functions have been run now */
int board_eth_init(bd_t *bis)
{
#if !defined(CONFIG_SOCFPGA_VIRTUAL_TARGET) && \
!defined(CONFIG_SPL_BUILD)

	/* Initialize EMAC */

	/*
	 * Putting the EMAC controller to reset when configuring the PHY
	 * interface select at System Manager
	*/
	emac0_reset_enable(1);
	emac1_reset_enable(1);

	/* Clearing emac0 PHY interface select to 0 */
	clrbits_le32(CONFIG_SYSMGR_EMAC_CTRL,
		(SYSMGR_EMACGRP_CTRL_PHYSEL_MASK <<
#if (CONFIG_EMAC_BASE == CONFIG_EMAC0_BASE)
		SYSMGR_EMACGRP_CTRL_PHYSEL0_LSB));
#elif (CONFIG_EMAC_BASE == CONFIG_EMAC1_BASE)
		SYSMGR_EMACGRP_CTRL_PHYSEL1_LSB));
#endif

	/* configure to PHY interface select choosed */
	setbits_le32(CONFIG_SYSMGR_EMAC_CTRL,
#if (CONFIG_PHY_INTERFACE_MODE == SOCFPGA_PHYSEL_ENUM_GMII)
		(SYSMGR_EMACGRP_CTRL_PHYSEL_ENUM_GMII_MII <<
#elif (CONFIG_PHY_INTERFACE_MODE == SOCFPGA_PHYSEL_ENUM_MII)
		(SYSMGR_EMACGRP_CTRL_PHYSEL_ENUM_GMII_MII <<
#elif (CONFIG_PHY_INTERFACE_MODE == SOCFPGA_PHYSEL_ENUM_RGMII)
		(SYSMGR_EMACGRP_CTRL_PHYSEL_ENUM_RGMII <<
#elif (CONFIG_PHY_INTERFACE_MODE == SOCFPGA_PHYSEL_ENUM_RMII)
		(SYSMGR_EMACGRP_CTRL_PHYSEL_ENUM_RMII <<
#endif
#if (CONFIG_EMAC_BASE == CONFIG_EMAC0_BASE)
		SYSMGR_EMACGRP_CTRL_PHYSEL0_LSB));
	/* Release the EMAC controller from reset */
	emac0_reset_enable(0);
#elif (CONFIG_EMAC_BASE == CONFIG_EMAC1_BASE)
		SYSMGR_EMACGRP_CTRL_PHYSEL1_LSB));
	/* Release the EMAC controller from reset */
	emac1_reset_enable(0);
#endif

	/* initialize and register the emac */
	int rval = designware_initialize(0, CONFIG_EMAC_BASE,
		CONFIG_EPHY_PHY_ADDR,
#if (CONFIG_PHY_INTERFACE_MODE == SOCFPGA_PHYSEL_ENUM_GMII)
		PHY_INTERFACE_MODE_GMII);
#elif (CONFIG_PHY_INTERFACE_MODE == SOCFPGA_PHYSEL_ENUM_MII)
		PHY_INTERFACE_MODE_MII);
#elif (CONFIG_PHY_INTERFACE_MODE == SOCFPGA_PHYSEL_ENUM_RGMII)
		PHY_INTERFACE_MODE_RGMII);
#elif (CONFIG_PHY_INTERFACE_MODE == SOCFPGA_PHYSEL_ENUM_RMII)
		PHY_INTERFACE_MODE_RMII);
#endif
	debug("board_eth_init %d\n", rval);
	return rval;
#else
	return 0;
#endif
}

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
	return 0;
}
#endif

