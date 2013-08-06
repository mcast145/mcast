/*
 * Control 
 *
 * Copyright (C) 2013 Critical Link LLC
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <command.h>
#include <asm/gpio.h>

int gpio_request(unsigned gpio, const char *label)
{
	if (gpio >= 3*GPIO_WIDTH) {
		printf("%s: Invalid GPIO requested %d\n", __func__, gpio);
		return -1;
	}
	return 0;
}

int gpio_free(unsigned gpio)
{
	return 0;
}

#define GET_BANK_MASK(gpio) \
\
	if (gpio >= 3*GPIO_WIDTH) { \
		printf("%s: Invalid GPIO %d\n", __func__, gpio); \
		return -1; \
	} \
\
	bank = gpio / GPIO_WIDTH; \
	mask = 1<<(gpio-(bank*GPIO_WIDTH)); \
\
	switch(bank) { \
		case 0: \
			base_addr = (unsigned int*)GPIO_MODULE0_ADDR; \
			break; \
		case 1: \
			base_addr = (unsigned int*)GPIO_MODULE1_ADDR; \
			break; \
		case 2: \
			base_addr = (unsigned int*)GPIO_MODULE2_ADDR; \
			break; \
		default: \
			return -1; \
	} \
	printf("gpio: pin %d (bank/mask = %d/0x%08X)\n", gpio, bank, mask);

int gpio_direction_input(unsigned gpio)
{
	unsigned int bank = 0;
	unsigned int mask = 0;
	unsigned int* base_addr;

	GET_BANK_MASK(gpio);

	base_addr[GPIO_SWPORTA_DDR_OFFSET] &= ~mask;
	return 0;
}

int gpio_direction_output(unsigned gpio, int value)
{
	unsigned int bank = 0;
	unsigned int mask = 0;
	unsigned int* base_addr;

	GET_BANK_MASK(gpio)

	if (value)
		base_addr[GPIO_SWPORTA_DR_OFFSET] |= mask;
	else
		base_addr[GPIO_SWPORTA_DR_OFFSET] &= ~mask;
	base_addr[GPIO_SWPORTA_DDR_OFFSET] |= mask;
	return 0;
}

int gpio_get_value(unsigned gpio)
{
	unsigned int bank = 0;
	unsigned int mask = 0;
	unsigned int* base_addr;

	GET_BANK_MASK(gpio)

	return (base_addr[GPIO_SWPORTA_DR_OFFSET] & mask) ? 1 : 0;
}

int gpio_set_value(unsigned gpio, int value)
{
	unsigned int bank = 0;
	unsigned int mask = 0;
	unsigned int* base_addr;

	GET_BANK_MASK(gpio)

	if (value)
		base_addr[GPIO_SWPORTA_DR_OFFSET] |= mask;
	else
		base_addr[GPIO_SWPORTA_DR_OFFSET] &= ~mask;

	return 0;
}
