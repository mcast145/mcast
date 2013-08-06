/*
 * config_block.c
 *
 *  Created on: Mar 18, 2010
 *      Author: mikew
 */
#include <common.h>
#include <errno.h>
#include <malloc.h>
#include <command.h>
#include "config_block.h"
#include <asm/io.h>
#include <asm/errno.h>
#include <i2c.h>

static struct I2CFactoryConfig default_factory_config = {
		.ConfigMagicWord = CONFIG_I2C_MAGIC_WORD,
		.ConfigVersion = CONFIG_I2C_VERSION,
                .MACADDR = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
		.SerialNumber = 130001,
                .FpgaType = 0, /* Leave FpgaType in struct to maintain compatibilty across products */
                .ModelNumber = { '5','C','S','X','-','H','K','-','4','X','A','X','-','R','C', 0 }
};

struct I2CFactoryConfig __attribute__((section (".data"))) factory_config_block;

int put_factory_config_block(void)
{
	int i, ret;
	u16 sum = 0;
	unsigned char* tmp;
	unsigned int addr = 0x00;

	tmp = (unsigned char*)&factory_config_block;

	for (i = 0; i < sizeof(factory_config_block); i++)
	{
		sum += *tmp++;
	}

	tmp = (unsigned char*)&factory_config_block;
	ret = 0;
	for (i = 0; i < sizeof(factory_config_block); i++)
	{
		ret |= i2c_write(0x50, addr++, 1, tmp++, 1);
		udelay(11000);
	}

	tmp = (unsigned char*)&sum;
	for (i = 0; i < 2; i++)
	{
		ret |= i2c_write(0x50, addr++, 1, tmp++, 1);
		udelay(11000);
	}

	if (ret) {
		puts("Error Writing Factory Configuration Block\n");
	}
	else {
		puts("Factory Configuration Block Saved\n");
	}
	return ret;
}

/**
 *  return value: -1 i2c access error, 1 bad factory configuratio, 0 = OK
 */
int get_factory_config_block(void)
{
	unsigned char* tmp;
	int i;
	u16 sum, CheckSum;

	i = i2c_read(0x50, 0x00, 1, (unsigned char*)&factory_config_block, sizeof(factory_config_block));
	if (0 != i)
	{
		printf("ERROR %d reading Factory Configuration Block\n", i);
		return -1;
	}

	i = i2c_read(0x50, sizeof(factory_config_block), 1, (unsigned char*)&CheckSum, 2);
	if (0 != i)
	{
		printf("ERROR %d reading Factory Configuration checksum\n", i);
		return -1;
	}

	/* verify the configuration block is sane */
	tmp = (unsigned char*)&factory_config_block;
	sum = 0;
	for (i = 0; i < sizeof(factory_config_block); i++)
	{
		sum += *tmp++;
	}

	if (sum != CheckSum)
	{
		puts("Error - Factory Configuration Invalid\n");
		puts("You must set the factory configuration to make permanent\n");
		memcpy(&factory_config_block, &default_factory_config, sizeof(factory_config_block));
		return 1;
	}
	return 0;
}

/* This is a trivial atoi implementation since we don't have one available */
int atoi(char *string)
{
        int length;
        int retval = 0;
        int i;
        int sign = 1;

        length = strlen(string);
        for (i = 0; i < length; i++) {
                if (0 == i && string[0] == '-') {
                        sign = -1;
                        continue;
                }
                if (string[i] > '9' || string[i] < '0') {
                        break;
                }
                retval *= 10;
                retval += string[i] - '0';
        }
        retval *= sign;
        return retval;
}

void get_board_serial(struct tag_serialnr *sn)
{
	sn->low = factory_config_block.SerialNumber;
	sn->high = 0;
}

/**
 * Set the factory config block mac address from an ascii string
 * of the form xx:xx:xx:xx:xx:xx
 */
static void set_mac_from_string(char *buffer)
{
	char *p = buffer;
	int i=0;
	for (i = 0; i < 6; i++) {
		int j = 0;
		while(p[j] && (p[j] != ':')) j++;
		if (1) {
			unsigned int t;
			char temp = p[j];
			p[j] = 0;
			t = simple_strtoul(p, NULL, 16);
			p[j] = temp;
			factory_config_block.MACADDR[i] = t&0xFF;
		}
		p = &p[j];
		if (*p) p++;
	}
}
/* Only build u-boot commands for u-boot proper, not the SPL */
#ifndef CONFIG_SPL_BUILD

static int do_factoryconfig (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	int ret = CMD_RET_FAILURE;
	char buffer[80];
	char *strcopy = NULL;

	if (argc == 1) {
		/* List configuration info */
			puts ("Factory Configuration:\n");
			printf("Config Version : %d.%d\n",
				factory_config_block.ConfigVersion>>16,
				factory_config_block.ConfigVersion&0xFFFF);
                        printf("MAC Address    : %02X:%02X:%02X:%02X:%02X:%02X\n",
                                        factory_config_block.MACADDR[0],
                                        factory_config_block.MACADDR[1],
                                        factory_config_block.MACADDR[2],
                                        factory_config_block.MACADDR[3],
                                        factory_config_block.MACADDR[4],
                                        factory_config_block.MACADDR[5]);

			printf("Serial Number  : %d\n",
				factory_config_block.SerialNumber);
			printf("Model Number    : %s\n",
				factory_config_block.ModelNumber);
	} else {
		unsigned int i;
		if (0 == strncmp(argv[1],"set",3)) {
                        sprintf(buffer, "%02X:%02X:%02X:%02X:%02X:%02X",
                                        factory_config_block.MACADDR[0],
                                        factory_config_block.MACADDR[1],
                                        factory_config_block.MACADDR[2],
                                        factory_config_block.MACADDR[3],
                                        factory_config_block.MACADDR[4],
                                        factory_config_block.MACADDR[5]);
                        readline_into_buffer ("MAC Address  : ", buffer, 0);
			set_mac_from_string(buffer);
			sprintf(buffer, "%d", factory_config_block.SerialNumber);
			readline_into_buffer ("Serial Number :", buffer, 0);
			i = atoi(buffer);
			if (i > 0) factory_config_block.SerialNumber = i;
			sprintf(buffer, "%s", factory_config_block.ModelNumber);
			readline_into_buffer ("Model Number   :", buffer, 0);
			memcpy(factory_config_block.ModelNumber, buffer,
				sizeof(factory_config_block.ModelNumber));
			factory_config_block.ModelNumber
				[sizeof(factory_config_block.ModelNumber)-1] = 0;
		} else if (0 == strncmp(argv[1],"fix",3)) {
			/* Fix is so you can spec the whole config in a line */
			char *s;
			char *v, *k;
			strcopy = strdup(buffer);
			if (strcopy == NULL) {
				printf("error %d in strdup\n", errno);
				goto done;
			}

			s = strcopy;
			while (s) {
				v = strsep(&s, ",");
				if (!v)
					break;
				k = strsep(&v, "=");
				if (!k)
					break;
				if  (strncmp(k, "mac",3) == 0) {
					set_mac_from_string(v);
				} else if  (strncmp(k, "sn",2) == 0) {
					i = atoi(v);
					if (i > 0) factory_config_block.SerialNumber = i;
				} else if  (strncmp(k, "mn",3) == 0) {
					strncpy(factory_config_block.ModelNumber, v,
							sizeof(factory_config_block.ModelNumber));
					factory_config_block.ModelNumber
						[sizeof(factory_config_block.ModelNumber)-1] = 0;
				} else {
					printf("Unknown key %s\n",k);
					goto done;
				}
			}

			put_factory_config_block();
			puts("Configuration Saved\n");

		} else if (0 == strncmp(argv[1],"save",4)) {
			put_factory_config_block();
			puts("Configuration Saved\n");
		}
		else {
			puts("Unknown Option\n");
			goto done;
		}
	}
	ret = CMD_RET_SUCCESS;
done:
	if(strcopy)
		free(strcopy);
	return ret;
}

U_BOOT_CMD(factoryconfig,	CONFIG_SYS_MAXARGS,	0,	do_factoryconfig,
        "mityarm-5csx factory config block operations",
	     "    - print current configuration\n"
	"factoryconfig set\n"
	"         - set new configuration (interactive)\n"
	"factoryconfig fix\n"
	"         - set new configuration and save with one line\n"
	"           mac=xx:xx:xx:xx:xx:xx,sn=nnnnn,mn=sss...sss\n"
	"factoryconfig save\n"
	"         - write new configuration to I2C FLASH\n"
);

#endif // CONFIG_SPL_BUILD
