/* Copyright (C) 2020 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */

#include <TWELITE>

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
#include "tweutils.h"

#include "mwx_stgs_standard_core.h"
#include "mwx_stgs_standard.hpp"

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

#define STGS_KIND_MAX 1				//! 種別数の最大

#define MENU_CONFIG 1 // 設定モード

// Application Specific
#define APPVER				((VERSION_MAIN << 24) | (VERSION_SUB << 16) | (VERSION_VAR << 8))

/**********************************************************************************
 * FUNCTION PROTOTYPES
 **********************************************************************************/

/**********************************************************************************
 * CONSTANTS
 **********************************************************************************/

/**********************************************************************************
 * EXTERNS with "C" linkage (works with twesettings C library)
 **********************************************************************************/
extern "C" TWE_tsFILE* _psSerial;

extern "C" void INTRCT_USER_vProcessInputByte(TWESERCMD_tsSerCmd_Context *pSerCmd, int16 u16Byte) __attribute__((weak));
extern "C" TWE_APIRET TWEINTRCT_cbu32GenericHandler(TWEINTRCT_tsContext *pContext, uint32 u32Op, uint32 u32Arg1, uint32 u32Arg2, void *vpArg);
extern "C" TWE_APIRET TWESTG_cbu32LoadSetting(TWE_tsBuffer *pBuf, uint8 u8kind, uint8 u8slot, uint32 u32Opt, TWESTG_tsFinal *psFinal);
extern "C" TWE_APIRET TWESTG_cbu32SaveSetting(TWE_tsBuffer *pBuf, uint8 u8kind, uint8 u8slot, uint32 u32Opt, TWESTG_tsFinal *psFinal);

void TWEINTCT_vSerUpdateScreen_hw(TWEINTRCT_tsContext *psIntr);
void TWEINTCT_vProcessInputByte_hw(TWEINTRCT_tsContext *psIntr, TWEINTRCT_tkeycode keycode);
void TWEINTCT_vProcessInputString_hw(TWEINTRCT_tsContext *psIntr, TWEINPSTR_tsInpStr_Context *pContext);
TWE_APIRET TWEINTCT_u32ProcessMenuEvent_hw(TWEINTRCT_tsContext *psIntr, uint32 u32Op, uint32 u32Arg1, uint32 u32Arg2, void *vpArg);

/**********************************************************************************
 * VARIABLES
 **********************************************************************************/
/*!
 * tsFinal 構造体のデータ領域を宣言する
 */
TWESTG_DECLARE_FINAL(SET_STD, STGS_MAX_SETTINGS_COUNT, 32, 8); // 確定設定リストの配列等の宣言

/*!
 * 確定設定リスト
 */
static TWESTG_tsFinal sFinal;

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

/**
 * @brief sFinal のリロードが必要な場合に 1 をセットする
 */
static uint8 u8Dirty = 0; // should reload this

/*!
 * E_TWESTG_DEFSETS_CHANNELS_3（チャネル設定、最大３チャネル）を削除する
 */
uint8 SETSTD_CUST_COMMON[SIZE_SETSTD_CUST_COMMON] = {
	8   // 総バイト数(このバイトは含まない。手計算で間違えないように入力！)
	, E_TWESTG_DEFSETS_APPID, (TWESTG_DATATYPE_UINT32 << 4) | 4, 0x00, 0x34, 0x56, 0x78 
	, E_TWESTG_DEFSETS_CHANNELS_3, TWESTG_DATATYPE_UNUSE
};

/*!
 * 設定定義(tsSettings)
 *   スロット0..7までの定義を記述
 */
#if 0
TWESTG_tsSettingsListItem SetList[1][9] = {
	{
		{ STGS_KIND_APPDEF, TWESTG_SLOT_DEFAULT, { SET_STD_DEFSETS, NULL, INTRCT_USER_SLOT0_SETTINGS, SETSTD_CUST_COMMON, NULL, NULL } },
		{ STGS_KIND_APPDEF, TWESTG_SLOT_1,       { SET_STD_DEFSETS, NULL, INTRCT_USER_SLOT1_SETTINGS, SETSTD_CUST_COMMON, NULL, TWESTG_DEFCUST_SLOT[1] } },
		{ STGS_KIND_APPDEF, TWESTG_SLOT_2,       { SET_STD_DEFSETS, NULL, INTRCT_USER_SLOT2_SETTINGS, SETSTD_CUST_COMMON, NULL, TWESTG_DEFCUST_SLOT[2] } },
		{ STGS_KIND_APPDEF, TWESTG_SLOT_3,       { SET_STD_DEFSETS, NULL, INTRCT_USER_SLOT3_SETTINGS, SETSTD_CUST_COMMON, NULL, TWESTG_DEFCUST_SLOT[3] } },
		{ STGS_KIND_APPDEF, TWESTG_SLOT_4,       { SET_STD_DEFSETS, NULL, INTRCT_USER_SLOT4_SETTINGS, SETSTD_CUST_COMMON, NULL, TWESTG_DEFCUST_SLOT[4] } },
		{ STGS_KIND_APPDEF, TWESTG_SLOT_5,       { SET_STD_DEFSETS, NULL, INTRCT_USER_SLOT5_SETTINGS, SETSTD_CUST_COMMON, NULL, TWESTG_DEFCUST_SLOT[5] } },
		{ STGS_KIND_APPDEF, TWESTG_SLOT_6,       { SET_STD_DEFSETS, NULL, INTRCT_USER_SLOT6_SETTINGS, SETSTD_CUST_COMMON, NULL, TWESTG_DEFCUST_SLOT[6] } },
		{ STGS_KIND_APPDEF, TWESTG_SLOT_7,       { SET_STD_DEFSETS, NULL, INTRCT_USER_SLOT7_SETTINGS, SETSTD_CUST_COMMON, NULL, TWESTG_DEFCUST_SLOT[7] } },
		{ TWESTG_KIND_VOID, TWESTD_SLOT_VOID, { NULL }}, // TERMINATE
	},
};
#endif

/**
 * デフォルトの設定リスト (単一設定、スロット無し)
 */
static const TWESTG_tsSettingsListItem SetList_SINGLE[] = {
	{ STGS_KIND_APPDEF, TWESTG_SLOT_DEFAULT, { SET_STD_DEFSETS, NULL, NULL, SETSTD_CUST_COMMON, NULL, NULL } },
	{ TWESTG_KIND_VOID, TWESTD_SLOT_VOID, { NULL }}, // TERMINATE
};

/**
 * スロットのテーブル
 */
static const TWESTG_tsSettingsListItem *pSetList = SetList_SINGLE;
static uint8 u8SetSlotCount = 1;

/**
 * @brief 設定リストのメニュー名
 */
static const uint8 SetList_names[][8] = { "*", };

/**
 * @brief メニュー定義
 */
static const TWEINTRCT_tsFuncs asFuncs[] = {
	{ 0, (uint8*)"ROOT MENU", TWEINTCT_vSerUpdateScreen_defmenus, TWEINTCT_vProcessInputByte_defmenus, TWEINTCT_vProcessInputString_defmenus, TWEINTCT_u32ProcessMenuEvent_defmenus }, // standard _settings
	{ 1, (uint8*)"CONFIG", TWEINTCT_vSerUpdateScreen_settings, TWEINTCT_vProcessInputByte_settings, TWEINTCT_vProcessInputString_settings, TWEINTCT_u32ProcessMenuEvent_settings }, // standard _settings
	{ 2, (uint8*)"EEPROM UTIL", TWEINTCT_vSerUpdateScreen_nvmutils, TWEINTCT_vProcessInputByte_nvmutils, TWEINTCT_vProcessInputString_nvmutils, TWEINTCT_u32ProcessMenuEvent_nvmutils }, // standard _settings
	{ 3, (uint8*)"H/W UTIL", TWEINTCT_vSerUpdateScreen_hw, TWEINTCT_vProcessInputByte_hw, TWEINTCT_vProcessInputString_hw, TWEINTCT_u32ProcessMenuEvent_hw }, // standard _settings
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
	TWESTG_INIT_FINAL(SET_STD, &sFinal);
	// tsFinal 構造体に基本情報を適用する
	TWESTG_u32SetBaseInfoToFinal(&sFinal, INTRCT_USER_APP_ID, APPVER, STGS_SET_VER, STGS_SET_VER_COMPAT);
	// tsFinal 構造体に kind, slot より、デフォルト設定リストを構築する
	TWESTG_u32SetSettingsToFinal(&sFinal, u8kind, u8slot, pSetList);
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
		Serial << crlf << "!INF RESET SYSTEM...";
		delay(100);

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
					if (u8AppSlot == 0xFF) u8AppSlot = u8SetSlotCount - 1;
					else if (u8AppSlot >= u8SetSlotCount) u8AppSlot = 0;
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
			//TWE_snprintf(*((char**)vpArg), 16, "tick=%d", u32TickCount_ms);
			TWE_snprintf(*((char**)vpArg), 16, "SID=%8X", ToCoNet_u32GetSerial());
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

/*********************************************************************************
 * menu item (hw utils)
 *********************************************************************************/
/**
 * 接続したPALのデータを保持する
 */
typedef struct {
	uint8 u8EEPROMStatus;		// EEPROMのステータス
	uint8 u8FormatVersion;		// データ書式のバージョン
	uint8 u8PALModel;			// 接続したPALのモデル
	uint8 u8PALVersion;			// 接続したPALの基板バージョン
	uint32 u32SerialID;			// シリアルID
	uint8 u8OptionLength;		// オプションの長さ
	uint8 au8Option[64];		// オプション
} tsPALData;

void TWEINTCT_vSerUpdateScreen_hw(TWEINTRCT_tsContext *psIntr) {
	TWEINTRCT_vSerHeadLine(psIntr, 0UL);
	
	Serial << crlf << "\033[7md\033[0m : DI status";
	Serial << crlf << "\033[7mi\033[0m : probe i2c bus";
	Serial << crlf << "\033[7mp\033[0m : PAL board EEPROM";
	
	Serial << crlf << crlf;
	TWEINTRCT_vSerFootLine(psIntr, 0); // フッター行の表示
}

void TWEINTCT_vProcessInputByte_hw(TWEINTRCT_tsContext *psIntr, TWEINTRCT_tkeycode keycode) {
	bool_t bHandled = FALSE;
	bool_t bForceRedraw = FALSE;
	TWE_APIRET ret = 0;

	psIntr->u16HoldUpdateScreen = 0;

	switch (keycode) {
		case 'i': 
			Serial << "\033[2J\033[HPROBING I2C BUS..." << mwx::crlf;
			{
				Wire.begin();	

				int stat;
				int ct = 0;
				for (int i = 1; i < 127; i++) {
					
					Wire.beginTransmission(i);
					stat = Wire.endTransmission();

					delay(10);
					
					if (stat == 0) {
						Serial << format("\033[7m[%02x]\033[0m", i, stat);
						ct++;
					}
					else
						Serial << format("[%02x]", i, stat);

					if ((i & 15) == 0) Serial << crlf;
				}
				Serial << crlf << "found " << ct << " device(s)." << crlf;
			}
			bHandled = TRUE;			
			break;
		
		case 'd':
			Serial <<  "\033[2J\033[HDI STATUS..." << crlf;
			{
				uint32_t u32bm = digitalReadBitmap();
				Serial << crlf << " 0 . . . . 5 . . . .10 . . . .15 . . . ." << crlf;
				for (int i = 0; i < 20; i++) {
					char_t c = (u32bm & (1UL << i)) ? 'H' : 'L';
					Serial << ' ' << c;
				}
				Serial << crlf;
			}
			bHandled = TRUE;
			break;

		case 'p': // pal EEP
			{	
				bHandled = TRUE;
			
				Serial << crlf << "\033[2J\033[HPAL EEPROM...";
				
				const uint8_t DEV_ADDR = 0x56;
				uint8_t u8StartAddr = 0x00;
				uint8_t u8Data[64];
				uint8_t i = 0;

				uint32_t u32MagicNumber;
				tsPALData sPALData;

				memset(u8Data, 0, sizeof(u8Data));

				Wire.begin();

				if (!Wire.probe(DEV_ADDR)) {
					Serial << crlf << "<PAL EEPROM IS NOT FOUND>";
					break;
				}

				if (auto&& wrt = Wire.get_writer(DEV_ADDR)) {
					wrt(u8StartAddr);
				}
				if (auto&& rdr = Wire.get_reader(DEV_ADDR, sizeof(u8Data))) {
					rdr >> u8Data;
				}
				
				Serial << crlf;
				for (i = 0; i < sizeof(u8Data); i++) {
					Serial << format("%02X ", u8Data[i]);
					if (i != sizeof(u8Data) - 1 && (i & 0xF) == 0xF) Serial << crlf;
				}
				Serial << crlf;

				auto&& np = expand_bytes(std::begin(u8Data), std::end(u8Data)
					, u32MagicNumber
					, sPALData.u8OptionLength
					, sPALData.u8FormatVersion
					);

				if(u32MagicNumber != 0xA55A00FF) {
					Serial << crlf << "<MAGIC NUMBER ERROR>";
				}
				
				sPALData.u8OptionLength--;

				// Serial << crlf << "OptionLength : " << int(sPALData.u8OptionLength);
				Serial << crlf << "Format Version : " << int(sPALData.u8FormatVersion);

				// for 0x01 format
				uint16_t u16Reserved;
				if (sPALData.u8FormatVersion == 0x01) {
					np = expand_bytes(np, std::end(u8Data)
						, sPALData.u8PALModel
						, sPALData.u8PALVersion
						, u16Reserved
					);

					Serial << crlf << format("PAL MODEL=%d VER=%d", sPALData.u8PALModel, sPALData.u8PALVersion);

					sPALData.u8OptionLength -= 4; // reserve area
					Serial << crlf << format("  Opt[%d]: ", sPALData.u8OptionLength);
					for(i = 0; i < sPALData.u8OptionLength; i++, np++) {
						Serial << format("%02X ", *np);	
					}

					uint8_t u8EEPcrc = G_OCTET(np);
					uint8_t u8crc = CRC8_u8Calc(u8Data+5, sPALData.u8OptionLength + 5);
					if (u8crc != u8EEPcrc) {
						Serial << crlf << format("  <CRC ERROR %02X:%02X>", u8EEPcrc, u8crc);
					}
				}
			}
			break;

		default:
			ret = TWEINTRCT_u32MenuOpKey(psIntr, keycode);
			if (TWE_APIRET_IS_SUCCESS(ret)) {
				bHandled = TRUE;
				if (TWE_APIRET_VALUE(ret) == 1) {
					bForceRedraw = TRUE;
				}
			}
			break;
	}

	if (!bHandled || bForceRedraw) {
		psIntr->u16HoldUpdateScreen = 5;
	}
}

void TWEINTCT_vProcessInputString_hw(TWEINTRCT_tsContext *psIntr, TWEINPSTR_tsInpStr_Context *pContext) {
}

TWE_APIRET TWEINTCT_u32ProcessMenuEvent_hw(TWEINTRCT_tsContext *psIntr, uint32 u32Op, uint32 u32Arg1, uint32 u32Arg2, void *vpArg) {
	TWE_APIRET ret = TWE_APIRET_FAIL;
	return ret;
}

/*********************************************************************************
 * IMPL of StgsStandard
 *********************************************************************************/
#define DEBUG_LVL 99

namespace mwx { inline namespace L1 {	
	void StgsStandard::reload(uint8_t slot) {
		s_appLoadData(0x00, slot, FALSE);
	}

	void StgsStandard::_reload(bool bSetDirty) {
		if (bSetDirty) u8Dirty = 1;
		else {
			if (u8Dirty) {
				reload();
			}
		}
	}
	void StgsStandard::_setup() {
		MWX_DebugMsg(DEBUG_LVL, "_setup()\r\n");
		// load initial data
		//s_appLoadData(0x00, 0xFF, FALSE);

		// Serial surrogate
		_ser_surr.pobj = (void*)&this->serial;
		_ser_surr.pf_avail = _serial::_available;
		_ser_surr.pf_read = _serial::_read;
		Serial.register_surrogate(&_ser_surr);

		// initialize the structure
		_psIntr = TWEINTRCT_pscInit(
			&sFinal, nullptr, _psSerial,
			(void*)_vProcessInputByte, // process bytes
			(const TWEINTRCT_tsFuncs*)asFuncs);
		
		// optional settings
		_psIntr->config.u8Mode = 0; // 0:normal 1:init mode is the interactive.
		// _psIntr->config.u8AlwaysKeyReport = 1; // 0:normal 1:always has a key call back
		_psIntr->config.u8OptSerCmd = 0x01; // echo back seeting
		// _psIntr->config.u8NoSysReset = 1; // no reset required to take effect
		_psIntr->config.u8screen_default = 1; // opening screen (0: menu, 1: settigns)
		//_psIntr->config.u8DefMenusSlots = 1; // list slot 1..MAX
		_psIntr->pvWrapObjCpp = (void*)this; // store this object pointer
		//_psIntr->u16HoldUpdateScreen = 96; // refresh count (set 1 or above)
		TWEINTRCT_vReConf(_psIntr); // apply optional settings

		// configure settings
		set.attach(&sFinal);    
	}

	void StgsStandard::_begin() {
		MWX_DebugMsg(DEBUG_LVL, "_begin()\r\n");
	}

	void StgsStandard::set_default_appid(uint32_t u32appid) {
		uint8_t *q = &SETSTD_CUST_COMMON[3];
		S_DWORD(q, u32appid);
		INTRCT_USER_APP_ID = u32appid;
	}

	void StgsStandard::set_ch_multi() {
		SETSTD_CUST_COMMON[7] = E_TWESTG_DEFSETS_CHANNEL;
	}

	bool StgsStandard::_get_ch_multi() {
		return SETSTD_CUST_COMMON[7] == E_TWESTG_DEFSETS_CHANNEL;
	} 

	void StgsStandard::set_appname(const char* appname) {
		INTRCT_USER_APP_NAME = appname;
	}

	StgsStandard& StgsStandard::operator << (SETTINGS::open_at_start&& v) {
		Serial << crlf << "!INF ENTERING CONFIG MODE...";
		_psIntr->config.u8Mode = 1;
		_psIntr->u16HoldUpdateScreen = 32; // refresh count (set 1 or above)
		TWEINTRCT_vReConf(_psIntr); // apply optional settings
		return *this;
	}
}}