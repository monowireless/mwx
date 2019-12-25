/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */
#pragma once

#include <ToCoNet.h>
#include "mwx_utils.hpp"
#include "mwx_debug.h"

namespace E_MWX {
	static const uint32_t STATE_0 = 0;
	static const uint32_t STATE_1 = (1UL << 0);
	static const uint32_t STATE_2 = (1UL << 1);
	static const uint32_t STATE_3 = (1UL << 2);
	static const uint32_t STATE_4 = (1UL << 3);
	static const uint32_t STATE_5 = (1UL << 4);
	static const uint32_t STATE_6 = (1UL << 5);
	static const uint32_t STATE_7 = (1UL << 6);
	static const uint32_t STATE_8 = (1UL << 7);
	static const uint32_t STATE_9 = (1UL << 8);
}

namespace mwx { inline namespace L1 {
	template <class T>
	class processev_crtp {
	protected:
		uint8_t _u8hnd_state_machine;
		tsEvent* _ps_event_context;

	public:
		processev_crtp() : _u8hnd_state_machine(0xFF), _ps_event_context(nullptr) {
			// ステートマシンの登録
			_u8hnd_state_machine = PRSEV_u8RegisterEx((void*)processev_crtp::vProcessEvCore, (void*)this);
			if (_u8hnd_state_machine == 0) {
				MWX_Panic(0, "processev_crtp: can't registered!");
			}
			_ps_event_context = PRSEV_peGetContextH(_u8hnd_state_machine);
		}

		/**
		 * 状態を遷移させる。状態が遷移されるとイベント処理関数の処理後に、もう一度、E_EVENT_NEW_STATE
		 * にてイベント処理関数が呼び出される。
		 */
		inline void PEV_SetState(uint32_t s) {
			PRSEV_vSetState(_ps_event_context, (teState)s);
		}

		/**
		 * 現在の状態に遷移してからの経過時間[ms]を返す。
		 */
		inline uint32_t PEV_u32Elaspsed_ms() {
			return u32TickCount_ms - _ps_event_context->u32tick_new_state;
		}

		/**
		 * スリープ復帰後も状態を維持するフラグをセットする。
		 *   セットしなければ、IDLE 状態からスタートする。
		 *   スリープ復帰後にはフラグはリセットされる。
		 */
		inline void PEV_KeepStateOnWakeup() {
			_ps_event_context->bKeepStateOnSetAll = TRUE;
		}

		/**
		 * イベント処理を強制する
		 */
		inline void PEV_Process(uint32_t ev, uint32_t u32evarg) {
			ToCoNet_Event_ProcessH((teEvent)ev, u32evarg, _u8hnd_state_machine);
		}

		/**
		 * コールドブート(電源投入orリセット)かどうか判定する。
		 */
		inline bool PEV_is_coldboot(uint32_t ev, uint32_t u32evarg) {
			return (ev == E_EVENT_START_UP && u32evarg == 0);
		}

		/**
		 * ウォームブート(スリープ復帰)かどうか判定する。
		 */
		inline bool PEV_is_warmboot(uint32_t ev, uint32_t u32evarg) {
			return (ev == E_EVENT_START_UP && u32evarg != 0);
		}

	public:
		/**
		 * TWENET のコールバック関数。
		 *   ここでは、pvExtraDataより自身のクラスインスタンスを見つけ、
		 *   状態に対応したメンバー関数を呼び出している。
		 */
		static void vProcessEvCore(tsEvent* pEv, teEvent eEvent, uint32 u32evarg) {
			T* derived = static_cast<T*>((processev_crtp*)(pEv->pvExtraData));
			if (derived != nullptr) {
				derived->processStateMachine(pEv, eEvent, u32evarg);
			}
		}
	};
}}
