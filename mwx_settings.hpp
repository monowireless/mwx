/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */

#pragma once

#include <iterator>

#include "twecommon.h"
#include "twesettings.h"
#include "twesettings_std.h"
#include "twesettings_std_defsets.h"
#include "twesettings_cmd.h"
#include "twesettings_validator.h"

#include "tweinteractive.h"

#include "mwx_debug.h"

// exportable setting information.
//   if not set, use default instead.
extern const char *INTRCT_USER_APP_NAME;
extern const uint32_t INTRCT_USER_APP_ID;
extern const TWESTG_tsElement INTRCT_USER_BASE_SETTINGS[];
extern const TWESTG_tsElement INTRCT_USER_SLOT0_SETTINGS[];
extern const TWESTG_tsElement INTRCT_USER_SLOT1_SETTINGS[];
extern const TWESTG_tsElement INTRCT_USER_SLOT2_SETTINGS[];
extern const TWESTG_tsElement INTRCT_USER_SLOT3_SETTINGS[];
extern const TWESTG_tsElement INTRCT_USER_SLOT4_SETTINGS[];
extern const TWESTG_tsElement INTRCT_USER_SLOT5_SETTINGS[];
extern const TWESTG_tsElement INTRCT_USER_SLOT6_SETTINGS[];
extern const TWESTG_tsElement INTRCT_USER_SLOT7_SETTINGS[];

namespace mwx { inline namespace L1 {
	class settings {
	public:
		typedef uint8_t size_type;

		class element_type {
		public:
			typedef element_type value_type;

		protected:
			size_type _pos;
			settings* _body;

		public:
			element_type(size_type pos, settings* body) : _pos(pos), _body(body) { }

			// get settings ID
			inline uint16_t get_id() {
				return _body->get_psFinal()->apEle[_pos]->u16Id;
			}

			// get configuration (setting definition and default values)
			inline TWESTG_tsElement* get_definition() {
				TWESTG_tsFinal* psFinal = _body->get_psFinal();
				TWESTG_tsElement* psEle = psFinal->apEle[_pos];
				return psEle;
			}

			// get setting value
			inline TWESTG_tsDatum* get_value() {
				TWESTG_tsFinal* psFinal = _body->get_psFinal();
				TWESTG_tsDatum* psDat = &psFinal->asDatum[_pos];
				return psDat;
			}

			// operator to save value into variable.
			friend void operator << (uint32_t&, value_type&);
			friend void operator << (int32_t&, value_type&);
			friend void operator << (uint16_t&, value_type&);
			friend void operator << (int16_t&, value_type&);
			friend void operator << (uint8_t&, value_type&);
			friend void operator << (int8_t&, value_type&);
		};

		typedef element_type value_type;

		class iterator : protected element_type {
		public:
			typedef element_type value_type;
			typedef iterator self_type;
			typedef std::forward_iterator_tag iterator_category;
			typedef value_type* pointer;
			typedef value_type& reference;
			typedef int difference_type;

		public:
			iterator(size_type pos, settings* body) : element_type(pos, body) {
				//MWX_DebugMsg(0, "{C:%d}", pos);
			}

			inline self_type& operator ++() {
				value_type::_pos++;
				return *this;
			}

			inline self_type operator ++(int) {
				self_type p(value_type::_pos, value_type::_body);
				operator++();
				return p;
			}

			inline reference operator *() {	return *this; }
			inline pointer operator->() { return reinterpret_cast<pointer>(this); }
			inline bool operator == (self_type && itr2) { return !operator != (itr2); }
			inline bool operator != (self_type & itr2) { 
				return !(value_type::_pos == itr2._pos && value_type::_body == itr2._body); }
		};

	public:	
		settings() : _psFinal(nullptr) { }
		settings(TWESTG_tsFinal* ps) : _psFinal(ps) { }

		void attach(TWESTG_tsFinal* ps) {
			_psFinal = ps;
		}

		inline value_type operator[] (int i) {
			return value_type(i, this);
		}

		inline iterator begin() {
			return iterator(0, this);
		}

		inline iterator end() {
			return iterator(_psFinal->u8DataCount, this);
		}

		inline TWESTG_tsFinal* get_psFinal() {
			return _psFinal;
		}

	private:
		TWESTG_tsFinal* _psFinal;
	};


	class interactive_settings {
		settings _stgs;

	public:
		interactive_settings() {}
		
		void begin(uint8_t slot = 0xFF);

		void loop() { TWEINTRCT_vHandleSerialInput(); }

		settings get_settings() {
			return _stgs;
		}

	};

	inline void operator << (uint32_t& u32, settings::value_type& e) {
		u32 = e._body->get_psFinal()->asDatum[e._pos].uDatum.u32;
	}
	inline void operator << (int32_t& i32, settings::value_type& e) {
		i32 = e._body->get_psFinal()->asDatum[e._pos].uDatum.i32;
	}
	inline void operator << (uint16_t& u16, settings::value_type& e) {
		u16 = e._body->get_psFinal()->asDatum[e._pos].uDatum.u16;
	}
	inline void operator << (int16_t& i16, settings::value_type& e) {
		i16 = e._body->get_psFinal()->asDatum[e._pos].uDatum.i16;
	}
	inline void operator << (uint8_t& u8, settings::value_type& e) {
		u8 = e._body->get_psFinal()->asDatum[e._pos].uDatum.u8;
	}
	inline void operator << (int8_t& i8, settings::value_type& e) {
		i8 = e._body->get_psFinal()->asDatum[e._pos].uDatum.i8;
	}
}}
