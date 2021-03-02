#
# Paintball2 Makefile for Linux
#
# Nov '97 by Zoid <zoid@idsoftware.com>
# Modified Jan 2010 by jitspoe
#
# ELF only
#

# Check OS type.
OSTYPE := $(shell uname -s)

# This nice line comes from the linux kernel makefile
ARCH := $(shell uname -m | sed -e s/i.86/i386/ -e s/sun4u/sparc/ -e s/sparc64/sparc/ -e s/arm.*/arm/ -e s/sa110/arm/ -e s/alpha/axp/)

ifneq ($(OSTYPE),Linux)
ifneq ($(OSTYPE),FreeBSD)
$(error OS $(OSTYPE) is currently not supported)
endif
endif

ifneq ($(ARCH),x86_64)
ifneq ($(ARCH),i386)
ifneq ($(ARCH),ppc)
$(error arch $(ARCH) is currently not supported)
endif
endif
endif


# start of configurable options

# Here are your build options:
# (Note: not all options are available for all platforms).
# paintball2 (uses OSS for sound, cdrom ioctls for cd audio) is automatically built.
# game$(ARCH).so is automatically built.
#

#=============================#
# Set to YES or NO for a      #
# verbose compilation process.#
#=============================#
VERBOSE?=YES

#=============================#
# Paintball2 Client           #
#=============================#
BUILD_PB2?=YES

#=============================#
# Bot library                 #
#=============================#
BUILD_BOTLIB?=YES

#=============================#
# Paintball2 dedicated server #
#=============================#
BUILD_DEDICATED?=NO

#=============================#
# Compile with IPv6 (protocol #
# independent API). Tested on #
# FreeBSD.                    #
#=============================#
HAVE_IPV6?=NO 


#USE_SSE?=YES
ENABLE_PROFILING?=NO
USE_X86_ASM?=NO

#===========================#
# Read from $(DATADIR) and  #
# write to "~/.paintball2"  #
#===========================#
WITH_DATADIR?=NO

#===========================#
# Load renderers            #
# from $(LIBDIR)            #
#===========================#
WITH_LIBDIR?=NO

#===========================#
# Sound Drivers             #            
#===========================#
ifeq ($(OSTYPE),Linux)
BUILD_ALSA_SND?=YES      # Alsa sound driver
endif
BUILD_OSS_SND?=YES       # OSS sound driver
BUILD_SDL_SND?=YES       # SDL sound driver

#===========================#
# OGG Vorbis Support        #
#===========================#
WITH_OGG_SUPPORT?=NO

#===========================#
# GLX render.               #
#===========================#
BUILD_GLX?=YES

#===========================#
# Built in DGA mouse        #
# support.                  #
#===========================#
USE_XF86_DGA?=NO

#===========================#
# SDL GL render.            #
#===========================#
BUILD_SDLGL?=NO


#===========================#
# Joystick support.         #
#===========================#
BUILD_JOYSTICK?=NO

# (hopefully) end of configurable options

VERSION=2.0
BUILD=27

MOUNT_DIR=.
BINDIR=~/paintball2

BUILD_DEBUG_DIR=build_debug
BUILD_RELEASE_DIR=build_release

X11BASE?= /usr/X11R6
LOCALBASE?= /usr/local
GAMEBASE?=/usr/local
SYSBINDIR?=$(LOCALBASE)/bin

ifeq ($(OSTYPE),FreeBSD)
 DATADIR?= $(GAMEBASE)/share/paintball2
 LIBDIR?= $(GAMEBASE)/lib/paintball2
else
 DATADIR?= $(GAMEBASE)/games/paintball2
 LIBDIR?= $(GAMEBASE)/games/paintball2
endif

SHLIBEXT=so
GAME_NAME=game.$(SHLIBEXT)
SHLIBCFLAGS=-fPIC
SHLIBLDFLAGS=-shared

ifeq ($(OSTYPE),Linux)
ifeq ($(ARCH),i386)
GAME_NAME=game$(ARCH).$(SHLIBEXT)
endif
endif

CLIENT_DIR=$(MOUNT_DIR)/client
A3D_DIR=$(MOUNT_DIR)/a3d
SERVER_DIR=$(MOUNT_DIR)/server
REF_GL_DIR=$(MOUNT_DIR)/ref_gl
BOTLIB_DIR=$(MOUNT_DIR)/bots
COMMON_DIR=$(MOUNT_DIR)/qcommon
LINUX_DIR=$(MOUNT_DIR)/linux
GAME_DIR=$(MOUNT_DIR)/game

CC?=gcc

BASE_CFLAGS=-I$(LOCALBASE)/include -I$(X11BASE)/include \
            -Dstricmp=strcasecmp -D_inline= -DSleep=usleep \
            -DGAME_NAME='\"$(GAME_NAME)\"'\
	    -DPAINTBALL2_VERSION='\"$(VERSION)\"' \
	    -DBUILD_VERSION='\"$(BUILD)\"' -Wno-pointer-sign \
            -D_THREAD_SAFE -fno-stack-protector -msse

ifneq ($(strip $(USE_X86_ASM)),YES)
BASE_CFLAGS+=-DC_ONLY
endif

ifeq ($(strip $(USE_XF86_DGA)),YES)
BASE_CFLAGS+=-DUSE_XF86_DGA
endif

ifeq ($(strip $(WITH_OGG_SUPPORT)),YES)
BASE_CFLAGS+=-DOGG_SUPPORT
endif

ifeq ($(ENABLE_PROFILING),YES)
BASE_CFLAGS+= -pg
endif

ifeq ($(HAVE_IPV6),YES)
BASE_CFLAGS+= -DHAVE_IPV6
ifeq ($(OSTYPE),FreeBSD)
BASE_CFLAGS+= -DHAVE_SIN6_LEN
endif
NET_UDP=net_udp6
else
NET_UDP=net_udp
endif

ifeq ($(strip $(BUILD_JOYSTICK)),YES)
BASE_CFLAGS+=-DJoystick
endif

ifneq ($(ARCH),i386)
BASE_CFLAGS+=-DC_ONLY
endif

ifeq ($(strip $(WITH_DATADIR)),YES)
 BASE_CFLAGS+=-DDATADIR='\"$(DATADIR)\"'
endif

ifeq ($(strip $(WITH_LIBDIR)),YES)
 BASE_CFLAGS+=-DLIBDIR='\"$(LIBDIR)\"'
endif

#ifeq ($(ARCH),i386)
#RELEASE_CFLAGS=$(BASE_CFLAGS) -O2 -mmmx \
#	-ffast-math -ffloat-store -funroll-loops -fno-strict-aliasing
RELEASE_CFLAGS=$(BASE_CFLAGS) \
	-ffast-math -ffloat-store -funroll-loops
#endif

ifeq ($(ARCH),ppc)
RELEASE_CFLAGS=$(BASE_CFLAGS) -O2 -ffast-math -funroll-loops \
	-fomit-frame-pointer -fexpensive-optimizations
endif

ifeq ($(USE_SSE),YES)
RELEASE_CFLAGS+=-msse -mfpmath=sse
endif

ifeq ($(ARCH),x86_64)
_LIB := 64
RELEASE_CFLAGS=$(BASE_CFLAGS) -O2 -ffast-math -funroll-loops \
	-fomit-frame-pointer -fexpensive-optimizations -fno-strict-aliasing
endif

DEBUG_CFLAGS=$(BASE_CFLAGS) -g -ggdb -pipe -ffloat-store -ffast-math -D_DEBUG 

LDFLAGS=-L$(LOCALBASE)/lib -lm

ifeq ($(OSTYPE),Linux)
LDFLAGS+= -ldl -lrt
endif

ifeq ($(OSTYPE),FreeBSD)
LDFLAGS+= -pthread
endif

ifeq ($(OSTYPE),Linux)
ifeq ($(strip $(BUILD_ALSA_SND)),YES)
ALSALDFLAGS=-lasound
endif
endif

XLDFLAGS=-L$(X11BASE)/lib$(_LIB) -lX11 -lXext -lXxf86vm
ifeq ($(strip $(USE_XF86_DGA)),YES)
XLDFLAGS+= -lXxf86dga
endif

ifeq ($(strip $(WITH_OGG_SUPPORT)),YES)
LDFLAGS+= -lvorbisfile -lvorbis -logg 
endif

GLX_LDFLAGS=-L$(X11BASE)/lib$(_LIB) -L$(LOCALBASE)/lib$(_LIB) -lGL -ljpeg -lpng

SDLCONFIG?= sdl-config
SDLCFLAGS= $(shell $(SDLCONFIG) --cflags)
SDLLDFLAGS= $(shell $(SDLCONFIG) --libs)
SDLGLCFLAGS=$(SDLCFLAGS)
SDLGLLDFLAGS=$(SDLLDFLAGS)

ifneq ($(VERBOSE),YES)
CC_OUTPUT=echo "Compiling >> $<" &&
endif

DO_CC=$(CC_OUTPUT) $(CC) $(CFLAGS) -o $@ -c $<
DO_DED_CC=$(CC_OUTPUT) $(CC) $(CFLAGS) -DDEDICATED_ONLY -o $@ -c $<
DO_SHLIB_CC=$(CC_OUTPUT) $(CC) $(CFLAGS) $(SHLIBCFLAGS) -o $@ -c $<
DO_AS=$(CC_OUTPUT) $(CC) $(CFLAGS) -DELF -x assembler-with-cpp -o $@ -c $<

#############################################################################
# SETUP AND BUILD
#############################################################################

ifeq ($(strip $(BUILD_PB2)),YES)
TARGETS += $(BINDIR)/paintball2
endif

ifeq ($(strip $(BUILD_BOTLIB)),YES)
TARGETS += $(BINDIR)/dp_botlib.$(SHLIBEXT)
endif

ifeq ($(strip $(BUILD_DEDICATED)),YES)
TARGETS += $(BINDIR)/pb2-ded
endif

ifeq ($(strip $(BUILD_ALSA_SND)),YES)
TARGETS += $(BINDIR)/snd_alsa.$(SHLIBEXT)
endif

ifeq ($(strip $(BUILD_OSS_SND)),YES)
TARGETS += $(BINDIR)/snd_oss.$(SHLIBEXT)
endif

ifeq ($(strip $(BUILD_SDL_SND)),YES)
TARGETS += $(BINDIR)/snd_sdl.$(SHLIBEXT)
endif

ifeq ($(strip $(BUILD_GLX)),YES)
TARGETS += $(BINDIR)/ref_pbgl.$(SHLIBEXT)
endif

ifeq ($(strip $(BUILD_SDLGL)),YES)
TARGETS += $(BINDIR)/ref_pbsdl.$(SHLIBEXT)
endif

all:
	@echo 
	@echo Set to YES or NO at the top of this file the possible options to build by the makefile.
	@echo By default, it will build paintball2 and glx renderer.
	@echo 
	@echo Possible targets:
	@echo
	@echo "   "">> Add VERBOSE=YES to a verbose compilation output, defaults to NO"
	@echo "   "">> make release"
	@echo "   "">> make debug"
	@echo "   "">> make install (paintball2 home dir)."
	@echo "   "">> make install_root (required when was built with DATADIR/LIBDIR" 
	@echo "   ""                      options enabled, you must gain root privileges)."
	@echo "   "">> make clean (clean objects)."
	@echo "   "">> make clean_bin (clean executables)."
	@echo "   "">> make distclean (clean objects, executables and modified files)."
	@echo "   "">> make bz2 (create a tar.bz2 package with the full release distribution)."
	@echo

debug:
	@-mkdir -p $(BUILD_DEBUG_DIR) \
		$(BINDIR) \
		$(BUILD_DEBUG_DIR)/a3d \
		$(BUILD_DEBUG_DIR)/client
		
ifeq ($(strip $(BUILD_DEDICATED)),YES)
	@-mkdir -p $(BUILD_DEBUG_DIR) \
		$(BUILD_DEBUG_DIR)/ded 
endif

ifeq ($(strip $(BUILD_BOTLIB)),YES)
	@-mkdir -p $(BUILD_DEBUG_DIR) \
		$(BUILD_DEBUG_DIR)/bots 
endif

ifeq ($(strip $(BUILD_GLX)),YES)
	@-mkdir -p $(BUILD_DEBUG_DIR) \
		$(BUILD_DEBUG_DIR)/ref_gl
endif
		
ifeq ($(strip $(BUILD_SDLGL)),YES)
	@-mkdir -p $(BUILD_DEBUG_DIR) \
		$(BUILD_DEBUG_DIR)/ref_gl
endif

ifneq ($(VERBOSE),YES)
	@-$(MAKE) --silent targets BUILDDIR=$(BUILD_DEBUG_DIR) CFLAGS="$(DEBUG_CFLAGS)"
else
	$(MAKE) targets BUILDDIR=$(BUILD_DEBUG_DIR) CFLAGS="$(DEBUG_CFLAGS)"
endif

release:
	@-mkdir -p $(BUILD_RELEASE_DIR) \
		$(BINDIR) \
		$(BUILD_RELEASE_DIR)/a3d \
		$(BUILD_RELEASE_DIR)/client
		
ifeq ($(strip $(BUILD_DEDICATED)),YES)
	@-mkdir -p $(BUILD_RELEASE_DIR) \
		$(BUILD_RELEASE_DIR)/ded 
endif

ifeq ($(strip $(BUILD_BOTLIB)),YES)
	@-mkdir -p $(BUILD_RELEASE_DIR) \
		$(BUILD_RELEASE_DIR)/bots 
endif

ifeq ($(strip $(BUILD_GLX)),YES)
	@-mkdir -p $(BUILD_RELEASE_DIR) \
		$(BUILD_RELEASE_DIR)/ref_gl 
endif
		
ifeq ($(strip $(BUILD_SDLGL)),YES)
	@-mkdir -p $(BUILD_RELEASE_DIR) \
		$(BUILD_RELEASE_DIR)/ref_gl 
endif

ifneq ($(VERBOSE),YES)
	@-$(MAKE) --silent targets BUILDDIR=$(BUILD_RELEASE_DIR) CFLAGS="$(RELEASE_CFLAGS)"
else
	$(MAKE) targets BUILDDIR=$(BUILD_RELEASE_DIR) CFLAGS="$(RELEASE_CFLAGS)"
endif


targets: $(TARGETS)
	@echo "Paintball2 compilation finished succesfully =)"

#############################################################################
# CLIENT/SERVER
#############################################################################

PB2_OBJS = \
	$(BUILDDIR)/a3d/q2a3d.o        $(BUILDDIR)/client/cl_cin.o        $(BUILDDIR)/client/cl_ents.o \
	$(BUILDDIR)/client/cl_fx.o     $(BUILDDIR)/client/cl_input.o      $(BUILDDIR)/client/cl_inv.o \
	$(BUILDDIR)/client/cl_main.o   $(BUILDDIR)/client/cl_parse.o      $(BUILDDIR)/client/cl_pred.o \
	$(BUILDDIR)/client/cl_tent.o   $(BUILDDIR)/client/cl_scrn.o       $(BUILDDIR)/client/cl_view.o \
	$(BUILDDIR)/client/cl_newfx.o  $(BUILDDIR)/client/console.o       $(BUILDDIR)/client/keys.o \
	$(BUILDDIR)/client/menu.o      $(BUILDDIR)/client/cl_vote.o       $(BUILDDIR)/client/cl_loc.o \
	$(BUILDDIR)/client/cl_scores.o $(BUILDDIR)/client/cl_serverlist.o $(BUILDDIR)/client/cl_decode.o \
	$(BUILDDIR)/client/cl_hud.o    $(BUILDDIR)/client/cl_images.o     $(BUILDDIR)/client/snd_dma.o \
	$(BUILDDIR)/client/snd_mem.o   $(BUILDDIR)/client/snd_mix.o       $(BUILDDIR)/client/m_flash.o \
	$(BUILDDIR)/client/cmd.o       $(BUILDDIR)/client/cmodel.o        $(BUILDDIR)/client/common.o \
	$(BUILDDIR)/client/crc.o       $(BUILDDIR)/client/cvar.o          $(BUILDDIR)/client/files.o \
	$(BUILDDIR)/client/md4.o       $(BUILDDIR)/client/net_chan.o      $(BUILDDIR)/client/cl_profile.o \
	$(BUILDDIR)/client/sv_ccmds.o  $(BUILDDIR)/client/sv_ents.o       $(BUILDDIR)/client/sv_game.o \
	$(BUILDDIR)/client/sv_init.o   $(BUILDDIR)/client/sv_main.o       $(BUILDDIR)/client/sv_send.o \
	$(BUILDDIR)/client/sv_user.o   $(BUILDDIR)/client/sv_world.o      $(BUILDDIR)/client/md5.o \
	$(BUILDDIR)/client/q_shlinux.o $(BUILDDIR)/client/vid_so.o        $(BUILDDIR)/client/sys_linux.o \
	$(BUILDDIR)/client/glob.o      $(BUILDDIR)/client/$(NET_UDP).o    $(BUILDDIR)/client/net_common.o \
	$(BUILDDIR)/client/net_tcp.o   $(BUILDDIR)/client/q_shared.o      $(BUILDDIR)/client/pmove.o \
	$(BUILDDIR)/client/snd_ogg.o   $(BUILDDIR)/client/vid_common.o	  $(BUILDDIR)/client/cl_stats.o \
	$(BUILDDIR)/client/cl_irc.o \
	

CD_OBJS = \
	$(BUILDDIR)/client/cd_linux.o 
	
ifeq ($(OSTYPE),Linux)
ifeq ($(strip $(BUILD_ALSA_SND)),YES)
ALSA_SND_OBJS= $(BUILDDIR)/client/snd_alsa.o
endif
endif

ifeq ($(strip $(BUILD_OSS_SND)),YES)
OSS_SND_OBJS= $(BUILDDIR)/client/snd_oss.o
endif

ifeq ($(strip $(BUILD_SDL_SND)),YES)
SDL_SND_OBJS= $(BUILDDIR)/client/snd_sdl.o
endif

ifeq ($(ARCH),i386)
ifeq ($(strip $(USE_X86_ASM)),YES)
PB2_AS_OBJS = $(BUILDDIR)/client/snd_mixa.o
endif
else
PB2_AS_OBJS =  #blank
endif

$(BINDIR)/paintball2 : $(PB2_OBJS) $(CD_OBJS) $(PB2_AS_OBJS)
	@echo
	@echo "**Built Paintball2 client with cflags:"
	@echo "$(CFLAGS)";
	@echo
	@echo "**Now linking Paintball2 client with flags:"
	@echo "$(LDFLAGS)";
	@echo
	$(CC) $(CFLAGS) -o $@ $(PB2_OBJS) $(CD_OBJS) $(PB2_AS_OBJS) $(LDFLAGS)

ifeq ($(OSTYPE),Linux)
$(BINDIR)/snd_alsa.$(SHLIBEXT) : $(ALSA_SND_OBJS)
	@echo
	@echo "**Built Alsa sound driver with cflags:"
	@echo "$(CFLAGS)";
	@echo
	@echo "**Now linking Alsa sound driver with flags:"
	@echo "$(LDFLAGS) $(SHLIBLDFLAGS) $(ALSALDFLAGS)";
	@echo
	$(CC) $(CFLAGS) $(SHLIBLDFLAGS) -o $@ $(ALSA_SND_OBJS) $(LDFLAGS) $(ALSALDFLAGS)
endif
	
$(BINDIR)/snd_oss.$(SHLIBEXT) : $(OSS_SND_OBJS)
	@echo
	@echo "**Built OSS sound driver with cflags:"
	@echo "$(CFLAGS)";
	@echo
	@echo "**Now linking OSS sound driver with flags:"
	@echo "$(LDFLAGS) $(SHLIBLDFLAGS)";
	@echo
	$(CC) $(CFLAGS) $(SHLIBLDFLAGS) -o $@ $(OSS_SND_OBJS) $(LDFLAGS)

$(BINDIR)/snd_sdl.$(SHLIBEXT) : $(SDL_SND_OBJS)
	@echo
	@echo "**Built SDL sound driver with cflags:"
	@echo "$(CFLAGS)";
	@echo
	@echo "**Now linking SDL sound driver with flags:"
	@echo "$(LDFLAGS) $(SHLIBLDFLAGS) $(SDLLDFLAGS)";
	@echo
	$(CC) $(CFLAGS) $(SHLIBLDFLAGS) -o $@ $(SDL_SND_OBJS) $(LDFLAGS) $(SDLLDFLAGS)

$(BUILDDIR)/a3d/q2a3d.o :	     $(A3D_DIR)/q2a3d.c;		$(DO_CC)
$(BUILDDIR)/client/cl_cin.o :	     $(CLIENT_DIR)/cl_cin.c;		$(DO_CC)
$(BUILDDIR)/client/cl_ents.o :	     $(CLIENT_DIR)/cl_ents.c;		$(DO_CC)
$(BUILDDIR)/client/cl_profile.o :    $(CLIENT_DIR)/cl_profile.c;	$(DO_CC)
$(BUILDDIR)/client/cl_fx.o :	     $(CLIENT_DIR)/cl_fx.c;		$(DO_CC)
$(BUILDDIR)/client/cl_input.o :      $(CLIENT_DIR)/cl_input.c;		$(DO_CC)
$(BUILDDIR)/client/cl_inv.o :	     $(CLIENT_DIR)/cl_inv.c;		$(DO_CC)
$(BUILDDIR)/client/cl_main.o :       $(CLIENT_DIR)/cl_main.c;		$(DO_CC)
$(BUILDDIR)/client/cl_parse.o :      $(CLIENT_DIR)/cl_parse.c;		$(DO_CC)
$(BUILDDIR)/client/cl_pred.o :       $(CLIENT_DIR)/cl_pred.c;		$(DO_CC)
$(BUILDDIR)/client/cl_tent.o :       $(CLIENT_DIR)/cl_tent.c;		$(DO_CC)
$(BUILDDIR)/client/cl_scrn.o :       $(CLIENT_DIR)/cl_scrn.c;		$(DO_CC)
$(BUILDDIR)/client/cl_view.o :       $(CLIENT_DIR)/cl_view.c;		$(DO_CC)
$(BUILDDIR)/client/cl_newfx.o :      $(CLIENT_DIR)/cl_newfx.c;		$(DO_CC)
$(BUILDDIR)/client/console.o :       $(CLIENT_DIR)/console.c;		$(DO_CC)
$(BUILDDIR)/client/keys.o :          $(CLIENT_DIR)/keys.c;		$(DO_CC)
$(BUILDDIR)/client/menu.o :          $(CLIENT_DIR)/menu.c;		$(DO_CC)
$(BUILDDIR)/client/cl_loc.o :        $(CLIENT_DIR)/cl_loc.c;		$(DO_CC)
$(BUILDDIR)/client/cl_scores.o :     $(CLIENT_DIR)/cl_scores.c;		$(DO_CC)
$(BUILDDIR)/client/cl_hud.o :        $(CLIENT_DIR)/cl_hud.c;		$(DO_CC)
$(BUILDDIR)/client/cl_decode.o :     $(CLIENT_DIR)/cl_decode.c;		$(DO_CC)
$(BUILDDIR)/client/cl_vote.o :       $(CLIENT_DIR)/cl_vote.c;		$(DO_CC)
$(BUILDDIR)/client/cl_serverlist.o : $(CLIENT_DIR)/cl_serverlist.c;	$(DO_CC)
$(BUILDDIR)/client/cl_images.o :     $(CLIENT_DIR)/cl_images.c;		$(DO_CC)
$(BUILDDIR)/client/cl_irc.o :        $(CLIENT_DIR)/cl_irc.c;		$(DO_CC)
$(BUILDDIR)/client/cl_stats.o :      $(CLIENT_DIR)/cl_stats.c;		$(DO_CC)
$(BUILDDIR)/client/snd_dma.o :       $(CLIENT_DIR)/snd_dma.c;		$(DO_CC)
$(BUILDDIR)/client/snd_mem.o :       $(CLIENT_DIR)/snd_mem.c;		$(DO_CC)
$(BUILDDIR)/client/snd_mix.o :       $(CLIENT_DIR)/snd_mix.c;		$(DO_CC)
$(BUILDDIR)/client/snd_ogg.o :       $(CLIENT_DIR)/snd_ogg.c;		$(DO_CC)
$(BUILDDIR)/client/cmd.o :           $(COMMON_DIR)/cmd.c;		$(DO_CC)
$(BUILDDIR)/client/cmodel.o :        $(COMMON_DIR)/cmodel.c;		$(DO_CC)
$(BUILDDIR)/client/common.o :        $(COMMON_DIR)/common.c;		$(DO_CC)
$(BUILDDIR)/client/crc.o :           $(COMMON_DIR)/crc.c;		$(DO_CC)
$(BUILDDIR)/client/cvar.o :          $(COMMON_DIR)/cvar.c;		$(DO_CC)
$(BUILDDIR)/client/files.o :         $(COMMON_DIR)/files.c;		$(DO_CC)
$(BUILDDIR)/client/md4.o :           $(COMMON_DIR)/md4.c;		$(DO_CC)
$(BUILDDIR)/client/md5.o :           $(COMMON_DIR)/md5.c;		$(DO_CC)
$(BUILDDIR)/client/net_chan.o :      $(COMMON_DIR)/net_chan.c;		$(DO_CC)
$(BUILDDIR)/client/net_common.o :    $(COMMON_DIR)/net_common.c;	$(DO_CC)
$(BUILDDIR)/client/pmove.o :         $(COMMON_DIR)/pmove.c;		$(DO_CC)
$(BUILDDIR)/client/m_flash.o :       $(GAME_DIR)/m_flash.c;		$(DO_CC)
$(BUILDDIR)/client/q_shared.o :      $(GAME_DIR)/q_shared.c;		$(DO_CC)
$(BUILDDIR)/client/sv_ccmds.o :      $(SERVER_DIR)/sv_ccmds.c;		$(DO_CC)
$(BUILDDIR)/client/sv_ents.o :       $(SERVER_DIR)/sv_ents.c;		$(DO_CC)
$(BUILDDIR)/client/sv_game.o :       $(SERVER_DIR)/sv_game.c;		$(DO_CC)
$(BUILDDIR)/client/sv_init.o :       $(SERVER_DIR)/sv_init.c;		$(DO_CC)
$(BUILDDIR)/client/sv_main.o :       $(SERVER_DIR)/sv_main.c;		$(DO_CC)
$(BUILDDIR)/client/sv_send.o :       $(SERVER_DIR)/sv_send.c;		$(DO_CC)
$(BUILDDIR)/client/sv_user.o :       $(SERVER_DIR)/sv_user.c;		$(DO_CC)
$(BUILDDIR)/client/sv_world.o :      $(SERVER_DIR)/sv_world.c;		$(DO_CC)
$(BUILDDIR)/client/q_shlinux.o :     $(LINUX_DIR)/q_shlinux.c;		$(DO_CC)
$(BUILDDIR)/client/vid_so.o :        $(LINUX_DIR)/vid_so.c;		$(DO_CC)
$(BUILDDIR)/client/vid_common.o :    $(CLIENT_DIR)/vid_common.c;	$(DO_CC)
$(BUILDDIR)/client/snd_mixa.o :      $(LINUX_DIR)/snd_mixa.s;		$(DO_AS)
$(BUILDDIR)/client/sys_linux.o :     $(LINUX_DIR)/sys_linux.c;		$(DO_CC)
$(BUILDDIR)/client/glob.o :          $(LINUX_DIR)/glob.c;		$(DO_CC)
$(BUILDDIR)/client/net_udp.o :       $(LINUX_DIR)/net_udp.c;		$(DO_CC)
$(BUILDDIR)/client/net_tcp.o :       $(LINUX_DIR)/net_tcp.c;		$(DO_CC)
$(BUILDDIR)/client/net_udp6.o :      $(LINUX_DIR)/net_udp6.c;		$(DO_CC)
$(BUILDDIR)/client/cd_linux.o :      $(LINUX_DIR)/cd_linux.c;		$(DO_CC)
$(BUILDDIR)/client/snd_alsa.o :      $(LINUX_DIR)/snd_alsa.c;		$(DO_SHLIB_CC)
$(BUILDDIR)/client/snd_oss.o :       $(LINUX_DIR)/snd_oss.c;		$(DO_SHLIB_CC)
$(BUILDDIR)/client/cd_sdl.o :        $(LINUX_DIR)/cd_sdl.c;		$(DO_CC) $(SDLCFLAGS)
$(BUILDDIR)/client/snd_sdl.o :       $(LINUX_DIR)/snd_sdl.c;		$(DO_SHLIB_CC) $(SDLCFLAGS)
	
#############################################################################
# DEDICATED SERVER
#############################################################################

PB2DED_OBJS = \
       $(BUILDDIR)/ded/cmd.o       $(BUILDDIR)/ded/cmodel.o     $(BUILDDIR)/ded/common.o \
       $(BUILDDIR)/ded/crc.o       $(BUILDDIR)/ded/cvar.o       $(BUILDDIR)/ded/files.o \
       $(BUILDDIR)/ded/md4.o       $(BUILDDIR)/ded/net_common.o $(BUILDDIR)/ded/net_chan.o \
       $(BUILDDIR)/ded/sv_ccmds.o  $(BUILDDIR)/ded/sv_ents.o    $(BUILDDIR)/ded/sv_game.o \
       $(BUILDDIR)/ded/sv_init.o   $(BUILDDIR)/ded/sv_main.o    $(BUILDDIR)/ded/sv_send.o \
       $(BUILDDIR)/ded/sv_user.o   $(BUILDDIR)/ded/sv_world.o   $(BUILDDIR)/ded/q_shlinux.o \
       $(BUILDDIR)/ded/sys_linux.o $(BUILDDIR)/ded/glob.o       $(BUILDDIR)/ded/net_udp.o \
       $(BUILDDIR)/ded/net_tcp.o   $(BUILDDIR)/ded/q_shared.o   $(BUILDDIR)/ded/pmove.o \
       $(BUILDDIR)/ded/cl_null.o   $(BUILDDIR)/ded/cd_null.o    $(BUILDDIR)/ded/md5.o

$(BINDIR)/pb2-ded : $(PB2DED_OBJS)
	@echo
	@echo "**Built Paintball2 dedicated client with cflags:"
	@echo "$(CFLAGS)";
	@echo
	@echo "**Now linking Paintball2 dedicated client with flags:"
	@echo "$(LDFLAGS)";
	@echo
	$(CC) $(CFLAGS) -o $@ $(PB2DED_OBJS) $(LDFLAGS)

$(BUILDDIR)/ded/cmd.o :         $(COMMON_DIR)/cmd.c;		$(DO_DED_CC)
$(BUILDDIR)/ded/cmodel.o :      $(COMMON_DIR)/cmodel.c;		$(DO_DED_CC)
$(BUILDDIR)/ded/common.o :      $(COMMON_DIR)/common.c;		$(DO_DED_CC)
$(BUILDDIR)/ded/crc.o :	        $(COMMON_DIR)/crc.c;		$(DO_DED_CC)
$(BUILDDIR)/ded/net_common.o :	$(COMMON_DIR)/net_common.c;	$(DO_DED_CC)
$(BUILDDIR)/ded/cvar.o :	$(COMMON_DIR)/cvar.c;		$(DO_DED_CC)
$(BUILDDIR)/ded/files.o :       $(COMMON_DIR)/files.c;		$(DO_DED_CC)
$(BUILDDIR)/ded/md4.o :	 	$(COMMON_DIR)/md4.c;		$(DO_DED_CC)
$(BUILDDIR)/ded/md5.o :	 	$(COMMON_DIR)/md5.c;		$(DO_DED_CC)
$(BUILDDIR)/ded/net_chan.o :    $(COMMON_DIR)/net_chan.c;	$(DO_DED_CC)
$(BUILDDIR)/ded/pmove.o :       $(COMMON_DIR)/pmove.c;		$(DO_DED_CC)
$(BUILDDIR)/ded/q_shared.o :    $(GAME_DIR)/q_shared.c;		$(DO_DED_CC)
$(BUILDDIR)/ded/sv_ccmds.o :    $(SERVER_DIR)/sv_ccmds.c;	$(DO_DED_CC)
$(BUILDDIR)/ded/sv_ents.o :     $(SERVER_DIR)/sv_ents.c;	$(DO_DED_CC)
$(BUILDDIR)/ded/sv_game.o :     $(SERVER_DIR)/sv_game.c;	$(DO_DED_CC)
$(BUILDDIR)/ded/sv_init.o :     $(SERVER_DIR)/sv_init.c;	$(DO_DED_CC)
$(BUILDDIR)/ded/sv_main.o :     $(SERVER_DIR)/sv_main.c;	$(DO_DED_CC)
$(BUILDDIR)/ded/sv_send.o :     $(SERVER_DIR)/sv_send.c;	$(DO_DED_CC)
$(BUILDDIR)/ded/sv_user.o :     $(SERVER_DIR)/sv_user.c;	$(DO_DED_CC)
$(BUILDDIR)/ded/sv_world.o :    $(SERVER_DIR)/sv_world.c;	$(DO_DED_CC)
$(BUILDDIR)/ded/cd_null.o :     $(SERVER_DIR)/null/cd_null.c;	$(DO_DED_CC)
$(BUILDDIR)/ded/cl_null.o :     $(SERVER_DIR)/null/cl_null.c;	$(DO_DED_CC)
$(BUILDDIR)/ded/q_shlinux.o :   $(LINUX_DIR)/q_shlinux.c;	$(DO_DED_CC)
$(BUILDDIR)/ded/sys_linux.o :   $(LINUX_DIR)/sys_linux.c;	$(DO_DED_CC)
$(BUILDDIR)/ded/glob.o :	$(LINUX_DIR)/glob.c;		$(DO_DED_CC)
$(BUILDDIR)/ded/net_udp.o :     $(LINUX_DIR)/net_udp.c;		$(DO_DED_CC)
$(BUILDDIR)/ded/net_tcp.o :     $(LINUX_DIR)/net_tcp.c;		$(DO_DED_CC)

#############################################################################
# REF_GL
#############################################################################

REF_GL_OBJS = \
	$(BUILDDIR)/ref_gl/gl_draw.o   $(BUILDDIR)/ref_gl/gl_script.o $(BUILDDIR)/ref_gl/gl_image.o \
	$(BUILDDIR)/ref_gl/gl_light.o  $(BUILDDIR)/ref_gl/gl_mesh.o   $(BUILDDIR)/ref_gl/r_skm.o \
	$(BUILDDIR)/ref_gl/gl_refl.o   $(BUILDDIR)/ref_gl/gl_model.o  $(BUILDDIR)/ref_gl/gl_rmain.o \
	$(BUILDDIR)/ref_gl/gl_rmisc.o  $(BUILDDIR)/ref_gl/gl_rsurf.o  $(BUILDDIR)/ref_gl/gl_warp.o \
	$(BUILDDIR)/ref_gl/qgl_linux.o $(BUILDDIR)/ref_gl/q_shared.o  $(BUILDDIR)/ref_gl/q_shlinux.o \
	$(BUILDDIR)/ref_gl/glob.o      $(BUILDDIR)/ref_gl/gl_cin.o    $(BUILDDIR)/ref_gl/gl_debug.o

REF_GLX_OBJS = \
	$(BUILDDIR)/ref_gl/gl_glx.o    $(BUILDDIR)/ref_gl/rw_linux.o  $(BUILDDIR)/ref_gl/joystick.o

REF_SDLGL_OBJS = \
	$(BUILDDIR)/ref_gl/gl_sdl.o    $(BUILDDIR)/ref_gl/rw_linux.o  $(BUILDDIR)/ref_gl/joystick.o

$(BINDIR)/ref_pbgl.$(SHLIBEXT) : $(REF_GL_OBJS) $(REF_GLX_OBJS)
	@echo
	@echo "**Built GLX video driver with cflags:"
	@echo "$(CFLAGS)";
	@echo
	@echo "**Now linking GLX video driver with flags:"
	@echo "$(XLDFLAGS) $(GLX_LDFLAGS) $(SHLIBLDFLAGS)";
	@echo
	$(CC) $(CFLAGS) $(SHLIBLDFLAGS) -o $@ $(REF_GL_OBJS) $(REF_GLX_OBJS) $(XLDFLAGS) $(GLX_LDFLAGS)

$(BINDIR)/ref_pbsdl.$(SHLIBEXT) : $(REF_GL_OBJS) $(REF_SDLGL_OBJS)
	@echo
	@echo "**Built SDL video driver with cflags:"
	@echo "$(CFLAGS)";
	@echo
	@echo "**Now Linking SDL video driver with flags:"
	@echo "$(SDLGLLDFLAGS) $(GLX_LDFLAGS) $(SHLIBLDFLAGS)";
	@echo
	$(CC) $(CFLAGS) $(SHLIBLDFLAGS) -o $@ $(REF_GL_OBJS) $(REF_SDLGL_OBJS) $(GLX_LDFLAGS) $(SDLGLLDFLAGS)

$(BUILDDIR)/ref_gl/gl_script.o :  $(REF_GL_DIR)/gl_script.c;	$(DO_SHLIB_CC)
$(BUILDDIR)/ref_gl/gl_draw.o :    $(REF_GL_DIR)/gl_draw.c;	$(DO_SHLIB_CC)
$(BUILDDIR)/ref_gl/gl_image.o :   $(REF_GL_DIR)/gl_image.c;	$(DO_SHLIB_CC)
$(BUILDDIR)/ref_gl/gl_cin.o :     $(REF_GL_DIR)/gl_cin.c;	$(DO_SHLIB_CC)
$(BUILDDIR)/ref_gl/gl_debug.o :   $(REF_GL_DIR)/gl_debug.c;	$(DO_SHLIB_CC)
$(BUILDDIR)/ref_gl/gl_light.o :   $(REF_GL_DIR)/gl_light.c;	$(DO_SHLIB_CC)
$(BUILDDIR)/ref_gl/gl_refl.o :    $(REF_GL_DIR)/gl_refl.c;	$(DO_SHLIB_CC)
$(BUILDDIR)/ref_gl/r_skm.o :      $(REF_GL_DIR)/r_skm.c;	$(DO_SHLIB_CC)
$(BUILDDIR)/ref_gl/gl_mesh.o :    $(REF_GL_DIR)/gl_mesh.c;	$(DO_SHLIB_CC)
$(BUILDDIR)/ref_gl/gl_model.o :   $(REF_GL_DIR)/gl_model.c;	$(DO_SHLIB_CC)
$(BUILDDIR)/ref_gl/gl_rmain.o :   $(REF_GL_DIR)/gl_rmain.c;	$(DO_SHLIB_CC)
$(BUILDDIR)/ref_gl/gl_rmisc.o :   $(REF_GL_DIR)/gl_rmisc.c;	$(DO_SHLIB_CC)
$(BUILDDIR)/ref_gl/gl_rsurf.o :   $(REF_GL_DIR)/gl_rsurf.c;	$(DO_SHLIB_CC)
$(BUILDDIR)/ref_gl/gl_warp.o :    $(REF_GL_DIR)/gl_warp.c;	$(DO_SHLIB_CC)
$(BUILDDIR)/ref_gl/q_shared.o :   $(GAME_DIR)/q_shared.c;	$(DO_SHLIB_CC)
$(BUILDDIR)/ref_gl/qgl_linux.o :  $(LINUX_DIR)/qgl_linux.c;	$(DO_SHLIB_CC)
$(BUILDDIR)/ref_gl/q_shlinux.o :  $(LINUX_DIR)/q_shlinux.c;	$(DO_SHLIB_CC)
$(BUILDDIR)/ref_gl/glob.o :       $(LINUX_DIR)/glob.c;		$(DO_SHLIB_CC)
$(BUILDDIR)/ref_gl/gl_glx.o :     $(LINUX_DIR)/gl_glx.c;	$(DO_SHLIB_CC) $(GLXCFLAGS)
$(BUILDDIR)/ref_gl/rw_linux.o :   $(LINUX_DIR)/rw_linux.c;	$(DO_SHLIB_CC) $(GLXCFLAGS)
$(BUILDDIR)/ref_gl/joystick.o :   $(LINUX_DIR)/joystick.c;	$(DO_SHLIB_CC) $(GLXCFLAGS)
$(BUILDDIR)/ref_gl/gl_sdl.o :     $(LINUX_DIR)/gl_sdl.c;	$(DO_SHLIB_CC) $(SDLGLCFLAGS)

#############################################################################
# BOTLIB
#############################################################################

BOTLIB_OBJS = \
	$(BUILDDIR)/bots/bot_aim.o       $(BUILDDIR)/bots/bot_astar.o      $(BUILDDIR)/bots/bot_commands.o \
	$(BUILDDIR)/bots/bot_debug.o     $(BUILDDIR)/bots/bot_event.o      $(BUILDDIR)/bots/bot_files.o \
	$(BUILDDIR)/bots/bot_goals.o     $(BUILDDIR)/bots/bot_main.o       $(BUILDDIR)/bots/bot_move.o \
	$(BUILDDIR)/bots/bot_observe.o   $(BUILDDIR)/bots/bot_shared.o     $(BUILDDIR)/bots/bot_waypoints.o

$(BINDIR)/dp_botlib.$(SHLIBEXT) : $(BOTLIB_OBJS)
	@echo
	@echo "**Built BOTLIB cflags:"
	@echo "$(CFLAGS)";
	@echo
	@echo "**Now linking BOTLIB with flags:"
	@echo "$(SHLIBLDFLAGS)";
	@echo
	$(CC) $(CFLAGS) $(SHLIBLDFLAGS) -o $@ $(BOTLIB_OBJS)

$(BUILDDIR)/bots/bot_aim.o :       $(BOTLIB_DIR)/bot_aim.c;		$(DO_SHLIB_CC)
$(BUILDDIR)/bots/bot_astar.o :     $(BOTLIB_DIR)/bot_astar.c;		$(DO_SHLIB_CC)
$(BUILDDIR)/bots/bot_commands.o :  $(BOTLIB_DIR)/bot_commands.c;	$(DO_SHLIB_CC)
$(BUILDDIR)/bots/bot_debug.o :     $(BOTLIB_DIR)/bot_debug.c;		$(DO_SHLIB_CC)
$(BUILDDIR)/bots/bot_event.o :     $(BOTLIB_DIR)/bot_event.c;		$(DO_SHLIB_CC)
$(BUILDDIR)/bots/bot_files.o :     $(BOTLIB_DIR)/bot_files.c;		$(DO_SHLIB_CC)
$(BUILDDIR)/bots/bot_goals.o :     $(BOTLIB_DIR)/bot_goals.c;		$(DO_SHLIB_CC)
$(BUILDDIR)/bots/bot_main.o :      $(BOTLIB_DIR)/bot_main.c;		$(DO_SHLIB_CC)
$(BUILDDIR)/bots/bot_move.o :      $(BOTLIB_DIR)/bot_move.c;		$(DO_SHLIB_CC)
$(BUILDDIR)/bots/bot_observe.o :   $(BOTLIB_DIR)/bot_observe.c;		$(DO_SHLIB_CC)
$(BUILDDIR)/bots/bot_shared.o :    $(BOTLIB_DIR)/bot_shared.c;		$(DO_SHLIB_CC)
$(BUILDDIR)/bots/bot_waypoints.o : $(BOTLIB_DIR)/bot_waypoints.c;	$(DO_SHLIB_CC)

#############################################################################
# MISC
#############################################################################

clean:
	@echo Cleaning objects...
	@rm -rf \
	$(BUILD_DEBUG_DIR) \
	$(BUILD_RELEASE_DIR)
	@echo ...................
	@echo .... Done
	
clean_bin:
	@echo Cleaning binaries...
	@-rm -rf $(BINDIR)
	@echo ....................
	@echo .... Done

distclean:
	@echo Cleaning objects and binaries...
	@-rm -rf $(BUILD_DEBUG_DIR) $(BUILD_RELEASE_DIR) $(BINDIR)
	@-rm -f `find . \( -not -type d \) -and \
		\( -name '*~' \) -type f -print`
	@echo ................................
	@echo .... Done

install:
	@echo Copying files to your home dir
	@echo ......
	@cp -rv $(BINDIR) ~/
	@cp -rv linux/paintball2.run ~/paintball2
	@chmod +x ~/paintball2/paintball2.run
ifeq ($(strip $(BUILD_DEDICATED)),YES)
	@cp -rv linux/paintball2-dedicated.run ~/paintball2
	@chmod +x ~/paintball2/paintball2-dedicated.run
endif
	@echo .....
	@echo Done 

install_root:
	@mkdir -pv $(DATADIR)
	@echo Copying files to $(DATADIR)
ifeq ($(OSTYPE),FreeBSD)
	@cp -rv $(BINDIR) $(LOCALBASE)/share
	@rm -fv $(DATADIR)/ref_* $(DATADIR)/snd_*
	@mkdir -pv $(LIBDIR)
	@cp -rv $(BINDIR)/snd_* $(BINDIR)/ref_* $(LIBDIR)
else
	@cp -rv $(BINDIR) $(LOCALBASE)/games
endif
	@cp -rv linux/paintball2_datadir.run linux/paintball2.png $(DATADIR)
	@chmod +x $(DATADIR)/paintball2_datadir.run
	@cp -rv linux/paintball2.desktop /home/*/Desktop/Paintball2
	@ln -sfv $(DATADIR)/paintball2_datadir.run $(SYSBINDIR)/paintball2
	@printf "Symlinking executable to $(SYSBINDIR)\n"
ifeq ($(strip $(BUILD_DEDICATED)),YES)
	@cp -rv linux/paintball2-dedicated_datadir.run $(DATADIR)
	@chmod +x $(DATADIR)/paintball2-dedicated_datadir.run
	@ln -sfv $(DATADIR)/paintball2-dedicated_datadir.run $(SYSBINDIR)/paintball2-dedicated
	@printf "Symlinking dedicated executable to $(SYSBINDIR)\n"
endif
	@echo 
	@printf "Type paintball2 as user to start.\n"
ifeq ($(strip $(BUILD_DEDICATED)),YES)
	@printf "Type paintball2-dedicated as user to start a dedicated server.\n"
endif
	@echo 
	@echo Done 
	
	
bz2:
	@printf "Creating bzip2 compressed file ...\n"
	@cp -r linux/paintball2.run linux/paintball2-dedicated.run paintball2/
ifeq ($(strip $(USE_SSE)),YES)
	@tar cjvf Paintball2-$(VERSION)-build$(BUILD)-SSE-$(OSTYPE).tar.bz2 paintball2/ 
else
	@tar cjvf Paintball2-$(VERSION)-build$(BUILD)-NOSSE-$(OSTYPE).tar.bz2 paintball2/ 
endif
	@printf ".... Done.\n"
	
