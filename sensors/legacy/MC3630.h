/* Copyright (C) 2017 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */

#ifndef  MC3630_INCLUDED
#define  MC3630_INCLUDED

#if defined __cplusplus
extern "C" {
#endif

/****************************************************************************/
/***        Include Files                                                 ***/
/****************************************************************************/

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
// Sensor Registry Address
#define MC3630_EXT_STAT_1       (0x00)
#define MC3630_EXT_STAT_2       (0x01)
#define MC3630_XOUT_LSB         (0x02)
#define MC3630_XOUT_MSB         (0x03)
#define MC3630_YOUT_LSB         (0x04)
#define MC3630_YOUT_MSB         (0x05)
#define MC3630_ZOUT_LSB         (0x06)
#define MC3630_ZOUT_MSB         (0x07)
#define MC3630_STATUS_1         (0x08)
#define MC3630_STATUS_2         (0x09)
#define MC3630_FREG_1           (0x0D)
#define MC3630_FREG_2           (0x0E)
#define MC3630_INIT_1           (0x0F)
#define MC3630_MODE_C           (0x10)
#define MC3630_RATE_1           (0x11)
#define MC3630_SNIFF_C          (0x12)
#define MC3630_SNIFFTH_C        (0x13)
#define MC3630_SNIFFCF_C        (0x14)
#define MC3630_RANGE_C          (0x15)
#define MC3630_FIFO_C           (0x16)
#define MC3630_INTR_C           (0x17)
#define MC3630_CHIP_ID          (0x18)
#define MC3630_INIT_3           (0x1A)
#define MC3630_PMCR             (0x1C)
#define MC3630_DMX              (0x20)
#define MC3630_DMY              (0x21)
#define MC3630_DMZ              (0x22)
#define MC3630_RESET            (0x24)
#define MC3630_INIT_2           (0x28)
#define MC3630_TRIGC            (0x29)
#define MC3630_XOFFL            (0x2A)
#define MC3630_XOFFH            (0x2B)
#define MC3630_YOFFL            (0x2C)
#define MC3630_YOFFH            (0x2D)
#define MC3630_ZOFFL            (0x2E)
#define MC3630_ZOFFH            (0x2F)
#define MC3630_XGAIN            (0x30)
#define MC3630_YGAIN            (0x31)
#define MC3630_ZGAIN            (0x32)

#define MC3630_WRITE            (0x40)
#define MC3630_READ             (0xC0)

#define MC3630_CONVTIME         (0)

#define MC3630_X                (0)
#define MC3630_Y                (1)
#define MC3630_Z                (2)

#define MC3630_SAMPLING25HZ     (0x06)
#define MC3630_SAMPLING50HZ     (0x07)
#define MC3630_SAMPLING100HZ    (0x08)
#define MC3630_SAMPLING190HZ    (0x09)
#define MC3630_SAMPLING380HZ    (0x0A)

#define MC3630_RANGE2G          (0x00)
#define MC3630_RANGE4G          (0x10)
#define MC3630_RANGE8G          (0x20)
#define MC3630_RANGE16G         (0x30)

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
typedef struct {
	// protected
	bool_t	bBusy;			// should block going into sleep

	// data
	uint8	u8FIFOSample;
	int16	ai16Result[3][32];
	uint8	u8Interrupt;
	uint8	u8SampleFreq;

	// working
	uint8	u8TickCount, u8TickWait;
} tsObjData_MC3630;

/****************************************************************************/
/***        Exported Functions (state machine)                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions (primitive funcs)                          ***/
/****************************************************************************/
PUBLIC void vMC3630_Init(tsObjData_MC3630 *pData, tsSnsObj *pSnsObj);
PUBLIC bool_t bMC3630_reset( uint8 Freq, uint8 Range, uint8 SplNum );
PUBLIC bool_t bMC3630_startRead();
PUBLIC uint8 u8MC3630_readResult( int16* ai16x, int16* ai16y, int16* ai16z );
PUBLIC uint8 u8MC3630_ReadInterrupt();
PUBLIC void vMC3630_ClearInterrupReg();
PUBLIC uint8 u8MC3630_ReadSamplingFrequency();
PUBLIC void vMC3630_Wakeup();
PUBLIC void vMC3630_Sleep();


/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

#if defined __cplusplus
}
#endif

#endif  /* MC3630_INCLUDED */

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/

