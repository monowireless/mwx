/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */
#pragma once

#ifndef MWX_UNUSE_USING_DEF
using namespace mwx::L1;
using format = mwx::mwx_format; // rename to format

using TWENET = mwx::L1::twenet;
using LED_TIMER = mwx::periph_led_timer;

using ANALOGUE = mwx::periph_analogue;
using PULSE_COUNTER = mwx::periph_pulse_counter;

using TwoWire = mwx::periph_twowire<MWX_TWOWIRE_BUFF>;

using std::make_pair;

// function aliases
static constexpr auto& mwx_snprintf = ::snprintf_;
static constexpr auto& mwx_printf = ::printf_;
#endif
