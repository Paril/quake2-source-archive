/*
*
* Lithium simulation
*
* Coded by Anthony Jacques, June 1998
*
*/

#include "g_local.h"
#include "bot_procs.h" // for lithium_motd (Eraser version)
#include <stdio.h> // for lithium_motd()

// for directory creation
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#endif

#include "aj_runes.h"
#include "aj_startmax.h"
#include "aj_statusbars.h"
#include "aj_replacelist.h"
#include "aj_banning.h"


gitem_t *item_phalanx, *item_ionripper, *item_trap, *item_quadfire, *item_magslugs, *item_ir_goggles, *item_double, *item_sphere_vengeance, *item_sphere_hunter, *item_sphere_defender, *item_doppleganger;



void lithium_init(void)
{
	char none[6]="none";
	char *c=none;
	char dir[256];
	int i;
	cvar_t *game_dir;

	while (*c)
	{
		*c += 128;
		c++;
	}

	gi.configstring(CS_TECHNONE, none);
	gi.configstring(CS_SAFETY, "Safety Mode");
	gi.configstring(CS_OBSERVER, "Observer Mode");

	if (use_runes->value)
	{
		lithium_setuprunes();
		CTFSetupTechSpawn();
	}

	if (use_replacelist->value)
		read_replacelist();

	read_banlist();

	if (use_onegun->value)
		aj_onegun((int)use_onegun->value);

	game_dir = gi.cvar ("game", "", 0);

#ifdef  _WIN32
	i =  sprintf(dir, ".\\");
	i += sprintf(dir + i, game_dir->string);
	i += sprintf(dir + i, "\\routes");
	mkdir(dir);
#else
	strcpy(dir, "./");
	strcat(dir, game_dir->string);
	strcat(dir, "/routes");
	mkdir(dir, 755);
#endif

	aj_choosebar();
}



void lithium_initclient(gclient_t *client)
{
// (set the start_* values)
// FIXME: can all these ITEM_INDEX(find_item()) calls be replaced by item_shells etc?
	client->pers.inventory[ITEM_INDEX(item_shells)] = start_shells->value;
	client->pers.inventory[ITEM_INDEX(item_bullets)] = start_bullets->value;
	client->pers.inventory[ITEM_INDEX(item_cells)] = start_cells->value;
	client->pers.inventory[ITEM_INDEX(item_rockets)] = start_rockets->value;
	client->pers.inventory[ITEM_INDEX(item_slugs)] = start_slugs->value;
	client->pers.inventory[ITEM_INDEX(item_grenades)] = start_grenades->value;
	client->pers.inventory[ITEM_INDEX(item_magslugs)] = start_magslugs->value;
	client->pers.inventory[ITEM_INDEX(item_trap)] = start_traps->value;
	client->pers.inventory[ITEM_INDEX(item_flechettes)] = start_flechettes->value;
	client->pers.inventory[ITEM_INDEX(item_rounds)] = start_rounds->value;

	client->pers.inventory[ITEM_INDEX(item_blaster)] = start_blaster->value;
	client->pers.inventory[ITEM_INDEX(item_shotgun)] = start_shotgun->value;
	client->pers.inventory[ITEM_INDEX(item_supershotgun)] = start_sshotgun->value;
	client->pers.inventory[ITEM_INDEX(item_machinegun)] = start_machinegun->value;
	client->pers.inventory[ITEM_INDEX(item_chaingun)] = start_chaingun->value;
	client->pers.inventory[ITEM_INDEX(item_grenadelauncher)] = start_grenadelauncher->value;
	client->pers.inventory[ITEM_INDEX(item_rocketlauncher)] = start_rocketlauncher->value;
	client->pers.inventory[ITEM_INDEX(item_hyperblaster)] = start_hyperblaster->value;
	client->pers.inventory[ITEM_INDEX(item_railgun)] = start_railgun->value;
	client->pers.inventory[ITEM_INDEX(item_bfg10k)] = start_bfg->value;
	client->pers.inventory[ITEM_INDEX(item_phalanx)] = start_phalanx->value;
	client->pers.inventory[ITEM_INDEX(item_ionripper)] = start_ionripper->value;
	client->pers.inventory[ITEM_INDEX(item_etfrifle)] = start_etfrifle->value;
	client->pers.inventory[ITEM_INDEX(item_disruptor)] = start_disruptor->value;
	client->pers.inventory[ITEM_INDEX(item_proxlauncher)] = start_proxlauncher->value;
	client->pers.inventory[ITEM_INDEX(item_plasmabeam)] = start_plasmabeam->value;
	client->pers.inventory[ITEM_INDEX(item_chainfist)] = start_chainfist->value;
	client->pers.inventory[ITEM_INDEX(item_shockwave)] = start_shockwave -> value;


	switch ((int)start_rune->value)
	{
	case 1:
		client->pers.inventory[ITEM_INDEX(item_tech1)] = 1;
		client->ps.stats[STAT_LITHIUM_RUNE] = CS_TECH1;
		break;
	case 2:
		client->pers.inventory[ITEM_INDEX(item_tech2)] = 1;
		client->ps.stats[STAT_LITHIUM_RUNE] = CS_TECH2;
		break;
	case 3:
		client->pers.inventory[ITEM_INDEX(item_tech3)] = 1;
		client->ps.stats[STAT_LITHIUM_RUNE] = CS_TECH3;
		break;
	case 4:
		client->pers.inventory[ITEM_INDEX(item_tech4)] = 1;
		client->ps.stats[STAT_LITHIUM_RUNE] = CS_TECH4;
		break;
	case 5:
		client->pers.inventory[ITEM_INDEX(item_tech5)] = 1;
		client->ps.stats[STAT_LITHIUM_RUNE] = CS_TECH5;
		break;
	}
	
	client->pers.health			= start_health->value; 

// set the max_* values
	client->pers.max_health		= max_health->value;
	client->pers.max_bullets	= max_bullets->value;
	client->pers.max_shells		= max_shells->value;
	client->pers.max_rockets	= max_rockets->value;
	client->pers.max_grenades	= max_grenades->value;
	client->pers.max_cells		= max_cells->value;
	client->pers.max_slugs		= max_slugs->value;
	client->pers.max_magslug	= max_magslugs->value;
	client->pers.max_trap		= max_traps->value;
	client->pers.max_prox		= max_prox->value;
	client->pers.max_tesla		= max_tesla->value;
	client->pers.max_flechettes = max_flechettes->value;
#ifndef KILL_DISRUPTOR
	client->pers.max_rounds     = max_rounds->value;
#endif
	client->pers.max_armor		= max_armor->value;

	client->resp.id_state = def_id->value;
	client->ps.stats[STAT_LITHIUM_RUNE]=CS_TECHNONE;
}


void lithium_choosestartweap(edict_t *ent)
{
	gclient_t *client=ent->client;

	client->newweapon=0;

	if ((start_weapon->value==0 || start_weapon->value==1)
		&& client->pers.inventory[ITEM_INDEX(item_blaster)])
		client->newweapon = item_blaster;

	if (client->pers.inventory[ITEM_INDEX(item_shotgun)] &&
		( (client->pers.inventory[ITEM_INDEX(item_shells)] && start_weapon->value==0)
		 || start_weapon->value==2))
	{
		client->newweapon = item_shotgun;
	}

	if (client->pers.inventory[ITEM_INDEX(item_supershotgun)] && 
		( (client->pers.inventory[ITEM_INDEX(item_shells)] && start_weapon->value==0)
		|| start_weapon->value==3))
	{
		client->newweapon = item_supershotgun;
	}

	if (client->pers.inventory[ITEM_INDEX(item_machinegun)] && 
		( (client->pers.inventory[ITEM_INDEX(item_bullets)] && start_weapon->value==0)
		|| start_weapon->value==4))
	{
		client->newweapon = item_machinegun;
	}

	if (client->pers.inventory[ITEM_INDEX(item_chaingun)] &&
		( (client->pers.inventory[ITEM_INDEX(item_bullets)] && start_weapon->value==0)
		|| start_weapon->value==5))
	{
		client->newweapon = item_chaingun;
	}

	if (client->pers.inventory[ITEM_INDEX(item_grenadelauncher)] && 
		( (client->pers.inventory[ITEM_INDEX(item_grenades)] && start_weapon->value==0)
		|| start_weapon->value==6))
	{
		client->newweapon = item_grenadelauncher;
	}

	if (client->pers.inventory[ITEM_INDEX(item_rocketlauncher)] &&
		( (client->pers.inventory[ITEM_INDEX(item_rockets)] && start_weapon->value==0)
		|| start_weapon->value==7))
	{
		client->newweapon = item_rocketlauncher;
	}

	if (client->pers.inventory[ITEM_INDEX(item_hyperblaster)] && 
		( (client->pers.inventory[ITEM_INDEX(item_cells)] && start_weapon->value==0)
		|| start_weapon->value==8))
	{
		client->newweapon = item_hyperblaster;
	}

	if (client->pers.inventory[ITEM_INDEX(item_railgun)] && 
		( (client->pers.inventory[ITEM_INDEX(item_slugs)] && start_weapon->value==0)
		|| start_weapon->value==9))
	{
		client->newweapon = item_railgun;
	}

	if (client->pers.inventory[ITEM_INDEX(item_bfg10k)] && 
		( (client->pers.inventory[ITEM_INDEX(item_cells)] && start_weapon->value==0)
		|| start_weapon->value==10))
	{
		client->newweapon = item_bfg10k;
	}

	if (ent->bot_client)
		botPickBestWeapon(ent);
	else
	{
		client->pers.weapon = client->newweapon;
		client->pers.lastweapon = client->newweapon;
	}
}

// function returns 1 if the item should be spawned
int lithium_weaponbanning(edict_t *ent)
{
//	if (bot_calc_nodes->value)
//		return 1; // NEVER ban an item if calc'ing nodes as the route will be incomplete if loaded with items un-banned

	if ((no_shotgun->value && (strcmp(ent->classname, "weapon_shotgun")==0))
		|| (no_sshotgun->value && (strcmp(ent->classname, "weapon_supershotgun")==0))
		|| (no_machinegun->value && (strcmp(ent->classname, "weapon_machinegun")==0))
		|| (no_chaingun->value && (strcmp(ent->classname, "weapon_chaingun")==0))
		|| (no_grenadelauncher->value && (strcmp(ent->classname, "weapon_grenadelauncher")==0))
		|| (no_rocketlauncher->value && (strcmp(ent->classname, "weapon_rocketlauncher")==0))
		|| (no_hyperblaster->value && (strcmp(ent->classname, "weapon_hyperblaster")==0))
		|| (no_railgun->value && (strcmp(ent->classname, "weapon_railgun")==0))
		|| (no_bfg->value && (strcmp(ent->classname, "weapon_bfg")==0))
		|| (no_quad->value && (strcmp(ent->classname, "item_quad")==0))
		|| (no_pent->value && (strcmp(ent->classname, "item_invulnerability")==0))
		|| (no_phalanx->value && (strcmp(ent->classname, "weapon_phalanx")==0))
		|| (no_ionripper->value && (strcmp(ent->classname, "weapon_boomer")==0))
		|| (no_traps->value && (strcmp(ent->classname, "ammo_trap")==0))
		|| (no_disruptor->value && (strcmp(ent->classname, "weapon_disintegrator")==0))
		|| (no_proxlauncher->value && (strcmp(ent->classname, "weapon_proxlauncher")==0))
		|| (no_etfrifle->value && (strcmp(ent->classname, "weapon_etf_rifle")==0))
		|| (no_plasmabeam->value && (strcmp(ent->classname, "weapon_plasmabeam")==0))
		|| (no_chainfist->value && (strcmp(ent->classname, "weapon_chainfist")==0)))
	{
		G_FreeEdict(ent);
		return 0;
	}
	return 1;
}


void lithium_motd(edict_t *ent)
{
	FILE	*fh=NULL;
	char	output[1024], line[257];
	int		line_count=0, i=0;
	char	filename[256];
	cvar_t	*game_dir;

	if (!motd->string)
	{
		gi.centerprintf(ent, "\n\n=====================================\nEraser Ultra v%1.3f BETA\nby Anthony Jacques\nBased upon Eraser %1.3f by Ryan Feltrin\n\nNo MOTD defined.\n\nwww.users.zetnet.co.uk/jacquesa/q2/\n\n",ULTRA_VERSION , ERASER_VERSION);
		return;
	}
	
	game_dir = gi.cvar ("game", "", 0);

#ifdef	_WIN32
	strcpy(filename, ".\\");
	strcat(filename, game_dir->string);
	strcat(filename, "\\");
	strcat(filename, motd->string);
#else
	strcpy(filename, "./");
	strcat(filename, game_dir->string);
	strcat(filename, "/");
	strcat(filename, motd->string);
#endif

	fh=fopen(filename, "r");
	if (fh==NULL)
	{
		gi.centerprintf(ent, "\n\n=====================================\nEraser Ultra v%1.3f BETA\nby Anthony Jacques\nBased upon Eraser %1.3f by Ryan Feltrin\n\nMOTD file not found.\n\nwww.users.zetnet.co.uk/jacquesa/q2/\n\n",ULTRA_VERSION , ERASER_VERSION);
		return;
	}

	output[0]=0;
	while (fgets(line, 256, fh) && i<1024)
	{
		strcat(output, line);
		i+=strlen(line);
	}

	if (i > 0)
		output[i] = '\0';

	fclose(fh);

	gi.centerprintf(ent, output);
}




void lithium_observer(edict_t *ent, pmenu_t *menu)
{
	if (use_observer->value && !ent->bot_client)
	{
		ent->movetype = MOVETYPE_NOCLIP;
		ent->solid = SOLID_NOT;
		ent->svflags |= SVF_NOCLIENT;
		ent->client->observer_mode = true;
		ent->client->ps.gunindex = 0;
		gi.linkentity (ent);
		ent->client->ps.stats[STAT_LITHIUM_MODE] = CS_OBSERVER;
		if (ent->client->bIsCamera)
			ent->client->bIsCamera=0;
	}
}
