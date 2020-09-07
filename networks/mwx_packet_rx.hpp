/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */
#pragma once
#include "ToCoNet.h"

#include "../mwx_utils_smplbuf.hpp"
#include "../mwx_debug.h"

namespace mwx { inline namespace L1 {
	class packet_rx {
		const tsRxDataApp* _pRx;
		mwx::smplbuf<uint8_t> _buf;

		uint8_t _addr_src_lid; // source address (logical id)
		uint8_t _len;

	public:
		uint8_t _network_type;
		uint8_t _network_handled;

	public:
		using BUFTYPE = mwx::smplbuf<uint8_t>;

		packet_rx(const tsRxDataApp* pRx, uint8_t nwk_type = 0) :
			_pRx(pRx),
			_buf(pRx->auData, pRx->u8Len, MAX_RX_APP_PAYLOAD),
			_addr_src_lid(0xFF), _len(0),
			_network_type(nwk_type), _network_handled(0) {}

		packet_rx() : _pRx(nullptr), _buf() {}

		BUFTYPE& get_payload() { return _buf; }
		const tsRxDataApp* get_psRxDataApp() { return _pRx; }
		void _set_psRxDataApp(tsRxDataApp* pRx) { _pRx = pRx; }

		uint8_t get_length() { return _buf.size(); }
		uint8_t get_lqi() { return _pRx == nullptr ? 0 : _pRx->u8Lqi; }

		uint32_t get_addr_src_long() { return _pRx->u32SrcAddr; }
		uint32_t get_addr_dst() { return _pRx->u32DstAddr; }
		uint8_t get_addr_src_lid() { return _addr_src_lid; }
		void _set_addr_src_lid(uint8_t a) { _addr_src_lid = a; }

		bool is_secure_pkt() { return _pRx->bSecurePkt; }

	public:
		uint8_t _get_network_type() { return _network_type; }

	public:
		// debug out
		template <class T>
		stream<T>& operator >> (stream<T>& strm) {
			auto p = get_psRxDataApp();

			strm.println();
			strm.printfmt("RX:ln=%d/cm=%d/lq=%d/s=%08X/d=%X {", p->u8Len, p->u8Cmd, p->u8Lqi, p->u32SrcAddr, p->u32DstAddr);
			for (auto&& x : _buf) {
				strm.printfmt("%02X", x);
			}
			strm.putchar('}');
			strm.flush();

			strm.printfmt("\r\n    ");
			strm.printfmt("/secure=%d", p->bSecurePkt);
			strm.printfmt("/nwk=%d", p->bNwkPkt);
			strm.printfmt("/epan=%04X", p->u16ExtPan);
			strm.printfmt("/hops=%d", p->u8Hops);
			strm.printfmt("/seq=%d", p->u8Seq);
			strm.flush();

			return strm;
		}
	};

	template <class T>
	stream<T>& operator << (stream<T>& lhs, packet_rx& rhs) {
		rhs >> lhs;
		return lhs;
	}
}}
