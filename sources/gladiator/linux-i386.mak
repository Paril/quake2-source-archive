#
# Makefile for the Gladiator Bot game source: gamei386.so
# Intended for gcc/Linux
#

ARCH=i386
CC=gcc
BASE_CFLAGS=-Dstricmp=strcasecmp

#use these cflags to optimize it
CFLAGS=$(BASE_CFLAGS) -m486 -O6 -ffast-math -funroll-loops \
	-fomit-frame-pointer -fexpensive-optimizations -malign-loops=2 \
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
# GLADIATOR BOT GAME SOURCE
#############################################################################

.c.o:
	$(DO_CC)

GAME_OBJS = \
	bl_cmd.o\
	bl_botcfg.o bl_debug.o bl_main.o bl_redirgi.o bl_spawn.o dm_ball_rogue.o\
	dm_tag_rogue.o g_ai.o g_arena.o g_ch.o g_chase.o g_cmds.o g_combat.o\
	g_ctf.o g_func.o g_items.o g_log.o g_main.o g_misc.o g_monster.o\
	g_newai_rogue.o g_newdm_rogue.o g_newfnc_rogue.o g_newtarg_rogue.o\
	g_newtrig_rogue.o g_newweap_rogue.o g_phys.o g_save.o g_spawn.o\
	g_sphere_rogue.o g_svcmds.o g_target.o g_trigger.o g_turret.o\
	g_utils.o g_weapon.o m_actor.o m_berserk.o m_boss2.o m_boss3.o\
	m_boss31.o m_boss32.o m_boss5_xatrix.o m_brain.o m_carrier_rogue.o\
	m_chick.o m_fixbot_xatrix.o m_flash.o m_flipper.o m_float.o m_flyer.o\
	m_gekk_xatrix.o m_gladb_xatrix.o m_gladiator.o m_gunner.o m_hover.o\
	m_infantry.o m_insane.o m_medic.o m_move.o m_move2_rogue.o m_mutant.o\
	m_parasite.o m_soldier.o m_stalker_rogue.o m_supertank.o m_tank.o\
	m_turret_rogue.o m_widow2_rogue.o m_widow_rogue.o p_botmenu.o\
	p_client.o p_hud.o p_lag.o p_menu.o p_menulib.o p_observer.o p_trail.o\
	p_view.o p_weapon.o q_shared.o

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
	cp gamei386.so ..

#
# From "make depend"
#

