/****************************************************************************
 *
 * MODULE:             SMBus functions header file
 *
 * COMPONENT:          $RCSfile: SMBus.h,v $
 *
 * VERSION:            $Name: RD_RELEASE_6thMay09 $
 *
 * REVISION:           $Revision: 1.2 $
 *
 * DATED:              $Date: 2008/02/29 18:00:43 $
 *
 * STATUS:             $State: Exp $
 *
 * AUTHOR:             Lee Mitchell
 *
 * DESCRIPTION:
 * SMBus functions (header file)
 *
 * CHANGE HISTORY:
 *
 * $Log: SMBus.h,v $
 * Revision 1.2  2008/02/29 18:00:43  dclar
 * dos2unix
 *
 * Revision 1.1  2006/12/08 10:50:46  lmitch
 * Added to repository
 *
 *
 *
 * LAST MODIFIED BY:   $Author: dclar $
 *                     $Modtime: $
 *
 ****************************************************************************
 *
 * This software is owned by Jennic and/or its supplier and is protected
 * under applicable copyright laws. All rights are reserved. We grant You,
 * and any third parties, a license to use this software solely and
 * exclusively on Jennic products. You, and any third parties must reproduce
 * the copyright and warranty notice and any other legend of ownership on
 * each copy or partial copy of the software.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS". JENNIC MAKES NO WARRANTIES, WHETHER
 * EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE,
 * ACCURACY OR LACK OF NEGLIGENCE. JENNIC SHALL NOT, IN ANY CIRCUMSTANCES,
 * BE LIABLE FOR ANY DAMAGES, INCLUDING, BUT NOT LIMITED TO, SPECIAL,
 * INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON WHATSOEVER.
 *
 * Copyright Jennic Ltd 2005, 2006. All rights reserved
 *
 ***************************************************************************/

/* Copyright (C) 2017 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */

#ifndef  SMBUS_H_INCLUDED
#define  SMBUS_H_INCLUDED

#if defined __cplusplus
extern "C" {
#endif

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#define SMBUS_CLOCK_ID_12_5KHZ 255
#define SMBUS_CLOCK_ID_16_6KHZ 191
#define SMBUS_CLOCK_ID_20KHZ 159
#define SMBUS_CLOCK_ID_25KHZ 127
#define SMBUS_CLOCK_ID_33KHZ 95
#define SMBUS_CLOCK_ID_40KHZ 79
#define SMBUS_CLOCK_ID_50KHZ 63
#define SMBUS_CLOCK_ID_66KHZ 47
#define SMBUS_CLOCK_ID_80KHZ 39
#define SMBUS_CLOCK_ID_100KHZ 31
#define SMBUS_CLOCK_ID_133KHZ 23
#define SMBUS_CLOCK_ID_160KHZ 19
#define SMBUS_CLOCK_ID_200KHZ 15
#define SMBUS_CLOCK_ID_266KHZ 11
#define SMBUS_CLOCK_ID_320KHZ 9
#define SMBUS_CLOCK_ID_400KHZ 7

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

PUBLIC void vSMBusInit(void);

PUBLIC void vSMBusInit_setClk(uint8 u8Clk);

PUBLIC bool_t bSMBusWrite(uint8 u8Address, uint8 u8Command,
						  uint8 u8Length, uint8* pu8Data);

PUBLIC bool_t bSMBusRandomRead(uint8 u8Address, uint8 u8Command,
						       uint8 u8Length, uint8* pu8Data);

PUBLIC bool_t bSMBusRandomRead_NACK(uint8 u8Address, uint8 u8Command,
						       uint8 u8Length, uint8* pu8Data);

PUBLIC bool_t bSMBusSequentialRead(uint8 u8Address, uint8 u8Length,
								   uint8* pu8Data);

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/

#if defined __cplusplus
}
#endif

#endif  /* SMBUS_H_INCLUDED */

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/

