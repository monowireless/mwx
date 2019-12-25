#include "mwx_periph_analogue.hpp"
#include "mwx_debug.h"

void mwx::periph_analogue::ADC_handler(uint32 u32Device, uint32 u32ItemBitmap) {
	// this handler is called when AD conversion is finished.
	if (u32ItemBitmap & E_AHI_AP_CAPT_INT_STATUS_MASK) {
		// MWX_DebugMsg(0, "A");
		Analogue._adc_capt();
	}
}
