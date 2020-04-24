/* Copyright (C) 2020 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */

#include "mwx_settings.hpp"

// TWELITE hardware like
#include <jendefs.h>
#include <ToCoNet.h>
#include <AppHardwareApi.h>

#include "tweinteractive.h"
#include "tweinteractive_defmenus.h"
#include "tweinteractive_settings.h"
#include "tweinteractive_nvmutils.h"
#include "twenvm.h"
#include "twesysutils.h"


/**********************************************************************************
 * CONSTANTS
 **********************************************************************************/
extern const char *INTRCT_USER_APP_NAME __attribute__((weak));
const char *INTRCT_USER_APP_NAME = "APP";

extern const uint32_t INTRCT_USER_APP_ID __attribute__((weak));
const uint32_t INTRCT_USER_APP_ID = 0x1234abcd;


/**********************************************************************************
 * TABLES
 **********************************************************************************/
/**
 * the default settings.
 * note: if -flto is given to linker, it may have warning below.
 *    e.g. xxx does not match original declaration [enabled by default]
 */
extern const TWESTG_tsElement INTRCT_USER_BASE_SETTINGS[] __attribute__((weak));
const TWESTG_tsElement INTRCT_USER_BASE_SETTINGS[] = {
	{ E_TWESTG_DEFSETS_APPID, 
		{ TWESTG_DATATYPE_UINT32, sizeof(uint32), 0, 0, {.u32 = INTRCT_USER_APP_ID }}, // note: cannot refer to INTRCT_USER_APP_ID defined externally.
		{ "AID", "Application ID [HEX:32bit]", "" },
		{ E_TWEINPUTSTRING_DATATYPE_HEX, 8, 'a' },
		{ {.u32 = 0}, {.u32 = 0}, TWESTGS_VLD_u32AppId, NULL },
	},
	{ E_TWESTG_DEFSETS_LOGICALID,
		{ TWESTG_DATATYPE_UINT8,  sizeof(uint8),  0, 0, {.u8 = 0xFE }}, 
		{ "LID", "Device ID [HEX:00-FE]", "" }, 
		{ E_TWEINPUTSTRING_DATATYPE_HEX, 3, 'i' },
		{ {.u32 = 0}, {.u32 = 0xFE}, TWESTGS_VLD_u32MinMax, NULL },
	},
	{ E_TWESTG_DEFSETS_CHANNEL,
		{ TWESTG_DATATYPE_UINT8,  sizeof(uint8),  0, 0, {.u8 = 13 }},
		{ "CHN", "Channel [11-26]", "" },
		{ E_TWEINPUTSTRING_DATATYPE_DEC, 2, 'c' },
		{ {.u32 = 11}, {.u32 = 26}, TWESTGS_VLD_u32MinMax, NULL },
	},
	{ E_TWESTG_DEFSETS_OPTBITS, 
		{ TWESTG_DATATYPE_UINT32, sizeof(uint32), 0, 0, {.u32 = 0x00000000 }}, 
		{ "OPT", "Option Bits [HEX:32bit]", "" },
		{ E_TWEINPUTSTRING_DATATYPE_HEX, 8, 'o' },
		{ {.u32 = 0}, {.u32 = 0xFFFFFFFF}, TWESTGS_VLD_u32MinMax, NULL },
	},
	{E_TWESTG_DEFSETS_VOID} // FINAL DATA
};

extern const TWESTG_tsElement INTRCT_USER_SLOT0_SETTINGS[] __attribute__((weak));
const TWESTG_tsElement INTRCT_USER_SLOT0_SETTINGS[] = { {E_TWESTG_DEFSETS_VOID} };

extern const TWESTG_tsElement INTRCT_USER_SLOT1_SETTINGS[] __attribute__((weak));
const TWESTG_tsElement INTRCT_USER_SLOT1_SETTINGS[] = { {E_TWESTG_DEFSETS_VOID} };

extern const TWESTG_tsElement INTRCT_USER_SLOT2_SETTINGS[] __attribute__((weak));
const TWESTG_tsElement INTRCT_USER_SLOT2_SETTINGS[] = { {E_TWESTG_DEFSETS_VOID} };

extern const TWESTG_tsElement INTRCT_USER_SLOT3_SETTINGS[] __attribute__((weak));
const TWESTG_tsElement INTRCT_USER_SLOT3_SETTINGS[] = { {E_TWESTG_DEFSETS_VOID} };

extern const TWESTG_tsElement INTRCT_USER_SLOT4_SETTINGS[] __attribute__((weak));
const TWESTG_tsElement INTRCT_USER_SLOT4_SETTINGS[] = { {E_TWESTG_DEFSETS_VOID} };

extern const TWESTG_tsElement INTRCT_USER_SLOT5_SETTINGS[] __attribute__((weak));
const TWESTG_tsElement INTRCT_USER_SLOT5_SETTINGS[] = { {E_TWESTG_DEFSETS_VOID} };

extern const TWESTG_tsElement INTRCT_USER_SLOT6_SETTINGS[] __attribute__((weak));
const TWESTG_tsElement INTRCT_USER_SLOT6_SETTINGS[] = { {E_TWESTG_DEFSETS_VOID} };

extern const TWESTG_tsElement INTRCT_USER_SLOT7_SETTINGS[] __attribute__((weak));
const TWESTG_tsElement INTRCT_USER_SLOT7_SETTINGS[] = { {E_TWESTG_DEFSETS_VOID} };