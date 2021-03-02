OBJS =  	g_ai.o g_cmds.o g_combat.o g_func.o g_items.o g_main.o g_misc.o \
	g_monster.o g_phys.o g_save.o g_spawn.o g_svcmds.o g_target.o \
	g_trigger.o g_utils.o g_weapon.o g_ctf.o g_chase.o p_menu.o \
	m_move.o \
	p_trail.o p_client.o p_hud.o \
	p_view.o p_weapon.o q_shared.o \
	k2_keys2.o k2_feign.o k2_weapons.o k2_hook.o k2_cycle.o k2_menu.o \
	gslog.o stdlog.o \
	k2_botutils.o \
	bot_ai.o bot_die.o bot_items.o bot_nav.o bot_spawn.o \
	bot_wpns.o camclient.o g_map_mod.o bot_misc.o debug.o

OBJSCPP = 

EXE = gameppc.elf

OTHER = 

CCM68K = gcc -V 2.95 -m68060 -fomit-frame-pointer -I/ade/include -s -noixemul

CC603 = ppc-amigaos-gcc -V 2.95.2 -mcpu=603e -mmultiple -warpup
CC604 = ppc-amigaos-gcc -V 2.95.2 -mcpu=604e -mmultiple -warpup
CC750 = ppc-amigaos-gcc -V 2.95.2 -mcpu=750 -mmultiple -warpup

CC = $(CC604)

CODEGEN = -D_inline="static __inline" -D__saveds= -D__int64="long long" -DMAX_PATH=255 -DFALSE=0 -DTRUE=1 -DNDEBUG 

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
