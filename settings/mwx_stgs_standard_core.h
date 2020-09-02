#pragma once

/* Copyright (C) 2020 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */

#include "twecommon.h"
#include "twesettings.h"
#include "twesettings_std.h"
#include "twesettings_std_defsets.h"
#include "twesettings_cmd.h"
#include "twesettings_validator.h"

#include "tweinteractive.h"

#include "mwx_debug.h"

/**
 * the app name
 */
extern const char *INTRCT_USER_APP_NAME;

/**
 * the default app ID
 */
extern uint32_t INTRCT_USER_APP_ID;

#ifndef SIZE_SETSTD_CUST_COMMON // if you want to add more custom settings, increase.
# define SIZE_SETSTD_CUST_COMMON 24
#endif
/**
 * custom config data array. (e.g. remove items, change default value)
 */
extern uint8_t SETSTD_CUST_COMMON[SIZE_SETSTD_CUST_COMMON];

#ifndef SIZE_SET_STD_DEFSETS // if you want to put bigger settings table, increase.
# define SIZE_SET_STD_DEFSETS 16
#endif
/**
 * the setting master table
 */
extern const TWESTG_tsElement SET_STD_DEFSETS[SIZE_SET_STD_DEFSETS];

// 
enum class E_STGSTD_SETID {
	APPID = E_TWESTG_DEFSETS_APPID,
	LOGICALID = E_TWESTG_DEFSETS_LOGICALID,
	CHANNEL = E_TWESTG_DEFSETS_CHANNEL,
	CHANNELS_3 = E_TWESTG_DEFSETS_CHANNELS_3,
	POWER_N_RETRY = E_TWESTG_DEFSETS_POWER_N_RETRY,
	OPTBITS = E_TWESTG_DEFSETS_OPTBITS,
	UARTBAUD = E_TWESTG_DEFSETS_UARTBAUD,
	
	OPT_DWORD1 = E_TWESTG_DEFSETS_OPTBITS,
	OPT_DWORD2 = E_TWESTG_DEFSETS_OPT_DWORD1,
	OPT_DWORD3 = E_TWESTG_DEFSETS_OPT_DWORD2,
	OPT_DWORD4 = E_TWESTG_DEFSETS_OPT_DWORD3,

    ENC_MODE = 0x20,
    ENC_KEY_STRING,

	VOID = 0xFF,
};