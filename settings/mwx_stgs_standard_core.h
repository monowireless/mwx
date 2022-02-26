#pragma once

/* Copyright (C) 2020-2021 Mono Wireless Inc. All Rights Reserved. *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE     *
 * AGREEMENT).                                                     */

#include "twecommon.h"
#include "twesettings.h"
#include "twesettings_std.h"
#include "twesettings_std_defsets.h"
#include "twesettings_cmd.h"
#include "twesettings_validator.h"

#include "tweinteractive.h"

#include "mwx_debug.h"

/***************************************************************
 * NOTE: C linkage
 **************************************************************/
#ifdef __cplusplus
extern "C" {
#endif // C++

/**
 * the application version
 */
extern uint32_t INTRCT_USER_APP_VER;

/**
 * User App's callback to set APP_ID.
 */
extern void MWX_Set_User_App_Ver();

/**
 * the app name
 */
extern const char *INTRCT_USER_APP_NAME;

/**
 * the default app ID
 */
extern uint32_t INTRCT_USER_APP_ID;

#ifndef SIZE_SETSTD_CUST_COMMON // if you want to add more custom settings, increase.
# define SIZE_SETSTD_CUST_COMMON 32
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

#ifdef __cplusplus
}
#endif // C++
