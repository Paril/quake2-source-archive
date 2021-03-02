#
# Quake2 gamei386.so Makefile for Linux 2.0
#
# Jan '98 by Zoid <zoid@idsoftware.com>
#
# Modified for EraserULTRA by Anthony Jacques <AnthonyJ@btinternet.com>
#
# ELF only
#
# Probably requires GNU make
#

ARCH=i386
CC=gcc
BASE_CFLAGS=-Dstricmp=strcasecmp 

#use these cflags to optimize it
CFLAGS=$(BASE_CFLAGS) -m486 -O6 -fomit-frame-pointer \
	-fexpensive-optimizations -malign-loops=2 \
	-malign-jumps=2 -malign-functions=2
#use these when debugging 
#CFLAGS=$(BASE_CFLAGS) -g

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
	bot_ai.o bot_die.o bot_items.o bot_misc.o bot_nav.o bot_spawn.o \
	bot_wpns.o camclient.o \
	g_ai.o g_chase.o g_cmds.o g_combat.o g_ctf.o g_func.o g_items.o \
	g_main.o g_map_mod.o g_misc.o \
	g_monster.o g_phys.o g_save.o g_spawn.o g_svcmds.o g_target.o \
	g_trigger.o g_utils.o g_weapon.o \
	m_move.o  \
	p_client.o p_hud.o p_menu.o \
	p_view.o p_weapon.o q_shared.o \
	g_newfnc.o g_newtrig.o g_newtarg.o \
	aj_banning.o aj_confcycle.o aj_cvar.o aj_lmctf.o aj_main.o aj_menu.o \
	aj_replacelist.o aj_runes.o aj_scoreboard.o aj_statusbars.o \
	e_grapple.o e_hook.o funhook.o

NAV_LIB = NavLib/linux/p_trail.o

game$(ARCH).$(SHLIBEXT) : $(GAME_OBJS)
	$(CC) $(CFLAGS) $(SHLIBLDFLAGS) -o $@ $(GAME_OBJS) $(NAV_LIB)


#############################################################################
# MISC
#############################################################################

clean:
	-rm -f $(GAME_OBJS)

depend:
	gcc -MM $(GAME_OBJS:.o=.c)

