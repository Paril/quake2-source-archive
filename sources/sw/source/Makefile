OBJS =  b_ai.o b_spawn.o g_ai.o g_cmds.o g_items.o g_misc.o g_npc_ai.o \
	g_save.o g_svcmds.o g_trigger.o g_utils.o g_weapon.o m_bird.o m_flash.o \
	m_jawa2.o m_map.o m_officer.o m_trooper.o m_vehicle.o n_male.o \
	n_officer.o n_rat.o n_sparrow.o p_client.o p_hud.o p_trail.o s_computer.o \
	fog.o g_chase.o g_combat.o g_func.o g_main.o g_monster.o g_phys.o \
	g_spawn.o g_target.o m_bystand.o m_jawa.o m_jawa3.o m_move.o \
	m_r2.o m_ske27.o m_utils.o n_gonk.o n_mse.o n_raptor.o n_smuggler.o p_cam.o \
	p_force.o p_menu.o p_view.o p_weapon.o q_shared.o s_conversation.o m_route.o g_ctf.o 

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

LIBS = -lz -ldllppc -lppcamiga -lmgl

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
