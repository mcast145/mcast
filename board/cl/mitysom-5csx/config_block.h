#ifndef CONFIG_BLOCK_H_
#define CONFIG_BLOCK_H_

#define CONFIG_I2C_MAGIC_WORD	0x012C0138
#define CONFIG_I2C_VERSION_1_1	0x00010001 /* Never released */
#define CONFIG_I2C_VERSION_1_2	0x00010002 /* Base config */
#define CONFIG_I2C_VERSION_1_3	0x00010003 /* Base config with MACADDR2 */
#define CONFIG_I2C_VERSION	CONFIG_I2C_VERSION_1_3

struct I2CFactoryConfigV2 {
	u32               ConfigMagicWord;  /** CONFIG_I2C_MAGIC_WORD */
	u32               ConfigVersion;    /** CONFIG_I2C_VERSION */
	u8                MACADDR[6];       /** MAC Address assigned to part */
	u32               FpgaType;         /** Not Used */
	u32               Spare;            /** Not Used */
	u32               SerialNumber;     /** serial number assigned to part */
	char              ModelNumber[32];  /** board model number, human readable text, NULL terminated */
};
struct I2CFactoryConfig {
	u32               ConfigMagicWord;  /** CONFIG_I2C_MAGIC_WORD */
	u32               ConfigVersion;    /** CONFIG_I2C_VERSION */
	u8                MACADDR[6];       /** MAC Address assigned to part */
	u32               FpgaType;         /** Not Used */
	u32               Spare;            /** Not Used */
	u32               SerialNumber;     /** serial number assigned to part */
	char              ModelNumber[32];  /** board model number, human readable text, NULL terminated */
	u8                MACADDR2[6];      /** 2nd MAC Address assigned to part */
};

extern struct I2CFactoryConfig  factory_config_block;
extern int get_factory_config_block(void);

#endif
