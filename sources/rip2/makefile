OBJS =  g_ai.o g_chase.o g_cmds.o g_combat.o g_ctf.o g_fileio.o g_func.o \
	g_items.o g_main.o g_misc.o g_monster.o g_phys.o g_save.o \
	g_spawn.o g_svcmds.o g_target.o g_trigger.o g_turret.o g_utils.o \
	g_weapon.o m_flash.o ini.o m_move.o p_client.o p_hud.o p_trail.o p_view.o \
	p_weapon.o qmenu.o q_devels.o q_shared.o s_build.o s_cam.o s_class.o \
	s_laser.o s_main.o s_maps.o s_misc.o s_ptele.o \
	s_spells.o s_system.o s_tele.o s_weapons.o x_fbomb.o x_fire.o x_radio.o

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
