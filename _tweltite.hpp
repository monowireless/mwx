/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */
#pragma once

// VERY BASIC DEFS
#include <jendefs.h>
#include <AppHardwareApi.h>

// TWENET C Library
#include "ToCoNet.h"

// TWENET settings C Library
#include "twecommon.h"
#include "tweserial.h"
#include "tweserial_jen.h"
#include "tweprintf.h"
#include "twesercmd_gen.h"
#include "tweinteractive.h"
#include "twesysutils.h"

#include "tweinteractive_defmenus.h"
#include "tweinteractive_settings.h"
#include "tweinteractive_nvmutils.h"

// TWE C++ basic library
#include "mwx_common.hpp"

#include "mwx_utils.hpp"
#include "mwx_utils_alloc.hpp"
#include "mwx_utils_smplque.hpp"
#include "mwx_utils_crc8.hpp"
#include "mwx_utils_payload.hpp"
#include "mwx_utils_enum.hpp"
#include "mwx_utils_axis.hpp"

#include "mwx_duplicate_checker.hpp"

// mwx C++ library (RF handling)
#include "networks/mwx_networks.hpp"

// mwx C++ library (core handler classes)
#include "mwx_processev.hpp"
#include "mwx_appdefs.hpp"

// mwx C++ library (general classes)
#include "mwx_hardware.hpp"
#include "mwx_debug.h"
#include "mwx_memory.hpp"
#include "mwx_stream.hpp"
#include "mwx_utils_smplbuf.hpp"

// mwx C++ library (peripherals)
#include "mwx_serial_jen.hpp"

#include "mwx_periph.hpp"
#include "mwx_periph_dio.hpp"
#include "mwx_periph_timer.hpp"
#include "mwx_periph_analogue.hpp"
#include "mwx_periph_wire.hpp"
#include "mwx_periph_spi.hpp"
#include "mwx_periph_pulse_counter.hpp"

// mwx C++ library (twesettings wrapper)
#include "settings/mwx_settings.hpp"
#include "settings/mwx_stgs_standard.hpp"

// mwx C++ parsing library
#include "mwx_parser.hpp"

// mwx C++ global defined instances (Serial, Timer?, ...)
#include "mwx_common_instances.hpp"
