/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */

// C/C++ header

// mwx header
#include "../mwx_debug.h"
#define DEBUG_LVL 99

#include "mwx_nwk_simple.hpp"
#include "../mwx_utils_payload.hpp"

#include "ToCoNet_mod_DuplicateChecker.h"
extern "C" void ToCoNet_vReg_mod_DupChk();

/*****************************************************************/
namespace mwx {
// MUST DEFINE CLASS NAME HERE
#define __MWX_APP_CLASS_NAME NwkSimple
#include "../_mwx_cbs_cpphead.hpp"
/*****************************************************************/

/**
 * @fn	void NwkSimple::_init_dup_check()
 *
 * @brief	initialize the duplicate checker (memory allocation)
 *
 */
void NwkSimple::_init_dup_check() {
	
}

/**
 * @fn	void NwkSimple::on_create()
 *
 * @brief	When created, it's called (just after constructor).
 * 			note: this is automatically called at the_lite.network.use<>() procedure.
 * 			
 * 			initialize the object in the case begin() call is omitted.
 *
 */
void NwkSimple::on_create(uint32_t& val) {
	_config.u8Cmd = 0x7; // default cmd id
	_config.u8Lid = 0xFE; // no id child
	_config.u8RepeatMax = 2; // transmit same packet three times
	_config.u8Type = 1; // default type

	_dupchk.setup(); // apply the default parameter.
	
	the_twelite.change_short_addr(_config.u8Lid);

	MWX_DebugMsg(DEBUG_LVL, "{NwkSimple::on_create}");
}

/**
 * @fn	void NwkSimple::on_begin()
 *
 * @brief	called when the_twelite.begin() statement.
 * 			initialize the object in the case begin() call is omitted.
 *
 */
void NwkSimple::on_begin(uint32_t& val) {
	if (val == _TWENET_CALLED_FROM_TWE_TWELITE) { // called from the_twelite.begin().
		_dupchk.begin(); // start duplicate checker
		the_twelite.change_short_addr(_config.u8Lid);
		MWX_DebugMsg(DEBUG_LVL, "{NwkSimple::on_begin}");
	}
}


/**
 * @fn	void NwkSimple::begin()
 *
 * @brief	Begins this object
 *
 */
void NwkSimple::begin() {
	uint32_t val = 0;
	on_begin(val);
}

/**
 * @fn	int NwkSimple::transmit(mwx::packet_tx& pkt)
 *
 * @brief	Transmits the given packet
 * 			
 *
 * @param [in,out]	pkt	The packet.
 *
 * @returns	An int.
 */
MWX_APIRET NwkSimple::transmit(packet_tx_nwk_simple<NwkSimple>& pkt) {
	MWX_APIRET ret;

	if (_config.u8Type == 0x01) {
		// packet structure
		// uint8_t   packet type (0x01: simple)
		// if 0x01,
		// uint32_t  source address (long)
		// uint32_t  destination address (0x00000000-0x000000ff: LID, 0x80000000: Long)
		// uint8_t   repeat count
		
		{
			MWX_DebugMsg(DEBUG_LVL, "\r\n{TX ln=%d, %X/", pkt.get_payload().size(), (void*)pkt.get_payload().begin());
			for (auto&& x : pkt.get_payload()) {
				MWX_DebugMsg(DEBUG_LVL, "%02X", x);
			}
			MWX_DebugMsg(DEBUG_LVL, "}");
		}

		// take back reserved header area.
		pkt.get_payload().reserve_head(-_get_header_size());

		uint8_t* p = pkt.get_payload().begin();
		uint8_t* e = p + _get_header_size();

		mwx::pack_bytes(p, e,
			uint8_t(0x01),
			uint8_t(_config.u8Lid & 0xFF),
			uint32_t(the_twelite.get_hw_serial()),
			uint32_t(pkt.get_psTxDataApp()->u32DstAddr),
			uint8_t(0x00)
		);

		{
			MWX_DebugMsg(DEBUG_LVL, "\r\nln=%d, %X/", pkt.get_payload().size(), (void*)pkt.get_payload().begin());
			for (auto&& x : pkt.get_payload()) {
				MWX_DebugMsg(DEBUG_LVL, "%02X", x);
			}
		}

		// other header information
		pkt.get_psTxDataApp()->u32DstAddr = 0x0000FFFF; // set broadcast
		pkt.get_psTxDataApp()->u8Len = pkt.get_payload().size();
		pkt.get_psTxDataApp()->u8Cmd = _config.get_pkt_cmd_part();

		// callback id
		pkt.get_psTxDataApp()->u8CbId = ++_u8cbid & CBID_MASK;

		// set secure flag
		if(_config.is_mode_pkt_encrypt()) {
			pkt.get_psTxDataApp()->bSecurePacket = 1;
		}

		// debug!
		// pkt >> Serial;
		
		// do transmit
		ret = the_mac.transmit(pkt);
		MWX_DebugMsg(DEBUG_LVL, "\r\nthe_mac.transmit = %08X", uint32_t(ret));
	}
	
	return ret;
}

void NwkSimple::receive(mwx::packet_rx& rx) {
	// firstly, duplicate checking.
	uint8_t u8seq = rx.get_psRxDataApp()->u8Seq;

	uint8_t u8Type = 0;
	uint32_t u32AddrSrc = 0;
	uint8_t u8AddrSrc_Lid = 0;
	uint32_t u32AddrDst = 0;
	uint8_t u8Rpt = 0;

	rx._network_type = _config.u8Type;
	rx._network_handled = 1;

	if (_config.u8Type == 0x01) {
		// if cmd is not _config.u8cmd, skip
		if (rx.get_psRxDataApp()->u8Cmd != _config.get_pkt_cmd_part()) {
			return;
		}

		// check the payload and extract header content.
		if (rx.get_length() <= _get_header_size()) {
			return; // length check (has header block)
		}

		expand_bytes(
			rx.get_payload().begin(), rx.get_payload().end(),
			u8Type, u8AddrSrc_Lid, u32AddrSrc, u32AddrDst, u8Rpt);

		// ignore the packet from the self address. (reject repeat packet sent by itself.)
		if (	u32AddrSrc == the_twelite.get_hw_serial()
			||	(u8AddrSrc_Lid < 0xFE && u8AddrSrc_Lid == _config.u8Lid) ) {
			return;
		}

		// check duplicate
		bool_t bDup = _dupchk.add(u32AddrSrc, u8seq & 0x7f);
		if (bDup) {
			MWX_DebugMsg(DEBUG_LVL, "<dup=%d>", int(u8seq & 0x7f));
			return;
		}
		
		// now accepting the packet!
		tsRxDataApp* pRx = (tsRxDataApp*)(rx.get_psRxDataApp());
		MWX_DebugMsg(DEBUG_LVL, "{RX: %d:S=%X(%02x):D=%X:r=%d:Sq=%d}", u8Type, u8AddrSrc_Lid, u32AddrSrc, u32AddrDst, u8Rpt, pRx->u8Seq);

		// secure check
		{
			bool b_secure_check = false;

			// check secure packet or not
			if (_config.is_mode_pkt_encrypt()) {
				if (pRx->bSecurePkt) {
					b_secure_check = true;
				} else {
					if (_config.is_mode_rcv_plain()) {
						b_secure_check = true;
					}
				}
			} else {
				b_secure_check = true;
			}

			if (!b_secure_check) {
				MWX_DebugMsg(DEBUG_LVL, "{REJECTED PLAIN PACKET UNDER SECURE MODE}");
				return; // reject packet...
			}
		}

		// packet destination check		
		bool b_accept = false;
		bool b_repeat = true;

		// check if the packet is to the destination.
		if (u32AddrDst == 0xFE && _config.u8Lid != 0x00) { // child broad cast
			b_accept = true;
		} else
		if (u32AddrDst == 0xFF) { // broad casting.
			b_accept = true;
		}
		else if (u32AddrDst < 0xFE && u32AddrDst == _config.u8Lid) { // packet destination.
			b_repeat = false;
			b_accept = true;
		}
		else if (u32AddrDst == the_twelite.get_hw_serial()) { // packet destination.
			b_repeat = false;
			b_accept = true;
		}

		// if Lid == 0x00(should be parent device), packet sent by myself, no repeat.
		if (_config.u8Lid == 0x00 || u32AddrSrc == the_twelite.get_hw_serial()) {
			b_repeat = false;
		}

		// if repeat life expires, no more repeating.
		if ((_config.u8RepeatMax == 0) && (u8Rpt & 0x7F) >= _config.u8RepeatMax) {
			b_repeat = false;
		}

		// repeat the packet.
		if (b_repeat) {
			mwx::packet_tx pkt_rpt;

			MWX_DebugMsg(DEBUG_LVL, "{RPT: S=%X CT=%d}", u32AddrSrc, u8Rpt);

			// copy payload
			for (auto&& x : rx.get_payload()) {
				pkt_rpt.get_payload().push_back(x);
			}
			rx.get_payload()[10]++; // increment repeat count

			// set sending parameter
			pkt_rpt
				<< tx_addr_broadcast()
				<< tx_retry(0x2)
				<< tx_packet_delay(30, 100, 20);

			pkt_rpt.get_psTxDataApp()->u8Seq = rx.get_psRxDataApp()->u8Seq;
			pkt_rpt.get_psTxDataApp()->u8Cmd = _config.get_pkt_cmd_part();

			// secure pkt
			if (_config.is_mode_pkt_encrypt()) {
				pkt_rpt.get_psTxDataApp()->bSecurePacket = 1; // set encryption.
			}

			// call back id
			pkt_rpt.get_psTxDataApp()->u8CbId = (++_u8cbid_rpt & CBID_MASK) + (CBID_MASK + 1); // set Higher bit for repeating packet.

			the_mac.transmit(pkt_rpt);
		}

		// if the module is packet destination, pass the packet to the app.
		if (b_accept) {
			pRx->u32DstAddr = u32AddrDst;
			pRx->u32SrcAddr = u32AddrSrc;
			pRx->u8Hops = u8Rpt;

			rx._set_addr_src_lid(u8AddrSrc_Lid);

			rx.get_payload().attach(pRx->auData + _get_header_size(), pRx->u8Len - _get_header_size(), pRx->u8Len - _get_header_size());
			rx._network_handled = 0;
		}
	}
}

MWX_STATE(E_MWX::STATE_0, uint32_t eEvent, uint32_t u32evarg) {
	if (eEvent == E_EVENT_TICK_SECOND) {
		MWX_DebugMsg(DEBUG_LVL, "{DUP: Clean}");
		_dupchk.clean();
	}
}

/*****************************************************************/
// common procedure (DO NOT REMOVE)
#include "../_mwx_cbs_cpptail.cpp"
// MUST UNDEF CLASS NAME HERE
#undef __MWX_APP_CLASS_NAME
} // mwx
/*****************************************************************/


