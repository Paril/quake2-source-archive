
#include "g_local.h"


void	Svcmd_Test_f (void)
{
	gi.cprintf (NULL, PRINT_HIGH, "Svcmd_Test_f()\n");
}

//DEADLODE
// Calculate memory usage by major stucts
void Svcmd_Memory(void)
{
	int	cur, total;

	total = 0;
	gi.dprintf("\nMemory usage:\n");
	// Edicts
	total += (cur = sizeof(edict_t)*game.maxentities);
	gi.dprintf("Entities:  %10d\n", cur);
	// Weapons
	total += (cur = sizeof(weap_t)*MAX_WEAPONS);
	gi.dprintf("Weapons:   %10d\n", cur);
	// Ammo
	total += (cur = sizeof(ammo_t)*MAX_MUNITIONS);
	gi.dprintf("Variants:  %10d\n", cur);
	// Effects
	total += (cur = sizeof(effect_t)*MAX_EFFECTS);
	gi.dprintf("Effects:   %10d\n", cur);
	// Buildings
	total += (cur = sizeof(building_t)*MAX_BUILDINGS);
	gi.dprintf("Buildings: %10d\n", cur);
	// Supplies
	total += (cur = sizeof(supply_t)*MAX_SUPPLIES);
	gi.dprintf("Supplies:  %10d\n", cur);

	// Total
	gi.dprintf("-----\nTotal:     %10d bytes\n\n", total);
}

/*
=================
ServerCommand

ServerCommand will be called when an "sv" command is issued.
The game can issue gi.argc() / gi.argv() commands to get the rest
of the parameters
=================
*/
void	ServerCommand (void)
{
	char	*cmd;

	cmd = gi.argv(1);
	if (DL_strcmp (cmd, "test", -1, false) == 0)
		Svcmd_Test_f ();
	else if (DL_strcmp(cmd, "reinit", -1, false) == 0) {
		DeadLodeInit(false);
	}
	else if (DL_strcmp(cmd, "memory", -1, false) == 0)
		Svcmd_Memory();
	else
		gi.cprintf (NULL, PRINT_HIGH, "Unknown server command \"%s\"\n", cmd);
}

