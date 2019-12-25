/* Copyright (C) 2019 Mono Wireless Inc. All Rights Reserved.    *
 * Released under MW-SLA-*J,*E (MONO WIRELESS SOFTWARE LICENSE   *
 * AGREEMENT).                                                   */
#pragma once

/*** The mwx defs */
#if !defined(MWX_SER_TX_BUFF)
#define MWX_SER_TX_BUFF 768
#endif

#if !defined(MWX_SER_RX_BUFF)
#define MWX_SER_RX_BUFF 256
#endif

#if !defined(MWX_TWOWIRE_BUFF)
#define MWX_TWOWIRE_BUFF 32
#endif

/**
 * user defined procedure, intialization procedure.
 */
extern void setup();

/**
 * user defined procedure, intialization procedure.
 */
extern void begin();

/**
 * user defined procedure, main loop.
 */
extern void loop();

/**
 * user defined procedure, after wake up.
 */
extern void wakeup();
