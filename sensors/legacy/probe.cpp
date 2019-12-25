/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */

#include "../../_tweltite.hpp"
#include "../mwx_sns_legacy.hpp"
#include "SHTC3.h"
#include "LTR308ALS.h"

namespace mwx {
	bool sns_probe_true() {
		return true;
	}

	bool sns_proble_sht3c() {
		return Wire.probe(SHTC3_ADDRESS);
	}

	bool sns_proble_ltr308als() {
		if (auto&& wrt = Wire.get_writer(LTR308ALS_ADDRESS)) {
			// wrt << 0x00;
			// wrt << 0x00;
		} else 
			return false;
			
		return true;
	}
}
