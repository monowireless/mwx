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
PRIVATE void vProcessSnsObj_ADXL345_LowEnergy(void *pvObj, teEvent eEvent);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
void vADXL345_LowEnergy_Init(tsObjData_ADXL345 *pData, tsSnsObj *pSnsObj) {
	vSnsObj_Init(pSnsObj);

	pSnsObj->pvData = (void*)pData;
	pSnsObj->pvProcessSnsObj = (void*)vProcessSnsObj_ADXL345_LowEnergy;

	memset((void*)pData, 0, sizeof(tsObjData_ADXL345));
}

//	センサの設定を記述する関数
bool_t bADXL345_LowEnergy_Setting()
{
	bool_t bOk = TRUE;

	uint8 com = 0x0A;		//	100Hz Sampling frequency
	bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_BW_RATE, 1, &com );
	com = 0x0B;		//	Full Resolution Mode, +-16g
	bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_DATA_FORMAT, 1, &com );
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
PUBLIC bool_t bADXL345_LowEnergyStartRead()
{

	uint8 com = 0x08;		//	Start Measuring
	bool_t bOk = bSMBusWrite(ADXL345_ADDRESS, ADXL345_POWER_CTL, 1, &com );

	return bOk;
}

/****************************************************************************
 *
 * NAME: u16ADXL345_LowEnergyreadResult
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
PUBLIC bool_t bADXL345_LowEnergyReadResult( int16* ai16accel )
{
	bool_t	bOk = TRUE;
	uint8	au8data[6];

	//	各軸の読み込み
	GetAxis(bOk, au8data);
	if (bOk) {
		ai16accel[ADXL345_IDX_X] = (int16)((au8data[1] << 8) | au8data[0]);
		ai16accel[ADXL345_IDX_X] = ai16accel[ADXL345_IDX_X]*4/10;			//	1bitあたり4mg  10^-2まで有効
		ai16accel[ADXL345_IDX_Y] = (int16)((au8data[3] << 8) | au8data[2]);
		ai16accel[ADXL345_IDX_Y] = ai16accel[ADXL345_IDX_Y]*4/10;			//	1bitあたり4mg  10^-2まで有効
		ai16accel[ADXL345_IDX_Z] = (int16)((au8data[5] << 8) | au8data[4]);
		ai16accel[ADXL345_IDX_Z] = ai16accel[ADXL345_IDX_Z]*4/10;			//	1bitあたり4mg  10^-2まで有効
	}

	return bOk;
}

// the Main loop
PRIVATE void vProcessSnsObj_ADXL345_LowEnergy(void *pvObj, teEvent eEvent) {
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
			pObj->ai16Result[ADXL345_IDX_X] = SENSOR_TAG_DATA_ERROR;
			pObj->ai16Result[ADXL345_IDX_Y] = SENSOR_TAG_DATA_ERROR;
			pObj->ai16Result[ADXL345_IDX_Z] = SENSOR_TAG_DATA_ERROR;
			pObj->u8TickWait = ADXL345_LOWENERGY_CONVTIME;

			pObj->bBusy = TRUE;
#ifdef ADXL345_ALWAYS_RESET
			u8reset_flag = TRUE;
			if (!bADXL345reset()) {
				vSnsObj_NewState(pSnsObj, E_SNSOBJ_STATE_COMPLETE);
			}
#else
			if (!bADXL345_LowEnergyStartRead()) { // kick I2C communication
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
			bADXL345_LowEnergyReadResult( pObj->ai16Result );

			uint8 com = 0x07;		//	End Measuring
			bSMBusWrite(ADXL345_ADDRESS, ADXL345_POWER_CTL, 1, &com );

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
