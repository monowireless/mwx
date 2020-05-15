/* Copyright (C) 2017 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */

#pragma once

#include <cstring>
#include <jendefs.h>
#include <AppHardwareApi.h>

// TWENET C Library
#include "ToCoNet.h"

// mwx common def
#include "mwx_common.hpp"

namespace mwx { inline namespace L1 {

	/**
	 * Duplicate checker by sequence id and timestamp.
	 */
	class duplciate_checker {
	public:
		/**
		 * ビットマップのビット数（64なら 0..63のシーケンス番号の重複を管理する）
		 */
		static const uint8_t MAX_COUNTS = 64;

		/**
		 * ビットマップのバイト数
		 */
		static const uint8_t U8BMLEN = MAX_COUNTS/8;

		/**
		 * 8bit のティックカウントの計算 (32ms=1カウント)
		 */
		inline uint8_t U8TICK() { return ((u32TickCount_ms >> TICK_SCALE) & 0x7F); }

		/**
		 * 8bit のティックカウントの差分計算
		 */
		inline uint8_t U8TICK_DIF(uint8_t t) { return ((U8TICK() - (t))&0x7F); }

		/**
		 * 重複チェックのノード定義構造体
		 */
		typedef struct {
			uint8_t au8BmDup[MAX_COUNTS/8]; //!< Seq番号に対応する64bit分のビットマップ
			uint8_t au8TickDupPkt[MAX_COUNTS/8]; //!< 分割したブロック内で最初に受信したシステム時刻 1カウント=32ms
		} _node_ele;

		void debugout(uint32_t u32node);

	private:
		uint32_t *au32Nodes; // ノード情報（ハッシュテーブルで管理）
		_node_ele *asDupChk; // ノード情報
		uint8_t *au8TimeStamp; // 一番最近受信したタイムスタンプ（現時刻との差）
		uint8_t u8MaxNodes; // 最大管理ノード数
		uint8_t u8HashMask;

		uint8_t TICK_SCALE; // 時間単位 (5 なら 2^5=32msを１単位として管理, au8TimeStampにて8bitで管理)
		uint16_t TIMEOUT_ms; // 抹消するためのタイムアウト

	public:
		duplciate_checker() {
			// 乱暴だが、自身をゼロクリア
			memset((void*)this, 0, sizeof(duplciate_checker));
		}

		/**
		 * 事前の設定
		 */
		void setup(uint8_t maxnodes = 16, uint16_t timeout = 0, uint8_t tickscale = 0) {
			u8MaxNodes = maxnodes ? maxnodes : 16;
			TIMEOUT_ms = timeout ? timeout : 5000; // デフォルトのタイムアウトは５秒、秒10-12パケット程度まで許容できる。
			if (TIMEOUT_ms > 50000) TIMEOUT_ms = 50000;

			// tickscaleはブロックをタイムアウトするための時間スケーラ (6 なら 2^6=64ms 刻みで管理する)
			// 最大値は 127 だが１秒おきのタイムアウトチェックまでのカウンタの空走分(64msで16カウント)を
			// 加味してTIMEOUT_msは100カウント以内に収まるように設定する。
			if ((1 << tickscale) * 100 > TIMEOUT_ms) {
				TICK_SCALE = tickscale;
			} else {
				// 時間の刻みは 64ms から 512ms
				for (int i = 6; i < 10; i++) {
					if ((1 << i) * 100 > TIMEOUT_ms) {
						TICK_SCALE = i;
						break;
					} 
				}
			}
		}
		
		/**
		 * 初期化関数
		 */
		void begin();

		/**
		 * パケット受信時のシーケンス番号を投入し、重複の確認を行う。
		 * - アドレス値を検索するアルゴリズム
		 *   - ハッシュ値の示すスロットに該当アドレスデータが登録されるか探索する
		 *   - ハッシュ値のスロットに該当アドレスデータが無ければ、全探索で登録データを探索する
		 *     - 登録データが見つかった場合は、そのスロットにデータ投入する
		 *     - 登録データが見つからない場合は、新規スロットを探索する
		 *   - 新規スロットは以下の順で採用する
		 *     - ハッシュ値の示すスロット
		 *     - 空きスロット(ハッシュ値で利用されない配列の後ろを優先して利用)
		 *     - もっとも最後に値が投入されたスロット
		 *   - 最後に、ハッシュ値と異なるスロットに登録した場合、ハッシュ値スロットのデータと
		 *     スワップを行う (データは近い時刻で集中するはずである)
		 *
		 * @param u32addr アドレス
		 * @param u8Seq シーケンス番号
		 * @return 重複していると TRUE
		 */
		bool add(uint32_t u32Addr, uint8_t u8Seq);

		/** 
		 * 各スロットのタイムアウト処理を行う。空になったノードは未使用領域にする。
		 *
		 */
		void clean();

		/**
		 * 重複パケット除去アルゴリズムの初期化
		 *
		 * @param pDupPkt 管理構造体
		 */
		void _node_ele_init(_node_ele *pDupPkt);

		/**
		 * パケット重複のクリーンナップを行う。
		 * - タイムアウトしたビットマップの一部をクリア
		 *
		 * @param pDupPkt
		 * @return 最新のパケット受信カウント, ブランクなら 0xFF が返る
		 */
		uint8_t _node_ele_clean(_node_ele *pDupPkt);

		/**
		 * パケット受信時のシーケンス番号を投入し、重複の確認を行う。
		 * - 重複でなければ、内部のビットマップに値を格納する
		 * - 重複していれば、FALSE を返す
		 * - 受け取ったパケットのシーケンス番号から遠い番号のビットマップを消去する
		 *   - 注：本アルゴリズムのため、殆ど届かず稀に１周した番号のみが到達する様な
		 *         場合は重複パケットとして処理される可能性が有る。
		 *
		 * @param pDupPkt 管理構造体
		 * @param u8Seq シーケンス番号
		 * @return 重複していると TRUE
		 */
		bool _node_ele_add(_node_ele *pDupPkt, uint8_t u8Seq);

		/**
		 * uint32 から 8bit の XOR によるハッシュを作成する。
		 */
		static inline uint8 u8HashGen(uint32_t u32d) {
			uint32_t t = u32d;
			t ^= (u32d >> 16);
			t ^= (t >> 8);

			return t & 0xff;
		}
	};
}}
