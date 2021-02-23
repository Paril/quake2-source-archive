######################################################
#
# $Id: Makefile,v 1.1 1998/01/13 08:19:26 phooky Exp $
#
# Makefile for gamei386.so
# 
# Author: Peter Andreasen
#
# questions? mail me at pandr@rhk.dk
# 
# Keep an eye on OpenQuake at http://www.openquake.org
#
######################################################

INSTALL_DIR=/home/quake2/tetris/

######################################################

CFLAGS= -fPIC -O2

OBJS= g_cmds.o g_main.o \
g_save.o g_spawn.o g_utils.o \
p_client.o p_hud.o p_view.o q_shared.o \
tet_game.o

all: gamei386.so install

gamei386.so:       $(OBJS)
	gcc -shared -Wl,-soname,gamei386.so -o gamei386.so $(OBJS)

install: gamei386.so
	strip gamei386.so
	cp gamei386.so $(INSTALL_DIR)

clean:
	rm -f *.o *.so

# deps.
# this is probably not all deps. allways do a make clean from
# time to time... :-)
#

# yes, this is a bit overkill
$(OBJS) : g_local.h q_shared.h game.h

g_cmds.o: m_player.h
p_client.o: m_player.h
p_view.o: m_player.h
p_weapon.o: m_player.h
q_shared.o: q_shared.h

#
#
# Changes I did to make the source compile under linux
# 
# these are outputs from diff 
#
# see description at top of this makefile
#
#
#===================================================================
#file: q_shared.c
#935c935
#< 	return stricmp (s1, s2);
#---
#> 	return strcasecmp (s1, s2);
#===================================================================
#file: g_local.h
#754c754
#< typedef struct gclient_s
#---
#> struct gclient_s
#828c828
#< } gclient_t;
#---
#> };
#===================================================================
#file: game.h
#4c4
#< #define	GAME_API_VERSION	1
#---
#> #define	GAME_API_VERSION	2
#===================================================================
#file: m_player.h
#205d204
#< 
#\ No newline at end of file
