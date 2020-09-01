/* Copyright (C) 2020 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */

#pragma once

// the instances
#include "../mwx_sns_legacy.hpp"
#include "SHT31.h"

namespace mwx {
	// SHT3X (Temp/Humid)
	bool sns_proble_sht3x(uint32_t);
	using _SNS_SHT3X = sns_legacy<tsObjData_SHT31, vSHT31_Init, vSHT31_Final, sns_proble_sht3x>;
	class SnsSHT3X : public _SNS_SHT3X {
	public:
		double get_temp() {
			double d = _SNS_SHT3X::_get_data()->ai16Result[0];
			return d / 100.0;
		}
		int16_t get_temp_cent() {
			return _SNS_SHT3X::_get_data()->ai16Result[0];
		}
		double get_humid() {
			double d = _SNS_SHT3X::_get_data()->ai16Result[1];
			return d / 100.0;
		}
		int16_t get_humid_per_dmil() {
			return _SNS_SHT3X::_get_data()->ai16Result[1];
		}
	};
}
