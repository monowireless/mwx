/* Copyright (C) 2017 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */


#include "jendefs.h"
#include <string.h>

#include <AppHardwareApi.h>

#include "ToCoNet.h"
#include "ToCoNet_event.h"
#include "sensor_driver.h"

#define SERIAL_DEBUG
#ifdef SERIAL_DEBUG
# include <serial.h>
# include <fprintf.h>
#define sDebugStream _sSerLegacy
extern tsFILE sDebugStream;
#endif

/** @ingroup SNSDRV
 * センサー状態マシンを初期化する
 *
 * @param pObj 管理構造体
 */
void vSnsObj_Init(tsSnsObj *pObj) {
	pObj->u8State = 0;
	pObj->u8TickDelta = 1000 / sToCoNet_AppContext.u16TickHz;
	pObj->pvData = NULL;
	pObj->pvProcessSnsObj = NULL;
	
	// pObj->u32Opt; // setting data there, shall not overwrite.
	pObj->u32Stat = 0;
}

/** @ingroup SNSDRV
 *
 * 状態を遷移し、遷移後の状態が同じであれば終了。
 * 状態に変化が有れば E_EVENT_NEW_STATE イベントにて、再度関数を呼び出す。
 *
 * @param pObj センサ状態構造体
 * @param eEv イベント
 */
void vSnsObj_Process(tsSnsObj *pObj, teEvent eEv) {
	uint8 u8status_init;

#ifdef SERIAL_DEBUG
	vfPrintf(&sDebugStream, "<p%x>", pObj->pvData);
#endif

	u8status_init = pObj->u8State;
	pObj->pvProcessSnsObj(pObj, eEv);

	// 連続的に状態遷移させるための処理。pvProcessSnsObj() 処理後に状態が前回と変化が有れば、
	// 再度 E_EVENT_NEW_STATE により実行する。
	while (pObj->u8State != u8status_init) {
		u8status_init = pObj->u8State;
		pObj->pvProcessSnsObj(pObj, E_EVENT_NEW_STATE);
	}
}

