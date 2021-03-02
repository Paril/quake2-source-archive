/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/p_classes.c,v $
 *   $Revision: 1.10 $
 *   $Date: 2002/07/23 22:48:27 $
 *
 ***********************************

Copyright (C) 2002 Vipersoft

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "g_local.h"
#include "p_classes.h"

// g_classes.c
// D-Day: Normandy Player Classes
// this file is for definitions for classes in DDay.



void Give_Class_Weapon(edict_t *ent)
{
	gitem_t		*item;
	gclient_t	*client;
	gitem_t		*ammo_item;

	client=ent->client;

	//give everyone a knife & fists & helmet
	// UNLESS invuln_medic
	if (client->resp.mos == MEDIC && invuln_medic->value == 1)
	{
		item = FindItem("Morphine");
		client->pers.inventory[ITEM_INDEX(item)] = 1;
	}
	else
	{
		item = FindItem("Helmet");
		client->pers.inventory[ITEM_INDEX(item)] = 1;
		item = FindItem("Fists");
		client->pers.inventory[ITEM_INDEX(item)] = 1;
		item = FindItem("Knife");
		client->pers.inventory[ITEM_INDEX(item)] = 1;
	}

	item=FindItem(client->resp.team_on->mos[client->resp.mos]->weapon1);
	// Loads primary weapon when spawning
	ammo_item = FindItem(item->ammo);
	if (!ammo_item)
	{
		gi.dprintf("WARNING: in Give_Class_Weapon %s spawned with no ammo for %s -> %s\n", ent->client->pers.netname, item->pickup_name, item->ammo);
		return;
	}

//	if (!strcmp(item->ammo, "p38_mag"))
//		ent->client->mags[1].pistol_rnd = ammo_item->quantity;

	if (!strcmp(item->ammo, "mauser98k_mag") && !strcmp(item->pickup_name, "Mauser 98k"))
		ent->client->mags[1].rifle_rnd = ammo_item->quantity;
	else if (!strcmp(item->ammo, "mauser98k_mag") && !strcmp(item->pickup_name, "Mauser 98ks"))
		ent->client->mags[1].sniper_rnd = ammo_item->quantity;

        else if (!strcmp(item->dllname, team_list[1]->teamid))  //faf:  if its a team 1 weap...(usually grm)
        {
                if (item->position == LOC_PISTOL)
                        ent->client->mags[1].pistol_rnd = ammo_item->quantity;
                else if (item->position == LOC_SUBMACHINEGUN)
                        ent->client->mags[1].submg_rnd = ammo_item->quantity;
                else if (item->position == LOC_L_MACHINEGUN)
                        ent->client->mags[1].lmg_rnd = ammo_item->quantity;
                else if (item->position == LOC_H_MACHINEGUN)
                        ent->client->mags[1].hmg_rnd = ammo_item->quantity;
                else if (item->position == LOC_ROCKET)
                        ent->client->mags[1].antitank_rnd = ammo_item->quantity;
                else if ((item->position == LOC_RIFLE))
                        ent->client->mags[1].rifle_rnd = ammo_item->quantity;
                else if ((item->position == LOC_SNIPER))
                        ent->client->mags[1].sniper_rnd = ammo_item->quantity;
        }

        else if (!strcmp(item->dllname, team_list[0]->teamid))  //usually allied weapons here...
        {
                if (item->position == LOC_PISTOL)
                        ent->client->mags[0].pistol_rnd = ammo_item->quantity;
                else if (item->position ==  LOC_RIFLE)
                        ent->client->mags[0].rifle_rnd = ammo_item->quantity;
                else if (item->position == LOC_SNIPER)
                        ent->client->mags[0].sniper_rnd = ammo_item->quantity;  //faf:  not used for usa but so plugin team 1 can use sane ammo for inf rifle and sniper rifle
                else if (item->position == LOC_SUBMACHINEGUN)
                        ent->client->mags[0].submg_rnd = ammo_item->quantity;
                else if (item->position == LOC_L_MACHINEGUN)
                        ent->client->mags[0].lmg_rnd = ammo_item->quantity;
                else if (item->position == LOC_H_MACHINEGUN)
                        ent->client->mags[0].hmg_rnd = ammo_item->quantity;
                else if (item->position == LOC_ROCKET)
                        ent->client->mags[0].antitank_rnd = ammo_item->quantity;
                else if (item->position == LOC_SNIPER)
                        ent->client->mags[0].sniper_rnd = ammo_item->quantity;
        }

        else if (!strcmp(item->ammo, "flame_mag"))
                ent->client->flame_rnd = ammo_item->quantity;
		
		/*
	else if (!strcmp(item->ammo, "mp40_mag"))
		ent->client->mags[1].submg_rnd = ammo_item->quantity;
	else if (!strcmp(item->ammo, "mp43_mag"))
		ent->client->mags[1].lmg_rnd = ammo_item->quantity;
	else if (!strcmp(item->ammo, "mg42_mag"))
		ent->client->mags[1].hmg_rnd = ammo_item->quantity;
	else if (!strcmp(item->ammo, "grm_rockets"))
		ent->client->mags[1].antitank_rnd = ammo_item->quantity;
	else if (!strcmp(item->ammo, "colt45_mag"))
		ent->client->mags[0].pistol_rnd = ammo_item->quantity;
	else if (!strcmp(item->ammo, "m1_mag"))
		ent->client->mags[0].rifle_rnd = ammo_item->quantity;
	else if (!strcmp(item->ammo, "thompson_mag"))
		ent->client->mags[0].submg_rnd = ammo_item->quantity;
	else if (!strcmp(item->ammo, "bar_mag"))
		ent->client->mags[0].lmg_rnd = ammo_item->quantity;
	else if (!strcmp(item->ammo, "hmg_mag"))
		ent->client->mags[0].hmg_rnd = ammo_item->quantity;
	else if (!strcmp(item->ammo, "usa_rockets"))
		ent->client->mags[0].antitank_rnd = ammo_item->quantity;
	else if (!strcmp(item->ammo, "m1903_mag"))
		ent->client->mags[0].sniper_rnd = ammo_item->quantity;
	else if (!strcmp(item->ammo, "flame_mag"))
		ent->client->flame_rnd = ammo_item->quantity;
*/

	if (!item) { //pbowens: prevents from crashing the game
		gi.cprintf(ent, PRINT_HIGH, "weapon1 item not found!\n");
		return;
	}

	client->pers.selected_item=ITEM_INDEX(item);
	client->newweapon=item;
	client->pers.inventory[client->pers.selected_item]=1;

	item = NULL;
	//if(client->resp.team_on->mos[client->resp.mos]->weapon2)
	//{
		if(item=FindItem(client->resp.team_on->mos[client->resp.mos]->weapon2))
		client->pers.inventory[ITEM_INDEX(item)]=1;

	// Loads secondary weapon, if existant, when spawning
	if (item)
	{
		ammo_item = FindItem(item->ammo);
		if (!strcmp(item->dllname, team_list[1]->teamid) && item->position == LOC_PISTOL)
			ent->client->mags[1].pistol_rnd = ammo_item->quantity;
		else if (!strcmp(item->dllname, team_list[0]->teamid) && item->position == LOC_PISTOL)
			ent->client->mags[0].pistol_rnd = ammo_item->quantity;

		/*
		ammo_item = FindItem(item->ammo);
		if (!strcmp(item->ammo, "p38_mag"))
			ent->client->mags[1].pistol_rnd = ammo_item->quantity;
		else if (!strcmp(item->ammo, "colt45_mag"))
			ent->client->mags[0].pistol_rnd = ammo_item->quantity;
		*/
	}

	//}
	//if(client->resp.team_on->mos[client->resp.mos]->grenades)
	//{
		if(item=FindItem(client->resp.team_on->mos[client->resp.mos]->grenades) )
			client->pers.inventory[ITEM_INDEX(item)]=client->resp.team_on->mos[client->resp.mos]->grenadenum;
	//}
	//if(client->resp.team_on->mos[client->resp.mos]->special)
	//{
		if(item=FindItem(client->resp.team_on->mos[client->resp.mos]->special) )
		client->pers.inventory[ITEM_INDEX(item)]=client->resp.team_on->mos[client->resp.mos]->specnum;
	//}

	ChangeWeapon(ent);
}
			


//this function is for giving the player ammo...
void Give_Class_Ammo(edict_t *ent)
{
	gitem_t *item,*item2;
	
	if (ent->client->resp.team_on->mos[ent->client->resp.mos]->ammo1 )
	{
		item=FindItem(ent->client->resp.team_on->mos[ent->client->resp.mos]->weapon1);
		item2=FindItem(item->ammo);
		Add_Ammo(ent,item2,ent->client->resp.team_on->mos[ent->client->resp.mos]->ammo1);
	}

	if (ent->client->resp.team_on->mos[ent->client->resp.mos]->ammo2 )
	{
		item=FindItem(ent->client->resp.team_on->mos[ent->client->resp.mos]->weapon2);
		item2=FindItem(item->ammo);
		Add_Ammo(ent,item2,ent->client->resp.team_on->mos[ent->client->resp.mos]->ammo2);
	}
	
	
}


void Show_Mos(edict_t *ent)
{
	int i;
	
	if(!ent->client->resp.team_on || !ent->client->resp.mos)
	{
		gi.cprintf(ent,PRINT_HIGH,"Must be on a team to view the open class slots.\n");
		return;
	}

	gi.cprintf(ent,PRINT_HIGH,"\nOpen class slots for %s: \n",ent->client->resp.team_on->teamname);

	for(i=1; i < MAX_MOS; i++) {
		if (ent->client->resp.team_on->mos[i]->available == 99) {
			gi.cprintf(ent, PRINT_HIGH," %10s -- unlimited\n", ent->client->resp.team_on->mos[i]->name);
		} else {
			gi.cprintf(ent, PRINT_HIGH," %10s -- %i\n", 
			ent->client->resp.team_on->mos[i]->name,
			ent->client->resp.team_on->mos[i]->available);
		}
	}
}
 


void InitMOS_List(TeamS_t *team, SMos_t *mos_list)
{
	int i;
	
	SMos_t **MOS;
	team->mos=MOS=gi.TagMalloc( (sizeof(SMos_t)*MAX_MOS),TAG_LEVEL);
	
	MOS[0] = NULL;
	for(i=1;i<MAX_MOS;i++)
	{
		MOS[i]=gi.TagMalloc( sizeof(SMos_t), TAG_LEVEL);
		MOS[i]->name=gi.TagMalloc( MAX_MOS_NAME_CHAR, TAG_LEVEL);
		MOS[i]->weapon1=gi.TagMalloc( MAX_MOS_NAME_CHAR, TAG_LEVEL);
		MOS[i]->weapon2=gi.TagMalloc( MAX_MOS_NAME_CHAR, TAG_LEVEL);
		MOS[i]->grenades=gi.TagMalloc( MAX_MOS_NAME_CHAR, TAG_LEVEL);
		MOS[i]->special=gi.TagMalloc( MAX_MOS_NAME_CHAR, TAG_LEVEL);
		MOS[i]->MOS_Spaw_Point=gi.TagMalloc( sizeof(mos_list[i].MOS_Spaw_Point), TAG_LEVEL);

		strcpy(MOS[i]->name,mos_list[i].name);

		if(mos_list[i].weapon1)
			strcpy(MOS[i]->weapon1,mos_list[i].weapon1);
		MOS[i]->ammo1=mos_list[i].ammo1;

		if(mos_list[i].weapon2) 
			strcpy(MOS[i]->weapon2,mos_list[i].weapon2);
		MOS[i]->ammo2=mos_list[i].ammo2;
		
		if(mos_list[i].grenades) 
			strcpy(MOS[i]->grenades,mos_list[i].grenades);
		MOS[i]->grenadenum=mos_list[i].grenadenum;
		
		if(mos_list[i].special)
			strcpy(MOS[i]->special,mos_list[i].special);

		MOS[i]->specnum=mos_list[i].specnum;
		MOS[i]->mos=mos_list[i].mos;
		MOS[i]->available=mos_list[i].available;
		MOS[i]->normal_weight=mos_list[i].normal_weight;
		MOS[i]->max_weight=mos_list[i].max_weight;
		MOS[i]->speed_mod=mos_list[i].speed_mod;
		MOS[i]->MOS_Spaw_Point=mos_list[i].MOS_Spaw_Point;
		MOS[i]->skinname=mos_list[i].skinname;
	}

	//team->language = language;
	//team->playermodel = playermodel;
	//team->teamid = teamid;

	//team->mos gi.TagMalloc( (sizeof(char)*playermodel),TAG_LEVEL);

}
