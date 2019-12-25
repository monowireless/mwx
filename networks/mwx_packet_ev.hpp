/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */
#pragma once
#include "ToCoNet.h"
#include "../mwx_debug.h"

namespace mwx { inline namespace L1 {
    struct packet_ev_tx {
		uint8_t _network_type;
		uint8_t _network_handled;

        uint8_t u8CbId;
        uint8_t bStatus;
    };

    struct packet_ev_nwk {
		uint8_t _network_type;
		uint8_t _network_handled;

        uint32_t eEvent;
        uint32_t u32arg;
    };
}}
