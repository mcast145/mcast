#include "sequencer_defines.h"
#include "alt_types.h"
#if HCX_COMPAT_MODE || ENABLE_INST_ROM_WRITE
const alt_u32 ac_rom_init_size = 36;
const alt_u32 ac_rom_init[36] = {
	0x20700000,
	0x20780000,
	0x10080221,
	0x10080320,
	0x10090044,
	0x100a0000,
	0x100b0000,
	0x10380400,
	0x10080241,
	0x100802c0,
	0x100a0024,
	0x10090000,
	0x100b0000,
	0x30780000,
	0x38780000,
	0x30780000,
	0x10680000,
	0x106b0000,
	0x10280400,
	0x10480000,
	0x1c980000,
	0x1c9b0000,
	0x1c980008,
	0x1c9b0008,
	0x38f80000,
	0x3cf80000,
	0x38780000,
	0x18180000,
	0x18980000,
	0x13580000,
	0x135b0000,
	0x13580008,
	0x135b0008,
	0x33780000,
	0x10580008,
	0x10780000
};
#endif