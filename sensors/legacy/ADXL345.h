/* Copyright (C) 2017 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */

#ifndef  ADXL345_INCLUDED
#define  ADXL345_INCLUDED

#if defined __cplusplus
extern "C" {
#endif

/****************************************************************************/
/***        Include Files                                                 ***/
/****************************************************************************/
#include "appsave.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#define ADXL345_IDX_X 0
#define ADXL345_IDX_Y 1
#define ADXL345_IDX_Z 2

#define ADXL345_IDX_BEGIN 0
#define ADXL345_IDX_END (ADXL345_IDX_Z+1) // should be (last idx + 1)

//#define ADXL345_ADDRESS		(0x53)
#define ADXL345_ADDRESS		(0x1D)

#define ADXL345_CONVTIME    (0)
#define ADXL345_LOWENERGY_CONVTIME    (10) // LowEnergyの時はADC開始から10msまつ

#define ADXL345_DATA_NOTYET	(-32768)
#define ADXL345_DATA_ERROR	(-32767)

#define ADXL345_THRESH_TAP		0x1D
#define ADXL345_OFSX			0x1E
#define ADXL345_OFSY			0x1F
#define ADXL345_OFSZ			0x20
#define ADXL345_DUR				0x21
#define ADXL345_LATENT			0x22
#define ADXL345_WINDOW			0x23
#define ADXL345_THRESH_ACT		0x24
#define ADXL345_THRESH_INACT	0x25
#define ADXL345_TIME_INACT		0x26
#define ADXL345_ACT_INACT_CTL	0x27
#define ADXL345_THRESH_FF		0x28
#define ADXL345_TIME_FF			0x29
#define ADXL345_TAP_AXES		0x2A
#define ADXL345_ACT_TAP_STATUS	0x2B
#define ADXL345_BW_RATE			0x2C
#define ADXL345_POWER_CTL		0x2D
#define ADXL345_INT_ENABLE		0x2E
#define ADXL345_INT_MAP			0x2F
#define ADXL345_INT_SOURCE		0x30
#define ADXL345_DATA_FORMAT		0x31
#define ADXL345_DATAX0			0x32
#define ADXL345_DATAX1			0x33
#define ADXL345_DATAY0			0x34
#define ADXL345_DATAY1			0x35
#define ADXL345_DATAZ0			0x36
#define ADXL345_DATAZ1			0x37
#define ADXL345_FIFO_CTL		0x38
#define ADXL345_FIFO_STATUS		0x39

//	センサパラメータの設定値
#define NORMAL				0
#define S_TAP				1
#define D_TAP				2
#define FREEFALL			4
#define ACTIVE				8
#define INACTIVE			16
#define SHAKE				32
#define SHAKE_ACC1			33
#define SHAKE_ACC2			32+2
#define SHAKE_ACC3			32+3
#define SHAKE_HOLD			32+4
#define SHAKE_FAN			32+5
#define DICE				64
#define AIRVOLUME			128
#define NEKOTTER			256
#define LOWENERGY			512
#define FIFO				1024
#define LINK				2048
#define DIS_ZAXES			4096
#define DIS_YAXES			8192
#define DIS_XAXES			16384

#define READ_FIFO 10
#define READ_FIFO_SHAKE 5
#define READ_FIFO_AIR 3

#define TH_ACCEL 120
#define TH_COUNT 0

#define ADXL345_X	ADXL345_DATAX0
#define ADXL345_Y	ADXL345_DATAY0
#define ADXL345_Z	ADXL345_DATAZ0

#define GetAxis(c, data) \
{\
	c &= bSMBusWrite( ADXL345_ADDRESS, ADXL345_DATAX0, 0, NULL );\
	c &= bSMBusSequentialRead( ADXL345_ADDRESS, 6, data );\
}


/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
typedef struct {
	// protected
	bool_t	bBusy;			// should block going into sleep

	// data
	uint8	u8FIFOSample;
	int16	ai16Result[3];
	int16	ai16ResultX[33];
	int16	ai16ResultY[33];
	int16	ai16ResultZ[33];
	uint8	u8Interrupt;

	// working
	uint8	u8TickCount, u8TickWait;
} tsObjData_ADXL345;

/****************************************************************************/
/***        Exported Functions (state machine)                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions (primitive funcs)                          ***/
/****************************************************************************/
// normal
void vADXL345_Init(tsObjData_ADXL345 *pData, tsSnsObj *pSnsObj );
bool_t bADXL345_Setting( int16 i16mode, tsADXL345Param sParam, bool_t bLink );

PUBLIC bool_t bADXL345reset();
PUBLIC bool_t bADXL345startRead();
PUBLIC bool_t bADXL345readResult( int16* ai16accel );		// 普通に加速度を読み込む
PUBLIC bool_t bNekotterreadResult( int16* ai16accel );		// 加速度を読み込んで分散をとったものを返す
PUBLIC bool_t bShakereadResult( int16* ai16accel );			// 加速度の変化量の合計を返す

uint16 u16ADXL345_GetSamplingFrequency(void);
bool_t bADXL345_StartMeasuring( bool_t bLink );
bool_t bADXL345_EndMeasuring(void);

// LowEnergy
PUBLIC bool_t bADXL345_LowEnergyStartRead();
void vADXL345_LowEnergy_Init(tsObjData_ADXL345 *pData, tsSnsObj *pSnsObj );
bool_t bADXL345_LowEnergy_Setting();
PUBLIC bool_t bADXL345_LowEnergyReadResult( int16* ai16accel );

// fifo
void vADXL345_FIFO_Init(tsObjData_ADXL345 *pData, tsSnsObj *pSnsObj );
bool_t bADXL345_FIFO_Setting( uint16 u16mode, tsADXL345Param sParam );
PUBLIC bool_t bADXL345FIFOreadResult( int16* ai16accelx, int16* ai16accely, int16* ai16accelz );
bool_t bADXL345_EnableFIFO( uint16 u16mode );
bool_t bADXL345_DisableFIFO( uint16 u16mode );

// AirVolume
void vADXL345_AirVolume_Init(tsObjData_ADXL345 *pData, tsSnsObj *pSnsObj );
bool_t bADXL345_AirVolume_Setting();
bool_t bSetFIFO_Air();
bool_t bSetActive();
PUBLIC bool_t b16ADXL345_AirVolumeReadResult( int16* ai16accel );
PUBLIC bool_t b16ADXL345_AirVolumeSingleReadResult( int16* ai16accel );

//
void vADXL345_Final(tsObjData_ADXL345 *pData, tsSnsObj *pSnsObj);
uint8 u8Read_Interrupt( void );


/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/
extern uint8 u8Interrupt;
extern uint8 ADXL345_AXIS[3];

#if defined __cplusplus
}
#endif

#endif  /* ADXL345_INCLUDED */

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/

