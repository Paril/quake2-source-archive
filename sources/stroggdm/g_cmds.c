#include "g_local.h"
#include "m_player.h"

#define STALKER_ON_CEILING(ent)  ( ent->gravityVector[2] > 0 ? 1 : 0 )
//VectorSet(self->gravityVector, 0, 0, -1)

// controlling parameters
// controlling parameters
#define        LASER_TIME                                                             30
#define        CELLS_FOR_LASER                                                20
#define        LASER_DAMAGE                                                   100
#define        LASER_MOUNT_DAMAGE                                             50
#define        LASER_MOUNT_DAMAGE_RADIUS                              64

// In-built Quake2 routines
void           target_laser_use (edict_t *self, edict_t *other, edict_t *activator);
void           target_laser_think (edict_t *self);
void           target_laser_on (edict_t *self);
void           target_laser_off (edict_t *self);



extern void make_soldier_stand (edict_t *self);
extern void make_soldier_walk (edict_t *self);
extern void make_soldier_shoot (edict_t *self);

void SelMonster_FaceLocation (edict_t *ent)
{
	vec3_t	v;
	
	if (!ent->client->pers.selected)
		return;
	
	VectorSubtract (ent->s.origin, ent->client->pers.selected->s.origin, v);
	ent->client->pers.selected->ideal_yaw = vectoyaw(v);
	M_ChangeYaw (ent->client->pers.selected);
}

void SelMonster_GoToPlayer (edict_t *ent)
{
	vec3_t	v;
	if (!ent->client->pers.selected)
		return;
	if (!ent->client->pers.selected->goalentity)
	{
		ent->client->pers.selected->goalentity = ent;
		VectorSubtract (ent->client->pers.selected->goalentity->s.origin, ent->client->pers.selected->s.origin, v);
		ent->client->pers.selected->ideal_yaw = ent->client->pers.selected->s.angles[YAW] = vectoyaw(v);
		ent->client->pers.selected->monsterinfo.walk (ent->client->pers.selected);
		ent->client->pers.selected->target = NULL;
		safe_cprintf (ent, PRINT_HIGH, "Monster following (walk).\n");
		return;
	}
	else
	{
		ent->client->pers.selected->goalentity = NULL;
		ent->client->pers.selected->monsterinfo.stand (ent->client->pers.selected);
		ent->client->pers.selected->target = NULL;
		safe_cprintf (ent, PRINT_HIGH, "Monster not following.\n");
	}
	
}
void Cmd_SelMonster (edict_t *ent)
{
	vec3_t  start;
	vec3_t  forward;
	vec3_t  end;
	trace_t tr;
	
	VectorCopy(ent->s.origin, start);
	start[2] += ent->viewheight;
	AngleVectors(ent->client->v_angle, forward, NULL, NULL);
	VectorMA(start, 8192, forward, end);
	tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT);
	if ( tr.ent )
	{
		if (tr.ent->client)
			return;
		if ((ent->client->pers.selected) && (tr.ent != ent->client->pers.selected)) // If you have a selected monster, and the traced entity is not selected, don't.
			return;
		if (tr.ent->team_owner == ent->team_owner) // teamname check
		{
			if (tr.ent != ent->client->pers.selected) // If traced entity is not currently selected
			{
				ent->client->pers.selected = tr.ent; // select him
				ent->client->pers.selected->s.effects = EF_PENT;
				safe_cprintf (ent, PRINT_HIGH, "Monster selected!\n"); // do the print
			}
			else // if you have a selected
			{
				ent->client->pers.selected->s.effects = EF_QUAD;
				safe_cprintf (ent, PRINT_HIGH, "Monster de-selected!\n"); // do the print
				ent->client->pers.selected = NULL; // no more selected.
			}
		}
	}
}


void Cmd_MonStandGround (edict_t *ent)
{
	if (!ent->client->pers.selected)
		return;
	if (!(ent->client->pers.selected->monsterinfo.aiflags & AI_STAND_GROUND))
	{
		ent->client->pers.selected->monsterinfo.aiflags |= AI_STAND_GROUND;
		safe_cprintf (ent, PRINT_HIGH, "Monster standing ground.\n");
		return;
	}
	else
	{
		ent->client->pers.selected->monsterinfo.aiflags &= AI_STAND_GROUND;
		safe_cprintf (ent, PRINT_HIGH, "Monster not standing ground.\n");
		return;
	}
}

void Cmd_MonRemove (edict_t *ent)
{
	if (ent->client->pers.monster1)
	{
		G_FreeEdict (ent->client->pers.monster1);
		ent->client->pers.monster1 = NULL;
		ent->client->resp.spawned_monsters--;
	}
	if (ent->client->pers.monster2)
	{
		G_FreeEdict (ent->client->pers.monster2);
		ent->client->pers.monster2 = NULL;
		ent->client->resp.spawned_monsters--;
	}
	if (ent->client->pers.monster3)
	{
		G_FreeEdict (ent->client->pers.monster3);
		ent->client->pers.monster3 = NULL;
		ent->client->resp.spawned_monsters--;
	}
	if (ent->client->pers.monster4)
	{
		G_FreeEdict (ent->client->pers.monster4);
		ent->client->pers.monster4 = NULL;
		ent->client->resp.spawned_monsters--;
	}
	if (ent->client->pers.monster5)
	{
		G_FreeEdict (ent->client->pers.monster5);
		ent->client->pers.monster5 = NULL;
		ent->client->resp.spawned_monsters--;
	}
	if (ent->client->pers.monster6)
	{
		G_FreeEdict (ent->client->pers.monster6);
		ent->client->pers.monster6 = NULL;
		ent->client->resp.spawned_monsters--;
	}
	// Free the selected monster
	ent->client->pers.selected = NULL;
	safe_cprintf (ent, PRINT_HIGH, "All monsters removed.\n");
}




char *ClientTeam (edict_t *ent)
{
	char		*p;
	static char	value[512];
	
	value[0] = 0;
	
	if (!ent->client)
		return value;
	
	strcpy(value, Info_ValueForKey (ent->client->pers.userinfo, "skin"));
	p = strchr(value, '/');
	if (!p)
		return value;
	
	if ((int)(dmflags->value) & DF_MODELTEAMS)
	{
		*p = 0;
		return value;
	}
	
	// if ((int)(dmflags->value) & DF_SKINTEAMS)
	return ++p;
}

qboolean OnSameTeam (edict_t *ent1, edict_t *ent2)
{
	char	ent1Team [512];
	char	ent2Team [512];
	
	if (!((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS)))
		return false;
	
	strcpy (ent1Team, ClientTeam (ent1));
	strcpy (ent2Team, ClientTeam (ent2));
	
	if (strcmp(ent1Team, ent2Team) == 0)
		return true;
	return false;
}


void SelectNextItem (edict_t *ent, int itflags)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;
	
	cl = ent->client;
	
	
	//ZOID
	if (cl->menu) {
		PMenu_Next(ent);
		return;
	} else if (cl->chase_target) {
		ChaseNext(ent);
		return;
	}
	//ZOID
	
	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (cl->pers.selected_item + i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (!(it->flags & itflags))
			continue;
		
		cl->pers.selected_item = index;
		return;
	}
	
	cl->pers.selected_item = -1;
}

void SelectPrevItem (edict_t *ent, int itflags)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;
	
	cl = ent->client;
	
	//ZOID
	if (cl->menu) {
		PMenu_Prev(ent);
		return;
	} else if (cl->chase_target) {
		ChasePrev(ent);
		return;
	}
	//ZOID
	
	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (cl->pers.selected_item + MAX_ITEMS - i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (!(it->flags & itflags))
			continue;
		
		cl->pers.selected_item = index;
		return;
	}
	
	cl->pers.selected_item = -1;
}

void ValidateSelectedItem (edict_t *ent)
{
	gclient_t	*cl;
	
	cl = ent->client;
	
	if (cl->pers.inventory[cl->pers.selected_item])
		return;		// valid
	
	SelectNextItem (ent, -1);
}


//=================================================================================

/*
==================
Cmd_Give_f
Give items to a client
==================
*/
void Cmd_Give_f (edict_t *ent)
{
	char		*name;
	gitem_t		*it;
	int			index;
	int			i;
	qboolean	give_all;
	edict_t		*it_ent;
	
	if (deathmatch->value && !sv_cheats->value)
	{
		safe_cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}
	
	name = gi.args();
	
	if (Q_stricmp(name, "all") == 0)
		give_all = true;
	else
		give_all = false;
	
	if (give_all || Q_stricmp(gi.argv(1), "health") == 0)
	{
		if (gi.argc() == 3)
			ent->health = atoi(gi.argv(2));
		else
			ent->health = ent->max_health;
		if (!give_all)
			return;
	}
	
	if (give_all || Q_stricmp(name, "weapons") == 0)
	{
		for (i=0 ; i<game.num_items ; i++)
		{
			it = itemlist + i;
			if (!it->pickup)
				continue;
			if (!(it->flags & IT_WEAPON))
				continue;
			ent->client->pers.inventory[i] += 1;
		}
		if (!give_all)
			return;
	}
	
	if (give_all || Q_stricmp(name, "ammo") == 0)
	{
		for (i=0 ; i<game.num_items ; i++)
		{
			it = itemlist + i;
			if (!it->pickup)
				continue;
			if (!(it->flags & IT_AMMO))
				continue;
			Add_Ammo (ent, it, 1000);
		}
		if (!give_all)
			return;
	}
	
	if (give_all || Q_stricmp(name, "armor") == 0)
	{
		gitem_armor_t	*info;
		
		it = FindItem("Jacket Armor");
		ent->client->pers.inventory[ITEM_INDEX(it)] = 0;
		
		it = FindItem("Combat Armor");
		ent->client->pers.inventory[ITEM_INDEX(it)] = 0;
		
		it = FindItem("Body Armor");
		info = (gitem_armor_t *)it->info;
		ent->client->pers.inventory[ITEM_INDEX(it)] = info->max_count;
		
		if (!give_all)
			return;
	}
	
	if (give_all || Q_stricmp(name, "Power Shield") == 0)
	{
		it = FindItem("Power Shield");
		it_ent = G_Spawn();
		it_ent->classname = it->classname;
		SpawnItem (it_ent, it);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);
		
		if (!give_all)
			return;
	}
	
	if (give_all)
	{
		for (i=0 ; i<game.num_items ; i++)
		{
			it = itemlist + i;
			if (!it->pickup)
				continue;
			if (it->flags & (IT_ARMOR|IT_WEAPON|IT_AMMO))
				continue;
			ent->client->pers.inventory[i] = 1;
		}
		return;
	}
	
	it = FindItem (name);
	if (!it)
	{
		name = gi.argv(1);
		it = FindItem (name);
		if (!it)
		{
			safe_cprintf (ent, PRINT_HIGH, "unknown item\n");
			return;
		}
	}
	
	if (!it->pickup)
	{
		safe_cprintf (ent, PRINT_HIGH, "non-pickup item\n");
		return;
	}
	
	index = ITEM_INDEX(it);
	
	if (it->flags & IT_AMMO)
	{
		if (gi.argc() == 3)
			ent->client->pers.inventory[index] = atoi(gi.argv(2));
		else
			ent->client->pers.inventory[index] += it->quantity;
	}
	else
	{
		it_ent = G_Spawn();
		it_ent->classname = it->classname;
		SpawnItem (it_ent, it);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);
	}
}
					 
					 
/*
==================
Cmd_God_f
Sets client to godmode
argv(0) god
==================
*/
void Cmd_God_f (edict_t *ent)
{
	char	*msg;
	
	if (deathmatch->value && !sv_cheats->value)
	{
							 safe_cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
							 return;
	}
	
	ent->flags ^= FL_GODMODE;
	if (!(ent->flags & FL_GODMODE) )
							 msg = "godmode OFF\n";
	else
							 msg = "godmode ON\n";
	
	safe_cprintf (ent, PRINT_HIGH, msg);
}
					 
					 
/*
==================
Cmd_Notarget_f
Sets client to notarget
argv(0) notarget
==================
*/
void Cmd_Notarget_f (edict_t *ent)
{
	char	*msg;
	
	if (deathmatch->value && !sv_cheats->value)
	{
							 safe_cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
							 return;
	}
	
	ent->flags ^= FL_NOTARGET;
	if (!(ent->flags & FL_NOTARGET) )
							 msg = "notarget OFF\n";
	else
							 msg = "notarget ON\n";
	
	safe_cprintf (ent, PRINT_HIGH, msg);
}


/*
==================
Cmd_Noclip_f
argv(0) noclip
==================
*/
void Cmd_Noclip_f (edict_t *ent)
{
	char	*msg;
	
	if (deathmatch->value && !sv_cheats->value)
	{
		safe_cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}
	
	if (ent->movetype == MOVETYPE_NOCLIP)
	{
		ent->movetype = MOVETYPE_WALK;
		msg = "noclip OFF\n";
	}
	else
	{
		 ent->movetype = MOVETYPE_NOCLIP;
		 msg = "noclip ON\n";
	}
	
	safe_cprintf (ent, PRINT_HIGH, msg);
}
					 
					 
/*
==================
Cmd_Use_f
Use an inventory item
==================
*/
void Cmd_Use_f (edict_t *ent)
{
	int			index;
	gitem_t		*it;
	char		*s;
	
	s = gi.args();
	
	// Paril - Aliases
	// First Weapons
	if (!stricmp (s, "weapon 1") || !stricmp (s, "weapon1"))
	{
							 if (ent->client->resp.class == 1)
								 s = "Soldier Blaster";
							 else if (ent->client->resp.class == 2)
								 s = "Infantry Chaingun";
							 else if (ent->client->resp.class == 3)
								 s = "Tank Blaster";
							 else if (ent->client->resp.class == 4)
								 s = "Medic HyperBlaster";
							 else if (ent->client->resp.class == 5)
								 s = "Gunner Chaingun";
							 else if (ent->client->resp.class == 6)
								 s = "Gladiator Railgun";
							 else if (ent->client->resp.class == 7)
								 s = "SuperTank Chaingun";
							 else if (ent->client->resp.class == 8)
								 s = "Flyer HyperBlaster";
							 else if (ent->client->resp.class == 32)
								 s = "Fixbot Blaster";
							 else if (ent->client->resp.class == 9)
								 s = "Berserker Club";
							 else if (ent->client->resp.class == 10)
								 s = "Iron Maiden Rocket Launcher";
							 else if (ent->client->resp.class == 11)
								 s = "Boss2 Chainguns";
							 else if (ent->client->resp.class == 12)
								 s = "Brain Tentacles";
							 else if (ent->client->resp.class == 13)
								 s = "Mutant Claws";
							 else if (ent->client->resp.class == 41)
								 s = "Tank Blaster";
							 else if (ent->client->resp.class == 15)
								 s = "Icarus HyperBlaster";
							 else if (ent->client->resp.class == 16)
								 s = "Floater HyperBlaster";
							 else if (ent->client->resp.class == 17)
								 s = "Shark Bite";
							 else if (ent->client->resp.class == 18)
								 s = "Jorg Chainguns";
							 else if (ent->client->resp.class == 19)
								 s = "Makron Blaster";
							 else if (ent->client->resp.class == 20)
								 s = "Soldier HyperGun";
							 else if (ent->client->resp.class == 21)
								 s = "Gladiator Plasma";
							 else if (ent->client->resp.class == 22)
								 s = "BETA Brain Tentacles";
							 else if (ent->client->resp.class == 23)
								 s = "BETA Iron Maiden Rocket Launcher";
							 else if (ent->client->resp.class == 24)
								 s = "SuperTank Chaingun";
							 else if (ent->client->resp.class == 25)
								 s = "Gekk Spit";
							 else if (ent->client->resp.class == 26)
								 s = "Daedalus HyperBlaster";
							 else if (ent->client->resp.class == 27)
								 s = "Stalker Blaster";
							 else if (ent->client->resp.class == 28)
								 s = "Medic Commander HyperBlaster";
							 else if (ent->client->resp.class == 29)
								 s = "Carrier Chainguns";
							 else if (ent->client->resp.class == 30)
								 s = "Widow Blaster";
							 else if (ent->client->resp.class == 31)
								 s = "Black Widow Heat Beam";
							 else
							 {
								 safe_cprintf (ent, PRINT_HIGH, "Unknown Class, Ignoring Selection..\n");
								 s = "Unknown";
							 }
	}
	if (!stricmp (s, "weapon 2") || !stricmp (s, "weapon2"))
	{
							 if (ent->client->resp.class == 1)
								 s = "Soldier Shotgun";
							 else if (ent->client->resp.class == 2)
								 s = "Infantry Melee";
							 else if (ent->client->resp.class == 3)
								 s = "Tank Machinegun";
							 else if (ent->client->resp.class == 4)
								 s = "Medic HyperBlaster";
							 else if (ent->client->resp.class == 5)
								 s = "Gunner Grenade Launcher";
							 else if (ent->client->resp.class == 6)
								 s = "Gladiator Spike";
							 else if (ent->client->resp.class == 7)
								 s = "SuperTank Rocket Launcher";
							 else if (ent->client->resp.class == 8)
								 s = "Flyer Melee";
							 else if (ent->client->resp.class == 32)
								 s = "Fixbot Blaster";
							 else if (ent->client->resp.class == 9)
								 s = "Berserker Spike";
							 else if (ent->client->resp.class == 10)
								 s = "Iron Maiden Slash";
							 else if (ent->client->resp.class == 11)
								 s = "Boss2 Rocket Launcher";
							 else if (ent->client->resp.class == 12)
								 s = "Brain Claws";
							 else if (ent->client->resp.class == 13)
								 s = "Mutant Claws";
							 else if (ent->client->resp.class == 41)
								 s = "Tank Machinegun";
							 else if (ent->client->resp.class == 15)
								 s = "Icarus HyperBlaster";
							 else if (ent->client->resp.class == 16)
								 s = "Floater Clamp";
							 else if (ent->client->resp.class == 17)
								 s = "Shark Tail";
							 else if (ent->client->resp.class == 18)
								 s = "Jorg Chainguns";
							 else if (ent->client->resp.class == 19)
								 s = "Makron Railgun";
							 else if (ent->client->resp.class == 20)
								 s = "Soldier RipperGun";
							 else if (ent->client->resp.class == 21)
								 s = "Gladiator Spike";
							 else if (ent->client->resp.class == 22)
								 s = "Brain Claws";
							 else if (ent->client->resp.class == 23)
								 s = "Iron Maiden Slash";
							 else if (ent->client->resp.class == 24)
								 s = "SuperTank Rocket Launcher";
							 else if (ent->client->resp.class == 25)
								 s = "Gekk Claws";
							 else if (ent->client->resp.class == 26)
								 s = "Daedalus HyperBlaster";
							 else if (ent->client->resp.class == 27)
								 s = "Stalker Melee";
							 else if (ent->client->resp.class == 28)
								 s = "Medic Commander Spawner";
							 else if (ent->client->resp.class == 29)
								 s = "Carrier Grenade Launcher";
							 else if (ent->client->resp.class == 30)
								 s = "Widow Railgun";
							 else if (ent->client->resp.class == 31)
								 s = "Black Widow Disruptor";
							 else
							 {
								 safe_cprintf (ent, PRINT_HIGH, "Unknown Class, Ignoring Selection..\n");
								 s = "Unknown";
							 }
	}
	if (!stricmp (s, "weapon 3") || !stricmp (s, "weapon3"))
	{
							 if (ent->client->resp.class == 1)
								 s = "Soldier Machinegun";
							 else if (ent->client->resp.class == 2)
								 s = "Infantry Melee";
							 else if (ent->client->resp.class == 3)
								 s = "Tank Rocket Launcher";
							 else if (ent->client->resp.class == 4)
								 s = "Medic HyperBlaster";
							 else if (ent->client->resp.class == 5)
								 s = "Gunner Grenade Launcher";
							 else if (ent->client->resp.class == 6)
								 s = "Gladiator Spike";
							 else if (ent->client->resp.class == 7)
								 s = "SuperTank Rocket Launcher";
							 else if (ent->client->resp.class == 8)
								 s = "Flyer Kamikaze";
							 else if (ent->client->resp.class == 32)
								 s = "Fixbot Blaster";
							 else if (ent->client->resp.class == 9)
								 s = "Berserker Spike";
							 else if (ent->client->resp.class == 10)
								 s = "Iron Maiden Slash";
							 else if (ent->client->resp.class == 11)
								 s = "Boss2 Rocket Launcher";
							 else if (ent->client->resp.class == 12)
								 s = "Brain Claws";
							 else if (ent->client->resp.class == 13)
								 s = "Mutant Claws";
							 else if (ent->client->resp.class == 41)
								 s = "Tank Rocket Launcher";
							 else if (ent->client->resp.class == 15)
								 s = "Icarus HyperBlaster";
							 else if (ent->client->resp.class == 16)
								 s = "Floater Melee";
							 else if (ent->client->resp.class == 17)
								 s = "Shark Tail";
							 else if (ent->client->resp.class == 18)
								 s = "Jorg Chainguns";
							 else if (ent->client->resp.class == 19)
								 s = "Makron BFG";
							 else if (ent->client->resp.class == 20)
								 s = "Soldier LaserGun";
							 else if (ent->client->resp.class == 21)
								 s = "Gladiator Spike";
							 else if (ent->client->resp.class == 22)
								 s = "Brain Eye Lasers";
							 else if (ent->client->resp.class == 23)
								 s = "Iron Maiden Slash";
							 else if (ent->client->resp.class == 24)
								 s = "SuperTank Rocket Launcher";
							 else if (ent->client->resp.class == 25)
								 s = "Gekk Claws";
							 else if (ent->client->resp.class == 26)
								 s = "Daedalus HyperBlaster";
							 else if (ent->client->resp.class == 27)
								 s = "Stalker Melee";
							 else if (ent->client->resp.class == 28)
								 s = "Medic Commander HyperBlaster";
							 else if (ent->client->resp.class == 29)
								 s = "Carrier Railgun";
							 else if (ent->client->resp.class == 30)
								 s = "Widow Railgun";
							 else if (ent->client->resp.class == 31)
								 s = "Black Widow Tongs";
							 else
							 {
								 safe_cprintf (ent, PRINT_HIGH, "Unknown Class, Ignoring Selection..\n");
								 s = "Unknown";
							 }
	}
	
	it = FindItem (s);
	if (!it)
	{
							 safe_cprintf (ent, PRINT_HIGH, "Unknown Item: %s\n", s);
							 return;
	}
	if (!it->use)
	{
							 safe_cprintf (ent, PRINT_HIGH, "Item Is Not Usable.\n");
							 return;
	}
	index = ITEM_INDEX(it);
	if (!ent->client->pers.inventory[index])
	{
							 safe_cprintf (ent, PRINT_HIGH, "Out If Item: %s\n", s);
							 return;
	}
	
	it->use (ent, it);
}
										
/*
==================
Cmd_Drop_f
Drop an inventory item
==================
*/
void Cmd_Drop_f (edict_t *ent)
{
	int			index;
	gitem_t		*it;
	char		*s;

//ZOID--special case for tech powerups
	if (Q_stricmp(gi.args(), "tech") == 0 && (it = CTFWhat_Tech(ent)) != NULL) {
		it->drop (ent, it);
		return;
	}
//ZOID
	
	s = gi.args();
	it = FindItem (s);
	if (!it)
	{
		safe_cprintf (ent, PRINT_HIGH, "unknown item: %s\n", s);
		return;
	}
	if (!it->drop)
	{
		safe_cprintf (ent, PRINT_HIGH, "Item is not dropable.\n");
		return;
	}
	index = ITEM_INDEX(it);
	if (!ent->client->pers.inventory[index])
	{
		safe_cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
		return;
	}
	
	it->drop (ent, it);
}
					
/*
=================
Cmd_Thrust_f
MUCE:
To set jetpack on or off
=================
*/
void Cmd_Thrust_f (edict_t *ent)
{
	char    *string;
	
	string=gi.args();
	
	if (Q_stricmp ( string, "on") == 0)
	{
		ent->client->resp.thrusting=1;
		ent->client->resp.next_thrust_sound=0;
	}
	else
	{
		ent->client->resp.thrusting=0;
	}
}

/*
=================
Cmd_Inven_f
=================
*/
void Cmd_Inven_f (edict_t *ent)
{
	int			i;
	gclient_t	*cl;
	
	cl = ent->client;
	
	cl->showscores = false;
	cl->showhelp = false;
	
//ZOID
	if (ent->client->menu) {
		PMenu_CloseIt(ent);
		ent->client->update_chase = true;
		return;
	}
//ZOID

	if (cl->showinventory)
	{
		cl->showinventory = false;
		return;
	}

//ZOID
	if (ctf->value && cl->resp.ctf_team == CTF_NOTEAM) 
	{
		CTFOpenJoinMenu(ent);
		return;
	}
//ZOID

	// Paril
	if (ctf->value && cl->resp.ctf_team != CTF_NOTEAM && cl->resp.class == 0)
	{
		OpenClassMenu (ent, NULL);
		return;
	}
	// Paril
	
	cl->showinventory = true;
	
	gi.WriteByte (svc_inventory);
	for (i=0 ; i<MAX_ITEMS ; i++)
	{
		gi.WriteShort (cl->pers.inventory[i]);
	}
	gi.unicast (ent, true);
}

/*
=================
Cmd_InvUse_f
=================
*/
void Cmd_InvUse_f (edict_t *ent)
{
	gitem_t		*it;
	
	//ZOID
	if (ent->client->menu) {
		PMenu_Select(ent);
		return;
	}
	//ZOID
	
	ValidateSelectedItem (ent);
	
	if (ent->client->pers.selected_item == -1)
	{
		safe_cprintf (ent, PRINT_HIGH, "No item to use.\n");
		return;
	}
	
	it = &itemlist[ent->client->pers.selected_item];
	if (!it->use)
	{
		safe_cprintf (ent, PRINT_HIGH, "Item is not usable.\n");
		return;
	}
	it->use (ent, it);
}

/*
=================
Cmd_WeapPrev_f
=================
*/
void Cmd_WeapPrev_f (edict_t *ent)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;
	int			selected_weapon;
	
	cl = ent->client;
	
	if (!cl->pers.weapon)
		return;
	
	selected_weapon = ITEM_INDEX(cl->pers.weapon);
	
	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (selected_weapon + i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (! (it->flags & IT_WEAPON) )
			continue;
		it->use (ent, it);
		if (cl->pers.weapon == it)
			return;	// successful
	}
}

/*
=================
Cmd_WeapNext_f
=================
*/
void Cmd_WeapNext_f (edict_t *ent)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;
	int			selected_weapon;
	
	cl = ent->client;
	
	if (!cl->pers.weapon)
		return;
	
	selected_weapon = ITEM_INDEX(cl->pers.weapon);
	
	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (selected_weapon + MAX_ITEMS - i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (! (it->flags & IT_WEAPON) )
			continue;
		it->use (ent, it);
		if (cl->pers.weapon == it)
			return;	// successful
	}
}

//ZOID
/*
=================
Cmd_LastWeap_f
=================
*/
void Cmd_LastWeap_f (edict_t *ent)
{
	gclient_t	*cl;

	cl = ent->client;

	if (!cl->pers.weapon || !cl->pers.lastweapon)
		return;

	cl->pers.lastweapon->use (ent, cl->pers.lastweapon);
}
//ZOID

/*
=================
Cmd_WeapLast_f
=================
*/
void Cmd_WeapLast_f (edict_t *ent)
{
	gclient_t	*cl;
	int			index;
	gitem_t		*it;

	cl = ent->client;

	if (!cl->pers.weapon || !cl->pers.lastweapon)
		return;

	index = ITEM_INDEX(cl->pers.lastweapon);
	if (!cl->pers.inventory[index])
		return;
	it = &itemlist[index];
	if (!it->use)
		return;
	if (! (it->flags & IT_WEAPON) )
		return;
	it->use (ent, it);
}

/*
=================
Cmd_InvDrop_f
=================
*/
void Cmd_InvDrop_f (edict_t *ent)
{
	gitem_t		*it;
	
	ValidateSelectedItem (ent);
	
	if (ent->client->pers.selected_item == -1)
	{
		safe_cprintf (ent, PRINT_HIGH, "No item to drop.\n");
		return;
	}
	
	it = &itemlist[ent->client->pers.selected_item];
	if (!it->drop)
	{
		safe_cprintf (ent, PRINT_HIGH, "Item is not dropable.\n");
		return;
	}
	it->drop (ent, it);
}

/*
=================
Cmd_Kill_f
=================
*/
void Cmd_Kill_f (edict_t *ent)
{
//ZOID
	if (ent->solid == SOLID_NOT)
		return;
//ZOID

	//if (ent->client->pers.abilities.flyer_kaboom)
	//{
		//safe_cprintf (ent, PRINT_HIGH, "Sorry cheapy\n");
		//return;
	//}
	if((level.time - ent->client->respawn_time) < 5)
		return;
	ent->flags &= ~FL_GODMODE;
	ent->health = 0;
	meansOfDeath = MOD_SUICIDE;
	player_die (ent, ent, ent, 100000, vec3_origin);
}

/*
=================
Cmd_PutAway_f
=================
*/
void Cmd_PutAway_f (edict_t *ent)
{
	ent->client->showscores = false;
	ent->client->showhelp = false;
	ent->client->showinventory = false;
//ZOID
	if (ent->client->menu)
		PMenu_CloseIt(ent);
	ent->client->update_chase = true;
//ZOID
}


int PlayerSort (void const *a, void const *b)
{
	int		anum, bnum;
	
	anum = *(int *)a;
	bnum = *(int *)b;
	
	anum = game.clients[anum].ps.stats[STAT_FRAGS];
	bnum = game.clients[bnum].ps.stats[STAT_FRAGS];
	
	if (anum < bnum)
		return -1;
	if (anum > bnum)
		return 1;
	return 0;
}

/*
=================
Cmd_Players_f
=================
*/
void Cmd_Players_f (edict_t *ent)
{
	int		i;
	int		count;
	char	small[64];
	char	large[1280];
	int		index[256];
	
	count = 0;
	for (i = 0 ; i < maxclients->value ; i++)
		if (game.clients[i].pers.connected)
		{
			index[count] = i;
			count++;
		}
		
		// sort by frags
		qsort (index, count, sizeof(index[0]), PlayerSort);
		
		// print information
		large[0] = 0;
		
		for (i = 0 ; i < count ; i++)
		{
			Com_sprintf (small, sizeof(small), "%3i %s\n",
				game.clients[index[i]].ps.stats[STAT_FRAGS],
				game.clients[index[i]].pers.netname);
			if (strlen (small) + strlen(large) > sizeof(large) - 100 )
			{	// can't print all of them in one packet
				strcat (large, "...\n");
				break;
			}
			strcat (large, small);
		}
		
		safe_cprintf (ent, PRINT_HIGH, "%s\n%i players\n", large, count);
}
					
/*
=================
Cmd_Wave_f
=================
*/
void Cmd_Wave_f (edict_t *ent)
{
	int		i;
	
	i = atoi (gi.argv(1));
	
	// can't wave when ducked
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
		return;
	
	if (ent->client->anim_priority > ANIM_WAVE)
		return;
	
	ent->client->anim_priority = ANIM_WAVE;
	
	switch (i)
	{
	case 0:
		safe_cprintf (ent, PRINT_HIGH, "flipoff\n");
		ent->s.frame = FRAME_flip01-1;
		ent->client->anim_end = FRAME_flip12;
		break;
	case 1:
		safe_cprintf (ent, PRINT_HIGH, "salute\n");
		ent->s.frame = FRAME_salute01-1;
		ent->client->anim_end = FRAME_salute11;
		break;
	case 2:
		safe_cprintf (ent, PRINT_HIGH, "taunt\n");
		ent->s.frame = FRAME_taunt01-1;
		ent->client->anim_end = FRAME_taunt17;
		break;
	case 3:
		safe_cprintf (ent, PRINT_HIGH, "wave\n");
		ent->s.frame = FRAME_wave01-1;
		ent->client->anim_end = FRAME_wave11;
		break;
	case 4:
	default:
		safe_cprintf (ent, PRINT_HIGH, "point\n");
		ent->s.frame = FRAME_point01-1;
		ent->client->anim_end = FRAME_point12;
		break;
	}
}

/*
==================
Cmd_Say_f
==================
*/
void Cmd_Say_f (edict_t *ent, qboolean team, qboolean arg0)
{
	int		i, j;
	edict_t	*other;
	char	*p;
	char	text[2048];
	gclient_t *cl;
	char *monstername;
	char tmpbuf[20];
	char *teamc;

	_strtime(tmpbuf);

	if (ent->client->resp.ctf_team == 1)
		teamc = "[Red]";
	else if (ent->client->resp.ctf_team == 2)
		teamc = "[Blue]";
	else
		teamc = "";
	
	if (ent->client->resp.class == 1)
		monstername = "Soldier";
	else if (ent->client->resp.class == 2)
		monstername = "Enforcer";
	else if (ent->client->resp.class == 3)
		monstername = "Tank";
	else if (ent->client->resp.class == 4)
		monstername = "Medic";
	else if (ent->client->resp.class == 5)
		monstername = "Gunner";
	else if (ent->client->resp.class == 6)
		monstername = "Gladiator";
	else if (ent->client->resp.class == 7)
		monstername = "Supertank";
	else if (ent->client->resp.class == 8)
		monstername = "Flyer";
	else if (ent->client->resp.class == 9)
		monstername = "Berserk";
	else if (ent->client->resp.class == 10)
		monstername = "Iron Maiden";
	else if (ent->client->resp.class == 11)
		monstername = "Biggun";
	else if (ent->client->resp.class == 12)
		monstername = "Brain";
	else if (ent->client->resp.class == 13)
		monstername = "Mutant";
	else if (ent->client->resp.class == 41)
		monstername = "Tank Commander";
	else if (ent->client->resp.class == 15)
		monstername = "Icarus";
	else if (ent->client->resp.class == 16)
		monstername = "Technician";
	else if (ent->client->resp.class == 17)
		monstername = "Shark";
	else if (ent->client->resp.class == 18)
		monstername = "JORG";
	else if (ent->client->resp.class == 19)
		monstername = "Makron";
	else if (ent->client->resp.class == 20)
		monstername = "Beta Soldier";
	else if (ent->client->resp.class == 21)
		monstername = "Beta Gladiator";
	else if (ent->client->resp.class == 22)
		monstername = "Beta Brain";
	else if (ent->client->resp.class == 23)
		monstername = "Beta Iron Maiden";
	else if (ent->client->resp.class == 24)
		monstername = "Beta Supertank";
	else if (ent->client->resp.class == 25)
		monstername = "Gekk";
	else if (ent->client->resp.class == 26)
		monstername = "Daedalus";
	else if (ent->client->resp.class == 27)
		monstername = "Stalker";
	else if (ent->client->resp.class == 28)
		monstername = "Medic Commander";
	else if (ent->client->resp.class == 29)
		monstername = "Carrier";
	else if (ent->client->resp.class == 30)
		monstername = "Widow";
	else if (ent->client->resp.class == 31)
		monstername = "Black Widow";
	else if (ent->client->resp.class == 32)
		monstername = "Repair Bot";
	else if (ent->client->resp.class == 33)
		monstername = "Death Tank";
	else if (ent->client->resp.class == 34)
		monstername = "Laser Gladiator";
	else if (ent->client->resp.class == 35)
		monstername = "Flyer Hornet";
	else if (ent->client->resp.class == 36)
		monstername = "Stormtrooper Soldier";
	else if (ent->client->resp.class == 37)
		monstername = "Male Traitor";	
	else if (ent->client->resp.class == 38)
		monstername = "Female Traitor";	
	else if (ent->client->resp.class == 39)
		monstername = "Hypertank";
	else if (ent->client->resp.class == 40)
		monstername = "Parasite";
	
	if (gi.argc () < 2 && !arg0)
		return;
	
	if ((!((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS))) || (!teamplay_landtype->value) || (!teamplay_classes->value))
		team = false;
	
	if (team)
		Com_sprintf (text, sizeof(text), "%s[%s](%s): ",teamc, monstername, ent->client->pers.netname);
	else if (!ent->client->resp.class)
		Com_sprintf (text, sizeof(text), "%s[No Class]%s: ",teamc, ent->client->pers.netname);
	else
		Com_sprintf (text, sizeof(text), "%s[%s]%s: ",teamc, monstername, ent->client->pers.netname);
	
	if (arg0)
	{
		strcat (text, gi.argv(0));
		strcat (text, " ");
		strcat (text, gi.args());
	}
	else
	{
		p = gi.args();
		
		if (*p == '"')
		{
			p++;
			p[strlen(p)-1] = 0;
		}
		strcat(text, p);
	}
	
	// don't let text be too long for malicious reasons
	if (strlen(text) > 150)
		text[150] = 0;
	
	strcat(text, "\n");
	
	if (flood_msgs->value) {
		cl = ent->client;
		
		if (level.time < cl->flood_locktill) {
			safe_cprintf(ent, PRINT_HIGH, "You can't talk for %d more seconds\n",
				(int)(cl->flood_locktill - level.time));
			return;
		}
		i = cl->flood_whenhead - flood_msgs->value + 1;
		if (i < 0)
			i = (sizeof(cl->flood_when)/sizeof(cl->flood_when[0])) + i;
		if (cl->flood_when[i] && 
			level.time - cl->flood_when[i] < flood_persecond->value) {
			cl->flood_locktill = level.time + flood_waitdelay->value;
			safe_cprintf(ent, PRINT_CHAT, "Flood protection:  You can't talk for %d seconds.\n",
				(int)flood_waitdelay->value);
			return;
		}
		cl->flood_whenhead = (cl->flood_whenhead + 1) %
			(sizeof(cl->flood_when)/sizeof(cl->flood_when[0]));
		cl->flood_when[cl->flood_whenhead] = level.time;
	}
	
	if (dedicated->value)
		safe_cprintf(NULL, PRINT_CHAT, "%s", text);
	
	for (j = 1; j <= game.maxclients; j++)
	{
		other = &g_edicts[j];
		if (!other->inuse)
			continue;
		if (!other->client)
			continue;
		if (team)
		{
			if (!OnSameTeam(ent, other))
				continue;
		}
		safe_cprintf(other, PRINT_CHAT, "%s", text);
	}
	
	if (logging->value)
		fprintf (logged, "%s       (%s)\n", monstername, text, tmpbuf);
}

/*
==================
Cmd_Say_f
==================
*/
/*void Cmd_Whisper_f (edict_t *ent, qboolean team, qboolean arg0, char *scommand)
{
	int		i, j;
	edict_t	*other;
	char	*p;
	char *pe;
	char	text[2048];
	gclient_t *cl;
	char *monstername;
	char tmpbuf[20];
	char *teamc;

	_strtime(tmpbuf);

	if (ent->client->resp.ctf_team == 1)
		teamc = "[Red]";
	else if (ent->client->resp.ctf_team == 2)
		teamc = "[Blue]";
	else
		teamc = "";
	
	if (ent->client->resp.class == 1)
		monstername = "Soldier";
	else if (ent->client->resp.class == 2)
		monstername = "Enforcer";
	else if (ent->client->resp.class == 3)
		monstername = "Tank";
	else if (ent->client->resp.class == 4)
		monstername = "Medic";
	else if (ent->client->resp.class == 5)
		monstername = "Gunner";
	else if (ent->client->resp.class == 6)
		monstername = "Gladiator";
	else if (ent->client->resp.class == 7)
		monstername = "Supertank";
	else if (ent->client->resp.class == 8)
		monstername = "Flyer";
	else if (ent->client->resp.class == 9)
		monstername = "Berserk";
	else if (ent->client->resp.class == 10)
		monstername = "Iron Maiden";
	else if (ent->client->resp.class == 11)
		monstername = "Biggun";
	else if (ent->client->resp.class == 12)
		monstername = "Brain";
	else if (ent->client->resp.class == 13)
		monstername = "Mutant";
	else if (ent->client->resp.class == 41)
		monstername = "Tank Commander";
	else if (ent->client->resp.class == 15)
		monstername = "Icarus";
	else if (ent->client->resp.class == 16)
		monstername = "Technician";
	else if (ent->client->resp.class == 17)
		monstername = "Shark";
	else if (ent->client->resp.class == 18)
		monstername = "JORG";
	else if (ent->client->resp.class == 19)
		monstername = "Makron";
	else if (ent->client->resp.class == 20)
		monstername = "Beta Soldier";
	else if (ent->client->resp.class == 21)
		monstername = "Beta Gladiator";
	else if (ent->client->resp.class == 22)
		monstername = "Beta Brain";
	else if (ent->client->resp.class == 23)
		monstername = "Beta Iron Maiden";
	else if (ent->client->resp.class == 24)
		monstername = "Beta Supertank";
	else if (ent->client->resp.class == 25)
		monstername = "Gekk";
	else if (ent->client->resp.class == 26)
		monstername = "Daedalus";
	else if (ent->client->resp.class == 27)
		monstername = "Stalker";
	else if (ent->client->resp.class == 28)
		monstername = "Medic Commander";
	else if (ent->client->resp.class == 29)
		monstername = "Carrier";
	else if (ent->client->resp.class == 30)
		monstername = "Widow";
	else if (ent->client->resp.class == 31)
		monstername = "Black Widow";
	else if (ent->client->resp.class == 32)
		monstername = "Repair Bot";
	else if (ent->client->resp.class == 33)
		monstername = "Death Tank";
	else if (ent->client->resp.class == 34)
		monstername = "Laser Gladiator";
	else if (ent->client->resp.class == 35)
		monstername = "Flyer Hornet";
	else if (ent->client->resp.class == 36)
		monstername = "Stormtrooper Soldier";
	else if (ent->client->resp.class == 37)
		monstername = "Male Traitor";	
	else if (ent->client->resp.class == 38)
		monstername = "Female Traitor";	
	else if (ent->client->resp.class == 39)
		monstername = "Hypertank";
	else if (ent->client->resp.class == 40)
		monstername = "Parasite";
	
	if (gi.argc () < 2 && !arg0)
		return;
	
	if ((!((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS))) || (!teamplay_landtype->value) || (!teamplay_classes->value))
		team = false;
	
	if (team)
		Com_sprintf (text, sizeof(text), "(Whisper)%s[%s](%s): ",teamc, monstername, ent->client->pers.netname);
	else if (!ent->client->resp.class)
		Com_sprintf (text, sizeof(text), "(Whisper)%s[No Class]%s: ",teamc, ent->client->pers.netname);
	else
		Com_sprintf (text, sizeof(text), "(Whisper)%s[%s]%s: ",teamc, monstername, ent->client->pers.netname);
	
	if (arg0)
	{
		strcat (text, gi.argv(0));
		strcat (text, " ");
		strcat (text, gi.args());
	}
	else
	{
		int t;

		p = gi.args();

		t = strlen(p);
		//p[strlen(p)-1] = 0;
		strncpy (&pe, p+5, t);
		strcat (pe, "\0");
		
		if (*p == '"')
		{
			p++;
			p[strlen(p)-1] = 0;
		}
		strcat(text, p);
	}
	
	// don't let text be too long for malicious reasons
	if (strlen(text) > 150)
		text[150] = 0;
	
	strcat(text, "\n");
	
	if (flood_msgs->value) {
		cl = ent->client;
		
		if (level.time < cl->flood_locktill) {
			safe_cprintf(ent, PRINT_HIGH, "You can't talk for %d more seconds\n",
				(int)(cl->flood_locktill - level.time));
			return;
		}
		i = cl->flood_whenhead - flood_msgs->value + 1;
		if (i < 0)
			i = (sizeof(cl->flood_when)/sizeof(cl->flood_when[0])) + i;
		if (cl->flood_when[i] && 
			level.time - cl->flood_when[i] < flood_persecond->value) {
			cl->flood_locktill = level.time + flood_waitdelay->value;
			safe_cprintf(ent, PRINT_CHAT, "Flood protection:  You can't talk for %d seconds.\n",
				(int)flood_waitdelay->value);
			return;
		}
		cl->flood_whenhead = (cl->flood_whenhead + 1) %
			(sizeof(cl->flood_when)/sizeof(cl->flood_when[0]));
		cl->flood_when[cl->flood_whenhead] = level.time;
	}
	
	for (j = 1; j <= game.maxclients; j++)
	{
		char	userinfo[MAX_INFO_STRING];
		char	*owner;
		other = &g_edicts[j];
		if (!other->inuse)
			continue;
		if (!other->client)
			continue;
		
		memcpy (userinfo, other->client->pers.userinfo, sizeof(userinfo));
		
		owner = Info_ValueForKey (userinfo, "name");

		if (Q_stricmp(scommand, owner) == 1)
			continue;

		safe_cprintf(other, PRINT_CHAT, "%s", text);

		goto ret;
	}

	safe_cprintf (ent, PRINT_HIGH, "%s not found.\n", scommand);

ret:
	return;
}*/

void CheckEnteredPassword (edict_t *ent, qboolean team, qboolean arg0)
{
	int		i, j;
	edict_t	*other;
	char	*p;
	char	text[2048];
	gclient_t *cl;
	
	if (gi.argc () < 2 && !arg0)
		return;
	
	if (arg0)
	{
		strcat (text, gi.argv(0));
		strcat (text, " ");
		strcat (text, gi.args());
	}
	else
	{
		p = gi.args();
		
		if (*p == '"')
		{
			p++;
			p[strlen(p)-1] = 0;
		}
		//strncpy(text, p);
	}
	
	// don't let text be too long for malicious reasons
	if (strlen(text) > 150)
		text[150] = 0;
	
	//strcat(text, "\n");

	strcpy (ent->client->entered_password, p);
	Step2(ent);
}
					 
/*
======================================================================
SAY_TEAM
======================================================================
*/

// This array is in 'importance order', it indicates what items are
// more important when reporting their names.
/*
=================
findradius
Returns entities that have origins within a spherical area
findradius (origin, radius)
=================
*/
/*static edict_t *loc_findradius (edict_t *from, vec3_t org, float rad)
{
	vec3_t	eorg;
	int		j;
	
	if (!from)
							 from = g_edicts;
	else
							 from++;
	for ( ; from < &g_edicts[globals.num_edicts]; from++)
	{
							 if (!from->inuse)
								 continue;
							 
							 for (j=0 ; j<3 ; j++)
								 eorg[j] = org[j] - (from->s.origin[j] + (from->mins[j] + from->maxs[j])*0.5);
							 if (VectorLength(eorg) > rad)
								 continue;
							 return from;
	}
	
	return NULL;
}

struct {
	char *classname;
	int priority;
} loc_names[] = 
{
	{	"item_flag_team1",			1 },
	{	"item_flag_team2",			1 },
	{	"item_quad",				2 }, 
	{	"item_invulnerability",		2 },
	{	"weapon_bfg",				3 },
	{	"weapon_railgun",			4 },
	{	"weapon_rocketlauncher",	4 },
	{	"weapon_hyperblaster",		4 },
	{	"weapon_chaingun",			4 },
	{	"weapon_grenadelauncher",	4 },
	{	"weapon_machinegun",		4 },
	{	"weapon_supershotgun",		4 },
	{	"weapon_shotgun",			4 },
	{	"item_power_screen",		5 },
	{	"item_power_shield",		5 },
	{	"item_armor_body",			6 },
	{	"item_armor_combat",		6 },
	{	"item_armor_jacket",		6 },
	{	"item_silencer",			7 },
	{	"item_breather",			7 },
	{	"item_enviro",				7 },
	{	"item_adrenaline",			7 },
	{	"item_bandolier",			8 },
	{	"item_pack",				8 },
	{ NULL, 0 }
};

static void loc_buildboxpoints(vec3_t p[8], vec3_t org, vec3_t mins, vec3_t maxs)
{
	VectorAdd(org, mins, p[0]);
	VectorCopy(p[0], p[1]);
	p[1][0] -= mins[0];
	VectorCopy(p[0], p[2]);
	p[2][1] -= mins[1];
	VectorCopy(p[0], p[3]);
	p[3][0] -= mins[0];
	p[3][1] -= mins[1];
	VectorAdd(org, maxs, p[4]);
	VectorCopy(p[4], p[5]);
	p[5][0] -= maxs[0];
	VectorCopy(p[0], p[6]);
	p[6][1] -= maxs[1];
	VectorCopy(p[0], p[7]);
	p[7][0] -= maxs[0];
	p[7][1] -= maxs[1];
}

static qboolean loc_CanSee (edict_t *targ, edict_t *inflictor)
{
	trace_t	trace;
	vec3_t	targpoints[8];
	int i;
	vec3_t viewpoint;
	
	// bmodels need special checking because their origin is 0,0,0
	if (targ->movetype == MOVETYPE_PUSH)
							 return false; // bmodels not supported
	
	loc_buildboxpoints(targpoints, targ->s.origin, targ->mins, targ->maxs);
	
	VectorCopy(inflictor->s.origin, viewpoint);
	viewpoint[2] += inflictor->viewheight;
	
	for (i = 0; i < 8; i++) {
							 trace = gi.trace (viewpoint, vec3_origin, vec3_origin, targpoints[i], inflictor, MASK_SOLID);
							 if (trace.fraction == 1.0)
								 return true;
	}
	
	return false;
}

static void CTFSay_Team_Location(edict_t *who, char *buf)
{
	edict_t *what = NULL;
	edict_t *hot = NULL;
	float hotdist = 999999, newdist;
	vec3_t v;
	int hotindex = 999;
	int i;
	gitem_t *item;
	int nearteam = -1;
	qboolean hotsee = false;
	qboolean cansee;
	
	while ((what = loc_findradius(what, who->s.origin, 1024)) != NULL) {
							 // find what in loc_classnames
							 for (i = 0; loc_names[i].classname; i++)
								 if (strcmp(what->classname, loc_names[i].classname) == 0)
									 break;
								 if (!loc_names[i].classname)
									 continue;
								 // something we can see get priority over something we can't
								 cansee = loc_CanSee(what, who);
								 if (cansee && !hotsee) {
									 hotsee = true;
									 hotindex = loc_names[i].priority;
									 hot = what;
									 VectorSubtract(what->s.origin, who->s.origin, v);
									 hotdist = VectorLength(v);
									 continue;
								 }
								 // if we can't see this, but we have something we can see, skip it
								 if (hotsee && !cansee)
									 continue;
								 if (hotsee && hotindex < loc_names[i].priority)
									 continue;
								 VectorSubtract(what->s.origin, who->s.origin, v);
								 newdist = VectorLength(v);
								 if (newdist < hotdist || 
									 (cansee && loc_names[i].priority < hotindex)) {
									 hot = what;
									 hotdist = newdist;
									 hotindex = i;
									 hotsee = loc_CanSee(hot, who);
								 }
	}
	
	if (!hot) {
							 strcpy(buf, "nowhere");
							 return;
	}
	
	// we now have the closest item
	// see if there's more than one in the map, if so
	// we need to determine what team is closest
	what = NULL;
	while ((what = G_Find(what, FOFS(classname), hot->classname)) != NULL) {
							 if (what == hot)
								 continue;
							 // if we are here, there is more than one, find out if hot
							 // is closer to red flag or blue flag
	}
	
	if ((item = FindItemByClassname(hot->classname)) == NULL) {
							 strcpy(buf, "nowhere");
							 return;
	}
	
	// in water?
	if (who->waterlevel)
							 strcpy(buf, "in the water ");
	else
							 *buf = 0;
	
	// near or above
	VectorSubtract(who->s.origin, hot->s.origin, v);
	if (fabs(v[2]) > fabs(v[0]) && fabs(v[2]) > fabs(v[1]))
							 if (v[2] > 0)
								 strcat(buf, "above ");
							 else
								 strcat(buf, "below ");
							 else
								 strcat(buf, "near ");
							 
							 //	if (nearteam == CTF_TEAM1)
							 //		strcat(buf, "the red ");
							 //	else if (nearteam == CTF_TEAM2)
							 //		strcat(buf, "the blue ");
							 //	else
							 strcat(buf, "the ");
							 
							 strcat(buf, item->pickup_name);
}
					
static void CTFSay_Team_Armor(edict_t *who, char *buf)
{
	gitem_t		*item;
	int			index, cells;
	int			power_armor_type;
	
	*buf = 0;
	
	power_armor_type = PowerArmorType (who);
	if (power_armor_type)
	{
		cells = who->client->pers.inventory[ITEM_INDEX(FindItem ("cells"))];
		if (cells)
			sprintf(buf+strlen(buf), "%s with %i cells ",
			(power_armor_type == POWER_ARMOR_SCREEN) ?
			"Power Screen" : "Power Shield", cells);
	}
	
	index = ArmorIndex (who);
	if (index)
	{
		item = GetItemByIndex (index);
		if (item) {
			if (*buf)
				strcat(buf, "and ");
			sprintf(buf+strlen(buf), "%i units of %s",
				who->client->pers.inventory[index], item->pickup_name);
		}
	}
	
	if (!*buf)
		strcpy(buf, "no armor");
}

static void CTFSay_Team_Health(edict_t *who, char *buf)
{
	if (who->health <= 0)
		strcpy(buf, "dead");
	else
		sprintf(buf, "%i health", who->health);
}

static void CTFSay_Team_Weapon(edict_t *who, char *buf)
{
	if (who->client->pers.weapon)
		strcpy(buf, who->client->pers.weapon->pickup_name);
	else
		strcpy(buf, "none");
}

static void CTFSay_Team_Sight(edict_t *who, char *buf)
{
	int i;
	edict_t *targ;
	int n = 0;
	char s[1024];
	char s2[1024];
	
	*s = *s2 = 0;
	for (i = 1; i <= maxclients->value; i++) {
		targ = g_edicts + i;
		if (!targ->inuse || 
			targ == who ||
			!loc_CanSee(targ, who))
			continue;
		if (*s2) {
			if (strlen(s) + strlen(s2) + 3 < sizeof(s)) {
				if (n)
					strcat(s, ", ");
				strcat(s, s2);
				*s2 = 0;
			}
			n++;
		}
		strcpy(s2, targ->client->pers.netname);
	}
	if (*s2) {
		if (strlen(s) + strlen(s2) + 6 < sizeof(s)) {
			if (n)
				strcat(s, " and ");
			strcat(s, s2);
		}
		strcpy(buf, s);
	} else
		strcpy(buf, "no one");
}

void CTFSay_Team(edict_t *who, char *msg)
{
	char outmsg[256];
	char buf[256];
	int i;
	char *p;
	edict_t *cl_ent;
	char *monstername;
	edict_t *ent;
	
	ent = who;
	
	if (ent->client->resp.class == 1)
		monstername = "Soldier";
	else if (ent->client->resp.class == 2)
		monstername = "Enforcer";
	else if (ent->client->resp.class == 3)
		monstername = "Tank";
	else if (ent->client->resp.class == 4)
		monstername = "Medic";
	else if (ent->client->resp.class == 5)
		monstername = "Gunner";
	else if (ent->client->resp.class == 6)
		monstername = "Gladiator";
	else if (ent->client->resp.class == 7)
		monstername = "Supertank";
	else if (ent->client->resp.class == 8)
		monstername = "Flyer";
	else if (ent->client->resp.class == 9)
		monstername = "Berserk";
	else if (ent->client->resp.class == 10)
		monstername = "Iron Maiden";
	else if (ent->client->resp.class == 11)
		monstername = "Biggun";
	else if (ent->client->resp.class == 12)
		monstername = "Brain";
	else if (ent->client->resp.class == 13)
		monstername = "Mutant";
	else if (ent->client->resp.class == 41)
		monstername = "Tank Commander";
	else if (ent->client->resp.class == 15)
		monstername = "Icarus";
	else if (ent->client->resp.class == 16)
		monstername = "Technician";
	else if (ent->client->resp.class == 17)
		monstername = "Shark";
	else if (ent->client->resp.class == 18)
		monstername = "JORG";
	else if (ent->client->resp.class == 19)
		monstername = "Makron";
	else if (ent->client->resp.class == 20)
		monstername = "Beta Soldier";
	else if (ent->client->resp.class == 21)
		monstername = "Beta Gladiator";
	else if (ent->client->resp.class == 22)
		monstername = "Beta Brain";
	else if (ent->client->resp.class == 23)
		monstername = "Beta Iron Maiden";
	else if (ent->client->resp.class == 24)
		monstername = "Beta Supertank";
	else if (ent->client->resp.class == 25)
		monstername = "Gekk";
	else if (ent->client->resp.class == 26)
		monstername = "Daedalus";
	else if (ent->client->resp.class == 27)
		monstername = "Stalker";
	else if (ent->client->resp.class == 28)
		monstername = "Medic Commander";
	else if (ent->client->resp.class == 29)
		monstername = "Carrier";
	else if (ent->client->resp.class == 30)
		monstername = "Widow";
	else if (ent->client->resp.class == 31)
		monstername = "Black Widow";
	else if (ent->client->resp.class == 32)
		monstername = "Repair Bot";
	else if (ent->client->resp.class == 33)
		monstername = "Death Tank";
	else if (ent->client->resp.class == 34)
		monstername = "Laser Gladiator";
	else if (ent->client->resp.class == 35)
		monstername = "Flyer Hornet";
	else if (ent->client->resp.class == 36)
		monstername = "Stormtrooper Soldier";
	else if (ent->client->resp.class == 37)
		monstername = "Male Traitor";	
	
	if (!teamplay_landtype->value)
		return;
	
	outmsg[0] = 0;
	
	if (*msg == '\"') {
		msg[strlen(msg) - 1] = 0;
		msg++;
	}
	
	for (p = outmsg; *msg && (p - outmsg) < sizeof(outmsg) - 2; msg++) {
		if (*msg == '%') {
			switch (*++msg) {
			case 'l' :
			case 'L' :
				CTFSay_Team_Location(who, buf);
				if (strlen(buf) + (p - outmsg) < sizeof(outmsg) - 2) {
					strcpy(p, buf);
					p += strlen(buf);
				}
				break;
			case 'a' :
			case 'A' :
				CTFSay_Team_Armor(who, buf);
				if (strlen(buf) + (p - outmsg) < sizeof(outmsg) - 2) {
					strcpy(p, buf);
					p += strlen(buf);
				}
				break;
			case 'h' :
			case 'H' :
				CTFSay_Team_Health(who, buf);
				if (strlen(buf) + (p - outmsg) < sizeof(outmsg) - 2) {
					strcpy(p, buf);
					p += strlen(buf);
				}
				break;
			case 'w' :
			case 'W' :
				CTFSay_Team_Weapon(who, buf);
				if (strlen(buf) + (p - outmsg) < sizeof(outmsg) - 2) {
					strcpy(p, buf);
					p += strlen(buf);
				}
				break;
				
			case 'n' :
			case 'N' :
				CTFSay_Team_Sight(who, buf);
				if (strlen(buf) + (p - outmsg) < sizeof(outmsg) - 2) {
					strcpy(p, buf);
					p += strlen(buf);
				}
				break;
				
			default :
				*p++ = *msg;
			}
		} else
			*p++ = *msg;
	}
	*p = 0;
	
	for (i = 0; i < maxclients->value; i++) {
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;
		if (cl_ent->client->resp.landtype == who->client->resp.landtype)
			safe_cprintf(cl_ent, PRINT_CHAT, "[%s](%s): %s\n", 
			monstername, who->client->pers.netname, outmsg);
	}
}*/
					
void Cmd_PlayerList_f(edict_t *ent)
{
	int i;
	char st[80];
	char text[1400];
	edict_t *e2;
	
	// connect time, ping, score, name
	*text = 0;
	for (i = 0, e2 = g_edicts + 1; i < maxclients->value; i++, e2++) {
		if (!e2->inuse)
			continue;
		
		Com_sprintf(st, sizeof(st), "%02d:%02d %4d %3d %s%s\n",
			(level.framenum - e2->client->resp.enterframe) / 600,
			((level.framenum - e2->client->resp.enterframe) % 600)/10,
			e2->client->ping,
			e2->client->resp.score,
			e2->client->pers.netname,
			e2->client->resp.spectator ? " (spectator)" : "");
		if (strlen(text) + strlen(st) > sizeof(text) - 50) {
			sprintf(text+strlen(text), "And more...\n");
			safe_cprintf(ent, PRINT_HIGH, "%s", text);
			return;
		}
		strcat(text, st);
	}
	safe_cprintf(ent, PRINT_HIGH, "%s", text);
}
void     pre_target_laser_think (edict_t *self)
{
	target_laser_on (self);
	
	self->think = target_laser_think;
}
					
void PlaceLaser (edict_t *ent)
{
	edict_t *self, *grenade;
	
	vec3_t forward, wallp;
	
	trace_t tr;
	
	
	int            laser_colour[] = {
		0xf2f2f0f0,            // red
			0xd0d1d2d3,            // green
			0xf3f3f1f1,            // blue
			0xdcdddedf,            // yellow
			0xe0e1e2e3             // bitty yellow strobe
	};
	
	
	// valid ent ?
	if ((!ent->client) || (ent->health<=0))
		return;
	
	// Setup "little look" to close wall
	VectorCopy(ent->s.origin,wallp);         
	
	// Cast along view angle
	AngleVectors (ent->client->v_angle, forward, NULL, NULL);
	
	// Setup end point
	wallp[0]=ent->s.origin[0]+forward[0]*50;
	wallp[1]=ent->s.origin[1]+forward[1]*50;
	wallp[2]=ent->s.origin[2]+forward[2]*50;  
	
	// trace
	tr = gi.trace (ent->s.origin, NULL, NULL, wallp, ent, MASK_SOLID);
	
	// Line complete ? (ie. no collision)
	if (tr.fraction == 1.0)
	{
		safe_cprintf (ent, PRINT_HIGH, "Too far from wall.\n");
		return;
	}
	
	// Hit sky ?
	if (tr.surface)
	{
		if (tr.surface->flags & SURF_SKY)
			return;
	}
	
	// Ok, lets stick one on then ...
	safe_cprintf (ent, PRINT_HIGH, "Laser attached.\n");
	gi.sound (ent, CHAN_AUTO, gi.soundindex ("world/fusein.wav"), 1, ATTN_NORM, 0);
	// -----------
	// Setup laser
	// -----------
	self = G_Spawn();
	
	
	self -> movetype               = MOVETYPE_NONE;
	self -> solid                  = SOLID_NOT;
	self -> s.renderfx             = RF_BEAM|RF_TRANSLUCENT;
	self -> s.modelindex   = 1;                   // must be non-zero
	self -> s.sound                = gi.soundindex ("world/laser.wav");
	self -> classname              = "laser_yaya";
	self -> s.frame                = 2;    // beam diameter
	self -> owner                  = self;
	self -> s.skinnum              = 0xf2f3f0f1;
	self -> dmg                           = LASER_DAMAGE;
	self -> think                  = pre_target_laser_think;
	self -> delay                  = level.time + 20;
	
	// Set orgin of laser to point of contact with wall
	VectorCopy(tr.endpos,self->s.origin);
	
	// convert normal at point of contact to laser angles
	vectoangles(tr.plane.normal,self -> s.angles);
	
	// setup laser movedir (projection of laser)
	G_SetMovedir (self->s.angles, self->movedir);
	
	VectorSet (self->mins, -8, -8, -8);
	VectorSet (self->maxs, 8, 8, 8);
	
	// link to world
	gi.linkentity (self);
	
	// start off ...
	target_laser_off (self);
	
	// ... but make automatically come on
	self -> nextthink = level.time + 2;
	grenade = G_Spawn();
	
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	VectorCopy (tr.endpos, grenade->s.origin);
	vectoangles(tr.plane.normal,grenade -> s.angles);
	
	grenade -> movetype            = MOVETYPE_NONE;
	grenade -> clipmask            = MASK_SHOT;
	grenade -> solid               = SOLID_NOT;
	grenade -> s.modelindex        = gi.modelindex ("models/objects/grenade2/tris.md2");
	grenade -> owner               = self;
	grenade -> nextthink   = level.time + 20;
	grenade -> think               = G_FreeEdict;
	grenade -> classname              = "laser_grenade";
	gi.linkentity (grenade);
	
	self->owner = grenade;
}
					
void Earthquake_Think(edict_t *quake) {
	int i;
	edict_t *ent; 
	
	// Make rumbling sounds..
	if (quake->wait < level.time) {
		gi.positioned_sound(quake->s.origin, quake, CHAN_AUTO, gi.soundindex("world/quake.wav"), 1.0, ATTN_NONE, 0);
		quake->wait = level.time + 0.5; } 
	
	// Shake around all clients in game..
	for(i=0;i < game.maxclients;i++) {
		ent=g_edicts+i+1;
		//if (!G_ClientInGame(ent)) continue;
		if (!ent->groundentity) continue; // Is ent currently airborne??
		ent->groundentity = NULL; // If not, then keep'em on the ground..
		ent->velocity[0] += crandom()*250;
		ent->velocity[1] += crandom()*150;
		ent->velocity[2] = 250*(100.0/ent->mass); } 
	
	// Time to stop shaking yet..
	if (level.time < quake->delay)
		quake->nextthink = level.time + 0.1;
	else {
		quake->nextthink = level.time + (2*60) + (60*(rand()%8));
		quake->delay=level.time + 5; 
	}
} 

void Init_Earthquake_Generator(void) {
	edict_t *quake;
	
	
	quake = G_Spawn();
	quake->classname = "EarthQuake";
	VectorClear(quake->s.origin);
	quake->svflags |= SVF_NOCLIENT;
	quake->wait=0;
	quake->think = Earthquake_Think; 
	
	// Random Earthquakes every 2..10 minutes
	quake->nextthink = level.time + 1; 
	
	// Duration of shaking 10..20 seconds..
	quake->delay=level.time + 5; 
	
	gi.linkentity(quake);
} 

void Secondary_Explosion(edict_t *dcharge) {
	vec3_t forward,backward,right,left,up;
	
	VectorSet(up,0,0,1);
	VectorSet(forward,0,1,0);
	VectorSet(right,1,0,0);
	VectorSet(backward,0,-1,0);
	VectorSet(left,-1,0,0);
	
	
	fire_bfg(dcharge, dcharge->s.origin, up, 25, 800,		100);
	fire_bfg(dcharge, dcharge->s.origin, forward, 25,	800, 100);
	fire_bfg(dcharge, dcharge->s.origin, right, 25,		800,	100);
	fire_bfg(dcharge, dcharge->s.origin, backward,25,	800, 100);
	fire_bfg(dcharge, dcharge->s.origin, left, 25, 800,		100);
	
	G_FreeEdict(dcharge);
}

void BFGthrow(edict_t *dcharge) {
	vec3_t forward,backward,right,left,up;
	
	VectorSet(up,0,0,1);
	VectorSet(forward,0,1,0);
	VectorSet(right,1,0,0);
	VectorSet(backward,0,-1,0);
	VectorSet(left,-1,0,0);
	
	
	fire_bfg(dcharge, dcharge->s.origin, right, 25, 800, 100);
}

/*
=================
Cmd_Airstrike_f
CCH: new function to call in airstrikes
=================
*/
void Cmd_Airstrike_f (edict_t *ent)
{
	vec3_t  start;
	vec3_t  forward;
	vec3_t  end;
	trace_t tr;
	
	// make sure an airstrike hasn't already been called
	if ( ent->client->airstrike_called )
	{
		safe_cprintf(ent, PRINT_HIGH, "The airstrike is already on its way.\n");
		return;
	}
	
	// make sure we're pointed at the sky
	VectorCopy(ent->s.origin, start);
	start[2] += ent->viewheight;
	AngleVectors(ent->client->v_angle, forward, NULL, NULL);
	VectorMA(start, 8192, forward, end);
	tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA);
	if ( tr.surface && !(tr.surface->flags & SURF_SKY) )
	{
		safe_cprintf(ent, PRINT_HIGH, "Airstrikes have to come through the sky!\n");
		return;
	}
	
	// set up for the airstrike
	VectorCopy(tr.endpos, ent->client->airstrike_entry);
	ent->client->airstrike_called = 1;
	ent->client->airstrike_time = level.time + 5;
	safe_cprintf(ent, PRINT_HIGH, "Airstrike en route, ETA 5 seconds. Light up target.\n");
}
					
// Laser Beam Color Codes
#define Laser_Red 0xf2f2f0f0 // bright red
#define Laser_Green 0xd0d1d2d3 // bright green
#define Laser_Blue 0xf3f3f1f1 // bright blue
#define Laser_Yellow 0xdcdddedf // bright yellow
#define Laser_YellowS 0xe0e1e2e3 // yellow strobe
#define Laser_DkPurple 0x80818283 // dark purple
#define Laser_LtBlue 0x70717273 // light blue
#define Laser_Green2 0x90919293 // different green
#define Laser_Purple 0xb0b1b2b3 // purple
#define Laser_Red2 0x40414243 // different red
#define Laser_Orange 0xe2e5e3e6 // orange
#define Laser_Mix 0xd0f1d3f3 // mixture
#define Laser_RedBlue 0xf2f3f0f1 // inner = red, outer = blue
#define Laser_BlueRed 0xf3f2f1f0 // inner = blue, outer = red
#define Laser_GreenY 0xdad0dcd2 // inner = green, outer = yellow
#define Laser_YellowG 0xd0dad2dc // inner = yellow, outer = green
//=======================================================
void abs_vector(vec3_t vec1, vec3_t vec2, vec3_t result){
	int j;
	
	for (j=0; j<3; j++)
		result[j]=abs(vec1[j]-vec2[j]);
}
//======================================================
void G_Spawn_Splash(int type, int count, int color, vec3_t start, vec3_t movdir, vec3_t origin ) {
	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(type);
	gi.WriteByte(count);
	gi.WritePosition(start);
	gi.WriteDir(movdir);
	gi.WriteByte(color);
	gi.multicast(origin, MULTICAST_PVS);
}
//=======================================================
void min_vector(vec3_t start, vec3_t end1, vec3_t end2, vec3_t result){
	vec3_t eorg1={0,0,0};
	vec3_t eorg2={0,0,0};
	
	abs_vector(start, end1, eorg1);
	abs_vector(start, end2, eorg2);
	
	if (VectorLength(eorg1) < VectorLength(eorg2))
		VectorCopy(end1, result);
	else
		VectorCopy(end2, result);
}

//=======================================================
void beam_laser_think(edict_t *beam) {
	vec3_t end;
	vec3_t zvec={0,0,0};
	trace_t tr;
	
	// Has the time expired?
	if (beam->wait < level.time) {
		gi.unlinkentity(beam);
		return; }
	
	VectorMA(beam->s.origin, beam->spawnflags, beam->movedir, end);
	
	tr=gi.trace(beam->s.origin, NULL, NULL, end, beam, MASK_ALL);
	
	// Sparks on top of Post.
	if (beam->spawnflags==55)
		G_Spawn_Splash(TE_LASER_SPARKS, 4, Laser_BlueRed, end, zvec, end);
	
	// Anybody step across these laser beam traces yet?
	if ((tr.ent) && (tr.ent->takedamage) && !(tr.ent->flags & FL_IMMUNE_LASER))
		T_Damage(tr.ent, beam, beam->activator, beam->movedir, tr.endpos, zvec, beam->dmg, 1, DAMAGE_ENERGY, MOD_TARGET_LASER);
	
	VectorCopy(tr.endpos, beam->s.old_origin);
	
	beam->nextthink = level.time + 0.1;
}

//=======================================================
// Spawns a 2 post fence with 4 electric laser rails.
//=======================================================
void Spawn_LaserFence(edict_t *ent) {
	edict_t *beam[6];
	int i;
	vec3_t post1, post2;
	vec3_t forward, up, right, end, torigin, torigin2, len;
	trace_t tr;
	
	VectorCopy(ent->s.origin, torigin);
	AngleVectors(ent->s.angles, forward, right, up);
	// Move torigin forward 50 units to end.
	VectorMA(torigin, 50, forward, end);
	// Trace this path to see what we hit.
	tr = gi.trace(torigin, NULL, NULL, end, NULL, MASK_SOLID);
	// torigin2 is the shorter of these vectors.
	min_vector(torigin, tr.endpos, end, torigin2);
	VectorClear(end);
	// Move left from torigin2 1/2 fence length to end.
	VectorMA(torigin2, -120/2, right, end);
	// Trace this path to see what we hit.
	tr = gi.trace(torigin2, NULL, NULL, end, NULL, MASK_SHOT);
	// Left post1 is the shorter of these vectors.
	min_vector(torigin2, tr.endpos, end, post1);
	VectorClear(end);
	// Move right from post1 entire length to end.
	VectorMA(post1, 120, right, end);
	// Trace this path to see what we hit.
	tr = gi.trace(torigin2, NULL, NULL, end, NULL, MASK_SHOT);
	// Right post2 is the shorter of these vectors.
	min_vector(torigin2, tr.endpos, end, post2);
	
	// Get length of fence.
	abs_vector(post1, post2, len);
	
	// Okay, let's make the damn beam already!!
	for (i=0; i<=5; i++) {
		beam[i] = G_Spawn();
		beam[i]->owner = beam[i];
		beam[i]->classname= "LaserFence";
		beam[i]->activator = ent; // Must set for frags!!
		beam[i]->movetype = MOVETYPE_NONE;
		beam[i]->solid = SOLID_NOT;
		beam[i]->svflags &= ~SVF_NOCLIENT;
		beam[i]->s.renderfx = RF_BEAM;
		beam[i]->s.modelindex = 1;
		beam[i]->spawnflags = VectorLength(len); // Store Length here..
		beam[i]->s.skinnum = Laser_Red;
		if (i==0) // Only need sound for first one..
			beam[i]->s.sound = gi.soundindex ("world/laser.wav");
		else
			beam[i]->s.sound=0;
		beam[i]->s.frame=2;
		VectorSet(beam[i]->mins,-8,-8,-8);
		VectorSet(beam[i]->maxs, 8, 8, 8);
		beam[i]->dmg = 25; // Each Rail..
		VectorClear(beam[i]->s.angles);
		VectorCopy(right, beam[i]->movedir);
		VectorCopy(post1, beam[i]->s.origin);
		VectorCopy(post1, beam[i]->s.old_origin);
		
		switch (i) {
		case 0: // Bottom Fence Rail
			VectorMA(beam[i]->s.origin,-15, up, beam[i]->s.origin);
			break;
		case 1: // Mid-Lower Fence Rail
			break;
		case 2: // Mid-Upper Fence Rail
			VectorMA(beam[i]->s.origin, 15, up, beam[i]->s.origin);
			break;
		case 3: // Top Fence Rail
			VectorMA(beam[i]->s.origin, 30, up, beam[i]->s.origin);
			break;
		case 4: // Make 1st Fence Post.
			beam[i]->spawnflags = 55;
			beam[i]->s.frame *= 4;
			VectorCopy(post1, beam[i]->s.origin);
			VectorMA(beam[i]->s.origin, -20, up, beam[i]->s.origin);
			VectorCopy(up, beam[i]->movedir);
			beam[i]->s.skinnum = Laser_Blue;
			break;
		case 5: // Make 2nd Fence Post.
			beam[i]->spawnflags = 55;
			beam[i]->s.frame *= 4;
			VectorCopy(post2, beam[i]->s.origin);
			VectorMA(beam[i]->s.origin, -20, up, beam[i]->s.origin);
			VectorCopy(up, beam[i]->movedir);
			beam[i]->s.skinnum = Laser_Blue;
			break;
		} // end switch
		beam[i]->wait = level.time + 20; // 20 seconds to beam Destruct.
		beam[i]->think = beam_laser_think;
		beam[i]->nextthink = level.time + 1.0;
		gi.linkentity(beam[i]);
	} // end for
}

void Spawn_LaserCorral(edict_t *ent);
//===========================================
void Cmd_LaserFence_f(edict_t *ent) {
	int index;
	
	index = ITEM_INDEX(FindItem("cells"));
	//if (ITEM_IN_ENTS_INVENTORY < 20)
	//gi.centerprintf(ent, "LaserFence requires 50 PowerCells\n");
	//else {
	//ITEM_IN_ENTS_INVENTORY -= 20;
	Spawn_LaserFence(ent); //}
}
#define CORRAL_LENGTH 80


//=======================================================
// Spawns an Electric Laser Coral
//=======================================================
void Spawn_LaserCorral(edict_t *ent) {
	int i,j;
	edict_t *beam[4][4];
	vec3_t post[4], tangles, len;
	vec3_t forward, up, right, dir, end;
	vec3_t torigin, torigin2;
	trace_t tr;
	
	VectorCopy(ent->s.origin, torigin);
	
	for (j=0; j<=3; j++) {
		switch (j) {
		case 0: // Get first post position
			VectorCopy(ent->s.angles, tangles);
			AngleVectors(tangles, forward, right, up);
			VectorMA(torigin, 50, forward, end);
			tr = gi.trace(torigin, NULL, NULL, end, NULL, MASK_SOLID);
			min_vector(torigin, tr.endpos, end, torigin2);
			VectorClear(end);
			VectorMA(torigin2, -CORRAL_LENGTH/2, right, end);
			tr = gi.trace(torigin2, NULL, NULL, end, NULL, MASK_SHOT);
			min_vector(torigin2, tr.endpos, end, post[j]);
			break;
		case 1: // Next post
		case 2: // Next post
		case 3: // Next post
			// Get dir of post[j-1] --> post[j]
			VectorSubtract(post[j], post[j-1], dir);
			// Split dir into angle vectors.
			vectoangles(dir, tangles);
			// Find which way is 'right' of post[j].
			AngleVectors(tangles, NULL, right, NULL);
		} // end switch
		
		// post[j+1] is CORRAL_LENGTH 'right' of post[j]
		VectorClear(end);
		VectorMA(post[j], CORRAL_LENGTH, right, end);
		tr = gi.trace(torigin2, NULL, NULL, end, NULL, MASK_SHOT);
		min_vector(torigin2, tr.endpos, end, post[j+1]);
		AngleVectors(tangles, NULL, right, NULL);
		VectorClear(end);
		
		// Rails need to know length between posts!
		abs_vector(post[j], post[j+1], len);
		
		for (i=0; i<=3; i++) {
			beam[j][i] = G_Spawn();
			beam[j][i]->owner = beam[j][i];
			beam[j][i]->classname= "LaserCorral";
			beam[j][i]->activator = ent; // Must set for frags!!
			beam[j][i]->movetype = MOVETYPE_NONE;
			beam[j][i]->solid = SOLID_NOT;
			beam[j][i]->svflags &= ~SVF_NOCLIENT;
			beam[j][i]->s.renderfx = RF_BEAM;
			beam[j][i]->s.modelindex = 1;
			beam[j][i]->spawnflags = VectorLength(len); // Store Length here..
			beam[j][i]->s.skinnum = Laser_Red;
			if (i==0) // Only need sound for first one..
				beam[j][i]->s.sound = gi.soundindex ("world/laser.wav");
			else
				beam[j][i]->s.sound=0;
			beam[j][i]->s.frame=2;
			VectorSet(beam[j][i]->mins,-8,-8,-8);
			VectorSet(beam[j][i]->maxs, 8, 8, 8);
			beam[j][i]->dmg = 50; // Damage for each Rail..
			VectorClear(beam[j][i]->s.angles);
			VectorCopy(right, beam[j][i]->movedir);
			VectorCopy(post[j], beam[j][i]->s.origin);
			VectorCopy(post[j], beam[j][i]->s.old_origin);
			switch (i) {
			case 0: // Mid-Lower Fence Rail
				break;
			case 1: // Mid-Upper Fence Rail
				VectorMA(beam[j][i]->s.origin, 15, up, beam[j][i]->s.origin);
				break;
			case 2: // Make a Fence Post.
				beam[j][i]->spawnflags = 56; // No particles.
				beam[j][i]->s.frame *= 4;
				VectorCopy(post[j], beam[j][i]->s.origin);
				VectorMA(beam[j][i]->s.origin, -20, up, beam[j][i]->s.origin);
				VectorCopy(up, beam[j][i]->movedir);
				beam[j][i]->s.skinnum = Laser_Blue;
				break;
			} // end switch
			beam[j][i]->wait = level.time + 60.0; // 1 Minute to Destruct.
			beam[j][i]->think = beam_laser_think;
			beam[j][i]->nextthink = level.time + 2.0;
			gi.linkentity(beam[j][i]);
		} // end for
	} // end for
}





/*
+=================
+Think_Airstrike
+CCH: This will bring the airstrike ordinance into existence in the game
+Called by ClientThink
+=================
*/
void Think_Airstrike (edict_t *ent)
{
	vec3_t  start;
	vec3_t  forward;
	vec3_t  end;
	vec3_t  targetdir;
	trace_t tr;
	
	// find the target point
	VectorCopy(ent->s.origin, start);
	start[2] += ent->viewheight;
	AngleVectors(ent->client->v_angle, forward, NULL, NULL);
	VectorMA(start, 8192, forward, end);
	tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA);
	// find the direction from the entry point to the target
	VectorSubtract(tr.endpos, ent->client->airstrike_entry, targetdir);
	VectorNormalize(targetdir);
	VectorAdd(ent->client->airstrike_entry, targetdir, start);
	
	// check to make sure we're not materializing in a solid
	if ( gi.pointcontents(start) == CONTENTS_SOLID )
	{
		safe_cprintf(ent, PRINT_HIGH, "Airstrike intercepted en route.\n");
		return;
	}
	
	// fire away!
	fire_clust(ent, start, targetdir, 600, 550, 600, 600);
	safe_cprintf(ent, PRINT_HIGH, "Airstrike has arrived.\n");
}

/*
+=================
+Think_Airstrike
+CCH: This will bring the airstrike ordinance into existence in the game
+Called by ClientThink
+=================
*/
void Think_Lightn (edict_t *ent)
{
	vec3_t  start;
	vec3_t  forward;
	vec3_t  end;
	vec3_t  targetdir;
	trace_t tr;
	
	// find the target point
	VectorCopy(ent->s.origin, start);
	start[2] += ent->viewheight;
	AngleVectors(ent->client->v_angle, forward, NULL, NULL);
	VectorMA(start, 8192, forward, end);
	tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA);
	// find the direction from the entry point to the target
	VectorSubtract(tr.endpos, ent->client->airstrike_entry, targetdir);
	VectorNormalize(targetdir);
	VectorAdd(ent->client->airstrike_entry, targetdir, start);
	
	// check to make sure we're not materializing in a solid
	if ( gi.pointcontents(start) == CONTENTS_SOLID )
	{
		safe_cprintf(ent, PRINT_HIGH, "Lightning intercepted en route.\n");
		return;
	}
	
	// fire away!
	fire_lightning_bigbolt(ent,start, targetdir, 60000, 1500, 1200, 1200);
	safe_cprintf(ent, PRINT_HIGH, "Lightning has arrived.\n");
}
					
/***************************************************/
/**************  Anti-Matter Cannon   **************/
/***************************************************/
void BombExplosion (edict_t *self)
{
	T_RadiusDamage(self, self->owner, self->dmg, self->enemy, self->dmg_radius, MOD_BLASTER);
	
	gi.WriteByte (svc_temp_entity);
	if (self->waterlevel)
		gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	else
		gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PHS);
	
	G_FreeEdict (self);
	
}

void fire_asha  (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int mod)
{
	trace_t		tr;
	vec3_t		dir;
	vec3_t		forward, right, up;
	vec3_t		end;
	float		r;
	float		u;
	vec3_t		water_start;
	qboolean	water = false;
	int			content_mask = MASK_SHOT | MASK_WATER;
	qboolean        is_onturret=false;
	
	//eo! Turret encara no posat! FIXME
	//#if 0
	//       if (self->client)
	//                is_onturret = (self->client->onturret > 0);
	//        else
	//                if (self->flags & FL_TEAMSLAVE)
	//                       is_onturret=true;
	//#endif
	
	if (!is_onturret)
		tr = gi.trace (self->s.origin, NULL, NULL, start, self, MASK_SHOT);
	else
		tr = gi.trace (start, NULL, NULL, start, self, MASK_SHOT);
	
	if (!(tr.fraction < 1.0))
	{
		vectoangles (aimdir, dir);
		AngleVectors (dir, forward, right, up);
		
		r = crandom()*hspread;
		u = crandom()*vspread;
		VectorMA (start, 8192, forward, end);
		VectorMA (end, r, right, end);
		VectorMA (end, u, up, end);
		
		if (gi.pointcontents (start) & MASK_WATER)
		{
			water = true;
			VectorCopy (start, water_start);
			content_mask &= ~MASK_WATER;
		}
		
		tr = gi.trace (start, NULL, NULL, end, self, content_mask);
		
		// see if we hit water
		if (tr.contents & MASK_WATER)
		{
			int		color;
			
			water = true;
			VectorCopy (tr.endpos, water_start);
			
			if (!VectorCompare (start, tr.endpos))
			{
				if (tr.contents & CONTENTS_WATER)
				{
					if (strcmp(tr.surface->name, "*brwater") == 0)
						color = SPLASH_BROWN_WATER;
					else
						color = SPLASH_BLUE_WATER;
				}
				else if (tr.contents & CONTENTS_SLIME)
					color = SPLASH_SLIME;
				else if (tr.contents & CONTENTS_LAVA)
					color = SPLASH_LAVA;
				else
					color = SPLASH_UNKNOWN;
				
				if (color != SPLASH_UNKNOWN)
				{
					gi.WriteByte (svc_temp_entity);
					gi.WriteByte (TE_SPLASH);
					gi.WriteByte (8);
					gi.WritePosition (tr.endpos);
					gi.WriteDir (tr.plane.normal);
					gi.WriteByte (color);
					gi.multicast (tr.endpos, MULTICAST_PVS);
				}
				
				// change bullet's course when it enters water
				VectorSubtract (end, start, dir);
				vectoangles (dir, dir);
				AngleVectors (dir, forward, right, up);
				r = crandom()*hspread*2;
				u = crandom()*vspread*2;
				VectorMA (water_start, 8192, forward, end);
				VectorMA (end, r, right, end);
				VectorMA (end, u, up, end);
			}
			
			// re-trace ignoring water this time
			tr = gi.trace (water_start, NULL, NULL, end, self, MASK_SHOT);
		}
	}
	
	// send gun puff / flash
	if (!((tr.surface) && (tr.surface->flags & SURF_SKY)))
	{
		if (tr.fraction < 1.0)
		{
			edict_t *bomb;
			vec3_t bdest;
			
			if (tr.ent->takedamage)
			{
				T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, 0, mod);
			}
			
			VectorMA(tr.endpos, -4, aimdir, bdest);
			bomb = G_Spawn();
			bomb->think = BombExplosion;
			bomb->dmg = 80;
			bomb->enemy = tr.ent;
			bomb->dmg_radius = 100;
			bomb->nextthink = level.time + 0.1 + random() * 0.3;
			VectorCopy(bdest, bomb->s.origin);
			bomb->svflags = SVF_NOCLIENT;
			bomb->owner = self;
			
			gi.linkentity(bomb);
		}
	}
	
	// if went through water, determine where the end and make a bubble trail
	if (water)
	{
		vec3_t	pos;
		
		VectorSubtract (tr.endpos, water_start, dir);
		VectorNormalize (dir);
		VectorMA (tr.endpos, -2, dir, pos);
		if (gi.pointcontents (pos) & MASK_WATER)
			VectorCopy (pos, tr.endpos);
		else
			tr = gi.trace (pos, NULL, NULL, water_start, tr.ent, MASK_WATER);
		
		VectorAdd (water_start, tr.endpos, pos);
		VectorScale (pos, 0.5, pos);
		
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BUBBLETRAIL);
		gi.WritePosition (water_start);
		gi.WritePosition (tr.endpos);
		gi.multicast (pos, MULTICAST_PVS);
	}
}

void SightSound (edict_t *ent)
{
		if (ent->client->resp.class == 1)
			gi.sound (ent, CHAN_VOICE, gi.soundindex("soldier/solsrch1.wav"), 1, ATTN_NORM, 0);
		else if (ent->client->resp.class == 36)
			gi.sound (ent, CHAN_VOICE, gi.soundindex("soldier/solsrch1.wav"), 1, ATTN_NORM, 0);
		else if (ent->client->resp.class == 2)
			gi.sound (ent, CHAN_VOICE, gi.soundindex("infantry/infsght1.wav"), 1, ATTN_NORM, 0);
		else if (ent->client->resp.class == 3)
			gi.sound (ent, CHAN_VOICE, gi.soundindex("tank/sight1.wav"), 1, ATTN_NORM, 0);
		else if (ent->client->resp.class == 33)
			gi.sound (ent, CHAN_VOICE, gi.soundindex("tank/sight1.wav"), 1, ATTN_NORM, 0);
		else if (ent->client->resp.class == 4)
			gi.sound (ent, CHAN_VOICE, gi.soundindex("medic/medsght1.wav"), 1, ATTN_NORM, 0);
		else if (ent->client->resp.class == 28)
			gi.sound (ent, CHAN_VOICE, gi.soundindex("medic_commander/medsght.wav"), 1, ATTN_NORM, 0);
		else if (ent->client->resp.class == 5)
			gi.sound (ent, CHAN_VOICE, gi.soundindex("gunner/sight1.wav"), 1, ATTN_NORM, 0);
		else if (ent->client->resp.class == 6)
			gi.sound (ent, CHAN_VOICE, gi.soundindex("gladiator/sight.wav"), 1, ATTN_NORM, 0);
		else if (ent->client->resp.class == 34)
			gi.sound (ent, CHAN_VOICE, gi.soundindex("gladiator/sight.wav"), 1, ATTN_NORM, 0);
		else if (ent->client->resp.class == 21)
			gi.sound (ent, CHAN_VOICE, gi.soundindex("gladiator/sight.wav"), 1, ATTN_NORM, 0);
		else if (ent->client->resp.class == 7)
			gi.sound (ent, CHAN_VOICE, gi.soundindex("bosstank/BTKUNQV2.wav"), 1, ATTN_NORM, 0);
		else if (ent->client->resp.class == 39)
			gi.sound (ent, CHAN_VOICE, gi.soundindex("bosstank/BTKUNQV2.wav"), 1, ATTN_NORM, 0);
		else if (ent->client->resp.class == 24)
			gi.sound (ent, CHAN_VOICE, gi.soundindex("bosstank/BTKUNQV2.wav"), 1, ATTN_NORM, 0);
		else if (ent->client->resp.class == 8)
			gi.sound (ent, CHAN_VOICE, gi.soundindex("flyer/flysght1.wav"), 1, ATTN_NORM, 0);
		else if (ent->client->resp.class == 35)
			gi.sound (ent, CHAN_VOICE, gi.soundindex("flyer/flysght1.wav"), 1, ATTN_NORM, 0);
		else if (ent->client->resp.class == 32)
			gi.sound (ent, CHAN_VOICE, gi.soundindex("flyer/flysght1.wav"), 1, ATTN_NORM, 0);
		else if (ent->client->resp.class == 9)
			gi.sound (ent, CHAN_VOICE, gi.soundindex("berserk/sight.wav"), 1, ATTN_NORM, 0);
		else if (ent->client->resp.class == 10)
			gi.sound (ent, CHAN_VOICE, gi.soundindex("chick/Chksght1.wav"), 1, ATTN_NORM, 0);
		else if (ent->client->resp.class == 23)
			gi.sound (ent, CHAN_VOICE, gi.soundindex("chick/Chksght1.wav"), 1, ATTN_NORM, 0);
		else if (ent->client->resp.class == 11)
			gi.sound (ent, CHAN_VOICE, gi.soundindex("bosshovr/BHVUNQV1.WAV"), 1, ATTN_NORM, 0);
		else if (ent->client->resp.class == 29)
			gi.sound (ent, CHAN_VOICE, gi.soundindex("carrier/sight.WAV"), 1, ATTN_NORM, 0);
		else if (ent->client->resp.class == 12)
			gi.sound (ent, CHAN_VOICE, gi.soundindex("brain/brnsght1.WAV"), 1, ATTN_NORM, 0);
		else if (ent->client->resp.class == 22)
			gi.sound (ent, CHAN_VOICE, gi.soundindex("brain/brnsght1.WAV"), 1, ATTN_NORM, 0);
		else if (ent->client->resp.class == 13)
			gi.sound (ent, CHAN_VOICE, gi.soundindex("mutant/Mutsrch1.wav"), 1, ATTN_NORM, 0);
		else if (ent->client->resp.class == 18)
			gi.sound (ent, CHAN_VOICE, gi.soundindex("boss3/BS3SRCH1.WAV"), 1, ATTN_NORM, 0);
		else if (ent->client->resp.class == 19)
			gi.sound (ent, CHAN_VOICE, gi.soundindex("makron/voice3.wav"), 1, ATTN_NORM, 0);
		else if (ent->client->resp.class == 30)
			gi.sound (ent, CHAN_VOICE, gi.soundindex("widow/laugh.wav"), 1, ATTN_NORM, 0);
		else if (ent->client->resp.class == 31)
			gi.sound (ent, CHAN_VOICE, gi.soundindex("widow/laugh.wav"), 1, ATTN_NORM, 0);
		else if (ent->client->resp.class == 16)
			gi.sound (ent, CHAN_VOICE, gi.soundindex("floater/Fltsght1.wav"), 1, ATTN_NORM, 0);
		else if (ent->client->resp.class == 17)
			gi.sound (ent, CHAN_VOICE, gi.soundindex("flipper/Flpsght1.wav"), 1, ATTN_NORM, 0);
		else if (ent->client->resp.class == 41)
			//gi.sound (ent, CHAN_VOICE, gi.soundindex("parasite/Parsght1.wav"), 1, ATTN_NORM, 0);
			gi.sound (ent, CHAN_VOICE, gi.soundindex("tank/sight1.wav"), 1, ATTN_NORM, 0);
		else if (ent->client->resp.class == 40)
			gi.sound (ent, CHAN_VOICE, gi.soundindex("parasite/Parsght1.wav"), 1, ATTN_NORM, 0);
		else if (ent->client->resp.class == 15)
			gi.sound (ent, CHAN_VOICE, gi.soundindex("hover/hovsght1.wav"), 1, ATTN_NORM, 0);
		else if (ent->client->resp.class == 26)
			gi.sound (ent, CHAN_VOICE, gi.soundindex("daedalus/daedsght1.wav"), 1, ATTN_NORM, 0);
		else if (ent->client->resp.class == 20)
			gi.sound (ent, CHAN_VOICE, gi.soundindex("soldier/solsrch2.wav"), 1, ATTN_NORM, 0);
		else if (ent->client->resp.class == 25)
			gi.sound (ent, CHAN_VOICE, gi.soundindex("gek/gk_sght1.wav"), 1, ATTN_NORM, 0);
		else if (ent->client->resp.class == 27)
			gi.sound (ent, CHAN_VOICE, gi.soundindex("stalker/sight.wav"), 1, ATTN_NORM, 0);
}

/*

Function: Start_Kamikaze_Mode

 

  Places the edict passed to it into Kamikaze Mode

  (probably best to pass a player to it)

 

  Warns everyone that so and so is a kamikaze...

 

*/

void Start_Kamikaze_Mode(edict_t *the_doomed_one){

        

        /* see if we are already in  kamikaze mode*/

        if (the_doomed_one->client->kamikaze_mode & 1)  {

               safe_cprintf(the_doomed_one, PRINT_MEDIUM, "Already in Kamikaze Mode!! Kiss you butt Goodbye!");

               return;            

        }

        

        /* dont run if in god mode  */

        if (the_doomed_one->flags & FL_GODMODE){

               safe_cprintf(the_doomed_one, PRINT_MEDIUM, "Can't Kamikaze in God Mode, Whats the Point?");

               return;

        }

        /* not in kamikaze mode yet */

        the_doomed_one->client->kamikaze_mode = 1;

 

        /*  Give us only so long */

        the_doomed_one->client->kamikaze_timeleft = 50;

        the_doomed_one->client->kamikaze_framenum = level.framenum + the_doomed_one->client->kamikaze_timeleft;

 

        /* Warn the World */

        safe_bprintf (PRINT_MEDIUM,"%s is a Kamikaze - BANZAI!!\n", the_doomed_one->client->pers.netname);

    gi.sound( the_doomed_one, CHAN_WEAPON, gi.soundindex("makron/rail_up.wav"), 1, ATTN_NONE, 0 );

 

        return;

}

 

/* 

Function: Kamikaze_Active

        Are we in Kamikaze Mode? 

        a helper function to see if we are running in Kamikaze Mode

 

*/

qboolean Kamikaze_Active(edict_t *the_doomed_one){

        return (the_doomed_one->client->kamikaze_mode);

}

 

 

/* 

Function: Kamikaze_Cancel

  Canceled for Some Reason

  Call if Player is killed before time is up

*/

void Kamikaze_Cancel(edict_t *the_spared_one){

        /* not in kamikaze mode yet */

        the_spared_one->client->kamikaze_mode = 0;

        /* Give us only so long */

        the_spared_one->client->kamikaze_timeleft = 0;

        the_spared_one->client->kamikaze_framenum = 0;

        

        return;

}

 

void Kamikaze_Explode(edict_t *the_doomed_one){

 

 

    /* A whole Lotta Damage */

    T_RadiusDamage (the_doomed_one, the_doomed_one, 300, NULL, 800, MOD_R_SPLASH);

 

     /* BANG ! and show the clients */

     gi.WriteByte (svc_temp_entity);

     gi.WriteByte (TE_EXPLOSION1);

     gi.WritePosition(the_doomed_one -> s.origin);

     gi.multicast (the_doomed_one->s.origin, MULTICAST_PVS);

}

void ClassAbilities (edict_t *ent)
{
	if (ent->health <= 0)
	{
		safe_cprintf (ent, PRINT_HIGH, "Not alive, no health, no abilities!\n");
		return;
	}

	if ((int)(stroggflags->value) & SF_NO_ABILITIES)
	{
		safe_cprintf (ent, PRINT_HIGH, "Sorry, abilities have been disabled.\n");
		return;
	}

	//		if (noabiltity->value)
	//		{
	//			safe_cprintf (ent, PRINT_HIGH, "Abilities are off.\n");
	//			return;
	//		}
	if (ent->client->resp.class == 1)
	{
		if (ent->client->pers.abilities.powerpoints < 1)
		{
			safe_cprintf (ent, PRINT_HIGH, "Let them regen!\n");
			return;
		}
		if (!ent->client->pers.abilities.soldier_haste)
		{
			safe_cprintf (ent, PRINT_HIGH, "Weapon Haste!\n");
			ent->client->pers.abilities.soldier_haste = 1;
			ent->client->pers.abilities.powerpoints -= 2;
		}
		else
		{
			safe_cprintf (ent, PRINT_HIGH, "Weapon Haste off.\n");
			ent->client->pers.abilities.soldier_haste = 0;
			ent->client->pers.abilities.powerpoints += 2;
		}
	}
	if (ent->client->resp.class == 2)
	{
		if (ent->client->pers.abilities.powerpoints < 100)
		{
			safe_cprintf (ent, PRINT_HIGH, "Let them regen!\n");
			return;
		}
		PlaceLaser (ent);
		ent->client->pers.abilities.powerpoints -= 100;
	}
	if (ent->client->resp.class == 3)
	{
		if (ent->client->pers.abilities.powerpoints < 2)
		{
			safe_cprintf (ent, PRINT_HIGH, "Let them regen!\n");
			return;
		}
		if (!ent->client->pers.abilities.tank_dualrockets)
		{
			safe_cprintf (ent, PRINT_HIGH, "Dual Rockets!\n");
			ent->client->pers.abilities.tank_dualrockets = 1;
			ent->client->pers.abilities.powerpoints -= 5;
		}
		else
		{
			safe_cprintf (ent, PRINT_HIGH, "Dual Rockets off.\n");
			ent->client->pers.abilities.tank_dualrockets = 0;
			ent->client->pers.abilities.powerpoints += 5;
		}
	}
	if (ent->client->resp.class == 4)
	{
		if (ent->client->pers.abilities.powerpoints < 100)
		{
			safe_cprintf (ent, PRINT_HIGH, "Let them regen!\n");
			return;
		}
		Init_Earthquake_Generator ();
		ent->client->pers.abilities.powerpoints -= 100;
	}
	if (ent->client->resp.class == 5)
	{
		if (ent->client->pers.abilities.powerpoints < 2)
		{
			safe_cprintf (ent, PRINT_HIGH, "Let them regen!\n");
			return;
		}
		if (!ent->client->pers.abilities.gunner_forcegrenades)
		{
			safe_cprintf (ent, PRINT_HIGH, "Force Grenades!\n");
			ent->client->pers.abilities.gunner_forcegrenades = 1;
			ent->client->pers.abilities.powerpoints -= 10;
		}
		else
		{
			safe_cprintf (ent, PRINT_HIGH, "Force Grenades off.\n");
			ent->client->pers.abilities.gunner_forcegrenades = 0;
			ent->client->pers.abilities.powerpoints += 10;
		}
	}
	if (ent->client->resp.class == 6)
	{
		if (ent->client->pers.abilities.powerpoints < 2)
		{
			safe_cprintf (ent, PRINT_HIGH, "Let them regen!\n");
			return;
		}
		if (!ent->client->pers.abilities.gladiator_wallpierce)
		{
			safe_cprintf (ent, PRINT_HIGH, "Wall Piercing Railgun!\n");
			ent->client->pers.abilities.gladiator_wallpierce = 1;
			ent->client->pers.abilities.powerpoints -= 2;
		}
		else
		{
			safe_cprintf (ent, PRINT_HIGH, "Wall Piercing Railgun off.\n");
			ent->client->pers.abilities.gladiator_wallpierce = 0;
			ent->client->pers.abilities.powerpoints += 2;
		}
	}
	if (ent->client->resp.class == 7)
	{
		if (ent->client->pers.abilities.powerpoints < 200)
		{
			safe_cprintf (ent, PRINT_HIGH, "Let them regen!\n");
			return;
		}
		Cmd_Airstrike_f (ent);
		ent->client->pers.abilities.powerpoints -= 200;
	}
	if (ent->client->resp.class == 9)
	{
		if (ent->client->pers.abilities.powerpoints < 2)
		{
			safe_cprintf (ent, PRINT_HIGH, "Let them regen!\n");
			return;
		}
		if (!ent->client->pers.abilities.berserk_shield)
		{
			safe_cprintf (ent, PRINT_HIGH, "Shield!\n");
			ent->client->pers.abilities.berserk_shield = 1;
			ent->client->pers.abilities.powerpoints -= 2;
		}
		else
		{
			safe_cprintf (ent, PRINT_HIGH, "Shield off.\n");
			ent->client->pers.abilities.berserk_shield = 0;
			ent->client->pers.abilities.powerpoints += 2;
			ent->s.effects |= 0;
			ent->s.renderfx |= 0;
		}
	}
	if (ent->client->resp.class == 10)
	{
		if (ent->client->pers.abilities.powerpoints < 2)
		{
			safe_cprintf (ent, PRINT_HIGH, "Let them regen!\n");
			return;
		}
		if (!ent->client->pers.abilities.chick_triplespread)
		{
			safe_cprintf (ent, PRINT_HIGH, "Triple Explosion Spread!\n");
			ent->client->pers.abilities.chick_triplespread = 1;
			ent->client->pers.abilities.powerpoints -= 2;
		}
		else
		{
			safe_cprintf (ent, PRINT_HIGH, "Triple Explosion Spread off.\n");
			ent->client->pers.abilities.chick_triplespread = 0;
			ent->client->pers.abilities.powerpoints += 2;
		}
	}
	if (ent->client->resp.class == 23)
	{
		if (ent->client->pers.abilities.powerpoints < 2)
		{
			safe_cprintf (ent, PRINT_HIGH, "Let them regen!\n");
			return;
		}
		if (!ent->client->pers.abilities.dchick_plasma)
		{
			safe_cprintf (ent, PRINT_HIGH, "Rockets Plasma Explosion!\n");
			ent->client->pers.abilities.dchick_plasma = 1;
			ent->client->pers.abilities.powerpoints -= 2;
		}
		else
		{
			safe_cprintf (ent, PRINT_HIGH, "Rockets Plasma Explosion Spread off.\n");
			ent->client->pers.abilities.dchick_plasma = 0;
			ent->client->pers.abilities.powerpoints += 2;
		}
	}
	if (ent->client->resp.class == 24)
	{
		if (ent->client->pers.abilities.powerpoints < 2)
		{
			safe_cprintf (ent, PRINT_HIGH, "Let them regen!\n");
			return;
		}
		if (!ent->client->pers.abilities.dsupertank_ir)
		{
			safe_cprintf (ent, PRINT_HIGH, "IR Vision!\n");
			ent->client->pers.abilities.dsupertank_ir = 1;
			ent->client->pers.abilities.powerpoints -= 2;
			ent->client->ps.rdflags |= RDF_IRGOGGLES;
		}
		else
		{
			safe_cprintf (ent, PRINT_HIGH, "IR Vision off.\n");
			ent->client->pers.abilities.dsupertank_ir = 0;
			ent->client->pers.abilities.powerpoints += 2;
			ent->client->ps.rdflags &= ~RDF_IRGOGGLES;
		}
	}
	if (ent->client->resp.class == 36)
	{
		vec3_t	offset, start;
		vec3_t	forward, right;
		
		
		if (ent->client->pers.abilities.powerpoints < 300)
		{
			safe_cprintf (ent, PRINT_HIGH, "Let them regen!\n");
			return;
		}
		
		AngleVectors (ent->client->v_angle, forward, right, NULL);
		VectorScale (forward, -2, ent->client->kick_origin);
		ent->client->kick_angles[0] = -1;
		VectorSet(offset, 8, 8, ent->viewheight-8);
		Middle_Source (ent->client, ent->s.origin, offset, forward, right, start);
		
		fire_lightning_bigbolt (ent, start, forward, 200, 700, 80, 80);
		ent->client->pers.abilities.powerpoints -= 300;
	}
	if (ent->client->resp.class == 33)
	{
		if (ent->client->pers.abilities.powerpoints < 500)
		{
			safe_cprintf (ent, PRINT_HIGH, "Let them regen!\n");
			return;
		}
		
		SpawnTrappedTech(ent);
		ent->client->pers.abilities.powerpoints -= 500;
	}

	if (ent->client->resp.class == 40)
	{
		vec3_t	offset, start;
		vec3_t	forward, right;
		
		
		if (ent->client->pers.abilities.powerpoints < 55)
		{
			safe_cprintf (ent, PRINT_HIGH, "Let them regen!\n");
			return;
		}
		
		AngleVectors (ent->client->v_angle, forward, right, NULL);
		VectorScale (forward, -2, ent->client->kick_origin);
		ent->client->kick_angles[0] = -1;
		VectorSet(offset, 8, 8, ent->viewheight-8);
		Middle_Source (ent->client, ent->s.origin, offset, forward, right, start);
		
		fire_tip (ent, start, forward, 35, 300, EF_BLUEHYPERBLASTER, MOD_BLASTER);
		ent->client->pers.abilities.powerpoints -= 55;
	}
	if (ent->client->resp.class == 8)
	{
		vec3_t	offset, start;
		vec3_t	forward, right;
		
		
		if (ent->client->pers.abilities.powerpoints < 50)
		{
			safe_cprintf (ent, PRINT_HIGH, "Let them regen!\n");
			return;
		}
		
		AngleVectors (ent->client->v_angle, forward, right, NULL);
		VectorScale (forward, -2, ent->client->kick_origin);
		ent->client->kick_angles[0] = -1;
		VectorSet(offset, 8, 8, ent->viewheight-8);
		Middle_Source (ent->client, ent->s.origin, offset, forward, right, start);
		
		//Launch_BigBomb (ent, start, forward, 50, 300, 100, 100);
		fire_fire (ent, start, forward, 100, 500, 2, 100, false);
		ent->client->pers.abilities.powerpoints -= 50;
	}
	if (ent->client->resp.class == 15)
	{
		if (ent->client->pers.abilities.powerpoints < 2)
		{
			safe_cprintf (ent, PRINT_HIGH, "Let them regen!\n");
			return;
		}
		if (!ent->client->pers.abilities.icarus_explobolts)
		{
			safe_cprintf (ent, PRINT_HIGH, "Explosive-Tipped Bolts!\n");
			ent->client->pers.abilities.icarus_explobolts = 1;
			ent->client->pers.abilities.powerpoints -= 2;
		}
		else
		{
			safe_cprintf (ent, PRINT_HIGH, "Explosive-Tipped Bolts off.\n");
			ent->client->pers.abilities.icarus_explobolts = 0;
			ent->client->pers.abilities.powerpoints += 2;
		}
	}
	if (ent->client->resp.class == 27)
	{
		if (ent->client->pers.abilities.powerpoints < 2)
		{
			safe_cprintf (ent, PRINT_HIGH, "Let them regen!\n");
			return;
		}
		if (!ent->client->pers.abilities.stalker_homing)
		{
			safe_cprintf (ent, PRINT_HIGH, "Homing Bolts!\n");
			ent->client->pers.abilities.stalker_homing = 1;
			ent->client->pers.abilities.powerpoints -= 2;
		}
		else
		{
			safe_cprintf (ent, PRINT_HIGH, "Homing Bolts off.\n");
			ent->client->pers.abilities.stalker_homing = 0;
			ent->client->pers.abilities.powerpoints += 2;
		}
	}
	/*if (ent->client->resp.class == 17)
	{
	if (ent->client->pers.abilities.powerpoints < 80)
	{
	safe_cprintf (ent, PRINT_HIGH, "Let them regen!\n");
	return;
	}
	
	  Vortex_Function_Here
	  ent->client->pers.abilities.powerpoints -= 80;
	  } // FIXME: Make it possible one day.
	*/
	if (ent->client->resp.class == 21)
	{
		vec3_t		start;
		vec3_t		forward, right;
		vec3_t		offset;
		int			damage;
		int			kick;
		int      count;
		if (ent->client->pers.abilities.powerpoints < 100)
		{
			safe_cprintf (ent, PRINT_HIGH, "Let them regen!\n");
			return;
		}
		
		
		damage = 80;
		kick = 200;
		
		
		AngleVectors (ent->client->v_angle, forward, right, NULL);
		
		VectorScale (forward, -3, ent->client->kick_origin);
		ent->client->kick_angles[0] = -3;
		
		if (ent->groundentity)
		{
			ent->groundentity = NULL;
			ent->s.origin[2]++;
		}
		
		VectorMA (ent->velocity, -300, forward, ent->velocity);
		
		VectorSet(offset, 0, 7,  ent->viewheight-8);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
		
		for (count = 1 ; count <= 6 ; count++)
			fire_asha (ent, start, forward, damage, kick, 1310, 655, MOD_BLASTER);
		
		//Cmd_Airstrike_f (ent);
		ent->client->pers.abilities.powerpoints -= 100;
	}
	if (ent->client->resp.class == 20)
	{
		if (ent->client->pers.abilities.powerpoints < 1)
		{
			safe_cprintf (ent, PRINT_HIGH, "Let them regen!\n");
			return;
		}
		if (!ent->client->pers.abilities.soldierb_speed)
		{
			safe_cprintf (ent, PRINT_HIGH, "Weapon Projectile Speed!\n");
			ent->client->pers.abilities.soldierb_speed = 1;
			ent->client->pers.abilities.powerpoints -= 2;
		}
		else
		{
			safe_cprintf (ent, PRINT_HIGH, "Weapon Projectile Speed off.\n");
			ent->client->pers.abilities.soldierb_speed = 0;
			ent->client->pers.abilities.powerpoints += 2;
		}
	}
	if (ent->client->resp.class == 16)
	{
		if (ent->client->pers.abilities.powerpoints < 2)
		{
			safe_cprintf (ent, PRINT_HIGH, "Let them regen!\n");
			return;
		}
		if (!ent->client->pers.abilities.floater_fragmulti)
		{
			safe_cprintf (ent, PRINT_HIGH, "2x frags for kills!\n");
			ent->client->pers.abilities.floater_fragmulti = 1;
			ent->client->pers.abilities.powerpoints -= 2;
		}
		else
		{
			safe_cprintf (ent, PRINT_HIGH, "2x frags for kills off.\n");
			ent->client->pers.abilities.floater_fragmulti = 0;
			ent->client->pers.abilities.powerpoints += 2;
		}
	}
	if (ent->client->resp.class == 22)
	{
		if (ent->client->pers.abilities.powerpoints < 2)
		{
			safe_cprintf (ent, PRINT_HIGH, "Let them regen!\n");
			return;
		}
		if (!ent->client->pers.abilities.bbrain_tentacles)
		{
			safe_cprintf (ent, PRINT_HIGH, "Tentacles Drain!\n");
			ent->client->pers.abilities.bbrain_tentacles = 1;
			ent->client->pers.abilities.powerpoints -= 2;
		}
		else
		{
			safe_cprintf (ent, PRINT_HIGH, "Tentacles Drain off.\n");
			ent->client->pers.abilities.bbrain_tentacles = 0;
			ent->client->pers.abilities.powerpoints += 2;
		}
	}
	
	if (ent->client->resp.class == 13 || ent->client->resp.class == 25)
	{
		if (!ent->client->sentry)
		{
			if (ent->client->pers.abilities.powerpoints < 500)
			{
				safe_cprintf (ent, PRINT_HIGH, "Let them regen!\n");
				return;
			}

			Create_Sentry (ent);

			ent->client->pers.abilities.powerpoints -= 500;
		}
		else
			Create_Sentry (ent);
	}
	gi.cvar_forceset("gl_saturatelighting","0"); 
	gi.cvar_forceset("r_fullbright","0"); 
	//fire_proxgrenade
	if (ent->client->resp.class == 12)
	{
		vec3_t down;
		if (ent->client->pers.abilities.brain_proxies == 15)
		{
			safe_cprintf (ent, PRINT_HIGH, "Too many active.\n");
			return;
		}
		if (ent->client->pers.abilities.powerpoints < 30)
		{
			safe_cprintf (ent, PRINT_HIGH, "Let them regen!\n");
			return;
		}
		ent->client->pers.abilities.brain_proxies++;
		fire_proxgrenade (ent, ent->s.origin, down, 45, 0, 60, 100, false);
		ent->client->pers.abilities.powerpoints -= 30;
	}
	if (ent->client->resp.class == 28)
	{
		if (ent->client->pers.abilities.powerpoints < 65)
		{
			safe_cprintf (ent, PRINT_HIGH, "Let them regen!\n");
			return;
		}
		Secondary_Explosion (ent);
		ent->client->pers.abilities.powerpoints -= 65;
	}
	if (ent->client->resp.class == 32)
	{
		if (ent->client->pers.abilities.powerpoints < 400)
		{
			safe_cprintf (ent, PRINT_HIGH, "Let them regen!\n");
			return;
		}
		Cmd_LaserFence_f (ent);
		ent->client->pers.abilities.powerpoints -= 400;
	}
	if (ent->client->resp.class == 26)
	{
		if (ent->client->pers.abilities.powerpoints < 2)
		{
			safe_cprintf (ent, PRINT_HIGH, "Let them regen!\n");
			return;
		}
		if (!ent->client->pers.abilities.daed_ir)
		{
			safe_cprintf (ent, PRINT_HIGH, "Cloaked!\n");
			ent->client->pers.abilities.daed_ir = 1;
			ent->client->pers.abilities.powerpoints -= 2;
			//ent->s.effects |= EF_SPHERETRANS;
			//if (ent->client->cloak_time > level.time) 
			//{ 
			//ent->s.effects = EF_SPHERETRANS; 
			//if (!VectorLength(ent->velocity)) 
			ent->svflags |= SVF_NOCLIENT; 
			//else if (ent->movetype != MOVETYPE_NOCLIP) 
			//ent->svflags &= ~SVF_NOCLIENT; 
			//} 
			//else
			//{ 
			//	ent->svflags &= ~SVF_NOCLIENT; 
			//	ent->client->invisible = 0; 
			//} 
		}
		else
		{
			safe_cprintf (ent, PRINT_HIGH, "Cloaking off.\n");
			ent->client->pers.abilities.daed_ir = 0;
			ent->client->pers.abilities.powerpoints += 2;
			ent->s.effects |= 0;
			ent->svflags = 0;
		}
	}
	
	if (ent->client->resp.class == 41)
	{
		vec3_t	offset, start;
		vec3_t	forward, right;
		
		
		if (ent->client->pers.abilities.powerpoints < 500)
		{
			safe_cprintf (ent, PRINT_HIGH, "Let them regen!\n");
			return;
		}
		
		AngleVectors (ent->client->v_angle, forward, right, NULL);
		VectorScale (forward, -2, ent->client->kick_origin);
		ent->client->kick_angles[0] = -1;
		VectorSet(offset, 8, 8, ent->viewheight-8);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
		
		Launch_Ball (ent, start, forward, 2, 70);
		ent->client->pers.abilities.powerpoints -= 500;
	}

	if (ent->client->resp.class == 37)
	{
		vec3_t	offset, start;
		vec3_t	forward, right;
		
		
		if (ent->client->pers.abilities.powerpoints < 100)
		{
			safe_cprintf (ent, PRINT_HIGH, "Let them regen!\n");
			return;
		}
		
		AngleVectors (ent->client->v_angle, forward, right, NULL);
		VectorScale (forward, -2, ent->client->kick_origin);
		ent->client->kick_angles[0] = -1;
		VectorSet(offset, 8, 8, ent->viewheight-8);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
		
		fire_explogib (ent, start, forward, 50, 600, EF_GIB, 0);
		ent->client->pers.abilities.powerpoints -= 100;
	}

	if (ent->client->resp.class == 38)
	{
		
		
		if (ent->client->pers.abilities.powerpoints < 500)
		{
			safe_cprintf (ent, PRINT_HIGH, "Let them regen!\n");
			return;
		}
		
		if (!ent->client->kamikaze_mode)
		{
			Start_Kamikaze_Mode(ent);
			ent->client->pers.abilities.powerpoints -= 500;
		}
		else
			Start_Kamikaze_Mode(ent);
		// Cancellation
	}
	if (ent->client->resp.class == 34)
	{
		vec3_t	offset, start;
		vec3_t	forward, right;
		
		
		if (ent->client->pers.abilities.powerpoints < 200)
		{
			safe_cprintf (ent, PRINT_HIGH, "Let them regen!\n");
			return;
		}
		
		AngleVectors (ent->client->v_angle, forward, right, NULL);
		VectorScale (forward, -2, ent->client->kick_origin);
		ent->client->kick_angles[0] = -1;
		VectorSet(offset, 8, 8, ent->viewheight-8);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
		
		fire_freezer(ent, start, forward, 20, 600, EF_GRENADE);
		ent->client->pers.abilities.powerpoints -= 200;
	}
	
}

gitem_t	*FindRandomItem ()
{
	int		i;
	gitem_t	*it;

	it = itemlist;
	for (i=0 ; i<game.num_items ; i++, it++)
	{
		if (!it->classname)
			continue;
		if (random() < 0.1)
			return it;
	}

	return NULL;
}

pmenu_t devmenu[] = {
	{ "",				PMENU_ALIGN_LEFT, NULL },
	{ "",				PMENU_ALIGN_LEFT, NULL },
	{ "",				PMENU_ALIGN_LEFT, NULL },
	{ "",				PMENU_ALIGN_LEFT, NULL },
	{ "",				PMENU_ALIGN_LEFT, NULL },
	{ "",				PMENU_ALIGN_LEFT, NULL },
	{ "",				PMENU_ALIGN_LEFT, NULL },
	{ "",				PMENU_ALIGN_LEFT, NULL },
	{ "",				PMENU_ALIGN_LEFT, NULL },
	{ "",				PMENU_ALIGN_LEFT, NULL },
	{ "",				PMENU_ALIGN_LEFT, NULL },
	{ "",				PMENU_ALIGN_LEFT, NULL },
	{ "",				PMENU_ALIGN_LEFT, NULL },
	{ "",				PMENU_ALIGN_LEFT, NULL },
	{ "",				PMENU_ALIGN_LEFT, NULL }
};

/*#define PARTY_CREATE		0
#define PARTY_DESTROY		1
#define PARTY_CHECKING		2
#define PARTY_JOIN			3

int max_parties = 9;
int party_number = 0;

void PartyCommands (edict_t *ent, int command, char *cmd3)
{
	if (command == 0) // Creating a party.
	{
		if (ent->client->resp.party.has_party)
		{
			gi.cprintf (ent, PRINT_HIGH, "You already have a party! Destroy it first!\n");
			return;
		}

		// Has he entered a specific name?
		if (cmd3)
		{
				char *end = "'s Party";
				char *netname;
				char *owner;
				char *newname;
				char            userinfo[MAX_INFO_STRING];

				//memset(owner,0,sizeof (owner));
				//strcpy (owner, ent->client->pers.netname);



                memcpy (userinfo, ent->client->pers.userinfo, sizeof(userinfo));

				owner = Info_ValueForKey (userinfo, "name");

				ent->client->resp.party.owner = owner;

				//sprintf (netname, "%s", owner);
				//netname = &owner;
				//memcpy (owner, netname, sizeof(netname));

				//netname = ent->client->pers.netname;

				//sprintf (end, "%s's Party", ent->client->pers.netname);
				//netname = owner;
				newname = Info_ValueForKey (userinfo, "name");
				strcat (newname, end);
				ent->client->resp.party.name = newname;

				gi.cprintf (ent, PRINT_HIGH, "Party created: %s\n", ent->client->resp.party.name);
		}
		else // Something odd happened.
		{
			gi.cprintf (ent, PRINT_HIGH, "Unknown error happened with the party. Try again!\n");
		}

		// Either way, he has a party.
		// Revision: Special flag telling them we're the owner
		ent->client->resp.party.has_party = 1;
	}
	else if (command == 1) // Destroy
	{
		int i;
		gi.cprintf (ent, PRINT_HIGH, "Destroying Party.. Done\n");
		// No longer an owner.
		ent->client->resp.party.has_party = 0;
		// Clear name.
		ent->client->resp.party.name = NULL;
		// Clear members
		// ClientThink will handle notifying them that the party blew up.
		for (i = 0; i < 5; i++)
		{
			ent->client->resp.party.member[i] = NULL;
		}
		ent->client->resp.party.owner = NULL;
	}
	else if (command == 2) // Checking
	{
		int i;
		edict_t *foundent;
		party_s party[9]; // Max 9 parties.
		char *members[9];
		int p;
		int j;
		int g;

		p = 0;
		// Go through the list, let's find all the PARTAYS! WHOOHOOOOOOOOOOO!
		for (i=0 ; i<maxclients->value ; i++)
		{
			foundent = g_edicts + 1 + i;

			if (foundent->client->resp.party.has_party) // Does this client own a party?
			{
				int te;
				char *ownername;
				//party[p] = foundent->client->resp.party;

				memset (&party[p], 0, sizeof (party[p]));

				for (te = 0; te < 9; te++)
				{
					if (foundent->client->resp.party.member[te])
						party[p].member[te] = foundent->client->resp.party.member[te];
				}

				party[p].name = foundent->client->resp.party.name;

				party[p].owner = foundent->client->resp.party.owner;

				p++;
			}
		}
		
		gi.cprintf (ent, PRINT_HIGH, "These are the current parties:\n\n");

		for (j = 0; j < p; j++)
		{
			if (Q_stricmp(cmd3, "full") == 0)
			{
				int h;
				gi.cprintf (ent, PRINT_HIGH, "%s - Owner: %s\n", party[j].name, party[j].owner);

				for (g = 0; g < 9; g++)
				{
					if (party[j].member[g] && party[j].member[g]->inuse == 1)
					{
						char	userinfo2[MAX_INFO_STRING];
						memcpy (userinfo2, party[j].member[g]->client->pers.userinfo, sizeof(userinfo2));
						
						members[g] = Info_ValueForKey (userinfo2, "name");
					}
					else
						members[g] = "Empty Slot";
				}

				// Paril: This __WILL__ cause overflows if there
				// are 9 parties, each having 9 members filled in.
				// I need to find a way to either not mention spots that have
				// no members, or scrap this whole idea. Then again, when there are never
				// enough Quake2 players to even have three parties, why bother!
				// Marked as a FIXME, return to later and fix this....

				// Revision: Did the best I could to stop overflowing, just put less text
				// Doesn't show their health and frags anymore.. useless information
				// when checking members of parties, I don't think they wanted to know
				// how OTHER people's parties are doing. Can be used for cheating anyway :s
				for (h = 0; h < 9; h++)
				{
					gi.cprintf (ent, PRINT_HIGH, "--Member %i: %s\n", h, members[h]);
				}
			}
			else
				gi.cprintf (ent, PRINT_HIGH, "%s - Owner: %s\n", party[j].name, party[j].owner);
		}
	}
	else if (command == 3) // Joining
	{
		int i;
		edict_t *foundent;
		party_s party[9]; // Max 9 parties.
		int p;
		int j;

		if (Q_stricmp(cmd3, "") == 0)
		{
			gi.cprintf (ent, PRINT_HIGH, "You must enter a party owner's name!\n");
			return;
		}

		if (ent->client->resp.party.has_party) // The client owns a party.
		{
			gi.cprintf (ent, PRINT_HIGH, "You can't own a party AND join one!\n");
			return;
		}

		p = 0;

		// Go through the list, let's find all the PARTAYS! WHOOHOOOOOOOOOOO!
		for (i=0 ; i<maxclients->value ; i++)
		{
			foundent = g_edicts + 1 + i;

			if (foundent->client->resp.party.has_party) // Does this client own a party?
			{
				if (Q_stricmp(cmd3, foundent->client->resp.party.owner) == 0) // We entered their name!
				{
					int mem;

					// Paril Revision
					// We'll do another For statement. We'll find an empty spot in here.
					// This replaces the way I did yesterday that just puts him in no matter what.
					// This way, if there's an empty space in the party, he'll just join that.
					// I don't feel like doing statements to push the numbers down. :)
					for (mem = 0; mem < 5; mem++)
					{
						// Paril, revision
						// Added first statement so he doesn't keep going through the list.
						if ((!ent->client->resp.in_party) && foundent->client->resp.party.member[mem] == NULL) // Yay, we found an empty spot!
						{
							gi.cprintf (foundent, PRINT_HIGH, "%s has entered your party!\n", ent->client->pers.netname);
							gi.cprintf (ent, PRINT_HIGH, "You have entered %s's party!\n", ent->client->pers.netname);

							foundent->client->resp.party.member[mem] = ent; // Okay, we are now on his roster!
							ent->client->resp.in_party = 1; // We're in a party.
							ent->client->resp.current_party = foundent->client->resp.party; // We're in THIS party.
							// Paril, note: This won't stay updated forever, this is only for tracking the owner's name.
							// Once he's in a party, his current_party struct will be filled with the correct
							// information when he kills someone (for shared exp).

							return;
						}
					}
				}
			}
		}

		gi.cprintf (ent, PRINT_HIGH, "No such owner: %s\n", cmd3);
	}
}*/

/*
=================
ClientCommand
=================
*/
void ClientCommand (edict_t *ent)
{
	char	*cmd;
	char	*scommand;
	char	*weap;
	char *seccmd = gi.argv(2);
	char *num = gi.argv(3);
	char *textt;

	if (!ent->client)
		return;		// not fully in game yet
	
	if( ent->frozen ) //acrid 3/19 no cmds if frozen
		return;	

	cmd = gi.argv(0);
	scommand = gi.argv(1);
	weap = gi.argv(2);
	textt = gi.argv(3);

	if (Q_stricmp (cmd, "players") == 0)
	{
		Cmd_Players_f (ent);
		return;
	}
	if (Q_stricmp (cmd, "say") == 0)
	{
		if (ent->client->asking_for_pass)
			CheckEnteredPassword (ent, false, false);
		else
			Cmd_Say_f (ent, false, false);
		return;
	}
	/*if (Q_stricmp (cmd, "whisper") == 0)
	{
		Cmd_Whisper_f (ent, false, false, scommand);
		return;
	}*/
	if (Q_stricmp (cmd, "say_team") == 0)
	{
		CTFSay_Team(ent, gi.args());
		return;
	}
	if (Q_stricmp (cmd, "score") == 0)
	{
		Cmd_Score_f (ent);
		return;
	}
	if (Q_stricmp (cmd, "help") == 0)
	{
		Cmd_Help_f (ent);
		return;
	}
	
	if (level.intermissiontime)
		return;
	
	if (Q_stricmp (cmd, "use") == 0)
		Cmd_Use_f (ent);
	else if (Q_stricmp (cmd, "drop") == 0)
		Cmd_Drop_f (ent);
	else if (Q_stricmp (cmd, "give") == 0)
		Cmd_Give_f (ent);
	else if (Q_stricmp (cmd, "god") == 0)
		Cmd_God_f (ent);
	else if (Q_stricmp (cmd, "notarget") == 0)
		Cmd_Notarget_f (ent);
	else if (Q_stricmp (cmd, "noclip") == 0)
		Cmd_Noclip_f (ent);
	else if (Q_stricmp (cmd, "inven") == 0)
		Cmd_Inven_f (ent);
	else if (Q_stricmp (cmd, "invnext") == 0)
		SelectNextItem (ent, -1);
	else if (Q_stricmp (cmd, "invprev") == 0)
		SelectPrevItem (ent, -1);
	else if (Q_stricmp (cmd, "invnextw") == 0)
		SelectNextItem (ent, IT_WEAPON);
	else if (Q_stricmp (cmd, "invprevw") == 0)
		SelectPrevItem (ent, IT_WEAPON);
	else if (Q_stricmp (cmd, "invnextp") == 0)
		SelectNextItem (ent, IT_POWERUP);
	else if (Q_stricmp (cmd, "invprevp") == 0)
		SelectPrevItem (ent, IT_POWERUP);
	else if (Q_stricmp (cmd, "invuse") == 0)
		Cmd_InvUse_f (ent);
	else if (Q_stricmp (cmd, "invdrop") == 0)
		Cmd_InvDrop_f (ent);
	else if (Q_stricmp (cmd, "weapprev") == 0)
		Cmd_WeapPrev_f (ent);
	else if (Q_stricmp (cmd, "weapnext") == 0)
		Cmd_WeapNext_f (ent);
	else if (Q_stricmp (cmd, "weaplast") == 0)
		Cmd_WeapLast_f (ent);
	else if (Q_stricmp (cmd, "kill") == 0)
		Cmd_Kill_f (ent);
	else if (Q_stricmp (cmd, "putaway") == 0)
		Cmd_PutAway_f (ent);
	//else if (Q_stricmp (cmd, "wave") == 0)
	//Cmd_Wave_f (ent);
	else if (Q_stricmp(cmd, "sight") == 0)
		SightSound (ent);

	//ZOID
	else if (Q_stricmp (cmd, "team") == 0)
	{
		CTFTeam_f (ent);
	} else if (Q_stricmp(cmd, "id") == 0) {
		CTFID_f (ent);
	} else if (Q_stricmp(cmd, "yes") == 0) {
		CTFVoteYes(ent);
	} else if (Q_stricmp(cmd, "no") == 0) {
		CTFVoteNo(ent);
	} else if (Q_stricmp(cmd, "ready") == 0) {
		CTFReady(ent);
	} else if (Q_stricmp(cmd, "notready") == 0) {
		CTFNotReady(ent);
	} else if (Q_stricmp(cmd, "ghost") == 0) {
		CTFGhost(ent);
	} else if (Q_stricmp(cmd, "admin") == 0) {
		CTFAdmin(ent);
	} else if (Q_stricmp(cmd, "stats") == 0) {
		CTFStats(ent);
	} else if (Q_stricmp(cmd, "warp") == 0) {
		CTFWarp(ent);
	} else if (Q_stricmp(cmd, "boot") == 0) {
		CTFBoot(ent);
	} else if (Q_stricmp(cmd, "playerlist") == 0) {
		CTFPlayerList(ent);
	} else if (Q_stricmp(cmd, "observer") == 0) {
		CTFObserver(ent);
	}
//ZOID
	
	// MUCE:  added to jetpack thrust!
	//else if (Q_stricmp(cmd, "thrust") == 0 )
	//Cmd_Thrust_f (ent);
	else if (Q_stricmp(cmd, "menu") == 0)
		OpenMainMenus (ent);
	
	else if (Q_stricmp(cmd, "loc") == 0)
		safe_cprintf (ent, PRINT_HIGH, "%s\n", vtos(ent->s.origin));
	
	else if (Q_stricmp(cmd, "ability") == 0)
		ClassAbilities (ent);
						
	//14
	else if (Q_stricmp(cmd, "hook") == 0) 
		Cmd_Hook_f(ent,cmd); 
	//Wyrm: chasecam
	else if (Q_stricmp(cmd, "chasecam") == 0)
		Cmd_Chasecam_Toggle(ent);
	else if (Q_stricmp(cmd, "cam_maxdistance") == 0)
		Cmd_CamMaxDistance(ent);
	else if (Q_stricmp(cmd, "experience") == 0)
		safe_cprintf (ent, PRINT_HIGH, "Your current level is %i.\nYou have %i/%i experience.\n", ent->client->resp.lvl, ent->client->resp.exp, 250 * (ent->client->resp.lvl) * (ent->client->resp.lvl));
	
	else if (Q_stricmp (cmd, "hurttest") == 0)
		T_Damage (ent, ent, ent, vec3_origin, ent->s.origin, vec3_origin, 20, 0, 0, MOD_UNKNOWN);
	else if (Q_stricmp (cmd, "monsters") == 0)
		safe_cprintf (ent, PRINT_HIGH, "You have used %i monsters out of 6.\n", ent->client->resp.spawned_monsters);
	
	// Sentry control :D
	else if  (Q_stricmp (cmd, "sentry") == 0)
	{ 
		//scommand weap
		if (ent->client->resp.class != 13 && ent->client->resp.class != 25)
		{
			safe_cprintf (ent, PRINT_HIGH, "You do not have the ability to control sentries.\n");
			return;
		}
		
		if (Q_stricmp(scommand, "destroy") == 0)
		{
			if (ent->client->sentry == 1)
				Base_Explode (ent->client->sentry_base);
			else
				safe_cprintf (ent, PRINT_HIGH, "You have no sentry!\n");
		}
		// FIXME Paril
		// Better way: Remove (save weapon info), and respawn.
		/*else if (Q_stricmp(scommand, "move") == 0)
		{
			if (ent->client->sentry == 1)
			{
				VectorCopy (ent->s.origin, ent->client->sentry_base->s.origin);
				VectorCopy (ent->s.origin, ent->client->sentry_sent->s.origin);
			}
			else
				safe_cprintf (ent, PRINT_HIGH, "You have no sentry!\n");
		}*/
		else if (Q_stricmp(scommand, "weapon") == 0)
		{
			Sentry_ControlWeapon (ent);
		}
	}
	/*else if (Q_stricmp (cmd, "party") == 0)
	{
		if (Q_stricmp (scommand, "create") == 0)
		{
			PartyCommands (ent, PARTY_CREATE, weap);
		}
		else if (Q_stricmp (scommand, "destroy") == 0)
		{
			PartyCommands (ent, PARTY_DESTROY, weap);
		}
		else if (Q_stricmp (scommand, "check") == 0)
		{
			PartyCommands (ent, PARTY_CHECKING, weap);
		}
		else if (Q_stricmp (scommand, "join") == 0)
		{
			PartyCommands (ent, PARTY_JOIN, weap);
		}
		else
			gi.cprintf (ent, PRINT_HIGH, "Invalid command. Valid party commands: \"party create (partyname)/destroy/check (full)/join (owner's name)\"\n");
	}*/
	
	// Profile, Paril
	//else if (Q_stricmp (cmd, "profile_age") == 0)
	//Cmd_Profile_Age (ent);
	// Monster control
	else if (Q_stricmp (cmd, "monster_standground") == 0)
		Cmd_MonStandGround (ent);
	else if (Q_stricmp (cmd, "monster_remove") == 0)
		Cmd_MonRemove (ent);
	else if (Q_stricmp (cmd, "monster_select") == 0)
		Cmd_SelMonster (ent);
	else if (Q_stricmp (cmd, "monster_move") == 0)
		SelMonster_GoToPlayer (ent);
	else if (Q_stricmp (cmd, "monster_face") == 0)
		SelMonster_FaceLocation(ent);
	else if (Q_stricmp (cmd, "streak") == 0)
		safe_cprintf (ent, PRINT_HIGH, "%i kills, %i streak\n", ent->client->resp.score, ent->client->pers.streak);
	/*else if (Q_stricmp (cmd, "devmenu") == 0)
	{
		Cmd_DevMenu_f (scommand, ent);
	}
	else if (Q_stricmp(cmd, "exptest") == 0)
	{
		int current_level, needed;
		
		current_level = ent->client->resp.lvl;
		needed = 250 * (current_level) * (current_level);

		ent->client->resp.exp += needed;
		Check_Levelup (ent);
	}*/
	else	// anything that doesn't match a command will be a chat
		//Cmd_Say_f (ent, false, true);
		safe_cprintf (ent, PRINT_HIGH, "Unknown Command: \"%s\"\n", cmd);
						
}
	
	
	//=================================Abilities========================================\\
	// Soldier = Haste																	\\
	// Infantry = Laser Trips (high explosive or something)								\\
	// Tank = Dual Rockets																\\
	// Medic = Earthquake																\\
	// Medic Commander = Shoots BFG around the player, 180 angle						\\
	// Gunner = Shoots force grenades													\\
	// Gladiator = Wall Piercing Railgun, trail has radius damage						\\
	// Supertank =	Airstrike															\\
	// Flyer = Death does huge fireball radius damage explosion							\\
	// Shark = Cyclone attack															\\
	// Floater = 2x frags																\\
	// Berserk = Shield (takes less damage)												\\
	// Iron Maiden = Triple rocket spread												\\
	// Brain = Prox Mine (Temp)															\\
	// Mutant = Turret																	\\
	// Parasite = Shoots a magical bullet tip, does 50 damage							\\
	// JORG = Lightning, shoots a Lightning Bolt.										\\
	// Icarus = Bolts will do explosive damage, EF_PLASMA								\\
	// BETA Soldier = Weapon projectile speeds (HyperGun & RipperGun) go 2x faster		\\
	// BETA Brain = Tentacles steal health												\\
	// BETA Iron Maiden = Plasma rockets												\\
	// BETA Supertank = IR Goggles														\\
	// BETA Gladiator = Shoots anti-matter cannon										\\
	// Gekk = Turret																	\\
	// Daedalus = Cloaking																\\
	// Stalker = HighJump and Homing Blaster											\\
	// Fixbot = Laser Fence																\\
	// Male Traitor = Gib Launcher (Exploding gibs)										\\
	// Female Traitor = Kamikaze														\\
	// Laser Gladiator = Freezer														\\
	//=================================Abilities========================================\\
