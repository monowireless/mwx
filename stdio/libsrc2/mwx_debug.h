/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */
#pragma once

#include "twecommon.h"
#if !defined(TWE_STDINOUT_ONLY)
#include "tweserial.h"
#include "tweserial_jen.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif // C++


#define MWX_DEBUGOUT
#ifdef MWX_DEBUGOUT
void MWX_DebugMsg_SetLevel(uint8_t lv_min, uint8_t lv_max);
void MWX_DebugMsg(int lv, const char* fmt, ...);
#else
#define MWX_DebugMsg_SetLevel(lv)
#define MWX_DebugMsg(lv, fmt, __VA_ARGS__);
#endif

void MWX_Panic(uint32_t u32val, const char* ftm, ...);

#define CHECK_MEMORY
#ifdef CHECK_MEMORY
#define _CHKMEM(a,v,msg) { uint32_t *p = (uint32_t*)(void*)a; if(*p != v) MWX_DebugMsg(0, "{%s:%X}", msg, *p); else MWX_DebugMsg(0, "*"); }
#else
#define _CHKMEM(a,v,msg)
#endif

#ifdef __cplusplus
}
#endif // C++
