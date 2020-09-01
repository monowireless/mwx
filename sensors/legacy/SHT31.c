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
#include "SHT31.h"
#include "SMBus.h"

#include "ccitt8.h"

#undef SERIAL_DEBUG
#ifdef SERIAL_DEBUG
# include <serial.h>
# include <fprintf.h>
extern tsFILE sDebugStream;
#endif

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
//#define SHT31_ADDRESS     (0x44)
// #define SHT31_ADDRESS	(0x45)

#define SHT31_TRIG_H	(0x24)
#define SHT31_TRIG_L	(0x00)

#define SHT31_SOFT_RST_H	(0x30)
#define SHT31_SOFT_RST_L	(0xA2)

#define SHT31_CONVTIME		(15) // 15ms

#define SHT31_DATA_NOTYET	(-32768)
#define SHT31_DATA_ERROR	(-32767)

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
PRIVATE void vProcessSnsObj_SHT31(void *pvObj, teEvent eEvent);
PRIVATE uint8 u8CRC8( uint8* u8buf, uint8 u8len);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
static uint8 SHT31_ADDRESS = DEFAULT_SHT31_ADDRESS;

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
void vSHT31_Init(tsObjData_SHT31 *pData, tsSnsObj *pSnsObj) {
	vSnsObj_Init(pSnsObj);

	pSnsObj->pvData = (void*)pData;
	pSnsObj->pvProcessSnsObj = (void*)vProcessSnsObj_SHT31;

	memset((void*)pData, 0, sizeof(tsObjData_SHT31));
	
	// I2C address
	uint8 u8addr = (pSnsObj->u32Opt & 0xFF);
	if (u8addr) SHT31_ADDRESS = u8addr;

}

void vSHT31_Final(tsObjData_SHT31 *pData, tsSnsObj *pSnsObj) {
	pSnsObj->u8State = E_SNSOBJ_STATE_INACTIVE;
}

/****************************************************************************
 *
 * NAME: vSHT31reset
 *
 * DESCRIPTION:
 *   to reset SHT31 device
 *
 * RETURNS:
 * bool_t	fail or success
 *
 ****************************************************************************/
PUBLIC bool_t bSHT31reset()
{
	bool_t bOk = TRUE;
	uint8 u8data = SHT31_SOFT_RST_L;

	bOk &= bSMBusWrite(SHT31_ADDRESS, SHT31_SOFT_RST_H, 1, &u8data );
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
PUBLIC bool_t bSHT31startRead()
{
	bool_t bOk = TRUE;

	// start conversion (will take some ms according to bits accuracy)
	uint8 u8data = SHT31_TRIG_L;
	bOk &= bSMBusWrite(SHT31_ADDRESS, SHT31_TRIG_H, 1, &u8data );
#ifdef SERIAL_DEBUG
//vfPrintf(&sDebugStream, "\n\rSHT31 TRIG %x", u8trig);
#endif

	return bOk;
}

/****************************************************************************
 *
 * NAME: u16SHT31readResult
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
PUBLIC int16 i16SHT31readResult(int16 *pi16Temp, int16 *pi16Humid)
{
	bool_t bOk = TRUE;
    int32 i32result;
    uint16 u16result;
    uint8 au8data[6];

    bOk &= bSMBusSequentialRead(SHT31_ADDRESS, 6, au8data);
    if(!bOk) return SHT31_DATA_NOTYET; // error
#ifdef SERIAL_DEBUG
vfPrintf(&sDebugStream, "\n\rSHT_DT %x %x %x", au8data[0], au8data[1], au8data[2]);
#endif

	// CRC8 check
	uint8 u8crc = u8CRC8(au8data, 2);
	if (au8data[2] != u8crc) return SHT31_DATA_ERROR;

	u8crc = u8CRC8(au8data+3,2);
	if (au8data[5] != u8crc) return SHT31_DATA_ERROR;

    i32result = au8data[1] | (au8data[0] << 8);
	if(pi16Temp) *pi16Temp = (int16)( (-45.0+175.0*i32result/65535.0)*100.0 );
	else return SHT31_DATA_ERROR;

    u16result = au8data[4] | (au8data[3] << 8);
	if(pi16Humid) *pi16Humid = (int16)( (u16result/65535.0)*10000.0 );
	else return SHT31_DATA_ERROR;

#ifdef SERIAL_DEBUG
//vfPrintf(&sDebugStream, "\n\rSHT31 CORRECT DATA %s=%d",
//		(au8data[1] & 0x02) ? "HUMID" : "TEMP", i32result);
#endif

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

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/
// the Main loop
void vProcessSnsObj_SHT31(void *pvObj, teEvent eEvent) {
	tsSnsObj *pSnsObj = (tsSnsObj *)pvObj;
	tsObjData_SHT31 *pObj = (tsObjData_SHT31 *)pSnsObj->pvData;

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
vfPrintf(&sDebugStream, "\n\rSHT31 WAKEUP");
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
			vfPrintf(&sDebugStream, "\n\rSHT31 KICKED");
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

			pObj->ai16Result[SHT31_IDX_TEMP] = SENSOR_TAG_DATA_ERROR;
			pObj->ai16Result[SHT31_IDX_HUMID] = SENSOR_TAG_DATA_ERROR;
			pObj->u8TickWait = SHT31_CONVTIME;

			// kick I2C communication
			if (!bSHT31startRead()) {
				vSnsObj_NewState(pSnsObj, E_SNSOBJ_STATE_COMPLETE); // error
			}

			pObj->u8TickCount = 0;
			break;

		default:
			break;
		}

		// wait until completion
		if (pObj->u8TickCount > pObj->u8TickWait) {
			int16 i16ret;
			i16ret = i16SHT31readResult(
					&(pObj->ai16Result[SHT31_IDX_TEMP]),
					&(pObj->ai16Result[SHT31_IDX_HUMID]) );

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
					pObj->ai16Result[SHT31_IDX_TEMP],
					pObj->ai16Result[SHT31_IDX_HUMID]);
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
