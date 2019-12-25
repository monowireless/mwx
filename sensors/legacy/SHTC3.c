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
#include "SHTC3.h"
#include "SMBus.h"

#include "ccitt8.h"

#undef SERIAL_DEBUG
#ifdef SERIAL_DEBUG
# include <serial.h>
# include <fprintf.h>
#define sDebugStream _sSerLegacy
extern tsFILE sDebugStream;
#endif

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

#define LOWPOWER
#ifndef LOWPOWER 
	#define SHTC3_TRIG_H	(0x78)
	#define SHTC3_TRIG_L	(0x66)
#else
	// Low Power Mode
	#define SHTC3_TRIG_H	(0x60)
	#define SHTC3_TRIG_L	(0x9C)
#endif

#define SHTC3_SLEEP_H	(0xB0)
#define SHTC3_SLEEP_L	(0x98)

#define SHTC3_WAKEUP_H	(0x35)
#define SHTC3_WAKEUP_L	(0x17)

#define SHTC3_SOFT_RST_H	(0x80)
#define SHTC3_SOFT_RST_L	(0x5)

#ifndef LOWPOWER 
	#define SHTC3_CONVTIME		(15) // 15ms
#else
	#define SHTC3_CONVTIME		(2) // 2ms
#endif

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
PRIVATE void vProcessSnsObj_SHTC3(void *pvObj, teEvent eEvent);
PRIVATE uint8 u8CRC8( uint8* u8buf, uint8 u8len);

extern void vWait();

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
void vSHTC3_Init(tsObjData_SHTC3 *pData, tsSnsObj *pSnsObj) {
	vSnsObj_Init(pSnsObj);

	pSnsObj->pvData = (void*)pData;
	pSnsObj->pvProcessSnsObj = (void*)vProcessSnsObj_SHTC3;

	memset((void*)pData, 0, sizeof(tsObjData_SHTC3));

#ifdef SERIAL_DEBUG
	vfPrintf(&sDebugStream, "\n\rSHTC3 INIT DATA:%x PRCS:%x DELTA:%d", 
		pSnsObj->pvData, pSnsObj->pvProcessSnsObj, pSnsObj->u8TickDelta);
#endif
#if 0
	bSHTC3wakeup();
	vWait(600);
#endif
}

void vSHTC3_Final(tsObjData_SHTC3 *pData, tsSnsObj *pSnsObj) {
	pSnsObj->u8State = E_SNSOBJ_STATE_INACTIVE;
}

/****************************************************************************
 *
 * NAME: vSHTC3reset
 *
 * DESCRIPTION:
 *   to reset SHTC3 device
 *
 * RETURNS:
 * bool_t	fail or success
 *
 ****************************************************************************/
PUBLIC bool_t bSHTC3reset()
{
	bool_t bOk = TRUE;
	uint8 u8data = SHTC3_SOFT_RST_L;

	bOk &= bSMBusWrite(SHTC3_ADDRESS, SHTC3_SOFT_RST_H, 1, &u8data );
	// then will need to wait at least 15ms

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
PUBLIC bool_t bSHTC3startRead()
{
	bool_t bOk = TRUE;

	// start conversion (will take some ms according to bits accuracy)
	uint8 u8data = SHTC3_TRIG_L;
	bOk &= bSMBusWrite(SHTC3_ADDRESS, SHTC3_TRIG_H, 1, &u8data );

	return bOk;
}

/****************************************************************************
 *
 * NAME: u16SHTC3readResult
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
 *      HUMID: -6+125*ReadValue/65536
 *
 *    where the 14bit ReadValue is scaled up to 16bit
 *
 ****************************************************************************/
PUBLIC int16 i16SHTC3readResult(int16 *pi16Temp, int16 *pi16Humid)
{
	bool_t bOk = TRUE;
    int32 i32result;
    uint32 u32result;
    uint8 au8data[6];

    bOk &= bSMBusSequentialRead(SHTC3_ADDRESS, 6, au8data);
    if(!bOk) return SHTC3_DATA_NOTYET; // error
	bSHTC3sleep();

	// CRC8 check
	uint8 u8crc = u8CRC8(au8data, 2);
	if (au8data[2] != u8crc) return SHTC3_DATA_ERROR;

	u8crc = u8CRC8(au8data+3,2);
	if (au8data[5] != u8crc) return SHTC3_DATA_ERROR;

    i32result = au8data[1] | (au8data[0] << 8);
	if(pi16Temp) *pi16Temp = (int16_t)(-4500 + ((17500 * i32result) >> 16));
	else return SHTC3_DATA_ERROR;

    u32result = au8data[4] | (au8data[3] << 8);
	if(pi16Humid) *pi16Humid = (int16_t)((u32result * 10000) >> 16);
	else return SHTC3_DATA_ERROR;

    return i32result;
}

uint8 u8CRC8( uint8* buf, uint8 u8len )
{
	uint8 u8crc = 0xFF;
	uint8 u8GP = 0x31;	// X8+X5+X4+1
	uint8 i = 0;
	uint8 j = 0;

	for( i=0; i<u8len; i++ ){
		u8crc ^= buf[i];
		for( j=0;j<8;j++ ){
			if( u8crc&0x80 ){
				u8crc <<= 1;
				u8crc ^= u8GP;
			}else{
				u8crc <<= 1;
			}
		}
	}
	return u8crc;
}

PUBLIC bool_t bSHTC3sleep(){
	bool_t bOk = TRUE;

	uint8 u8data = SHTC3_SLEEP_L;
	bOk &= bSMBusWrite(SHTC3_ADDRESS, SHTC3_SLEEP_H, 1, &u8data );

	return bOk;
}

PUBLIC bool_t bSHTC3wakeup(){
	bool_t bOk = TRUE;

	uint8 u8data = SHTC3_WAKEUP_L;
	bOk &= bSMBusWrite(SHTC3_ADDRESS, SHTC3_WAKEUP_H, 1, &u8data );
	
	return bOk;
}
/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/
// the Main loop
void vProcessSnsObj_SHTC3(void *pvObj, teEvent eEvent) {
	tsSnsObj *pSnsObj = (tsSnsObj *)pvObj;
	tsObjData_SHTC3 *pObj = (tsObjData_SHTC3 *)(pSnsObj->pvData);

#ifdef SERIAL_DEBUG
	//vfPrintf(&sDebugStream, "<s%x>", pObj);
#endif
	// general process
	switch (eEvent) {
		case E_EVENT_TICK_TIMER:
			if (pObj->u8TickCount < 100) {
				pObj->u8TickCount += pSnsObj->u8TickDelta;
#ifdef SERIAL_DEBUG
//vfPrintf(&sDebugStream, "+(%d/%d)", pObj->u8TickCount, pSnsObj->u8TickDelta);
#endif
			}
			break;
		case E_EVENT_START_UP:
			pObj->u8TickCount = 100; // expire immediately
#ifdef SERIAL_DEBUG
vfPrintf(&sDebugStream, "\n\rSHTC3 WAKEUP");
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
#ifdef SERIAL_DEBUG
			vfPrintf(&sDebugStream, "\n\rSHTC3 IDLE NEW");
#endif
			break;

		case E_ORDER_KICK:
			vSnsObj_NewState(pSnsObj, E_SNSOBJ_STATE_MEASURING);

			#ifdef SERIAL_DEBUG
			vfPrintf(&sDebugStream, "\n\rSHTC3 KICKED");
			#endif
			break;

		default:
			break;
		}
		break;

	case E_SNSOBJ_STATE_MEASURING:
		switch (eEvent) {
		case E_EVENT_NEW_STATE:
			bSHTC3wakeup(); // needs waking up.
			vWait(600);

			pObj->ai16Result[SHTC3_IDX_TEMP] = SENSOR_TAG_DATA_ERROR;
			pObj->ai16Result[SHTC3_IDX_HUMID] = SENSOR_TAG_DATA_ERROR;
			pObj->u8TickWait = SHTC3_CONVTIME;

#ifdef SERIAL_DEBUG
			vfPrintf(&sDebugStream, "\r\nSHTC3 start");
#endif
			// kick I2C communication
			if (!bSHTC3startRead()) {
				vSnsObj_NewState(pSnsObj, E_SNSOBJ_STATE_COMPLETE); // error
#ifdef SERIAL_DEBUG
				vfPrintf(&sDebugStream, "\r\n!bSHTC3startRead()");
#endif
			}


			pObj->u8TickCount = 0;
			break;

		default:
			break;
		}

		// wait until completion
		if (pObj->u8TickCount > pObj->u8TickWait) {
			int16 i16ret;
			i16ret = i16SHTC3readResult(
					&(pObj->ai16Result[SHTC3_IDX_TEMP]),
					&(pObj->ai16Result[SHTC3_IDX_HUMID]) );

#ifdef SERIAL_DEBUG
			vfPrintf(&sDebugStream, "\r\nSHT RESULT ADDR = %X (%X)", pObj->ai16Result, pObj);
#endif

			if (i16ret == SENSOR_TAG_DATA_ERROR) {
				vSnsObj_NewState(pSnsObj, E_SNSOBJ_STATE_COMPLETE); // error
			} else
			if (i16ret == SENSOR_TAG_DATA_NOTYET) {
				// still conversion
				#ifdef SERIAL_DEBUG
				vfPrintf(&sDebugStream, "\r\nSHT_ND");
				#endif

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
			#ifdef SERIAL_DEBUG
			vfPrintf(&sDebugStream, "\n\rSHT_CP: T%d H%d",
					pObj->ai16Result[SHTC3_IDX_TEMP],
					pObj->ai16Result[SHTC3_IDX_HUMID]);
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
