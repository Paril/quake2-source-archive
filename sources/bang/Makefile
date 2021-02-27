#
# BANG makefile 
# Intended for gcc/Linux, may need modifying for other platforms
#

ARCH=i386
CC=gcc
BASE_CFLAGS=-Dstricmp=strcasecmp

#use these cflags to optimize it
#CFLAGS=$(BASE_CFLAGS) -m486 -O6 -ffast-math -funroll-loops \
#	-fomit-frame-pointer -fexpensive-optimizations -malign-loops=2 \
#	-malign-jumps=2 -malign-functions=2
#use these when debugging 
CFLAGS=$(BASE_CFLAGS) -g

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
g_ai.o \
g_chase.o \
g_cmds.o \
g_combat.o \
g_func.o \
g_items.o \
g_main.o \
g_misc.o \
g_monster.o \
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
q_shared.o \
q_devels.o \

game$(ARCH).$(SHLIBEXT) : $(GAME_OBJS)
	$(CC) $(CFLAGS) $(SHLIBLDFLAGS) -o $@ $(GAME_OBJS)


#############################################################################
# MISC
#############################################################################

clean:
	-rm -f $(GAME_OBJS)

depend:
	gcc -MM $(GAME_OBJS:.o=.c)


install:
	cp gamei386.so ../quake2/bang

#
# From "make depend"
#

g_ai.o: g_ai.c g_local.h q_shared.h game.h
g_chase.o: g_chase.c g_local.h q_shared.h game.h
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
g_svcmds.o: g_svcmds.c g_local.h q_shared.h game.h
g_target.o: g_target.c g_local.h q_shared.h game.h
g_trigger.o: g_trigger.c g_local.h q_shared.h game.h
g_turret.o: g_turret.c g_local.h q_shared.h game.h
g_utils.o: g_utils.c g_local.h q_shared.h game.h
g_weapon.o: g_weapon.c g_local.h q_shared.h game.h
m_actor.o: m_actor.c g_local.h q_shared.h game.h m_actor.h
m_berserk.o: m_berserk.c g_local.h q_shared.h game.h m_berserk.h
m_boss2.o: m_boss2.c g_local.h q_shared.h game.h m_boss2.h
m_boss3.o: m_boss3.c g_local.h q_shared.h game.h m_boss32.h
m_boss31.o: m_boss31.c g_local.h q_shared.h game.h m_boss31.h
m_boss32.o: m_boss32.c g_local.h q_shared.h game.h m_boss32.h
m_brain.o: m_brain.c g_local.h q_shared.h game.h m_brain.h
m_chick.o: m_chick.c g_local.h q_shared.h game.h m_chick.h
m_flash.o: m_flash.c q_shared.h
m_flipper.o: m_flipper.c g_local.h q_shared.h game.h m_flipper.h
m_float.o: m_float.c g_local.h q_shared.h game.h m_float.h
m_flyer.o: m_flyer.c g_local.h q_shared.h game.h m_flyer.h
m_gladiator.o: m_gladiator.c g_local.h q_shared.h game.h m_gladiator.h
m_gunner.o: m_gunner.c g_local.h q_shared.h game.h m_gunner.h
m_hover.o: m_hover.c g_local.h q_shared.h game.h m_hover.h
m_infantry.o: m_infantry.c g_local.h q_shared.h game.h m_infantry.h
m_insane.o: m_insane.c g_local.h q_shared.h game.h m_insane.h
m_medic.o: m_medic.c g_local.h q_shared.h game.h m_medic.h
m_move.o: m_move.c g_local.h q_shared.h game.h
m_mutant.o: m_mutant.c g_local.h q_shared.h game.h m_mutant.h
m_parasite.o: m_parasite.c g_local.h q_shared.h game.h m_parasite.h
m_soldier.o: m_soldier.c g_local.h q_shared.h game.h m_soldier.h
m_supertank.o: m_supertank.c g_local.h q_shared.h game.h m_supertank.h
m_tank.o: m_tank.c g_local.h q_shared.h game.h m_tank.h
p_client.o: p_client.c g_local.h q_shared.h game.h m_player.h
p_hud.o: p_hud.c g_local.h q_shared.h game.h
p_trail.o: p_trail.c g_local.h q_shared.h game.h
p_view.o: p_view.c g_local.h q_shared.h game.h m_player.h
p_weapon.o: p_weapon.c g_local.h q_shared.h game.h m_player.h
q_shared.o: q_shared.c q_shared.h
q_devels.o: q_devels.c g_local.h q_shared.h game.h q_devels.h
