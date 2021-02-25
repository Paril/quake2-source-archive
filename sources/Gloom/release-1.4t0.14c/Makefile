# makefile for Gloom

ARCH := $(shell gcc -dumpmachine | grep -oe '^[^-]*' | sed -e s/i.86/i386/ -e s/sun4u/sparc64/ -e s/arm.*/arm/ -e s/sa110/arm/ -e s/alpha/axp/)

ifeq ($(shell test $(ARCH) = x86_64 && echo $(CC) $(CFLAGS) | grep -oe -m32 ),-m32)
     ARCH:=i386
else ifeq ($(shell gcc -dumpmachine), mingw32)
     ARCH:=x86
endif

ifeq ($(shell gcc -dumpmachine), mingw32)
  LFLAGS+=-Wl,--export-all-symbols,game.def
  SHLIBEXT:=dll
else
  LFLAGS+=-lpthread
  SHLIBEXT=so
endif

BASE_CFLAGS:=-Dstricmp=strcasecmp -DQ_strncasecmp=strncasecmp $(CFLAGS) "-DGAMEDATE=\"$(shell export LANG=C;date +'%d %b %Y %H:%M %Z')\""
CFLAGS=$(BASE_CFLAGS) -Os -g -fPIC -std=c99 -shared

SHLIBLDFLAGS=-lm -shared

GAME_OBJS = \
g_cmds.o \
g_combat.o \
g_main.o \
g_weapon.o \
grapple.o \
m_cocoon.o \
m_gasser.o \
m_healer.o \
m_spiker.o \
p_client.o \
p_view.o \
p_weapon.o \
mersennetwister.o \
c4.o \
depot.o \
g_account.o \
g_breakin.o \
g_chase.o \
g_class.o \
g_func.o \
g_items.o \
g_log.o \
g_map.o \
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
g_wordfilter.o \
lmine.o \
m_breeder.o \
m_detector.o \
m_drone.o \
m_guardian.o \
m_hatchling.o \
m_obstacle.o \
m_stalker.o \
m_sting.o \
m_wraith.o \
p_biotech.o \
p_commando.o \
p_engineer.o \
p_exterm.o \
p_grunt.o \
p_heavy.o \
p_hud.o \
p_mech.o \
p_menu.o \
p_shotgun.o \
q_shared.o \
turret.o \
newqueues.o \
misc.o \
persistent.o \
fun.o \
glob.o \
override.o \
gloompmove_wrap.o \
practice.o

all: game

list:
	@echo gamedate.c
	@gcc -MM $(addsuffix .c, $(basename $(GAME_OBJS))) | tr '\\ 	' '\n' | grep '^[a-z].*[^:]$$' | sort -u

name:
	@echo game$(ARCH).$(SHLIBEXT)

game : $(GAME_OBJS)
	@echo -e \\nLinking game$(ARCH).$(SHLIBEXT)
	@$(CC) gamedate.c -o game$(ARCH).$(SHLIBEXT) $(SHLIBCFLAGS) $(GAME_OBJS) $(SHLIBLDFLAGS)
	#@chmod +x validate.sh
	#@./validate.sh

dist : game
	rm -f *.gz
	gzip -9 -n game$(ARCH).$(SHLIBEXT)
	mv gamei386.so.gz gamei386.`date '+%Y-%m-%d'`.so.gz

#############################################################################
# MISC
#############################################################################

clean:
	-rm -f $(GAME_OBJS) game$(ARCH).$(SHLIBEXT) gamei386.so.gz

# general defines
$(GAME_OBJS) : g_local.h game.h q_shared.h buildnum.h

# individual defines INCOMPLETE
g_cmds.o: m_player.h persistent.h
g_weapon.o c4.o : c4.h
p_view.o: m_mech.h m_stalker.h m_engineer.h m_wraith.h
p_weapon.o: m_player.h m_drone.h m_mech.h m_stalker.h m_guardian.h m_stinger.h m_wraith.h
g_class.o: m_player.h m_hatchling.h m_drone.h m_mech.h m_stalker.h \
	m_hsold.h m_breeder.h m_engineer.h m_guardian.h m_exterm.h \
	m_stinger.h m_wraith.h
m_cocoon.o: m_cocoon.h
m_healer.o: m_healer.h
m_spiker.o: m_spiker.h
g_breakin.o: m_breeder.h m_engineer.h m_player.h menus.h
m_breeder.o: m_breeder.h
m_gasser.o: m_obstacle.h
m_hatchling.o: m_hatchling.h
m_obstacle.o: m_obstacle.h
m_stalker.o: m_stalker.h
m_wraith.o: m_wraith.h
p_engineer.o: m_engineer.h
p_mech.o: m_mech.h
turret.o: turret.h
p_menus.o: menus.h g_local.h
persistent.o: persistent.h g_local.h
gloompmove_wrap.o: gloompmove.c g_local.h

%.o: %.c
	@echo -e \\nCompiling $(ARCH) "$<" 
	@$(CC) $(CFLAGS) -c $< -o $@
