
# LCC-WIN32 makefile for Quake II Game DLL <rballard@aracnet.com>
# Files added for the purposes of Artifact Quake 2 :) <singe@ix.netcom.com>

CC=lcc
CFLAGS=-O2 -w -DC_ONLY
OBJS=\
        g_ai.obj \
        g_chase.obj \
        g_cmds.obj \
        g_combat.obj \
        g_func.obj \
        g_items.obj \
        g_main.obj \
        g_misc.obj \
        g_monster.obj \
        g_phys.obj \
        g_save.obj \
        g_spawn.obj \
        g_svcmds.obj \
        g_target.obj \
        g_trigger.obj \
        g_turret.obj \
        g_utils.obj \
        g_weapon.obj \
        m_actor.obj \
        m_berserk.obj \
        m_boss2.obj \
        m_boss3.obj \
        m_boss31.obj \
        m_boss32.obj \
        m_brain.obj \
        m_chick.obj \
        m_flash.obj \
        m_flipper.obj \
        m_float.obj \
        m_flyer.obj \
        m_gladiator.obj \
        m_gunner.obj \
        m_hover.obj \
        m_infantry.obj \
        m_insane.obj \
        m_medic.obj \
        m_move.obj \
        m_mutant.obj \
        m_parasite.obj \
        m_soldier.obj \
        m_supertank.obj \
        m_tank.obj \
        p_client.obj \
        p_hud.obj \
        p_trail.obj \
        p_view.obj \
        p_weapon.obj \
        q_shared.obj \
        aq2_general.obj \
        aq2_passive.obj \
        aq2_active.obj \

all:	gamex86.dll

gamex86.dll:    $(OBJS)
	lcclnk -s -x -dll -entry GetGameAPI *.obj game.def -o gamex86.dll
	dllfix

clean:
	erase *.obj
	erase gamex86.dll

%.obj: %.c
	$(CC) $(CFLAGS) $<
