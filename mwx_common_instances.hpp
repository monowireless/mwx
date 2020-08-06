/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */
#pragma once

#include "mwx_common.hpp"
#include "mwx_appdefs.hpp"
#include "mwx_appcore.hpp"
#include "mwx_serial_jen.hpp"
#include "mwx_debug.h"
#include "mwx_periph.hpp"
#include "mwx_periph_wire.hpp"
#include "mwx_periph_spi.hpp"
#include "mwx_periph_timer.hpp"
#include "mwx_periph_buttons.hpp"
#include "mwx_periph_analogue.hpp"
#include "mwx_periph_pulse_counter.hpp"
#include "networks/mwx_networks.hpp"
#include "mwx_parser.hpp"
#include "mwx_utils_alloc.hpp"


extern mwx::serial_jen Serial, Serial1;
extern mwx::serial_parser<mwx::alloc_heap<uint8_t>> SerialParser;

extern mwx::periph_twowire<MWX_TWOWIRE_BUFF> Wire;
extern mwx::periph_spi SPI;
extern mwx::periph_buttons Buttons;
extern mwx::periph_analogue Analogue;
extern mwx::periph_pulse_counter PulseCounter0;
extern mwx::periph_pulse_counter PulseCounter1;
extern mwx::periph_pulse_counter& PulseCounter;

extern mwx::periph_ticktimer TickTimer;
extern mwx::periph_timer Timer0;
extern mwx::periph_timer Timer1;
extern mwx::periph_timer Timer2;
extern mwx::periph_timer Timer3;
extern mwx::periph_timer Timer4;

extern mwx::appdefs_virt the_vapp;
extern mwx::appdefs_virt the_vhw;
extern mwx::appdefs_virt the_vnet;
extern mwx::appdefs_virt the_vsettings;

extern mwx::twenet_mac the_mac;
