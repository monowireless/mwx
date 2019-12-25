/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */
#pragma once

#include <cstdint>
#include "mwx_boards_utils.hpp"

namespace mwx { inline namespace L1 {
	namespace BOARD {
		static const uint8_t NONE = 0;
		static const uint8_t MONOSTICK = 1;
		static const uint8_t PAL_MAG = 2;
		static const uint8_t PAL_AMB = 3;
		static const uint8_t PAL_MOT = 4;
		static const uint8_t PAL_LED = 5;
		static const uint8_t BRD_APPTWELITE = 0x11;
	};
}}
