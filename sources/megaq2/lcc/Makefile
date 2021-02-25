# Wedit Makefile for project megaq2
SRCDIR=d:\old-c\quake2\megaq2\game
CFLAGS= -g2 
CC=$(LCCROOT)\bin\lcc.exe
LINKER=$(LCCROOT)\bin\lcclnk.exe
OBJS=\
	q_shared.obj \
	p_weapon.obj \
	p_view.obj \
	p_trail.obj \
	p_hud.obj \
	p_client.obj \
	m_turret.obj \
	m_tank.obj \
	m_supertank.obj \
	m_soldier.obj \
	m_parasite.obj \
	m_mutant.obj \
	m_move.obj \
	m_medic.obj \
	m_insane.obj \
	m_infantry.obj \
	m_hover.obj \
	m_gunner.obj \
	m_gladiator.obj \
	m_flyer.obj \
	m_fly.obj \
	m_float.obj \
	m_flipper.obj \
	m_flash.obj \
	m_cutbox.obj \
	m_chick.obj \
	m_brain.obj \
	m_boss32.obj \
	m_boss31.obj \
	m_boss3.obj \
	m_boss2.obj \
	m_block.obj \
	m_berserk.obj \
	m_actor.obj \
	lcchack.obj \
	g_weapon.obj \
	g_utils.obj \
	g_turret.obj \
	g_trigger.obj \
	g_target.obj \
	g_svcmds.obj \
	g_spawn.obj \
	g_save.obj \
	g_phys.obj \
	g_monster.obj \
	g_misc.obj \
	g_main.obj \
	g_items.obj \
	g_func.obj \
	g_combat.obj \
	g_cmds.obj \
	g_chase.obj \
	g_ai.obj

LIBS=
EXE=gamex86.dll

$(EXE):	$(OBJS) Makefile
	$(LINKER)  -dll $(SRCDIR)\game.def -o $(SRCDIR)\lcc\gamex86.dll $(OBJS) $(LIBS)

# Build q_shared.c
Q_SHARED_C=\
	$(SRCDIR)\q_shared.h\

q_shared.obj: $(Q_SHARED_C) $(SRCDIR)\q_shared.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\q_shared.c

# Build p_weapon.c
P_WEAPON_C=\
	$(SRCDIR)\g_local.h\
	$(SRCDIR)\q_shared.h\
	$(SRCDIR)\game.h\
	$(SRCDIR)\m_player.h\

p_weapon.obj: $(P_WEAPON_C) $(SRCDIR)\p_weapon.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\p_weapon.c

# Build p_view.c
P_VIEW_C=\
	$(SRCDIR)\g_local.h\
	$(SRCDIR)\q_shared.h\
	$(SRCDIR)\game.h\
	$(SRCDIR)\m_player.h\

p_view.obj: $(P_VIEW_C) $(SRCDIR)\p_view.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\p_view.c

# Build p_trail.c
P_TRAIL_C=\
	$(SRCDIR)\g_local.h\
	$(SRCDIR)\q_shared.h\
	$(SRCDIR)\game.h\

p_trail.obj: $(P_TRAIL_C) $(SRCDIR)\p_trail.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\p_trail.c

# Build p_hud.c
P_HUD_C=\
	$(SRCDIR)\g_local.h\
	$(SRCDIR)\q_shared.h\
	$(SRCDIR)\game.h\

p_hud.obj: $(P_HUD_C) $(SRCDIR)\p_hud.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\p_hud.c

# Build p_client.c
P_CLIENT_C=\
	$(SRCDIR)\g_local.h\
	$(SRCDIR)\q_shared.h\
	$(SRCDIR)\game.h\
	$(SRCDIR)\m_player.h\

p_client.obj: $(P_CLIENT_C) $(SRCDIR)\p_client.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\p_client.c

# Build m_turret.c
M_TURRET_C=\
	$(SRCDIR)\m_turret.h\
	$(SRCDIR)\g_local.h\
	$(SRCDIR)\q_shared.h\
	$(SRCDIR)\game.h\

m_turret.obj: $(M_TURRET_C) $(SRCDIR)\m_turret.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\m_turret.c

# Build m_tank.c
M_TANK_C=\
	$(SRCDIR)\g_local.h\
	$(SRCDIR)\q_shared.h\
	$(SRCDIR)\game.h\
	$(SRCDIR)\m_tank.h\

m_tank.obj: $(M_TANK_C) $(SRCDIR)\m_tank.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\m_tank.c

# Build m_supertank.c
M_SUPERTANK_C=\
	$(SRCDIR)\g_local.h\
	$(SRCDIR)\q_shared.h\
	$(SRCDIR)\game.h\
	$(SRCDIR)\m_supertank.h\

m_supertank.obj: $(M_SUPERTANK_C) $(SRCDIR)\m_supertank.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\m_supertank.c

# Build m_soldier.c
M_SOLDIER_C=\
	$(SRCDIR)\g_local.h\
	$(SRCDIR)\q_shared.h\
	$(SRCDIR)\game.h\
	$(SRCDIR)\m_soldier.h\

m_soldier.obj: $(M_SOLDIER_C) $(SRCDIR)\m_soldier.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\m_soldier.c

# Build m_parasite.c
M_PARASITE_C=\
	$(SRCDIR)\g_local.h\
	$(SRCDIR)\q_shared.h\
	$(SRCDIR)\game.h\
	$(SRCDIR)\m_parasite.h\

m_parasite.obj: $(M_PARASITE_C) $(SRCDIR)\m_parasite.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\m_parasite.c

# Build m_mutant.c
M_MUTANT_C=\
	$(SRCDIR)\g_local.h\
	$(SRCDIR)\q_shared.h\
	$(SRCDIR)\game.h\
	$(SRCDIR)\m_mutant.h\

m_mutant.obj: $(M_MUTANT_C) $(SRCDIR)\m_mutant.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\m_mutant.c

# Build m_move.c
M_MOVE_C=\
	$(SRCDIR)\g_local.h\
	$(SRCDIR)\q_shared.h\
	$(SRCDIR)\game.h\

m_move.obj: $(M_MOVE_C) $(SRCDIR)\m_move.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\m_move.c

# Build m_medic.c
M_MEDIC_C=\
	$(SRCDIR)\g_local.h\
	$(SRCDIR)\q_shared.h\
	$(SRCDIR)\game.h\
	$(SRCDIR)\m_medic.h\

m_medic.obj: $(M_MEDIC_C) $(SRCDIR)\m_medic.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\m_medic.c

# Build m_insane.c
M_INSANE_C=\
	$(SRCDIR)\g_local.h\
	$(SRCDIR)\q_shared.h\
	$(SRCDIR)\game.h\
	$(SRCDIR)\m_insane.h\

m_insane.obj: $(M_INSANE_C) $(SRCDIR)\m_insane.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\m_insane.c

# Build m_infantry.c
M_INFANTRY_C=\
	$(SRCDIR)\g_local.h\
	$(SRCDIR)\q_shared.h\
	$(SRCDIR)\game.h\
	$(SRCDIR)\m_infantry.h\

m_infantry.obj: $(M_INFANTRY_C) $(SRCDIR)\m_infantry.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\m_infantry.c

# Build m_hover.c
M_HOVER_C=\
	$(SRCDIR)\g_local.h\
	$(SRCDIR)\q_shared.h\
	$(SRCDIR)\game.h\
	$(SRCDIR)\m_hover.h\

m_hover.obj: $(M_HOVER_C) $(SRCDIR)\m_hover.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\m_hover.c

# Build m_gunner.c
M_GUNNER_C=\
	$(SRCDIR)\g_local.h\
	$(SRCDIR)\q_shared.h\
	$(SRCDIR)\game.h\
	$(SRCDIR)\m_gunner.h\

m_gunner.obj: $(M_GUNNER_C) $(SRCDIR)\m_gunner.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\m_gunner.c

# Build m_gladiator.c
M_GLADIATOR_C=\
	$(SRCDIR)\g_local.h\
	$(SRCDIR)\q_shared.h\
	$(SRCDIR)\game.h\
	$(SRCDIR)\m_gladiator.h\

m_gladiator.obj: $(M_GLADIATOR_C) $(SRCDIR)\m_gladiator.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\m_gladiator.c

# Build m_flyer.c
M_FLYER_C=\
	$(SRCDIR)\g_local.h\
	$(SRCDIR)\q_shared.h\
	$(SRCDIR)\game.h\
	$(SRCDIR)\m_flyer.h\

m_flyer.obj: $(M_FLYER_C) $(SRCDIR)\m_flyer.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\m_flyer.c

# Build m_fly.c
M_FLY_C=\
	$(SRCDIR)\m_fly.h\
	$(SRCDIR)\g_local.h\
	$(SRCDIR)\q_shared.h\
	$(SRCDIR)\game.h\

m_fly.obj: $(M_FLY_C) $(SRCDIR)\m_fly.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\m_fly.c

# Build m_float.c
M_FLOAT_C=\
	$(SRCDIR)\g_local.h\
	$(SRCDIR)\q_shared.h\
	$(SRCDIR)\game.h\
	$(SRCDIR)\m_float.h\

m_float.obj: $(M_FLOAT_C) $(SRCDIR)\m_float.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\m_float.c

# Build m_flipper.c
M_FLIPPER_C=\
	$(SRCDIR)\g_local.h\
	$(SRCDIR)\q_shared.h\
	$(SRCDIR)\game.h\
	$(SRCDIR)\m_flipper.h\

m_flipper.obj: $(M_FLIPPER_C) $(SRCDIR)\m_flipper.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\m_flipper.c

# Build m_flash.c
M_FLASH_C=\
	$(SRCDIR)\q_shared.h\

m_flash.obj: $(M_FLASH_C) $(SRCDIR)\m_flash.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\m_flash.c

# Build m_cutbox.c
M_CUTBOX_C=\
	$(SRCDIR)\g_local.h\
	$(SRCDIR)\q_shared.h\
	$(SRCDIR)\game.h\

m_cutbox.obj: $(M_CUTBOX_C) $(SRCDIR)\m_cutbox.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\m_cutbox.c

# Build m_chick.c
M_CHICK_C=\
	$(SRCDIR)\g_local.h\
	$(SRCDIR)\q_shared.h\
	$(SRCDIR)\game.h\
	$(SRCDIR)\m_chick.h\

m_chick.obj: $(M_CHICK_C) $(SRCDIR)\m_chick.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\m_chick.c

# Build m_brain.c
M_BRAIN_C=\
	$(SRCDIR)\g_local.h\
	$(SRCDIR)\q_shared.h\
	$(SRCDIR)\game.h\
	$(SRCDIR)\m_brain.h\

m_brain.obj: $(M_BRAIN_C) $(SRCDIR)\m_brain.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\m_brain.c

# Build m_boss32.c
M_BOSS32_C=\
	$(SRCDIR)\g_local.h\
	$(SRCDIR)\q_shared.h\
	$(SRCDIR)\game.h\
	$(SRCDIR)\m_boss32.h\

m_boss32.obj: $(M_BOSS32_C) $(SRCDIR)\m_boss32.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\m_boss32.c

# Build m_boss31.c
M_BOSS31_C=\
	$(SRCDIR)\g_local.h\
	$(SRCDIR)\q_shared.h\
	$(SRCDIR)\game.h\
	$(SRCDIR)\m_boss31.h\

m_boss31.obj: $(M_BOSS31_C) $(SRCDIR)\m_boss31.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\m_boss31.c

# Build m_boss3.c
M_BOSS3_C=\
	$(SRCDIR)\g_local.h\
	$(SRCDIR)\q_shared.h\
	$(SRCDIR)\game.h\
	$(SRCDIR)\m_boss32.h\

m_boss3.obj: $(M_BOSS3_C) $(SRCDIR)\m_boss3.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\m_boss3.c

# Build m_boss2.c
M_BOSS2_C=\
	$(SRCDIR)\g_local.h\
	$(SRCDIR)\q_shared.h\
	$(SRCDIR)\game.h\
	$(SRCDIR)\m_boss2.h\

m_boss2.obj: $(M_BOSS2_C) $(SRCDIR)\m_boss2.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\m_boss2.c

# Build m_block.c
M_BLOCK_C=\
	$(SRCDIR)\m_block.h\
	$(SRCDIR)\g_local.h\
	$(SRCDIR)\q_shared.h\
	$(SRCDIR)\game.h\

m_block.obj: $(M_BLOCK_C) $(SRCDIR)\m_block.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\m_block.c

# Build m_berserk.c
M_BERSERK_C=\
	$(SRCDIR)\g_local.h\
	$(SRCDIR)\q_shared.h\
	$(SRCDIR)\game.h\
	$(SRCDIR)\m_berserk.h\

m_berserk.obj: $(M_BERSERK_C) $(SRCDIR)\m_berserk.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\m_berserk.c

# Build m_actor.c
M_ACTOR_C=\
	$(SRCDIR)\g_local.h\
	$(SRCDIR)\q_shared.h\
	$(SRCDIR)\game.h\
	$(SRCDIR)\m_actor.h\

m_actor.obj: $(M_ACTOR_C) $(SRCDIR)\m_actor.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\m_actor.c

# Build lcchack.c
LCCHACK_C=\

lcchack.obj: $(LCCHACK_C) $(SRCDIR)\lcchack.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\lcchack.c

# Build g_weapon.c
G_WEAPON_C=\
	$(SRCDIR)\g_local.h\
	$(SRCDIR)\q_shared.h\
	$(SRCDIR)\game.h\

g_weapon.obj: $(G_WEAPON_C) $(SRCDIR)\g_weapon.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\g_weapon.c

# Build g_utils.c
G_UTILS_C=\
	$(SRCDIR)\g_local.h\
	$(SRCDIR)\q_shared.h\
	$(SRCDIR)\game.h\

g_utils.obj: $(G_UTILS_C) $(SRCDIR)\g_utils.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\g_utils.c

# Build g_turret.c
G_TURRET_C=\
	$(SRCDIR)\g_local.h\
	$(SRCDIR)\q_shared.h\
	$(SRCDIR)\game.h\

g_turret.obj: $(G_TURRET_C) $(SRCDIR)\g_turret.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\g_turret.c

# Build g_trigger.c
G_TRIGGER_C=\
	$(SRCDIR)\g_local.h\
	$(SRCDIR)\q_shared.h\
	$(SRCDIR)\game.h\

g_trigger.obj: $(G_TRIGGER_C) $(SRCDIR)\g_trigger.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\g_trigger.c

# Build g_target.c
G_TARGET_C=\
	$(SRCDIR)\g_local.h\
	$(SRCDIR)\q_shared.h\
	$(SRCDIR)\game.h\

g_target.obj: $(G_TARGET_C) $(SRCDIR)\g_target.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\g_target.c

# Build g_svcmds.c
G_SVCMDS_C=\
	$(SRCDIR)\g_local.h\
	$(SRCDIR)\q_shared.h\
	$(SRCDIR)\game.h\

g_svcmds.obj: $(G_SVCMDS_C) $(SRCDIR)\g_svcmds.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\g_svcmds.c

# Build g_spawn.c
G_SPAWN_C=\
	$(SRCDIR)\g_local.h\
	$(SRCDIR)\q_shared.h\
	$(SRCDIR)\game.h\

g_spawn.obj: $(G_SPAWN_C) $(SRCDIR)\g_spawn.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\g_spawn.c

# Build g_save.c
G_SAVE_C=\
	$(SRCDIR)\g_local.h\
	$(SRCDIR)\q_shared.h\
	$(SRCDIR)\game.h\

g_save.obj: $(G_SAVE_C) $(SRCDIR)\g_save.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\g_save.c

# Build g_phys.c
G_PHYS_C=\
	$(SRCDIR)\g_local.h\
	$(SRCDIR)\q_shared.h\
	$(SRCDIR)\game.h\

g_phys.obj: $(G_PHYS_C) $(SRCDIR)\g_phys.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\g_phys.c

# Build g_monster.c
G_MONSTER_C=\
	$(SRCDIR)\g_local.h\
	$(SRCDIR)\q_shared.h\
	$(SRCDIR)\game.h\

g_monster.obj: $(G_MONSTER_C) $(SRCDIR)\g_monster.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\g_monster.c

# Build g_misc.c
G_MISC_C=\
	$(SRCDIR)\g_local.h\
	$(SRCDIR)\q_shared.h\
	$(SRCDIR)\game.h\

g_misc.obj: $(G_MISC_C) $(SRCDIR)\g_misc.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\g_misc.c

# Build g_main.c
G_MAIN_C=\
	$(SRCDIR)\g_local.h\
	$(SRCDIR)\q_shared.h\
	$(SRCDIR)\game.h\

g_main.obj: $(G_MAIN_C) $(SRCDIR)\g_main.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\g_main.c

# Build g_items.c
G_ITEMS_C=\
	$(SRCDIR)\g_local.h\
	$(SRCDIR)\q_shared.h\
	$(SRCDIR)\game.h\

g_items.obj: $(G_ITEMS_C) $(SRCDIR)\g_items.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\g_items.c

# Build g_func.c
G_FUNC_C=\
	$(SRCDIR)\g_local.h\
	$(SRCDIR)\q_shared.h\
	$(SRCDIR)\game.h\

g_func.obj: $(G_FUNC_C) $(SRCDIR)\g_func.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\g_func.c

# Build g_combat.c
G_COMBAT_C=\
	$(SRCDIR)\g_local.h\
	$(SRCDIR)\q_shared.h\
	$(SRCDIR)\game.h\

g_combat.obj: $(G_COMBAT_C) $(SRCDIR)\g_combat.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\g_combat.c

# Build g_cmds.c
G_CMDS_C=\
	$(SRCDIR)\g_local.h\
	$(SRCDIR)\q_shared.h\
	$(SRCDIR)\game.h\
	$(SRCDIR)\m_player.h\

g_cmds.obj: $(G_CMDS_C) $(SRCDIR)\g_cmds.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\g_cmds.c

# Build g_chase.c
G_CHASE_C=\
	$(SRCDIR)\g_local.h\
	$(SRCDIR)\q_shared.h\
	$(SRCDIR)\game.h\

g_chase.obj: $(G_CHASE_C) $(SRCDIR)\g_chase.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\g_chase.c

# Build g_ai.c
G_AI_C=\
	$(SRCDIR)\g_local.h\
	$(SRCDIR)\q_shared.h\
	$(SRCDIR)\game.h\

g_ai.obj: $(G_AI_C) $(SRCDIR)\g_ai.c
	$(CC) -c $(CFLAGS) $(SRCDIR)\g_ai.c

link:
	$(LINKER)  -dll $(SRCDIR)\game.def -o $(SRCDIR)\lcc\gamex86.dll $(OBJS) $(LIBS)

clean:
	del $(OBJS) gamex86.dll
