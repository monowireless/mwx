/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */

#ifdef MWX_DIO_INT
#undef MWX_DIO_INT
#endif
#define MWX_DIO_INT(N, arg, handled) template<> void __MWX_APP_CLASS_NAME::int_dio_handler<N>(arg, handled)

#ifdef MWX_TICKTIMER_INT
#undef MWX_TICKTIMER_INT
#endif
#define MWX_TICKTIMER_INT(arg, handled) template<> void __MWX_APP_CLASS_NAME::int_ticktimer_handler<0>(arg, handled)

#ifdef MWX_TIMER_INT
#undef MWX_TIMER_INT
#endif
#define MWX_TIMER_INT(N, arg, handled) template<> void __MWX_APP_CLASS_NAME::int_timer_handler<N>(arg, handled)

#ifdef MWX_MISC_INT
#undef MWX_MISC_INT
#endif
#define MWX_MISC_INT(arg, arg2, handled) template<> void __MWX_APP_CLASS_NAME::int_misc_handler<0>(arg, arg2, handled)

#ifdef MWX_DIO_EVENT
#undef MWX_DIO_EVENT
#endif
#define MWX_DIO_EVENT(N, arg) template<> void __MWX_APP_CLASS_NAME::event_dio_handler<N>(arg)

#ifdef MWX_TICKTIMER_EVENT
#undef MWX_TICKTIMER_EVENT
#endif
#define MWX_TICKTIMER_EVENT(arg) template<> void __MWX_APP_CLASS_NAME::event_ticktimer_handler<0>(arg)

#ifdef MWX_TIMER_EVENT
#undef MWX_TIMER_EVENT
#endif
#define MWX_TIMER_EVENT(N, arg) template<> void __MWX_APP_CLASS_NAME::event_timer_handler<N>(arg)

#ifdef MWX_MISC_EVENT
#undef MWX_MISC_EVENT
#endif
#define MWX_MISC_EVENT(arg, arg2) template<> void __MWX_APP_CLASS_NAME::event_misc_handler<0>(arg, arg2)

#ifdef MWX_STATE
#undef MWX_STATE
#endif
#define MWX_STATE(N, ev, arg) template <> void __MWX_APP_CLASS_NAME::state_handler<N>(ev, arg)
