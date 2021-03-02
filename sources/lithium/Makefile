#
# Quake2 gamei386.so Makefile for Linux 2.0
#
# Jan '98 by Zoid <zoid@idsoftware.com>
#
# ELF only
#
# Probably requires GNU make
#
# This builds the gamei386.so for Linux based on the q2source_12_11.zip
# release.  
# Put his Makefile in the game subdirectory you get when you unzip
# q2source_12_11.zip.
#
# There are two compiler errors you'll get, the following fixes
# are necessary:
#
# In g_local.h (around line 828), you must change the 
#    typedef struct g_client_s { ... } gclient_t;
# to just:
#    struct g_client_s { ... };
# The typedef is already defined elsewhere (seems to compile fine under
# MSCV++ for Win32 for some reason).
#
# m_player.h has a Ctrl-Z at the end (damn DOS editors).  Remove it or
# gcc complains.
#
# Note that the source in q2source_12_11.zip is for version 3.05.  To
# get it to run with Linux 3.10, change the following in game.h:
#    #define    GAME_API_VERSION        1
# change it to:
#    #define    GAME_API_VERSION        2

ARCH=i386
CC=gcc

#use these cflags to optimize it
CFLAGS=-O3
#use these when debugging 
#CFLAGS=-g

ifeq ($(shell uname),Linux)
CFLAGS+=-DNEED_STRLCAT -DNEED_STRLCPY
endif

# This enables warnings if a strlcat or strlcpy would have
# caused an overflow.
# CFLAGS+=-DSTRL_DEBUG

# This causes a backtrace to be emitted for detected buffer overflows
# Works only on linux
# CFLAGS+=-DSTRL_DEBUG_BACKTRACE -rdynamic

LDFLAGS=-ldl -lm
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

GAME_OBJS = \
	p_client.o g_cmds.o g_combat.o g_func.o g_items.o \
	g_main.o g_misc.o g_phys.o g_save.o g_spawn.o \
	g_target.o g_trigger.o g_turret.o g_utils.o g_weapon.o m_move.o \
	p_hud.o p_trail.o p_view.o p_weapon.o q_shared.o g_svcmds.o g_chase.o \
	lithium.o l_display.o l_fragtrak.o l_gslog.o l_hook.o \
	l_mapqueue.o l_nocamp.o l_obit.o l_pack.o l_rune.o \
	l_var.o l_menu.o l_admin.o l_vote.o l_net.o net.o \
	g_ctf.o l_hscore.o zbotcheck.o strl.o

lithium/game$(ARCH).$(SHLIBEXT): $(GAME_OBJS) 
	$(CC) $(CFLAGS) $(SHLIBLDFLAGS) -o $@ $(GAME_OBJS)


#############################################################################
# MISC
#############################################################################

clean:
	rm -f $(GAME_OBJS)

depend:
	gcc -MM $(GAME_OBJS:.o=.c)

#############################################################################
# DEPENDANCIES
#############################################################################

g_ai.o: g_ai.c g_local.h q_shared.h game.h
p_client.o: p_client.c g_local.h q_shared.h game.h m_player.h
g_cmds.o: g_cmds.c g_local.h q_shared.h game.h m_player.h
g_combat.o: g_combat.c g_local.h q_shared.h game.h
g_func.o: g_func.c g_local.h q_shared.h game.h
g_items.o: g_items.c g_local.h q_shared.h game.h
g_main.o: g_main.c g_local.h q_shared.h game.h
g_misc.o: g_misc.c g_local.h q_shared.h game.h
g_monster.o: g_monster.c g_local.h q_shared.h game.h
g_phys.o: g_phys.c g_local.h q_shared.h game.h
g_save.o: g_save.c g_local.h q_shared.h game.h
g_spawn.o: g_spawn.c g_local.h q_shared.h game.h
g_target.o: g_target.c g_local.h q_shared.h game.h
g_trigger.o: g_trigger.c g_local.h q_shared.h game.h
g_turret.o: g_turret.c g_local.h q_shared.h game.h
g_utils.o: g_utils.c g_local.h q_shared.h game.h
g_weapon.o: g_weapon.c g_local.h q_shared.h game.h
m_move.o: m_move.c g_local.h q_shared.h game.h
p_hud.o: p_hud.c g_local.h q_shared.h game.h
p_trail.o: p_trail.c g_local.h q_shared.h game.h
p_view.o: p_view.c g_local.h q_shared.h game.h m_player.h
p_weapon.o: p_weapon.c g_local.h q_shared.h game.h m_player.h
q_shared.o: q_shared.c q_shared.h
g_svcmds.o: g_svcmds.c g_svcmds.c q_shared.h
