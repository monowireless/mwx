/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */

#include <TWELITE>
#include "mwx_sns_MC3630.hpp"

// should define at some place (PAL ports)
static const uint8_t PIN_SNS_EN = 16;
static const uint8_t PIN_SNS_INT = 17;

bool mwx::SnsMC3630::_reset_and_init( uint16_t ModeFreq, uint8_t Range, uint8_t SplNum ) {
	// set SPI mode as slower mode.
	SPI.begin(0, SPISettings(2000000, SPI_CONF::MSBFIRST, SPI_CONF::SPI_MODE3));

    // set standby mode.
    _sleep();
    // reset
    _spi_single_op(MC3630_RESET | MC3630_WRITE, 0x40);
    // ...abracadabra
    _spi_single_op(0x1B | MC3630_WRITE, 0x00);

	// note: need enough delay (>=1ms) here.
	delay(1);
	
    // Set SPI mode
    _spi_single_op(MC3630_FREG_1 | MC3630_WRITE, 0x90);

    // ...abracadabra
    _spi_single_op(MC3630_INIT_1 | MC3630_WRITE, 0x42);
    _spi_single_op(MC3630_DMX | MC3630_WRITE, 0x1);
    _spi_single_op(MC3630_DMY | MC3630_WRITE, 0x80);
    _spi_single_op(MC3630_INIT_2 | MC3630_WRITE, 0x00);
    _spi_single_op(MC3630_INIT_3 | MC3630_WRITE, 0x00);
    // abracadabra...

    // NOW read chip id (should be 0x71)
    uint8_t chip_id = _spi_single_op(MC3630_CHIP_ID | MC3630_READ, 0x00);
    //Serial << crlf << format("MC3630 Chip ID=%02X", chip_id);
	if (chip_id != 0x71) return false;

    // standby again
    _sleep();

	// set SPI clock higher and accelerometer ultra low power.
	//_spi_single_op(MC3630_PMCR | MC3630_WRITE, 0x80 | 0x30 | 0x03); // Ultra Low Power
	_spi_single_op(MC3630_PMCR | MC3630_WRITE, ModeFreq >> 8); // Nowmal = Low Power
	
	// Change SPI Config (for higher clock)
    //SPI.begin(0, SPISettings(8000000, SPI_CONF::MSBFIRST, SPI_CONF::SPI_MODE3));

	// set sampling rate
	_spi_single_op(MC3630_RATE_1 | MC3630_WRITE, ModeFreq & 0xFF);

	// set resolution (16g, 12bit)
	_spi_single_op(MC3630_RANGE_C | MC3630_WRITE, Range | 0x04);

	// set FIFO count
	_spi_single_op(MC3630_FIFO_C | MC3630_WRITE, 0x40 | SplNum);

	// set interrupt at new data and set pin as push-pull.
	_spi_single_op(MC3630_INTR_C | MC3630_WRITE, 0x41);

	// return accelerometer's status by read-write register.
	_spi_single_op(MC3630_FREG_2 | MC3630_WRITE, 0x04);

	// now start capturing continuously.
	_wakeup();

	return true;
}

uint8_t mwx::SnsMC3630::_read_result() {
	if (!bConnect) return 0;

	uint8_t	au8data[6] = { 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF };
	uint8_t	u8num = 0;

	while (1) {
		static uint16_t t;

		// the end check.
		if (auto&& x = SPI.get_rwer()) {
			if (x.transfer(MC3630_READ | MC3630_XOUT_LSB) & 0x10) break;
		
			// read data
			for (int i = 0; i < 6; i++) {
				au8data[i] = x.transfer(0x00);
			}
		}

		if (_samples_que.is_full()) _samples_que.pop();

		_samples_que.push(
			axis_xyzt(int16_t(au8data[1] << 8 | au8data[0]) * u8RangeScale,
			     int16_t(au8data[3] << 8 | au8data[2]) * u8RangeScale,
			     int16_t(au8data[5] << 8 | au8data[4]) * u8RangeScale,
			     t++)
		);
			
		u8num++;
	}

	_clear_int_reg();

	//Serial << '(' << int(u8num) << ')';
	return u8num;
}
