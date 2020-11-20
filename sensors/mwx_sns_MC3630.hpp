/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */
#pragma once

#include "../_tweltite.hpp"
#include "mwx_sensors.hpp"
#include "mwx_debug.h"

namespace mwx {
    inline namespace L1 {
        class SnsMC3630 : public sensor_crtp<SnsMC3630> {
        public: // common interface.
            using que_type = smplque<axis_xyzt, mwx::alloc_local<axis_xyzt, 32>>;

            bool probe() {
                return true;
            }

            void setup(uint32_t arg1 = 0, uint32_t arg2 = 0) {
                bConnect = false;
                u16FreqMode = 0;
                u8SampleNum = 0;
                u8Range = 0;
                u8RangeScale = 0;
            }

            void begin(uint32_t arg1 = DEFAULT_CONF, 
                       uint32_t arg2 = 0) {

                // init the values
				u16FreqMode = arg1 & 0xFFFF; // MC3630_SAMPLING25HZ;

                u8Range = (arg1 >> 16) & 0xFF; // RANGE
                switch (u8Range) {
                    case MC3630_RANGE16G: u8RangeScale = 8; break;
                    case MC3630_RANGE8G: u8RangeScale = 4; break;
                    case MC3630_RANGE4G: u8RangeScale = 2; break;
                    case MC3630_RANGE2G: u8RangeScale = 1; break;
                    default:
                        u8Range = MC3630_RANGE16G;
                        u8RangeScale = 8;
                        break;
                }
                
				u8SampleNum = arg1 >> 24;  // int every 28 samples (assume 10bitx3ch, 20samples)
                if (u8SampleNum == 0 || u8SampleNum > SAMPLE_COUNT_MAX) {
                    u8SampleNum = SAMPLE_COUNT_DEFAULT;
                }

				// init, Int every 20 samples
				if (!_reset_and_init(u16FreqMode, u8Range, u8SampleNum)) {
                    // Serial << "MC3630: cannot access the device";
					bConnect = false;
				}
				else {
					_clear_int_reg();
					bConnect = true;
				}
            }

            void process_ev(uint32_t arg1, uint32_t arg2 = 0) {
                // this sensor is not timer event driven.
            }

            bool available() {
                return !_samples_que.empty();
            }

            void end() {
                // stop the working.
                if (bConnect) {
                    bConnect = false;
                    _sleep();
                }
            }

            void on_sleep() {
                // no action, FIFO is running.
            }

            void wakeup() {
                if (bConnect) {
                    SPI.begin(0, SPISettings(2000000, SPI_CONF::MSBFIRST, SPI_CONF::SPI_MODE3));

                    if (_read_int()) {
                        // if samples are stored in FIFO, read them.
                        _read_result();
                    }
                }
            }

        public: // local procedure  
            bool _reset_and_init( uint16_t ModeFreq, uint8_t Range, uint8_t SplNum );
            bool _start_read() { return true; }
            uint8_t _read_result();
            uint8_t _read_int() { return _spi_single_op(MC3630_STATUS_2|MC3630_READ, 0x00); }
            void _clear_int_reg() { _spi_single_op(MC3630_STATUS_2|MC3630_WRITE, 0x00); }
            uint8_t _read_sampling_freq() { return _spi_single_op(MC3630_RATE_1|MC3630_READ, 0x00); }
            void _wakeup() { _spi_single_op(MC3630_MODE_C|MC3630_WRITE, 0x05); }
            void _sleep() { _spi_single_op(MC3630_MODE_C|MC3630_WRITE, 0x01); }


            uint8_t read() { return _read_result(); }

            que_type& get_que() { return _samples_que; }

            struct Settings {
                uint16_t _mode_freq;
                uint8_t _range;
                uint8_t _smp_ct;

                Settings() : _mode_freq(MODE_LP_14HZ), _range(MC3630_RANGE16G), _smp_ct(SAMPLE_COUNT_DEFAULT) {}
                Settings(uint16_t mf, uint8_t rn = RANGE_DEFAULT, uint8_t sc = SAMPLE_COUNT_DEFAULT) {
                    _mode_freq = mf;
                    _range = rn;
                    _smp_ct = sc;
                }

                operator uint32_t () { 
                    return (  (_smp_ct << 24) 
                            | (_range << 16) 
                            | (_mode_freq & 0x0000FFFF));
                }
            };

        private: // local control vars
  			que_type _samples_que;
			uint16_t u16FreqMode;
     		uint8_t u8SampleNum;
            uint8_t u8Range;
            uint8_t u8RangeScale;

			bool_t bConnect;

        private:
            inline uint8_t _spi_single_op(uint8_t cmd, uint8_t arg) {
                uint8_t d0, d1;
                if (auto&& x = SPI.get_rwer()) {
                    d0 = x.transfer(cmd); (void)d0;
                    d1 = x.transfer(arg);
                    // (x << (cmd)) >> d0;
                    // (x << (arg)) >> d1;
                }
                //Serial << mwx_format("<%02X%02X>%02X>", cmd, arg, d1);

                return d1;
            }
        
        private: // const defs
            static const uint8_t MC3630_EXT_STAT_1        = (0x00);
            static const uint8_t MC3630_EXT_STAT_2        = (0x01);
            static const uint8_t MC3630_XOUT_LSB          = (0x02);
            static const uint8_t MC3630_XOUT_MSB          = (0x03);
            static const uint8_t MC3630_YOUT_LSB          = (0x04);
            static const uint8_t MC3630_YOUT_MSB          = (0x05);
            static const uint8_t MC3630_ZOUT_LSB          = (0x06);
            static const uint8_t MC3630_ZOUT_MSB          = (0x07);
            static const uint8_t MC3630_STATUS_1          = (0x08);
            static const uint8_t MC3630_STATUS_2          = (0x09);
            static const uint8_t MC3630_FREG_1            = (0x0D);
            static const uint8_t MC3630_FREG_2            = (0x0E);
            static const uint8_t MC3630_INIT_1            = (0x0F);
            static const uint8_t MC3630_MODE_C            = (0x10);
            static const uint8_t MC3630_RATE_1            = (0x11);
            static const uint8_t MC3630_SNIFF_C           = (0x12);
            static const uint8_t MC3630_SNIFFTH_C         = (0x13);
            static const uint8_t MC3630_SNIFFCF_C         = (0x14);
            static const uint8_t MC3630_RANGE_C           = (0x15);
            static const uint8_t MC3630_FIFO_C            = (0x16);
            static const uint8_t MC3630_INTR_C            = (0x17);
            static const uint8_t MC3630_CHIP_ID           = (0x18);
            static const uint8_t MC3630_INIT_3            = (0x1A);
            static const uint8_t MC3630_PMCR              = (0x1C);
            static const uint8_t MC3630_DMX               = (0x20);
            static const uint8_t MC3630_DMY               = (0x21);
            static const uint8_t MC3630_DMZ               = (0x22);
            static const uint8_t MC3630_RESET             = (0x24);
            static const uint8_t MC3630_INIT_2            = (0x28);
            static const uint8_t MC3630_TRIGC             = (0x29);
            static const uint8_t MC3630_XOFFL             = (0x2A);
            static const uint8_t MC3630_XOFFH             = (0x2B);
            static const uint8_t MC3630_YOFFL             = (0x2C);
            static const uint8_t MC3630_YOFFH             = (0x2D);
            static const uint8_t MC3630_ZOFFL             = (0x2E);
            static const uint8_t MC3630_ZOFFH             = (0x2F);
            static const uint8_t MC3630_XGAIN             = (0x30);
            static const uint8_t MC3630_YGAIN             = (0x31);
            static const uint8_t MC3630_ZGAIN             = (0x32);
            static const uint8_t MC3630_WRITE             = (0x40);
            static const uint8_t MC3630_READ              = (0xC0);
            static const uint8_t MC3630_CONVTIME          = (0);
            static const uint8_t MC3630_X                 = (0);
            static const uint8_t MC3630_Y                 = (1);
            static const uint8_t MC3630_Z                 = (2);
            static const uint8_t MC3630_RANGE2G           = (0x00);
            static const uint8_t MC3630_RANGE4G           = (0x10);
            static const uint8_t MC3630_RANGE8G           = (0x20);
            static const uint8_t MC3630_RANGE16G          = (0x30);

        public:
            static const uint16_t MODE_LP_1HZ_UNOFFICIAL  = 0x0002; // may work, but no information in the datasheet!
            static const uint16_t MODE_LP_2HZ_UNOFFICIAL  = 0x0003; // may work, but no information in the datasheet!
            static const uint16_t MODE_LP_7HZ_UNOFFICIAL  = 0x0004; // may work, but no information in the datasheet!
            static const uint16_t MODE_LP_14HZ     = 0x0005;
            static const uint16_t MODE_LP_28HZ     = 0x0006;
            static const uint16_t MODE_LP_54HZ     = 0x0007;
            static const uint16_t MODE_LP_105HZ    = 0x0008;
            static const uint16_t MODE_LP_210HZ    = 0x0009;
            static const uint16_t MODE_LP_400HZ    = 0x000A;
            static const uint16_t MODE_ULP_25HZ    = 0xB306;
            static const uint16_t MODE_ULP_50HZ    = 0xB307;
            static const uint16_t MODE_ULP_100HZ   = 0xB308;
            static const uint16_t MODE_ULP_190HZ   = 0xB309;
            static const uint16_t MODE_ULP_380HZ   = 0xB30A;
            
            static const uint8_t RANGE_PLUS_MINUS_1G = MC3630_RANGE2G;
            static const uint8_t RANGE_PLUS_MINUS_2G = MC3630_RANGE4G;
            static const uint8_t RANGE_PLUS_MINUS_4G = MC3630_RANGE8G;
            static const uint8_t RANGE_PLUS_MINUS_8G = MC3630_RANGE16G;
            static const uint8_t RANGE_DEFAULT = MC3630_RANGE16G;

            static const uint8_t SAMPLE_COUNT_DEFAULT = 28;
            static const uint8_t SAMPLE_COUNT_MAX = 31;
            static const uint32_t DEFAULT_CONF = ((SAMPLE_COUNT_DEFAULT << 16) | (MC3630_RANGE16G << 16) | MODE_LP_14HZ);
        };
    }
}