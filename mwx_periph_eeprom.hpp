/* Copyright (C) 2020 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */

#pragma once

#ifndef _MWX_PERIPH_EEPROM_HPP_
# define _MWX_PERIPH_EEPROM_HPP_
#endif

#include "twecommon.h"
#include "mwx_common.hpp"
#include "mwx_stream.hpp"
#include "mwx_debug.h"

extern "C" bool_t EEP_6x_bRead(uint16  u16StartAddr, uint16 u16Bytes, uint8 *pu8Buffer);
extern "C" bool_t EEP_6x_bWrite(uint16 u16StartAddr, uint16 u16Bytes, uint8 *pu8Buffer);

namespace mwx {  inline namespace L1 {
    class periph_eeprom {
    public:
        void write(uint16_t address, uint8_t value) {
            EEP_6x_bWrite(address, 1, &value);
        }

        uint8_t read(uint16_t address) {
            uint8_t value = 0xFF;
            EEP_6x_bRead(address, 1, &value);
            return value;
        }
        
        void update(uint16_t address, uint8_t value) {
            uint8_t value_read = read(address);
            if (value != value_read) {
                write(address, value);
            }
        }

        // returns size of EEPROM
        static constexpr uint16_t size() { return 3840; }

    public:

    	/**
		 * @brief helper class with mwx::stream interface.
		 */
		class _eep_strm_hlpr : public mwx::stream<_eep_strm_hlpr>, public mwx::_stream_helper_array_type<periph_eeprom> {
			using SUPER_STREAM = mwx::stream<_eep_strm_hlpr>;
            using SUPER_HLPR = mwx::_stream_helper_array_type<periph_eeprom>;
			using self_type = _eep_strm_hlpr;

			/**
			 * @brief Set the this object pointer to stream class.
			 *        (note: this is used for calling printf_ library in C)
			 */
			inline void set_obj() {
				SUPER_STREAM::set_pvOutputContext(static_cast<_eep_strm_hlpr*>(this));
			}

		public: // override constructors/assignment operators/initialization methods.
			_eep_strm_hlpr(periph_eeprom& ref) : SUPER_HLPR(ref) {
				set_obj();
			}

			// handles move constructor
			_eep_strm_hlpr(self_type&& ref) : SUPER_HLPR(std::forward<self_type>(ref)) {
				set_obj();
			}

            // remove copy operator
            _eep_strm_hlpr(const self_type&) = delete;

		public: // implement stream interfacee
			/**
			 * @brief read a byte from buffer.
			 * 
			 * @return int -1:error or read byte
			 */
			inline int read() { 
                if (available()) {
                    uint8_t c = SUPER_HLPR::_ref->read(SUPER_HLPR::_idx_rw);
                    SUPER_HLPR::_idx_rw++;
                    return c;
                } else {
                    return -1;
                }
			}

			inline void flush(void) { } // do nothing

			/**
			 * @brief append one entry
			 * 
			 * @param n 
			 * @return size_t 
			 */
			inline size_t write(int n) {
				// append a byte
                if (SUPER_HLPR::_idx_rw < SUPER_HLPR::_ref->size()) {
                    SUPER_HLPR::_ref->write(SUPER_HLPR::_idx_rw, n);
                    SUPER_HLPR::_idx_rw++;
				    return 1;
                } else return 0;
			}

			/**
			 * @brief output function for printf_ library.
			 * 
			 * @param out 
			 * @param vp 
			 */
			inline static void vOutput(char out, void* vp) {
				// append a byte
				if (vp != nullptr) {
					auto thisobj = reinterpret_cast<self_type*>(vp);
					thisobj->write(uint8_t(out));
				}
			}
		}; // _eep_strm_hlpr

		/**
		 * @brief	generate helper class with stream interface.
		 */
		_eep_strm_hlpr get_stream_helper() {
			return std::move(_eep_strm_hlpr(*this));
		}
    };

    extern periph_eeprom EEPROM;
}} // L1 // mwx