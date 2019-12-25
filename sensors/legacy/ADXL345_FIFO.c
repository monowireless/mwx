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
#include "ADXL345.h"
#include "SMBus.h"

#include "ccitt8.h"

#include "utils.h"

#include "Interactive.h"

#include "EndDevice_Input.h"

# include <serial.h>
# include <fprintf.h>
#undef SERIAL_DEBUG
#ifdef SERIAL_DEBUG
extern tsFILE sDebugStream;
#endif

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
PRIVATE void vProcessSnsObj_ADXL345_FIFO(void *pvObj, teEvent eEvent);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
uint8	u8num;

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
void vADXL345_FIFO_Init(tsObjData_ADXL345 *pData, tsSnsObj *pSnsObj) {
	vSnsObj_Init(pSnsObj);

	pSnsObj->pvData = (void*)pData;
	pSnsObj->pvProcessSnsObj = (void*)vProcessSnsObj_ADXL345_FIFO;

	memset((void*)pData, 0, sizeof(tsObjData_ADXL345));
}

//	センサの設定を記述する関数
bool_t bADXL345_FIFO_Setting( uint16 u16mode, tsADXL345Param sParam )
{
	uint8 com;
	uint8 u8UseAxis = (uint8)(((~u16mode)&0x7000)>>12);

	switch( sParam.u16Duration ){
	case 1:
		com = 0x04;		//	1.56Hz Sampling frequency
		break;
	case 3:
		com = 0x05;		//	3.13Hz Sampling frequency
		break;
	case 6:
		com = 0x06;		//	6.25Hz Sampling frequency
		break;
	case 12:
		com = 0x07;		//	12.5Hz Sampling frequency
		break;
	case 25:
		com = 0x08;		//	25Hz Sampling frequency
		break;
	case 50:
		com = 0x09;		//	50Hz Sampling frequency
		break;
	case 100:
		com = 0x0A;		//	100Hz Sampling frequency
		break;
	case 200:
		com = 0x0B;		//	200Hz Sampling frequency
		break;
	case 400:
		com = 0x0C;		//	400Hz Sampling frequency
		break;
	case 800:
		com = 0x0D;		//	800Hz Sampling frequency
		break;
	default:
		com = 0x0A;		//	100Hz Sampling frequency
		break;
	}
	bool_t bOk = bSMBusWrite(ADXL345_ADDRESS, ADXL345_BW_RATE, 1, &com );

	com = 0x0B;		//	Full Resolution Mode, +-16g
	bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_DATA_FORMAT, 1, &com );

	com = 0x02;
	bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_INT_MAP, 1, &com );

	bOk &= bADXL345_StartMeasuring(FALSE);

	//	タップを判別するための閾値
	if( (u16mode&S_TAP) || (u16mode&D_TAP) ){
		com = 0x32;			//	threshold of tap
		bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_THRESH_TAP, 1, &com );

		//	タップを認識するための時間
		com = 0x0F;
		bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_DUR, 1, &com );

		//	タップを検知する軸の設定
		com = u8UseAxis;
		bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_TAP_AXES, 1, &com );
	}

	//	次のタップまでの時間
	if( u16mode&D_TAP ){
		com = 0x50;
		bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_LATENT, 1, &com );

		//	ダブルタップを認識するための時間
		com = 0xD9;			// Window Width
		bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_WINDOW, 1, &com );
	}

	//	自由落下を検知するための閾値
	if( u16mode&FREEFALL ){
		com = 0x07;			// threshold of freefall
		bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_THRESH_FF, 1, &com );

		//	自由落下を検知するための時間
		com = 0x2D;			// time of freefall
		bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_TIME_FF, 1, &com );
	}

	//	動いていることを判断するための閾値
	if( u16mode&ACTIVE ){
		com = 0x15;
		bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_THRESH_ACT, 1, &com );

		//	動いていないことを判断するための閾値
		com = 0x14;
		bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_THRESH_INACT, 1, &com );

		//	動いていないことを判断するための時間(s)
		com = 0x02;
		bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_TIME_INACT, 1, &com );

		//	動いている/いないことを判断するための軸
		com = u8UseAxis|(uint8)(u8UseAxis<<4);
		bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_ACT_INACT_CTL, 1, &com );
	}

	if( (u16mode&0x000F) == 0 ){
		bOk &= bADXL345_EnableFIFO( 0 );
	}else{
		bOk &= bADXL345_DisableFIFO( (uint16)(u16mode&0x000F) );
	}

//	com = 0xC0 | 0x20 | READ_FIFO;
	com = 0x80 | 0x00 | READ_FIFO;
	bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_FIFO_CTL, 1, &com );

	return bOk;
}

bool_t bADXL345_EnableFIFO( uint16 u16mode )
{
	//	有効にする割り込みの設定
	uint8 com = 0x02;
	if( u16mode&S_TAP ){
		com += 0x40;
	}
	if( u16mode&D_TAP ){
		com += 0x20;
	}
	if( u16mode&FREEFALL ){
		com += 0x04;
	}
	if( u16mode&ACTIVE ){
		com += 0x18;
	}
	bool_t bOk = bSMBusWrite(ADXL345_ADDRESS, ADXL345_INT_ENABLE, 1, &com );

//	com = 0xC0 | 0x20 | READ_FIFO;
//	bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_FIFO_CTL, 1, &com );

	return bOk;
}

bool_t bADXL345_DisableFIFO( uint16 u16mode )
{
	// FIFOを止める
//	uint8 com = 0x00 | 0x20 | READ_FIFO;
//	bool_t bOk = bSMBusWrite(ADXL345_ADDRESS, ADXL345_FIFO_CTL, 1, &com );

	//	有効にする割り込みの設定
	uint8 com = 0;
	if( u16mode&S_TAP ){
		com += 0x40;
	}
	if( u16mode&D_TAP ){
		com += 0x20;
	}
	if( u16mode&FREEFALL ){
		com += 0x04;
	}
	if( u16mode&ACTIVE ){
		com += 0x18;
	}
	bool_t bOk = bSMBusWrite(ADXL345_ADDRESS, ADXL345_INT_ENABLE, 1, &com );

	return bOk;
}

/****************************************************************************
 *
 * NAME: u16ADXL345readResult
 *
 * DESCRIPTION:
 * Wrapper to read a measurement, followed by a conversion function to work
 * out the value in degrees Celcius.
 *
 * RETURNS:
 * int16: 0~10000 [1 := 5Lux], 100 means 500 Lux.
 *        0x8000, error
 *
 * NOTES:
 * the data conversion fomula is :
 *      ReadValue / 1.2 [LUX]
 *
 ****************************************************************************/
PUBLIC bool_t bADXL345FIFOreadResult( int16* ai16accelx, int16* ai16accely, int16* ai16accelz )
{
	bool_t	bOk = TRUE;
	uint8	au8data[6];
	uint8	i=0;

	//	FIFOでたまった個数を読み込む
	bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_FIFO_STATUS, 0, NULL );
	bOk &= bSMBusSequentialRead( ADXL345_ADDRESS, 1, &u8num );

	//	FIFOの中身を全部読む
	u8num = (u8num&0x7f);

	//	各軸の読み込み
	for( i=0; i<u8num; i++ ){
		//	加速度を読み込む
		GetAxis(bOk, au8data);
		ai16accelx[i] = ((au8data[1] << 8) | au8data[0])<<2;
		ai16accely[i] = ((au8data[3] << 8) | au8data[2])<<2;
		ai16accelz[i] = ((au8data[5] << 8) | au8data[4])<<2;
	}


	//	終わり

    return bOk;
}

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/
// the Main loop
PRIVATE void vProcessSnsObj_ADXL345_FIFO(void *pvObj, teEvent eEvent) {
	tsSnsObj *pSnsObj = (tsSnsObj *)pvObj;
	tsObjData_ADXL345 *pObj = (tsObjData_ADXL345 *)pSnsObj->pvData;

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
vfPrintf(&sDebugStream, "\n\rADXL345 WAKEUP");
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
			vfPrintf(&sDebugStream, "\n\rADXL345 KICKED");
			#endif

			break;

		default:
			break;
		}
		break;

	case E_SNSOBJ_STATE_MEASURING:
		switch (eEvent) {
		case E_EVENT_NEW_STATE:
			pObj->u8Interrupt = u8Interrupt;
			pObj->ai16ResultX[0] = ADXL345_DATA_NOTYET;
			pObj->ai16ResultY[0] = ADXL345_DATA_NOTYET;
			pObj->ai16ResultZ[0] = ADXL345_DATA_NOTYET;
			pObj->u8TickWait = ADXL345_CONVTIME;

			pObj->bBusy = TRUE;
#ifdef ADXL345_ALWAYS_RESET
			u8reset_flag = TRUE;
			if (!bADXL345reset()) {
				vSnsObj_NewState(pSnsObj, E_SNSOBJ_STATE_COMPLETE);
			}
#else
			//if (!bADXL345startRead()) { // kick I2C communication
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
#ifdef ADXL345_ALWAYS_RESET
			if (u8reset_flag) {
				u8reset_flag = 0;
				if (!bADXL345startRead()) {
					vADXL345_new_state(pObj, E_SNSOBJ_STATE_COMPLETE);
				}

				pObj->u8TickCount = 0;
				pObj->u8TickWait = ADXL345_CONVTIME;
				break;
			}
#endif

			bADXL345FIFOreadResult( pObj->ai16ResultX, pObj->ai16ResultY, pObj->ai16ResultZ);
			pObj->u8FIFOSample = u8num;

			// data arrival
			pObj->bBusy = FALSE;
			vSnsObj_NewState(pSnsObj, E_SNSOBJ_STATE_COMPLETE);
		}
		break;

	case E_SNSOBJ_STATE_COMPLETE:
		switch (eEvent) {
		case E_EVENT_NEW_STATE:
			#ifdef SERIAL_DEBUG
			vfPrintf(&sDebugStream, "\n\rADXL345_CP: %d", pObj->i16Result);
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
