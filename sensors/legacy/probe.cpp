/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */

#include "../../_tweltite.hpp"
#include "../mwx_sns_legacy.hpp"

#include "SHT31.h"
#include "SHTC3.h"
#include "LTR308ALS.h"
#include "BME280.h"

namespace mwx {
	bool sns_probe_true(uint32_t u32opt) {
		return true;
	}

	bool sns_proble_sht3c(uint32_t u32opt) {
		return Wire.probe(SHTC3_ADDRESS);
	}

	bool sns_proble_sht3x(uint32_t u32opt) {
		uint8_t u8addr = u32opt & 0x000000FFUL;
		if (!u8addr) u8addr = DEFAULT_SHT31_ADDRESS;
		// Serial << "probe" << mwx::mwx_format("(%02X)", u8addr);
		return Wire.probe(u8addr);
	}

	bool sns_proble_ltr308als(uint32_t u32opt) {
		if (auto&& wrt = Wire.get_writer(LTR308ALS_ADDRESS)) {
			(void)wrt;
			// wrt << 0x00;
			// wrt << 0x00;
		} else 
			return false;
			
		return true;
	}

	bool sns_probe_bme280(uint32_t u32opt) {
		uint8_t u8addr = u32opt & 0x000000FFUL;
		if (!u8addr) u8addr = DEFAULT_BME280_ADDRESS;
		// Serial << "probe" << mwx::mwx_format("(%02X)", u8addr);
		return Wire.probe(u8addr);
	}
}
