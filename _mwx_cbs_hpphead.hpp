/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */

public:
using SUPER_CallBacks = mwx::appdefs_crtp<__MWX_APP_CLASS_NAME>;
using SUPER_States = mwx::processev_crtp<__MWX_APP_CLASS_NAME>;

// state machine
void processStateMachine(tsEvent* pEv, teEvent eEvent, uint32_t u32evarg);
template<int N> void state_handler(uint32_t ev, uint32_t arg) { ; }

// necessary code for user defined application class
uint8 cbTweNet_u8HwInt(uint32 u32DeviceId, uint32 u32ItemBitmap);
template<int N> void int_dio_handler(uint32_t arg, uint8_t& handled) { ; }
template<int N> void int_ticktimer_handler(uint32_t arg, uint8_t& handled) { ; }
template<int N> void int_timer_handler(uint32_t arg, uint8_t& handled) { ; }
template<int N> void int_misc_handler(uint32_t arg, uint32_t arg2, uint8_t& handled) { ; }

void cbTweNet_vHwEvent(uint32 u32DeviceId, uint32 u32ItemBitmap);
template<int N> void event_dio_handler(uint32_t arg) { ; }
template<int N> void event_ticktimer_handler(uint32_t arg) { ; }
template<int N> void event_timer_handler(uint32_t arg) { ; }
template<int N> void event_misc_handler(uint32_t arg, uint32_t arg2) { ; }

private:
__MWX_APP_CLASS_NAME(const __MWX_APP_CLASS_NAME&) = delete; // remove copy constructor
__MWX_APP_CLASS_NAME& operator =(const __MWX_APP_CLASS_NAME&) = delete; // remove assign operator
