#pragma once

// the instances
#include "../mwx_sns_legacy.hpp"
#include "LTR308ALS.h"

namespace mwx {
	// LTR308ALS (Light Sensor)
	bool sns_proble_ltr308als(uint32_t);
	using _SNS_LTR308ALS = sns_legacy<tsObjData_LTR308ALS, vLTR308ALS_Init, vLTR308ALS_Final, sns_proble_ltr308als>;
	class SnsLTR308ALS : public _SNS_LTR308ALS {
	public:
		uint32_t get_luminance() {
			return _SNS_LTR308ALS::_get_data()->u32Result;
		}
	};
}
