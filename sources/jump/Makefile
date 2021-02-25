# makefile for jump

ARCH=i386
CC=gcc
BASE_CFLAGS=-Dstricmp=strcasecmp -DQ_strncasecmp=strncasecmp

CFLAGS=$(BASE_CFLAGS) $(SHLIBCFLAGS) -O3 -march=i586 -malign-loops=2 -malign-jumps=2 -malign-functions=2 -fno-caller-saves -Wall

#gcc -nostdlib files... `gcc -print-libgcc-file-name`
SHLIBEXT=so
SHLIBLDFLAGS=-lm -shared #-static-libgcc #`gcc -print-libgcc-file-name` #-ldl

GAME_OBJS = \
g_chase.o \
g_cmds.o \
g_combat.o \
g_func.o \
g_items.o \
g_main.o \
g_misc.o \
g_phys.o \
g_save.o \
g_spawn.o \
g_svcmds.o \
g_target.o \
g_trigger.o \
g_utils.o \
p_client.o \
p_hud.o \
p_view.o \
q_shared.o

all: game

game : $(GAME_OBJS)
	$(CC) $(CFLAGS) -o game$(ARCH).$(SHLIBEXT) $(SHLIBCFLAGS) $(GAME_OBJS) $(SHLIBLDFLAGS)
	ls -l game$(ARCH).$(SHLIBEXT)
	strip game$(ARCH).$(SHLIBEXT)
	ls -l game$(ARCH).$(SHLIBEXT)

dist :
	bzip2 -k -9 game$(ARCH).$(SHLIBEXT)
	ls -l game$(ARCH).$(SHLIBEXT).bz2

#############################################################################
# MISC
#############################################################################

clean:
	-rm -f $(GAME_OBJS) game$(ARCH).$(SHLIBEXT)

