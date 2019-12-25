/* Copyright (C) 2017 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include "jendefs.h"
#include "AppHardwareApi.h"
#include "string.h"

#include "sensor_driver.h"
#include "LTR308ALS.h"
#include "SMBus.h"

#include "ccitt8.h"

#include <math.h>

#undef SERIAL_DEBUG
#ifdef SERIAL_DEBUG
# include <serial.h>
# include <fprintf.h>
extern tsFILE sDebugStream;
#endif

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#define LTR308ALS_ADDRESS		(0x53)

#define LTR308ALS_MAIN_CTRL		(0x00)
#define LTR308ALS_MEAS_RATE		(0x04)
#define LTR308ALS_GAIN			(0x05)
#define LTR308ALS_PART_ID		(0x06)
#define LTR308ALS_MAIN_STATUS	(0x07)

#define LTR308ALS_DATA_0		(0x0D)
#define LTR308ALS_DATA_1		(0x0E)
#define LTR308ALS_DATA_2		(0x0F)

#define LTR308ALS_INT_CFG		(0x19)
#define LTR308ALS_INT_PST		(0x1A)

#define LTR308ALS_THRES_UP_0	(0x21)
#define LTR308ALS_THRES_UP_1	(0x22)
#define LTR308ALS_THRES_UP_2	(0x23)
#define LTR308ALS_THRES_LOW_0	(0x24)
#define LTR308ALS_THRES_LOW_1	(0x25)
#define LTR308ALS_THRES_LOW_2	(0x26)

#define LTR308ALS_CONVTIME		(50) // 100ms

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
PRIVATE void vProcessSnsObj_LTR308ALS(void *pvObj, teEvent eEvent);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

void vLTR308ALS_Init(tsObjData_LTR308ALS *pData, tsSnsObj *pSnsObj) {
	vSnsObj_Init(pSnsObj);

	pSnsObj->pvData = (void*)pData;
	pSnsObj->pvProcessSnsObj = vProcessSnsObj_LTR308ALS;

	memset((void*)pData, 0, sizeof(tsObjData_LTR308ALS));
}

void vLTR308ALS_Final(tsObjData_LTR308ALS *pData, tsSnsObj *pSnsObj) {
	pSnsObj->u8State = E_SNSOBJ_STATE_INACTIVE;
}

/****************************************************************************
 *
 * NAME: vLTR308ALSreset
 *
 * DESCRIPTION:
 *   to reset LTR308ALS device
 *
 * RETURNS:
 * bool_t	fail or success
 *
 ****************************************************************************/
PUBLIC bool_t bLTR308ALSreset()
{
	bool_t bOk = TRUE;
	uint8 u8data = 0x10;
	bOk &= bSMBusWrite(LTR308ALS_ADDRESS, LTR308ALS_MAIN_CTRL, 1, &u8data );

	return bOk;
}

/****************************************************************************
 *
 * NAME: vHTSstartReadTemp
 *
 * DESCRIPTION:
 * Wrapper to start a read of the temperature sensor.
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC bool_t bLTR308ALSstartRead()
{
	bool_t bOk = TRUE;
	// 照度センサの設定
	uint8 u8data = 0x00;
	bSMBusWrite(LTR308ALS_ADDRESS, LTR308ALS_GAIN, 1, &u8data );
	u8data = 0x32;
	bOk &= bSMBusWrite(LTR308ALS_ADDRESS, LTR308ALS_MEAS_RATE, 1, &u8data );
	u8data = 0x02;
	bOk &= bSMBusWrite(LTR308ALS_ADDRESS, LTR308ALS_MAIN_CTRL, 1, &u8data );

	return bOk;
}

/****************************************************************************
 *
 * NAME: u16LTR308ALSreadResult
 *
 * DESCRIPTION:
 * Wrapper to read a measurement, followed by a conversion function to work
 * out the value in degrees Celcius.
 *
 * RETURNS:
 * int16: temperature in degrees Celcius x 100 (-4685 to 12886)
 *        0x8000, error
 *
 * NOTES:
 * the data conversion fomula is :
 *      TEMP:  -46.85+175.72*ReadValue/65536
 *      HUMID: -6+125*ReadValue/65536B;
 *
 *    where the 14bit ReadValue is scaled up to 16bit
 *
 ****************************************************************************/
PUBLIC uint32 u32LTR308ALSreadResult( void )
{
	bool_t bOk = TRUE;
    uint32 u32result = 0x00;
    uint8 u8data;

	float fData;

	bOk &= bSMBusWrite(LTR308ALS_ADDRESS, LTR308ALS_DATA_0, 0, NULL );
    bOk &= bSMBusSequentialRead(LTR308ALS_ADDRESS, 1, &u8data);
	u32result = u8data;
	bOk &= bSMBusWrite(LTR308ALS_ADDRESS, LTR308ALS_DATA_1, 0, NULL );
    bOk &= bSMBusSequentialRead(LTR308ALS_ADDRESS, 1, &u8data);
	u32result |= u8data<<8;
	bOk &= bSMBusWrite(LTR308ALS_ADDRESS, LTR308ALS_DATA_2, 0, NULL );
    bOk &= bSMBusSequentialRead(LTR308ALS_ADDRESS, 1, &u8data);
	u32result |= u8data<<16;
	fData = (float)u32result;

	// センサをスタンバイモードにする
	u8data = 0x00;
	bSMBusWrite(LTR308ALS_ADDRESS, LTR308ALS_MAIN_CTRL, 1, &u8data );

	if(!bOk){
		return LTR308ALS_DATA_ERROR;
	}

	fData = (fData*0.6*2.0);
	u32result = (uint32)fData;

    return u32result;
}

PUBLIC bool_t bLTR308ALSdataArrived()
{
	uint8 u8data = 0xFF;
	bool_t bOk = bSMBusWrite(LTR308ALS_ADDRESS, LTR308ALS_MAIN_STATUS, 0, NULL );
	bOk &= bSMBusSequentialRead( LTR308ALS_ADDRESS, 1, &u8data );

	// 通信エラーが起きた場合、データを読ませて異常があることを親に教える
	if(!bOk){
		return FALSE;
	}
	
	if( u8data&0x08 ){
		return TRUE;
	}

	return FALSE;
}
/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/
// the Main loop
void vProcessSnsObj_LTR308ALS(void *pvObj, teEvent eEvent) {
	tsSnsObj *pSnsObj = (tsSnsObj *)pvObj;
	tsObjData_LTR308ALS *pObj = (tsObjData_LTR308ALS *)pSnsObj->pvData;

	// general process
	switch (eEvent) {
		case E_EVENT_TICK_TIMER:
			if (pObj->u8TickCount < 100) {
				pObj->u8TickCount += pSnsObj->u8TickDelta;
			}
			break;
		case E_EVENT_START_UP:
			pObj->u8TickCount = 100; // expire immediately
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
			break;

		default:
			break;
		}
		break;

	case E_SNSOBJ_STATE_MEASURING:
		switch (eEvent) {
		case E_EVENT_NEW_STATE:

			pObj->u32Result = LTR308ALS_DATA_NOTYET;
			pObj->u8TickWait = LTR308ALS_CONVTIME;

			// kick I2C communication
			if (!bLTR308ALSstartRead()) {
				vSnsObj_NewState(pSnsObj, E_SNSOBJ_STATE_COMPLETE); // error
			}

			pObj->u8TickCount = 0;
			break;

		default:
			break;
		}

		// wait until completion
//		if (pObj->u8TickCount > pObj->u8TickWait) {
		if (bLTR308ALSdataArrived()) {
			uint32 u32ret = u32LTR308ALSreadResult();
			pObj->u32Result = u32ret;

			if (u32ret == LTR308ALS_DATA_ERROR) {
				vSnsObj_NewState(pSnsObj, E_SNSOBJ_STATE_COMPLETE); // error
			} else
			if (u32ret == LTR308ALS_DATA_NOTYET) {
				pObj->u8TickCount /= 2; // wait more...
			} else {
				// data arrival
				vSnsObj_NewState(pSnsObj, E_SNSOBJ_STATE_COMPLETE);
			}

		}
		break;

	case E_SNSOBJ_STATE_COMPLETE:
		switch (eEvent) {
		case E_EVENT_NEW_STATE:
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
