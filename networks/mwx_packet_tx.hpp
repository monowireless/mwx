/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */
#pragma once

#include <cstring>
#include <cstdint>
#include "ToCoNet.h"
#include "../mwx_utils_smplbuf.hpp"

namespace mwx { inline namespace L1 {
	struct tx_addr {
		uint32_t _u32addr;
		tx_addr (uint32_t u32addr) : _u32addr(u32addr) {}
	};

	struct tx_addr_broadcast { };

	struct tx_ack_required { };

	struct tx_retry {
		uint8_t _u8retry;
		tx_retry(uint8_t u8count, bool force_retry = false) {
			_u8retry = u8count & 0xF;
			if (force_retry) _u8retry |= 0x80;
		}
	};

	struct tx_packet_type_id {
		uint8_t _u8id;
		tx_packet_type_id(uint8_t u8id) : _u8id(u8id) {}
	};

	struct tx_packet_delay {
		uint16_t _u16DelayMin, _u16DelayMax, _u16RetryDur;
		tx_packet_delay(uint16_t u16DelayMin, uint16_t u16DelayMax, uint16_t u16RetryDur) :
			_u16DelayMin(u16DelayMin), _u16DelayMax(u16DelayMax), _u16RetryDur(u16RetryDur) {}
	};

	struct tx_process_immediate { };

	class packet_tx {
		tsTxDataApp _Tx;
		mwx::smplbuf<uint8_t> _buf;
		uint8_t _b_immediate;
		uint8_t _b_can_handle_request_now;

	public:
		using BUFTYPE = mwx::smplbuf<uint8_t>;

		packet_tx() : _buf(_Tx.auData, 0, MAX_TX_APP_PAYLOAD), _b_immediate(false) {
			memset(&_Tx, 0, sizeof(tsTxDataApp));
			_Tx.u32SrcAddr = ToCoNet_u32GetSerial();
			_Tx.u8CbId = 0xFF;

			_b_can_handle_request_now = ToCoNet_bMaxTxQueueAvailable();
			//MWX_DebugMsg(0, "{packet_tx::C}");
		}

		inline operator bool () {
			return _b_can_handle_request_now;
		}

		BUFTYPE& get_payload() { return _buf; }
		tsTxDataApp* get_psTxDataApp() { return &_Tx; }

		inline packet_tx& operator << (tx_addr&& a) {
			_Tx.u32DstAddr = a._u32addr;
			return *this;
		}

		inline packet_tx& operator << (tx_addr_broadcast&& a) {
			_Tx.u32DstAddr = 0xFFFF;
			return *this;
		}

		inline packet_tx& operator << (tx_ack_required&& a) {
			_Tx.bAckReq = TRUE;
			return *this;
		}

		inline packet_tx& operator << (tx_retry&& a) {
			_Tx.u8Retry = a._u8retry;
			return *this;
		}

		inline packet_tx& operator << (tx_packet_delay&& a) {
			_Tx.u16DelayMin = a._u16DelayMin;
			_Tx.u16DelayMax = a._u16DelayMax;
			_Tx.u16RetryDur = a._u16RetryDur;
			return *this;
		}

		inline packet_tx& operator << (tx_packet_type_id&& a) {
			_Tx.u8Cmd = a._u8id;
			return *this;
		}

		inline packet_tx operator << (tx_process_immediate&& a) {
			_b_immediate = true;
			return *this;
		}

		bool get_tx_process_immediate() { return _b_immediate; }

		// debug out
		template <class T>
		stream<T>& operator >> (stream<T>& strm) {
			auto&& p = get_psTxDataApp();
			strm.println();
			strm.printfmt("TX:ln=%d/cm=%d/cb=%d/s=%08X/d=%X {", p->u8Len, p->u8Cmd, p->u8CbId, p->u32SrcAddr, p->u32DstAddr);
			for (int i = 0; i < p->u8Len; i++) {
				strm.printfmt("%02X", p->auData[i]);
			}
			strm.putchar('}');
			strm.flush();

			strm.printfmt("\r\n    rerty=%d", p->u8Retry);
			strm.printfmt("/ack=%d", p->bAckReq);
			strm.printfmt("/secure=%d", p->bSecurePacket);
			strm.printfmt("/dmax=%d", p->u16DelayMax);
			strm.printfmt("/dmin=%d", p->u16DelayMin);
			strm.printfmt("/dret=%d", p->u16RetryDur);
			strm.printfmt("/epan=%04X", p->u16ExtPan);
			strm.flush();

			return strm;
		}

	};

	template <class T>
	stream<T>& operator << (stream<T>& lhs, packet_tx& rhs) {
		rhs >> lhs;
		return lhs;
	}
}}
