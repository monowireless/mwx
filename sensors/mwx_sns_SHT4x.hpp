/* Copyright (C) 2021 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */

#pragma once

#include "../_tweltite.hpp"
#include "mwx_sensors.hpp"

namespace mwx {
	inline namespace L1 {

		class SnsSHT4x{
			uint8_t _i2c_addr;

		public:
			/**
			 * @brief Construct a new SnsSHT4x object
			 *        (note: it's never called if declared in global scope)
			 * @param i2c_addr device address over I2C
			 */
			SnsSHT4x(uint8_t i2c_addr = DEFAULT_I2C_ADDRESS) {setup(i2c_addr);};

			/**
			 * @brief init value (doing same when constuctor is called)
			 *  
			 * @param i2c_addr device address over I2C
			 */
			void setup(uint8_t i2c_addr = DEFAULT_I2C_ADDRESS) {
				_i2c_addr = i2c_addr;
				_available = false;
			}

			/**
			 * @brief set registers as defaults.
			 * 
			 * @return true success
			 * @return false fail to communicate with the device. 
			 */
			bool reset();

			/**
			 * @brief start measurement
			 * 
			 * @return true success
			 * @return false fail to communicate with the device. 
			 */
			bool begin(uint8_t mode = SHT4x_DEFAULT_PRECISION);

			/**
			 * @brief check end of measurement
			 * 
			 * @return true success
			 * @return false fail to communicate with the device. 
			 */
			bool available();

			/**
			 * @brief get temperature
			 * 
			 * @return temperature
			 */

			double get_temp() { return _temp/100.0; } 

			/**
			 * @brief get temperature
			 * 
			 * @return temperature
			 */
			int16_t get_temp_cent() { return _temp; }

			/**
			 * @brief get humidity
			 * 
			 * @return humidity
			 */
			double get_humid() { return _hum/100.0; }

			/**
			 * @brief get humidity
			 * 
			 * @return humidity
			 */
			int16_t get_humid_per_dmil() { return _hum; }

			/**
			 * @brief get measurement time(ms)
			 * 
			 * @return time(ms)
			 */
			uint16_t get_waittime_ms() { return _wait_ms; }

			/**
			 * @brief process event
			 */
			MWX_APIRET process_ev(uint32_t arg1, uint32_t arg2 = 0);

			/**
			 * @brief probe I2C device
			 * 
			 * @return true success (found the device with _i2c_addr)
			 * @return false fail to communicate with the device.  
			 */
			bool probe() {
				return Wire.probe(_i2c_addr);
			}

			/**
			 * @brief end measurement
			 */
			void end() { _available = false; }

			void wakeup(){
				if(!Wire._has_begun()) Wire.begin();

				if(_read()){
					_available = true;
				}
			}

			void on_sleep(){
				// no action
			}

		private:
			/**
			 * @brief read sensor data
			 * 
			 * @return true success
			 * @return false fail to communicate with the device. 
			 */
			bool _read();

		public:
			static const uint8_t DEFAULT_I2C_ADDRESS = 0x44;

			static const uint8_t SHT4x_DEFAULT_PRECISION = 0x01;
			static const uint8_t SHT4x_HIGH_PRECISION = 0x00;
			static const uint8_t SHT4x_MIDIUM_PRECISION = 0x01;
			static const uint8_t SHT4x_LOW_PRECISION = 0x02;
			static const uint8_t SHT4x_HIGH_PRECISION_200mWHEATER_LONG = 0x03;
			static const uint8_t SHT4x_HIGH_PRECISION_200mWHEATER_SHORT = 0x04;
			static const uint8_t SHT4x_HIGH_PRECISION_110mWHEATER_LONG = 0x05;
			static const uint8_t SHT4x_HIGH_PRECISION_110mWHEATER_SHORT = 0x06;
			static const uint8_t SHT4x_HIGH_PRECISION_90mWHEATER_LONG = 0x07;
			static const uint8_t SHT4x_HIGH_PRECISION_90mWHEATER_SHORT = 0x08;

		private:
			int16_t _temp;
			int16_t _hum;
			uint16_t _wait_ms;
			uint8_t _mode;
			bool _available;
		};
	}
}