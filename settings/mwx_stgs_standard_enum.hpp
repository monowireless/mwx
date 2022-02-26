#pragma once

/* Copyright (C) 2020-2021 Mono Wireless Inc. All Rights Reserved. *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE     *
 * AGREEMENT).                                                     */

#include "twecommon.h"
#include "twesettings_std_defsets.h"

/**
 * Enumerate configuration names
 */
enum class E_STGSTD_SETID {
	APPID = E_TWESTG_DEFSETS_APPID,
	LOGICALID = E_TWESTG_DEFSETS_LOGICALID,
	CHANNEL = E_TWESTG_DEFSETS_CHANNEL,
	CHANNELS_3 = E_TWESTG_DEFSETS_CHANNELS_3,
	POWER_N_RETRY = E_TWESTG_DEFSETS_POWER_N_RETRY,
	OPTBITS = E_TWESTG_DEFSETS_OPTBITS,
	UARTBAUD = E_TWESTG_DEFSETS_UARTBAUD,

	OPT_DWORD2 = E_TWESTG_DEFSETS_OPT_DWORD1,
	OPT_DWORD3 = E_TWESTG_DEFSETS_OPT_DWORD2,
	OPT_DWORD4 = E_TWESTG_DEFSETS_OPT_DWORD3,

    ENC_MODE = 0x20,
    ENC_KEY_STRING,

	VOID = 0xFF,
};
