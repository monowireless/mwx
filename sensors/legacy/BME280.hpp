/* Copyright (C) 2020 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */

#pragma once

// the instances
#include "../mwx_sns_legacy.hpp"
#include "BME280.h"

namespace mwx {
	// SHTC3 (Temp/Humid)
	bool sns_probe_bme280(uint32_t);
	using _SNS_BME280 = sns_legacy<tsObjData_BME280, vBME280_Init, vBME280_Final, sns_probe_bme280>;
	class SnsBME280 : public _SNS_BME280 {
	public:
		double get_temp() {
			double d = _SNS_BME280::_get_data()->i16Temp;
			return d / 100.0;
		}
		int16_t get_temp_cent() {
			return _SNS_BME280::_get_data()->i16Temp;
		}
		double get_humid() {
			double d = _SNS_BME280::_get_data()->u16Hum;
			return d / 100.0;
		}
		int16_t get_humid_per_dmil() {
			return _SNS_BME280::_get_data()->u16Hum;
		}
        int16_t get_press() {
            return _SNS_BME280::_get_data()->u16Pres;
        }
	};
}
