/* Copyright (C) 2020 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */
#pragma once

#include "../_tweltite.hpp"
#include "mwx_brd_pal.hpp"
#include "mwx_boards.hpp"

#include "../mwx_twenet.hpp" // if using the_twelite instance.

#include "../sensors/mwx_sns_MC3630.hpp"
#include "../sensors/mwx_sns_PCA9632.hpp"

namespace mwx { inline namespace L1 {
	class BrdPalNotice : public mwx::BrdPal, MWX_APPDEFS_CRTP(BrdPalNotice)
	{
	public:
		static const uint8_t TYPE_ID = mwx::BOARD::PAL_AMB;

		// load common definition for handlers
		#define __MWX_APP_CLASS_NAME BrdPalNotice
		#include "../_mwx_cbs_hpphead.hpp"
		#undef __MWX_APP_CLASS_NAME

	public: // sensor instances
		SnsMC3630 sns_MC3630;
		SnsPCA9632 sns_PCA9632;
		
	public:
		// constructor
		BrdPalNotice() {}

		// begin method (if necessary, configure object here)
		void _setup() {
			BrdPal::_hardware_init();
			
			sns_MC3630.setup();

			Wire.begin();
			sns_PCA9632.setup();
		}

		// begin method (if necessary, start object here)
		void _begin() {
			BrdPal::_begin();

			pinMode(PIN_SNS_INT, INPUT_PULLUP);
			attachIntDio(PIN_SNS_INT, FALLING);

			// SNS_EN is used to switch output of PCA9632.
			sns_PCA9632.reset();
			pinMode(PIN_SNS_EN, OUTPUT_INIT_LOW);
		}

	public:
		// TWENET callback handler (mandate)
		void loop() {
			BrdPal::_loop();
		}

		void on_sleep(uint32_t & val) {
			BrdPal::_on_sleep();
		}

		void warmboot(uint32_t & val) {}

		void wakeup(uint32_t & val) {
			BrdPal::_wakeup();

			pinMode(PIN_SNS_INT, INPUT_PULLUP);
			attachIntDio(PIN_SNS_INT, FALLING);

			sns_MC3630.wakeup();
		}

		void on_create(uint32_t& val) { _setup();  }
		void on_begin(uint32_t& val) { _begin(); }
		void on_message(uint32_t& val) { }

	public: // never called the following as hardware class, but define it!
		void network_event(mwx::packet_ev_nwk& pEvNwk) {}
		void receive(mwx::packet_rx& rx) {
			BrdPal::_receive();
		}
		void transmit_complete(mwx::packet_ev_tx& pEvTx) {
			BrdPal::_transmit_complete();
		}

	public: // LED output controls.
		static const uint8_t LED_OFF = SnsPCA9632::LED_OFF;
		static const uint8_t LED_ON = SnsPCA9632::LED_PWM;
		static const uint8_t LED_BLINK = SnsPCA9632::LED_BLINK;
		static const uint8_t LED_NOP = SnsPCA9632::LED_NOP;
		
		static const uint8_t LED_R = SnsPCA9632::LED1;
		static const uint8_t LED_G = SnsPCA9632::LED2;
		static const uint8_t LED_B = SnsPCA9632::LED3;
		static const uint8_t LED_W = SnsPCA9632::LED4;

		static const uint8_t LED_REG_MAX_PWM = 127;
		static const uint8_t LED_REG_BOOST_PWM = 255;

		void set_led_master_sw_on() { digitalWrite(PIN_SNS_EN, LOW); }
		void set_led_master_sw_off() { digitalWrite(PIN_SNS_EN, HIGH); }
		void set_led_r_blink() { sns_PCA9632.set_led_status(LED_BLINK, LED_NOP, LED_NOP, LED_NOP); }
		void set_led_r_on() { sns_PCA9632.set_led_status(LED_ON, LED_NOP, LED_NOP, LED_NOP); }
		void set_led_r_off() { sns_PCA9632.set_led_status(LED_OFF, LED_NOP, LED_NOP, LED_NOP); }
		void set_led_g_on() { sns_PCA9632.set_led_status(LED_NOP, LED_ON, LED_NOP, LED_NOP); }
		void set_led_g_blink() { sns_PCA9632.set_led_status(LED_NOP, LED_BLINK, LED_NOP, LED_NOP); }
		void set_led_g_off() { sns_PCA9632.set_led_status(LED_NOP, LED_OFF, LED_NOP, LED_NOP); }
		void set_led_b_on() { sns_PCA9632.set_led_status(LED_NOP, LED_NOP, LED_ON, LED_NOP); }
		void set_led_b_blink() { sns_PCA9632.set_led_status(LED_NOP, LED_NOP, LED_BLINK, LED_NOP); }
		void set_led_b_off() { sns_PCA9632.set_led_status(LED_NOP, LED_NOP, LED_OFF, LED_NOP); }
		void set_led_w_on() { sns_PCA9632.set_led_status(LED_NOP, LED_NOP, LED_NOP, LED_ON); }
		void set_led_w_blink() { sns_PCA9632.set_led_status(LED_NOP, LED_NOP, LED_NOP, LED_BLINK); }
		void set_led_w_off() { sns_PCA9632.set_led_status(LED_NOP, LED_NOP, LED_NOP, LED_OFF); }

		void set_leds(uint8_t r, uint8_t g, uint8_t b, uint8_t w) { sns_PCA9632.set_led_status(r, g, b, w); }
		void set_leds_off() { sns_PCA9632.set_led_status(LED_OFF, LED_OFF, LED_OFF, LED_OFF); }

		void set_led_brightness_r_reg(uint8_t duty) { sns_PCA9632.set_led_duty(LED_R, duty); }
		void set_led_brightness_g_reg(uint8_t duty) { sns_PCA9632.set_led_duty(LED_G, duty); }
		void set_led_brightness_b_reg(uint8_t duty) { sns_PCA9632.set_led_duty(LED_B, duty); }
		void set_led_brightness_w_reg(uint8_t duty) { sns_PCA9632.set_led_duty(LED_W, duty); }
		void set_leds_brightness_reg(uint8_t r, uint8_t g, uint8_t b, uint8_t w) { sns_PCA9632.set_led_duty_all(r, g, b, w); }

		void set_led_brightness_r1000(uint16_t duty, bool boost = false) { sns_PCA9632.set_led_duty(LED_R, boost ? scale_1000_to_256u8(duty) : scale_1000_to_127u8(duty)); }
		void set_led_brightness_g1000(uint16_t duty, bool boost = false) { sns_PCA9632.set_led_duty(LED_G, boost ? scale_1000_to_256u8(duty) : scale_1000_to_127u8(duty)); }
		void set_led_brightness_b1000(uint16_t duty, bool boost = false) { sns_PCA9632.set_led_duty(LED_B, boost ? scale_1000_to_256u8(duty) : scale_1000_to_127u8(duty)); }
		void set_led_brightness_w1000(uint16_t duty, bool boost = false) { sns_PCA9632.set_led_duty(LED_W, boost ? scale_1000_to_256u8(duty) : scale_1000_to_127u8(duty)); }
		void set_leds_brightness1000(uint16_t r, uint16_t g,uint16_t b,uint16_t w, bool boost = false);

		void set_blink_duty1000(uint16_t x) { sns_PCA9632.set_blink_duty(scale_1000_to_256u8(x)); }
		void set_blink_cycle_ms(uint16_t x) { sns_PCA9632.set_blink_cycle_ms(x); }

		/**
		 * @brief test LEDs by flashing.
		 *        note: set_led_master_sw_on() is called regardless of master_sw status. 
		 */
		void test_led();
	};
}}
