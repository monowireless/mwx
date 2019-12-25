/* Copyright (C) 2017 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */

#ifndef  BH1715_INCLUDED
#define  BH1715_INCLUDED

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
} tsObjData_BH1715;

/****************************************************************************/
/***        Exported Functions (state machine)                            ***/
/****************************************************************************/
void vBH1715_Init(tsObjData_BH1715 *pData, tsSnsObj *pSnsObj);
void vBH1715_Final(tsObjData_BH1715 *pData, tsSnsObj *pSnsObj);


/****************************************************************************/
/***        Exported Functions (primitive funcs)                          ***/
/****************************************************************************/
PUBLIC bool_t bBH1715reset();
PUBLIC bool_t bBH1715startRead();
PUBLIC int16 i16BH1715readResult();

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

#if defined __cplusplus
}
#endif

#endif  /* BH1715_INCLUDED */

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/

