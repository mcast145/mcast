#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;
typedef unsigned char uchar;

#include "board/cl/mitysom-5csx/config_block.h"
#define FACTORYEEPROM_NAME	"/sys/bus/i2c/devices/i2c-0/0-0050/eeprom"

void usage(void)
{
	fprintf(stdout,
	"Usage : \n"
	"factoryconfig [-s] [-n] [-v] [-m index] [-h]\n"
	"Options:\n"
	"-s : print serial number\n"
	"-n : print model number\n"
	"-v : print configuration version number\n"
	"-m index : print MAC address, index should be 1\n"
	"-h : this help data\n"
	"\n"
	"No arguments will dump all parameters\n");
}

int main(int argc, char* argv[])
{

	int opt, index = 0;

	struct I2CFactoryConfig config;
	FILE* fp = fopen(FACTORYEEPROM_NAME, "rb");
	if (!fp)
	{
		fprintf(stderr, "Uneable to open eeprom file %s\n", FACTORYEEPROM_NAME);
		return -1;
	}

	int rv = fread(&config, sizeof(config), 1, fp);
	if (1 != rv)
	{
		fprintf(stderr, "Unable to read from eeprom\n");
		fclose(fp);
		return -1;
	}
	fclose(fp);

	int need_tab = 0;
	while ((opt = getopt(argc, argv, "snvm:h")) != -1)
	{
		switch(opt)
		{
		case 's':
			fprintf(stdout, "%s%d", need_tab ? "\t" : "", config.SerialNumber);
			need_tab = 1;
			break;

		case 'n':
			fprintf(stdout, "%s%s", need_tab ? "\t" : "", config.ModelNumber);
			need_tab = 1;
			break;

		case 'v':
			fprintf(stdout, "%s%d.%02d", need_tab ? "\t" : "", config.ConfigVersion>>16,
				config.ConfigVersion&0xFFFF);
			need_tab = 1;
			break;

		case 'm':
			index = atoi(optarg);
			if (1 == index)
			{
				fprintf(stdout, "%s%02X:%02X:%02X:%02X:%02X:%02X", need_tab ? "\t" : "",
					config.MACADDR[0], config.MACADDR[1], config.MACADDR[2],
					config.MACADDR[3], config.MACADDR[4], config.MACADDR[5]);
				need_tab = 1;
			}
			else
			{
				fprintf(stderr, "Only MAC index 1 supported\n");
			}
			break;

		case 'h':
		default:
			usage();
			return 0;
			break;
		}
	}

	if (!need_tab)
	{
		fprintf(stdout, "Serial Number  : %d\n", config.SerialNumber);
		fprintf(stdout, "Model Number   : %s\n", config.ModelNumber);
		fprintf(stdout, "Config Version : %d.%02d\n", config.ConfigVersion>>16,
				config.ConfigVersion&0xFFFF);
		fprintf(stdout, "MAC Address    : %02X:%02X:%02X:%02X:%02X:%02X\n",
			config.MACADDR[0], config.MACADDR[1], config.MACADDR[2],
			config.MACADDR[3], config.MACADDR[4], config.MACADDR[5]);
	}
	else
	{
		fprintf(stdout, "\n");
	}

	return 0;
}
