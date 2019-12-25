/* Copyright (C) 2017 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */

#ifndef  HUM_LTR308ALS_INCLUDED
#define  HUM_LTR308ALS_INCLUDED

/****************************************************************************/
/***        Include Files                                                 ***/
/****************************************************************************/
#include "sensor_driver.h"
#ifdef __cplusplus
extern "C" {
#endif // C++
/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#define LTR308ALS_ADDRESS		(0x53)

#define LTR308ALS_IDX_TEMP 0
#define LTR308ALS_IDX_HUMID 1
#define LTR308ALS_IDX_BEGIN 0
#define LTR308ALS_IDX_END (LTR308ALS_IDX_HUMID+1) // should be (last idx + 1)

#define LTR308ALS_DATA_NOTYET	(0xFFFFFFFF)
#define LTR308ALS_DATA_ERROR	(0xFFFFFFFE)

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
typedef struct {
	// data
	uint32 u32Result;

	// working
	uint8 u8TickCount; // __attribute__((aligned(4)));
	uint8 u8TickWait; // __attribute__((aligned(4)));
	uint8 u8IdxMeasuruing; // __attribute__((aligned(4)));
} tsObjData_LTR308ALS;

/****************************************************************************/
/***        Exported Functions (state machine)                            ***/
/****************************************************************************/

void vLTR308ALS_Init(tsObjData_LTR308ALS *pData, tsSnsObj *pSnsObj);
void vLTR308ALS_Final(tsObjData_LTR308ALS *pData, tsSnsObj *pSnsObj);


/****************************************************************************/
/***        Exported Functions (primitive funcs)                          ***/
/****************************************************************************/
PUBLIC bool_t bLTR308ALSreset();
PUBLIC bool_t bLTR308ALSstartRead();
PUBLIC uint32 u32LTR308ALSreadResult();
PUBLIC bool_t bLTR308ALSdataArrived();

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/
#ifdef __cplusplus
}
#endif // C++

#endif  /* HUM_LTR308ALS_INCLUDED */

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/

