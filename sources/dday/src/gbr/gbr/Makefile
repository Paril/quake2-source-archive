#         D-Day: Normandy by Vipersoft
# ************************************
# *   $Source: /usr/local/cvsroot/dday/src/gbr/Makefile,v $
# *   $Revision: 1.6 $
# *   $Date: 2002/07/25 09:07:45 $
# *
# ***********************************
#
# Copyright (C) 2002 Vipersoft
# 
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
# 
# See the GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, gbr.
#

# Check OS type.
OSTYPE := $(shell uname -s)

# this nice line comes from the linux kernel makefile
ARCH := $(shell uname -m | sed -e s/i.86/i386/ -e s/sun4u/sparc/ -e s/sparc64/sparc/ -e s/arm.*/arm/ -e s/sa110/arm/ -e s/alpha/axp/)

CC=gcc
BASE_CFLAGS=-Dstricmp=strcasecmp -I..
CFLAGS_RELEASE=$(BASE_CFLAGS) -O2 -ffast-math -funroll-loops \
	-fexpensive-optimizations -fomit-frame-pointer

ifeq ($(ARCH),i386)
  RELEASE_CFLAGS + =-falign-loops=2 -falign-jumps=2 -falign-functions=2 -fno-strict-aliasing
endif

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
 
build_debug:
	$(MAKE) gbr$(ARCH).$(SHLIBEXT) CFLAGS="$(CFLAGS_DEBUG)"

build_release:
	$(MAKE) gbr$(ARCH).$(SHLIBEXT) CFLAGS="$(CFLAGS_RELEASE)"

#############################################################################
# SETUP AND BUILD
# GAME
#############################################################################

.c.o:
	$(DO_CC)

GAME_OBJS = gbr_item.o gbr_spawn.o gbr_guninfo.o gbr_weapon.o gbr_main.o
#	g_so.o # this file is a shared library thingie needed for Solaris

gbr$(ARCH).$(SHLIBEXT) : $(GAME_OBJS)
	$(CC) $(CFLAGS) $(SHLIBLDFLAGS) -o $@ $(GAME_OBJS) $(LDFLAGS)


#############################################################################
# MISC
#############################################################################

clean:
	-rm -f $(GAME_OBJS)

depend:
	gcc -MM $(GAME_OBJS:.o=.c)
gbr_item.o: gbr_item.c gbr_main.h gbr_defines.h ../g_local.h ../g_defines.h \
 ../q_shared.h ../game.h ../p_menu.h ../g_cmds.h ../g_dll.h
gbr_spawn.o: gbr_spawn.c gbr_main.h gbr_defines.h ../g_local.h ../g_defines.h \
 ../q_shared.h ../game.h ../p_menu.h ../g_cmds.h ../g_dll.h
gbr_guninfo.o: gbr_guninfo.c gbr_main.h gbr_defines.h ../g_local.h ../g_defines.h \
 ../q_shared.h ../game.h ../p_menu.h ../g_cmds.h ../g_dll.h
gbr_weapon.o: gbr_weapon.c gbr_main.h gbr_defines.h ../g_local.h ../g_defines.h \
 ../q_shared.h ../game.h ../p_menu.h ../g_cmds.h ../g_dll.h
gbr_main.o: gbr_main.c gbr_main.h gbr_defines.h ../g_local.h ../g_defines.h ../q_shared.h \
 ../game.h ../p_menu.h ../g_cmds.h ../g_dll.h 