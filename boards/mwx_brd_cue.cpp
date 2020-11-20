/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */

// mwx header
#include "mwx_brd_cue.hpp"

/*****************************************************************/
namespace mwx {
// MUST DEFINE CLASS NAME HERE
#define __MWX_APP_CLASS_NAME BrdCUE
#include "../_mwx_cbs_cpphead.hpp"
/*****************************************************************/

	// INT handler: MOT SENSOR INT.
#if 0 // no need to add this hander explicitely,
	  // only if setting handled true.
	MWX_DIO_INT(BrdCUE::PIN_SNS_INT, uint32_t arg, uint8_t& handled) {
		handled = true; // if true, no further event.
	}
#endif

	// for non-sleeping application, read MOT SENSOR FIFO on the
	// pin event.
	// normally, this should be done at DIO_EVENT to avoid
	// considering critical section while operating the queue.
	MWX_DIO_EVENT(BrdCUE::PIN_SNS_INT, uint32_t arg) {
		sns_MC3630._read_result(); // read into queue and clear INT.
	}

/*****************************************************************/
// common procedure (DO NOT REMOVE)
#include "../_mwx_cbs_cpptail.cpp"
// MUST UNDEF CLASS NAME HERE
#undef __MWX_APP_CLASS_NAME
} // mwx
/*****************************************************************/


