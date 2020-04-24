APPSRC_CXX+=mwx_utils_crc8.cpp
APPSRC_CXX+=mwx_appcore.cpp
APPSRC_CXX+=mwx_memory.cpp
APPSRC_CXX+=mwx_periph.cpp
APPSRC_CXX+=mwx_periph_dio.cpp
APPSRC_CXX+=mwx_periph_buttons.cpp
APPSRC_CXX+=mwx_periph_analogue.cpp
APPSRC_CXX+=mwx_stream.cpp
APPSRC_CXX+=mwx_duplicate_checker.cpp

APPSRC_CXX+=mwx_settings.cpp
APPSRC_CXX+=mwx_settings_default.cpp

APPSRC_CXX+=boards/mwx_brd_pal_amb.cpp
APPSRC_CXX+=boards/mwx_brd_pal_mot.cpp
APPSRC_CXX+=boards/mwx_brd_pal_mag.cpp
APPSRC_CXX+=boards/mwx_brd_monostick.cpp
APPSRC_CXX+=boards/mwx_brd_app_twelite.cpp

APPSRC_CXX+=networks/mwx_nwk_simple.cpp

APPSRC+=sensors/legacy/SMBus.c
APPSRC+=sensors/legacy/sensor_driver.c
APPSRC+=sensors/legacy/SHTC3.c
APPSRC+=sensors/legacy/LTR308ALS.c
APPSRC_CXX+=sensors/legacy/probe.cpp
APPSRC_CXX+=sensors/mwx_sns_MC3630.cpp
# APPSRC+=sensors/legacy/SPI.c
# APPSRC+=sensors/legacy/MC3630.c

APPSRC+=mwx_debug.c