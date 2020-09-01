/* Copyright (C) 2017 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */

#ifndef  HUM_SHT31_INCLUDED
#define  HUM_SHT31_INCLUDED

/****************************************************************************/
/***        Include Files                                                 ***/
/****************************************************************************/
#include "sensor_driver.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#define SHT31_IDX_TEMP 0
#define SHT31_IDX_HUMID 1
#define SHT31_IDX_BEGIN 0
#define SHT31_IDX_END (SHT31_IDX_HUMID+1) // should be (last idx + 1)

#define DEFAULT_SHT31_ADDRESS     (0x44)

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
typedef struct {
	// data
	int16 ai16Result[2];

	// working
	uint8 u8TickCount, u8TickWait;
	uint8 u8IdxMeasuruing;
} tsObjData_SHT31;

#ifdef __cplusplus
extern "C" {
#endif // C++
/****************************************************************************/
/***        Exported Functions (state machine)                            ***/
/****************************************************************************/

void vSHT31_Init(tsObjData_SHT31 *pData, tsSnsObj *pSnsObj);
void vSHT31_Final(tsObjData_SHT31 *pData, tsSnsObj *pSnsObj);

#define i16SHT31_GetTemp(pSnsObj) ((tsObjData_SHT31 *)(pSnsObj->pData)->ai16Result[SHT31_IDX_TEMP])
#define i16SHT31_GetHumd(pSnsObj) ((tsObjData_SHT31 *)(pSnsObj->pData)->ai16Result[SHT31_IDX_HUMID])

/****************************************************************************/
/***        Exported Functions (primitive funcs)                          ***/
/****************************************************************************/
PUBLIC bool_t bSHT31reset();
PUBLIC bool_t bSHT31startRead();
PUBLIC int16 i16SHT31readResult(int16*, int16*);


/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

#ifdef __cplusplus
}
#endif // C++

#endif  /* HUM_SHT31_INCLUDED */

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/

