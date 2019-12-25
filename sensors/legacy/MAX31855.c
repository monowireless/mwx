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
#include "MAX31855.h"
#include "SPI.h"

#undef SERIAL_DEBUG
#ifdef SERIAL_DEBUG
# include <serial.h>
# include <fprintf.h>
extern tsFILE sDebugStream;
#endif

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#define MAX31855_DATA_NOTYET  (-32768)
#define MAX31855_DATA_ERROR   (-32767)

//	有効にするポート数の指定
#define SLAVE_ENABLE1		(0)			//	DIO19を用いる
#define SLAVE_ENABLE2		(1)			//	DIO19とDIO0を用いる
#define SLAVE_ENABLE3		(2)			//	DIO19,0,1を用いる

//	ChipSelect
#define CS_DIO19			(0x01)		//	DIO19に接続したものを使う
#define CS_DIO0				(0x02)		//	DIO0に接続したものを使う
#define CS_DIO1				(0x04)		//	DIO1に接続したものを使

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
PRIVATE void vProcessSnsObj_MAX31855(void *pvObj, teEvent eEvent);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
void vMAX31855_Init(tsObjData_MAX31855 *pData, tsSnsObj *pSnsObj) {
	vSnsObj_Init(pSnsObj);

	pSnsObj->pvData = (void*)pData;
	pSnsObj->pvProcessSnsObj = (void*)vProcessSnsObj_MAX31855;

	memset((void*)pData, 0, sizeof(tsObjData_MAX31855));
}

void vMAX31855_Final(tsObjData_MAX31855 *pData, tsSnsObj *pSnsObj) {
	pSnsObj->u8State = E_SNSOBJ_STATE_INACTIVE;
}

/****************************************************************************
 *
 * NAME: vMAX31855reset
 *
 * DESCRIPTION:
 *   to reset MAX31855 device
 *
 * RETURNS:
 * bool_t	fail or success
 *
 ****************************************************************************/
PUBLIC bool_t bMAX31855reset()
{
    /* configure SPI interface */
	/*	SPI Mode1	*/
	vSPIInit( SPI_MODE1, SLAVE_ENABLE1, 2 );	// 16/(2*u8ClockDivider) Mhz

	return TRUE;
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
PUBLIC bool_t bMAX31855startRead(uint8 u8trig)
{
	return TRUE;
}

/****************************************************************************
 *
 * NAME: u16MAX31855readResult
 *
 * DESCRIPTION:
 *
 * RETURNS:
 *
 ****************************************************************************/
PUBLIC int32 i32MAX31855readResult(int32 *pi32Temp, int32 *pi32ITemp)
{
	int32	i32result=SENSOR_TAG_DATA_ERROR;
	uint8	au8data[4];
	uint8	i;

	vSPIChipSelect(CS_DIO19);
	for(i=0;i<4;i++){
		vSPIWrite( au8data[i] );
		au8data[i] = u8SPIRead();
	}
	vSPIStop();

	bool_t bSigned = au8data[0]&0x80 ? TRUE:FALSE;
	int16 i16Dec = ((au8data[1]&0x0C)>>2)*25;

	i32result = ((au8data[0]&0xFF)<<8)+(au8data[1]&0xF0);
	if(bSigned){
		uint16 temp = i32result&0xFFFF;
		temp = ~temp + 1;   // 正数に戻す
		temp = temp>>4;
		i32result = temp*(-100);
	}else{
		i32result = (i32result>>4)*100;
	}
	i32result += i16Dec;
	*pi32Temp = i32result;

	bSigned = au8data[2]&0x80 ? TRUE:FALSE;
	i16Dec = ((au8data[3]&0xF0)>>4)*625;

	i32result = au8data[2];
	if(bSigned){
		uint8 temp = i32result&0xFF;
		temp = ~temp + 1;   // 正数に戻す
		i32result = temp*(-10000);
	}else{
		i32result = i32result*10000;
	}
	i32result += i16Dec;
	*pi32ITemp = i32result;

    return *pi32Temp;
}


/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/
// the Main loop
void vProcessSnsObj_MAX31855(void *pvObj, teEvent eEvent) {
	tsSnsObj *pSnsObj = (tsSnsObj *)pvObj;
	tsObjData_MAX31855 *pObj = (tsObjData_MAX31855 *)pSnsObj->pvData;

	// general process
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
vfPrintf(&sDebugStream, "\n\rMAX31855 WAKEUP");
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
			pObj->u8IdxMeasuruing = MAX31855_IDX_BEGIN;
			break;

		case E_ORDER_KICK:
			pObj->ai32Result[MAX31855_IDX_THERMOCOUPLE_TEMP] = SENSOR_TAG_DATA_ERROR;
			pObj->ai32Result[MAX31855_IDX_INTERNAL_TEMP] = SENSOR_TAG_DATA_ERROR;

			vSnsObj_NewState(pSnsObj, E_SNSOBJ_STATE_MEASURING);

			#ifdef SERIAL_DEBUG
			vfPrintf(&sDebugStream, "\n\rMAX31855 KICKED");
			#endif
			break;

		default:
			break;
		}
		break;

	case E_SNSOBJ_STATE_MEASURING:
		switch (eEvent) {
		case E_EVENT_NEW_STATE:
#ifdef SERIAL_DEBUG
vfPrintf(&sDebugStream, "\n\rSHT_ST:%d", pObj->u8IdxMeasuruing);
#endif

			pObj->ai32Result[pObj->u8IdxMeasuruing] = SENSOR_TAG_DATA_ERROR;
			pObj->u8TickWait = 0;

			// kick I2C communication
			if (!bMAX31855startRead(0)) {
				vSnsObj_NewState(pSnsObj, E_SNSOBJ_STATE_COMPLETE); // error
			}

			pObj->u8TickCount = 0;
			break;

		default:
			break;
		}

		// wait until completion
		if (pObj->u8TickCount > pObj->u8TickWait) {
			int32 i32ret;
			i32ret = i32MAX31855readResult(
					&(pObj->ai32Result[MAX31855_IDX_THERMOCOUPLE_TEMP]),
					&(pObj->ai32Result[MAX31855_IDX_INTERNAL_TEMP]) );

			if (i32ret == SENSOR_TAG_DATA_ERROR) {
				vSnsObj_NewState(pSnsObj, E_SNSOBJ_STATE_COMPLETE); // error
			} else
			if (i32ret == SENSOR_TAG_DATA_NOTYET) {
				// still conversion
				#ifdef SERIAL_DEBUG
				vfPrintf(&sDebugStream, "\r\nSHT_ND");
				#endif

				pObj->u8TickCount /= 2; // wait more...
			} else {
				// data arrival
				vSnsObj_NewState(pSnsObj, E_SNSOBJ_STATE_MEASURE_NEXT);
			}
		}
		break;

	case E_SNSOBJ_STATE_MEASURE_NEXT:
		switch (eEvent) {
			case E_EVENT_NEW_STATE:
				pObj->u8IdxMeasuruing++;

				if (pObj->u8IdxMeasuruing < MAX31855_IDX_END) {
					// complete all data
					vSnsObj_NewState(pSnsObj, E_SNSOBJ_STATE_MEASURING);
				} else {
					// complete all data
					vSnsObj_NewState(pSnsObj, E_SNSOBJ_STATE_COMPLETE);
				}
				break;

			default:
				break;
		}
		break;

	case E_SNSOBJ_STATE_COMPLETE:
		switch (eEvent) {
		case E_EVENT_NEW_STATE:
			#ifdef SERIAL_DEBUG
			vfPrintf(&sDebugStream, "\n\rSHT_CP: T%d H%d",
					pObj->ai16Result[MAX31855_IDX_TEMP],
					pObj->ai16Result[MAX31855_IDX_HUMID]);
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
