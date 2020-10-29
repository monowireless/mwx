/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */
#pragma once

#include <cstdint>
#include <type_traits>

#include <ToCoNet.h>

#include "mwx_common.hpp"
#include "mwx_common_instances.hpp"

#include "mwx_debug.h"
#include "mwx_stream.hpp"
#include "mwx_utils.hpp"

#include "networks/mwx_networks.hpp"

extern "C" uint32_t u32DioPortWakeUp; //! Wake Up Port 
extern "C" uint8_t u8TimerWakeUp; //! Wake Up by Timer
extern "C" tsFILE _sSerLegacy;

extern void _MWX_vOnSleep();

// TWENET internal defs (internal defs)
extern "C" uint32 _ToCoNet_u32PhyChan();        // get the current channel number
extern "C" void ToCoNet_vReg_mod_Channel_Mgr(); // enabler function for Channel Manager
extern "C" void _ToCoNet_Mod_vReg_PRSEV();      // register mod PRSEV.

namespace mwx { inline namespace L1 {
	class twenet;

	static const uint32_t _TWENET_CALLED_FROM_TWE_TWELITE = 0x80000000;

	// manage appdefs_virt instance at the_twelite.
	class twenet_instance_manager {
		uint8_t _id;
		uint8_t _setup_finished;
		mwx::appdefs_virt& _the_instance;

	public:
		twenet_instance_manager(mwx::appdefs_virt& tinst) 
				: _id(0), _setup_finished(0),_the_instance(tinst) { }

		operator bool() {
			return _id != 0;
		}

		void _finish_setup() {
			_setup_finished = 1;
		}

		// use board (with parameter)
		template <class T>
		T& use(uint32_t& opt) {
			if (_id != 0) {
				// already has an instance.
				return get<T>();
			}
			else if (!_setup_finished) {
				_id = T::TYPE_ID;
				T* p = new T(); // NOTE: do not consider about delete object.
				
				_the_instance = *p;
				p->on_create(opt);

				return *p;
			} else {
				MWX_Panic(2, "TIM::use() new instance can add only at setup().");
				
				T* p = reinterpret_cast<T*>((void*)0xdeadbeef); // when beaf dies, what would it be?
				return *p;
			}
		}

		// use board
		template <class T>
		T& use() {
			uint32_t opt = _TWENET_CALLED_FROM_TWE_TWELITE; // on create (might not be used) 
			return use<T>(opt);
		}

		// get board instance
		template <class T>
		inline T& get() {
			if (_id == T::TYPE_ID) {
				// debugg
				return _the_instance.cast<T>();
			} else {
				MWX_Panic(2, "TIM::get() type mismatch %d->%d.", _id, T::TYPE_ID);

				T* p = reinterpret_cast<T*>((void*)0xdeadbeef); // when beaf dies, what would it be?
				return *p;
			}
		}

		// get virt instance
		inline mwx::appdefs_virt& vget() { return _the_instance; }
	};

	class twenet {
		uint8_t _setup_finished;

	public:
		static const uint8_t CPUCLK_32MHZ = 3;
		static const uint8_t CPUCLK_BOOST = 3;
		static const uint8_t CPUCLK_16MHZ = 2;
		static const uint8_t CPUCLK_DEFAULT = 2;
		static const uint8_t CPUCLK_8MHZ = 1;
		static const uint8_t CPUCLK_HALF = 1;
		static const uint8_t CPUCLK_4MHZ = 0;
		static const uint8_t CPUCLK_QUARTER = 0;

		static const uint8_t SLEEP_WAKETIMER_PRIMARY = E_AHI_WAKE_TIMER_0;
		static const uint8_t SLEEP_WAKETIMER_SECONDARY = E_AHI_WAKE_TIMER_1;


	public:
		// define helper class for setting parameter on TWENET (sToCoNet_AppContext)
		#define _MWX_TWENET_GEN(ty, name, twenetname, nochange, def) \
			inline ty get_##name() { return sToCoNet_AppContext.twenetname; } \
			inline void set_##name(ty _##name) { if(_setup_finished && !nochange) sToCoNet_AppContext.twenetname = _##name; } \
			struct name : public setting { \
					typedef setting SUPER; \
					ty _##name; name(ty name##_ def) : _##name(name##_){}\
					void set(uint8_t s) { if(!s || (s && !nochange)) sToCoNet_AppContext.twenetname = _##name; } \
					template <class T> static void to_stream(stream<T>& strm) { strm << mwx::L2::crlf << mwx::mwx_format(#name"("#twenetname")=%x", sToCoNet_AppContext.twenetname) << mwx::L2::flush; } \
			}

		class setting {}; // dummy super class (for static_assert)
		_MWX_TWENET_GEN(uint32_t, appid, u32AppId, 1, );
		// _MWX_TWENET_GEN(uint32_t, chmask, u32ChMask, 0, );
		_MWX_TWENET_GEN(uint16_t, short_addr, u16ShortAddress, 0, );
		_MWX_TWENET_GEN(uint8_t, channel, u8Channel, 0, );
		_MWX_TWENET_GEN(uint8_t, cpu_base_clock, u8CPUClk, 0, );
		_MWX_TWENET_GEN(uint8_t, tx_power, u8TxPower, 0, );
		_MWX_TWENET_GEN(uint8_t, mac_retry_count, u8TxMacRetry, 0, );
		_MWX_TWENET_GEN(uint8_t, rx_when_idle, bRxOnIdle, 0, = 1);
		_MWX_TWENET_GEN(uint8_t, _promiscuous_mode, bPromiscuousMode, 0, = 1);
		_MWX_TWENET_GEN(uint8_t, skip_rc_calib, bSkipBootCalib, 1, = 1);
		_MWX_TWENET_GEN(uint8_t, osc_32k_mode, u8Osc32Kmode, 1, );
		_MWX_TWENET_GEN(uint8_t, cca_retry, u8CCA_Retry, 0, );
		_MWX_TWENET_GEN(uint8_t, cca_level, u8CCA_Level, 0, );
		_MWX_TWENET_GEN(uint8_t, mac_init_pending, u8MacInitPending, 1, = 1);
		_MWX_TWENET_GEN(uint8_t, _no_ack_mode, bNoAckMode, 1, = 1);
		_MWX_TWENET_GEN(uint8_t, receive_external_pan, bRxExtPan, 0, = 1);
		_MWX_TWENET_GEN(uint8_t, rand_mode, u8RandMode, 1, );

		// set channel mask (can configure up to 3 channels at once)
		struct chmask : public setting {
			static const uint32_t U32MASK_11_26 = 0x7FFF800UL;
			uint32_t u32chmask;

			chmask(uint32_t u32chmask_) {
				u32chmask = u32chmask_ & U32MASK_11_26;
			} 

			// if having 2 or more
			template <typename... Tail>
			chmask(uint8_t ch1, uint8_t ch2, Tail&&... tail) {
				u32chmask = pack_bits(ch1, ch2, std::forward<Tail>(tail)...);
				u32chmask &= U32MASK_11_26;
			}
			void set(uint8_t setup_finised) { // if this call is during setup, the `setup_finished' is set 0.
				sToCoNet_AppContext.u32ChMask = u32chmask;
			}

			template <class T>
			static void to_stream(stream<T>& strm) {
				strm << mwx::L2::crlf
					 << mwx::mwx_format("chmask(u32ChMask)=%x", sToCoNet_AppContext.u32ChMask)
					 << mwx::L2::flush; }
		};

		// activate channel manager
		struct chmgr : public setting {
			uint8_t u8ch_prime;
			uint32_t u32chmask;
			chmgr(uint8_t ch1 = 18, uint8_t ch2 = 0, uint8_t ch3 = 0) {
				u8ch_prime = ch1;
				u32chmask = (1UL << ch1) | (1UL << ch2) | (1UL << ch3);
				u32chmask &= 0x7FFF800UL;
			}
			void set(uint8_t setup_finised) { // if this call is during setup, the `setup_finished' is set 0.
				if(setup_finised == 0) {
					sToCoNet_AppContext.u8Channel = u8ch_prime;
					sToCoNet_AppContext.u32ChMask = u32chmask;
					ToCoNet_vReg_mod_Channel_Mgr();
					_ToCoNet_Mod_vReg_PRSEV(); // require libTWENET_1_3_4
				}
			}
		};

		template <class T>
		twenet& operator << (T&& setobj) {
			static_assert(std::is_base_of<mwx::twenet::setting, T>::value == true, "use setting objecct derived from twenet::setting");
			setobj.set(_setup_finished); // if nochange flag is set, it's not changed anymore.
			return *this;
		}

		// check setting values
		template <class T>
		void operator >> (stream<T>& strm) {
			appid::to_stream(strm);
			chmask::to_stream(strm);
			short_addr::to_stream(strm);
			channel::to_stream(strm);
			cpu_base_clock::to_stream(strm);
			tx_power::to_stream(strm);
			mac_retry_count::to_stream(strm);
			rx_when_idle::to_stream(strm);
			cca_retry::to_stream(strm);
			cca_level::to_stream(strm);
			mac_init_pending::to_stream(strm);
			receive_external_pan::to_stream(strm);
			rand_mode::to_stream(strm);
		}

	public:
		twenet() : receiver(), _setup_finished(0),
				   board(the_vhw), network(the_vnet), app(the_vapp), settings(the_vsettings) {}

		// this shall be run at setup() as the_twelite.begin().
		void begin() {
			// set chmask explicitly.
			if (sToCoNet_AppContext.u32ChMask == 0) {
				sToCoNet_AppContext.u32ChMask = (1UL << sToCoNet_AppContext.u8Channel);
			}
			
			uint32_t var;
			var = _TWENET_CALLED_FROM_TWE_TWELITE;
			board.vget().on_event(_MWX_EV_ON_BEGIN, var);
			board._finish_setup();

			_setup_finished = 1;
		}

		// begin the objects (at the first call of loop())
		void _begin() {
			uint32_t var;
			
			var = _TWENET_CALLED_FROM_TWE_TWELITE;
			settings.vget().on_event(_MWX_EV_ON_BEGIN, var);
			settings._finish_setup();

			var = _TWENET_CALLED_FROM_TWE_TWELITE;
			network.vget().on_event(_MWX_EV_ON_BEGIN, var);
			network._finish_setup();

			var = _TWENET_CALLED_FROM_TWE_TWELITE;
			app.vget().on_event(_MWX_EV_ON_BEGIN, var);
			app._finish_setup();
		}

		// if setup is finished
		inline bool _has_setup_finished() {
			return _setup_finished == 1;
		}

		// direct access to sToCoNet_AppContext.
		inline tsToCoNet_AppContext& get_app_context() {
			return sToCoNet_AppContext;
		}

		// reconfig with updated app context
		inline void reconfigure() {
			if (sToCoNet_Context.bToCoNetStarted) {
				ToCoNet_vRfConfig();
			}
		}

		// change channel
		inline bool change_channel(uint8_t u8Channel) {
			uint8_t chprev = sToCoNet_AppContext.u8Channel;
			sToCoNet_AppContext.u8Channel = u8Channel;
			if (sToCoNet_Context.bToCoNetStarted) {
				ToCoNet_vChConfig();
			}

			bool ret = (get_channel_now() != u8Channel);
			if (ret) {
				sToCoNet_AppContext.u8Channel = chprev;
			}

			return ret;
		}

		inline uint8_t get_channel_now() {
			return uint8_t(_ToCoNet_u32PhyChan());
		}

		inline bool change_short_addr(uint16_t u16ShortAddress) {
			sToCoNet_AppContext.u16ShortAddress = u16ShortAddress;
			if (sToCoNet_Context.bToCoNetStarted) {
				ToCoNet_vRfConfig();
			}
			return true;
		}

		// if mac_init_pending is set, need to call before RF operation.
		void start_pending_mac() {
			ToCoNet_vMacStart();
		}

		// channel
		inline uint16_t get_panid() { return ToCoNet_u16GetPanId(); }
		inline uint32_t get_hw_serial() { return sToCoNet_Context.u32MySerial; }

		// operation
		inline void sleep(uint32_t u32Periodms, bool bPeriodic = true, bool bRamOff = false, uint8_t u8Device = SLEEP_WAKETIMER_PRIMARY) {
			_MWX_vOnSleep();
			ToCoNet_vSleep(u8Device, u32Periodms, bPeriodic, bRamOff);
		}

		// crypt
		inline bool register_crypt_key(uint8_t* pu8Key, tsCryptDefs* sCryptDefs = nullptr) {
			return ToCoNet_bRegisterAesKey(pu8Key, sCryptDefs);
		}

		// system reset
		inline void reset_system() {
			vAHI_SwReset();
		}

		// timer calibration
		inline uint16_t calib_rc_timer() {
			return ToCoNet_u16RcCalib(0);
		}
		inline void set_rc_timer(uint16_t val) {
			ToCoNet_u16RcCalib(val);
		}
		inline uint16_t get_rc_timer() {
			return ToCoNet_u16RcCalib(0xFFFF);
		}

		// watch dog
		inline void stop_watchdog() {
			vAHI_WatchdogStop();
		}
		inline void restart_watchdog() {
			vAHI_WatchdogRestart();
		}

		// debug
		inline void debug_level(uint8_t u8lvl) {
			ToCoNet_vDebugInit(&_sSerLegacy);
			ToCoNet_vDebugLevel(u8lvl);
		}

		// version
		inline uint32_t get_twenet_version() { return ToCoNet_u32GetVersion(); }
		inline uint32_t get_twenet_libex_version() { return ToCoNet_u32GetVersion_LibEx(); }

		// wakeup source
		bool is_wokeup_by_dio(uint8_t port) {
			return ((u32DioPortWakeUp & (1UL << port)) != 0);
		}
		bool is_wokeup_by_wktimer() {
			return ((u8TimerWakeUp & (E_AHI_WAKE_TIMER_MASK_0 | E_AHI_WAKE_TIMER_MASK_1)) != 0); // WAKE_TIMER_0 or 1
		}

	private:
		class _tx_stat {
		public:
			inline bool is_complete(uint8_t cbid) { return the_mac.tx_status.is_complete(cbid); }
			inline bool is_success(uint8_t cbid) { return the_mac.tx_status.is_success(cbid); }
		};

	public:
		// instances
		_tx_stat tx_status;
		twenet_rx_packet_queue receiver;

		twenet_instance_manager board;
		twenet_instance_manager network;
		twenet_instance_manager app;
		twenet_instance_manager settings;
	};
}}

// declaration of the instance.
extern mwx::L1::twenet the_twelite;
