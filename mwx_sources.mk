APPSRC_CXX+=mwx_utils_crc8.cpp
APPSRC_CXX+=mwx_utils.cpp
APPSRC_CXX+=mwx_appcore.cpp
APPSRC_CXX+=mwx_memory.cpp
APPSRC_CXX+=mwx_periph.cpp
APPSRC_CXX+=mwx_periph_dio.cpp
APPSRC_CXX+=mwx_periph_buttons.cpp
APPSRC_CXX+=mwx_periph_analogue.cpp
APPSRC_CXX+=mwx_stream.cpp
APPSRC_CXX+=mwx_duplicate_checker.cpp

APPSRC_CXX+=settings/mwx_settings.cpp
APPSRC_CXX+=settings/mwx_stgs_standard.cpp
APPSRC_CXX+=settings/mwx_stgs_standard_core.cpp
APPSRC_CXX+=settings/mwx_stgs_standard_tbl.cpp

APPSRC_CXX+=boards/mwx_brd_pal_amb.cpp
APPSRC_CXX+=boards/mwx_brd_pal_mot.cpp
APPSRC_CXX+=boards/mwx_brd_pal_mag.cpp
APPSRC_CXX+=boards/mwx_brd_pal_notice.cpp
APPSRC_CXX+=boards/mwx_brd_monostick.cpp
APPSRC_CXX+=boards/mwx_brd_app_twelite.cpp

APPSRC_CXX+=networks/mwx_nwk_simple.cpp

APPSRC+=sensors/legacy/SMBus.c
APPSRC+=sensors/legacy/sensor_driver.c
APPSRC+=sensors/legacy/SHTC3.c
APPSRC+=sensors/legacy/SHT31.c
APPSRC+=sensors/legacy/LTR308ALS.c
APPSRC+=sensors/legacy/BME280.c
APPSRC_CXX+=sensors/legacy/probe.cpp
APPSRC_CXX+=sensors/mwx_sns_MC3630.cpp
APPSRC_CXX+=sensors/mwx_sns_PCA9632.cpp

APPSRC+=mwx_debug.c