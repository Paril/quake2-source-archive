#
# Quake2 pball beta4 gamesparc.so Makefile for Solaris 2.5
#
# Mar '98 by Zoid <zoid@idsoftware.com>
#
# Probably requires GNU make
#

# Check OS type.
OSTYPE := $(shell uname -s)

# this nice line comes from the linux kernel makefile
ARCH := $(shell uname -m | sed -e s/i.86/i386/ -e s/sun4u/sparc/ -e s/sparc64/sparc/ -e s/arm.*/arm/ -e s/sa110/arm/ -e s/alpha/axp/)


CC=gcc

BASE_CFLAGS=-Dstricmp=strcasecmp -I..

# Uncomment following for release build
CFLAGS_RELEASE=$(BASE_CFLAGS) -O2 -ffast-math -funroll-loops \
	-fexpensive-optimizations -fomit-frame-pointer

ifeq ($(ARCH),i386)
  RELEASE_CFLAGS + =-falign-loops=2 -falign-jumps=2 -falign-functions=2 -fno-strict-aliasing
endif

# Uncomment following for debug build
CFLAGS_DEBUG=$(BASE_CFLAGS) -g -ggdb -Wall

# Always uncommented
ifeq ($(OSTYPE),FreeBSD)
  LDFLAGS=-lm
endif
ifeq ($(OSTYPE),Linux)
  LDFLAGS=-lm -ldl
endif

SHLIBEXT=so
SHLIBCFLAGS=-fPIC
SHLIBLDFLAGS=-shared

DO_CC=$(CC) $(CFLAGS) $(SHLIBCFLAGS) -o $@ -c $<

#############################################################################
# SETUP AND BUILD
# GAME
#############################################################################
.c.o:
	$(DO_CC)

GAME_OBJS = rus_item.o rus_spawn.o rus_guninfo.o rus_weapon.o rus_main.o
#	g_so.o # this file is a shared library thingie needed for Solaris

rus$(ARCH).$(SHLIBEXT) : $(GAME_OBJS)
	$(CC) $(CFLAGS) $(SHLIBLDFLAGS) -o $@ $(GAME_OBJS) $(LDFLAGS)

#############################################################################
# MISC
#############################################################################
clean:
	-rm -f $(GAME_OBJS)
depend:
	gcc -MM $(GAME_OBJS:.o=.c)
rus_item.o: rus_item.c rus_main.h rus_defines.h ../g_local.h ../g_defines.h \
../q_shared.h ../game.h ../p_menu.h ../g_cmds.h ../g_dll.h
rus_spawn.o: rus_spawn.c rus_main.h rus_defines.h ../g_local.h ../g_defines.h \
 ../q_shared.h ../game.h ../p_menu.h ../g_cmds.h ../g_dll.h
rus_guninfo.o: rus_guninfo.c rus_main.h rus_defines.h ../g_local.h ../g_defines.h \
../q_shared.h ../game.h ../p_menu.h ../g_cmds.h ../g_dll.h
rus_weapon.o: rus_weapon.c rus_main.h rus_defines.h ../g_local.h ../g_defines.h \
../q_shared.h ../game.h ../p_menu.h ../g_cmds.h ../g_dll.h
rus_main.o: rus_main.c rus_main.h rus_defines.h ../g_local.h ../g_defines.h ../q_shared.h \
../game.h ../p_menu.h ../g_cmds.h ../g_dll.h 

