# makefile for giex

ARCH=i386
CC=gcc
#BASE_CFLAGS=-Dstricmp=strcasecmp -DQ_strncasecmp=strncasecmp
CFLAGS=$(BASE_CFLAGS) -march=i386 -O2 -ffast-math -Wall -g
SHLIBEXT=so
SHLIBLDFLAGS=-lm -ldl -shared #`gcc -print-libgcc-file-name` #-ldl

GAME_OBJS = \
g_ai.o \
g_chase.o \
g_cmds.o \
g_combat.o \
g_domination.o \
g_fileio.o \
g_func.o \
g_giexitems.o \
g_items.o \
g_magic.o \
g_main.o \
g_maprotation.o \
g_misc.o \
g_monster.o \
g_phys.o \
g_powerups.o \
g_save.o \
g_spawn.o \
g_svcmds.o \
g_target.o \
g_trigger.o \
g_turret.o \
g_utils.o \
g_weapon.o \
m_actor.o \
m_berserk.o \
m_boss2.o \
m_boss3.o \
m_boss31.o \
m_boss32.o \
m_brain.o \
m_chick.o \
m_flash.o \
m_flipper.o \
m_float.o \
m_flyer.o \
m_gladiator.o \
m_gunner.o \
m_hover.o \
m_infantry.o \
m_insane.o \
m_medic.o \
m_move.o \
m_mutant.o \
m_parasite.o \
m_soldier.o \
m_supertank.o \
m_tank.o \
p_client.o \
p_hud.o \
p_trail.o \
p_view.o \
p_weapon.o \
q_shared.o

all: game

game : $(GAME_OBJS)
	$(CC) $(CFLAGS) -o game$(ARCH).$(SHLIBEXT) $(SHLIBCFLAGS) $(GAME_OBJS) $(SHLIBLDFLAGS)


dist : game
	bzip2 -9 -k game$(ARCH).$(SHLIBEXT)

#############################################################################
# MISC
#############################################################################

clean:
	-rm -f $(GAME_OBJS) game$(ARCH).$(SHLIBEXT)
install:
	cp game$(ARCH).$(SHLIBEXT) ..
insttest:
	cp game$(ARCH).$(SHLIBEXT) ../../giextest/
