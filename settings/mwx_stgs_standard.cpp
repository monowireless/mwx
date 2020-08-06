/* Copyright (C) 2020 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */

// mwx header
#include "mwx_stgs_standard.hpp"
#include "mwx_stgs_standard_core.h"

#define DEBUG_LVL 99

// default value of interactive mode (app specific.)
const char *INTRCT_USER_APP_NAME = "APP";
uint32_t INTRCT_USER_APP_ID = 0x1234abcd;

/*****************************************************************/
namespace mwx {
// MUST DEFINE CLASS NAME HERE
#define __MWX_APP_CLASS_NAME StgsStandard
#include "../_mwx_cbs_cpphead.hpp"
/*****************************************************************/

// if not in uart mode, push uart input byte into a queue
void StgsStandard::_vProcessInputByte(TWEINTRCT_tsContext *pContext, int16 u16chr) {
    StgsStandard* pthis = static_cast<StgsStandard*>(pContext->pvWrapObjCpp);

    if (pthis) {
        pthis->serial.push(uint8_t(u16chr));
    }
}

/*****************************************************************/
// common procedure (DO NOT REMOVE)
#include "../_mwx_cbs_cpptail.cpp"
// MUST UNDEF CLASS NAME HERE
#undef __MWX_APP_CLASS_NAME
} // mwx
/*****************************************************************/

