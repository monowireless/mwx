/* Copyright (C) 2017 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */

#ifndef  HUM_MAX31855_INCLUDED
#define  HUM_MAX31855_INCLUDED

/****************************************************************************/
/***        Include Files                                                 ***/
/****************************************************************************/
#include "sensor_driver.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#define MAX31855_IDX_THERMOCOUPLE_TEMP 0
#define MAX31855_IDX_INTERNAL_TEMP 1
#define MAX31855_IDX_BEGIN 0
#define MAX31855_IDX_END (MAX31855_IDX_INTERNAL_TEMP+1) // should be (last idx + 1)

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
typedef struct {
	// data
	int32 ai32Result[2];

	// working
	uint8 u8TickCount, u8TickWait;
	uint8 u8IdxMeasuruing;
} tsObjData_MAX31855;

/****************************************************************************/
/***        Exported Functions (state machine)                            ***/
/****************************************************************************/
void vMAX31855_Init(tsObjData_MAX31855 *pData, tsSnsObj *pSnsObj);
void vMAX31855_Final(tsObjData_MAX31855 *pData, tsSnsObj *pSnsObj);

/****************************************************************************/
/***        Exported Functions (primitive funcs)                          ***/
/****************************************************************************/
PUBLIC bool_t bMAX31855reset();
PUBLIC bool_t bMAX31855startRead(uint8);
PUBLIC int32 i32MAX31855readResult(int32*, int32*);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

#endif  /* HUM_MAX31855_INCLUDED */

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/

