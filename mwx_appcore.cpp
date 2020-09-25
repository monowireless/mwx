/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include <string.h>

#include <jendefs.h>
#include <AppHardwareApi.h>

#include "twecommon.h"
#include "tweserial.h"
#include "tweserial_jen.h"
#include "tweprintf.h"

#include "twesettings.h"
#include "tweutils.h"

#include "mwx_common.hpp"
#include "mwx_utils_alloc.hpp"

#include "mwx_serial_jen.hpp"
#include "mwx_periph_dio.hpp"

#include "mwx_appdefs.hpp"

#include "mwx_twenet.hpp"
#include "mwx_memory.hpp"
#include "mwx_debug.h"

#include "mwx_appcore.hpp"

/****************************************************************************/
/***        ToCoNet Definitions                                           ***/
/****************************************************************************/
// Select Modules (define befor include "ToCoNet.h")
//#define ToCoNet_USE_MOD_NBSCAN // Neighbour scan module
//#define ToCoNet_USE_MOD_NBSCAN_SLAVE
#define ToCoNet_USE_MOD_TXRXQUEUE_SMALL

// includes
#include "ToCoNet.h"
#include "ToCoNet_mod_prototype.h"

/****************************************************************************
 * Function Prototypes
 * 
 * if user defined function is not defined, link empty function defined here.
 ****************************************************************************/

// cbAppColdStart(FALSE)
void init_coldboot() __attribute__((weak));
void init_coldboot() { }

// cbAppWarmStart(FALSE)
void init_warmboot() __attribute__((weak));
void init_warmboot() { }

// cbAppWarmStart(TRUE)
void wakeup() __attribute__((weak));
void wakeup() { }

// the first time of loop()
void begin() __attribute__((weak));
void begin() {}

// the first time of loop()
void loop() __attribute__((weak));
void loop() {}

// wake up procedure
extern void _MWX_vOnWakeup();
/****************************************************************************
 * Global Class Objects
 * 
 * NOTE: they are not initialized by constructor.
 *       actual construction is done at cbAppColdStart(FALSE).
 ****************************************************************************/
/**
 * twelite main instance
 */
mwx::twenet the_twelite;
 
/**
 * virtualized application(CTweNetCbs) instance
 */
mwx::appdefs_virt the_vapp;

/**
 * virtualized application(CTweNetCbs) instance
 */
mwx::appdefs_virt the_vsettings;

/**
 * virtualized hardware(CTweNetCbs) instance
 */
mwx::appdefs_virt the_vhw;

/**
 * virtualized network(CTweNetCbs) instance
 */
mwx::appdefs_virt the_vnet;

/** @brief	The maximum vcbs */
static const int _MAX_VCBS = 4;
/** @brief	The vcbs index application */
static const int _vcbs_idx_net = 0;
static const int _vcbs_idx_app = 1;
static const int _vcbs_idx_hw = 2;
static const int _vcbs_idx_settings = 3;
/** @brief	callback handler table (app & hw) */
static mwx::appdefs_virt* _vcbs[_MAX_VCBS];

/**
 * The Serial Object
 */
mwx::serial_jen Serial(E_AHI_UART_0);
mwx::serial_jen Serial1(E_AHI_UART_1);

/**
 * The Serial Parser Object
 */
mwx::serial_parser<mwx::alloc_heap<uint8_t>> SerialParser;

/**
 * Serial Port Stream (for C library)
 */
extern "C" TWE_tsFILE* _psSerial;
extern "C" tsFILE _sSerLegacy;

TWE_tsFILE* _psSerial;
tsFILE _sSerLegacy;

/**
 * Two wire object
 */
mwx::periph_twowire<MWX_TWOWIRE_BUFF> Wire;

/**
 * SPI object
 */
mwx::periph_spi SPI;

/**
 * ADC class
 */
mwx::periph_analogue Analogue;

/**
 * Pulse Counter
 */
mwx::periph_pulse_counter PulseCounter0(mwx::periph_pulse_counter::DEVICE_PC0);
mwx::periph_pulse_counter PulseCounter1(mwx::periph_pulse_counter::DEVICE_PC1);
mwx::periph_pulse_counter& PulseCounter = PulseCounter1;

/**
 * Timers
 */
mwx::periph_ticktimer TickTimer;
mwx::periph_timer Timer0;
mwx::periph_timer Timer1; 
mwx::periph_timer Timer2; 
mwx::periph_timer Timer3; 
mwx::periph_timer Timer4; 

mwx::periph_buttons Buttons;

/**
 * MAC handling functions
 * NOTE: this is not MAC itself, some necessary functions to control RF.
 */
mwx::twenet_mac the_mac;

/**
 * TX QUEUE cleanup flags
 */
extern "C" uint8 __attribute__((weak)) _toconet_tx_u8_init_queue_flag;
uint8 _toconet_tx_u8_init_queue_flag = 1;

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
extern "C" uint32_t u32DioPortWakeUp; //! Wake Up Port 
extern "C" uint8_t u8TimerWakeUp;
uint32_t u32DioPortWakeUp = 0;//! Wake Up Port 
uint8_t u8TimerWakeUp = 0;//! Wake Up Port 

uint32_t _periph_availmap = 0;
uint32_t _twenet_mod_registered = 0;

// just for debugging purpose
#ifdef CHECK_TIMESTAMP
uint32_t _check_timestamp = 0;
uint32_t _check_timestamp_ev = 0;
#endif

/**
 * @brief TWENET callback on system cold boot (power on/etc.)
 *   This callback function is called twice, with parameter FALSE or TRUE.
 * @param bAfterAhiInit 
 *   FALSE, when very initial stage at TWENET library.
 *   TRUE, when some initialize has done and now application can perform
 *         specific initialization.
 */
extern "C" void cbAppColdStart(bool_t bAfterAhiInit)
{
	//static uint8 u8WkState;
	if (!bAfterAhiInit) {
		// before AHI init, very first of code.
		init_coldboot();

		// Register modules
		ToCoNet_REG_MOD_ALL();

		// new instances (constuctor of global instance is not called with current compiler settings.)
		//   note: do not call hardware related API during construction.
		(void)new ((void*)&the_twelite) mwx::twenet();

		(void)new ((void*)&the_vapp) mwx::appdefs_virt();
		(void)new ((void*)&the_vsettings) mwx::appdefs_virt();
		(void)new ((void*)&the_vhw) mwx::appdefs_virt();
		(void)new ((void*)&the_vnet) mwx::appdefs_virt();

		_vcbs[0] = &the_vnet; // fistly handled to intercept net events.
		_vcbs[1] = &the_vhw;
		_vcbs[2] = &the_vapp;
		_vcbs[3] = &the_vsettings;
		
		(void)new ((void*)&Serial) mwx::serial_jen(E_AHI_UART_0);
		(void)new ((void*)&Serial1) mwx::serial_jen(E_AHI_UART_1);

		(void)new ((void*)&SerialParser) mwx::serial_parser<mwx::alloc_heap<uint8_t>>();
		
		(void)new ((void*)&Wire) mwx::periph_twowire<MWX_TWOWIRE_BUFF>();
		(void)new ((void*)&SPI) mwx::periph_spi();

		(void)new ((void*)&Analogue) mwx::periph_analogue();
		(void)new ((void*)&PulseCounter0) mwx::periph_pulse_counter(mwx::periph_pulse_counter::DEVICE_PC0);
		(void)new ((void*)&PulseCounter1) mwx::periph_pulse_counter(mwx::periph_pulse_counter::DEVICE_PC1);

		(void)new ((void*)&Timer0) mwx::periph_timer(mwx::periph_timer::TIMER_0);
		(void)new ((void*)&Timer1) mwx::periph_timer(mwx::periph_timer::TIMER_1);
		(void)new ((void*)&Timer2) mwx::periph_timer(mwx::periph_timer::TIMER_2);
		(void)new ((void*)&Timer3) mwx::periph_timer(mwx::periph_timer::TIMER_3);
		(void)new ((void*)&Timer4) mwx::periph_timer(mwx::periph_timer::TIMER_4);

		(void)new ((void*)&Buttons) mwx::periph_buttons();

		(void)new ((void*)&the_mac) mwx::twenet_mac();

		// some settings
		_toconet_tx_u8_init_queue_flag = 0x01; // clean TX QUEUE on wakeup
	}
	else {
		// Hardware basic init
		_MWX_periph_init();

		// 1ms tick counter.
		sToCoNet_AppContext.u16TickHz = 1000;

		// call setup
		setup();

		// check if the_twelite has begun.
		if (!the_twelite._has_setup_finished()) {
			the_twelite.begin(); // force begin.
		}

		// MAC _start
		if (!sToCoNet_AppContext.u8MacInitPending) {
			ToCoNet_vMacStart();
		}
		else {
			; 	// need to manually call mac starting procedure.
		}

	}
}

extern "C" void cbAppWarmStart(bool_t bAfterAhiInit)
{
	if (!bAfterAhiInit) {
		// before AHI init, very first of code.
		
		uint32_t vapp_opt = 0;
		for (auto x : _vcbs) {
			if (*x) x->on_event(_MWX_EV_ON_WARMBOOT, vapp_opt);
		}

		// global def
		init_warmboot();

		// saving interrupt source information
		u32DioPortWakeUp = u32AHI_DioWakeStatus(); // 
		u8TimerWakeUp = u8AHI_WakeTimerFiredStatus(); //  E_AHI_WAKE_TIMER_MASK_0 or  E_AHI_WAKE_TIMER_MASK_1
	}
	else {
		// Serial should be re-initialize firstly.
		Serial._on_wakeup();
		Serial1._on_wakeup();
		
		// other global objects
		_MWX_vOnWakeup();

		// generate net/hw/app wakeup events
		uint32_t vapp_opt = 0;
		for (auto x: _vcbs) {
			if(*x) x->on_event(_MWX_EV_ON_WAKEUP, vapp_opt);
		}

        // call back procedure for waking up.
		wakeup();

		// MAC _start
		if (!sToCoNet_AppContext.u8MacInitPending) {
			ToCoNet_vMacStart();
		}
		else {
			; 	// need to manually call mac starting procedure.
		}
	}
}

extern "C" void cbToCoNet_vMain(void) {
	if (!(_twenet_mod_registered & _TWENET_MOD_BEGIN_PROC)) {
		// calls the first call of 
		_twenet_mod_registered |= _TWENET_MOD_BEGIN_PROC;

		the_twelite._begin(); // begins behaviors
		begin(); // user call back (first time only, OPTIONAL)
	}
	
	loop(); // user call back (every events)
	
	// callbacks for event handler.
	for (auto x : _vcbs) {
		if(*x) x->cbToCoNet_vMain();
	}

	// clear peripheral interrupt record.
	_periph_availmap = 0;
}

extern "C" void cbToCoNet_vNwkEvent(teEvent eEvent, uint32 u32arg) {
	if (the_vapp) the_vapp.cbToCoNet_vHwEvent(eEvent, u32arg);

	mwx::packet_ev_nwk ev;

	ev._network_type = 0;
	ev._network_handled = 0;
	ev.eEvent = uint32_t(eEvent);
	ev.u32arg = u32arg;

	if (the_vnet) {
		the_vnet.cbToCoNet_vNwkEvent(ev);
	}

	if (!ev._network_handled) {
		if (the_vapp) the_vapp.cbToCoNet_vNwkEvent(ev);
		if (the_vhw) the_vhw.cbToCoNet_vNwkEvent(ev);
	}
}

extern "C" void cbToCoNet_vRxEvent(tsRxDataApp *pRx) {
	mwx::packet_rx rx(pRx);

	rx._network_handled = false;

	if (the_vnet) {
		the_vnet.cbToCoNet_vRxEvent(rx);
	}

	if (!rx._network_handled) {
		if (the_vapp) {
			the_vapp.cbToCoNet_vRxEvent(rx);
		}
		else {
			// if app is not defined, use receiver instead.
			the_twelite.receiver._push(rx);
		}

		if (the_vhw) { the_vhw.cbToCoNet_vRxEvent(rx); }
		if (the_vsettings) { the_vsettings.cbToCoNet_vRxEvent(rx); }
	}
}

extern "C" void cbToCoNet_vTxEvent(uint8 u8CbId, uint8 bStatus) {
	mwx::packet_ev_tx ev;

	ev._network_type = 0;
	ev._network_handled = 0;
	ev.u8CbId = u8CbId;
	ev.bStatus = bStatus;

	// before sending a message, set state.
	the_mac.tx_status._tx_event(u8CbId, bStatus);

	// pass an event.
	if (the_vnet) {
		the_vnet.cbToCoNet_vTxEvent(ev);
	}

	if (!ev._network_handled) {
		if (the_vapp) the_vapp.cbToCoNet_vTxEvent(ev);
		if (the_vhw) the_vhw.cbToCoNet_vTxEvent(ev);
		if (the_vsettings) { the_vsettings.cbToCoNet_vTxEvent(ev); }
	}
}

extern "C" void cbToCoNet_vHwEvent(uint32 u32DeviceId, uint32 u32ItemBitmap) {
	// note, if the_vapp.cbToCoNet_u8HwInt() returns true, this won't be called.
	
	// check for Obj.available.
	switch (u32DeviceId) {
	case E_AHI_DEVICE_SYSCTRL:
		if (u32ItemBitmap & ((1UL << 22) | (1UL << 23))) {
			PulseCounter0._int_pc(u32ItemBitmap);
			PulseCounter1._int_pc(u32ItemBitmap);
		}
		break;
	case E_AHI_DEVICE_TICK_TIMER:
		_periph_availmap |= _PERIPH_AVAIL_TICKTIMER;
		if (Buttons) Buttons.tick();
		break;
	case E_AHI_DEVICE_TIMER0:
		_periph_availmap |= _PERIPH_AVAIL_TIMER0; break;
	case E_AHI_DEVICE_TIMER1:
		_periph_availmap |= _PERIPH_AVAIL_TIMER1; break;
	case E_AHI_DEVICE_TIMER2:
		_periph_availmap |= _PERIPH_AVAIL_TIMER2; break;
	case E_AHI_DEVICE_TIMER3:
		_periph_availmap |= _PERIPH_AVAIL_TIMER3; break;
	case E_AHI_DEVICE_TIMER4:
		_periph_availmap |= _PERIPH_AVAIL_TIMER4; break;
	}

	for (auto x : _vcbs) {
		if(*x) x->cbToCoNet_vHwEvent(u32DeviceId, u32ItemBitmap);
	}

#ifdef CHECK_TIMESTAMP
	// just for performance analysis.
	if (E_AHI_DEVICE_TICK_TIMER) {
		_check_timestamp_ev = u32AHI_TickTimerRead();
	}
#endif
}

extern "C" uint8 cbToCoNet_u8HwInt(uint32 u32DeviceId, uint32 u32ItemBitmap) {
	uint8_t ret = 0;

	// check for ADC capturing.
	if (Analogue._adc_event_source(u32DeviceId)) {
		Analogue.tick();
	}

	// call each handler.
	for (auto x : _vcbs) {
		if (*x) {
			ret += x->cbToCoNet_u8HwInt(u32DeviceId, u32ItemBitmap);
		}
	}
	
#ifdef CHECK_TIMESTAMP
	// just for performance analysis.
	if (E_AHI_DEVICE_TICK_TIMER) {
		_check_timestamp = u32AHI_TickTimerRead();
	}
#endif
	// in any case, TICK_TIMER shall return true for further system process.
	if (u32DeviceId == E_AHI_DEVICE_TICK_TIMER) ret = FALSE;

	// TRUE, if no further cbToCoNet_vHwEvent() coming.
	// FALSE, also calls cbToCoNet_vHwEvent() for process bigger task.
	return ret;
}

/**
 * @fn	void _MWX_periph_init()
 *
 * @brief	the very common peripherals initialization,
 * 			only done at abAppColdStart(TRUE).
 *
 */
void _MWX_periph_init() {
	// disable brown out detect
	vAHI_BrownOutConfigure(0,//0:2.0V 1:2.3V
		FALSE,
		FALSE,
		FALSE,
		FALSE);

	// serial port
	Serial.setup(MWX_SER_TX_BUFF, MWX_SER_RX_BUFF); // allocate buffer memory.
	Serial.begin(115200); // UART_0 is started here.
	_psSerial = Serial.get_tsFile();

	// TWENET debug 
	_sSerLegacy.u8Device = E_AHI_UART_0;
	_sSerLegacy.bPutChar = SERIAL_bTxChar;

	ToCoNet_vDebugInit(&_sSerLegacy);
	ToCoNet_vDebugLevel(0);
}

/**
 * @fn	void _MWX_vOnSleep()
 *
 * @brief	called just before sleep.
 * 			mainly stop peripherals.
 * 			
 * 			here, the_vapp.on_sleep() will be called.
 * 			(now vapp_opt is not used, for future use)
 */
void _MWX_vOnSleep() {
	uint32_t vapp_opt = 0;
	for (auto&& x : _vcbs) {
		if(*x) x->on_event(_MWX_EV_ON_SLEEP, vapp_opt);
	}

	Serial._on_sleep();
	Serial1._on_sleep();

	Wire._on_sleep();
	Analogue._on_sleep();
	PulseCounter0._on_sleep();
	PulseCounter1._on_sleep();

	Timer0._on_sleep();
	Timer1._on_sleep();
	Timer2._on_sleep();
	Timer3._on_sleep();
	Timer4._on_sleep();

	Buttons._on_sleep();
	
	(void)u32AHI_DioInterruptStatus(); // clear interrupt status
}

/**
 * @fn	void _MWX_vOnWakeup()
 *
 * @brief	called while cbAppWarmStart(TRUE) clause.
 * 			mainly to initialize peripherals.
 */
void _MWX_vOnWakeup() {
	Wire._on_wakeup();
	Analogue._on_wakeup();
	
	PulseCounter0._on_wakeup();
	PulseCounter1._on_wakeup();

	Timer0._on_wakeup();
	Timer1._on_wakeup();
	Timer2._on_wakeup();
	Timer3._on_wakeup();
	Timer4._on_wakeup();

	Buttons._on_wakeup();
}


/**
 * @brief support function for printf_()
 * 
 * @param c 
 */
void _putchar(char c) {
	Serial << c;
}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
