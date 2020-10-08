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
#include "BME280.h"
#include "SMBus.h"

#include "ccitt8.h"

#include "utils.h"

// #define BME280_ALWAYS_RESET // not implemented so far.

#define SERIAL_DEBUG 0
#if SERIAL_DEBUG > 0
# include <serial.h>
# include <fprintf.h>
extern tsFILE _sSerLegacy;
extern tsFILE* psSer = &_sSerLegacy;
#endif

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#define CHIPMODEL_BME280 0x60
#define CHIPMODEL_BMP280 0x58

#define IS_BME280() (u8ChipModel == CHIPMODEL_BME280)
#define IS_BMP280() (u8ChipModel == CHIPMODEL_BMP280)

#define BME280_CONVTIME    (10)

#define BME280_CONFIG		0xF5
#define BME280_CTRL_MEAS	0xF4
#define BME280_STATUS		0xF3
#define BME280_CTRL_HUM		0xF2

#define BME280_DATA_NOTYET	(-32768)
#define BME280_DATA_ERROR	(-32767)

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
PRIVATE bool_t bReadTrim();
PRIVATE bool_t bReadData( uint8* pu8Data );
PRIVATE void vProcessSnsObj_BME280(void *pvObj, teEvent eEvent);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
static uint8 BME280_ADDRESS; // 0x76:DEFAULT 0x77:SECONDARY
static uint8 u8ChipModel;

static uint16 dig_T1;
static int16 dig_T2;
static int16 dig_T3;
static uint16 dig_P1;
static int16 dig_P2;
static int16 dig_P3;
static int16 dig_P4;
static int16 dig_P5;
static int16 dig_P6;
static int16 dig_P7;
static int16 dig_P8;
static int16 dig_P9;
static uint8 dig_H1;
static int16 dig_H2;
static uint8 dig_H3;
static int16 dig_H4;
static int16 dig_H5;
static int8 dig_H6;

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
void vBME280_Init(tsObjData_BME280 *pData, tsSnsObj *pSnsObj) {
	vSnsObj_Init(pSnsObj);
	memset((void*)pData, 0, sizeof(tsObjData_BME280));

	pSnsObj->pvData = (void*)pData;
	pSnsObj->pvProcessSnsObj = (void*)vProcessSnsObj_BME280;

	// I2C addr
	//   (pSnsObj->u32Opt & 0xFF) == 0 : default
	uint8_t u8addr = pSnsObj->u32Opt & 0xFF;
	BME280_ADDRESS = u8addr ? u8addr : DEFAULT_BME280_ADDRESS;

	// Chipmodel
	bool_t bOk = TRUE;
	bOk &= bSMBusWrite( BME280_ADDRESS, 0xD0, 0, NULL );
	bOk &= bSMBusSequentialRead( BME280_ADDRESS, 1, &u8ChipModel );
	pSnsObj->u32Stat |= u8ChipModel; // set chip model

	// init configure and read calibration.
	bBME280_Setting();

#if SERIAL_DEBUG >= 99
vfPrintf(psSer, "\n\rBME280 INIT: ADDR=%02X MODEL=%02X", BME280_ADDRESS, u8ChipModel );
#endif
}

void vBME280_Final(tsObjData_BME280 *pData, tsSnsObj *pSnsObj) {
	pSnsObj->u8State = E_SNSOBJ_STATE_INACTIVE;
}

//	センサの設定を記述する関数
bool_t bBME280_Setting()
{
	bool_t bOk = TRUE;

	uint8 com;
	com = (0x06<<5) | (0x00<<2) | 0x00;		//	WaitTime | Filter | 3 wires or 4 wires SPI
	bOk &= bSMBusWrite(BME280_ADDRESS, BME280_CONFIG, 1, &com );
	
	bReadTrim();

	return bOk;
}

/****************************************************************************
 *
 * NAME: bBME280reset
 *
 * DESCRIPTION:
 *   to reset BME280 device
 *
 * RETURNS:
 * bool_t	fail or success
 *
 ****************************************************************************/
PUBLIC bool_t bBME280Reset()
{
	bool_t bOk = TRUE;
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
PUBLIC bool_t bBME280StartRead()
{
#if SERIAL_DEBUG >= 99
vfPrintf(psSer, "\n\rBME280 bBME280StartRead");
#endif
	bool_t bOk = TRUE;
	uint8 com[4];
	
	// put small delay here
	//{ volatile int i = 1000; while(i > 0) --i; }

	if (IS_BME280()) {
		com[0] = 0x01;		//	Hum Enable
		bOk &= bSMBusWrite(BME280_ADDRESS, BME280_CTRL_HUM, 1, com );
	}

	com[0] = (0x01<<5) | (0x01<<2) | 0x01;		//	Temp Enable | Pres Enable | Mode (0x01:forced mode)
	bOk &= bSMBusWrite(BME280_ADDRESS, BME280_CTRL_MEAS, 1, com );

	return bOk;
}

/****************************************************************************
 *
 * NAME: u16BME280readResult
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

PUBLIC int16 i16BME280ReadResult( int16* Temp, uint16* Pres, uint16* Hum )
{
	int16	i16result=0;
	int32	i32Temp_raw;
	int32	i32Pres_raw;
	int32	i32Hum_raw;
	int32	i32var1, i32var2;
	int32	t_fine;

	uint8	_BM280_au8data[8]; // DEBUG
	uint8 *au8data = _BM280_au8data;

	bool_t bOk = bReadData(au8data);

	i32Pres_raw = au8data[0]<<12 | au8data[1]<<4 | au8data[2]>>4;
	i32Temp_raw = au8data[3]<<12 | au8data[4]<<4 | au8data[5]>>4;
	i32Hum_raw = au8data[6]<<8 | au8data[7];

#if SERIAL_DEBUG >= 99
vfPrintf(psSer, "\n\rBME280 RAW: P:%x T:%x H:%x", i32Pres_raw, i32Temp_raw, i32Hum_raw);
#endif

	if (bOk == FALSE) {
		i16result = SENSOR_TAG_DATA_ERROR;
	}

	//	ここからのアルゴリズムの詳細はデータシートの4.2.3 Compensation formulas, 8.2 Pressure compensation in 32 bit fixed point を参照
	//	Convert to Temperature
	i32var1 = ((((i32Temp_raw>>3)-((int32)dig_T1<<1))) * ((int32)dig_T2))>>11;
	i32var2 = (((((i32Temp_raw>>4)-((int32)dig_T1)) * ((i32Temp_raw>>4)-((int32)dig_T1)))>>12) * ((int32)dig_T3))>>14;
	t_fine = i32var1+i32var2;
	int16 i16Temp = (int16)((t_fine*5+128)>>8);
	*Temp = i16Temp;

	//	Convert to Pressure
	i32var1 = (t_fine>>1)-64000;
	i32var2 = ((i32var1>>2)*(i32var1>>2)>>11)*(int16)dig_P6;
	i32var2 = i32var2+((i32var1*dig_P5)<<1);
	i32var2 = (i32var2>>2)+(((int32)dig_P4)<<16);
	i32var1 = (((dig_P3 * (((i32var1>>2) * (i32var1>>2)) >> 13 )) >> 3) + ((((int32)dig_P2) * i32var1)>>1))>>18;
	i32var1 = ((((32768+i32var1))*((int32)dig_P1))>>15);

	uint16 u16Pres;
	uint32 u32Pres_Tmp;
	if (i32var1 == 0){
		u16Pres = 0;// avoid exception caused by division by zero
	}else{
		u32Pres_Tmp = (((uint32)(((int32)1048576)-i32Pres_raw)-(i32var2>>12)))*3125;
		if (u32Pres_Tmp < 0x80000000){
			u32Pres_Tmp = (u32Pres_Tmp << 1) / ((uint32)i32var1);
		}else{
			u32Pres_Tmp = (u32Pres_Tmp / (uint32)i32var1) * 2;
		}
		i32var1 = (((int32)dig_P9) * ((int32)(((u32Pres_Tmp>>3) * (u32Pres_Tmp>>3))>>13)))>>12;
		i32var2 = (((int32)(u32Pres_Tmp>>2)) * ((int32)dig_P8))>>13;
		u32Pres_Tmp = (uint32)((int32)u32Pres_Tmp + ((i32var1 + i32var2 + dig_P7) >> 4));
		u16Pres = (uint16)(u32Pres_Tmp/100);
	}
	*Pres = u16Pres;

	if (IS_BME280()) {
		// Convert to Humidity
		int32 i32Hum_Tmp = t_fine-76800;
		i32Hum_Tmp = (((((i32Hum_raw<<14)-(((int32)dig_H4) << 20)-(((int32)dig_H5)*i32Hum_Tmp))+((int32)16384))>>15)*(((((((i32Hum_Tmp*((int32)dig_H6))>>10)*(((i32Hum_Tmp*((int32)dig_H3))>>11)+((int32)32768)))>>10)+((int32)2097152))*((int32)dig_H2)+ 8192)>>14));
		i32Hum_Tmp = (i32Hum_Tmp-(((((i32Hum_Tmp>>15)*(i32Hum_Tmp>>15))>>7)*((int32)dig_H1))>> 4));
		i32Hum_Tmp = (i32Hum_Tmp<0 ? 0 : i32Hum_Tmp);
		i32Hum_Tmp = (i32Hum_Tmp>419430400 ? 419430400 : i32Hum_Tmp);
		uint16 u16Hum = (uint16)((i32Hum_Tmp>>12)/10);
		*Hum = u16Hum;
	}

	return i16result;
}

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/
PRIVATE bool_t bReadTrim()
{
	bool_t	bOk = TRUE;
	uint8	au8data[33];
	uint8*	p = au8data;

	bOk &= bSMBusWrite( BME280_ADDRESS, 0x88, 0, NULL );
	bOk &= bSMBusSequentialRead( BME280_ADDRESS, 26, p );

	dig_T1 = au8data[1]<<8 | au8data[0];
	dig_T2 = au8data[3]<<8 | au8data[2];
	dig_T3 = au8data[5]<<8 | au8data[4];

	dig_P1 = au8data[7]<<8 | au8data[6];
	dig_P2 = au8data[9]<<8 | au8data[8];
	dig_P3 = au8data[11]<<8 | au8data[10];
	dig_P4 = au8data[13]<<8 | au8data[12];
	dig_P5 = au8data[15]<<8 | au8data[14];
	dig_P6 = au8data[17]<<8 | au8data[16];
	dig_P7 = au8data[19]<<8 | au8data[18];
	dig_P8 = au8data[21]<<8 | au8data[20];
	dig_P9 = au8data[23]<<8 | au8data[22];

	if (IS_BME280()) {
		dig_H1 = au8data[25];

		p = au8data;
		bOk &= bSMBusWrite( BME280_ADDRESS, 0xE1, 0, NULL );
		bOk &= bSMBusSequentialRead( BME280_ADDRESS, 7, p );

		dig_H2 = (au8data[1]<<8) | au8data[0];
		dig_H3 = au8data[2];
		dig_H4 = (int16)((((int16) au8data[3] << 8) | (0x0F & au8data[4]) << 4) >> 4);
		dig_H5 = (int16)((((int16) au8data[5] << 8) | (0xF0 & au8data[4]) ) >> 4 );
		dig_H6 = au8data[6];
	}

	return bOk;
}

PRIVATE bool_t bReadData( uint8* pu8Data )
{
	bool_t	bOk = TRUE;

	bOk &= bSMBusWrite( BME280_ADDRESS, 0xF7, 0, NULL );
	bOk &= bSMBusSequentialRead( BME280_ADDRESS, IS_BME280() ? 8 : 6, pu8Data );

#if SERIAL_DEBUG >= 99
int i;
vfPrintf(psSer, "\r\nSNSDAT={", pu8Data[i]);
for (i = 0; i < 8; i++) {
	vfPrintf(psSer, "%02X", pu8Data[i]);
}
vfPrintf(psSer, "}", pu8Data[i]);
#endif

	uint8 com = (0x01<<5) | (0x01<<2) | 0x00;		//	Temp Enable | Pres Enable | Mode
	bOk &= bSMBusWrite(BME280_ADDRESS, BME280_CTRL_MEAS, 1, &com );

	return bOk;
}

// the Main loop
PRIVATE void vProcessSnsObj_BME280(void *pvObj, teEvent eEvent) {
	tsSnsObj *pSnsObj = (tsSnsObj *)pvObj;
	tsObjData_BME280 *pObj = (tsObjData_BME280 *)pSnsObj->pvData;

	// general process (independent from each state)
	switch (eEvent) {
		case E_EVENT_TICK_TIMER:
			if (pObj->u8TickCount < 100) {
				pObj->u8TickCount += pSnsObj->u8TickDelta;
#if SERIAL_DEBUG >= 99
vfPrintf(psSer, "+");
#endif
			}
			break;
		case E_EVENT_START_UP:
			pObj->u8TickCount = 100; // expire immediately
#if SERIAL_DEBUG >= 99
vfPrintf(psSer, "\n\rBME280 WAKEUP");
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

#if SERIAL_DEBUG >= 99
vfPrintf(psSer, "\n\rBME280 KICKED");
#endif

			break;

		default:
			break;
		}
		break;

	case E_SNSOBJ_STATE_MEASURING:
		switch (eEvent) {
		case E_EVENT_NEW_STATE:
			pObj->i16Temp = SENSOR_TAG_DATA_ERROR;
			pObj->u16Hum = SENSOR_TAG_DATA_ERROR;
			pObj->u16Pres = SENSOR_TAG_DATA_ERROR;
			pObj->u8TickWait = BME280_CONVTIME;

			pObj->bBusy = TRUE;
#ifdef BME280_ALWAYS_RESET
			//u8reset_flag = TRUE;
			if (!bBME280reset()) {
				vSnsObj_NewState(pSnsObj, E_SNSOBJ_STATE_COMPLETE);
			}
#else
			if (!bBME280StartRead()) { // kick I2C communication
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
			i16BME280ReadResult(&pObj->i16Temp, &pObj->u16Pres, &pObj->u16Hum );

			// data arrival
			pObj->bBusy = FALSE;
			vSnsObj_NewState(pSnsObj, E_SNSOBJ_STATE_COMPLETE);
		}
		break;

	case E_SNSOBJ_STATE_COMPLETE:
		switch (eEvent) {
		case E_EVENT_NEW_STATE:
			#if SERIAL_DEBUG >= 99
			vfPrintf(psSer, "\n\rBME280_CP: t:%d p:%d h:%d", pObj->i16Temp, pObj->u16Pres, pObj->u16Hum);
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
