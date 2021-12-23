/* Copyright (C) 2021 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */

#include <TWELITE>
#include "mwx_sns_SHT4x.hpp"

bool SnsSHT4x::reset() {
	if(auto&& x = Wire.get_writer(_i2c_addr)){
		x << 0x94;
	}else return false;

	return true;
}

MWX_APIRET SnsSHT4x::process_ev( uint32_t arg1, uint32_t arg2){
	(void)arg2;
	static uint16_t count = 0;

	switch ( (teEvent)arg1 )
	{
	case E_EVENT_START_UP:
		count = get_waittime_ms();
		break;
	
	case E_EVENT_TICK_TIMER:
		{
			if(_available){
				count = 0;
			}

			if(count == 0 ){
				if( !_available ) {
					_available = _read();
				}
			}else{
				count--;
			}
		}
		break;
	
	default:
		break;
	}
	
	return MWX_APIRET(true);
}

bool SnsSHT4x::begin(uint8_t mode) {
	_mode = mode;

	_available = false;

	uint8_t _addr = 0xF6;
	_wait_ms = 5;
	switch (mode)
	{
	case SHT4x_HIGH_PRECISION:
		_addr = 0xFD;
		_wait_ms = 10;
		break;
	case SHT4x_MIDIUM_PRECISION:
		_addr = 0xF6;
		_wait_ms = 5;
		break;
	case SHT4x_LOW_PRECISION:
		_addr = 0xE0;
		_wait_ms = 2;
		break;
	case SHT4x_HIGH_PRECISION_200mWHEATER_LONG:
		_addr = 0x39;
		_wait_ms = 1100;
		break;
	case SHT4x_HIGH_PRECISION_200mWHEATER_SHORT:
		_addr = 0x32;
		_wait_ms = 110;
		break;
	case SHT4x_HIGH_PRECISION_110mWHEATER_LONG:
		_addr = 0x2F;
		_wait_ms = 1100;
		break;
	case SHT4x_HIGH_PRECISION_110mWHEATER_SHORT:
		_addr = 0x24;
		_wait_ms = 110;
		break;
	case SHT4x_HIGH_PRECISION_90mWHEATER_LONG:
		_addr = 0x1E;
		_wait_ms = 1100;
		break;
	case SHT4x_HIGH_PRECISION_90mWHEATER_SHORT: 
		_addr = 0x15;
		_wait_ms = 110;
		break;
	
	default:
		break;
	}
	

	if(auto&& x = Wire.get_writer(_i2c_addr)){
		x << _addr;
	}else return false;

	process_ev(E_EVENT_START_UP);

	return true;
}

bool SnsSHT4x::available() {
	return _available;
}

bool SnsSHT4x::_read() {
	uint16_t _u16temp, _u16hum;
	uint8_t _u8temp_csum, _u8hum_csum;
	if(auto&& x = Wire.get_reader(_i2c_addr)){
		x >> _u16temp;
		x >> _u8temp_csum;
		x >> _u16hum;
		x >> _u8hum_csum;
	}else return false;

	if(CRC8_u8CalcU16(_u16temp, 0xFF) != _u8temp_csum) return false;
	if(CRC8_u8CalcU16(_u16hum, 0xFF) != _u8hum_csum) return false;

	_temp = (int16_t)(-4500 + ((17500*int32_t(_u16temp)) >> 16) );
	_hum = (int16_t)(-600 + ((12500*int32_t(_u16hum)) >> 16) );

	return true;
}