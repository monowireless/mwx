/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */

// implementation of user defined application class


uint8 __MWX_APP_CLASS_NAME::cbTweNet_u8HwInt(uint32 u32DeviceId, uint32 u32ItemBitmap) {
    bool_t b_handled = FALSE;
    switch (u32DeviceId) {
	case E_AHI_DEVICE_TICK_TIMER:
		int_ticktimer_handler<0>(u32ItemBitmap, b_handled);
		break;

	case E_AHI_DEVICE_ANALOGUE:
		break;

	case E_AHI_DEVICE_SYSCTRL:
		#define __MWX_INT_HANDLER(n) if (u32ItemBitmap & (1UL << n))  { int_dio_handler<n>(n, b_handled); }
		__MWX_INT_HANDLER(0)
		__MWX_INT_HANDLER(1)
		__MWX_INT_HANDLER(2)
		__MWX_INT_HANDLER(3)
		__MWX_INT_HANDLER(4)
		__MWX_INT_HANDLER(5)
		__MWX_INT_HANDLER(6)
		__MWX_INT_HANDLER(7)
		__MWX_INT_HANDLER(8)
		__MWX_INT_HANDLER(9)
		__MWX_INT_HANDLER(10)
		__MWX_INT_HANDLER(11)
		__MWX_INT_HANDLER(12)
		__MWX_INT_HANDLER(13)
		__MWX_INT_HANDLER(14)
		__MWX_INT_HANDLER(15)
		__MWX_INT_HANDLER(16)
		__MWX_INT_HANDLER(17)
		__MWX_INT_HANDLER(18)
		__MWX_INT_HANDLER(19)
		;
		break;

	case E_AHI_DEVICE_TIMER0:
		int_timer_handler<0>(u32ItemBitmap, b_handled);
		break;
	case E_AHI_DEVICE_TIMER1:
		int_timer_handler<1>(u32ItemBitmap, b_handled);
		break;
	case E_AHI_DEVICE_TIMER2:
		int_timer_handler<2>(u32ItemBitmap, b_handled);
		break;
	case E_AHI_DEVICE_TIMER3:
		int_timer_handler<3>(u32ItemBitmap, b_handled);
		break;
	case E_AHI_DEVICE_TIMER4:
		int_timer_handler<4>(u32ItemBitmap, b_handled);
		break;

	default:
		int_misc_handler<0>(u32DeviceId, u32ItemBitmap, b_handled);
		break;
	}
    return b_handled;
}


void __MWX_APP_CLASS_NAME::cbTweNet_vHwEvent(uint32 u32DeviceId, uint32 u32ItemBitmap) {
	switch (u32DeviceId) {
	case E_AHI_DEVICE_TICK_TIMER:
		event_ticktimer_handler<0>(u32ItemBitmap);
		break;

	case E_AHI_DEVICE_ANALOGUE:
		break;

	case E_AHI_DEVICE_SYSCTRL:
		#define __MWX_EVENT_HANDLER(n) if (u32ItemBitmap && (1UL << n))  { event_dio_handler<n>(n); }
		__MWX_EVENT_HANDLER(0)
		__MWX_EVENT_HANDLER(1)
		__MWX_EVENT_HANDLER(2)
		__MWX_EVENT_HANDLER(3)
		__MWX_EVENT_HANDLER(4)
		__MWX_EVENT_HANDLER(5)
		__MWX_EVENT_HANDLER(6)
		__MWX_EVENT_HANDLER(7)
		__MWX_EVENT_HANDLER(8)
		__MWX_EVENT_HANDLER(9)
		__MWX_EVENT_HANDLER(10)
		__MWX_EVENT_HANDLER(11)
		__MWX_EVENT_HANDLER(12)
		__MWX_EVENT_HANDLER(13)
		__MWX_EVENT_HANDLER(14)
		__MWX_EVENT_HANDLER(15)
		__MWX_EVENT_HANDLER(16)
		__MWX_EVENT_HANDLER(17)
		__MWX_EVENT_HANDLER(18)
		__MWX_EVENT_HANDLER(19)
		;
		break;

	case E_AHI_DEVICE_TIMER0:
		event_timer_handler<0>(u32ItemBitmap);
		break;

	case E_AHI_DEVICE_TIMER1:
		event_timer_handler<1>(u32ItemBitmap);
		break;

	case E_AHI_DEVICE_TIMER2:
		event_timer_handler<2>(u32ItemBitmap);
		break;

	case E_AHI_DEVICE_TIMER3:
		event_timer_handler<3>(u32ItemBitmap);
		break;

	case E_AHI_DEVICE_TIMER4:
		event_timer_handler<4>(u32ItemBitmap);
		break;

	default:
		event_misc_handler<0>(u32DeviceId, u32ItemBitmap);
		break;
	}

	//DPRINT("b");
}

// STATE MACHINE
void __MWX_APP_CLASS_NAME::processStateMachine(tsEvent* pEv, teEvent eEvent, uint32_t u32evarg) {
	// dispatch event
	switch (pEv->eState) {
	case E_MWX::STATE_0: state_handler<E_MWX::STATE_0>((uint32_t)eEvent, (uint32_t)u32evarg); break;
	case E_MWX::STATE_1: state_handler<E_MWX::STATE_1>((uint32_t)eEvent, (uint32_t)u32evarg); break;
	case E_MWX::STATE_2: state_handler<E_MWX::STATE_2>((uint32_t)eEvent, (uint32_t)u32evarg); break;
	case E_MWX::STATE_3: state_handler<E_MWX::STATE_3>((uint32_t)eEvent, (uint32_t)u32evarg); break;
	case E_MWX::STATE_4: state_handler<E_MWX::STATE_4>((uint32_t)eEvent, (uint32_t)u32evarg); break;
	case E_MWX::STATE_5: state_handler<E_MWX::STATE_5>((uint32_t)eEvent, (uint32_t)u32evarg); break;
	case E_MWX::STATE_6: state_handler<E_MWX::STATE_6>((uint32_t)eEvent, (uint32_t)u32evarg); break;
	case E_MWX::STATE_7: state_handler<E_MWX::STATE_7>((uint32_t)eEvent, (uint32_t)u32evarg); break;
	case E_MWX::STATE_8: state_handler<E_MWX::STATE_8>((uint32_t)eEvent, (uint32_t)u32evarg); break;
	case E_MWX::STATE_9: state_handler<E_MWX::STATE_9>((uint32_t)eEvent, (uint32_t)u32evarg); break;
	}
}
