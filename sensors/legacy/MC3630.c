/* Copyright (C) 2017 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include "jendefs.h"
#include "AppHardwareApi.h"
#include "string.h"
#include "fprintf.h"

#include "sensor_driver.h"
#include "MC3630.h"
#include "SPI.h"

#include "ccitt8.h"

#include "utils.h"

#undef SERIAL_DEBUG
#ifdef SERIAL_DEBUG
# include <serial.h>
# include <fprintf.h>
extern tsFILE sDebugStream;
#endif

extern tsFILE _sSerLegacy; //for MWX lib
#define sSerStream _sSerLegacy

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
PRIVATE void vProcessSnsObj_MC3630(void *pvObj, teEvent eEvent);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

void vMC3630_Init(tsObjData_MC3630 *pData, tsSnsObj *pSnsObj) {
	vSnsObj_Init(pSnsObj);

	pSnsObj->pvData = (void*)pData;
	pSnsObj->pvProcessSnsObj = (void*)vProcessSnsObj_MC3630;

	memset((void*)pData, 0, sizeof(tsObjData_MC3630));

	// SPIの初期化
	vSPIInit( SPI_MODE3, SLAVE_ENABLE1, 1);
}

/****************************************************************************
 *
 * NAME: bMC3630reset
 *
 * DESCRIPTION:
 *
 * RETURNS:
 * bool_t	fail or success
 *
 ****************************************************************************/
//	リセットというよりは初期化処理
PUBLIC bool_t bMC3630_reset( uint8 Freq, uint8 Range, uint8 SplNum )
{
	bool_t bOk = TRUE;
	vfPrintf(&sSerStream, "\n\rMC3630 Reset!");
	
    /* configure SPI interface */
	/*	SPI Mode3	*/
	vSPIInit( SPI_MODE3, SLAVE_ENABLE1, 3 );

	// Standby
	vMC3630_Sleep();

	// Reset
	vSPIChipSelect(CS_DIO19);
	vSPIWrite(MC3630_RESET | MC3630_WRITE);
	vSPIWrite(0x40);
	vSPIStop();

	// おまじない
	vSPIChipSelect(CS_DIO19);
	vSPIWrite(0x1B | MC3630_WRITE);
	vSPIWrite(0x00);
	vSPIStop();
	// Wait
	vWait(5000);

	// SPIに設定
	vSPIChipSelect(CS_DIO19);
	vSPIWrite(MC3630_FREG_1 | MC3630_WRITE);
	vSPIWrite(0x90);
	vSPIStop();

	// ここからおまじない
	vSPIChipSelect(CS_DIO19);
	vSPIWrite(MC3630_INIT_1 | MC3630_WRITE);
	vSPIWrite(0x42);
	vSPIStop();

	vSPIChipSelect(CS_DIO19);
	vSPIWrite(MC3630_DMX | MC3630_WRITE);
	vSPIWrite(0x1);
	vSPIStop();

	vSPIChipSelect(CS_DIO19);
	vSPIWrite(MC3630_DMY | MC3630_WRITE);
	vSPIWrite(0x80);
	vSPIStop();

	vSPIChipSelect(CS_DIO19);
	vSPIWrite(MC3630_INIT_2 | MC3630_WRITE);
	vSPIWrite(0x00);
	vSPIStop();

	vSPIChipSelect(CS_DIO19);
	vSPIWrite(MC3630_INIT_3 | MC3630_WRITE);
	vSPIWrite(0x00);
	vSPIStop();
	// ここまでおまじない

	vSPIChipSelect(CS_DIO19);
	vSPIWrite(MC3630_CHIP_ID | MC3630_READ);
	vSPIWrite(0x00);
	uint8 u8Result = u8SPIRead();
	vSPIStop();
	if( u8Result != 0x71 ){
		bOk = FALSE;
		vfPrintf(&sSerStream, "\n\rMC3630 Not Connected. %02X", u8Result);
	}

	// 一旦STANBYモードにする
	vMC3630_Sleep();

	// SPIのクロックと加速度計測をUltraLowPowerに変更する
	vSPIChipSelect(CS_DIO19);
	vSPIWrite(MC3630_PMCR | MC3630_WRITE);
	vSPIWrite(0x80|0x30|0x03);
	//vSPIWrite(0x30|0x03);
	vSPIStop();

	// SPIをハイスピードモードに変更したのでクロックの周波数を変更する
	vSPIInit( SPI_MODE3, SLAVE_ENABLE1, 1 );

	// サンプリング周波数
	vSPIChipSelect(CS_DIO19);
	vSPIWrite(MC3630_RATE_1 | MC3630_WRITE);
	vSPIWrite(Freq);
	vSPIStop();

	// レンジを16g、12bitの分解能にする
	vSPIChipSelect(CS_DIO19);
	vSPIWrite(MC3630_RANGE_C | MC3630_WRITE);
	vSPIWrite(Range|0x04);
	vSPIStop();


	// 10サンプルのFIFOを使用する
	vSPIChipSelect(CS_DIO19);
	vSPIWrite(MC3630_FIFO_C | MC3630_WRITE);
	vSPIWrite(0x40|SplNum);
	vSPIStop();

	// 新しいデータが取れたら割り込み + 割り込みピンをプッシュプルに変える
	vSPIChipSelect(CS_DIO19);
	vSPIWrite(MC3630_INTR_C | MC3630_WRITE);
	vSPIWrite(0x41);
	vSPIStop();

	// 読み書きするときのレジスタ指定のところで加速度センサのステータスを返すようにする
	vSPIChipSelect(CS_DIO19);
	vSPIWrite(MC3630_FREG_2 | MC3630_WRITE);
	vSPIWrite(0x04);
	vSPIStop();

	// 連続で測定する。
	//vMC3630_Wakeup();

	return bOk;
}

/****************************************************************************
 *
 * NAME: vHTSstartReadTemp
 *
 * DESCRIPTION:
 *
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC bool_t bMC3630_startRead()
{
	//	通信できたかどうかを知るすべがないのでそのままTRUEを返す
	bool_t bOk = TRUE;

	return bOk;
}

/****************************************************************************
 *
 * NAME: u16MC3630readResult
 *
 * DESCRIPTION:
 *
 * RETURNS:
 *
 * NOTES:
 *
 ****************************************************************************/
//	各軸の加速度を読みに行く
PUBLIC uint8 u8MC3630_readResult( int16* ai16x, int16* ai16y, int16* ai16z )
{
	uint8	au8data[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
	uint8	i;
	uint8	u8status;
	uint8	u8num = 0;

	//	各軸の読み込み
	while(1){
		vSPIChipSelect(CS_DIO19);
	   	vSPIWrite(MC3630_READ|MC3630_XOUT_LSB);
		u8status = u8SPIRead();
		if(u8status&0x10){
			vSPIStop();
			break;
		}

		for(i=0;i<6;i++){
			vSPIWrite(0x00);
			au8data[i] = u8SPIRead();
		}
		vSPIStop();

		ai16x[u8num] = (au8data[1]<<8|au8data[0])*8;
		ai16y[u8num] = (au8data[3]<<8|au8data[2])*8;
		ai16z[u8num] = (au8data[5]<<8|au8data[4])*8;
		u8num++;
	}

	

	vMC3630_ClearInterrupReg();
    return u8num;
}

PUBLIC uint8 u8MC3630_ReadSamplingFrequency()
{
	vSPIChipSelect(CS_DIO19);
	vSPIWrite(MC3630_RATE_1 | MC3630_READ);
	uint8 data = u8SPIRead();
	vSPIWrite(0x00);
	data = u8SPIRead();
	vSPIStop();

	return data;
}

PUBLIC uint8 u8MC3630_ReadInterrupt()
{
	vSPIChipSelect(CS_DIO19);
	vSPIWrite( MC3630_STATUS_2|MC3630_READ );
	uint8 data = u8SPIRead();
	vSPIWrite( 0x00 );
	data = u8SPIRead();
	vSPIStop();

	return data;
}

PUBLIC void vMC3630_ClearInterrupReg()
{
	vSPIChipSelect(CS_DIO19);
	vSPIWrite( MC3630_STATUS_2|MC3630_WRITE );
	vSPIWrite( 0x00 );
	vSPIStop();
}

PUBLIC void vMC3630_Wakeup()
{
	// 連続で測定する。
	vSPIChipSelect(CS_DIO19);
	vSPIWrite(MC3630_MODE_C | MC3630_WRITE);
	vSPIWrite(0x05);
	vSPIStop();
}

PUBLIC void vMC3630_Sleep()
{
	// 一旦STANBYモードにする
	vSPIChipSelect(CS_DIO19);
	vSPIWrite(MC3630_MODE_C | MC3630_WRITE);
	vSPIWrite(0x01);
	vSPIStop();
}

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/
// the Main loop
PRIVATE void vProcessSnsObj_MC3630(void *pvObj, teEvent eEvent) {
	tsSnsObj *pSnsObj = (tsSnsObj *)pvObj;
	tsObjData_MC3630 *pObj = (tsObjData_MC3630 *)pSnsObj->pvData;

	// general process (independent from each state)
	switch (eEvent) {
		case E_EVENT_TICK_TIMER:
			if (pObj->u8TickCount < 100) {
				pObj->u8TickCount += pSnsObj->u8TickDelta;
#ifdef SERIAL_DEBUG
vfPrintf(&sDebugStream, "+");
#endif
			}
			break;
		case E_EVENT_START_UP:
			pObj->u8TickCount = 100; // expire immediately
#ifdef SERIAL_DEBUG
vfPrintf(&sDebugStream, "\n\rMC3630 WAKEUP");
#endif
			break;
		default:
			break;
	}

	// state machine
	switch(pSnsObj->u8State)
	{
	case E_SNSOBJ_STATE_INACTIVE:
		// do nothing until E_ORDER_INITIALIZE event
		break;

	case E_SNSOBJ_STATE_IDLE:
		switch (eEvent) {
		case E_EVENT_NEW_STATE:
			break;

		case E_ORDER_KICK:
			vSnsObj_NewState(pSnsObj, E_SNSOBJ_STATE_MEASURING);

			#ifdef SERIAL_DEBUG
			vfPrintf(&sDebugStream, "\n\rMC3630 KICKED");
			#endif

			break;

		default:
			break;
		}
		break;

	case E_SNSOBJ_STATE_MEASURING:
		switch (eEvent) {
		case E_EVENT_NEW_STATE:
			//pObj->u8Interrupt = u8Interrupt;
			pObj->u8TickWait = MC3630_CONVTIME;

			pObj->bBusy = TRUE;
#ifdef MC3630_ALWAYS_RESET
			u8reset_flag = TRUE;
			if (!bMC3630reset()) {
				vSnsObj_NewState(pSnsObj, E_SNSOBJ_STATE_COMPLETE);
			}
#else
			//if (!bMC3630startRead()) { // kick I2C communication
			//	vSnsObj_NewState(pSnsObj, E_SNSOBJ_STATE_COMPLETE);
			//}
#endif
			pObj->u8TickCount = 0;
			break;

		default:
			break;
		}

		// wait until completion
		if (pObj->u8TickCount > pObj->u8TickWait) {
#ifdef MC3630_ALWAYS_RESET
			if (u8reset_flag) {
				u8reset_flag = 0;
				if (!bMC3630_startRead()) {
					vMC3630_new_state(pObj, E_SNSOBJ_STATE_COMPLETE);
				}

				pObj->u8TickCount = 0;
				pObj->u8TickWait = MC3630_CONVTIME;
				break;
			}
#endif

			pObj->u8FIFOSample = u8MC3630_readResult( pObj->ai16Result[MC3630_X], pObj->ai16Result[MC3630_Y], pObj->ai16Result[MC3630_Z] );
			pObj->u8SampleFreq = u8MC3630_ReadSamplingFrequency();

			// data arrival
			pObj->bBusy = FALSE;
			vSnsObj_NewState(pSnsObj, E_SNSOBJ_STATE_COMPLETE);
		}
		break;

	case E_SNSOBJ_STATE_COMPLETE:
		switch (eEvent) {
		case E_EVENT_NEW_STATE:
			#ifdef SERIAL_DEBUG
			vfPrintf(&sDebugStream, "\n\rMC3630_CP: %d", pObj->i16Result);
			#endif

			break;

		case E_ORDER_KICK:
			// back to IDLE state
			vSnsObj_NewState(pSnsObj, E_SNSOBJ_STATE_IDLE);
			break;

		default:
			break;
		}
		break;

	default:
		break;
	}
}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
