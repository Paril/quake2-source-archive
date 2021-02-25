BUILD_DEBUG_DIR=debug
BUILD_RELEASE_DIR=release
ARCH=i386
CC=gcc
BASE_CFLAGS=-Dstricmp=strcasecmp
RELEASE_CFLAGS=$(BASE_CFLAGS) -ffast-math -funroll-loops \
	-fomit-frame-pointer -fexpensive-optimizations
DEBUG_CFLAGS=$(BASE_CFLAGS) -g
LDFLAGS=-ldl -lm

SHLIBEXT=so

SHLIBCFLAGS=-fPIC
SHLIBLDFLAGS=-shared

DO_CC=$(CC) $(CFLAGS) -o $@ -c $<
DO_SHLIB_CC=$(CC) $(CFLAGS) $(SHLIBCFLAGS) -o $@ -c $<

TARGETS=$(BUILDDIR)/game$(ARCH).$(SHLIBEXT) \

build_debug:
	@-mkdir $(BUILD_DEBUG_DIR)
	$(MAKE) targets BUILDDIR=$(BUILD_DEBUG_DIR) CFLAGS="$(DEBUG_CFLAGS)"

build_release:
	@-mkdir $(BUILD_RELEASE_DIR)
	$(MAKE) targets BUILDDIR=$(BUILD_RELEASE_DIR) CFLAGS="$(RELEASE_CFLAGS)"

all: build_debug build_release

targets: $(TARGETS)

GAME_OBJS = \
        $(BUILDDIR)/g_ai.o \
        $(BUILDDIR)/g_chase.o \
        $(BUILDDIR)/g_cmds.o \
        $(BUILDDIR)/g_combat.o \
        $(BUILDDIR)/g_func.o \
        $(BUILDDIR)/g_gen.o \
        $(BUILDDIR)/g_genctf.o \
        $(BUILDDIR)/g_geneavy.o \
        $(BUILDDIR)/g_genents.o \
        $(BUILDDIR)/g_genmaps.o \
        $(BUILDDIR)/g_genmenus.o \
        $(BUILDDIR)/g_genmonster.o \
        $(BUILDDIR)/g_genq1grap.o \
        $(BUILDDIR)/g_genq2grap.o \
        $(BUILDDIR)/g_genscore.o \
        $(BUILDDIR)/g_gentech.o \
        $(BUILDDIR)/g_genutil.o \
	$(BUILDDIR)/g_genweap.o \
        $(BUILDDIR)/g_items.o \
        $(BUILDDIR)/g_main.o \
        $(BUILDDIR)/g_misc.o \
        $(BUILDDIR)/g_monster.o \
        $(BUILDDIR)/g_phys.o \
        $(BUILDDIR)/g_save.o \
        $(BUILDDIR)/g_spawn.o \
        $(BUILDDIR)/g_svcmds.o \
        $(BUILDDIR)/g_target.o \
        $(BUILDDIR)/g_trigger.o \
        $(BUILDDIR)/g_turret.o \
        $(BUILDDIR)/g_utils.o \
        $(BUILDDIR)/g_weapon.o \
        $(BUILDDIR)/m_actor.o \
        $(BUILDDIR)/m_berserk.o \
        $(BUILDDIR)/m_boss2.o \
        $(BUILDDIR)/m_boss3.o \
        $(BUILDDIR)/m_boss31.o \
        $(BUILDDIR)/m_boss32.o \
        $(BUILDDIR)/m_brain.o \
        $(BUILDDIR)/m_chick.o \
        $(BUILDDIR)/m_flash.o \
        $(BUILDDIR)/m_flipper.o \
        $(BUILDDIR)/m_float.o \
        $(BUILDDIR)/m_flyer.o \
        $(BUILDDIR)/m_genq1chton.o \
        $(BUILDDIR)/m_genq1dog.o \
        $(BUILDDIR)/m_genq1fiend.o \
        $(BUILDDIR)/m_genq1grunt.o \
        $(BUILDDIR)/m_genq1knight.o \
        $(BUILDDIR)/m_genq1ogre.o \
        $(BUILDDIR)/m_genq1scrag.o \
        $(BUILDDIR)/m_genq1sham.o \
        $(BUILDDIR)/m_genq1strabo.o \
        $(BUILDDIR)/m_genq1zombie.o \
        $(BUILDDIR)/m_gladiator.o \
        $(BUILDDIR)/m_gunner.o \
        $(BUILDDIR)/m_hover.o \
        $(BUILDDIR)/m_infantry.o \
        $(BUILDDIR)/m_insane.o \
        $(BUILDDIR)/m_medic.o \
        $(BUILDDIR)/m_move.o \
        $(BUILDDIR)/m_mutant.o \
        $(BUILDDIR)/m_parasite.o \
        $(BUILDDIR)/m_soldier.o \
        $(BUILDDIR)/m_supertank.o \
        $(BUILDDIR)/m_tank.o \
        $(BUILDDIR)/menu.o \
        $(BUILDDIR)/p_client.o \
        $(BUILDDIR)/p_gencam.o \
        $(BUILDDIR)/p_genclient.o \
        $(BUILDDIR)/p_genweap.o \
        $(BUILDDIR)/p_hud.o \
        $(BUILDDIR)/p_trail.o \
        $(BUILDDIR)/p_view.o \
        $(BUILDDIR)/p_weapon.o \
        $(BUILDDIR)/q_shared.o

$(BUILDDIR)/game$(ARCH).$(SHLIBEXT) : $(GAME_OBJS)
	$(CC) $(CFLAGS) $(SHLIBLDFLAGS) -o $@ $(GAME_OBJS)

$(BUILDDIR)/g_ai.o :        g_ai.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/p_client.o :    p_client.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/g_cmds.o :      g_cmds.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/g_svcmds.o :    g_svcmds.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/g_chase.o :     g_chase.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/g_combat.o :    g_combat.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/g_func.o :      g_func.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/g_items.o :     g_items.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/g_main.o :      g_main.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/g_misc.o :      g_misc.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/g_monster.o :   g_monster.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/g_phys.o :      g_phys.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/g_save.o :      g_save.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/g_spawn.o :     g_spawn.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/g_target.o :    g_target.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/g_trigger.o :   g_trigger.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/g_turret.o :    g_turret.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/g_utils.o :     g_utils.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/g_weapon.o :    g_weapon.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/m_actor.o :     m_actor.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/m_berserk.o :   m_berserk.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/m_boss2.o :     m_boss2.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/m_boss3.o :     m_boss3.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/m_boss31.o :    m_boss31.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/m_boss32.o :    m_boss32.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/m_brain.o :     m_brain.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/m_chick.o :     m_chick.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/m_flipper.o :   m_flipper.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/m_float.o :     m_float.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/m_flyer.o :     m_flyer.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/m_gladiator.o : m_gladiator.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/m_gunner.o :    m_gunner.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/m_hover.o :     m_hover.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/m_infantry.o :  m_infantry.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/m_insane.o :    m_insane.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/m_medic.o :     m_medic.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/m_move.o :      m_move.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/m_mutant.o :    m_mutant.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/m_parasite.o :  m_parasite.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/m_soldier.o :   m_soldier.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/m_supertank.o : m_supertank.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/m_tank.o :      m_tank.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/p_hud.o :       p_hud.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/p_trail.o :     p_trail.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/p_view.o :      p_view.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/p_weapon.o :    p_weapon.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/q_shared.o :    q_shared.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/m_flash.o :     m_flash.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/g_gen.o :     g_gen.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/g_genctf.o :     g_genctf.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/g_geneavy.o :     g_geneavy.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/g_genents.o :     g_genents.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/g_genmaps.o :     g_genmaps.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/g_genmenus.o :     g_genmenus.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/g_genmonster.o :     g_genmonster.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/g_genq1grap.o :     g_genq1grap.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/g_genq2grap.o :     g_genq2grap.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/g_genscore.o :     g_genscore.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/g_gentech.o :     g_gentech.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/g_genutil.o :     g_genutil.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/g_genweap.o :     g_genweap.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/m_genq1chton.o :     m_genq1chton.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/m_genq1dog.o :     m_genq1dog.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/m_genq1fiend.o :     m_genq1fiend.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/m_genq1grunt.o :     m_genq1grunt.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/m_genq1knight.o :     m_genq1knight.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/m_genq1ogre.o :     m_genq1ogre.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/m_genq1scrag.o :     m_genq1scrag.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/m_genq1sham.o :     m_genq1sham.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/m_genq1strabo.o :     m_genq1strabo.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/m_genq1zombie.o :     m_genq1zombie.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/menu.o :     menu.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/p_gencam.o :     p_gencam.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/p_genclient.o :     p_genclient.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/p_genweap.o :     p_genweap.c
	$(DO_SHLIB_CC)

#####

clean: clean-debug clean-release

clean-debug:
	$(MAKE) clean2 BUILDDIR=$(BUILD_DEBUG_DIR) CFLAGS="$(DEBUG_CFLAGS)"

clean-release:
	$(MAKE) clean2 BUILDDIR=$(BUILD_RELEASE_DIR) CFLAGS="$(DEBUG_CFLAGS)"

clean2:
	-rm -f $(GAME_OBJS)
