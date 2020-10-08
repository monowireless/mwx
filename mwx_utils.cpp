/* Copyright (C) 2020 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */

#include "mwx_common.hpp"
#include "mwx_utils.hpp"

namespace mwx { inline namespace L1 {
	/**
	 * @brief faster dividing by 10 using bit shift.
	 * 
	 * @param val 
	 * @return int32_t 
	 */
	inline int32_t _div10_shift(int32_t val) {
		int32_t n = val < 0 ? -val : val;
		unsigned q, r;
		q = (n >> 1) + (n >> 2);
		q = q + (q >> 4);
		q = q + (q >> 8);
		q = q + (q >> 16);
		q = q >> 3;
		r = n - (((q << 2) + q) << 1);
		int32_t rval = (q + (r > 9));
		return rval;
	}

	// format functions
	_div_chars div_result_i32::format(int dig_quo, uint32_t opt) const {
		_div_chars s;

		// rem part
		int32_t n = rem;
		for(int i = 0; i < digits_rem - 1; i++) {
			int32_t nn = _div10_shift(n);
			s.push_front((n - nn*10) + '0');
			n = nn;
		}
		s.push_front(n+'0');

		// dot
		s.push_front('.');

		// quo part
		n = quo;
		int dig_quo_now = 0;
		while(n > 10) {
			int32_t nn = _div10_shift(n);
			s.push_front((n - nn*10) + '0');
			n = nn;
			dig_quo_now++;
		}
		s.push_front(n+'0');
		dig_quo_now++;

		// padding 0
		if (dig_quo && opt & ZERO_PAD) {
			while (dig_quo_now < dig_quo) {
				s.push_front('0');
				dig_quo_now++;
			}
		}

		// sign
		if (opt & SIGN_PLUS) {
			s.push_front(b_neg ? '-' : '+');
		} else if (opt & SIGN_SPACE) {
			s.push_front(b_neg ? '-' : ' ');
		} else if (b_neg) {
			s.push_front('-');
		}

		// padding SPACE
		if (dig_quo) {
			while (dig_quo_now < dig_quo) {
				s.push_front(' ');
				dig_quo_now++;
			}
		}    

		return s;
	}

	// div10
	//   compute division and remains by 100 without performing / nor %.
	// return: int val divided value (always positive)
	//         int&rem remains (always positive)
	//         bool&neg true if the value is negative.
	div_result_i32 div10(int32_t val) {
		bool neg = (val < 0);
		val = neg ? -val : val;
		int32_t dv = _div10_shift(val);
		int32_t rem = val - dv * 10;

		return { dv, (int16_t)rem, neg, 1};
	}
	
	// div100 (val can be up to 999999=>9999.99)
	//   compute division and remains by 100 without performing / nor %.
	// return: int val divided value (always positive)
	//         int&rem remains (always positive)
	//         bool&neg true if the value is negative.
	div_result_i32 div100(int32_t val) {
		bool neg = (val < 0);
		val = neg ? -val : val;
		int32_t dv = val * 1311 >> 17; // get intger part: 
									   //   int(V) ~= val/100 ~= V*131100/131072 ~= val*1311/131072
									   //     where V = float(val)/100.0
		                               //   note: dv might be bigger in some case.
		int32_t rem = val - dv * 100;
		while (rem < 0) {
			dv--;
			rem += 100;
		}

		return {dv, (int16_t)rem, neg, 2};
	}

	// div1000 (val can be up to 9999999>9999.999)
	//   compute division and remains by 1000 without performing / nor %.
	// return: int val divided value (always positive)
	//         int&rem remains (always positive)
	//         bool&neg true if the value is negative.
	div_result_i32 div1000(int32_t val) {
		bool neg = (val < 0);
		val = neg ? -val : val;
		int32_t dv = val * 131 >> 17; // get integer part:
								  	  //   int(V) ~= val/1000 =~ V*131000/131072 ~= val*131/131072 
								  	  //      where V = float(val)/1000.0
								  	  //   note: dv might be smaller in some case.
		int32_t rem = val - dv * 1000;
		while (rem >= 1000) {
			dv++;
			rem -= 1000;
		}

		return {dv, (int16_t)rem, neg, 3};
	}
}}