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
#include "ADXL345.h"
#include "SMBus.h"

#include "ccitt8.h"

#include "utils.h"

#include "Interactive.h"

#include "EndDevice_Input.h"

# include <serial.h>
# include <fprintf.h>
#undef SERIAL_DEBUG
#ifdef SERIAL_DEBUG
extern tsFILE sDebugStream;
#endif


/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
PRIVATE void vProcessSnsObj_ADXL345(void *pvObj, teEvent eEvent);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
static uint16 u16modeflag = 0x00;
static uint16 u16ThAccel = TH_ACCEL;

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
void vADXL345_Init(tsObjData_ADXL345 *pData, tsSnsObj *pSnsObj) {
	vSnsObj_Init(pSnsObj);

	pSnsObj->pvData = (void*)pData;
	pSnsObj->pvProcessSnsObj = (void*)vProcessSnsObj_ADXL345;

	memset((void*)pData, 0, sizeof(tsObjData_ADXL345));
}

void vADXL345_Final(tsObjData_ADXL345 *pData, tsSnsObj *pSnsObj) {
	pSnsObj->u8State = E_SNSOBJ_STATE_INACTIVE;
}

//	センサの設定を記述する関数
bool_t bADXL345_Setting( int16 i16mode, tsADXL345Param sParam, bool_t bLink )
{
	u16modeflag = (uint16)i16mode;
	u16modeflag = ((i16mode&SHAKE) != 0) ? SHAKE : u16modeflag;
	u16modeflag = ((i16mode&DICE) != 0) ? DICE : u16modeflag;
	uint8 u8UseAxis = (uint8)(((~i16mode)&0x7000)>>12);

	uint8 com;
	if( u16modeflag == NEKOTTER || u16modeflag == SHAKE ){
		switch(sParam.u16Duration){
		case 1:
			com = 0x14;		//	Low Power Mode, 1.56Hz Sampling frequency
			break;
		case 3:
			com = 0x15;		//	Low Power Mode, 3.13Hz Sampling frequency
			break;
		case 6:
			com = 0x06;		//	6.25Hz Sampling frequency
			break;
		case 12:
			com = 0x07;		//	12.5Hz Sampling frequency
			break;
		case 25:
			com = 0x18;		//	25Hz Sampling frequency
			break;
		case 50:
			com = 0x09;		//	50Hz Sampling frequency
			break;
		case 100:
			com = 0x0A;		//	100Hz Sampling frequency
			break;
		case 200:
			com = 0x0B;		//	200Hz Sampling frequency
			break;
		case 400:
			com = 0x0C;		//	400Hz Sampling frequency
			break;
//		case 800:
//			com = 0x0D;		//	800Hz Sampling frequency
//			break;
//		case 1600:
//			com = 0x0E;		//	1600Hz Sampling frequency
//			break;
//		case 3200:
//			com = 0x0F;		//	3200Hz Sampling frequency
//			break;
		default:
			com = 0x08;		//	25Hz Sampling frequency
			break;
		}
	}else{
		if( u16modeflag == NORMAL ){
			com = 0x19;		//	Low Power Mode, 50Hz Sampling frequency
		}else{
			com = 0x1A;		//	Low Power Mode, 100Hz Sampling frequency
		}
	}
	bool_t bOk = bSMBusWrite(ADXL345_ADDRESS, ADXL345_BW_RATE, 1, &com );

	com = 0x0B;		//	Full Resolution Mode, +-16g
	bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_DATA_FORMAT, 1, &com );

	bOk &= bADXL345_StartMeasuring(bLink);

	uint16 u16tempcom;
	//	タップを判別するための閾値
	if( (u16modeflag&S_TAP) || (u16modeflag&D_TAP) ){
		if( sParam.u16ThresholdTap != 0 ){
			u16tempcom = sParam.u16ThresholdTap*10/625;
			if( 0x00FF < u16tempcom ){
				com = 0xFF;
			}else{
				com = (uint8)u16tempcom;
			}
		}else{
			com = 0x32;			//	threshold of tap
		}
	}else{
		com = 0x00;
	}
	bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_THRESH_TAP, 1, &com );

	//	タップを認識するための時間
	if( (u16modeflag&S_TAP) || (u16modeflag&D_TAP) ){
		if( sParam.u16Duration != 0 ){
			u16tempcom = sParam.u16Duration*10/625;
			if( 0x00FF < u16tempcom ){
				com = 0xFF;
			}else{
				com = (uint8)u16tempcom;
			}
		}else{
			com = 0x0F;
		}
	}else{
		com = 0x00;
	}
	bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_DUR, 1, &com );

	//	次のタップまでの時間
	if( u16modeflag&D_TAP ){
		if( sParam.u16Latency != 0 ){
			u16tempcom = sParam.u16Latency*100/125;
			if( 0x00FF < u16tempcom ){
				com = 0xFF;
			}else{
				com = (uint8)u16tempcom;
			}
		}else{
			com = 0x50;
		}
	}else{
		com = 0x00;
	}
	bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_LATENT, 1, &com );

	//	ダブルタップを認識するための時間
	if( u16modeflag&D_TAP ){
		if( sParam.u16Window != 0 ){
			u16tempcom = sParam.u16Window*100/125;
			if( 0x00FF < u16tempcom ){
				com = 0xFF;
			}else{
				com = (uint8)u16tempcom;
			}
		}else{
			com = 0xD9;			// Window Width
		}
	}else{
		com = 0x00;
	}
	bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_WINDOW, 1, &com );

	//	タップを検知する軸の設定
	if( (u16modeflag&S_TAP) || (u16modeflag&D_TAP) ){
//		com = 0x07;
		com = u8UseAxis;
	}else{
		com = 0x00;
	}
	bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_TAP_AXES, 1, &com );

	//	自由落下を検知するための閾値
	if( (u16modeflag&FREEFALL) != 0 && u16modeflag < SHAKE ){
		if( sParam.u16ThresholdFreeFall != 0 ){
			u16tempcom = sParam.u16ThresholdFreeFall*10/625;
			if( 0x00FF < u16tempcom ){
				com = 0xFF;
			}else{
				com = (uint8)u16tempcom;
			}
		}else{
			com = 0x07;			// threshold of freefall
		}
	}else{
		com = 0x00;
	}
	bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_THRESH_FF, 1, &com );

	//	自由落下を検知するための時間
	if( u16modeflag&FREEFALL ){
		if( sParam.u16TimeFreeFall != 0 ){
			u16tempcom = sParam.u16TimeFreeFall/5;
			if( 0x00FF < u16tempcom ){
				com = 0xFF;
			}else{
				com = (uint8)u16tempcom;
			}
		}else{
			com = 0x2D;			// time of freefall
		}
	}else{
		com = 0x00;
	}
	bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_TIME_FF, 1, &com );

	//	動いていることを判断するための閾値
	if( (u16modeflag&ACTIVE) || u16modeflag == DICE ){
		if( sParam.u16ThresholdActive != 0 ){
			u16tempcom = sParam.u16ThresholdActive*10/625;
			if( 0x00FF < u16tempcom ){
				com = 0xFF;
			}else{
				com = (uint8)u16tempcom;
			}
		}else{
			com = 0x15;
		}
	}else{
		com = 0x00;
	}
	bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_THRESH_ACT, 1, &com );

	//	動いていないことを判断するための閾値
	if( (u16modeflag&ACTIVE) || u16modeflag == DICE ){
		if( sParam.u16ThresholdInactive != 0 ){
			u16tempcom = sParam.u16ThresholdInactive*10/625;
			if( 0x00FF < u16tempcom ){
				com = 0xFF;
			}else{
				com = (uint8)u16tempcom;
			}
		} else {
			com = 0x14;
		}
	}else{
		com = 0x00;
	}
	bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_THRESH_INACT, 1, &com );

	//	動いていないことを判断するための時間(s)
	if( (u16modeflag&ACTIVE) || u16modeflag == DICE ){
		if( sParam.u16TimeInactive != 0 ){
			u16tempcom = sParam.u16TimeInactive;
			if( 0x00FF < u16tempcom ){
				com = 0xFF;
			}else{
				com = (uint8)u16tempcom;
			}
		} else {
			if( u16modeflag == DICE ){
				com = 0x01;
			}else{
				com = 0x02;
			}
		}
	}else{
		com = 0x00;
	}
	bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_TIME_INACT, 1, &com );

	//	動いている/いないことを判断するための軸
	if( (u16modeflag&ACTIVE) || u16modeflag == DICE ){
//		com = 0x77;
		com = u8UseAxis|(uint8)(u8UseAxis<<4);
	}else{
		com = 0x00;
	}
	bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_ACT_INACT_CTL, 1, &com );

	//	割り込みピンの設定
	if( u16modeflag > 0  && u16modeflag < 32){
		com = 0x10;		//	ACTIVEは別ピン
	}else{
		com = 0x00;
	}
	bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_INT_MAP, 1, &com );

	//	有効にする割り込みの設定
	com = 0;
	if( (u16modeflag&SHAKE) != 0 || (u16modeflag&NEKOTTER) != 0 ){
		com += 0x02;
	}else{
		if( u16modeflag == DICE ){
			com += 0x18;		//	ACTIVEとINACTIVEで割り込みさせる
		}else{
			if( u16modeflag&S_TAP ){
				com += 0x40;
			}
			if( u16modeflag&D_TAP ){
				com += 0x20;
			}
			if( u16modeflag&FREEFALL ){
				com += 0x04;
			}
			if( u16modeflag&ACTIVE ){
				com += 0x18;		//	INACTIVEも割り込みさせる
			}
		}
	}
	bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_INT_ENABLE, 1, &com );

	if( u16modeflag == NEKOTTER || u16modeflag == SHAKE ){
		com = 0xC0 | 0x20 | READ_FIFO_SHAKE;
	}else{
		com = 0x00;
	}
	bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_FIFO_CTL, 1, &com );

	if( u16modeflag == SHAKE ){
		if( sParam.u16ThresholdTap != 0 ){
			u16ThAccel = sParam.u16ThresholdTap;
		}
	}

	return bOk;
}

/****************************************************************************
 *
 * NAME: bADXL345reset
 *
 * DESCRIPTION:
 *   to reset ADXL345 device
 *
 * RETURNS:
 * bool_t	fail or success
 *
 ****************************************************************************/
PUBLIC bool_t bADXL345reset()
{
	bool_t bOk = TRUE;

	//	レジスタ内を初期値にする (初期値に関してはADXL34xのデータシート参照)
	uint8 com = 0x00;
	bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_THRESH_TAP, 1, &com );
	bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_OFSX, 1, &com );
	bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_OFSY, 1, &com );
	bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_OFSZ, 1, &com );
	bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_DUR, 1, &com );
	bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_LATENT, 1, &com );
	bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_WINDOW, 1, &com );
	bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_THRESH_ACT, 1, &com );
	bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_THRESH_INACT, 1, &com );
	bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_TIME_INACT, 1, &com );
	bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_ACT_INACT_CTL, 1, &com );
	bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_THRESH_FF, 1, &com );
	bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_TIME_FF, 1, &com );
	bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_TAP_AXES, 1, &com );
	bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_POWER_CTL, 1, &com );
	bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_INT_ENABLE, 1, &com );
	bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_INT_MAP, 1, &com );
	bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_DATA_FORMAT, 1, &com );
	bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_FIFO_CTL, 1, &com );
	com = 0x0A;
	bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_BW_RATE, 1, &com );

	return bOk;
}

uint16 u16ADXL345_GetSamplingFrequency(void)
{
	uint8 data;
	bool_t bOk = bSMBusWrite(ADXL345_ADDRESS, ADXL345_BW_RATE, 0, NULL );
	bOk &= bSMBusSequentialRead( ADXL345_ADDRESS, 1, &data );

	data = data&0x0F;		// rateだけにする

	uint16 u16SR = 0;
	switch(data){
		case 0x04:
			u16SR = 1;
			break;
		case 0x05:
			u16SR = 3;
			break;
		case 0x06:
			u16SR = 6;
			break;
		case 0x07:
			u16SR = 12;
			break;
		case 0x08:
			u16SR = 25;
			break;
		case 0x09:
			u16SR = 50;
			break;
		case 0x0A:
			u16SR = 100;
			break;
		case 0x0B:
			u16SR = 200;
			break;
		case 0x0C:
			u16SR = 400;
			break;
		case 0x0D:
			u16SR = 800;
			break;
		case 0x0E:
			u16SR = 1600;
			break;
		case 0x0F:
			u16SR = 3200;
			break;
		default:
			break;
	}

	return u16SR;
}

bool_t bADXL345_StartMeasuring( bool_t bLink )
{
	uint8 com = 0x08 | (bLink ? 0x00 : 0x20);		//	Start Measuring
	bool_t bOk = bSMBusWrite(ADXL345_ADDRESS, ADXL345_POWER_CTL, 1, &com );

	return bOk;
}

bool_t bADXL345_EndMeasuring(void)
{
	uint8 com = 0x07;		//	End Measuring
	bool_t bOk = bSMBusWrite(ADXL345_ADDRESS, ADXL345_POWER_CTL, 1, &com );

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
PUBLIC bool_t bADXL345startRead()
{
	bool_t	bOk = TRUE;

	return bOk;
}

/****************************************************************************
 *
 * NAME: u16ADXL345readResult
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
PUBLIC bool_t bADXL345readResult( int16* ai16accel )
{
	bool_t	bOk = TRUE;
	uint8	au8data[6];
	uint8	i;

	GetAxis(bOk, au8data);
	//	X軸
	ai16accel[ADXL345_IDX_X] = (((au8data[1] << 8) | au8data[0]));
	//	Y軸
	ai16accel[ADXL345_IDX_Y] = (((au8data[3] << 8) | au8data[2]));
	//	Z軸
	ai16accel[ADXL345_IDX_Z] = (((au8data[5] << 8) | au8data[4]));

	if (bOk == FALSE) {
		ai16accel[ADXL345_IDX_X] = SENSOR_TAG_DATA_ERROR;
		ai16accel[ADXL345_IDX_Y] = SENSOR_TAG_DATA_ERROR;
		ai16accel[ADXL345_IDX_Z] = SENSOR_TAG_DATA_ERROR;
	}else{
		for( i=0; i<3; i++ ){
			ai16accel[i] = (ai16accel[i]<<2)/10;
		}
	}


	return bOk;
}

PUBLIC bool_t bNekotterreadResult( int16* ai16accel )
{
	bool_t	bOk = TRUE;
	int16	ai16result[3];
	uint8	au8data[6];
	int8	num;
	uint8	data;
	uint8	i, j;
	int16	ave;
	int16	sum[3] = { 0, 0, 0 };		//	サンプルの総和
	uint32	ssum[3] = { 0, 0, 0 };		//	サンプルの2乗和

	//	FIFOでたまった個数を読み込む
	bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_FIFO_STATUS, 0, NULL );
	bOk &= bSMBusSequentialRead( ADXL345_ADDRESS, 1, &data );

	num = (uint8)(data&0x7f);
	for( i=0; i<num; i++ ){
		//	各軸の読み込み
		//	X軸
		GetAxis(bOk, au8data);
		ai16result[ADXL345_IDX_X] = (((au8data[1] << 8) | au8data[0]));
		//	Y軸
		ai16result[ADXL345_IDX_Y] = (((au8data[3] << 8) | au8data[2]));
		//	Z軸
		ai16result[ADXL345_IDX_Z] = (((au8data[5] << 8) | au8data[4]));

		//	総和と二乗和の計算
		for( j=0; j<3; j++ ){
			sum[j] += ai16result[j];
			ssum[j] += ai16result[j]*ai16result[j];
		}
		//vfPrintf(& sSerStream, "\n\r%2d:%d,%d,%d", i, ai16result[ADXL345_IDX_X], ai16result[ADXL345_IDX_Y], ai16result[ADXL345_IDX_Z]);
		//SERIAL_vFlush(E_AHI_UART_0);
	}

	for( i=0; i<3; i++ ){
	//	分散が評価値 分散の式を変形
		ave = sum[i]/num;
		ai16accel[i] = (int16)sqrt((double)(-1*(ave*ave)+(ssum[i]/num)));
	}

	//	ねこったーモードはじめ
	//	FIFOの設定をもう一度
//	bOk &= bSetFIFO();
	//	終わり

    return bOk;
}

#define MY_ABS(c) ( c<0 ? -1*c : c )

PUBLIC bool_t bShakereadResult( int16* ai16accel )
{
	static 	int16	ai16TmpAccel[3]={0, 0, 0};
	bool_t	bOk = TRUE;
	uint8	au8data[6];
	int16	max = 0x8000;
	uint8	num;				//	FIFOのデータ数
	uint8	i;
	int16	sum[READ_FIFO_SHAKE];
	uint8	count = 0;
	int16	x[READ_FIFO_SHAKE];
	int16	y[READ_FIFO_SHAKE];
	int16	z[READ_FIFO_SHAKE];

	//	FIFOでたまった個数を読み込む
	bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_FIFO_STATUS, 0, NULL );
	bOk &= bSMBusSequentialRead( ADXL345_ADDRESS, 1, &num );

	//	FIFOの中身を全部読む
	num = (num&0x7f);
	if( num == READ_FIFO_SHAKE ){
		//	各軸の読み込み
		for( i=0; i<num; i++ ){
			GetAxis(bOk, au8data);
			//	X軸
			x[i] = (((au8data[1] << 8) | au8data[0]));
			//	Y軸
			y[i] = (((au8data[3] << 8) | au8data[2]));
			//	Z軸
			z[i] = (((au8data[5] << 8) | au8data[4]));
		}
		//	FIFOの設定をもう一度
		//bOk &= bSetFIFO();

		for( i=0; i<num; i++ ){
			x[i] = (x[i]<<2)/10;
			y[i] = (y[i]<<2)/10;
			z[i] = (z[i]<<2)/10;

			if( i == 0 ){
				sum[i] = ( x[i]-ai16TmpAccel[0] + y[i]-ai16TmpAccel[1] + z[i]-ai16TmpAccel[2] );
			}else{
				sum[i] = ( x[i]-x[i-1] + y[i]-y[i-1] + z[i]-z[i-1] );
			}

			if( sum[i] < 0 ){
				sum[i] *= -1;
			}

			max = sum[i]>max ? sum[i] : max;

			if( sum[i] > u16ThAccel ){
				count++;
			}
#if 0
			vfPrintf( &sSerStream, "\n\r%2d:%d,%d,%d %d", i, x[i], y[i], z[i], sum[i] );
			SERIAL_vFlush(E_AHI_UART_0);
		}
		vfPrintf( &sSerStream, "\n\r" );
#else
		}
#endif
		ai16accel[0] = max;
		ai16accel[1] = z[0];
		ai16accel[2] = count;
		ai16TmpAccel[0] = x[num-1];
		ai16TmpAccel[1] = y[num-1];
		ai16TmpAccel[2] = z[num-1];
	}else{
		//	FIFOの設定をもう一度
//		bOk &= bSetFIFO();
		ai16accel[0] = 0;
		ai16accel[1] = 0;
		ai16accel[2] = 0;
	}

	//	終わり

    return bOk;
}

uint8 u8Read_Interrupt( void )
{
	uint8	u8source;
	bool_t bOk = TRUE;

	bOk &= bSMBusWrite( ADXL345_ADDRESS, 0x30, 0, NULL );
	bOk &= bSMBusSequentialRead( ADXL345_ADDRESS, 1, &u8source );

	if(!bOk){
		u8source = 0xFF;
	}

	return u8source;
}

//PRIVATE bool_t bSetFIFO( void )
//{
	//	FIFOの設定をもう一度
//	uint8 com = 0x00 | 0x20 | READ_FIFO_SHAKE;
//	bool_t bOk = bSMBusWrite(ADXL345_ADDRESS, ADXL345_FIFO_CTL, 1, &com );
//	com = 0xC0 | 0x20 | READ_FIFO_SHAKE;
//	bOk &= bSMBusWrite(ADXL345_ADDRESS, ADXL345_FIFO_CTL, 1, &com );
	//	終わり

//	return bOk;
//}
/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/
// the Main loop
PRIVATE void vProcessSnsObj_ADXL345(void *pvObj, teEvent eEvent) {
	tsSnsObj *pSnsObj = (tsSnsObj *)pvObj;
	tsObjData_ADXL345 *pObj = (tsObjData_ADXL345 *)pSnsObj->pvData;

	// general process (independent from each state)
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
vfPrintf(&sDebugStream, "\n\rADXL345 WAKEUP");
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
			vfPrintf(&sDebugStream, "\n\rADXL345 KICKED");
			#endif

			break;

		default:
			break;
		}
		break;

	case E_SNSOBJ_STATE_MEASURING:
		switch (eEvent) {
		case E_EVENT_NEW_STATE:
			pObj->u8Interrupt = u8Interrupt;
			pObj->ai16Result[ADXL345_IDX_X] = SENSOR_TAG_DATA_ERROR;
			pObj->ai16Result[ADXL345_IDX_Y] = SENSOR_TAG_DATA_ERROR;
			pObj->ai16Result[ADXL345_IDX_Z] = SENSOR_TAG_DATA_ERROR;
			pObj->u8TickWait = ADXL345_CONVTIME;

			pObj->bBusy = TRUE;
#ifdef ADXL345_ALWAYS_RESET
			u8reset_flag = TRUE;
			if (!bADXL345reset()) {
				vSnsObj_NewState(pSnsObj, E_SNSOBJ_STATE_COMPLETE);
			}
#else
			//if (!bADXL345startRead()) { // kick I2C communication
			//	vSnsObj_NewState(pSnsObj, E_SNSOBJ_STATE_COMPLETE);
			//}
#endif
			pObj->u8TickCount = 0;
			break;

		default:
			break;
		}

		// wait until completion
		if (pObj->u8TickCount > pObj->u8TickWait) {
#ifdef ADXL345_ALWAYS_RESET
			if (u8reset_flag) {
				u8reset_flag = 0;
				if (!bADXL345startRead()) {
					vADXL345_new_state(pObj, E_SNSOBJ_STATE_COMPLETE);
				}

				pObj->u8TickCount = 0;
				pObj->u8TickWait = ADXL345_CONVTIME;
				break;
			}
#endif
			if(u16modeflag == NEKOTTER){
				bNekotterreadResult(pObj->ai16Result);
			}else if(u16modeflag == SHAKE){
				bShakereadResult(pObj->ai16Result);
			}else{
				bADXL345readResult(pObj->ai16Result);
			}

			// data arrival
			pObj->bBusy = FALSE;
			vSnsObj_NewState(pSnsObj, E_SNSOBJ_STATE_COMPLETE);
		}
		break;

	case E_SNSOBJ_STATE_COMPLETE:
		switch (eEvent) {
		case E_EVENT_NEW_STATE:
			#ifdef SERIAL_DEBUG
			vfPrintf(&sDebugStream, "\n\rADXL345_CP: %d", pObj->i16Result);
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
