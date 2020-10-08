/* Copyright (C) 2017 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */

#include "_tweltite.hpp"
#include "mwx_duplicate_checker.hpp"
#include "mwx_debug.h"
const int DEBUG_LVL = 101;

void mwx::L1::duplciate_checker::begin() {
	if(au32Nodes != nullptr) {
		return; // 初期化済み
	}

	if (u8MaxNodes == 0) {
		setup(); // place default parameter.
	}

	au32Nodes = new uint32_t[u8MaxNodes];
	asDupChk = new _node_ele[u8MaxNodes];
	au8TimeStamp = new uint8_t[u8MaxNodes];

	// ハッシュによる高速探索を有効にする場合
	// ただし簡易的なハッシュで (アドレス値の CRC8)&(配列数 - 1) が合致したインデックスに
	// 優先的に格納することで対応する。
	if (u8MaxNodes < 16) {
		this->u8HashMask = 0;
	} else if (u8MaxNodes < 32) {
		this->u8HashMask = (1 << 4) - 1;
	} else if (u8MaxNodes < 64) {
		this->u8HashMask = (1 << 5) - 1;
	} else if (u8MaxNodes < 128) {
		this->u8HashMask = (1 << 6) - 1;
	} else {
		this->u8HashMask = (1 << 7) - 1;
	}

	// 値の初期化
	memset(this->au32Nodes, 0, u8MaxNodes * sizeof(uint32_t));
	for (int i = 0; i < u8MaxNodes; i++) {
		_node_ele_init(&(this->asDupChk[i]));
	}
	memset(this->au8TimeStamp, 0xFF, u8MaxNodes);
}

bool mwx::L1::duplciate_checker::add(uint32_t u32Addr, uint8_t u8Seq) {
	bool_t bRet = FALSE;
	uint16_t u16OldestNode = 0xFFFF;
	uint16_t u16slot = 0xFFFF;
	uint16_t u16BlankNode = 0xFFFF;
	uint8_t u8Hash;
	bool_t bBlank = TRUE;
	int i;

	MWX_DebugMsg(DEBUG_LVL, "\r\n{DUP: INIT %d/to=%dms/sc=%d}"
		, u8MaxNodes
		, TIMEOUT_ms
		, TICK_SCALE
	);
	
	// 最大値を超える u8Seq の場合、下位ビットをマスクする
	u8Seq = u8Seq & (MAX_COUNTS - 1);
	u8Hash = u8HashGen(u32Addr) & u8HashMask;

	// ハッシュ検索を行う
	if (u8HashMask) {
		if (au32Nodes[u8Hash] == u32Addr) {
			u16slot = u8Hash;
			bBlank = FALSE;
		} else
		if (au8TimeStamp[u8Hash] == 0xFF) {
			u16BlankNode = u8Hash;
		}
	}

	// 線形探索で調べてみる(後ろの方はハッシュと関係ない予備エリアになる)
	if (u16slot == 0xFFFF) {
		for (i = u8MaxNodes - 1; i >= 0; i--) {
			if (au32Nodes[i] == u32Addr) {
				u16slot = i;
				bBlank = FALSE;
				break;
			}
		}

		// ここで u16slot が 0xFFFF でないなら見つからなかったので、
		// 格納先を探さないといけない。

		// ハッシュ探索にてハッシュに合致するインデックスが blank の場合は、
		// このスロットを採用する。
		if (u16slot == 0xFFFF && u16BlankNode != 0xFFFF) {
			u16slot = u16BlankNode;
		}

		// ブランクノード、一番最後に利用されたノードの順で採用していく
		if (u16slot == 0xFFFF) {
			uint8 u8tim_last = 0; // 一番最後に受信したタイムスタンプの比較のため
			for (i = u8MaxNodes - 1; i >= 0; i--) {
				if (au8TimeStamp[i] == 0xFF) {
					// 見つかれば即時採用
					u16slot = i;
					break;
				} else {
					// ブランクが無ければ、一番最後に受信したノードを採用する
					uint8 tdif = U8TICK_DIF(au8TimeStamp[i]);
					if (tdif > u8tim_last) {
						u8tim_last = tdif;
						u16OldestNode = i;
					}
				}
			}

			// 新しいノードが来たようだ
			if (u16slot == 0xFFFF) {
				if (u16OldestNode != 0xFFFF) {
					u16slot = u16OldestNode;
				} else {
					u16slot = u8MaxNodes - 1;
				}
			}
		}
	}

	// 格納する
	if (bBlank) {
		_node_ele_init(&asDupChk[u16slot]); // 念のため初期化
	}
	au32Nodes[u16slot] = u32Addr;
	au8TimeStamp[u16slot] = U8TICK(); // タイムスタンプの格納
	bRet = _node_ele_add(&asDupChk[u16slot], u8Seq);

#if 0
	// DEBUG
	{
		const int DEBUG_LVL = 101;
		MWX_DebugMsg(DEBUG_LVL, "\r\n{DUP:<T=%02d,V=%d", U8TICK(), bRet);

		for(int i = 0; i < 8; i++) {
			MWX_DebugMsg(DEBUG_LVL, "|%02X/%02X", asDupChk[u16slot].au8BmDup[i], asDupChk[u16slot].au8TickDupPkt[i]);
		}

		MWX_DebugMsg(DEBUG_LVL, ">}");
	}
#endif

	// ハッシュ値に合わないインデックスに格納した場合はハッシュ値の
	// インデックスとスワップする。次に同じものが来る可能性が高いため。
	if (u16slot != u8Hash) {
		uint32 u32Node = au32Nodes[u16slot];
		_node_ele sEle = asDupChk[u16slot];
		uint8 u8TimeStamp = au8TimeStamp[u16slot];

		au32Nodes[u16slot] = au32Nodes[u8Hash];
		asDupChk[u16slot] = asDupChk[u8Hash];
		au8TimeStamp[u16slot] = au8TimeStamp[u8Hash];

		au32Nodes[u8Hash] = u32Node;
		asDupChk[u8Hash] = sEle;
		au8TimeStamp[u8Hash] = u8TimeStamp;
	}

	return bRet;
}

void mwx::L1::duplciate_checker::clean() {
	int i;

	for (i = 0; i < u8MaxNodes; i++) {
		if (au8TimeStamp[i] != 0xFF) {
			uint8 u8tick = _node_ele_clean(&asDupChk[i]);
			if (u8tick == 0xFF) {
				au8TimeStamp[i] = 0xFF;
				au32Nodes[i] = 0;
			}
		}
	}
}

void mwx::L1::duplciate_checker::_node_ele_init(_node_ele *pDupPkt) {		
	memset(pDupPkt->au8BmDup, 0, sizeof(pDupPkt->au8BmDup));
	memset(pDupPkt->au8TickDupPkt, 0xFF, sizeof(pDupPkt->au8TickDupPkt));
}

uint8_t mwx::L1::duplciate_checker::_node_ele_clean(_node_ele *pDupPkt) {			
	int i;

	uint8_t u8dif_min = 0xFF;
	for (i = 0; i < U8BMLEN; i++) {
		if (!(pDupPkt->au8TickDupPkt[i] & 0x80)) {
			// タイムスタンプが経過したブロックは消去する
			uint8_t u8dif = U8TICK_DIF(pDupPkt->au8TickDupPkt[i]);
			if (u8dif > (TIMEOUT_ms >> TICK_SCALE)) { //  (TIMEOUT_ms/(1 << TICK_SCALE))) {
				pDupPkt->au8BmDup[i] = 0; // ビットマップはクリア
				pDupPkt->au8TickDupPkt[i] = 0xFF; // タイムスタンプは未入力
			} else
			if (u8dif < u8dif_min) {
				u8dif_min = u8dif;
			}
		}
	}

	return u8dif_min;
}

bool mwx::L1::duplciate_checker::_node_ele_add(_node_ele *pDupPkt, uint8_t u8Seq) {
	uint8_t u8ByteOff;
	uint8_t u8BitOff;

	u8ByteOff = u8Seq / 8;
	u8BitOff = 1 << (u8Seq % 8);

	bool_t bRet = pDupPkt->au8BmDup[u8ByteOff] & u8BitOff;
	if (!bRet) {
		pDupPkt->au8BmDup[u8ByteOff] |= u8BitOff;
	} else {
		return true;
	}

#if 0
	// 2020/5/15 連射パケット以外は不確定要素を増すだけなので削除
	/* 古いIDをクリア(u8Seq の値から見て、最も遠いインデックスを消してしまう) 
	   タイムアウトまでにカウンタ(64)が１週回るような連発時を想定 */
	uint8_t u8CleanIdx = (u8ByteOff + U8BMLEN/2) % U8BMLEN;
	pDupPkt->au8BmDup[u8CleanIdx] = 0;
	pDupPkt->au8TickDupPkt[u8CleanIdx] = 0xFF; // タイムスタンプ
#endif

#if 0
	// 対象ブロック、最初の受信タイムスタンプを記録
	if (pDupPkt->au8TickDupPkt[u8ByteOff] & 0x80) { // MSB が１の場合は、値が未入力
		pDupPkt->au8TickDupPkt[u8ByteOff] = U8TICK();
	}
#else
	// 対象ブロック、最後に受信したタイムスタンプを記録する
	pDupPkt->au8TickDupPkt[u8ByteOff] = U8TICK();
#endif

	return false;
}
