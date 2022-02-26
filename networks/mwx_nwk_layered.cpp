/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */

// C/C++ header

// mwx header
#include "../mwx_debug.h"
#define DEBUG_LVL 99 // 0

#if DEBUG_LVL < 99
# warning "DEBUG LEVEL IS SET"
#endif

#include "mwx_nwk_layered.hpp"
#include "../mwx_utils_payload.hpp"

#include "ToCoNet_mod_DuplicateChecker.h"

 //!< モジュールの登録関数
extern "C" void ToCoNet_vReg_mod_DupChk();
extern "C" void ToCoNet_vReg_mod_NbScan();
extern "C" void ToCoNet_vReg_mod_Nwk_LayerTree();
extern "C" void ToCoNet_vReg_mod_NbScan_Slave();
extern "C" void _ToCoNet_Mod_vReg_PRSEV();

extern "C" tsToCoNet_Nwk_Context* ToCoNet_NwkLyTr_psConfig(tsToCoNet_NwkLyTr_Config *_psConf);

/*****************************************************************/
namespace mwx {
// MUST DEFINE CLASS NAME HERE
#define __MWX_APP_CLASS_NAME NwkLayered
#include "../_mwx_cbs_cpphead.hpp"
/*****************************************************************/

/**
 * @fn	void NwkSimple::on_create()
 *
 * @brief	When created, it's called (just after constructor).
 * 			note: this is automatically called at the_lite.network.use<>() procedure.
 * 			
 * 			initialize the object in the case begin() call is omitted.
 *
 */
void NwkLayered::on_create(uint32_t& val) {
	_config.u8Cmd = 0x0; // default cmd id
	_config.u8Type = 0x80; // default type
	_config.u8Role = TOCONET_NWK_ROLE_PARENT;

	// register TWENET modules
	ToCoNet_vReg_mod_DupChk();
	ToCoNet_vReg_mod_NbScan();
	ToCoNet_vReg_mod_NbScan_Slave();
	ToCoNet_vReg_mod_Nwk_LayerTree();
	_ToCoNet_Mod_vReg_PRSEV();  // MUST call this when ToCoNet_vReg_mod_xxx() is called manually.
	
	//the_twelite.change_short_addr(_config.u8Lid);

	MWX_DebugMsg(DEBUG_LVL, "{NwkLayered::on_create}");
}

/**
 * @fn	void NwkSimple::on_begin()
 *
 * @brief	called when the_twelite.begin() statement.
 * 			initialize the object in the case begin() call is omitted.
 *
 */
void NwkLayered::on_begin(uint32_t& val) {
	if (val == _TWENET_CALLED_FROM_TWE_TWELITE) { // called from the_twelite.begin().
		// so far, init as parent device
		_sNwkLayerTreeConfig.u8Layer = 0;
		_sNwkLayerTreeConfig.u8Role = TOCONET_NWK_ROLE_PARENT;
		//_sNwkLayerTreeConfig.u8StartOpt = TOCONET_MOD_LAYERTREE_STARTOPT_NB_BEACON; // so far, it should not be beacon node master.
		//_sNwkLayerTreeConfig.u8Second_To_Beacon = TOCONET_MOD_LAYERTREE_DEFAULT_BEACON_DUR;

		// configures the network
		_pContextNwk = ToCoNet_NwkLyTr_psConfig(&_sNwkLayerTreeConfig);

		// start the network
		if (_pContextNwk) {
			ToCoNet_Nwk_bInit(_pContextNwk);
			ToCoNet_Nwk_bStart(_pContextNwk);
		}
	}
}


/**
 * @fn	void NwkSimple::begin()
 *
 * @brief	Begins this object
 *
 */
void NwkLayered::begin() {
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
MWX_APIRET NwkLayered::transmit(packet_tx_nwk_layered<NwkLayered>& pkt) {
	// other header information
	pkt.get_psTxDataApp()->u32SrcAddr = ToCoNet_u32GetSerial();
	// if (pkt.get_psTxDataApp()->u32DstAddr = 0xFFFF) pkt.get_psTxDataApp()->u32DstAddr = TOCONET_NWK_ADDR_BROADCAST; // broadcast
	pkt.get_psTxDataApp()->u8Len = pkt.get_payload().size();
	// pkt.get_psTxDataApp()->u8Cmd = TOCONET_PACKET_CMD_APP_DATA; // already set

	// callback id
	uint8_t u8cbid = the_mac.request_cbid_nwk();
	pkt.get_psTxDataApp()->u8CbId = u8cbid;

	// set secure flag
	if(_config.is_mode_pkt_encrypt()) {
		pkt.get_psTxDataApp()->bSecurePacket = 1;
	}

	// transmit
	bool b_tx = (bool)ToCoNet_Nwk_bTx(_pContextNwk, pkt.get_psTxDataApp());
	if(b_tx) {
		// places cbid to the MAC layer management.
		the_mac.tx_status._tx_request(u8cbid);
	}
	
	// returns state and cbid
	return MWX_APIRET(b_tx, u8cbid);
}

void NwkLayered::receive(mwx::packet_rx& rx) {
	// packet struct from TWENET C layer
	tsRxDataApp* pRx = (tsRxDataApp*)(rx.get_psRxDataApp());

	//MWX_DebugMsg(0, "<!NwkLayered::receive! nw=%d>", pRx->bNwkPkt);

	rx._network_handled = false; // true: for this network class, false: not (networkless or another)

	if (pRx->bNwkPkt) {
		rx._network_type = NETWORK::LAYERED; // set layered
		rx._network_handled = true; // for this class
	}

	return;
}

MWX_STATE(E_MWX::STATE_0, uint32_t eEvent, uint32_t u32evarg) {
	if (eEvent == E_EVENT_TICK_SECOND) {
	}
}

/*****************************************************************/
// common procedure (DO NOT REMOVE)
#include "../_mwx_cbs_cpptail.cpp"
// MUST UNDEF CLASS NAME HERE
#undef __MWX_APP_CLASS_NAME
} // mwx
/*****************************************************************/
