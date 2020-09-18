/* Copyright (C) 2020 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */

// mwx header
#include "mwx_brd_pal_notice.hpp"

/*****************************************************************/
namespace mwx {
// MUST DEFINE CLASS NAME HERE
#define __MWX_APP_CLASS_NAME BrdPalNotice
#include "../_mwx_cbs_cpphead.hpp"
/*****************************************************************/

	// INT handler: MOT SENSOR INT.
#if 0 // no need to add this hander explicitely,
	  // only if setting handled true.
	MWX_DIO_INT(BrdPal::PIN_SNS_INT, uint32_t arg, uint8_t& handled) {
		handled = true; // if true, no further event.
	}
#endif

	// for non-sleeping application, read MOT SENSOR FIFO on the
	// pin event.
	// normally, this should be done at DIO_EVENT to avoid
	// considering critical section while operating the queue.
	MWX_DIO_EVENT(BrdPal::PIN_SNS_INT, uint32_t arg) {
		sns_MC3630._read_result(); // read into queue and clear INT.
	}

	// set_led_brightness()
	void __MWX_APP_CLASS_NAME::set_leds_brightness1000(uint16_t r, uint16_t g, uint16_t b, uint16_t w, bool boost) {
		uint8_t reg_r, reg_g, reg_b, reg_w;
		if (boost) {
			reg_r = scale_1000_to_256u8(r);
			reg_g = scale_1000_to_256u8(g);
			reg_b = scale_1000_to_256u8(b);
			reg_w = scale_1000_to_256u8(w);
		} else {
			reg_r = scale_1000_to_127u8(r);
			reg_g = scale_1000_to_127u8(g);
			reg_b = scale_1000_to_127u8(b);
			reg_w = scale_1000_to_127u8(w);
		}
		sns_PCA9632.set_led_duty_all(reg_r, reg_g, reg_b, reg_w);
	} 

	// blink smaller
	void __MWX_APP_CLASS_NAME::test_led() {
		set_led_master_sw_on();
		set_leds_brightness_reg(LED_REG_MAX_PWM, LED_REG_MAX_PWM, LED_REG_MAX_PWM, LED_REG_MAX_PWM);
		set_leds(LED_ON, LED_ON, LED_ON, LED_ON);
		delay(4);
		set_leds(LED_OFF, LED_OFF, LED_OFF, LED_OFF);
		//set_led_master_sw_off();
	}

/*****************************************************************/
// common procedure (DO NOT REMOVE)
#include "../_mwx_cbs_cpptail.cpp"
// MUST UNDEF CLASS NAME HERE
#undef __MWX_APP_CLASS_NAME
} // mwx
/*****************************************************************/


