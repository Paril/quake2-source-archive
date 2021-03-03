#
# Makefile for the Gladiator Bot game source: gamex86.dll
# Intended for LCC-Win32
#

CC=lcc
CFLAGS=-DC_ONLY -o
OBJS= bl_cmd.obj \
	bl_botcfg.obj bl_debug.obj bl_main.obj bl_redirgi.obj bl_spawn.obj dm_ball_rogue.obj \
	dm_tag_rogue.obj g_ai.obj g_arena.obj g_ch.obj g_chase.obj g_cmds.obj g_combat.obj \
	g_ctf.obj g_func.obj g_items.obj g_log.obj g_main.obj g_misc.obj g_monster.obj \
	g_newai_rogue.obj g_newdm_rogue.obj g_newfnc_rogue.obj g_newtarg_rogue.obj \
	g_newtrig_rogue.obj g_newweap_rogue.obj g_phys.obj g_save.obj g_spawn.obj \
	g_sphere_rogue.obj g_svcmds.obj g_target.obj g_trigger.obj g_turret.obj \
	g_utils.obj g_weapon.obj m_actor.obj m_berserk.obj m_boss2.obj m_boss3.obj \
	m_boss31.obj m_boss32.obj m_boss5_xatrix.obj m_brain.obj m_carrier_rogue.obj \
	m_chick.obj m_fixbot_xatrix.obj m_flash.obj m_flipper.obj m_float.obj m_flyer.obj \
	m_gekk_xatrix.obj m_gladb_xatrix.obj m_gladiator.obj m_gunner.obj m_hover.obj \
	m_infantry.obj m_insane.obj m_medic.obj m_move.obj m_move2_rogue.obj m_mutant.obj \
	m_parasite.obj m_soldier.obj m_stalker_rogue.obj m_supertank.obj m_tank.obj \
	m_turret_rogue.obj m_widow2_rogue.obj m_widow_rogue.obj p_botmenu.obj \
	p_client.obj p_hud.obj p_lag.obj p_menu.obj p_menulib.obj p_observer.obj p_trail.obj \
	p_view.obj p_weapon.obj q_shared.obj

all:	gamex86.dll

gamex86.dll:	$(OBJS)
	lcclnk -dll -entry GetBotAPI *.obj game.def -o gamex86.dll

clean:
	del *.obj gamex86.dll

%.obj: %.c
	$(CC) $(CFLAGS) $<

