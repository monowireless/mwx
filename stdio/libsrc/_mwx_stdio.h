/* Copyright (C) 2020 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */

#include "mwx_common.hpp"

extern "C" volatile uint32_t u32TickCount_ms;

extern void loop();
extern void setup();
extern uint32_t millis();
extern void mwx_exit(int);