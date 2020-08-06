/* Copyright (C) 2020 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */

// TWELITE hardware like
#include "mwx_stgs_standard_core.h"

/**********************************************************************************
 * SETTINGS TABLE
 **********************************************************************************/
extern const TWESTG_tsElement SET_STD_DEFSETS[SIZE_SET_STD_DEFSETS] __attribute__((weak));

/*!
 * default table for SET_STD 
 */
const TWESTG_tsElement SET_STD_DEFSETS[SIZE_SET_STD_DEFSETS] = {
	{ E_TWESTG_DEFSETS_APPID,  // アプリケーションID
		{ TWESTG_DATATYPE_UINT32, sizeof(uint32), 0, 0, {.u32 = 0x1234abcd }}, // 32bit (デフォルトのIDは配列決め打ちなので、ボード定義でオーバライドが必要)
		{ "AID", "Application ID [HEX:32bit]", "" },
		{ E_TWEINPUTSTRING_DATATYPE_HEX, 8, 'a' },
		{ {.u32 = 0}, {.u32 = 0}, TWESTGS_VLD_u32AppId, NULL },
	},
	{ E_TWESTG_DEFSETS_LOGICALID,
		{ TWESTG_DATATYPE_UINT8,  sizeof(uint8),  0, 0, {.u8 = 1 }}, 
		{ "LID", "Device ID [1-100,etc]", "" }, 
		{ E_TWEINPUTSTRING_DATATYPE_DEC, 3, 'i' },
		{ {.u32 = 0}, {.u32 = 100}, TWESTGS_VLD_u32MinMax, NULL },
	},
	{ E_TWESTG_DEFSETS_CHANNEL,
		{ TWESTG_DATATYPE_UINT8,  sizeof(uint8),  0, 0, {.u8 = 13 }},
		{ "CHN", "Channel [11-26]", "" },
		{ E_TWEINPUTSTRING_DATATYPE_DEC, 2, 'c' },
		{ {.u32 = 11}, {.u32 = 26}, TWESTGS_VLD_u32MinMax, NULL },
	},
	{ E_TWESTG_DEFSETS_CHANNELS_3,
		{ TWESTG_DATATYPE_UINT16, sizeof(uint16), 0, 0, {.u16 = ((1UL << 18) >> 11) }},
		{ "CHL", "Channels Set", 
		  "Input up to 3 channels like '11,15,24'."
		},
		{ E_TWEINPUTSTRING_DATATYPE_CUSTOM_DISP_MASK | E_TWEINPUTSTRING_DATATYPE_STRING, 8, 'c' },
		{ {.u16 = 0}, {.u16 = 0xFFFF}, TWESTGS_VLD_u32ChList, NULL },
	},
	{ E_TWESTG_DEFSETS_POWER_N_RETRY,
		{ TWESTG_DATATYPE_UINT8,  sizeof(uint8),  0, 0, {.u8 = 0x03 }},
		{ "PWR", "RF Power/Retry [HEX:8bit]",
			"YZ Y=Retry(0:default,F:0,1-9:count\r\n"
			"Z = Power(3:Max,2,1,0 : Min)" },
		{ E_TWEINPUTSTRING_DATATYPE_HEX, 2, 'x' },
		{ {.u32 = 0}, {.u32 = 0xFF}, TWESTGS_VLD_u32MinMax, NULL },
	},
	{ E_TWESTG_DEFSETS_OPTBITS, 
		{ TWESTG_DATATYPE_UINT32, sizeof(uint32), 0, 0, {.u32 = 0x00000000 }}, 
		{ "OP1", "Option1 [HEX:32bit]", "" },
		{ E_TWEINPUTSTRING_DATATYPE_HEX, 8, 'o' },
		{ {.u32 = 0}, {.u32 = 0xFFFFFFFF}, TWESTGS_VLD_u32MinMax, NULL },
	},
	{ E_TWESTG_DEFSETS_OPT_DWORD1, 
		{ TWESTG_DATATYPE_UINT32, sizeof(uint32), 0, 0, {.u32 = 0x00000000 }}, 
		{ "OP2", "Option2 [HEX:32bit]", "" },
		{ E_TWEINPUTSTRING_DATATYPE_HEX, 8, 'p' },
		{ {.u32 = 0}, {.u32 = 0xFFFFFFFF}, TWESTGS_VLD_u32MinMax, NULL },
	},
	{ E_TWESTG_DEFSETS_OPT_DWORD2, 
		{ TWESTG_DATATYPE_UINT32, sizeof(uint32), 0, 0, {.u32 = 0x00000000 }}, 
		{ "OP3", "Option3 [HEX:32bit]", "" },
		{ E_TWEINPUTSTRING_DATATYPE_HEX, 8, 'q' },
		{ {.u32 = 0}, {.u32 = 0xFFFFFFFF}, TWESTGS_VLD_u32MinMax, NULL },
	},
	{ E_TWESTG_DEFSETS_OPT_DWORD3, 
		{ TWESTG_DATATYPE_UINT32, sizeof(uint32), 0, 0, {.u32 = 0x00000000 }}, 
		{ "OP4", "Option4 [HEX:32bit]", "" },
		{ E_TWEINPUTSTRING_DATATYPE_HEX, 8, 'r' },
		{ {.u32 = 0}, {.u32 = 0xFFFFFFFF}, TWESTGS_VLD_u32MinMax, NULL },
	},
	{E_TWESTG_DEFSETS_VOID} // FINAL DATA
};