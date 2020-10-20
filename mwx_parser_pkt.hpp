/* Copyright (C) 2020 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */

#pragma once

#include "mwx_common.hpp"
#include "mwx_parser.hpp"
#include "mwx_debug.h"

#undef _DEBUG
#ifdef _DEBUG
# warning "_DEBUG Sets!"
# define MWX_PARSER_PKT_APP_FIXED_BUF_DEFAULT 32
#else
# define MWX_PARSER_PKT_APP_FIXED_BUF_DEFAULT 0
#endif
#ifndef MWX_PARSER_PKT_APPUART_FIXED_BUF
#define MWX_PARSER_PKT_APPUART_FIXED_BUF MWX_PARSER_PKT_APP_FIXED_BUF_DEFAULT
#endif

#ifndef MWX_PARSER_PKT_APPPAL_FIXED_BUF
#define MWX_PARSER_PKT_APPPAL_FIXED_BUF 24
#endif

#ifndef MWX_PARSER_PKT_APPTAG_FIXED_BUF
#define MWX_PARSER_PKT_APPTAG_FIXED_BUF MWX_PARSER_PKT_APP_FIXED_BUF_DEFAULT
#endif

namespace mwx {  inline namespace L1 {
    enum class E_PKT : uint8_t {
		PKT_ERROR = 0,
		PKT_TWELITE, // for APP TWELITE
		PKT_PAL,     // for APP PAL
		PKT_APPIO,   // for APP IO
		PKT_APPUART, // for APP UART
		PKT_APPTAG,  // for APP UART
		PKT_ACT_STD  // for Act Standard packet structure
	};

    /*****************************************************
	 * identify packet type
	 *****************************************************/
    E_PKT identify_packet_type(const uint8_t* p, uint16_t u16len);
    static inline E_PKT identify_packet_type(const uint8_t* p, const uint8_t* e) {
		return identify_packet_type(p, uint16_t(e - p));
	}

    /*****************************************************
	 * BASE class of TwePacket
	 *****************************************************/
    class TwePacket {
	protected:
		E_PKT _type;		
	public:
		TwePacket(E_PKT ptyp = E_PKT::PKT_ERROR) : _type(ptyp), common{} { }
		E_PKT parse(const uint8_t* p, uint16_t len) { return E_PKT::PKT_ERROR; }
		E_PKT parse(const uint8_t* p, const uint8_t* e) { return E_PKT::PKT_ERROR; }
		inline E_PKT get_type() { return _type; }
		~TwePacket() {}
		
		static const E_PKT _pkt_id = E_PKT::PKT_ERROR;
		
		// common information set by parse()
		struct {
			uint32_t tick;
			uint32_t src_addr;
			uint8_t src_lid;
			uint8_t lqi;
			uint16_t volt;
			void clear() {
				tick = 0;
				src_addr = 0;
				src_lid = 0;
				lqi = 0;
				volt = 0;
			}
		} common;
	};

    /*****************************************************
	 * APP TWELITE 0x81 command
	 *****************************************************/
	struct DataTwelite {
		/**
		 * source address (Serial ID)
		 */
		uint32_t u32addr_src;

		/**
		 * source address (logical ID)
		 */
		uint8_t u8addr_src;

		/**
		 * destination address (logical ID)
		 */
		uint8_t u8addr_dst;

		/**
		 * sequence counter
		 */
		uint16_t u16timestamp;

		/**
		 * true when trying to low latency transmit (same packets will come)
		 */
		uint8_t b_lowlatency_tx;

		/**
		 * packet repeat count
		 *   e.g.) if set 1, the packet passed to one repeater (router) to the destination.
		 */
		uint8_t u8rpt_cnt;

		/**
		 * LQI value
		 */
		uint8_t u8lqi;

		/**
		 * true: DI1 is activated (set as Lo),
		 */
		uint8_t DI1;

		/**
		 * true: DI1 is activated before.
		 * false: the port had not been activated ever.
		 */
		uint8_t DI1_active;

		/**
		 * true: DI2 is activated (set as Lo)
		 */
		uint8_t DI2;

		/**
		 * true: DI2 is activated before.
		 * false: the port had not been activated ever.
		 */
		uint8_t DI2_active;

		/**
		 * true: DI3 is activated (set as Lo)
		 */
		uint8_t DI3;

		/**
		 * true: DI3 is activated before.
		 * false: the port had not been activated ever.
		 */
		uint8_t DI3_active;

		/**
		 * true: DI4 is activated (set as Lo)
		 */
		uint8_t DI4;

		/**
		 * true: DI4 is activated before.
		 * false: the port had not been activated ever.
		 */
		uint8_t DI4_active;

		/**
		 * DI state mask, active if bit set. (LSB:DI1, bit2:DI2, ...)
		 * Note: same values as DI?.
		 */
		uint8_t DI_mask;

		/**
		 * DI active mask, active if bit set. (LSB:DI1, bit2:DI2, ...)
		 * Note: same values as DI?_active.
		 */
		uint8_t DI_active_mask;

		/**
		 * module voltage in mV
		 */
		uint16_t u16Volt;

		/**
		 * ADC1 value in mV
		 */
		uint16_t u16Adc1;

		/**
		 * ADC2 value in mV
		 */
		uint16_t u16Adc2;

		/**
		 * ADC3 value in mV
		 */
		uint16_t u16Adc3;

		/**
		 * ADC4 value in mV
		 */
		uint16_t u16Adc4;

		/**
		 * if bit set, Adc has value (LSB: ADC1, bit2: ADC2, ...),
		 * otherwise Adc is connected to Vcc voltage.
		 * (note: Setting Adc as Vcc level means unused to App_Twelite firmware,
		 *        even the hardware can measure up to 2.47V)
		 */
		uint8_t Adc_active_mask;
    };

    class TwePacketTwelite : public TwePacket, public DataTwelite {
	public:
		static const E_PKT _pkt_id = E_PKT::PKT_TWELITE;

		TwePacketTwelite() : TwePacket(_pkt_id), DataTwelite({ 0 }) { }
		~TwePacketTwelite() { }
		E_PKT parse(const uint8_t* p, uint16_t len);
		E_PKT parse(const uint8_t* p, const uint8_t* e) { return parse(p, uint16_t(e - p)); }
	};


	/*****************************************************
	 * APP IO 0x81 command
	 *****************************************************/
	struct DataAppIO {
		/**
		 * source address (Serial ID)
		 */
		uint32_t u32addr_src;

		/**
		 * source address (logical ID)
		 */
		uint8_t u8addr_src;

		/**
		 * destination address (logical ID)
		 */
		uint8_t u8addr_dst;

		/**
		 * sequence counter
		 */
		uint16_t u16timestamp;

		/**
		 * true when trying to low latency transmit (same packets will come)
		 */
		uint8_t b_lowlatency_tx;

		/**
		 * packet repeat count
		 *   e.g.) if set 1, the packet passed to one repeater (router) to the destination.
		 */
		uint8_t u8rpt_cnt;

		/**
		 * LQI value
		 */
		uint8_t u8lqi;

		/**
		 * DI state mask, active if bit set. (LSB:DI1, bit2:DI2, ...)
		 */
		uint16_t DI_mask;

		/**
		 * DI active mask, active if bit set. (LSB:DI1, bit2:DI2, ...)
		 * note: if not changed from power/reset, 
		 */
		uint16_t DI_active_mask;

		/**
		 * DI interrupt mask, active if bit set. (LSB:DI1, bit2:DI2, ...)
		 * note: if the change is caused by an I/O interrupt, set the corresponding bit.
		 */
		uint16_t DI_int_mask;
	};

	class TwePacketAppIO : public TwePacket, public DataAppIO {
	public:
		static const E_PKT _pkt_id = E_PKT::PKT_APPIO;

		TwePacketAppIO() : TwePacket(_pkt_id), DataAppIO({ 0 }) { }
		~TwePacketAppIO() { }
		E_PKT parse(const uint8_t* p, uint16_t len);
		E_PKT parse(const uint8_t* p, const uint8_t* e) { return parse(p, uint16_t(e - p)); }
	};


	/*****************************************************
	 * APP UART PAYLOAD
	 *****************************************************/
	struct DataAppUART {
		/**
		 * source address (Serial ID)
		 */
		uint32_t u32addr_src;

		/**
		 * source address (Serial ID)
		 */
		uint32_t u32addr_dst;

		/**
		 * source address (logical ID)
		 */
		uint8_t u8addr_src;

		/**
		 * destination address (logical ID)
		 */
		uint8_t u8addr_dst;

		/**
		 * LQI value
		 */
		uint8_t u8lqi;

		/**
		 * Response ID
		 */
		uint8_t u8response_id;

		/**
		 * Payload length
		 */
		uint16_t u16paylen;

		/**
		 * payload
		 */
#if MWX_PARSER_PKT_APPUART_FIXED_BUF == 0
		mwx::smplbuf_u8_attach payload;
#else
		mwx::smplbuf_u8<MWX_PARSER_PKT_APPUART_FIXED_BUF> payload;
#endif
	};

	class TwePacketAppUART : public TwePacket, public DataAppUART {
		TwePacketAppUART(TwePacketAppUART&&) = delete; // delete copy constructor
		TwePacketAppUART(const TwePacketAppUART&) = delete; // delete copy constructor
	public:
		static const E_PKT _pkt_id = E_PKT::PKT_APPUART;

		TwePacketAppUART(E_PKT pktid = E_PKT::PKT_APPUART) : TwePacket(pktid) { }
		~TwePacketAppUART() { }
		E_PKT parse(const uint8_t* p, uint16_t len);
		E_PKT parse(const uint8_t* p, const uint8_t* e) { return parse(p, uint16_t(e - p)); }
	};

	/*****************************************************
	 * APP TAG PAYLOAD (very limited support)
	 *   - sensor data is not parsed, but store raw data
	 *     into 'payload'
	 *****************************************************/
	struct DataAppTAG {		
		//   0 1 2 3 4 5 6 7 8 9 a b c d e f 0 1 2 3 -
		// :80000000B10001810043C10032C9047C02AF0A41D2
 		//  ^^^^^^^1^2^^^3^^^^^^^4^5^6^7^^^8^^^9^^^a^b

		uint32_t u32addr_rpt;
		uint32_t u32addr_src;

		uint16_t u16seq;
		uint16_t u16Volt;

		uint8_t u8lqi;
		uint8_t u8addr_src;
		
		uint8_t u8sns;

#if MWX_PARSER_PKT_APPTAG_FIXED_BUF == 0
		mwx::smplbuf_u8_attach payload;
#else
		mwx::smplbuf_u8<MWX_PARSER_PKT_APPTAG_FIXED_BUF> payload;
#endif
	};

	class TwePacketAppTAG : public TwePacket, public DataAppTAG {
	public:
		static const E_PKT _pkt_id = E_PKT::PKT_APPTAG;

		TwePacketAppTAG() : TwePacket(_pkt_id) { }
		~TwePacketAppTAG() { }
		E_PKT parse(const uint8_t* p, uint16_t len);
		E_PKT parse(const uint8_t* p, const uint8_t* e) { return parse(p, uint16_t(e - p)); }
	};

	/*****************************************************
	 * Act standard packet structure
	 *****************************************************/
	class TwePacketActStd : public TwePacketAppUART {
	public:
		static const E_PKT _pkt_id = E_PKT::PKT_ACT_STD;

		TwePacketActStd() : TwePacketAppUART(this->_pkt_id) { }
		E_PKT parse(const uint8_t* p, uint16_t len) {
			E_PKT ret = TwePacketAppUART::parse(p, len);
			if (ret == E_PKT::PKT_APPUART) ret = E_PKT::PKT_ACT_STD;
			return ret;
		}
		E_PKT parse(const uint8_t* p, const uint8_t* e) { return parse(p, uint16_t(e - p)); }

		~TwePacketActStd() { }
	};

	/*****************************************************
	 * TWELITE PAL
	 *****************************************************/
	enum class E_PAL_PCB : uint8_t {
		NOPCB = 0x0,
		MAG = 0x01,
		AMB = 0x02,
		MOT = 0x03,
		NOTICE = 0x04
	};

	enum class E_SNSCD : uint8_t {
		HALL = 0x0,
		TEMP = 0x1,
		HUMD = 0x2,
		LUMI = 0x3,
		ACCEL = 0x4,
		EVENT = 0x5,
		ACCEL_XYZ = 0x24,
		VOLT = 0x30,
		DIO = 0x31,
		EEP = 0x32
	};

	enum class E_EXCD_VOLT : uint8_t {
		POWER = 0x8,
		ADC1 = 0x1,
		ADC2 = 0x2,
		ADC3 = 0x3,
		ADC4 = 0x4
	};

	struct DataPal {
		uint32_t u32addr_rpt;
		uint32_t u32addr_src;

		uint16_t u16seq;

		uint8_t u8lqi;
		uint8_t u8addr_src;
		E_PAL_PCB u8palpcb;		// PCB kind (lower 5bits) and revision (higher 3 bits)
		uint8_t u8palpcb_rev;	// PCB resision
		uint8_t u8sensors;		// MSB=1:include parse error, lower bits: num of sensors
		uint8_t u8snsdatalen;   // data length

		union {
			const uint8_t *au8snsdata; 
			uint8_t _pobj[MWX_PARSER_PKT_APPPAL_FIXED_BUF];
		};
	};

	// PAL event
	struct PalEvent {
		uint8_t b_stored;
		uint8_t u8event_source;
		uint8_t u8event_id;       // Event code
		uint32_t u32event_param;   // 24bit length
	};

	struct PalBase {
		uint32_t u32StoredMask; // bit0...: bit mask to store sensor data
	};

	struct PalMag : public PalBase {
		static const uint8_t U8VARS_CT = 2; // Volt + MagStat
		static const uint32_t STORE_COMP_MASK = (1 << U8VARS_CT) - 1;

		uint16_t u16Volt;       // module voltage
		uint8_t u8MagStat;
		uint8_t bRegularTransmit; // MSB flag of u8MagStat
	};

	struct PalAmb : public PalBase {
		static const uint8_t U8VARS_CT = 4; // Volt + 3Sensors
		static const uint32_t STORE_COMP_MASK = (1 << U8VARS_CT) - 1;

		uint16_t u16Volt;       // module voltage
		int16_t i16Temp;
		uint16_t u16Humd;
		uint32_t u32Lumi;
	};

	struct PalMot : public PalBase {
		static const uint8_t U8VARS_CT = 17; // Volt + 3 AXIS*16samples
		static const uint32_t STORE_COMP_MASK = 3; // Volt & 1sample

		uint16_t u16Volt;       // module voltage
		uint8_t u8samples; // num of sample stored.
		uint8_t u8sample_rate_code; // sample rate (0: 25Hz, 4:100Hz)
		int16_t i16X[16]; // X axis samples
		int16_t i16Y[16]; // Y axis samples
		int16_t i16Z[16]; // Z axis samples
	};

	class TwePacketPal : public TwePacket, public DataPal, public PalEvent {
		// parse each sensor data and convert into variables.
		uint32_t store_data(const uint8_t *psns, const uint8_t *ptr_end, uint8_t u8listct, void** vars, const uint8_t* pu8argsize, const uint8_t* pu8argcount_max, const uint16_t* pu8dsList, const uint16_t* pu8exList, uint8_t* pu8exListReads = nullptr);
		std::pair<bool, uint16_t> query_volt(const uint8_t *psns, const uint8_t *psns_end);
		std::pair<bool, PalEvent> query_event(const uint8_t *psns, const uint8_t *psns_end);

		// create a new PalXXX object.
		template <typename T>
		T& _new_palobj(T& out, const uint8_t *ptr, const uint8_t *ptr_end);

		// create PalXXX at local _pobj[].
		template <typename T>
		bool _new_local() {
			if (sizeof(T) <= MWX_PARSER_PKT_APPPAL_FIXED_BUF) {
				const uint8_t *psns = au8snsdata;
				const uint8_t *psns_end = au8snsdata + u8snsdatalen;
				T* pobj = new ((void*)_pobj) T(); // note: now au8snsdata is destroyed
				_new_palobj(*pobj, psns, psns_end);
				u8snsdatalen = 0; // mark as au8snsdata is diabled
				return true;
			} else {
				return false;
			}
		} 

		// get an object.
		//   if locally created an object into _pobj, use this.
		//   otherwise create Pal Object from data array via _new_palobj.
		template <typename T>
		T _get() {
			if (u8snsdatalen) {
				T out;
				_new_palobj(out, au8snsdata, au8snsdata + u8snsdatalen);
				return out;
			} else {
				//	T* p __attribute__((__may_alias__)) = reinterpret_cast<T*>((void*)_pobj);
				T* p = static_cast<T*>((void*)_pobj);
				return *p;
			}		
		}

	public:
		static const E_PKT _pkt_id = E_PKT::PKT_PAL;

		TwePacketPal() : TwePacket(_pkt_id), DataPal({ 0 }) { }
		~TwePacketPal() { }
		E_PKT parse(const uint8_t* p, uint16_t len);
		E_PKT parse(const uint8_t* p, const uint8_t* e) { return parse(p, uint16_t(e - p)); }

		// query Pal Event Data
		bool is_PalEvent() { return PalEvent::b_stored; }
		PalEvent& get_PalEvent() { return *static_cast<PalEvent*>(this); }
		PalEvent& operator >> (PalEvent& out) {
			out = get_PalEvent();
			return out;
		}

		// query PalMag data structure
		PalMag& operator >> (PalMag& out) { return out = _get<PalMag>(); }
		PalMag get_PalMag() { return _get<PalMag>(); }

		// query PalMag data structure
		PalAmb& operator >> (PalAmb& out) { return out = _get<PalAmb>(); }
		PalAmb get_PalAmb() { return _get<PalAmb>(); }

		// query PalMag data structure
		PalMot& operator >> (PalMot& out) { return out = _get<PalMot>(); }
		PalMot get_PalMot() { return _get<PalMot>(); }
	};
	template<> PalMag& TwePacketPal::_new_palobj<PalMag>(PalMag& out, const uint8_t *ptr, const uint8_t *ptr_end);
	template<> PalAmb& TwePacketPal::_new_palobj<PalAmb>(PalAmb& out, const uint8_t *ptr, const uint8_t *ptr_end);
	template<> PalMot& TwePacketPal::_new_palobj<PalMot>(PalMot& out, const uint8_t *ptr, const uint8_t *ptr_end);

    /*************************************************************
     * PACKET OBJECT
     *************************************************************/
	union _largest_size_of_TwePacket {
		TwePacketPal _p1;
		TwePacketTwelite _p2;
		TwePacketActStd _p3;
		TwePacketAppIO _p4;
		TwePacketAppTAG _p5;
		TwePacketAppUART _p6;
	};

    class packet_parser {
        uint8_t _pobj[sizeof(_largest_size_of_TwePacket)];
        E_PKT _pkt_id;
    public:
        E_PKT get_type() { return _pkt_id; }
        packet_parser() : _pkt_id(E_PKT::PKT_ERROR) {}

		template <class T>
		E_PKT parse(const uint8_t* p, const uint8_t* e) {
			static_assert(sizeof(T) <= sizeof(_pobj), "Pre-alloc size overflow. Check MAX_SIZE_PRINTOBJ.");
			T* pobj = new ((void*)_pobj) T();
			_pkt_id = pobj->parse(p, e);
			return _pkt_id;
		}

		template <class T>
		E_PKT parse(const uint8_t* p, uint16_t len) {
			return parse<T>(p, p + len);
		}

		template <class T> 
		T& use() {
			if (T::_pkt_id == E_PKT::PKT_ERROR || this->get_type() == T::_pkt_id) {
				return *static_cast<T*>(this->pv_get_pobj());
			} else {
				MWX_Panic(2, "refTwePacketGen() type mismatch %d->%d.", this->get_type(), T::_pkt_id);
				T* p = reinterpret_cast<T*>((void*)0xdeadbeef); // when beaf dies, what would it be?
				return *p;
			}
		}

        void *pv_get_pobj() { return (void*)_pobj; }
    };

	template <class T> 
	inline T& refTwePacket(packet_parser& p) {
		if (T::_pkt_id == E_PKT::PKT_ERROR || p.get_type() == T::_pkt_id) {
			return *static_cast<T*>(p.pv_get_pobj());
		} else {
            MWX_Panic(2, "refTwePacketGen() type mismatch %d->%d.", p.get_type(), T::_pkt_id);
			T* p = reinterpret_cast<T*>((void*)0xdeadbeef); // when beaf dies, what would it be?
            return *p;
        }
	}

	using pktparser = packet_parser;

}} // L1 // mwx