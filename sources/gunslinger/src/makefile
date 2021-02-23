# Makefile for iniread

# Use /bin/sh as shell
SHELL = /bin/sh

# Use GNU C compiler
CC = gcc

# Compilation options
#CFLAGS = -O2 -s -Wall
CFLAGS = -O2 -ffast-math -w -DGAME_INCLUDE

OUTFILES = g_ai.o g_cmds.o g_combat.o g_func.o g_items.o g_main.o g_misc.o \
        g_monster.o g_phys.o g_save.o g_spawn.o g_target.o g_trigger.o \
        g_turret.o g_utils.o g_weapon.o g_svcmds.o g_chase.o\
        m_actor.o m_berserk.o m_boss2.o m_boss3.o m_boss31.o m_boss32.o \
        m_brain.o m_chick.o m_flash.o m_flipper.o m_float.o m_flyer.o \
        m_gladiator.o m_gunner.o m_hover.o m_infantry.o m_insane.o \
        m_medic.o m_move.o m_mutant.o m_parasite.o m_soldier.o \
        m_supertank.o m_tank.o \
        p_client.o p_hud.o p_trail.o p_view.o p_weapon.o \
        q_shared.o q_devels.o z_gq.o z_menu.o botstuff.o


# Libraries to use
LIBS = 

# Name of program
all: gamei386.so

.PHONY: clean
clean:
	@rm -rf obj

# test game makefile

gamei386.so: $(OUTFILES)
	$(CC) $(OUTFILES) $(LDFLAGS) -shared -o gamei386.so

bostuff.o: botstuff.c g_lobal.h game.h q_devels.h q_shared.h z_menu.h
g_ai.o: g_ai.c g_local.h q_shared.h game.h q_devels.h z_menu.h
g_chase.o: g_chase.c g_local.h game.h q_devels.h z_menu.h
g_cmds.o: g_cmds.c g_local.h q_shared.h game.h m_player.h game.h q_devels.h z_menu.h z_gq.h
g_combat.o: g_combat.c g_local.h q_shared.h game.h z_gq.h q_devels.h z_menu.h
g_func.o: g_func.c g_local.h q_shared.h game.h q_devels.h z_menu.h
g_items.o: g_items.c g_local.h q_shared.h game.h z_gq.h q_devels.h z_menu.h
g_main.o: g_main.c g_local.h q_shared.h game.h z_gq.h q_devels.h z_menu.h
g_misc.o: g_misc.c g_local.h q_shared.h game.h z_gq.h q_devels.h z_menu.h
g_monster.o: g_monster.c g_local.h q_shared.h game.h q_devels.h z_menu.h
g_phys.o: g_phys.c g_local.h q_shared.h game.h q_devels.h z_menu.h
g_save.o: g_save.c g_local.h q_shared.h game.h q_devels.h z_menu.h
g_spawn.o: g_spawn.c g_local.h q_shared.h game.h z_gq.h q_devels.h z_menu.h
g_svcmds.o: g_svcmds.c g_local.h q_devels.h z_menu.h
g_target.o: g_target.c g_local.h q_shared.h game.h q_devels.h z_menu.h
g_trigger.o: g_trigger.c g_local.h q_shared.h game.h q_devels.h z_menu.h
g_turret.o: g_turret.c g_local.h q_shared.h game.h q_devels.h z_menu.h
g_utils.o: g_utils.c g_local.h q_shared.h game.h q_devels.h z_menu.h
g_weapon.o: g_weapon.c g_local.h q_shared.h game.h q_devels.h z_menu.h
m_actor.o: m_actor.c g_local.h q_shared.h game.h m_actor.h q_devels.h z_menu.h
m_berserk.o: m_berserk.c g_local.h q_shared.h game.h m_berserk.h q_devels.h z_menu.h
m_boss2.o: m_boss2.c g_local.h q_shared.h game.h m_boss2.h q_devels.h z_menu.h
m_boss3.o: m_boss3.c g_local.h q_shared.h game.h m_boss32.h q_devels.h z_menu.h
m_boss31.o: m_boss31.c g_local.h q_shared.h game.h m_boss31.h q_devels.h z_menu.h
m_boss32.o: m_boss32.c g_local.h q_shared.h game.h m_boss32.h q_devels.h z_menu.h
m_brain.o: m_brain.c g_local.h q_shared.h game.h m_brain.h q_devels.h z_menu.h
m_chick.o: m_chick.c g_local.h q_shared.h game.h m_chick.h q_devels.h z_menu.h
m_flash.o: m_flash.c q_shared.h
m_flipper.o: m_flipper.c g_local.h q_shared.h game.h m_flipper.h q_devels.h z_menu.h
m_float.o: m_float.c g_local.h q_shared.h game.h m_float.h q_devels.h z_menu.h
m_flyer.o: m_flyer.c g_local.h q_shared.h game.h m_flyer.h q_devels.h z_menu.h
m_gladiator.o: m_gladiator.c g_local.h q_shared.h game.h m_gladiator.h q_devels.h z_menu.h
m_gunner.o: m_gunner.c g_local.h q_shared.h game.h m_gunner.h q_devels.h z_menu.h
m_hover.o: m_hover.c g_local.h q_shared.h game.h m_hover.h q_devels.h z_menu.h
m_infantry.o: m_infantry.c g_local.h q_shared.h game.h m_infantry.h q_devels.h z_menu.h
m_insane.o: m_insane.c g_local.h q_shared.h game.h m_insane.h q_devels.h z_menu.h
m_medic.o: m_medic.c g_local.h q_shared.h game.h m_medic.h q_devels.h z_menu.h
m_move.o: m_move.c g_local.h q_shared.h game.h q_devels.h z_menu.h
m_mutant.o: m_mutant.c g_local.h q_shared.h game.h m_mutant.h q_devels.h z_menu.h
m_parasite.o: m_parasite.c g_local.h q_shared.h game.h m_parasite.h q_devels.h z_menu.h
m_soldier.o: m_soldier.c g_local.h q_shared.h game.h m_soldier.h q_devels.h z_menu.h
m_supertank.o: m_supertank.c g_local.h q_shared.h game.h m_supertank.h q_devels.h z_menu.h
m_tank.o: m_tank.c g_local.h q_shared.h game.h m_tank.h q_devels.h z_menu.h
p_client.o: p_client.c g_local.h q_shared.h game.h m_player.h z_gq.h q_devels.h z_menu.h
p_hud.o: p_hud.c g_local.h q_shared.h game.h z_gq.h q_devels.h z_menu.h
p_trail.o: p_trail.c g_local.h q_shared.h game.h q_devels.h z_menu.h
p_view.o: p_view.c g_local.h q_shared.h game.h m_player.h z_gq.h q_devels.h z_menu.h
p_weapon.o: p_weapon.c g_local.h q_shared.h game.h m_player.h z_gq.h q_devels.h z_menu.h
q_devels.o: q_devels.c g_local.h q_devels.h z_menu.h
q_shared.o: q_shared.c q_shared.h
z_gq.o: z_gq.c z_gq.h g_local.h game.h q_devels.h z_menu.h q_shared.h
z_menu.o: z_menu.c z_menu.h g_local.h game.h q_devels.h z_menu.h
