###### SOURCES(setup(),loop()) ######
APPSRC_GLANCER += glancer.cpp
APPSRC_MINI += mini.cpp

###### SOURCE FROM libsrc/ ######
APPSRC_CXX += mwx_stdio.cpp
APPSRC_C += twesercmd_ascii.c
APPSRC_C += twesercmd_binary.c

###### SOURCE FROM printf/ ######
APPSRC_C += printf.c

###### LIB_MWX_DIR SOURCES ######
APPSRC_CXX += mwx_stream.cpp
APPSRC_CXX += mwx_parser_pkt.cpp
APPSRC_CXX += mwx_utils_crc8.cpp

###### files copied from mwx/twesettings ######
LIB_FILES += libsrc/_mwx_stdio.h
LIB_FILES += libsrc/mwx_serial_con.hpp
LIB_FILES += libsrc/mwx_stdio.cpp
LIB_FILES += libsrc/mwx_stdio.h

LIB_MWX_FILES += mwx_stream.cpp
LIB_MWX_FILES += mwx_parser_pkt.cpp
LIB_MWX_FILES += mwx_utils_crc8.cpp
LIB_MWX_FILES += mwx_debug.h
LIB_MWX_FILES += mwx_common.hpp
LIB_MWX_FILES += mwx_parser.hpp
LIB_MWX_FILES += mwx_parser_pkt.hpp
LIB_MWX_FILES += mwx_stream.hpp
LIB_MWX_FILES += mwx_utils.hpp
LIB_MWX_FILES += mwx_utils_alloc.hpp
LIB_MWX_FILES += mwx_utils_crc8.hpp
LIB_MWX_FILES += mwx_utils_payload.hpp
LIB_MWX_FILES += mwx_utils_smplbuf.hpp

LIB_TWESETTINGS_FILES += twesercmd_ascii.c
LIB_TWESETTINGS_FILES += twesercmd_binary.c
LIB_TWESETTINGS_FILES += twecommon.h
LIB_TWESETTINGS_FILES += twesercmd_gen.h
LIB_TWESETTINGS_FILES += tweserial.h

###### SRC PATH ######
LIB_TWESETTINGS_DIR=libsrc1
LIB_MWX_DIR=libsrc2
INCLUDES += -Ilibsrc -I$(LIB_TWESETTINGS_DIR) -I$(LIB_MWX_DIR) -Iprintf
PATH_LIBSRC = libsrc:$(LIB_TWESETTINGS_DIR):$(LIB_MWX_DIR):printf

###### OPTIONS ######
#DEFINES += -DTWE_STDINOUT_ONLY
DEFINES +=
CFLAGS += -g
CXXFLAGS += -std=c++11

###### COMMON DEFS ######
# check OS
ifeq ($(OS),Windows_NT)
 OSNAME=win
 CXX=g++-9
 CPP=gcc-9
else
 UNAME_S := $(shell uname -s)
 ifeq ($(UNAME_S),Darwin)
  OSNAME=mac
  CXX=clang++
  CPP=clang
 endif
 ifeq ($(UNAME_S),Linux)
  OSNAME=linux
  CXX=g++
  CPP=gcc
 endif
endif

###### OBJS ######
# OBJDIR
OBJDIR=objs
APPOBJS_CXX = $(APPSRC_CXX:%.cpp=$(OBJDIR)/%.o)
APPOBJS_C = $(APPSRC_C:%.c=$(OBJDIR)/%.o)
vpath % $(PATH_LIBSRC):.

# LIBRARY FILES
LIB_MWX_FILES_COPY = $(LIB_MWX_FILES:%=$(LIB_MWX_DIR)/%)
LIB_MWX_FILES_ORIG = $(LIB_MWX_FILES:%=../%)
LIB_TWESETTINGS_FILES_COPY = $(LIB_TWESETTINGS_FILES:%=$(LIB_TWESETTINGS_DIR)/%)
LIB_TWESETTINGS_FILES_ORIG = $(LIB_TWESETTINGS_FILES:%=../../twesettings/%)

LIB_FILES += $(LIB_MWX_FILES_COPY) $(LIB_TWESETTINGS_FILES_COPY)
APPSRC_HPP = $(filter %.hpp,$(LIB_FILES)) $(filter %.h,$(LIB_FILES))
#$(info $(APPSRC_HPP))

# APPS
APPOBJS_GLANCER = $(APPSRC_GLANCER:%.cpp=$(OBJDIR)/%.o)
APPOBJS_MINI = $(APPSRC_MINI:%.cpp=$(OBJDIR)/%.o)

# ALL TARGETS
TARGETS=mini glancer

###### RULES ######
all: $(LIB_MWX_FILES_COPY) $(LIB_TWESETTINGS_FILES_COPY) | $(TARGETS)

$(OBJDIR)/%.o: %.cpp $(APPSRC_HPP)
	$(CXX) -c -o $@ $(CXXFLAGS) $(CFLAGS) $(DEFINES) $(INCLUDES) $< 

$(OBJDIR)/%.o: %.c $(APPSRC_HPP)
	$(CPP) -c -o $@ $(CFLAGS) $(DEFINES) $(INCLUDES) $< 

#$(LIB_MWX_DIR)/%: ../% 
#	cp -v $< $@  

#$(LIB_TWESETTINGS_DIR)/%: ../../twesettings/% 
#	cp -v $< $@  

glancer: $(APPOBJS_GLANCER) $(APPOBJS_CXX) $(APPOBJS_C) $(APPSRC_HPP) 
	$(CXX) -o $@  $(CFLAGS) $(OBJDIR)/$@.o $(APPOBJS_CXX) $(APPOBJS_C)

mini: $(APPOBJS_MINI) $(APPOBJS_CXX) $(APPOBJS_C) $(APPSRC_HPP)
	$(CXX) -o $@  $(CFLAGS) $(OBJDIR)/$@.o $(APPOBJS_CXX) $(APPOBJS_C)

objs:
	mkdir -p $(OBJDIR)

upd-mwx:
	rm -f $(LIB_MWX_DIR)/*
	mkdir -p $(LIB_MWX_DIR)
	cp -pvf $(LIB_MWX_FILES_ORIG) $(LIB_MWX_DIR)

upd-twesettings:
	rm -r $(LIB_TWESETTINGS_DIR)/*
	mkdir -p $(LIB_TWESETTINGS_DIR)
	cp -pvf $(LIB_TWESETTINGS_FILES_ORIG) $(LIB_TWESETTINGS_DIR)

clean: 
	rm -f $(TARGETS) $(APPOBJS_CXX) $(APPOBJS_C) $(APPOBJS_GLANCER) $(APPOBJS_MINI) 

test:
	@-echo
	@-bash -c "echo -n [TEST mini.cpp]\ "
	-./mini < test_msg.txt
	@-echo
	@-bash -c "echo -n [TEST glancer.cpp]\ "
	-./glancer < test_msg.txt
