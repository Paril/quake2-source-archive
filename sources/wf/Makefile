######################################################################
# Makefile for WF for Quake2
#
# Note:   If you developed your source files on a Win32 machine, type
#         "make stripcr" to get rid of any stray carriage returns that
#         may be lurking in your files or make sure that you use ASCII
#         mode to transfer via FTP.
#
######################################################################


######################################################################
# C_OBJS (Custom Objects): You should use this group below to
# specify any custom files required by you that are not
# included in id's sources.  e.g. if your mod requires the addition
# of custom files "foo.c" and "bar.c", you'd add:
#
# C_OBJS = foo.o bar.o
#
######################################################################
C_OBJS = alarm.o \
b_biosentry.o \
b_healingdepot.o \
b_missile.o \
b_supplydepot.o \
b_turret.o \
bot_ai.o \
bot_die.o \
bot_items.o \
bot_misc.o \
bot_nav.o \
bot_spawn.o \
bot_wpns.o \
camclient.o \
debug.o \
dwm.o \
g_ai.o \
g_chase.o \
g_cmds.o \
g_combat.o \
g_ctf.o \
g_ent.o \
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
g_utils.o \
g_weapon.o \
grapple.o \
j_diesease.o \
j_fire.o \
j_kamikaze.o \
m_flash.o \
m_move.o \
p_client.o \
p_hud.o \
p_menu.o \
p_trail.o \
p_view.o \
p_weapon.o \
q_devels.o \
q_shared.o \
r_weap.o \
remotecam.o \
stdlog.o \
throwup.o \
w_ak47.o \
w_armordart.o \
w_boltedblaster.o \
w_cgprojectilelauncher.o \
w_clustermissiles.o \
w_concussion.o \
w_flamethrower.o \
w_flare.o \
w_flaregun.o \
w_gasgrenade.o \
w_infectdartlauncher.o \
w_lasercutter.o \
w_lasersight.o \
w_laserweapons.o \
w_lightninggun.o \
w_magnotron.o \
w_mbpc.o \
w_megachaingun.o \
w_nag.o \
w_nailgun.o \
w_napalm.o \
w_napalmmissiles.o \
w_needler.o \
w_other.o \
w_pelletmissile.o \
w_pistol.o \
w_plague.o \
w_plaguetime.o \
w_plasmabomb.o \
w_poisondart.o \
w_sentrykiller.o \
w_shc.o \
w_shock.o \
w_shrapnal.o \
w_slowgrenade.o \
w_sniperrifle.o \
w_stinger.o \
w_telsacoil.o \
w_tranquilizer.o \
wf_classmgr.o \
wf_cluster.o \
wf_config.o \
wf_decoy.o \
wf_earthquake.o \
wf_feign.o \
wf_fileio.o \
wf_flagcap.o \
wf_flame.o \
wf_flash.o \
wf_freezer.o \
wf_friends.o \
wf_goodyear.o \
wf_ipban.o \
wf_jet.o \
wf_knife.o \
wf_laserball.o \
wf_maplist.o \
wf_misc.o \
wf_napalm.o \
wf_pipebomb.o \
wf_proximity.o \
wf_quake.o \
wf_referee.o \
wf_scanner.o \
wf_tempents.o \
wf_turret.o \
x_weap.o

# g_unzip.o
# g_zip.o

# Game-related objects
G_OBJS =

# Monster-related objects
M_OBJS =

# Player-related objects
P_OBJS =

# Quake2-related objects
Q_OBJS =

# This will compile the original C_OBJS along with any extra files that
# you defined above
#
OBJS = $(C_OBJS) $(G_OBJS) $(M_OBJS) $(P_OBJS) $(Q_OBJS)

######################################################################
# UNCOMMENT BELOW LINES FOR WF DEPENDING ON OS
######################################################################

# Uncomment for Linux
TARGET = gamei386.so

# Uncomment for Solaris
#TARGET = gamesparc.so

CC = gcc
# CC = cc

SHELL = /bin/sh

# This #define saves us from having to change stricmp() to
# strcasecmp() in the actual sources, which means we can leave
# the source code as portable as possible (Win32 doesn't like
# strcasecmp() any more than Linux likes stricmp()).  By doing
# it this way the same source code can be built under both
# platforms unchanged.
#
BASE_CFLAGS = -Dstricmp=strcasecmp

# These build objects optimized for speed rather than size on Linux.
#CFLAGS = $(BASE_CFLAGS) -O2 -DC_ONLY -ffast-math -funroll-loops \
#	 -fomit-frame-pointer -fexpensive-optimizations -malign-loops=2 \
#	 -malign-jumps=2 -malign-functions=2 -fno-strength-reduce -Wall
#CFLAGS = $(BASE_CFLAGS) -O3 -DC_ONLY -ffast-math \
#	 -fomit-frame-pointer -fexpensive-optimizations -malign-loops=2 \
#	 -malign-jumps=2 -malign-functions=2 -fno-strength-reduce -Wall
#	-fverbose-asm -S -g

# This builds optimized for speed under Solaris
#CFLAGS = $(BASE_CFLAGS) -O2 -DC_ONLY -ffast-math -funroll-loops \
#	 -fomit-frame-pointer -fexpensive-optimizations \
#	 -fno-strength-reduce -Wall

# Use this for testing/debugging under Linux or Solaris
CFLAGS = $(BASE_CFLAGS) -O2 -g -DC_ONLY

# Linker flags for building a shared library (*.so).
#
# Redhat Linux users don't need -ldl or -lm...
LDFLAGS =

# but Slackware people do
#LDFLAGS = -ldl -lm

SHLIBCFLAGS = -fPIC
SHLIBLDFLAGS = -shared

######################################################################
# Targets
######################################################################

all: $(TARGET)

.c.o:
	$(CC) $(CFLAGS) $(SHLIBCFLAGS) -o $@ -c $<

$(TARGET):	$(OBJS)
		$(CC) $(CFLAGS) $(SHLIBLDFLAGS) -o $@ $(OBJS) $(LDFLAGS)
#		strip $(TARGET)

dep:
	@echo "Updating dependencies..."
	@$(CC) -MM $(OBJS:.o=.c) > .depend

stripcr:	.
		@echo "Stripping carriage returns from source files..."
	 	@for f in *.[ch]; do \
		  cat $$f | tr -d '\015' > .stripcr; \
		  mv .stripcr $$f; \
		done; \
		rm -f .stripcr

clean:
		@echo "Deleting WF temporary and compiled files..."
		@rm -f $(OBJS) *.orig ~* core *.so
		@echo "Restoring p_trail.o..."
		@cp p_trail.o.sav p_trail.o

distclean:	clean
		@echo "Deleting everything that can be rebuilt..."
		@rm -f $(TARGET) .depend
