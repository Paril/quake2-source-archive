OBJS =  a_cmds.o a_game.o a_menu.o a_team.o g_ai.o g_cmds.o g_func.o \
				g_main.o g_monster.o g_save.o g_svcmds.o g_trigger.o g_utils.o \
        m_move.o p_hud.o p_view.o q_shared.o a_doorkick.o a_items.o a_radio.o \
        cgf_sfx_glass.o g_chase.o g_combat.o g_items.o g_misc.o g_phys.o \
        g_spawn.o g_target.o g_turret.o g_weapon.o p_client.o p_trail.o \
        p_weapon.o acesrc/acebot_ai.o acesrc/acebot_compress.o acesrc/acebot_movement.o \
	acesrc/acebot_spawn.o acesrc/acebot_cmds.o acesrc/acebot_items.o acesrc/acebot_nodes.o \
	acesrc/botnav.o acesrc/botchat.o acesrc/botscan.o  acesrc/fogstub.o

OBJSCPP = 

EXE = gameppc.elf

OTHER = 

CCM68K = gcc -V 2.95 -m68060 -fomit-frame-pointer -I/ade/include -s -noixemul

CC603 = ppc-amigaos-gcc -V 2.95.2 -mcpu=603e -mmultiple -warpup
CC604 = ppc-amigaos-gcc -V 2.95.2 -mcpu=604e -mmultiple -warpup
CC750 = ppc-amigaos-gcc -V 2.95.2 -mcpu=750 -mmultiple -warpup

CC = $(CC604)

CODEGEN = -Iacesrc -I../src -D_inline="static __inline" -D__saveds= -D__int64="long long" -DMAX_PATH=255 -DFALSE=0 -DTRUE=1 -DNDEBUG 

OPTIMIZE = -O3

CFLAGS= $(CODEGEN) $(OPTIMIZE)

LIBS = -lz -ldllppc -lppcamiga

%.o : %.c
	$(CC) $(CFLAGS) -S $< -o $*.s
	$(CC) $(CFLAGS) -c $*.s -o $*.o

%.o : %.cpp
	$(CC) $(CFLAGS) -fno-exceptions -fno-rtti -S $< -o $*.s
	$(CC) $(CFLAGS) -c $*.s -o $*.o

ASMS = $(OBJS:.o=.s) $(OBJSCPP:.o=.s)

$(EXE) : $(OBJS) $(OBJSCPP)
	$(CC) $(OBJS) $(OBJSCPP) $(LIBS) -o $@
	elf2exe2 $@ gameppc.dll

depend : 
	$(CC) $(CFLAGS) -MM $(OBJS:.o=.c) $(OBJSCPP:.o=.cpp) >depend

clean :
	rm $(EXE) $(OBJS) $(OBJSCPP) $(ASMS) $(OTHER) depend

#DEPENDENCIES

#include depend
