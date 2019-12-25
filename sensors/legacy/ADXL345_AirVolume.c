/* Copyright (C) 2017 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include<math.h>

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

#undef SERIAL_DEBUG
#ifdef SERIAL_DEBUG
# include <serial.h>
# include <fprintf.h>
extern tsFILE sDebugStream;
#endif
tsFILE sSerStream;

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
//PRIVATE bool_t bGetAxis( uint8 u8axis, uint8* au8data );
PRIVATE void vProcessSnsObj_ADXL345_AirVolume(void *pvObj, teEvent eEvent);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
void vADXL345_AirVolume_Init(tsObjData_ADXL345 *pData, tsSnsObj *pSnsObj) {
	vSnsObj_Init(pSnsObj);

	pSnsObj->pvData = (void*)pData;
	pSnsObj->pvProcessSnsObj = (void*)vProcessSnsObj_ADXL345_AirVolume;

	memset((void*)pData, 0, sizeof(tsObjData_ADXL345));
}

void vADXL345_AirVolume_Final(tsObjData_ADXL345 *pData, tsSnsObj *pSnsObj) {
	pSnsObj->u8State = E_SNSOBJ_STATE_INACTIVE;
}

//	センサの設定を記述する関数
bool_t bADXL345_AirVolume_Setting()
{
	bool_t bOk = TRUE;

	//uint8 com = 0x0A;		//	100Hz Sampling frequency
	uint8 com = 0x09;		//	50Hz Sampling frequency
	bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_BW_RATE, 1, &com );
	com = 0x0B;		//	Full Resolution Mode, +-16g
	bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_DATA_FORMAT, 1, &com );
	com = 0x08;		//	Start Measuring
	bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_POWER_CTL, 1, &com );
	bOk &= bSetActive();

	return bOk;
}
/****************************************************************************
 *
 * NAME: u16ADXL345_AirVolumereadResult
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
PUBLIC bool_t b16ADXL345_AirVolumeReadResult( int16* ai16accel )
{
	bool_t	bOk = TRUE;
	uint8	au8data[6];
	uint8	num;				//	FIFOのデータ数
	uint8	i;
	int16	x[33];
	int16	y[33];
	int16	z[33];
	int32	avex = 0;
	int32	avey = 0;
	int32	avez = 0;

	//	FIFOでたまった個数を読み込む
	bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_FIFO_STATUS, 0, NULL );
	bOk &= bSMBusSequentialRead( ADXL345_ADDRESS, 1, &num );

	//	FIFOの中身を全部読む
	num = (num&0x7f);
	if( num == READ_FIFO_AIR ){
		//	各軸の読み込み
		for( i=0; i<num; i++ ){
			//	X軸
			GetAxis( bOk, au8data );
			x[i] = (((au8data[1] << 8) | au8data[0]));
			y[i] = (((au8data[3] << 8) | au8data[2]));
			z[i] = (((au8data[5] << 8) | au8data[4]));
		}

		for( i=0; i<num; i++ ){
			x[i] = (x[i]<<2);
			avex += x[i];
			y[i] = (y[i]<<2);
			avey += y[i];
			z[i] = (z[i]<<2);
			avez += z[i];
#if 0
			vfPrintf(& sSerStream, "\n\r%2d:%d,%d,%d", i, x[i], y[i], z[i] );
			SERIAL_vFlush(E_AHI_UART_0);
		}
		vfPrintf( &sSerStream, "\n\r" );
#else
		}
#endif

		ai16accel[0] = avex/num;
		ai16accel[1] = avey/num;
		ai16accel[2] = avez/num;
	}else{
		ai16accel[0] = 0;
		ai16accel[1] = 0;
		ai16accel[2] = 0;
	}

	//	終わり

    return bOk;
}

PUBLIC bool_t b16ADXL345_AirVolumeSingleReadResult( int16* ai16accel )
{
	bool_t	bOk = TRUE;
	uint8	au8data[6];

	GetAxis( bOk, au8data );
	//	X軸
	ai16accel[0] = (((au8data[1] << 8) | au8data[0]));
	ai16accel[0] = (ai16accel[0]<<2);
	//	Y軸
	ai16accel[1] = (((au8data[3] << 8) | au8data[2]));
	ai16accel[1] = (ai16accel[1]<<2);
	//	Z軸
	ai16accel[2] = (((au8data[5] << 8) | au8data[4]));
	ai16accel[2] = (ai16accel[2]<<2);

    return bOk;
}
/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/
bool_t bSetFIFO_Air( void )
{
	uint8 com;
	bool_t bOk = TRUE;

	//	FIFOの設定をもう一度
//	com = 0x00 | 0x20 | READ_FIFO_AIR;
//	bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_FIFO_CTL, 1, &com );
	com = 0xC0 | 0x20 | READ_FIFO_AIR;
	bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_FIFO_CTL, 1, &com );
	//	有効にする割り込みの設定
	com = 0x02;
	bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_INT_ENABLE, 1, &com );
	//	終わり

    return bOk;
}

bool_t bSetActive(void)
{
	uint8 com;
	bool_t bOk = TRUE;
	// FIFOを止める
	com = 0x00 | 0x20 | READ_FIFO_AIR;
	bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_FIFO_CTL, 1, &com );
	//	動いていることを判断するための閾値
	com = 0x07;
	bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_THRESH_ACT, 1, &com );

	com = 0x60;
	bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_ACT_INACT_CTL, 1, &com );

	//	割り込みピンの設定
	com = 0x10;		//	ACTIVEは別ピン
	bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_INT_MAP, 1, &com );

	//	有効にする割り込みの設定
	com = 0x10;
	bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_INT_ENABLE, 1, &com );

    return bOk;
}

// the Main loop
PRIVATE void vProcessSnsObj_ADXL345_AirVolume(void *pvObj, teEvent eEvent) {
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
			pObj->ai16Result[ADXL345_IDX_X] = SENSOR_TAG_DATA_ERROR;
			pObj->ai16Result[ADXL345_IDX_Y] = SENSOR_TAG_DATA_ERROR;
			pObj->ai16Result[ADXL345_IDX_Z] = SENSOR_TAG_DATA_ERROR;
			pObj->u8TickWait = ADXL345_CONVTIME;

			pObj->bBusy = TRUE;
#ifdef ADXL345_ALWAYS_RESET
			u8reset_flag = TRUE;
			if (!bADXL345reset()) {
				vSnsObj_NewState(pSnsObj, E_SNSOBJ_STATE_COMPLETE);
			}
#else
			if (!bADXL345startRead()) { // kick I2C communication
				vSnsObj_NewState(pSnsObj, E_SNSOBJ_STATE_COMPLETE);
			}
#endif
			pObj->u8TickCount = 0;
			break;

		default:
			break;
		}

		// wait until completion
		if (pObj->u8TickCount > pObj->u8TickWait) {
			if( (pObj->u8Interrupt&0x02) != 0 ){
				b16ADXL345_AirVolumeReadResult( pObj->ai16Result );
			}else{
				b16ADXL345_AirVolumeSingleReadResult( pObj->ai16Result );
			}

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
