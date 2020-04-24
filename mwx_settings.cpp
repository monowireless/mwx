/* Copyright (C) 2020 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */

#include "mwx_settings.hpp"

// TWELITE hardware like
#include <jendefs.h>
#include <ToCoNet.h>
#include <AppHardwareApi.h>

#include "tweinteractive.h"
#include "tweinteractive_defmenus.h"
#include "tweinteractive_settings.h"
#include "tweinteractive_nvmutils.h"
#include "twenvm.h"
#include "twesysutils.h"

/**********************************************************************************
 * DEFINES
 **********************************************************************************/
#define STG_SAVE_BUFF_SIZE 64		//! セーブバッファサイズ

#define STGS_SET_VER 0x01			//! 設定バージョン
#define STGS_SET_VER_COMPAT 0x01	//! 互換性のある設定バージョン

#define STGS_MAX_SETTINGS_COUNT 16	//! 設定数の最大(確定設定リスト tsFinal の配列数を決める)

/* twesettings 関連 */
#define TWESTG_SLOT_DEFAULT 0       //! デフォルトのスロット番号
#define STGS_KIND_APPDEF 0x00		//! 親機用設定
#define STGS_KIND_APPDEF_SLOT_MAX 8 //! 親機は SLOT0 のみ

#define STGS_KIND_MAX 3				//! 種別数の最大

#define MENU_CONFIG 1 // 設定モード

// Application Specific
#define APPVER				((VERSION_MAIN << 24) | (VERSION_SUB << 16) | (VERSION_VAR << 8))

/**********************************************************************************
 * FUNCTION PROTOTYPES
 **********************************************************************************/
static void s_appLoadData(uint8 u8kind, uint8 u8slot, bool_t bNoLoad);

/**********************************************************************************
 * CONSTANTS
 **********************************************************************************/

/**********************************************************************************
 * EXTERNS
 **********************************************************************************/
extern "C" TWE_tsFILE* _psSerial;

extern "C" void INTRCT_USER_vProcessInputByte(TWESERCMD_tsSerCmd_Context *pSerCmd, int16 u16Byte) __attribute__((weak));
extern "C" TWE_APIRET TWEINTRCT_cbu32GenericHandler(TWEINTRCT_tsContext *pContext, uint32 u32Op, uint32 u32Arg1, uint32 u32Arg2, void *vpArg);
extern "C" TWE_APIRET TWESTG_cbu32LoadSetting(TWE_tsBuffer *pBuf, uint8 u8kind, uint8 u8slot, uint32 u32Opt, TWESTG_tsFinal *psFinal);
extern "C" TWE_APIRET TWESTG_cbu32SaveSetting(TWE_tsBuffer *pBuf, uint8 u8kind, uint8 u8slot, uint32 u32Opt, TWESTG_tsFinal *psFinal);

/**********************************************************************************
 * VARIABLES
 **********************************************************************************/
/*!
 * tsFinal 構造体のデータ領域を宣言する
 */
TWESTG_DECLARE_FINAL(MYDAT, STGS_MAX_SETTINGS_COUNT, 16, 4); // 確定設定リストの配列等の宣言

/*!
 * 確定設定リスト
 */
TWESTG_tsFinal sFinal;

/**
 * @brief 種別の保存
 */
static uint8 u8AppKind = STGS_KIND_APPDEF;

/**
 * @brief 現在のスロットの保存
 */
static uint8 u8AppSlot = 0;

/**
 * @brief メニューモード
 *   MSB が１はインタラクティブモード終了。
 */
static uint8 u8MenuMode = 0;

/*!
 * 設定定義(tsSettings)
 *   スロット0..7までの定義を記述
 */
const TWESTG_tsSettingsListItem SetList[1][9] = {
	{
		{ STGS_KIND_APPDEF, TWESTG_SLOT_DEFAULT, { INTRCT_USER_BASE_SETTINGS, NULL, INTRCT_USER_SLOT0_SETTINGS, NULL, NULL, NULL } },
		{ STGS_KIND_APPDEF, TWESTG_SLOT_1,       { INTRCT_USER_BASE_SETTINGS, NULL, INTRCT_USER_SLOT1_SETTINGS, NULL, NULL, NULL } },
		{ STGS_KIND_APPDEF, TWESTG_SLOT_2,       { INTRCT_USER_BASE_SETTINGS, NULL, INTRCT_USER_SLOT2_SETTINGS, NULL, NULL, NULL } },
		{ STGS_KIND_APPDEF, TWESTG_SLOT_3,       { INTRCT_USER_BASE_SETTINGS, NULL, INTRCT_USER_SLOT3_SETTINGS, NULL, NULL, NULL } },
		{ STGS_KIND_APPDEF, TWESTG_SLOT_4,       { INTRCT_USER_BASE_SETTINGS, NULL, INTRCT_USER_SLOT4_SETTINGS, NULL, NULL, NULL } },
		{ STGS_KIND_APPDEF, TWESTG_SLOT_5,       { INTRCT_USER_BASE_SETTINGS, NULL, INTRCT_USER_SLOT5_SETTINGS, NULL, NULL, NULL } },
		{ STGS_KIND_APPDEF, TWESTG_SLOT_6,       { INTRCT_USER_BASE_SETTINGS, NULL, INTRCT_USER_SLOT6_SETTINGS, NULL, NULL, NULL } },
		{ STGS_KIND_APPDEF, TWESTG_SLOT_7,       { INTRCT_USER_BASE_SETTINGS, NULL, INTRCT_USER_SLOT7_SETTINGS, NULL, NULL, NULL } },
		{ TWESTG_KIND_VOID, TWESTD_SLOT_VOID, { NULL }}, // TERMINATE
	},
};

/**
 * @brief 設定リストのメニュー名
 */
const uint8 SetList_names[][8] = { "*", };

/**
 * @brief メニュー定義
 */
const TWEINTRCT_tsFuncs asFuncs[] = {
	{ 0, (uint8*)"ROOT MENU", TWEINTCT_vSerUpdateScreen_defmenus, TWEINTCT_vProcessInputByte_defmenus, TWEINTCT_vProcessInputString_defmenus, TWEINTCT_u32ProcessMenuEvent_defmenus }, // standard _settings
	{ 1, (uint8*)"CONFIG MENU", TWEINTCT_vSerUpdateScreen_settings, TWEINTCT_vProcessInputByte_settings, TWEINTCT_vProcessInputString_settings, TWEINTCT_u32ProcessMenuEvent_settings }, // standard _settings
	{ 2, (uint8*)"EEPROM UTIL", TWEINTCT_vSerUpdateScreen_nvmutils, TWEINTCT_vProcessInputByte_nvmutils, TWEINTCT_vProcessInputString_nvmutils, TWEINTCT_u32ProcessMenuEvent_nvmutils }, // standard _settings
	{ 0xFF, NULL, NULL, NULL }
};

/**********************************************************************************
 * STATIC FUNCTIONS
 **********************************************************************************/
/*!
 * セーブ用に最初のセクタを計算する。
 * 
 * \param u8kind 種別
 * \param u8slot スロット
 * \param u32Opt オプション
 * \return 0xFF:error, その他:セクタ番号
 */
static uint8 s_u8GetFirstSect(uint8 u8kind, uint8 u8slot) {
	uint8 u8sec = 1; // 最初のセクターは飛ばす
	if (u8kind == STGS_KIND_APPDEF) {
		if (u8slot < STGS_KIND_APPDEF_SLOT_MAX) {
			u8sec = 1 + u8slot * 2; // parent は 1-2 (1block)
		} else {
			u8sec = 0xFF;
		}
	}
	if (u8sec > EEPROM_6X_USER_SEGMENTS) u8sec = 0xFF;

	return u8sec;
}

// ヘッダデータをセーブする (kind, slot の保存)
static TWE_APIRET TWESTG_cbu32SaveHeader(uint8 u8kind, uint8 u8slot) {
	uint8_t b_header[2] = { u8kind, u8slot };

	TWE_tsBuffer buf;
	buf.pu8buff = b_header;
	buf.u8bufflen = sizeof(b_header);
	buf.u8bufflen_max = buf.u8bufflen;

	bool_t bRes = TWENVM_bWrite(&buf, 0);

	return bRes ? TWE_APIRET_SUCCESS : TWE_APIRET_FAIL;
}

// ヘッダデータをロードする (kind, slot の読み出し)
static TWE_APIRET TWESTG_cbu32LoadHeader(uint8* pu8kind, uint8* pu8slot) {
	uint8_t b_header[2];
	
	TWE_tsBuffer buf;
	buf.pu8buff = b_header;
	buf.u8bufflen = sizeof(b_header);
	buf.u8bufflen_max = buf.u8bufflen;

	bool_t bRes = TWENVM_bRead(&buf, 0); 

	if (pu8kind) {
		*pu8kind = bRes && (buf.pu8buff[0] == STGS_KIND_APPDEF)
				? buf.pu8buff[0] : STGS_KIND_APPDEF; 
	}

	if (pu8slot) {
		*pu8slot = bRes && (buf.pu8buff[1] < STGS_KIND_APPDEF_SLOT_MAX)
				? buf.pu8buff[1] : 0;
	}

	return bRes ? TWE_APIRET_SUCCESS : TWE_APIRET_FAIL;
}

/*!
 * 確定設定データを再構築する。
 * 
 * \param u8kind  種別
 * \param u8slot  スロット
 * \param bNoLoad TRUEならslotに対応するセーブデータを読み込まない。
 */
static void s_appLoadData(uint8 u8kind, uint8 u8slot, bool_t bNoLoad) {
	// EEPROM の SLOT0 の値を読み出す
	if (u8kind == 0xFF || u8slot == 0xFF) {
		TWESTG_cbu32LoadHeader(&u8kind, &u8slot);
		// TWE_fprintf(_psSerial, "[LOAD HEADER %d/%d]", u8kind, u8slot);
	}

	// 値のチェック
	bool_t bOk = FALSE;
	if (u8kind == STGS_KIND_APPDEF && u8slot < STGS_KIND_APPDEF_SLOT_MAX) bOk = TRUE;
	if (!bOk) {
		return;
	}

	/// tsFinal 構造体の初期化とロード
	// tsFinal 構造体の初期化
	TWESTG_INIT_FINAL(MYDAT, &sFinal);
	// tsFinal 構造体に基本情報を適用する
	TWESTG_u32SetBaseInfoToFinal(&sFinal, INTRCT_USER_APP_ID, APPVER, STGS_SET_VER, STGS_SET_VER_COMPAT);
	// tsFinal 構造体に kind, slot より、デフォルト設定リストを構築する
	TWESTG_u32SetSettingsToFinal(&sFinal, u8kind, u8slot, SetList[u8kind]);
	// セーブデータがあればロードする
	TWESTG_u32LoadDataFrAppstrg(&sFinal, u8kind, u8slot, INTRCT_USER_APP_ID, STGS_SET_VER_COMPAT, bNoLoad ? TWESTG_LOAD_OPT_NOLOAD : 0);

	// グローバル変数に書き戻し
	u8AppSlot = u8slot;
	u8AppKind = u8kind;
}

/**********************************************************************************
 * INTERFACE FUNCTIONS (twesettings)
 **********************************************************************************/
/*!
 * データセーブを行う。
 * twesettings ライブラリから呼び出されるコールバック関数。
 * 
 * \param pBuf   データ領域 pBuf->pu8buff[-16..-1] を利用することができる。
 * \param u8kind 種別
 * \param u8slot スロット
 * \param u32Opt オプション
 * \param ...
 * \return TWE_APIRET
 */
TWE_APIRET TWESTG_cbu32SaveSetting(TWE_tsBuffer *pBuf, uint8 u8kind, uint8 u8slot, uint32 u32Opt, TWESTG_tsFinal *psFinal) {
	// 現在のKINDとSLOTをsector0に保存する。
	TWESTG_cbu32SaveHeader(u8kind, u8slot);

	uint8 u8sect = s_u8GetFirstSect(u8kind, u8slot);
	if (u8sect != 0xFF) {
		bool_t bRes = TWENVM_bWrite(pBuf, u8sect); //先頭セクターはコントロールブロックとして残し、2セクター単位で保存
		return bRes ? TWE_APIRET_SUCCESS : TWE_APIRET_FAIL;
	} else return TWE_APIRET_FAIL;
}

/**
 * データロードを行う。
 * twesettings ライブラリから呼び出されるコールバック関数。
 * 
 * @param pBuf 		データ領域 pBuf->pu8buff[-16..-1] を利用することができる。
 * @param u8kind 	種別
 * @param u8slot 	スロット
 * @param u32Opt 	オプション
 * @param ... 
 * @return TWE_APIRET 
 */
TWE_APIRET TWESTG_cbu32LoadSetting(TWE_tsBuffer *pBuf, uint8 u8kind, uint8 u8slot, uint32 u32Opt, TWESTG_tsFinal *psFinal) {
	uint8 u8sect = s_u8GetFirstSect(u8kind, u8slot);
	if (u8sect != 0xFF) {
		bool_t bRes = TWENVM_bRead(pBuf, u8sect); //先頭セクターはコントロールブロックとして残し、2セクター単位で保存
		return bRes ? TWE_APIRET_SUCCESS : TWE_APIRET_FAIL;
	} else return TWE_APIRET_FAIL;
}

/*!
 * 諸処理を行うコールバック。
 * 主としてインタラクティブモードから呼び出されるが、一部は他より呼び出される。
 * 
 * \param pContext インタラクティブモードのコンテキスト(NULLの場合はインタラクティブモード以外からの呼び出し)
 * \param u32Op    コマンド番号
 * \param u32Arg1  引数１（役割はコマンド依存）
 * \param u32Arg2  引数２（役割はコマンド依存）
 * \param vpArg    引数３（役割はコマンド依存、主としてデータを戻す目的で利用する）
 * \return コマンド依存の定義。TWE_APIRET_FAILの時は何らかの失敗。
 */
TWE_APIRET TWEINTRCT_cbu32GenericHandler(TWEINTRCT_tsContext *pContext, uint32 u32Op, uint32 u32Arg1, uint32 u32Arg2, void *vpArg) {
	uint32 u32ApiRet = TWE_APIRET_SUCCESS;

	switch (u32Op) {
	case E_TWEINTCT_MENU_EV_LOAD:
		u8MenuMode = (uint8)u32Arg1;
		// メニューロード時の KIND/SLOT の決定。
		if (u8MenuMode == MENU_CONFIG) {
			// 通常メニュー
			u8AppKind = STGS_KIND_APPDEF;
			// u8AppSlot = TWESTG_SLOT_DEFAULT;
			s_appLoadData(u8AppKind, u8AppSlot, FALSE); // 設定を行う
			u32ApiRet = TWE_APIRET_SUCCESS_W_VALUE((uint32)u8AppKind << 8 | u8AppSlot);
		}
		break;

	case E_TWEINRCT_OP_UNHANDLED_CHAR: // 未処理文字列があった場合、呼び出される。
		break;

	case E_TWEINRCT_OP_RESET: // モジュールリセットを行う
		vAHI_SwReset();
		break;

	case E_TWEINRCT_OP_REVERT: // 設定をもとに戻す。ただしセーブはしない。
		s_appLoadData(u8AppKind, u8AppSlot, u32Arg1);
		break;

	case E_TWEINRCT_OP_CHANGE_KIND_SLOT: 
		// KIND/SLOT の切り替えを行う。切り替え後 pContext->psFinal は、再ロードされること。
		// u32Arg1,2 0xFF: no set, 0xFF00: -1, 0x0100: +1, 0x00?? Direct Set

		//メニューモードが CONFIG の時は、Parent の設定のみ行う
		if (u8MenuMode == MENU_CONFIG) {
			u8AppKind = STGS_KIND_APPDEF;

			// SLOT の設定
			if (u32Arg2 != 0xFF) {
				if ((u32Arg2 & 0xff00) == 0x0000) {
					u8AppSlot = u32Arg2 & 0x7;
				}
				else {
					if ((u32Arg2 & 0xff00) == 0x0100) {
						u8AppSlot++;
					}
					else {
						u8AppSlot--;
					}
				}

				// SLOT 数のチェック
				if (u8AppKind == STGS_KIND_APPDEF) {
					if (u8AppSlot == 0xFF) u8AppSlot = STGS_KIND_APPDEF_SLOT_MAX - 1;
					else if (u8AppSlot >= STGS_KIND_APPDEF_SLOT_MAX) u8AppSlot = 0;
				}
			}
		}
		
		// データの再ロード（同じ設定でも再ロードするのが非効率だが・・・）
		s_appLoadData(u8AppKind, u8AppSlot, FALSE); // 設定を行う

		// 値を戻す。
		// ここでは設定の失敗は実装せず、SUCCESS としている。
		// VALUE は現在の KIND と SLOT。
		u32ApiRet = TWE_APIRET_SUCCESS_W_VALUE((uint16)u8AppKind << 8 | u8AppSlot);
		break;

	case E_TWEINRCT_OP_WAIT: // 一定時間待つ（ポーリング処理）
		TWESYSUTL_vWaitPoll(u32Arg1);
		break;

	case E_TWEINRCT_OP_GET_APPNAME: // CONFIG行, アプリ名
		if (vpArg != NULL) {
			// &(char *)vpArg: には、バッファ16bytesのアドレスが格納されているため strcpy() でバッファをコピーしてもよいし、
			// 別の固定文字列へのポインタに書き換えてもかまわない。
			*((uint8**)vpArg) = (uint8*)INTRCT_USER_APP_NAME;
		}
		break;

	case E_TWEINRCT_OP_GET_KINDNAME: // CONFIG行, KIND種別名
		if (vpArg != NULL) {
			// &(char *)vpArg: には、バッファ16bytesのアドレスが格納されているため strcpy() でバッファをコピーしてもよいし、
			// 別の固定文字列へのポインタに書き換えてもかまわない。

			// このスコープ内に const uint8 SetList_names[][8] = { .. }; としても、うまくいかない。理由不明。
			*((uint8**)vpArg) = nullptr; // (uint8*)SetList_names[u32Arg1];
		}
		break;

	case E_TWEINTCT_OP_GET_OPTMSG:
		if (vpArg != NULL) {
			// &(char *)vpArg: には、バッファ32bytesのアドレスが格納されているため strcpy() でバッファをコピーしてもよいし、
			// 別の固定文字列へのポインタに書き換えてもかまわない。

			// このコードは -Os 最適化では不具合を起こす場合がある（case 節内にあるのが原因？）
			TWE_snprintf(*((char**)vpArg), 16, "tick=%d", u32TickCount_ms);
		}
		break;

	case E_TWEINTCT_OP_GET_SID: // シリアル番号
		if (vpArg != NULL) {
			// シリアル値を書き込む
			*((uint32*)vpArg) = ToCoNet_u32GetSerial();
		}
		break;

	default:
		break;
	}

	return u32ApiRet;
}

/**********************************************************************************
 * weak link defs
 **********************************************************************************/
void INTRCT_USER_vProcessInputByte(TWESERCMD_tsSerCmd_Context *pSerCmd, int16 u16Byte) {
    return;
}

/**********************************************************************************
 * mwx::interactive_settings
 **********************************************************************************/
void mwx::interactive_settings::begin(uint8_t slot) {
    // initialize the structure
    TWEINTRCT_pscInit(
            &sFinal, nullptr, _psSerial,
            (void*)INTRCT_USER_vProcessInputByte, // process bytes
            (const TWEINTRCT_tsFuncs*)asFuncs);

    // load initial data
    s_appLoadData(0x00, slot, FALSE);

    // configure settings
    _stgs.attach(&sFinal);
}