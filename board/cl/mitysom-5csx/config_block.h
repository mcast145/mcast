/**
 * If you modify this file, you will need to copy this file over to the
 * kernel build area, or somehow make a link between the two...
 */
#ifndef CONFIG_BLOCK_H_
#define CONFIG_BLOCK_H_

#define CONFIG_I2C_MAGIC_WORD	0x012C0138
#define CONFIG_I2C_VERSION_1_1	0x00010001 /* prior to DDR3 configurations */
#define CONFIG_I2C_VERSION_1_2	0x00010002
#define CONFIG_I2C_VERSION_1_3	0x00010003 /* Add MACADDR2 */
#define CONFIG_I2C_VERSION	CONFIG_I2C_VERSION_1_3

/**
 *  Model numbering scheme:
 *  PPPP-YX-NAR-HC
 *
 *  PPPP	- Part number (3359, 3354, etc.)
 *  Y		- Speed Grade (E or G - 720 MHz)
 *  X		- not used (fpga type)
 *  N		- NOR size (3 - 16 MB, 2 - 8MB)
 *  A		- NAND size (2 - 256 MB, 3 - 512 MB)
 *  R		- RAM size (6 - 256 MB DDR2, 7 - 256 MB DDR3, 8 - 512 MB DDR3)
 *  H		- RoHS (R - compliant)
 *  C		- Temperature (C - commercial, I - industrial, L - Low Temp)
 */  

#define RAM_SIZE_POSITION	10
#define RAM_SIZE_256MB_DDR2	'6'
#define RAM_SIZE_512GB_DDR2	'9' /* Not Used */
#define RAM_SIZE_256MB_DDR3	'7'
#define RAM_SIZE_512MB_DDR3	'8'
#define RAM_SIZE_1GB_DDR3	'A'

struct I2CFactoryConfigV2 {
	u32               ConfigMagicWord;  /** CONFIG_I2C_MAGIC_WORD */
	u32               ConfigVersion;    /** CONFIG_I2C_VERSION */
        u8                MACADDR[6];       /** Set to 0.. MAC is in eFuse on AM-335x */
	u32               FpgaType;         /** fpga installed, see above */
	u32               Spare;            /** Not Used */
	u32               SerialNumber;     /** serial number assigned to part */
	char              ModelNumber[32];  /** board model number, human readable text, NULL terminated */
};
struct I2CFactoryConfig {
	u32               ConfigMagicWord;  /** CONFIG_I2C_MAGIC_WORD */
	u32               ConfigVersion;    /** CONFIG_I2C_VERSION */
        u8                MACADDR[6];       /** Set to 0.. MAC is in eFuse on AM-335x */
	u32               FpgaType;         /** fpga installed, see above */
	u32               Spare;            /** Not Used */
	u32               SerialNumber;     /** serial number assigned to part */
	char              ModelNumber[32];  /** board model number, human readable text, NULL terminated */
        u8                MACADDR2[6];      /** Set to 0.. MAC is in eFuse on AM-335x */
};

extern struct I2CFactoryConfig  factory_config_block;
extern int get_factory_config_block(void);

#endif
