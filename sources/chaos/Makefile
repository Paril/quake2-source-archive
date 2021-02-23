# 
# Makefile				Chaotic Dreams Makefile
#
# Major'Trips'
# Sat May 22 00:08:36 CDT 1999
#
#	This file was originally part of the Quake2 3.20 source tree.
#	I simply barrowed it for inclusion here.

BUILD_DEBUG_DIR=debug
BUILD_RELEASE_DIR=release

CC=gcc

# Base Flags
# If you want to add a CFLAG to be included in both the 
# debug and the release build, add it here
BASE_CFLAGS=-Dstricmp=strcasecmp -DC_ONLY -pedantic -m386 -V2.7.2.3

# Release Flags
# Flags added here are ONLY used for building the release
# binaries
RELEASE_CFLAGS=$(BASE_CFLAGS) -ffast-math -funroll-loops \
	-fomit-frame-pointer -fexpensive-optimizations

# Debug Flags
# These flags are only used when compiling the debug binaries
DEBUG_CFLAGS=$(BASE_CFLAGS) -g -Wall

# LD Flags
# These are typically not used, though I understand it's 
# necessary to use them on a Slakware system
#LDFLAGS=-ldl -lm 

##### Dont go beyond here unless you know what's going on ####

ARCH=i386

SHLIBEXT=so

SHLIBCFLAGS=-fPIC
SHLIBLDFLAGS=-shared

DO_CC=$(CC) $(CFLAGS) -o $@ -c $<
DO_SHLIB_CC=$(CC) $(CFLAGS) $(SHLIBCFLAGS) -o $@ -c $<

TARGETS=$(BUILDDIR)/game$(ARCH).$(SHLIBEXT) \

build_debug:
	@-if [ ! -d "$(BUILD_DEBUG_DIR)" ]; then \
	  mkdir $(BUILD_DEBUG_DIR);fi
	$(MAKE) targets BUILDDIR=$(BUILD_DEBUG_DIR) CFLAGS="$(DEBUG_CFLAGS)"

build_release:
	@-if [ ! -d "$(BUILD_RELEASE_DIR)" ]; then \
	  mkdir $(BUILD_RELEASE_DIR);fi
	$(MAKE) targets BUILDDIR=$(BUILD_RELEASE_DIR) CFLAGS="$(RELEASE_CFLAGS)"

all: build_debug build_release

targets: $(TARGETS)

GAME_OBJS = \
	$(BUILDDIR)/c_base.o \
	$(BUILDDIR)/c_botai.o \
	$(BUILDDIR)/c_botmisc.o \
	$(BUILDDIR)/c_botnav.o \
	$(BUILDDIR)/c_cam.o \
	$(BUILDDIR)/c_item.o \
	$(BUILDDIR)/c_weapon.o \
	$(BUILDDIR)/g_cmds.o \
	$(BUILDDIR)/g_combat.o \
	$(BUILDDIR)/g_ctf.o \
	$(BUILDDIR)/g_func.o \
	$(BUILDDIR)/g_items.o \
	$(BUILDDIR)/g_main.o \
	$(BUILDDIR)/g_misc.o \
	$(BUILDDIR)/g_phys.o \
	$(BUILDDIR)/g_save.o \
	$(BUILDDIR)/g_spawn.o \
	$(BUILDDIR)/g_svcmds.o \
	$(BUILDDIR)/g_target.o \
	$(BUILDDIR)/g_trigger.o \
	$(BUILDDIR)/g_utils.o \
	$(BUILDDIR)/g_weapon.o \
	$(BUILDDIR)/gslog.o \
	$(BUILDDIR)/m_move.o \
	$(BUILDDIR)/p_client.o \
	$(BUILDDIR)/p_hud.o \
	$(BUILDDIR)/p_menu.o \
	$(BUILDDIR)/p_view.o \
	$(BUILDDIR)/p_weapon.o \
	$(BUILDDIR)/q_shared.o \
	$(BUILDDIR)/stdlog.o

$(BUILDDIR)/game$(ARCH).$(SHLIBEXT) : $(GAME_OBJS)
	$(CC) $(SHLIBLDFLAGS) $(CFLAGS) -o $@ $(GAME_OBJS)


$(BUILDDIR)/c_base.o : c_base.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/c_botai.o : c_botai.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/c_botmisc.o : c_botmisc.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/c_botnav.o : c_botnav.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/c_cam.o : c_cam.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/c_item.o : c_item.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/c_weapon.o : c_weapon.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/g_cmds.o : g_cmds.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/g_combat.o : g_combat.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/g_ctf.o : g_ctf.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/g_func.o : g_func.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/g_items.o : g_items.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/g_main.o : g_main.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/g_misc.o : g_misc.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/g_phys.o : g_phys.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/g_save.o : g_save.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/g_spawn.o : g_spawn.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/g_svcmds.o : g_svcmds.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/g_target.o : g_target.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/g_trigger.o : g_trigger.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/g_utils.o : g_utils.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/g_weapon.o : g_weapon.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/gslog.o : gslog.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/m_move.o : m_move.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/p_client.o : p_client.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/p_hud.o : p_hud.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/p_menu.o : p_menu.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/p_view.o : p_view.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/p_weapon.o : p_weapon.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/q_shared.o : q_shared.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/stdlog.o : stdlog.c
	$(DO_SHLIB_CC)

#####

clean: clean-debug clean-release

clean-debug:
	$(MAKE) clean2 BUILDDIR=$(BUILD_DEBUG_DIR) CFLAGS="$(DEBUG_CFLAGS)"

clean-release:
	$(MAKE) clean2 BUILDDIR=$(BUILD_RELEASE_DIR) CFLAGS="$(DEBUG_CFLAGS)"

clean2:
	-rm -f $(GAME_OBJS)
