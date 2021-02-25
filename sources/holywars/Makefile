BUILD_DEBUG_DIR=debug
BUILD_RELEASE_DIR=release

CC=gcc
BASE_CFLAGS=-Dstricmp=strcasecmp
RELEASE_CFLAGS=$(BASE_CFLAGS) -ffast-math -funroll-loops \
	-fomit-frame-pointer -fexpensive-optimizations
DEBUG_CFLAGS=$(BASE_CFLAGS) -g
LDFLAGS=-ldl -lm

ARCH=sparc
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
	$(BUILDDIR)/cnf_maps.o \
	$(BUILDDIR)/gslog.o \
	$(BUILDDIR)/g_ai.o \
	$(BUILDDIR)/g_chase.o \
	$(BUILDDIR)/g_cmds.o \
	$(BUILDDIR)/g_combat.o \
	$(BUILDDIR)/g_ctf.o \
	$(BUILDDIR)/g_func.o \
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
	$(BUILDDIR)/g_utils.o \
	$(BUILDDIR)/g_weapon.o \
	$(BUILDDIR)/hw_halo.o \
	$(BUILDDIR)/hw_heres.o \
	$(BUILDDIR)/hw_main.o \
	$(BUILDDIR)/hw_utils.o \
	$(BUILDDIR)/m_move.o \
	$(BUILDDIR)/p_client.o \
	$(BUILDDIR)/p_hud.o \
	$(BUILDDIR)/p_menu.o \
	$(BUILDDIR)/p_trail.o \
	$(BUILDDIR)/p_view.o \
	$(BUILDDIR)/p_weapon.o \
	$(BUILDDIR)/q_shared.o \
	$(BUILDDIR)/stdlog.o

$(BUILDDIR)/game$(ARCH).$(SHLIBEXT) : $(GAME_OBJS)
	$(CC) $(CFLAGS) $(SHLIBLDFLAGS) -o $@ $(GAME_OBJS)

$(BUILDDIR)/cnf_maps.o :    cnf_maps.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/gslog.o :       gslog.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/g_ai.o :        g_ai.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/g_chase.o :     g_chase.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/g_cmds.o :      g_cmds.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/g_combat.o :    g_combat.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/g_ctf.o :       g_ctf.c
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

$(BUILDDIR)/g_svcmds.o :    g_svcmds.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/g_target.o :    g_target.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/g_trigger.o :   g_trigger.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/g_utils.o :     g_utils.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/g_weapon.o :    g_weapon.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/hw_halo.o :     hw_halo.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/hw_heres.o :    hw_heres.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/hw_main.o :     hw_main.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/hw_utils.o :    hw_utils.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/m_move.o :      m_move.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/p_client.o :    p_client.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/p_hud.o :       p_hud.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/p_menu.o :      p_menu.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/p_trail.o :     p_trail.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/p_view.o :      p_view.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/p_weapon.o :    p_weapon.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/q_shared.o :    q_shared.c
	$(DO_SHLIB_CC)

$(BUILDDIR)/stdlog.o :      stdlog.c
	$(DO_SHLIB_CC)

#####

clean: clean-debug clean-release

clean-debug:
	$(MAKE) clean2 BUILDDIR=$(BUILD_DEBUG_DIR) CFLAGS="$(DEBUG_CFLAGS)"

clean-release:
	$(MAKE) clean2 BUILDDIR=$(BUILD_RELEASE_DIR) CFLAGS="$(DEBUG_CFLAGS)"

clean2:
	-rm -f $(GAME_OBJS)
