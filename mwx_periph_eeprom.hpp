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
		class _eep_strm_hlpr : public mwx::stream<_eep_strm_hlpr> {
			using SUPER_STREAM = mwx::stream<_eep_strm_hlpr>;
			using self_type = _eep_strm_hlpr;

			periph_eeprom* _ref;
			uint16_t _idx_read;

			/**
			 * @brief Set the this object pointer to stream class.
			 *        (note: this is used for calling printf_ library in C)
			 */
			inline void set_obj() {
				SUPER_STREAM::set_pvOutputContext(static_cast<_eep_strm_hlpr*>(this));
			}

		public: // override constructors/assignment operators/initialization methods.
			_eep_strm_hlpr(periph_eeprom& ref) : _ref(&ref), _idx_read(0) {
				set_obj();
			}

            // move operator
            _eep_strm_hlpr(self_type&& ref) {
                set_obj(); // must do this!
                _idx_read = ref._idx_read;
            }

            // remove copy operator
            _eep_strm_hlpr(const self_type&) = delete;

		public: // implement stream interfacee
		
			/**
			 * @brief rewind the read index to the head.
			 * 
			 */
			inline void rewind() {
				_idx_read = 0;
			}

        	/**
			 * @brief set the posision
			 * 
			 */
			inline void seek(int n) {
                if (n < 0) _idx_read = (*_ref).size() - n;
                else _idx_read = n;
			}

			/**
			 * @brief check if there is remaining buffer to read.
			 * 
			 * @return int 0:no data 1:there is data to read
			 */
			inline int available() {
				return (_idx_read < _ref->size());
			}

			/**
			 * @brief read a byte from buffer.
			 * 
			 * @return int -1:error or read byte
			 */
			inline int read() { 
                if (available()) {
                    uint8_t c = (*_ref).read(_idx_read);
                    _idx_read++;
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
                (*_ref).write(_idx_read, n);
                _idx_read++;
				return 1;
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