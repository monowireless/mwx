/* Copyright (C) 2017 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */

#ifndef  ADT7410_INCLUDED
#define  ADT7410_INCLUDED

#if defined __cplusplus
extern "C" {
#endif

/****************************************************************************/
/***        Include Files                                                 ***/
/****************************************************************************/
#include "sensor_driver.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
typedef struct {
	// protected
	bool_t bBusy;          // should block going into sleep

	// public
	int16 i16Result;

	// private
	uint8 u8TickCount, u8TickWait;
} tsObjData_ADT7410;

/****************************************************************************/
/***        Exported Functions (state machine)                            ***/
/****************************************************************************/
void vADT7410_Init(tsObjData_ADT7410 *pData, tsSnsObj *pSnsObj);
void vADT7410_Final(tsObjData_ADT7410 *pData, tsSnsObj *pSnsObj);

/****************************************************************************/
/***        Exported Functions (primitive funcs)                          ***/
/****************************************************************************/
PUBLIC bool_t bADT7410reset(bool_t);
PUBLIC bool_t bADT7410startRead();
PUBLIC int16 i16ADT7410readResult(bool_t);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

#if defined __cplusplus
}
#endif

#endif  /* ADT7410_INCLUDED */

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/

