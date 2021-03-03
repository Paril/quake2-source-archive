/* g_cmds.c */

#include "g_local.h"
#include "m_player.h"
#include "wf_classmgr.h"//WF34
//ERASER START
#include "bot_procs.h"
///Q2 Camera Begin
#include "camclient.h"
///Q2 Camera End
#include "p_trail.h"
#include "stdlog.h"

void freeze_player(edict_t *ent);
void unfreeze_player(edict_t *ent);

void botRemovePlayer(edict_t *self);
//ERASER END
void cmd_PlasmaBomb(edict_t *ent);
void cmd_Camera(edict_t *ent);

//WF - Function prototypes
void WFClassCount(edict_t *self, int *classcount);
int BannedWords(edict_t *ent, char *str);

void Cmd_AutoConfig_f (edict_t *ent);
void cmd_Disguise (edict_t *ent);
void cmd_Sentry (edict_t *ent);
void cmd_Biosentry (edict_t *ent);
void Cmd_MapHelp_f (edict_t *ent);
void Cmd_ShowFriends_f (edict_t *ent);
void Cmd_Friend_f (edict_t *ent);
void cmd_Alarm(edict_t *ent);

//Ref functions
void Cmd_Ref_Show (edict_t *ent);
void Cmd_Ref_Password (edict_t *ent);
void Cmd_Ref_Kick (edict_t *ent);
void Cmd_Ref_NextMap (edict_t *ent);
void Cmd_Ref_PickMap (edict_t *ent);
void Cmd_Ref_Start (edict_t *ent);
void Cmd_Ref_Stop (edict_t *ent);
void Cmd_Ref_NoTalk (edict_t *ent);
void Cmd_Ref_Talk (edict_t *ent);
void Cmd_Ref_Skin_On (edict_t *ent);
void Cmd_Ref_Skin_Off (edict_t *ent);
void Cmd_Ref_Leave (edict_t *ent);

extern gitem_t *flag1_item;
extern gitem_t *flag2_item;

void I_AM_A_ZBOT(edict_t *ent);

//Global variables
int temp_ent_type = 0;

//WF
void CTFGrappleFire (edict_t *ent, vec3_t g_offset, int damage, int effect);

void cmd_Grapple(edict_t *ent)
{
	int		damage;

	damage = 10;
//	CTFGrappleFire (ent, vec3_origin, damage, 0);
  CTFGrappleFire2 (ent, vec3_origin, damage, 0);//newgrap 4/99
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
	return ++p;
}

qboolean OnSameTeam (edict_t *ent1, edict_t *ent2)
{
	if (ent1->wf_team == ent2->wf_team) return true;
	else return false;
}


void SelectNextItem (edict_t *ent, int itflags)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;

	cl = ent->client;

//ZOID
	if (cl->menu) 
	{
		PMenu_Next(ent);
		return;
	} 
	else if (cl->chase_target) 
	{
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
	if (cl->menu) 
	{
		PMenu_Prev(ent);
		return;
	} 
	else if (cl->chase_target) 
	{
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

void Cmd_ShowVotes_f(edict_t *ent)
{
	int i;
	for (i = 0; i < maplist.nummaps; ++i)
		safe_cprintf (ent, PRINT_HIGH, "%d. %s (%d votes)\n",
		   i, maplist.mapnames[i], maplist.votes[i]);
}

/*
==================
Cmd_Cloak_f
==================
*/

void Cmd_Cloak_f (edict_t *ent)
{
	if (ent->client->cloaking == 0)
	{
		gi.centerprintf (ent, "Cloaking Enabled!\n");
		ent->client->cloaktime = level.time + CLOAK_ACTIVATE_TIME;
		ent->client->cloaking = 1;
		ent->client->cloakdrain = 0;
	}
	else
	{
		gi.centerprintf (ent, "Cloaking Disabled!\n");
		ent->svflags &= ~SVF_NOCLIENT;
		ent->client->cloaking = 0;
	}
}

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

/*
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
*/
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


//Find out what slot (key number) is being used
int wfGetSlot(gitem_t *it)
{
	int slot;
	slot = 0;
	if(it == FindItem("blaster")) slot = 1;
	else if(it == FindItem("shotgun")) slot = 2;
	else if(it == FindItem("super shotgun")) slot = 3;
	else if(it == FindItem("Machinegun")) slot = 4;
	else if(it == FindItem("chaingun"))	slot = 5;
	else if(it == FindItem("grenade launcher")) slot = 6;
	else if(it == FindItem("rocket launcher")) slot = 7;
	else if(it == FindItem("hyperblaster")) slot = 8;
	else if(it == FindItem("railgun")) slot = 9;
	else if(it == FindItem("bfg10k")) slot = 10;
if (wfdebug) gi.dprintf("Slot = %d, class = %s\n", slot, it->classname);
	return (slot);
}

//Based on what slot is selected, return the right weapon number based on class
int wfGetWeaponNum(edict_t *ent, int slot)
{
	int classnum;
	int weapon;

	if (!ent->client) return 0;

	//Slot choosen, now select weapon
	classnum = ent->client->pers.player_class;
	if (slot)	//Things without a slot are not weapons
		weapon = classinfo[classnum].weapon[slot-1];
	else
		weapon = 0;

if (wfdebug) gi.dprintf("Weapon = %d, slot = %d\n", weapon, slot);

	return (weapon);
}

//Return the damage of the selected weapon
int wfWeaponDamage(gitem_t *it)
{
	int damage;

	if (it->tag && it->tag < WEAPON_COUNT)
		damage = wf_game.weapon_damage[it->tag];
	else
		damage = 0;

if (wfdebug) gi.dprintf("Damage of weapon %s (%d) = %d\n", it->classname, it->tag, damage);

	return (damage);
}



/*
==================
Cmd_Use_f

Use an inventory item
==================
*/
void Cmd_Use_f (edict_t *ent)
{
	int		index;
	gitem_t	*it;
	gitem_t	*orig_it;
	char	*s;
	char	*picked_item;
	int		slot;
//	int		classnum;
	int		weapon;
	int		foundmenuitem;
	int		olddamage;

	if (!ent->client)
	{
		gi.dprintf("Use: Not a client\n");
		return;
	}

	s = gi.args();
	it = FindItem (s);
	picked_item = s;
	if (!it)
	{
		safe_cprintf (ent, PRINT_HIGH, "unknown item: %s\n", s);
		return;
	}
	if (!it->use)
	{
		safe_cprintf (ent, PRINT_HIGH, "Item is not usable.\n");
		return;
	}

	olddamage = ent->client->weapon_damage;
	ent->client->weapon_damage = 0;


	//WF  - Switch weapons to custom weapons if needed
	orig_it = it;

	//Pick a slot
	slot = wfGetSlot(it);

	//If there is a menu open, we may need to pick an item from it
	//instead of choosing a weapon
	if (ent->client->menu && ent->client->menu->UseNumberKeys && slot > 0)
	{
		foundmenuitem = WFMenuFromNumberKey(ent, slot);
		if (foundmenuitem) return;	//We are done if we found a menu item
	}

	//Feign and Observer //newfeign 4/99 /newcmds
    if ((ent->client->pers.feign) || ((ent->movetype == MOVETYPE_NOCLIP)
		|| (ent->client->bIsCamera)))
        return;

	if (!ent->client->pers.player_class)
	{
//		gi.dprintf("Use: No current class selected\n");
		return;
	}


	weapon = wfGetWeaponNum(ent, slot);

if (wfdebug) gi.dprintf("Use: Slot=%d, weapon=%d\n", slot, weapon);

	if (weapon > 0 && weapon <= WEAPON_COUNT)
		ent->client->weapon_damage = wf_game.weapon_damage[weapon];

	if(weapon == WEAPON_BLASTER) 
	{
		it = FindItem("blaster");
		picked_item = "Blaster";
	}

	else if(weapon == WEAPON_SHOTGUN)
	{
		it = FindItem("shotgun");
		picked_item = "Shotgun";
	}

	else if(weapon == WEAPON_SUPERSHOTGUN)
	{
		it = FindItem("super shotgun");
		picked_item = "Super Shotgun";
	}

		
	else if (weapon == WEAPON_NAILGUN)
	{
		it = FindItem ("NailGun");
		picked_item = "Nail Gun";
	}

	else if (weapon == WEAPON_SHC)
	{
		it = FindItem ("SHC Rifle");
		picked_item = "SHC Rifle";
	}

	else if (weapon == WEAPON_TRANQUILIZER)
	{
		it = FindItem ("Tranquilizer");
		picked_item = "Tranquilizer"; 
	}

	else if (weapon == WEAPON_LRPROJECTILE)
	{
		it = FindItem ("Projectile Launcher");
		picked_item = "Projectile Launcher"; 
	}

	else if (weapon == WEAPON_INFECTEDDART)
	{
		it = FindItem ("Infected Dart Launcher");
		picked_item = "Infected Dart";

	}

	else if (weapon == WEAPON_ARMORDART)
	{
		it = FindItem ("Poison Dart Launcher");
		picked_item = "Armor Piercing Dart";
	}

	else if (weapon == WEAPON_SHOTGUNCHOKE)
	{
		it = FindItem ("Shotgun Choke");
		picked_item = "Shotgun with Choke";
	}

	else if(weapon == WEAPON_MACHINEGUN)
	{
		it = FindItem("Machinegun");
		picked_item = "Machine Gun";
	}

	else if (weapon == WEAPON_NAG)
	{
		it = FindItem ("Nervous Accelerator Gun");
		picked_item = "Nervous Accelerator Gun";
	}

	else if (weapon == WEAPON_LIGHTNING)
	{
		it = FindItem ("Lightning Gun");
		picked_item = "Lightning Gun";
	}

	else if (weapon == WEAPON_PULSE)
	{
		it = FindItem ("Pulse Cannon");
		picked_item = "Pulse Cannon";
	}

	else if (weapon == WEAPON_LASERSNIPER)
	{
		it = FindItem ("Laser Sniper Rifle");
		picked_item = "Laser Sniper Rifle";
	}

	else if(weapon == WEAPON_CHAINGUN)
	{       
		it = FindItem("Chaingun");
		picked_item = "Chaingun";
	}

	else if (weapon == WEAPON_NEEDLER)
	{
		it = FindItem ("needler");
		picked_item = "Needler";
	}

	else if (weapon == WEAPON_SNIPERRIFLE)
	{
		it = FindItem ("Sniper Rifle");
		picked_item = "Sniper Rifle";
	}

	else if (weapon == WEAPON_CLUSTERROCKET)
	{
		it = FindItem ("Cluster Rocket Launcher");
		picked_item = "Cluster Rocket Launcher";
	}
	else if (weapon == WEAPON_SENTRYKILLER)
	{
		it = FindItem ("Sentry Killer");
		picked_item = "Sentry Killer";
	}

	else if (weapon == WEAPON_MEGACHAINGUN)
	{
		it = FindItem ("Mega Chaingun");
		picked_item = "Mega Chaingun";
	}

	else  if(weapon == WEAPON_GRENADELAUNCHER)
	{	
		it = FindItem("grenade launcher");
		picked_item = "Grenade Launcher";
	}

	else if (weapon == WEAPON_PELLET)
	{
		it = FindItem ("Pellet Rocket Launcher");
		picked_item = "Pellet Rocket Launcher";
	}

	else if (weapon == WEAPON_MAGBOLTED)
	{
		it = FindItem ("Mag Bolted");
		picked_item = "Mag Bolted Blaster";
	}

	else if (weapon == WEAPON_FLAREGUN)
	{
		it = FindItem ("Flare Gun");
		picked_item = "Flare Gun"; 
	}

	else if(weapon == WEAPON_ROCKETLAUNCHER)
	{
		it = FindItem("rocket launcher");
		picked_item = "Rocket Launcher";
	}

	else if (weapon == WEAPON_NAPALMMISSLE)
	{
		it = FindItem ("Rocket Napalm Launcher");
		picked_item = "Rocket Napalm Launcher";
	}

	else if(weapon == WEAPON_HYPERBLASTER)
	{
		it = FindItem("hyperblaster");
		picked_item = "Hyperblaster";
	}

	else if (weapon == WEAPON_TELSA)
	{
		it = FindItem ("Telsa Coil");
		picked_item = "Telsa Coil";
	}

	else if(weapon == WEAPON_RAILGUN)
	{
		it = FindItem("railgun");
		picked_item = "Railgun";
	}

	else if (weapon == WEAPON_BFG)
	{                
		it = FindItem("bfg10k");
		picked_item = "BFG";
	}

	else if (weapon == WEAPON_FLAMETHROWER)
	{
		it = FindItem ("FlameThrower");
		picked_item = "FlameThrower";
	}
	else if (weapon == WEAPON_TRANQUILDART)
	{
		it = FindItem ("Tranquilizer Dart Launcher");
		picked_item = "Tranquilizer Dart";
	}
	else if (weapon == WEAPON_KNIFE)
	{
		it = FindItem ("Knife");
		picked_item = "Knife";
	}	
	else if (weapon == WEAPON_FREEZER)//acrid 3/99
	{
		it = FindItem ("Freezer");
		picked_item = "Freezer";
	}	
	else if (weapon == WEAPON_AK47)
	{
		it = FindItem ("AK47");
		picked_item = "AK47";
	}
	else if (weapon == WEAPON_PISTOL)
	{
		it = FindItem ("Pistol");
		picked_item = "Pistol";
	}

	else if (weapon == WEAPON_STINGER)
	{
		it = FindItem ("Stinger Launcher");
		picked_item = "Stinger Launcher";
	}
	else if (weapon == WEAPON_DISRUPTOR)
	{
		it = FindItem ("Disruptor");
		picked_item = "Disruptor";
	}
	else if (weapon == WEAPON_ETF_RIFLE)
	{
		it = FindItem ("ETF Rifle");
		picked_item = "ETF Rifle";
	}
/*
	else if (weapon == WEAPON_PLASMA_BEAM)
	{
		it = FindItem ("Plasma Beam");
		picked_item = "Plasma Beam";
	}
*/
	else if (weapon == WEAPON_ION_RIPPER)
	{
		it = FindItem ("Ionripper");
		picked_item = "Ion Ripper";
	}
	else if (weapon == WEAPON_PHALANX)
	{
		it = FindItem ("Phalanx");
		picked_item = "Phalanx";
	}
	if (it == NULL)
	{
		it = orig_it;
		safe_cprintf(ent, PRINT_HIGH, "Coudn't find that weapon!\n");        
	}

	//Is this weapon banned?
/*	if ((banweapon->string) && (it == FindItem(banweapon->string)))
	{
		safe_cprintf (ent, PRINT_HIGH, "Sorry - the %s is currently disabled.\n",banweapon->string);
		ent->client->weapon_damage = olddamage;
		return;
	}
*/

	if (slot != 0 && weapon == 0)
	{
		safe_cprintf (ent, PRINT_HIGH, "Nothing There.\n");
		ent->client->weapon_damage = olddamage;
		return;
	}

	index = ITEM_INDEX(it);
	if (!ent->client->pers.inventory[index])
	{
		safe_cprintf (ent, PRINT_HIGH, "Not In Inventory.\n");
		ent->client->weapon_damage = olddamage;
		return;
	}
	safe_cprintf(ent, PRINT_HIGH, "%s Selected\n",picked_item);

	it->use (ent, it);

	//They may have done a "use <weapon name>" directly instead of
	// the number keys
	if(it == FindItem("Rocket Napalm Launcher")) weapon = wf_game.weapon_damage[WEAPON_NAPALMMISSLE];

	//Turn on/off laser sight if needed.  Not on if homing is not on
	if ((weapon == WEAPON_NAPALMMISSLE) && ( ent->client->pers.homing_state) )
		lasersight_on (ent);
	else
		lasersight_off (ent);

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

//WF - Drop the flag
	if (Q_stricmp(gi.args(), "flag") == 0 ) 
	{
		CTFDeadDropFlag(ent);
		return;
	}
//WF
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
	if (ent->client->menu) 
	{
		PMenu_Close(ent);
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
	//has team been picked?
	if (ctf->value && cl->resp.ctf_team == CTF_NOTEAM) 
	{
		CTFOpenJoinMenu(ent);
		return;
	}
//ZOID
//WF
	//has class been picked?
	if (ent->client->pers.player_class == 0) 
	{
		WFOpenClassMenu(ent);
		return;
	}
//WF

	cl->showinventory = true;
	cl->showscores = false;

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
	if (ent->client->menu) 
	{
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
Cmd_WeapPrev_f
=================
*/
void Cmd_WeapPrev_f (edict_t *ent)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;
	int			selected_weapon;
	int	slot	= 0;
	int weapon	= 0;

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
		{
			return;	// successful
		}
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
	int	slot	= 0;
	int weapon	= 0;

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
		{
			return;	// successful
		}
	}
}

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

	int	slot	= 0;
	int weapon	= 0;

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
	//No kill cmd while disease 3/99 acrid
	//Acrid - I'm allowing you to kill yourself, but the nurse gets a frag for it.  See
	//        p_client.c
//    if (ent->disease)
//	{
//		if (((int)wfflags->value & WF_NO_PLAYER_CLASSES) == 0)
//		safe_cprintf (ent, PRINT_HIGH, "Take it like a %s\n", classinfo[ent->client->pers.player_class].name);
//		return;
//	}
	if((level.time - ent->client->respawn_time) < 5)
		return;
	ent->flags &= ~FL_GODMODE;
	ent->health = 0;
	meansOfDeath = MOD_SUICIDE;

	//WF - Treat this like a change class. Clean up all their toys
	WFPlayer_ChangeClassTeam(ent);

	player_die (ent, ent, ent, 100000, vec3_origin);

	// don't even bother waiting for death frames
//3.20	ent->deadflag = DEAD_DEAD;
//3.20	respawn (ent);
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
		PMenu_Close(ent);
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

//WF - prototype from wf_decoy.c
void decoy_taunt (edict_t *self);
void decoy_flip (edict_t *self);
void decoy_salute (edict_t *self);
void decoy_wave (edict_t *self);
void decoy_point (edict_t *self);


/*
=================
Cmd_NoSpam_f
=================
*/
void Cmd_NoSpam_f (edict_t *ent)
{
	int		i;
	if (!ent->client) return;

	i = atoi (gi.argv(1));
	ent->client->pers.nospam_level = i;
	safe_cprintf (ent, PRINT_HIGH, "NOSPAM set to %d\n", i);
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

		//WF - if we have a decoy, make it flipoff too
		if (ent->decoy) decoy_flip(ent->decoy);
		break;
	case 1:
		safe_cprintf (ent, PRINT_HIGH, "salute\n");
		ent->s.frame = FRAME_salute01-1;
		ent->client->anim_end = FRAME_salute11;

		//WF - if we have a decoy, make it salute too
		if (ent->decoy) decoy_salute(ent->decoy);
		break;
	case 2:
		safe_cprintf (ent, PRINT_HIGH, "taunt\n");
		ent->s.frame = FRAME_taunt01-1;
		ent->client->anim_end = FRAME_taunt17;

		//WF - if we have a decoy, make it taunt too
		if (ent->decoy) decoy_taunt(ent->decoy);
		break;
	case 3:
		safe_cprintf (ent, PRINT_HIGH, "wave\n");
		ent->s.frame = FRAME_wave01-1;
		ent->client->anim_end = FRAME_wave11;

		//WF - if we have a decoy, make it wave too
		if (ent->decoy) decoy_wave(ent->decoy);
		break;
	case 4:
	default:
		safe_cprintf (ent, PRINT_HIGH, "point\n");
		ent->s.frame = FRAME_point01-1;
		ent->client->anim_end = FRAME_point12;

		//WF - if we have a decoy, make it point too
		if (ent->decoy) decoy_point(ent->decoy);
		break;
	}
}

/*
=================
Cmd_Join_f
=================
*/
//ERASER START
void ClientUserinfoChanged (edict_t *ent, char *userinfo);
void Cmd_Join_f(edict_t *ent, char *teamname)
{
	int i;
	char	userinfo[MAX_INFO_STRING];

	if (ctf->value)
	{
		safe_cprintf(ent, PRINT_HIGH, "\nTeams not available in CTF mode.\nUse \"sv bluebots <name1> <name2> ..\" and \"sv bluebots <name1> <name2> ..\" to spawn groups of bots in CTF.\n");
		return;
	}

	if (ent->client->team)
	{
		safe_cprintf(ent, PRINT_HIGH, "\nYou are already a member of a team.\nYou must disconnect to change teams.\n\n");
		return;
	}

	for (i=0; i<MAX_TEAMS; i++)
	{
		if (!bot_teams[i])
		{
			safe_cprintf(ent, PRINT_HIGH, "Team \"%s\" does not exist.\n", teamname);
			return;
		}

		if (!bot_teams[i]->ingame && dedicated->value)
			continue;		// only allow joining a team that has been created on the server

#ifdef _WIN32
		if (!_stricmp(bot_teams[i]->teamname, teamname) || !_stricmp(bot_teams[i]->abbrev, teamname))
#else
		if (!strcasecmp(bot_teams[i]->teamname, teamname) || !strcasecmp(bot_teams[i]->abbrev, teamname))
#endif
		{	// match found

			// check team isn't already full
			if ((bot_teams[i]->num_players >= players_per_team->value) && (bot_teams[i]->num_bots == 0))
			{
				safe_cprintf(ent, PRINT_HIGH, "Team \"%s\" is full.\n", bot_teams[i]->teamname);
				return;
			}

			bot_teams[i]->num_players++;

			if (strlen(bot_teams[i]->default_skin) > 0)	// set the team's skin
			{//botDebugPrint("SKIN 3 (ACRID)\n");
				// copy userinfo
				strcpy(userinfo, ent->client->pers.userinfo);

				// set skin
				Info_SetValueForKey (userinfo, "skin", bot_teams[i]->default_skin);
    //botDebugPrint("SKIN 1 (ACRID)\n");
				// record change
				ClientUserinfoChanged(ent, userinfo);
			}

			// must set this after skin!
			ent->client->team = bot_teams[i];
			bot_teams[i]->ingame = true;		// make sure we enable the team

			my_bprintf(PRINT_HIGH, "%s has joined team %s\n", ent->client->pers.netname, bot_teams[i]->teamname);

    		sl_LogPlayerTeamChange( &gi,
				ent->client->pers.netname,
				CTFTeamName(ent->client->resp.ctf_team));
			return;
		}
	}
}

void Cmd_Lag_f (edict_t *ent, char *val)
{
	int i;

	i = atoi(val);

	if (i > 0)
	{
		if (i < 1000)
		{
			ent->client->latency = i;
			safe_cprintf(ent, PRINT_HIGH, "Latency set to %i\n", i);
		}
		else
		{
			safe_cprintf(ent, PRINT_HIGH, "lag must be lower than 1000\n");
		}
	}
	else
	{
		safe_cprintf(ent, PRINT_HIGH, "lag must be higher than 0\n");
	}
}

void Cmd_Teams_f (edict_t *ent)
{
	char str[256];
	int i, j;

	if (ctf->value)
	{	
		safe_cprintf(ent, PRINT_HIGH, "\nTeams not available in CTF mode.\nUse \"sv bluebots <name1> <name2> ..\" and \"sv redbots <name1> <name2> ..\" to spawn groups of bots in CTF.\n\n");
		return;
	}

	safe_cprintf(ent, PRINT_HIGH, "\n=====================================\nAvailable teams:\n\n");

	// list all available teams
	for (i=0; i<MAX_TEAMS; i++)
	{
		if (!bot_teams[i])
			break;

		if (!bot_teams[i]->ingame && dedicated->value)
			continue;	// don't show teams that haven't been created, when in dedicated server mode

		// print the team name
		safe_cprintf(ent, PRINT_HIGH, "%s ", bot_teams[i]->teamname);

		for (j=0; j<(15-strlen(bot_teams[i]->teamname)); j++)
			str[j] = ' ';
		str[j] = 0;

		safe_cprintf(ent, PRINT_HIGH, "%s(%s)", str, bot_teams[i]->abbrev);

		for (j=0; j<(4-strlen(bot_teams[i]->abbrev)); j++)
			str[j] = ' ';
		str[j] = 0;

		safe_cprintf(ent, PRINT_HIGH, str);

		if (bot_teams[i]->ingame)
		{
			safe_cprintf(ent, PRINT_HIGH, "%i plyrs", bot_teams[i]->num_players);
			if (bot_teams[i]->num_bots)
				safe_cprintf(ent, PRINT_HIGH, " (%i bots)\n", bot_teams[i]->num_bots);

			safe_cprintf(ent, PRINT_HIGH, "\n");
		}
		else
			safe_cprintf(ent, PRINT_HIGH, "[none]\n");

	}

	safe_cprintf(ent, PRINT_HIGH, "\n=====================================\n");
}

void Cmd_BotCommands_f	(edict_t	*ent)
{	// show bot info
	gi.dprintf("\n=================================\nSERVER ONLY COMMANDS:\n\n \"bot_num <n>\" - sets the maximum number of bots at once to <n>\n\n \"bot_name <name>\" - spawns a specific bot\n\n \"bot_free_clients <n>\" - makes sure there are always <n> free client spots\n\n \"bot_calc_nodes 0/1\" - Enable/Disable dynamic node-table calculation\n\n \"bot_allow_client_commands <n>\" - set to 1 to allow clients to spawn bots via \"cmd bots <n>\"\n=================================\n\n");
}

void Cmd_Tips_f (edict_t *ent)
{	
	safe_cprintf(ent, PRINT_HIGH, "\nERASER TIPS:\n\n * Set \"skill 0-3\" to vary the difficulty of your opponents (1 is default)\n\n * You can create your own bots by editing the file BOTS.CFG in the Eraser directory\n\n * Set \"maxclients 32\" to allow play against more bots\n\n");
}

void Cmd_Botpath_f (edict_t *ent)
{
	trace_t tr;
	vec3_t	dest;

	AngleVectors(ent->client->v_angle, dest, NULL, NULL);
	VectorScale(dest, 600, dest);
	VectorAdd(ent->s.origin, dest, dest);

	tr = gi.trace(ent->s.origin, VEC_ORIGIN, VEC_ORIGIN, dest, ent, MASK_PLAYERSOLID);

	if (tr.ent && tr.ent->bot_client)
	{
		tr.ent->flags |= FL_SHOWPATH;

		gi.dprintf("Showing path for %s\n", tr.ent->client->pers.netname);
	}
}

// toggles the debug path for this client
void Cmd_Showpath_f (edict_t *ent)
{
	if (ent->flags & FL_SHOWPATH)
		ent->flags -= FL_SHOWPATH;
	else
		ent->flags |= FL_SHOWPATH;
}
//ERASER END
//WF34 START
/* 
Flood Protection (anti-spam) code by John.  Moved to function by Gregg
*/
int FloodProtect(edict_t *ent)
{
	int		floodtime;

	//See if flood protection is turned off
	if (wf_game.floodertime) return 0;//dont worry about it

	if (ent->client)
	{

		//WF John flood protection code
		if(ent->client->floodtime2<level.time)
		{
			if(ent->client->floodtime1>level.time)
				ent->client->floodtime2 = level.time + wf_game.floodertime;
		}
		else
		{
			floodtime=(int)(ent->client->floodtime2-level.time);
			safe_cprintf(ent, PRINT_HIGH, "Cannot talk for %i more seconds\n",floodtime);
			return 1;
		}
	
		ent->client->floodtime1 = level.time + wf_game.floodertime;
	}

	return 0;
}
//WF34 END
/*
==================
Cmd_Say_f
==================
*/
void Cmd_Say_f (edict_t *ent, qboolean team, qboolean arg0)
{
	int		j;
	edict_t	*other;
	char	*p;
	char	text[2048];



	if (FloodProtect(ent))
		return;
	
	if (gi.argc () < 2 && !arg0)
		return;

	if (!((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS)))
		team = false;

	if (team)
		Com_sprintf (text, sizeof(text), "(%s): ", ent->client->pers.netname);
	else
		Com_sprintf (text, sizeof(text), "%s: ", ent->client->pers.netname);

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

	if (BannedWords(ent, text))
		return;

	j = 0;
	while (j < 150 && text[j] != 0)
	{
		if (text[j] == '%') text[j] = '_';		// don't allow formated characters in text
		++j;
	}


	if (dedicated->value)
		safe_cprintf(NULL, PRINT_CHAT, "%s", text);

	for (j = 1; j <= game.maxclients; j++)
	{
		other = &g_edicts[j];
		if (!other->inuse)
			continue;
		if (!other->client || other->bot_client)
			continue;
		if (team)
		{
			if (!OnSameTeam(ent, other))
				continue;
		}
		safe_cprintf(other, PRINT_CHAT, "%s", text);
	}
}
//ERASER START
//==========================================================
// CTF flagpath hack, provide alternate routes for bots to return
// the flag to base
void FlagPathTouch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (!other->bot_client)
		return;

	if (ent->last_goal)	// this is a source flagpath
	{
		if (other->client->resp.ctf_team != ent->skill_level)
			return;
		if (other->flagpath_goal)	// already heading for a destination
			return;
	}
	else	// this is a destination
	{
		if (other->flagpath_goal == ent)		// reached destination, so clear it
			other->flagpath_goal = NULL;

		return;
	}

	if (!CarryingFlag(other))
		return;

	// carring flag, if this is a src path, send them on an alternate (safest) route
	if (ent->last_goal)
	{
		float count1, count2;
		int i;

		if (!other->flagpath_goal)
		{
			count1 = count2 = 0;

			for (i=0; i<num_players; i++)
			{
				if (players[i]->client->resp.ctf_team && (players[i]->client->resp.ctf_team != other->client->resp.ctf_team))
				{
					count1 += entdist(ent->last_goal, players[i]);
					count2 += entdist(ent->target_ent, players[i]);
				}
			}

			if (count1 > count2)
				other->flagpath_goal = ent->last_goal;
			else
				other->flagpath_goal = ent->target_ent;
		}

	}
	else	// this is a destination path
	{
		other->flagpath_goal = NULL;
	}
}

extern int dropped_trail;

edict_t *flagpaths[3] = {NULL, NULL, NULL};
void FlagPath(edict_t *ent, int ctf_team)
{
	int i;
	static int flagpath_type=0;

	if (flagpath_type == 0)
	{
		// new source
		if (flagpaths[flagpath_type])
		{
			if (ent->client)
			{
				safe_cprintf(ent, PRINT_HIGH, "Incomplete FlagPath, starting a new path.\n");

				for (i=0; i<3; i++)
				{
					G_FreeEdict(flagpaths[i]);
					flagpaths[i] = NULL;
				}
			}
		}

		flagpaths[flagpath_type] = G_Spawn();
		flagpaths[flagpath_type]->classname = "flag_path_src";
		VectorCopy(ent->s.origin, flagpaths[flagpath_type]->s.origin);
		flagpaths[flagpath_type]->last_goal = NULL;
		flagpaths[flagpath_type]->target_ent = NULL;
		flagpaths[flagpath_type]->skill_level = ctf_team;

		safe_cprintf(ent, PRINT_HIGH, "Flagpath SOURCE dropped.\n");
	}
	else if (flagpath_type == 1)
	{

		flagpaths[flagpath_type] = G_Spawn();
		flagpaths[flagpath_type]->classname = "flag_path_dest";
		VectorCopy(ent->s.origin, flagpaths[flagpath_type]->s.origin);
		flagpaths[flagpath_type]->last_goal = NULL;
		flagpaths[flagpath_type]->target_ent = NULL;
		flagpaths[flagpath_type]->paths[0] = -1;

		flagpaths[0]->last_goal = flagpaths[flagpath_type];

		safe_cprintf(ent, PRINT_HIGH, "Flagpath DEST 1 dropped.\n");
	}
	else if (flagpath_type == 2)
	{
		flagpaths[flagpath_type] = G_Spawn();
		flagpaths[flagpath_type]->classname = "flag_path_dest";
		VectorCopy(ent->s.origin, flagpaths[flagpath_type]->s.origin);
		flagpaths[flagpath_type]->last_goal = NULL;
		flagpaths[flagpath_type]->target_ent = NULL;
		flagpaths[flagpath_type]->paths[0] = -1;

		flagpaths[0]->target_ent = flagpaths[flagpath_type];

		// completed paths, so make them triggers, and clear
		for (i=0; i<3; i++)
		{
			flagpaths[i]->solid = SOLID_TRIGGER;
			VectorSet(flagpaths[i]->mins, -16, -16, -16);
			VectorSet(flagpaths[i]->maxs,  16,  16,  4);
			flagpaths[i]->touch = FlagPathTouch;
			gi.linkentity(flagpaths[i]);

			flagpaths[i] = NULL;
		}

		flagpath_type = -1;

		if (ent->client)
		{
			safe_cprintf(ent, PRINT_HIGH, "Flagpath DEST 2 dropped.\nSequence complete.\n\n");
			dropped_trail = true;
		}
	}

	flagpath_type++;
}
extern float	team1_rushbase_time, team2_rushbase_time;	// used by RUSHBASE command
extern float	team1_defendbase_time, team2_defendbase_time;
#define	RUSHBASE_OVERRIDE_TIME	180.0
//ERASER END
/*/newcmds acrid
====================
ClientCommandModeUse
====================
*/
void ClientCommandModeUse (edict_t *ent,char *cmd)
{
	cmd = gi.argv(0);

//GREGG havent looked at zbot code so I added these to be safe, because this bypasses the cmds list
	// -- ANTI-ZBOT STARTS --
	/*
	if (Q_stricmp(cmd, "!zbot") == 0)
		ent->client->resp.bot_end = 0; // not a bot - test 1
	else if (Q_stricmp(cmd, "@zbot") == 0)
		ent->client->resp.bot_end3 = 0; // not a bot - test 2
	else if (Q_stricmp(cmd, "#zbot") == 0)
		ent->client->resp.bot_end3 = 0; // not a bot - test3
	*/
	// -- ANTI-ZBOT CODE ENDS -- //

	//Feign can use the special menu to live again newfeign 4/99 acrid
    if (ent->client->pers.feign)
	{
		if (Q_stricmp (cmd, "special") == 0) 
		{ 
			if (ent->health > 0 && ent->wf_team >= 1) 
					WFSpecialMenu(ent);
 		}
		else if (Q_stricmp (cmd, "vote") == 0) 
			{ 
				if (ent->client->pers.HasVoted == false)
				{
					WFMapVote(ent);
				}
				else
				safe_cprintf (ent, PRINT_HIGH, "You have already voted for a map.\n");
 			}
		//commands you can only use if menu open
        else if (ent->client->menu && ent->client->menu->UseNumberKeys)
				{		
					if (Q_stricmp (cmd, "use") == 0)
					Cmd_Use_f (ent);
					else if (Q_stricmp (cmd, "invuse") == 0)
					Cmd_InvUse_f (ent);
				}
	    //else print you can't you
		else
                safe_cprintf (ent, PRINT_HIGH, "You can't use the %s command in Feign mode.\n",cmd);
	}

	//Observer & Chasecam Commands 4/99
    if (ent->movetype == MOVETYPE_NOCLIP)
		{

		if (Q_stricmp (cmd, "inven") == 0)
			Cmd_Inven_f (ent);
		else if (Q_stricmp (cmd, "invnext") == 0)
			SelectNextItem (ent, -1);
		else if (Q_stricmp (cmd, "invprev") == 0)
			SelectPrevItem (ent, -1);
		
		//Commands you can only use if menu open
	    else if (ent->client->menu && ent->client->menu->UseNumberKeys)
			{		
			if (Q_stricmp (cmd, "use") == 0)
				Cmd_Use_f (ent);
			else if (Q_stricmp (cmd, "invuse") == 0)
				Cmd_InvUse_f (ent);
			}
	    //else print you can't you
		else
			safe_cprintf (ent, PRINT_HIGH, "You can't use the %s command in Observer mode. Use TAB key to join team\n",cmd);
	}


	//Bot CAM Commands
    if (ent->client->bIsCamera)
	{
        //BotCam
        if (Q_stricmp (cmd, "cam") == 0)
			{   
			//Acrid 3/99 prevent cam while using chasecam
			if(ent->client->chase_target)
				safe_cprintf (ent, PRINT_HIGH, "You must leave chasecam first.\n");
			else
			CameraCmd(ent,gi.argv(1));
			}
        //else print you can't you
	    else
			safe_cprintf (ent, PRINT_HIGH, "You can't use the %s command in CAM mode.\n",cmd);
	}
}
/*
=================
ClientCommand
=================
*/


void ClientCommand (edict_t *ent)
{

	int classcount[MAX_CLASSES + 1];

	char	*cmd;
	char	*args;
	int i;
	int class_picked;

	if (!ent->client /*|| ent->bot_client*/)
		return;		// not fully in game yet

	if( ent->frozen ) //acrid 3/19 no cmds if frozen
		return;	

	if (level.intermissiontime)
		return;

	cmd = gi.argv(0);

/////////////////General Commands you can use in any mode//////////////////
///////////////////////////Acrid 4/99//////////////////////////////////////
	//Feign 
	if (Q_stricmp (cmd, "feign") == 0)
	{ 
		if ((ent->client->player_special & SPECIAL_FEIGN)
			&& (ent->wf_team >= 1)
			&& (ent->health > 0))
			Cmd_Feign_f(ent);
		else
			safe_cprintf (ent, PRINT_HIGH, "Sorry, you are not allowed to feign death.\n");	
		return;
	}
	else if (Q_stricmp (cmd, "players") == 0)
	{
		Cmd_Players_f (ent);
		return;
	}
	else if (Q_stricmp (cmd, "showplayers") == 0)
	{
		Cmd_ShowPlayers(ent);
		return;//4/99
	}
	else if (Q_stricmp (cmd, "showclass") == 0)
	{
		Cmd_ShowClass(ent);
		return;//4/99
	}
	else if (Q_stricmp (cmd, "showvotes") == 0)
	{
        Cmd_ShowVotes_f (ent); 
		return;//4/99
	}
	else if (Q_stricmp (cmd, "score") == 0)
	{
		Cmd_Score_f (ent);
		return;
	}
	else if (Q_stricmp (cmd, "help") == 0)
	{
		Cmd_Help_f (ent);
		return;
	}

	else if (Q_stricmp (cmd, "maphelp") == 0)
	{
		Cmd_MapHelp_f (ent);
		return;
	}
	else if (Q_stricmp (cmd, "say") == 0)
	{
		if (ent->client->silenced)
		{
			safe_cprintf(ent, PRINT_HIGH, "The REF says you can't talk!\n");
			return;
		}

		Cmd_Say_f (ent, false, false);
		return;
	}
	//==============  START REF COMMANDS =================
    else if (Q_stricmp (cmd, "ref_password") == 0)
	{
		Cmd_Ref_Password(ent);
		return;
	}
    else if (Q_stricmp (cmd, "ref") == 0)
	{
		Cmd_Ref_Show(ent);
		return;
	}
    else if (Q_stricmp (cmd, "ref_kick") == 0)
	{
		Cmd_Ref_Kick(ent);
		return;
	}
    else if (Q_stricmp (cmd, "ref_nextmap") == 0)
	{
		Cmd_Ref_NextMap(ent);
		return;
	}
    else if (Q_stricmp (cmd, "ref_pickmap") == 0)
	{
		Cmd_Ref_PickMap(ent);
		return;
	}
    else if (Q_stricmp (cmd, "ref_start") == 0)
	{
		Cmd_Ref_Start(ent);
		return;
	}
    else if (Q_stricmp (cmd, "ref_stop") == 0)
	{
		Cmd_Ref_Stop(ent);
		return;
	}
    else if (Q_stricmp (cmd, "ref_talk") == 0)
	{
		Cmd_Ref_Talk(ent);
		return;
	}
    else if (Q_stricmp (cmd, "ref_notalk") == 0)
	{
		Cmd_Ref_NoTalk(ent);
		return;
	}
    else if (Q_stricmp (cmd, "ref_skin_on") == 0)
	{
		Cmd_Ref_Skin_On(ent);
		return;
	}
    else if (Q_stricmp (cmd, "ref_skin_off") == 0)
	{
		Cmd_Ref_Skin_Off(ent);
		return;
	}
    else if (Q_stricmp (cmd, "ref_leave") == 0)
	{
		Cmd_Ref_Leave(ent);
		return;
	}
    //same as team play, but goes out to everyone
    else if (Q_stricmp (cmd, "ref_play") == 0) 
    {
		if (FloodProtect(ent))
			return;

        args=gi.args();
		if (strlen(args) > 50)
		{
			return;
		}

		//Dont allow ref sounds to play
		if (wf_game.ref_ent != ent) 
		{
			safe_cprintf(ent, PRINT_HIGH, "Only the REF can use this command!\n");
			return;
		}

		
		Cmd_WFPlayTeam(ent, args, 1);
		return;
	}
	//==============  END REF COMMANDS =================

	else if (Q_stricmp (cmd, "say_team") == 0 || Q_stricmp (cmd, "steam") == 0)
	{
		if (ent->client->silenced)
		{
			safe_cprintf(ent, PRINT_HIGH, "The REF says you can't talk!\n");
			return;
		}

		if (ctf->value)
			CTFSay_Team(ent, gi.args());
		else//ERASER
			Cmd_Say_f (ent, true, false);
		return;
	}
	else if (Q_stricmp (cmd, "team") == 0)
	{
		CTFTeam_f (ent);
		return;
	}
	else if (Q_stricmp(cmd, "id") == 0) 
	{
		CTFID_f (ent);
		return;
	}
    // Help command
    else if (Q_stricmp (cmd, "wfhelp") == 0)
	{
		WFShowHelp(ent, NULL);
		return;
	}

    // wfflags command
    else if (Q_stricmp (cmd, "wfflags") == 0)
	{
		Cmd_WFFlags_f(ent);
		return;
	}
    else if (Q_stricmp (cmd, "vote") == 0) 
    { 
		if (ent->client->pers.HasVoted == false)
		{
			WFMapVote(ent);
		}
		else
			safe_cprintf (ent, PRINT_HIGH, "You have already voted for a map.\n");
		return;
 	}  
	
    //play the requested sound
    else if (Q_stricmp (cmd, "wfplay") == 0)
    {
		if (ent->client->silenced)
		{
			safe_cprintf(ent, PRINT_HIGH, "The REF says you can't play sounds!\n");
			return;
		}

		if (FloodProtect(ent))
			return;

        args = gi.args();
		if (strlen(args) > 50)
		{
			return;
		}

		//Dont allow ref sounds to play
		if (wf_game.ref_ent != ent) 
		{
			if ((strstr(args, "ref\\")) || (strstr(args, "ref/")))
			{
				safe_cprintf(ent, PRINT_HIGH, "Only the REF can play those sounds!\n");
				return;
			}
		}

		
//		gi.sound (ent, CHAN_VOICE, gi.soundindex (args), 1, ATTN_NORM, 0);
		gi.sound (ent, CHAN_AUTO, gi.soundindex (args), 1, ATTN_NORM, 0);
		return;
	}  
    //play the requested sound
    else if (Q_stricmp (cmd, "wfteamplay") == 0) 
    {
		if (ent->client->silenced)
		{
			safe_cprintf(ent, PRINT_HIGH, "The REF says you can't play sounds!\n");
			return;
		}

		if (FloodProtect(ent))
			return;

        args=gi.args();
		if (strlen(args) > 50)
		{
			return;
		}

		//Dont allow ref sounds to play
		if (wf_game.ref_ent != ent) 
		{
			if ((strstr(args, "ref\\")) || (strstr(args, "ref/")))
			{
				safe_cprintf(ent, PRINT_HIGH, "Only the REF can play those sounds!\n");
				return;
			}
		}

		
		Cmd_WFPlayTeam(ent, args, 0);
		return;
	}
	//no spam setting.  arg = number
	else if (Q_stricmp (cmd, "nospam") == 0)
	{
		Cmd_NoSpam_f (ent);
		return;
	}

    // 'autoconfig' command
	// arg = none, "on" or "off"
    else if (Q_stricmp (cmd, "autoconfig") == 0)
	{
        Cmd_AutoConfig_f (ent);
		return;
	}

    else if (Q_stricmp (cmd, "autozoom") == 0)
	{
		if (ent->client->pers.autozoom)
		{
			ent->client->pers.autozoom=0;
			ent->client->ps.fov = 90;
		}
		else
			ent->client->pers.autozoom=1;
		return;
	}

    //If in feign or observer mode use these /newcmds
    if ((!sv_cheats->value) && ( (ent->client->pers.feign) || 
		(ent->movetype == MOVETYPE_NOCLIP) || (ent->client->bIsCamera)))
	{
		ClientCommandModeUse (ent ,cmd);
		return;
	}
////////////////////////END of General Commands//////////////////////////// 
///////////////////////////////////////////////////////////////////////////
	if (Q_stricmp (cmd, "use") == 0)
		Cmd_Use_f (ent);
	else if (Q_stricmp (cmd, "invuse") == 0)
		Cmd_InvUse_f (ent);
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
	else if (Q_stricmp (cmd, "wave") == 0)
		Cmd_Wave_f (ent);
	else if (Q_stricmp (cmd, "changeclass") == 0)
	{
		if (ent->wf_team >= 1)
			WFOpenClassMenu(ent);
	}

	// -- ANTI-ZBOT STARTS -- //WF34
	/*
	else if (Q_stricmp(cmd, "!zbot") == 0)
		ent->client->resp.bot_end = 0; // not a bot - test 1
	else if (Q_stricmp(cmd, "@zbot") == 0)
		ent->client->resp.bot_end3 = 0; // not a bot - test 2
	else if (Q_stricmp(cmd, "#zbot") == 0)
		ent->client->resp.bot_end3 = 0; // not a bot - test3
		*/
	// -- ANTI-ZBOT CODE ENDS -- //

    ///Q2 Camera Begin //acrid 3/99 botcam
	else if (Q_stricmp (cmd, "cam") == 0)
    {   //Acrid 3/99 prevent cam while using chasecam
		if(ent->client->chase_target)
			safe_cprintf (ent, PRINT_HIGH, "You must leave chasecam first.\n");
	else
		CameraCmd(ent,gi.argv(1));
//orig 3/99        if (CameraCmd(ent))
// orig 3/99			botRemovePlayer(ent);	// Ridah, must remove from list first (make sure to add to list, if ability to change back to player is added)
    }
    ///Q2 Camera End

/*===========
WF Commands 
=============*/
    // 'homing' command
    else if (Q_stricmp (cmd, "homing") == 0)
        Cmd_Homing_f (ent);

    // 'grenade' command
    else if (Q_stricmp (cmd, "grenade") == 0)
	{
		if (ent->health > 0 && ent->wf_team >= 1) 
	        Cmd_Grenade_f (ent);
	}

    // 'grenade1' command
    else if (Q_stricmp (cmd, "grenade1") == 0)
	{
		if (ent->health > 0 && ent->wf_team >= 1) 
		{
	        Cmd_Grenade1 (ent);
		}
	}

    // 'grenade2' command
    else if (Q_stricmp (cmd, "grenade2") == 0)
	{
		if (ent->health > 0 && ent->wf_team >= 1) 
	        Cmd_Grenade2 (ent);
	}

    // 'grenade3' command
    else if (Q_stricmp (cmd, "grenade3") == 0)
	{
		if (ent->health > 0 && ent->wf_team >= 1) 
	        Cmd_Grenade3 (ent);
	}

    // 'decoy' command
    else if (Q_stricmp (cmd, "decoy") == 0)
	{
if (wfdebug) gi.dprintf("team = %d, resp team = %d\n", ent->wf_team, ent->client->resp.ctf_team);

		   if (ent->health > 0 && ent->wf_team >= 1) 
	        SP_Decoy (ent);	
	}

    // jetpack thrust
    else if (Q_stricmp(cmd, "thrust") == 0 )
        Cmd_Thrust_f (ent);     

	// Scanner
//    else if (Q_stricmp (cmd, "scanner") == 0)
//        Toggle_Scanner(ent);	

	// Compass 5/99
//    else if (Q_stricmp (cmd, "compass") == 0)
  //      Toggle_Compass(ent,gi.argv(1));

    // detpipes command
    else if (Q_stricmp (cmd, "detpipes") == 0)
        Cmd_DetPipes_f (ent);

    // reno commands
    else if (Q_stricmp (cmd, "reno") == 0)
        Cmd_Reno_f (ent);

    // decoy skins commands
    //else if (Q_stricmp (cmd, "dskin") == 0)
    //    Cmd_DSkin_f (ent);

	//Grenade turrets
//	else if (Q_stricmp (cmd, "turret") == 0)
//		Cmd_Turret_f (ent);

//showclass 4/99
//showplayers 4/99



    else if (Q_stricmp (cmd, "special") == 0) 
    { 
		if (ent->health > 0 && ent->wf_team >= 1) 
				WFSpecialMenu(ent);
 	}  
    //grenade dump (show active grenades)
    else if (Q_stricmp (cmd, "gdump") == 0) 
    {
		for (i = 1; i <= GRENADE_TYPE_COUNT; ++i)
			gi.dprintf("%d. %d\n",i,ent->client->pers.active_grenades[i]);
	}
	else if (Q_stricmp (cmd, "snipezoom") == 0)
	{

		ent->PlayerSnipingZoom= atoi(gi.argv(1));
		if (ent->PlayerSnipingZoom > 35)
			ent->PlayerSnipingZoom =35;
		else if (ent->PlayerSnipingZoom <25)
			ent->PlayerSnipingZoom = 25;
	}
	//simulate the removal of flags from a map
//	else if (Q_stricmp (cmd, "debug-killflags") == 0)
//	{
//		ent->client->pers.inventory[ITEM_INDEX(flag1_item)] = 0;
//		ent->client->pers.inventory[ITEM_INDEX(flag2_item)] = 0;
//	}

	//zbot debugging command
	else if (Q_stricmp (cmd, "iamazbot") == 0)
	{
		I_AM_A_ZBOT(ent);
	}


//	else if (Q_stricmp (cmd, "tripbomb") == 0)	
//	{
//		cmd_TripBomb(ent);
//	}
//	cant track down the other grapple, cmd grapple is 1 at console
	else if (Q_stricmp (cmd, "grapple") == 0)
	{
		//jR The great bug fix
		
		if (ent->health <= 0)//acrid added = 4/99
		{
			return;
		}
		if (ent->movetype == MOVETYPE_NOCLIP)
			return;

		//Has server turned off grappling on server? WF34
		if ((int)wfflags->value & WF_NO_GRAPPLE) 
		{
			safe_cprintf(ent, PRINT_HIGH, "Sorry - The grapple is turned off on this server.\n");
			return;
		}
		//Is this class capable of using the grapple?
		if ((ent->client) &&  ((ent->client->player_special & SPECIAL_GRAPPLE) == 0))
		{
			safe_cprintf(ent, PRINT_HIGH, "Sorry - This class cannot use the grapple.\n");
			return;
		}

		if (ent->wf_team <= 0)
			return;
//WF34 END
		if (ent->client->ctf_grapple)
		{
//			CTFResetGrapple(ent->client->ctf_grapple);
			CTFResetGrapple2(ent->client->ctf_grapple);//newgrap 4/99
		}
		else
		{
			cmd_Grapple(ent);
		}
	}
	//New commands in 3.3, 3.4

	//toggle on/off
	else if (Q_stricmp (cmd, "lasersight") == 0)
	{
		if(ent->client->pers.laseron)
			ent->client->pers.laseron=0;
		else
			ent->client->pers.laseron=1;
	}
	//toggle on/off
    else if (Q_stricmp (cmd, "supplydepot") == 0)
	{
		if ((ent->client->player_special & SPECIAL_SUPPLY_DEPOT)
			&& (ent->wf_team >= 1)
			&& (ent->health > 0))
			SP_SupplyDepot(ent);
		else
			safe_cprintf (ent, PRINT_HIGH, "Sorry, you can't build a supply depot.\n");
	}

	//toggle on/off
    else if (Q_stricmp (cmd, "healingdepot") == 0)
	{
		if ((ent->client->player_special & SPECIAL_HEALING)
			&& (ent->wf_team >= 1)
			&& (ent->health > 0))
			SP_HealingDepot(ent);
		else
			safe_cprintf (ent, PRINT_HIGH, "Sorry, you can't build a healing depot\n");
	}

	//argument = "short", "medium" or "long"
    else if (Q_stricmp (cmd, "plasmabomb") == 0)
	{
		if (ent->client->player_special & SPECIAL_PLASMA_BOMB)
			cmd_PlasmaBomb(ent);
		else
			safe_cprintf (ent, PRINT_HIGH, "Sorry, you can't create a plasma bomb.\n");
	}

	//argument = "build", "detonate", "remove", "view"
    else if (Q_stricmp (cmd, "camera") == 0)
	{
		if (ent->client->player_special & SPECIAL_REMOTE_CAMERA)
			cmd_Camera(ent);
		else
			safe_cprintf (ent, PRINT_HIGH, "Sorry, you can't use the camera.\n");
	}

	//argument = "build", "remove", "upgrade", "repair" and "reload"
    else if (Q_stricmp (cmd, "sentry") == 0)
	{
		if ((ent->client->player_special & SPECIAL_SENTRY_GUN)
			&& (ent->wf_team >= 1)
			&& (ent->health > 0))
		{
			cmd_Sentry(ent);
		}
		else
			safe_cprintf (ent, PRINT_HIGH, "Sorry, you can't create a sentry gun.\n");
	}

	//argument = "build", "remove", "upgrade", "repair" and "reload"
    else if (Q_stricmp (cmd, "biosentry") == 0)
	{
		if ((ent->client->player_special & SPECIAL_BIOSENTRY)
			&& (ent->wf_team >= 1)
			&& (ent->health > 0))
			cmd_Biosentry(ent);
		else
			safe_cprintf (ent, PRINT_HIGH, "Sorry, you can't create a biosentry.\n");
	}

	//argument = class number.  default to class # 1
    else if (Q_stricmp (cmd, "disguise") == 0)
	{
		if (ent->client->player_special & SPECIAL_DISGUISE)
			cmd_Disguise(ent);
		else
			safe_cprintf (ent, PRINT_HIGH, "Sorry, you are not allowed to disguise.\n");
	}

    else if (Q_stricmp (cmd, "cloak") == 0)
	{
		if (ent->client->player_special & SPECIAL_CLOAK)
			Cmd_Cloak_f (ent);
		else
			safe_cprintf (ent, PRINT_HIGH, "Sorry, you are not allowed to cloak.\n");
	}

//WF
    else if (Q_stricmp (cmd, "alarm") == 0)
        cmd_Alarm (ent);

    
	// find friend commands
    else if (Q_stricmp (cmd, "friend") == 0)
        Cmd_Friend_f (ent);
    else if (Q_stricmp (cmd, "showfriends") == 0)
        Cmd_ShowFriends_f (ent);
    else if (Q_stricmp (cmd, "ihavenofriends") == 0)
	{
		ent->client->pers.hasfriends = 0;
		safe_cprintf (ent, PRINT_HIGH, "Friends can no longer find you.\n");
	}
    else if (Q_stricmp (cmd, "ihavefriends") == 0)
	{
		ent->client->pers.hasfriends = 1;
		safe_cprintf (ent, PRINT_HIGH, "Friends can now find you.\n");
	}
    else if (Q_stricmp (cmd, "motd") == 0)
	{
		gi.centerprintf(ent, "\n\n=====================================\n%s\n%s\n%s\n\n-------------------------------------\n", 
			wf_game.motd[0], wf_game.motd[1], wf_game.motd[2]);
	}

/////////////////////////////ERASER START//////////////////////////////////
///////////////////////////////////////////////////////////////////////////
	
	//acrid 3/7/99 added && bot_allow_client_commands->value to all bot commands
	else if (Q_stricmp (cmd, "botname") == 0) 
	{
		if (!bot_allow_client_commands->value)
		{
			gi.dprintf("Server has disabled bot commands\n");
			return;
		}
		spawn_bot (gi.argv(1));
	}
	else if (Q_stricmp (cmd, "bots") == 0)
	{
		safe_cprintf(ent, PRINT_HIGH, "\nThis command is not used anymore.\nUse bot_num # to spawn some bots.\n\n");
	}
	else if (Q_stricmp (cmd, "servcmd") == 0)
	{
		Cmd_BotCommands_f(ent);
	}
	else if (Q_stricmp (cmd, "tips") == 0)
	{
		Cmd_Tips_f(ent);
	}
	else if (Q_stricmp (cmd, "addmd2skin") == 0 && bot_allow_client_commands->value)
	{
		AddModelSkin(gi.argv(1), gi.argv(2));
	}
	else if (Q_stricmp (cmd, "join") == 0 && bot_allow_client_commands->value)
	{
		Cmd_Join_f(ent, gi.argv(1));
	}
	else if (Q_stricmp (cmd, "lag") == 0 && bot_allow_client_commands->value)
	{
		Cmd_Lag_f(ent, gi.argv(1));
	}
	else if (Q_stricmp (cmd, "teams") == 0 && bot_allow_client_commands->value)
	{
		Cmd_Teams_f(ent);
	}
	else if (Q_stricmp (cmd, "botpath") == 0 && bot_allow_client_commands->value)
	{
		Cmd_Botpath_f(ent);
	}
	else if (Q_stricmp (cmd, "showpath") == 0 && bot_allow_client_commands->value)
	{
		Cmd_Showpath_f(ent);
	}
	else if (Q_stricmp (cmd, "group") == 0 && bot_allow_client_commands->value)
	{
		TeamGroup(ent);
	}
	else if (Q_stricmp (cmd, "disperse") == 0 && bot_allow_client_commands->value)
	{
		TeamDisperse(ent);
	}
	else if (Q_stricmp (cmd, "rushbase") == 0 && bot_allow_client_commands->value)
	{

		edict_t	*plyr;
		int	i;
		edict_t *flag, *enemy_flag;

		if (!ctf->value)
		{
			safe_cprintf(ent, PRINT_HIGH, "Command only available during CTF play\n");
			return;
		}

		if (ent->client->resp.ctf_team == CTF_TEAM1)
		{
			flag = flag1_ent;
			enemy_flag = flag2_ent;
			team1_rushbase_time = RUSHBASE_OVERRIDE_TIME;
			team1_defendbase_time = 0;
		}
		else
		{
			flag = flag2_ent;
			enemy_flag = flag1_ent;
			team2_rushbase_time = RUSHBASE_OVERRIDE_TIME;
			team2_defendbase_time = 0;
		}

		gi.centerprintf(ent, "All available units RUSH BASE!\n\n(Type \"freestyle\" to return to normal)\n", ent->client->pers.netname);

		for (i=0; i<num_players; i++)
		{
			plyr = players[i];

			if (plyr->client->resp.ctf_team != ent->client->resp.ctf_team)
				continue;
//			if (plyr->target_ent)
//				continue;
			if (plyr->bot_client)
			{
				plyr->movetarget = enemy_flag;
				plyr->movetarget_want = 99;
			}
			else if (plyr != ent)
			{
				safe_cprintf(plyr, PRINT_CHAT, "<%s> Rushing base!\n", ent->client->pers.netname);
			}
		}
	}

	else if (Q_stricmp (cmd, "defendbase") == 0 && bot_allow_client_commands->value)
	{

		edict_t	*plyr;
		int	i;
		edict_t *flag, *enemy_flag;

		if (!ctf->value)
		{
			safe_cprintf(ent, PRINT_HIGH, "Command only available during CTF play\n");
			return;
		}

		if (ent->client->resp.ctf_team == CTF_TEAM1)
		{
			flag = flag1_ent;
			enemy_flag = flag2_ent;
			team1_rushbase_time = 0;
			team1_defendbase_time = RUSHBASE_OVERRIDE_TIME;
		}
		else
		{
			flag = flag2_ent;
			enemy_flag = flag1_ent;
			team2_rushbase_time = 0;
			team2_defendbase_time = RUSHBASE_OVERRIDE_TIME;
		}

		gi.centerprintf(ent, "All available units DEFEND BASE!\n\n(Type \"freestyle\" to return to normal)\n", ent->client->pers.netname);

		for (i=0; i<num_players; i++)
		{
			plyr = players[i];

			if (plyr->client->resp.ctf_team != ent->client->resp.ctf_team)
				continue;
//			if (plyr->target_ent)
//				continue;
			if (plyr->bot_client)
			{
				plyr->movetarget = flag;
				plyr->target_ent = flag;
				plyr->movetarget_want = WANT_SHITYEAH;
			}
			else if (plyr != ent)
			{
				safe_cprintf(plyr, PRINT_CHAT, "<%s> Defending base!\n", ent->client->pers.netname);
			}
		}
	}

	else if (Q_stricmp (cmd, "freestyle") == 0 && bot_allow_client_commands->value)
	{
		if (ent->client->resp.ctf_team == CTF_TEAM1)
		{
			team1_rushbase_time = 0;
			team1_defendbase_time = 0;
		}
		else
		{
			team2_rushbase_time = 0;
			team2_defendbase_time = 0;
		}

		safe_cprintf(ent, PRINT_HIGH, "Returning bots to Freestyle mode.\n");
	}

	else if (Q_stricmp (cmd, "flagpath") == 0 && bot_allow_client_commands->value)
	{
		FlagPath(ent, ent->client->resp.ctf_team);
	}
	else if (Q_stricmp (cmd, "clear_flagpaths") == 0 && bot_allow_client_commands->value)
	{
		edict_t *trav=NULL, *last=NULL;
		int count=0;

		while (trav = G_Find(last, FOFS(classname), "flag_path_src"))
		{
			last = trav;
			G_FreeEdict(trav);
			count++;
		}

		last = NULL;
		while (trav = G_Find(last, FOFS(classname), "flag_path_dest"))
		{
			last = trav;
			G_FreeEdict(trav);
			count++;
		}

		if (count)
			safe_cprintf(ent, PRINT_HIGH, "\nSuccessfully cleared all flagpaths\n\n");
	}
	else if (Q_stricmp (cmd, "botpause") == 0 && bot_allow_client_commands->value)
	{
		paused = !paused;
		
		if (!paused)
		{	// just resumed play
			int i;

			for (i=0; i<num_players; i++)
			{
				if (!players[i]->bot_client && !ent->client->ctf_grapple)
				{
					players[i]->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
				}
			}

			safe_bprintf(PRINT_HIGH, "%s unpaused the game\n", ent->client->pers.netname);
		}
	}
	else if ((Q_stricmp (cmd, "redflag") == 0)  && bot_allow_client_commands->value || (Q_stricmp (cmd, "blueflag") == 0) && bot_allow_client_commands->value)
	{	// spawn a red flag, which gets saved as node data to make DM levels into CTF levels
		edict_t *flag;
 
		flag = G_Spawn();
		flag->classname = gi.TagMalloc(16, TAG_LEVEL);
		strcpy(flag->classname, cmd);
		VectorCopy(ent->s.origin, flag->s.origin);
		VectorCopy(ent->s.angles, flag->s.angles);

		safe_cprintf(ent, PRINT_HIGH, "Dropped \"%s\" node\n", cmd);
		dropped_trail = true;
	}
	else if (Q_stricmp (cmd, "clearflags") == 0 && bot_allow_client_commands->value)
	{
		edict_t *flag;

		flag = NULL;
		while (flag = G_Find( flag, FOFS(classname), "blueflag" ) )
		{
			G_FreeEdict( flag );
		}

		flag = NULL;
		while (flag = G_Find( flag, FOFS(classname), "redflag" ) )
		{
			G_FreeEdict( flag );
		}

		safe_cprintf( ent, PRINT_HIGH, "Cleared user created CTF flags.\n");
	}
	else if (Q_stricmp (cmd, "ctf_item") == 0 && bot_allow_client_commands->value)
	{
		ctf_item_t *old_ctf_item_head;

		cmd = gi.argv(1);

		if (strlen(cmd) == 0)
		{
			safe_cprintf(ent, PRINT_HIGH, "No classname specified, ignored.\n");
			return;
		}

		old_ctf_item_head = ctf_item_head;

		ctf_item_head = gi.TagMalloc(sizeof(ctf_item_t), TAG_LEVEL);
		memset(ctf_item_head, 0, sizeof(ctf_item_t));

		strcpy(ctf_item_head->classname, cmd);
		VectorCopy(ent->s.origin, ctf_item_head->origin);
		VectorCopy(ent->s.angles, ctf_item_head->angles);
		ctf_item_head->next = old_ctf_item_head;

		safe_cprintf(ent, PRINT_HIGH, "Successfully placed %s at (%i %i %i)\nThis item will appear upon reloading the current map\n", cmd, (int)ent->s.origin[0], (int)ent->s.origin[1], (int)ent->s.origin[2]);
		dropped_trail = true;
	}
	else if (Q_stricmp (cmd, "clear_items") == 0 && bot_allow_client_commands->value)
	{
		ctf_item_head = NULL;
		safe_cprintf(ent, PRINT_HIGH, "Cleared CTF_ITEM data\n");
		dropped_trail = true;
	}

	else if (Q_stricmp (cmd, "toggle_flagpaths") == 0 && bot_allow_client_commands->value)
	{
		if (ent->flags & FL_SHOW_FLAGPATHS)
		{
			ent->flags &= ~FL_SHOW_FLAGPATHS;

			// turn off the models
			{
				edict_t *trav;

				// show lines between alternate routes
				trav = NULL;
				while (trav = G_Find(trav, FOFS(classname), "flag_path_src"))
					trav->s.modelindex = 0;
			}
		}
		else
			ent->flags |= FL_SHOW_FLAGPATHS;
	}
///////////////////////////ERASER END//////////////////////////////////////

	else	
	{
		//See if this command is the name of a player class
		class_picked = 0;
		for (i = 1; i <= numclasses; ++i)
		{
			if ((Q_stricmp (cmd, classinfo[i].name) == 0) && (class_picked == 0))
				class_picked = i;
		}

		if (class_picked)
		{

			//See if there are any class limits defined
			WFClassCount(ent, classcount);

			if (classcount[class_picked] >= classinfo[class_picked].limit)
			{
				safe_cprintf (ent, PRINT_HIGH, "Sorry, class limit on %s is %d. Pick another class.\n",
						classinfo[class_picked].name,classinfo[class_picked].limit);
				return;
			}

			ent->client->pers.next_player_class = class_picked;
			safe_cprintf (ent, PRINT_HIGH, "You will become a %s the next time you respawn. \n",
					classinfo[class_picked].name);
		}
		else
		{
			// anything that doesn't match a command will be a chat
			Cmd_Say_f (ent, false, true);
		}
	}
}
