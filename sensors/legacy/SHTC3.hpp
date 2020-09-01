/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */

#pragma once

// the instances
#include "../mwx_sns_legacy.hpp"
#include "SHTC3.h"

namespace mwx {
	// SHTC3 (Temp/Humid)
	bool sns_proble_sht3c(uint32_t);
	
	using _SNS_SHTC3 = sns_legacy<tsObjData_SHTC3, vSHTC3_Init, vSHTC3_Final, sns_proble_sht3c>;
	class SnsSHTC3 : public _SNS_SHTC3 {
	public:
		double get_temp() {
			double d = _SNS_SHTC3::_get_data()->ai16Result[0];
			return d / 100.0;
		}
		int16_t get_temp_cent() {
			return _SNS_SHTC3::_get_data()->ai16Result[0];
		}
		double get_humid() {
			double d = _SNS_SHTC3::_get_data()->ai16Result[1];
			return d / 100.0;
		}
		int16_t get_humid_per_dmil() {
			return _SNS_SHTC3::_get_data()->ai16Result[1];
		}
	};
}
