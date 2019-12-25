/************************************
 * CCITT-8 CRC Function
 * Author: Rob Magee
 * Copyright 2007 CompuGlobalHyperMegaNet LLC
 * Use this code at your own risk.
 * CRC.cs
 * **********************************/

/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   *
 * 
 * Made modification as below:
 * - change names (vars/funcs)
 * - add TWE_XOR_u8Calc(), TWE_CRC8_u8CalcU32()
 */

#pragma once
#include "mwx_common.hpp"

namespace mwx { inline namespace L1 {
	uint8_t CRC8_u8Calc(uint8_t *pu8Data, uint8_t size, uint8_t initval = 0);
	uint8_t CRC8_u8CalcU32(uint32_t u32c, uint8_t initval = 0);
	uint8_t CRC8_u8CalcU16(uint16_t u16c, uint8_t initval = 0);
	uint8_t XOR_u8Calc(uint8_t *pu8Data, uint8_t size);
	uint8_t LRC_u8Calc(uint8_t* pu8Data, uint8_t size);
}}

