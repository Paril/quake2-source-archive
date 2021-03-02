#         D-Day: Normandy by Vipersoft
# ************************************
# *   $Source: /usr/local/cvsroot/dday/src/Makefile,v $
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
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#

# Check OS type.
OSTYPE := $(shell uname -s)

# this nice line comes from the linux kernel makefile
ARCH := $(shell uname -m | sed -e s/i.86/i386/ -e s/sun4u/sparc/ -e s/sparc64/sparc/ -e s/arm.*/arm/ -e s/sa110/arm/ -e s/alpha/axp/)

CC=gcc
BASE_CFLAGS=-Dstricmp=strcasecmp

CFLAGS_RELEASE=$(BASE_CFLAGS) -O2 -ffast-math -funroll-loops \
	-fexpensive-optimizations -fomit-frame-pointer

ifeq ($(ARCH),i386)
  RELEASE_CFLAGS +=-falign-loops=2 -falign-jumps=2 -falign-functions=2 -fno-strict-aliasing
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
	$(MAKE) game$(ARCH).$(SHLIBEXT) CFLAGS="$(CFLAGS_DEBUG)"
	
build_release:
	$(MAKE) game$(ARCH).$(SHLIBEXT) CFLAGS="$(CFLAGS_RELEASE)"

#############################################################################
# SETUP AND BUILD
# GAME
#############################################################################

.c.o:
	$(DO_CC)

GAME_OBJS =	\
	g_ai.o \
	g_arty.o \
	g_cmds.o \
	g_combat.o \
	g_dll.linux.o \
	g_ents.o \
	g_func.o \
	g_items.o \
	g_main.o \
	g_maps.o \
	g_misc.o \
	g_monster.o \
	g_objectives.o \
	g_phys.o \
	g_save.o \
	g_spawn.o \
	g_svcmds.o \
	g_target.o \
	g_trigger.o \
	g_turret.o \
	g_utils.o \
	g_weapon.o \
	m_actor.o \
	m_flash.o \
	m_insane.o \
	m_move.o \
	p_classes.o \
	p_client.o \
	p_generic_wep.o \
	p_hud.o \
	p_menu.o \
	p_observer.o \
	p_trail.o \
	p_view.o \
	p_weapon.o \
	q_shared.o \
	u_entmgr.o \
	u_findfunc.o \
	x_fbomb.o \
	x_fire.o \
	z_removed.o

#	g_so.o # this file is a shared library thingie needed for Solaris

game$(ARCH).$(SHLIBEXT) : $(GAME_OBJS)
	$(MAKE) usa$(ARCH).$(SHLIBEXT) -C usa
	$(MAKE) grm$(ARCH).$(SHLIBEXT) -C grm
	$(MAKE) usm$(ARCH).$(SHLIBEXT) -C usm
	$(MAKE) rus$(ARCH).$(SHLIBEXT) -C rus/rus
	$(MAKE) gbr$(ARCH).$(SHLIBEXT) -C gbr/gbr
	$(MAKE) jpn$(ARCH).$(SHLIBEXT) -C jpn/jpn
	$(CC) $(CFLAGS) $(SHLIBLDFLAGS) -o $@ $(GAME_OBJS) $(LDFLAGS)



#############################################################################
# MISC
#############################################################################

clean:
	-rm -f $(GAME_OBJS)
	$(MAKE) clean -C usa
	$(MAKE) clean -C grm
	$(MAKE) clean -C usm
	$(MAKE) clean -C rus/rus
	$(MAKE) clean -C gbr/gbr
	$(MAKE) clean -C jpn/jpn
depend:
	gcc -MM $(GAME_OBJS:.o=.c)
g_ai.o: g_ai.c g_local.h g_defines.h q_shared.h game.h p_menu.h \
 g_maps.h
g_arty.o: g_arty.c g_local.h g_defines.h q_shared.h game.h p_menu.h \
 g_maps.h
g_cmds.o: g_cmds.c g_local.h g_defines.h q_shared.h game.h p_menu.h \
 g_maps.h m_player.h g_cmds.h
g_combat.o: g_combat.c g_local.h g_defines.h q_shared.h game.h \
 p_menu.h g_maps.h
g_dll.linux.o: g_dll.linux.c g_local.h g_defines.h q_shared.h game.h p_menu.h \
 g_maps.h g_dll.h u_findfunc.h u_entmgr.h
g_ents.o: g_ents.c g_local.h g_defines.h q_shared.h game.h p_menu.h \
 g_maps.h
g_feeder.o: g_feeder.c g_local.h g_defines.h q_shared.h game.h \
 p_menu.h g_maps.h
g_func.o: g_func.c g_local.h g_defines.h q_shared.h game.h p_menu.h \
 g_maps.h
g_items.o: g_items.c g_local.h g_defines.h q_shared.h game.h p_menu.h \
 g_maps.h
g_main.o: g_main.c g_local.h g_defines.h q_shared.h game.h p_menu.h \
 g_maps.h g_dll.h
g_maps.o: g_maps.c g_local.h g_defines.h q_shared.h game.h p_menu.h \
 g_maps.h
g_misc.o: g_misc.c g_local.h g_defines.h q_shared.h game.h p_menu.h \
 g_maps.h x_fire.h
g_monster.o: g_monster.c g_local.h g_defines.h q_shared.h game.h \
 p_menu.h g_maps.h x_fire.h
g_objectives.o: g_objectives.c g_local.h g_defines.h q_shared.h game.h \
 p_menu.h g_maps.h
g_phys.o: g_phys.c g_local.h g_defines.h q_shared.h game.h p_menu.h \
 g_maps.h
g_save.o: g_save.c g_local.h g_defines.h q_shared.h game.h p_menu.h \
 g_maps.h
g_spawn.o: g_spawn.c g_local.h g_defines.h q_shared.h game.h p_menu.h \
 g_maps.h
g_svcmds.o: g_svcmds.c g_local.h g_defines.h q_shared.h game.h \
 p_menu.h g_maps.h
g_target.o: g_target.c g_local.h g_defines.h q_shared.h game.h \
 p_menu.h g_maps.h
g_trigger.o: g_trigger.c g_local.h g_defines.h q_shared.h game.h \
 p_menu.h g_maps.h
g_turret.o: g_turret.c g_local.h g_defines.h q_shared.h game.h \
 p_menu.h g_maps.h x_fire.h
g_utils.o: g_utils.c g_local.h g_defines.h q_shared.h game.h p_menu.h \
 g_maps.h
g_weapon.o: g_weapon.c g_local.h g_defines.h q_shared.h game.h \
 p_menu.h g_maps.h
m_actor.o: m_actor.c g_local.h g_defines.h q_shared.h game.h p_menu.h \
 g_maps.h m_actor.h
m_flash.o: m_flash.c q_shared.h
m_insane.o: m_insane.c g_local.h g_defines.h q_shared.h game.h \
 p_menu.h g_maps.h m_insane.h
m_move.o: m_move.c g_local.h g_defines.h q_shared.h game.h p_menu.h \
 g_maps.h
p_classes.o: p_classes.c g_local.h g_defines.h q_shared.h game.h \
 p_menu.h g_maps.h p_classes.h
p_client.o: p_client.c g_local.h g_defines.h q_shared.h game.h \
 p_menu.h g_maps.h m_player.h g_cmds.h x_fire.h
p_generic_wep.o: p_generic_wep.c g_local.h g_defines.h q_shared.h \
 game.h p_menu.h g_maps.h m_player.h
p_hud.o: p_hud.c g_local.h g_defines.h q_shared.h game.h p_menu.h \
 g_maps.h
p_menu.o: p_menu.c g_local.h g_defines.h q_shared.h game.h p_menu.h \
 g_maps.h
p_observer.o: p_observer.c g_local.h g_defines.h q_shared.h game.h \
 p_menu.h g_maps.h
p_trail.o: p_trail.c g_local.h g_defines.h q_shared.h game.h p_menu.h \
 g_maps.h
p_view.o: p_view.c g_local.h g_defines.h q_shared.h game.h p_menu.h \
 g_maps.h m_player.h
p_weapon.o: p_weapon.c g_local.h g_defines.h q_shared.h game.h \
 p_menu.h g_maps.h m_player.h x_fbomb.h x_fire.h
q_shared.o: q_shared.c q_shared.h
removed.o: removed.c g_local.h g_defines.h q_shared.h game.h p_menu.h \
 g_maps.h
u_entmgr.o: u_entmgr.c g_local.h g_defines.h q_shared.h game.h \
 p_menu.h g_maps.h u_entmgr.h
u_findfunc.o: u_findfunc.c g_local.h g_defines.h q_shared.h game.h \
 p_menu.h g_maps.h u_findfunc.h allfuncs.h funcarry.h
x_fbomb.o: x_fbomb.c g_local.h g_defines.h q_shared.h game.h p_menu.h \
 g_maps.h x_fbomb.h x_fire.h
x_fire.o: x_fire.c g_local.h g_defines.h q_shared.h game.h p_menu.h \
 g_maps.h x_fire.h
z_removed.o: z_removed.c g_local.h g_defines.h q_shared.h game.h \
 p_menu.h g_maps.h
