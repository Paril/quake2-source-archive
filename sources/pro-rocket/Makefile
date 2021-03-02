OBJS =		g_ai.o g_combat.o g_items.o g_misc.o g_phys.o g_spawn.o g_target.o \
		g_turret.o g_weapon.o m_berserk.o m_boss3.o m_boss32.o m_chick.o m_flipper.o \
		m_flyer.o m_gunner.o m_infantry.o m_medic.o m_mutant.o m_soldier.o m_tank.o \
		p_hud.o p_view.o q_shared.o g_cmds.o g_func.o g_main.o g_monster.o g_save.o \
		g_svcmds.o g_trigger.o g_utils.o m_actor.o m_boss2.o m_boss31.o m_brain.o \
		m_flash.o m_float.o m_gladiator.o m_hover.o m_insane.o m_move.o m_parasite.o \
		m_supertank.o p_client.o p_trail.o p_weapon.o

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
