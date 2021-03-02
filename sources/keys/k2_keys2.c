#include "g_local.h"

#include "stdlog.h"	//	StdLog - Mark Davies
#include "gslog.h"	//	StdLog - Mark Davies


//k2_keys2.c
//
//funtions for keys2 mod.
//
//A few of these are duplicates of original source.  I recreate/modify them
//here instead of modifying the original functions.


//Spawns all the keys when level begins
void K2_SpawnAllKeys(void)
{
	int	i;
	
	if(haste->value)
	{
		if ((int)haste->value > 4)
			gi.cvar_set("haste","4");

		for(i=0;i<(int)haste->value;i++)
			K2_SpawnKey(NULL, KEY_HASTE, 0);
	}
		
	if(regeneration->value)
	{
		if ((int)regeneration->value > 4)
			gi.cvar_set("regeneration","4");

		for(i=0;i<(int)regeneration->value;i++)
			K2_SpawnKey(NULL, KEY_REGENERATION, 0);
	}
	
	if(futility->value)
	{
		if ((int)futility->value > 4)
			gi.cvar_set("futility","4");

		for(i=0;i<(int)futility->value;i++)
			K2_SpawnKey(NULL, KEY_FUTILITY, 0);
	}
	
	if(infliction->value)
	{
		if ((int)infliction->value > 4)
			gi.cvar_set("infliction","4");

		for(i=0;i<(int)infliction->value;i++)
			K2_SpawnKey(NULL, KEY_INFLICTION, 0);
	}
	
	if(bfk->value)
	{
		if ((int)bfk->value > 4)
			gi.cvar_set("bfk","4");

		for(i=0;i<(int)bfk->value;i++)
			K2_SpawnKey(NULL, KEY_BFK, 0);
	}

	if(stealth->value)
	{
		if ((int)stealth->value > 4)
			gi.cvar_set("stealth","4");

		for(i=0;i<(int)stealth->value;i++)
			K2_SpawnKey(NULL, KEY_STEALTH, 0);
	}
	
	if(antikey->value)
	{
		if ((int)antikey->value > 4)
			gi.cvar_set("antikey","4");

		for(i=0;i<(int)antikey->value;i++)
			K2_SpawnKey(NULL, KEY_ANTIKEY, 0);
	}
	
	if(homing->value)
	{
		
		if ((int)homing->value > 4)
			gi.cvar_set("homing","4");
		
		for(i=0;i<(int)homing->value;i++)
			K2_SpawnKey(NULL, KEY_HOMING, 0);
	}
	

}

void K2_SpawnItem(edict_t *ent, gitem_t *item, int key)
{
	PrecacheItem (item);
	ent->item = item;

	//Keys can't be damaged
	ent->takedamage= DAMAGE_NO;

	ent->nextthink = level.time + 2 * FRAMETIME;
	ent->think = K2_droptofloor;
	ent->s.event = EV_ITEM_RESPAWN;		
	
	ent->s.effects = EF_ROTATE;
	ent->s.renderfx = RF_GLOW;
	
	if (!keyshells->value)
		ent->s.renderfx = RF_FULLBRIGHT;
		
	//Apply colored shell to key
	if (keyshells->value)
	{
		if ( (qversion > 3.19) || (qversion < 3.19) )
		{
			
			if (key == KEY_HOMING) {
				ent->s.effects |= EF_COLOR_SHELL;
				ent->s.renderfx |= RF_GREENSHELL;
			}
			else if (key == KEY_FUTILITY) {
				ent->s.effects |= EF_COLOR_SHELL;
				ent->s.renderfx |= (RF_BLUESHELL | RF_GREENSHELL);
			}
			else if (key == KEY_INFLICTION) {
				ent->s.effects |= EF_COLOR_SHELL;
				ent->s.renderfx |= RF_BLUESHELL;
			}
			else if (key == KEY_REGENERATION) {
				ent->s.effects |= EF_COLOR_SHELL;
				ent->s.renderfx |= (RF_REDSHELL | RF_BLUESHELL);
			}
			else if (key == KEY_BFK) {
				ent->s.effects |= EF_COLOR_SHELL;
				ent->s.renderfx |= RF_REDSHELL;
			}
			else if (key == KEY_ANTIKEY) {
				ent->s.effects |= EF_COLOR_SHELL;
				ent->s.renderfx |= RF_WHITESHELL;
			}
			else if (key == KEY_HASTE) {
				ent->s.effects |= EF_COLOR_SHELL;
				ent->s.renderfx |= (RF_REDSHELL | RF_GREENSHELL);
			}
		}
		else
		{ 
			if (key == KEY_HOMING) {
				ent->s.effects |= EF_COLOR_SHELL;
				ent->s.renderfx |= RF_GREENSHELL;
			}
			else if (key == KEY_FUTILITY)
			{
				ent->s.effects |= EF_K2COLORSHELL;
				ent->s.renderfx |= RF_AQUASHELL;
			}
			else if (key == KEY_INFLICTION) {
				ent->s.effects |= EF_COLOR_SHELL;
				ent->s.renderfx |= RF_BLUESHELL;
			}
			else if (key == KEY_REGENERATION)
			{
				ent->s.effects |= EF_K2COLORSHELL;
				ent->s.renderfx |= RF_PURPLESHELL;
			}
			else if (key == KEY_BFK) {
				ent->s.effects |= EF_COLOR_SHELL;
				ent->s.renderfx |= RF_REDSHELL;
			}
			else if (key == KEY_ANTIKEY) {
				ent->s.effects |= EF_COLOR_SHELL;
				ent->s.renderfx |= RF_WHITESHELL;
			}
			else if (key == KEY_HASTE)
			{
				ent->s.effects |= EF_K2COLORSHELL;
				ent->s.renderfx |= RF_YELLOWSHELL;
			}
						
		}
	}
	
	if (key == KEY_STEALTH)
	{
		ent->s.renderfx = RF_TRANSLUCENT;
	}
	

	if (ent->model)
		gi.modelindex (ent->model);
}
/*
K2_SpawnKey

	The self entity is used only if the function is called
	by the player_die function or for Respawn Timer.
	
	key is used to tell which key to spawn

	spawntype is used to tell which type of spawn should occur
	
	0 = Initial spawn and for expired keys 
	1 = Player Died - Respawn the key near the dead player
	2 = Player Dropped - Respawn, but don't let him touch it again until someone else
	                     has had it, or it has respawned again.
	3 = Respawn time reached
	4 = Take Key caused anti-key to be respawned
*/
void K2_SpawnKey(edict_t *self, int key, int spawntype)
{
	edict_t		*ent = NULL;
	edict_t		*spot = NULL;
	gitem_t		*item = NULL;
	char		*key_pickup_name = NULL;
	vec3_t	forward, right;
	vec3_t	offset;
	trace_t	trace;
	int		i=0;

	/*
	if (key == KEY_REGENERATION) 
		strcpy(key_pickup_name,"Regeneration Key");
	else if (key == KEY_HASTE)
		strcpy(key_pickup_name,"Haste Key");
	else if (key == KEY_FUTILITY)
		strcpy(key_pickup_name,"Futility Key");
	else if (key == KEY_INFLICTION)
		strcpy(key_pickup_name,"Infliction Key");
	else if (key == KEY_BFK)
		strcpy(key_pickup_name,"BFK");
	else if (key == KEY_STEALTH)
		strcpy(key_pickup_name,"Stealth Key");
	else if (key == KEY_ANTIKEY)
		strcpy(key_pickup_name,"Anti-Key");
	else if (key == KEY_HOMING)
		strcpy(key_pickup_name,"Homing Key");
	*/
	if (key == KEY_REGENERATION) 
		key_pickup_name = "Regeneration Key";
	else if (key == KEY_HASTE)
		key_pickup_name ="Haste Key";
	else if (key == KEY_FUTILITY)
		key_pickup_name ="Futility Key";
	else if (key == KEY_INFLICTION)
		key_pickup_name ="Infliction Key";
	else if (key == KEY_BFK)
		key_pickup_name ="BFK";
	else if (key == KEY_STEALTH)
		key_pickup_name ="Stealth Key";
	else if (key == KEY_ANTIKEY)
		key_pickup_name ="Anti-Key";
	else if (key == KEY_HOMING)
		key_pickup_name ="Homing Key";
	
	ent = G_Spawn();
	while(!spot) {
		spot = K2_SelectRandomDeathmatchSpawnPoint();
	}

	item = FindItem(key_pickup_name);
	if (item)
	{
		if ( (spawntype == 0) || (spawntype == 3) ) 
		{
				if (spot)
				{
					VectorSet (ent->mins, -15, -15, -15);
					VectorSet (ent->maxs, 15, 15, 15);
					VectorCopy (spot->s.origin, ent->s.origin);
					VectorCopy (spot->s.angles, ent->s.angles);
					AngleVectors (ent->s.angles, forward, right, NULL);
					VectorSet(offset, 24, 0, -16);
			
					G_ProjectSource (spot->s.origin, offset, forward, right, ent->s.origin);
					trace = gi.trace (spot->s.origin, ent->mins, ent->maxs,	ent->s.origin, spot, CONTENTS_SOLID);
					VectorCopy (trace.endpos, ent->s.origin);
					VectorScale (forward, random()*300, ent->velocity);
					ent->velocity[2] = 300;
					
					K2_SpawnItem(ent,item,key);
					
				}
				else
					gi.dprintf("Couldn't find deathmatch start\n");
		}
		else if (spawntype == 4) 
		{
										
				if (spot)
				{
					VectorSet (ent->mins, -15, -15, -15);
					VectorSet (ent->maxs, 15, 15, 15);
					VectorCopy (spot->s.origin, ent->s.origin);
					VectorCopy (spot->s.angles, ent->s.angles);
					AngleVectors (ent->s.angles, forward, right, NULL);
					VectorSet(offset, 24, 0, -16);
			
					G_ProjectSource (spot->s.origin, offset, forward, right, ent->s.origin);
					trace = gi.trace (spot->s.origin, ent->mins, ent->maxs,	ent->s.origin, spot, CONTENTS_SOLID);
					VectorCopy (trace.endpos, ent->s.origin);
					VectorScale (forward, random()*300, ent->velocity);
					ent->velocity[2] = 300;
					ent->prev_owner = self;
					
					K2_SpawnItem(ent,item,key);
					
				}
				else
					gi.dprintf("Couldn't find deathmatch start\n");
		}
		else if (spawntype)
		{
			
			VectorSet (ent->mins, -15, -15, -15);
			VectorSet (ent->maxs, 15, 15, 15);

			AngleVectors (self->client->v_angle, forward, right, NULL);
			VectorSet(offset, 64, 0, 128);
			G_ProjectSource (self->s.origin, offset, forward, right, ent->s.origin);
			trace = gi.trace (self->s.origin, ent->mins, ent->maxs,	ent->s.origin, self, CONTENTS_SOLID);
			VectorCopy (trace.endpos, ent->s.origin);
			
			VectorScale (forward, 300, ent->velocity);
			ent->velocity[2] = 300;
						
			//set previous owner if player dropped the key.
			if (spawntype == 2)
				ent->prev_owner = self;

			K2_SpawnItem(ent,item,key);
		}
	}
	else
		
		gi.dprintf("Couldn't find %s\n",key_pickup_name);
			
}

//K2_droptofloor
//
edict_t	*AddToItemList(edict_t *ent, edict_t	*head);
void K2_droptofloor(edict_t *ent)
{
	trace_t		tr;
	vec3_t		dest;
	float		*v;

	v = tv(-15,-15,-15);
	VectorCopy (v, ent->mins);
	v = tv(15,15,15);
	VectorCopy (v, ent->maxs);

	if (ent->model)
		gi.setmodel (ent, ent->model);
	else
		gi.setmodel (ent, ent->item->world_model);
	
	ent->solid = SOLID_TRIGGER;
	ent->movetype = MOVETYPE_TOSS;  
	ent->think = K2_RespawnKey;
	ent->touch = K2_Touch_Item;
		
	//K2: Set respawn time
	if ((int)respawntime->value == 120)
		ent->nextthink = level.time + 120.0; 
	else 
		ent->nextthink = level.time + 60.0; 
		
	v = tv(0,0,-128);
	VectorAdd (ent->s.origin, v, dest);
	tr = gi.trace (ent->s.origin, ent->mins, ent->maxs, dest, ent, MASK_SOLID);
	if (tr.startsolid)
	{
		int keytype = K2_KeyType(ent);

		gi.dprintf ("K2_droptofloor: %s startsolid at %s\n", ent->classname, vtos(ent->s.origin));
		G_FreeEdict (ent);
		
		//Call the spawn key function again, so we don't lose the key in the game
		gi.dprintf("Spawning new key\n");
		K2_SpawnKey(NULL,keytype,0);
		
		return;
	}
	VectorCopy (tr.endpos, ent->s.origin);

	//Eraser
	bonus_head = AddToItemList(ent, bonus_head);

	gi.linkentity (ent);
	
	//Play respawn sound if new spawn/respawn or player die
	if (!ent->prev_owner)
		gi.sound(ent,CHAN_AUTO,gi.soundindex("misc/spawn1.wav"), 1, ATTN_NORM, 0);
}

//K2_Touch_Item
//
//
void K2_Touch_Item (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
		
	float volume = 1.0;

	if (strcmp(other->classname, "player"))
		return;
	if (other->health < 1)
		return;		// dead people can't pickup
	if (!ent->item->pickup)
		return;		// not a grabbable item?
	
	//Bots can't have them...for now
	//if (other->bot_client)
	//	return;

	//Can't have it if feigned
	if(other->client->is_feigning)
		return;

	//K2: One key only
	if (other->client->key)
	{
		if (!other->client->next_try || (level.framenum > other->client->next_try))
		{
			safe_centerprintf(other,"You already have a key!\n");
			
			//Stealth - don't make a "loud" key try sound
			if (K2_IsStealth(other) )
				volume = 0.2;

			gi.sound(other,CHAN_ITEM,gi.soundindex("misc/keytry.wav"),volume,ATTN_NORM,0);
			
			other->client->next_try = level.framenum + 30;
			return;
		}
		return;
	}

	//Previous owner can't have it
	if (ent->prev_owner ==  other)
	{
		if (!other->client->next_try || (level.framenum > other->client->next_try))
		{
			safe_centerprintf(other,"You dropped this key.\nYou can't have it back yet.\n");
			
			//Stealth - don't make a "loud" key try sound
			if (K2_IsStealth(other) )
				volume = 0.2;
			
			gi.sound(other,CHAN_AUTO,gi.soundindex("misc/keytry.wav"),volume,ATTN_NORM,0);
			
			other->client->next_try = level.framenum + 30;
			return;
		}
		return;
	}

	//Wait xx seconds if you dropped a key
	if (!K2_CanPickupKey(other))
		return;

	if (!ent->item->pickup(ent, other))
		return;		// player can't hold it
		
	//K2: Set the "new" previous owner
	ent->prev_owner = other;
	
	
	//Set the next_try var so client won't get a "you already have a key" message repeatedly.
	other->client->next_try = level.framenum + 30;
	
	//Store the key here
	other->client->key = K2_KeyType(ent);

	// show icon and name on status bar
	other->client->ps.stats[STAT_PICKUP_ICON] = gi.imageindex(ent->item->icon);
	other->client->ps.stats[STAT_PICKUP_STRING] = CS_ITEMS+ITEM_INDEX(ent->item);
	other->client->pickup_msg_time = level.time + 3.0; 
 
	//Stealth - don't make a "loud" key try sound
	if (K2_IsStealth(other) )
		volume = 0.2;
			
	gi.sound(other, CHAN_ITEM, gi.soundindex(ent->item->pickup_sound), volume, ATTN_NORM, 0);
	
	if( pickupannounce->value )
		my_bprintf(PRINT_HIGH, "%s got the %s!\n",other->client->pers.netname,ent->item->pickup_name);
	
	//Set the timer
	other->client->k2_key_framenum = K2_SetClientKeyTimer(other->client->key);

	//Tell client he/she got the key
	if ( other->client->key == KEY_HASTE )
		safe_centerprintf(other,"You got the HASTE Key!\n");
	else if (other->client->key == KEY_REGENERATION)
		safe_centerprintf(other,"You got the REGENERATION Key!\n");
	else if (other->client->key == KEY_FUTILITY)
		safe_centerprintf(other,"You got the FUTILITY Key!\n");
	else if (other->client->key == KEY_INFLICTION)
		safe_centerprintf(other,"You got the INFLICTION Key!\n");
	else if (other->client->key == KEY_STEALTH)
		safe_centerprintf(other,"You got the STEALTH Key!\n");
	else if (other->client->key == KEY_ANTIKEY)
		safe_centerprintf(other,"You got the ANTI-KEY Key!\n");
	else if (other->client->key == KEY_BFK)
		safe_centerprintf(other,"You got the BFK!\n");
	else if (other->client->key == KEY_HOMING)
		safe_centerprintf(other,"You got the HOMING Key!\n");

	//Eraser
	//If other is a bot, and he picked up the BFK or Homing, max out his aggression
	if (other->bot_client && (K2_IsBFK(other) || K2_IsHoming(other)))
			other->bot_stats->aggr = 5;
	
	G_FreeEdict (ent);
		
}

//Respawns the key if not touched
void K2_RespawnKey(edict_t *ent)
{
	K2_SpawnKey(ent,K2_KeyType(ent),3);
	G_FreeEdict(ent);
}


//K2_Drop_Key
//
// Used to intercept key inventory drop commands.
//
void K2_Drop_Key (edict_t *ent, gitem_t *item)
{
	safe_cprintf(ent,PRINT_HIGH,"Type \"drop key\" at the console to drop keys\n");
}

void K2_UseKey(edict_t *ent, gitem_t *item)
{
	safe_cprintf (ent, PRINT_HIGH, "You're already using the key.\n");
	return;
}

//When player issues drop key command
void K2_DropKeyCommand (edict_t *ent)
{
	float volume=1.0;

	if (ent->client->key)
	{
		K2_RemoveKeyFromInventory(ent);
		K2_SpawnKey(ent,ent->client->key,2);
		K2_ResetClientKeyVars(ent);
		ent->client->next_key_pickup = level.time + pickuptime->value;
		//Stealth - don't make a "loud" key try sound
		if (K2_IsStealth(ent) )
			volume = 0.2;
			
		gi.sound(ent,CHAN_AUTO,gi.soundindex("misc/keyuse.wav"),volume, ATTN_STATIC, 0);
	}
	else
		safe_cprintf (ent, PRINT_HIGH, "You don't have a key to drop.\n");
}



//
void K2_RemoveKeyFromInventory(edict_t *ent)
{

	char		*keyname = NULL;
	
	if ( K2_IsBFK(ent) )
		keyname = "BFK";
	else if ( K2_IsRegen(ent) )
		keyname = "REGENERATION KEY";
	else if ( K2_IsHaste(ent) )
		keyname = "HASTE KEY";
	else if ( K2_IsInfliction(ent) )
		keyname = "INFLICTION KEY";
	else if ( K2_IsFutility(ent) )
		keyname = "FUTILITY KEY";
	else if ( K2_IsStealth(ent) )
		keyname = "STEALTH KEY";
	else if ( K2_IsAnti(ent) )
		keyname = "ANTI-KEY";
	else if ( K2_IsHoming(ent) )
		keyname = "HOMING KEY";
	else
		return;

	//Eraser
	//If ent is a bot, and he had the BFK or Homing, change to old aggression value
	if (ent->bot_client && (K2_IsBFK(ent) || K2_IsHoming(ent)))
		ent->bot_stats->aggr = ent->botdata->bot_stats.aggr;
		
	//Remove Key from inventory
	ent->client->pers.inventory[ITEM_INDEX(FindItem(keyname))]--;
	ent->client->pers.selected_item = -1;

	//Print expired message if that's what happened
	if( ent->client->k2_key_framenum <= level.framenum )
		safe_centerprintf(ent,"\nYour %s Has Expired\n",keyname);
		
	my_bprintf(PRINT_HIGH,"%s dropped the %s!\n",ent->client->pers.netname,keyname);
}

edict_t *K2_SelectRandomDeathmatchSpawnPoint(void)
{
	edict_t *spot = NULL;
	int i = rand() % 16;

	while (i--)
		spot = G_Find (spot, FOFS(classname), "info_player_deathmatch");
	if (!spot)
		spot = G_Find (spot, FOFS(classname), "info_player_deathmatch");
	return spot;
}

void K2_InitClientVars(edict_t *ent)
{
	ent->client->grenadeType = GRENADE_NORMAL;
	ent->client->blasterType = BLASTER_NORMAL;
	ent->client->rocketType = ROCKET_NORMAL;
    ent->client->blindBase = 0;
    ent->client->blindTime = 0;
	ent->client->hook = NULL;				
	ent->client->hook_touch = NULL;
	ent->client->hook_frame = 0;
	ent->client->burntime = 0;
	ent->client->gibtime = 0;
	ent->client->rank = 0;
	ent->client->next_try = 0;
	ent->client->burnframe = 0;
	ent->client->gibframe = 0;
	
}

void K2_ResetClientKeyVars(edict_t *ent)
{
	ent->s.effects = 0;
	ent->s.renderfx = 0;
	ent->client->key = 0;
	ent->client->k2_regeneration_time = 0;
	ent->client->k2_key_framenum = 0;
}



void K2_BonusFrags(edict_t *self,edict_t *inflictor, edict_t *attacker)
{
	int bonus=0;
	int i;
	//K2: Bonus if attacker did not have a key, but killed a player with a key
	
	//If we've gotten here, self has a key

	//K2: Bonus if attacker did not have a key, but killed a player with a key
	if(!attacker->client->key)
	{
		//BFK Key and Stealth key gives 3 more (4 total)
		//Futility and Homing give 2 more (3 total)
		//Others give 1 more (2 total)
		if( K2_IsBFK(self) || K2_IsStealth(self))
			bonus=3;
		else if ( K2_IsFutility(self) || K2_IsHoming(self))
			bonus=2;
		else
			bonus=1;
	}
	else if ( (attacker->client->key) &&
		      ( K2_IsBFK(self) || K2_IsStealth(self) || K2_IsHoming(self) )
			)
		bonus=1;
		
	if(bonus > 0)
	{
		attacker->client->resp.score += bonus;
		attacker->client->resp.bonus_frags += bonus;
		for(i=0;i<bonus;i++)
		{
			if(!attacker->bot_client) //Only log if attacker is a real player
			{
				//If self is a bot and there is no botfraglogging, break;
				if(self->bot_client && !botfraglogging->value)
					goto next;
				
				sl_WriteStdLogDeath( &gi, level, self, inflictor, attacker);	// StdLog - Mark Davies
				if(qwfraglog->value)
					WriteQWLogDeath(self,inflictor,attacker);
			}
		}
next:
	
		safe_centerprintf(attacker,"You got %i bonus frags!\n",bonus);
	}
}

qboolean SameTeam(edict_t *plyr1, edict_t *plyr2);
qboolean IsVisible(edict_t *plyr1, edict_t *plyr2);
void	K2_TakePlayerKey(edict_t *self)
{
	edict_t	*player = NULL;
	int		player_index = 0;

	edict_t *closest_player = NULL;
	int		distance;
	vec3_t	v;
	int		shortest = 1000;
	int		i;

	//Look at 4 players
	for(i=0;i<4;i++)
	{
		if ((player = findradius(player,self->s.origin,1000)) != NULL)
		{
			if (!player->client)
				continue;
			
			if (player == self)
				continue;
			
			if(!player->client->key)
				continue;

			if(ctf->value)
				if(player->client->resp.ctf_team == self->client->resp.ctf_team)
					continue;
			
			
			//Make sure player is visible and infront
			if(IsVisible(self,player))// && infront(self,player))
			{
				VectorSubtract(self->s.origin, player->s.origin, v);
				distance = VectorLength(v);

				if (distance < shortest)
				{
					closest_player = player;
					shortest = distance;
				}
	
			}
			
		}

	}
	
	if(!closest_player)
	while(player_index < num_players)
	{
		//gi.dprintf("Checking player index %i\n",player_index);
		closest_player = players[player_index++];
		if ( (!closest_player->bot_client) ||
			 (closest_player == self) ||
			 (!closest_player->client->key) ||
			 (SameTeam(self,closest_player)) ||
			 (!IsVisible(self,closest_player))
		   )
			closest_player = NULL;
		else
			break;
	
	
	}

	if(!closest_player)
	{
		safe_cprintf(self,PRINT_HIGH,"No one around to take a key from\n");
		return;
	}
	else
	{
		player = closest_player;
		//ReSpawn AntiKey first
		K2_RemoveKeyFromInventory(self);
		K2_SpawnKey(self,self->client->key,4);
		K2_ResetClientKeyVars(self);
		
		//Take other player's key
		self->client->key = player->client->key;
		
		//Reset other players Key vars
		K2_RemoveKeyFromInventory(player);
		K2_ResetClientKeyVars(player);
			
		//Set the timer
		self->client->k2_key_framenum = K2_SetClientKeyTimer(self->client->key);
				
		//Print messages and make sound effects
		gi.sound(self, CHAN_ITEM, gi.soundindex("items/m_health.wav"), 1, ATTN_NORM, 0);
		gi.sound(player, CHAN_ITEM, gi.soundindex("items/m_health.wav"), 1, ATTN_NORM, 0);
		
		safe_centerprintf(player,"%s took your key!\n",self->client->pers.netname);
		safe_centerprintf(self,"You took %s's key!\n",player->client->pers.netname);

	}

}

qboolean	K2_IsHaste(edict_t *ent)
{
	if (!ent->client)
		return false;

	if (!ent->client->key)
		return false;

	if( ent->client->key == KEY_HASTE)
		return true;
	else
		return false;
}

qboolean	K2_IsRegen(edict_t *ent)
{
	if (!ent->client)
		return false;

	if (!ent->client->key)
		return false;

	if(ent->client->key == KEY_REGENERATION)
		return true;
	else
		return false;

}

qboolean	K2_IsInfliction(edict_t *ent)
{
	if (!ent->client)
		return false;

	if (!ent->client->key)
		return false;

	if( ent->client->key == KEY_INFLICTION)
		return true;
	else
		return false;

}

qboolean	K2_IsFutility(edict_t *ent)
{
	if (!ent->client)
		return false;

	if (!ent->client->key)
		return false;

	if( ent->client->key == KEY_FUTILITY)
		return true;
	else
		return false;

}

qboolean	K2_IsHoming(edict_t *ent)
{
	if (!ent->client)
		return false;
	
	if (!ent->client->key)
		return false;

	if( ent->client->key == KEY_HOMING)
		return true;
	else
		return false;
}

qboolean	K2_IsBFK(edict_t *ent)
{
	if (!ent->client)
		return false;

	if (!ent->client->key)
		return false;

	if( ent->client->key == KEY_BFK)
		return true;
	else
		return false;
}

qboolean	K2_IsAnti(edict_t *ent)
{
	if (!ent->client)
		return false;

	if (!ent->client->key)
		return false;

	if( ent->client->key == KEY_ANTIKEY)
		return true;
	else
		return false;
}

qboolean	K2_IsStealth(edict_t *ent)
{
	if (!ent->client)
		return false;

	if (!ent->client->key)
		return false;

	if( ent->client->key == KEY_STEALTH)
		return true;
	else
		return false;
}

qboolean K2_IsProtected(edict_t *ent)
{
	if (!ent->client)
		return false;

	if (ent->client->protecttime > level.time)
		return true;
	else
		return false;
}

qboolean K2_CanPickupKey(edict_t *ent)
{
	if (ent->client->next_key_pickup > level.time)
	{
		if (!ent->client->next_try || (level.framenum > ent->client->next_try))
		{
			safe_centerprintf(ent,"You can't have a key for\n%i more seconds\n",
		              (int)(ent->client->next_key_pickup - level.time));
			gi.sound(ent,CHAN_ITEM,gi.soundindex("misc/secret.wav"),1,ATTN_NORM,0);

			ent->client->next_try = level.framenum + 30;
		}
		
		return false;
	}
	else
		return true;
}

void WriteQWLogDeath(edict_t *self, edict_t *inflictor, edict_t *attacker)
{
	// Only print information to Log if QW loging is on
	if ( !qwfraglog->value )
		return;

	if (!QWLogFile)
		gi.error ("Couldn't access QW Log File\n");

	// A Suicide occured
	if (attacker == self)
		fprintf(QWLogFile,"/%s/%s/\n",self->client->pers.netname,self->client->pers.netname);
	//Normal Frag
	else if (attacker && attacker->client)
		fprintf(QWLogFile,"/%s/%s/\n",attacker->client->pers.netname,self->client->pers.netname);
	//Strange suicide death
	else 		
		fprintf(QWLogFile,"/%s/%s/\n",self->client->pers.netname,self->client->pers.netname);
	
}


/*
================
Cycle_Weapon
================
*/
void Cycle_Weapon (edict_t *ent, gitem_t *item)
{
	int			ammo_index;
	gitem_t		*ammo_item;

	if (item == ent->client->pers.weapon)
			return;

	ent->client->blasterType = BLASTER_NORMAL;	
	ent->client->rocketType = ROCKET_NORMAL;		
	ent->client->grenadeType = GRENADE_NORMAL;
	
	if (item->ammo && !g_select_empty->value && !(item->flags & IT_AMMO))
	{
		ammo_item = FindItem(item->ammo);
		ammo_index = ITEM_INDEX(ammo_item);

		if (!ent->client->pers.inventory[ammo_index])
		{
			safe_cprintf (ent, PRINT_HIGH, "No %s for %s.\n", ammo_item->pickup_name, item->pickup_name);
			return;
		}

		if (ent->client->pers.inventory[ammo_index] < item->quantity)
		{
			safe_cprintf (ent, PRINT_HIGH, "Not enough %s for %s.\n", ammo_item->pickup_name, item->pickup_name);
			return;
		}
	}
	
	// change to this weapon when down
	ent->client->newweapon = item;
	
}

/*
K2_SetClientKeyTimer - Sets the client's key timer (in frames)
*/
float K2_SetClientKeyTimer (int key)
{

	if (key == KEY_REGENERATION) 
		return level.framenum + (regentime->value * 10);
	else if (key == KEY_HASTE)
		return level.framenum + (hastetime->value * 10);
	else if (key == KEY_FUTILITY)
		return level.framenum + (futilitytime->value * 10);
	else if (key == KEY_INFLICTION)
		return level.framenum + (inflictiontime->value * 10);
	else if (key == KEY_BFK)
		return level.framenum + (bfktime->value * 10);
	else if (key == KEY_STEALTH)
		return level.framenum + (stealthtime->value * 10);
	else if (key == KEY_ANTIKEY)
		return level.framenum + (antitime->value * 10);
	else if (key == KEY_HOMING)
		return level.framenum + (homingtime->value * 10);
	
	return 0.0;

}


/*
==============
K2_InitClientPersistant

This is only called if server is using swaat rules, when starting
with more weapons, health, etc.
==============
*/
void K2_InitClientPersistant (gclient_t *client)
{
	gitem_t		*item = NULL;
	int			index;
	
	memset (&client->pers, 0, sizeof(client->pers));

	//Add weapons to inventory
	
	item = FindItem("Blaster");
	client->pers.inventory[ITEM_INDEX(item)] = 1;
	
	if(giveshotgun->value)
	{
		item = FindItem("Shotgun");
		client->pers.inventory[ITEM_INDEX(item)] = 1;
	}
	if(givesupershotgun->value)
	{
		item = FindItem("Super Shotgun");
		client->pers.inventory[ITEM_INDEX(item)] = 1;
	}
	if(givemachinegun->value)
	{
		item = FindItem("Machinegun");
		client->pers.inventory[ITEM_INDEX(item)] = 1;
	}
	if(givechaingun->value)
	{
		item = FindItem("Chaingun");
		client->pers.inventory[ITEM_INDEX(item)] = 1;
	}
	
	if(givegrenadelauncher->value)
	{
		item = FindItem("Grenade Launcher");
		client->pers.inventory[ITEM_INDEX(item)] = 1;
	}
	if(giverocketlauncher->value)
	{
		item = FindItem("Rocket Launcher");
		client->pers.inventory[ITEM_INDEX(item)] = 1;
	}
	if(givehyperblaster->value)
	{
		item = FindItem("Hyperblaster");
		client->pers.inventory[ITEM_INDEX(item)] = 1;
	}
	if(giverailgun->value)
	{
		item = FindItem("Railgun");
		client->pers.inventory[ITEM_INDEX(item)] = 1;
	}
	if(givebfg->value)
	{
		item = FindItem("BFG10K");
		client->pers.inventory[ITEM_INDEX(item)] = 1;
	}


	//Add armor
	
	if(startingarmorcount->value)
	{
		if((int)startingarmortype->value == 1)
			index = ITEM_INDEX(FindItem("Combat Armor"));
		else if((int)startingarmortype->value == 2)
			index = ITEM_INDEX(FindItem("Jacket Armor"));
		else if((int)startingarmortype->value == 3)
			index = ITEM_INDEX(FindItem("Body Armor"));
		else
			index = 0;
		
		if(index)		
		{
			client->pers.inventory[index] = startingarmorcount->value;
		}
		else
			gi.dprintf("***KEYS2 ERROR!! - Starting armor type is invalid!\n");

	}
	

	//Add ammo
	if(startingshells->value)
	{

		client->pers.inventory[ITEM_INDEX(FindItem("Shells"))] += (int)startingshells->value;
	}
	
	if(startinggrenades)
	{
		client->pers.inventory[ITEM_INDEX(FindItem("Grenades"))] += (int)startinggrenades->value;
	}

	if(startingrockets)
	{
		client->pers.inventory[ITEM_INDEX(FindItem("Rockets"))] += (int)startingrockets->value;
	}

	if(startingslugs)
	{
		client->pers.inventory[ITEM_INDEX(FindItem("Slugs"))] += (int)startingslugs->value;
	}

	if(startingcells)
	{
		client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] += (int)startingcells->value;
	}

	if(startingbullets)
	{
		client->pers.inventory[ITEM_INDEX(FindItem("Bullets"))] += (int)startingbullets->value;
	}

	//Select weapon to start with
	if( (int)startingweapon->value == 1)
		item = FindItem("Blaster");
	else if( (int)startingweapon->value == 2 && !noshotgun->value && giveshotgun->value && (startingshells->value) )
		item = FindItem("Shotgun");
	else if( (int)startingweapon->value == 3 && !nosupershotgun->value && givesupershotgun->value && ((int)startingshells->value > 1) )
		item = FindItem("Super Shotgun");
	else if( (int)startingweapon->value == 4 && !nomachinegun->value && givemachinegun->value && (startingbullets->value) )
		item = FindItem("Machinegun");
	else if( (int)startingweapon->value == 5 && !nochaingun->value && givechaingun->value && (startingbullets->value) )
		item = FindItem("Chaingun");
	else if( (int)startingweapon->value == 6 && !nogrenadelauncher->value && givegrenadelauncher->value && (startinggrenades->value) )
		item = FindItem("Grenade Launcher");
	else if( (int)startingweapon->value == 7 && !norocketlauncher->value && giverocketlauncher->value && (startingrockets->value) )
		item = FindItem("Rocket Launcher");
	else if( (int)startingweapon->value == 8 && !nohyperblaster->value && givehyperblaster->value && (startingcells->value) )
		item = FindItem("Hyperblaster");
	else if( (int)startingweapon->value == 9 && !norailgun->value && giverailgun->value && (startingslugs->value) )
		item = FindItem("Railgun");
	else if( (int)startingweapon->value == 10 && !nobfg->value && givebfg->value && ((int)startingcells->value >= 50) )
		item = FindItem("BFG10K");
	else
		item = FindItem("Blaster");
	
	client->pers.selected_item = ITEM_INDEX(item);
			
	client->pers.weapon = item;
	//ZOID
	client->pers.lastweapon = item;
	//ZOID

	//ZOID
	item = FindItem("Grapple");
	client->pers.inventory[ITEM_INDEX(item)] = 1;
	//ZOID

	client->pers.health			= (int)startinghealth->value;
	client->pers.max_health		= (int)maxhealth->value;

	client->pers.max_bullets	= (int)maxbullets->value;
	client->pers.max_shells		= (int)maxshells->value;
	client->pers.max_rockets	= (int)maxrockets->value;
	client->pers.max_grenades	= (int)maxgrenades->value;
	client->pers.max_cells		= (int)maxcells->value;
	client->pers.max_slugs		= (int)maxslugs->value;

	client->pers.connected = true;

	//K2:Begin - Clear these vars
	client->key = 0;
	client->k2_regeneration_time = 0;
	client->k2_key_framenum = 0;
	//K2:End

}

int	K2_KeyType(edict_t *ent)
{
	//ent passed is the key entity

	if (Q_stricmp(ent->item->pickup_name,"Regeneration Key") == 0)
		return KEY_REGENERATION;
	else if (Q_stricmp(ent->item->pickup_name,"Infliction Key") == 0)
		return KEY_INFLICTION;
	else if (Q_stricmp(ent->item->pickup_name,"Haste Key") == 0)
		return KEY_HASTE;
	else if (Q_stricmp(ent->item->pickup_name,"Futility Key") == 0)
		return KEY_FUTILITY;
	else if (Q_stricmp(ent->item->pickup_name,"Stealth Key") == 0)
		return KEY_STEALTH;
	else if (Q_stricmp(ent->item->pickup_name,"Anti-Key") == 0)
		return KEY_ANTIKEY;
	else if (Q_stricmp(ent->item->pickup_name,"Homing Key") == 0)
		return KEY_HOMING;
	else if (Q_stricmp(ent->item->pickup_name,"BFK") == 0)
		return KEY_BFK;
	else
		return 0;
	
}
void K2_SetClientEffects(edict_t *ent)
{
	int	FRAME;

	if ( (ent->client->invincible_framenum > level.framenum) ||
		 (ent->client->quad_framenum > level.framenum))
		FRAME = 4;
	else
		FRAME = 8;

	if(ent->client->resp.inServer || ent->bot_client)
		ent->svflags = 0;
	if(ent->client->resp.spectator)
	{
		ent->svflags = SVF_NOCLIENT;
		return;
	}

	
	//Stealth effect, but make player visible if he's firing or takes damage
	if(K2_IsStealth(ent))
	{
		if ((ent->client->anim_priority == ANIM_PAIN) ||
			(ent->client->anim_priority == ANIM_ATTACK))
		{
			
			ent->svflags = 0;
			ent->s.renderfx &= (RF_BEAM|RF_TRANSLUCENT);
		}
		else
		{
			if(totalstealth->value)
				ent->svflags |= SVF_NOCLIENT;
			
			ent->s.renderfx |= (RF_BEAM|RF_TRANSLUCENT);
		}
	}
	
	//Frozen
	if (ent->client->freezeTime > level.framenum)
	{
		ent->s.effects = EF_FROZEN;
		ent->s.renderfx = 0;
		ent->svflags = 0;
		return;
	}
	else
		ent->s.effects &= EF_FROZEN;

	//Alternate shells with normal effects/skin color every second when playing CTF
	if ( ctf->value && (level.framenum & FRAME) )
		return;


	//BFK Key always has shell
	if(K2_IsBFK(ent))
	{
			ent->s.effects |= EF_COLOR_SHELL | EF_HYPERBLASTER;
			ent->s.renderfx |= RF_SHELL_RED;
				
	}
	
	//Other shells
	if( playershells->value)
	{
			if(K2_IsInfliction(ent))
			{
				ent->s.effects |= EF_COLOR_SHELL;
				ent->s.renderfx = RF_SHELL_BLUE;
			}
			else if(K2_IsHaste(ent))
			{
				ent->s.effects |= EF_COLOR_SHELL;
				ent->s.renderfx = (RF_SHELL_GREEN | RF_SHELL_RED);
			}
			else if(K2_IsFutility(ent))
			{
				ent->s.effects |= EF_COLOR_SHELL;
				ent->s.renderfx = (RF_SHELL_GREEN | RF_SHELL_BLUE);
			}
			else if(K2_IsRegen(ent))
			{
				ent->s.effects |= EF_COLOR_SHELL;
				ent->s.renderfx = (RF_SHELL_RED | RF_SHELL_BLUE);
			}
			else if(K2_IsAnti(ent))
			{
				ent->s.effects |= EF_COLOR_SHELL;
				ent->s.renderfx = (RF_SHELL_RED|RF_SHELL_BLUE|RF_SHELL_GREEN);
			}
			else if(K2_IsHoming(ent))
			{
				ent->s.effects |= (EF_COLOR_SHELL | EF_HYPERBLASTER);
				ent->s.renderfx = RF_SHELL_GREEN;
			}
	}

	//Spawn protect
	if(ent->client->protecttime > level.time)
		ent->s.effects |= EF_TELEPORTER;
	else
		ent->s.effects &= ~EF_TELEPORTER;
	

}
void CTFSetIDView(edict_t *ent);
void K2_SetClientStats(edict_t *ent, qboolean hasPowerup)
{
	
	if (!ent->client->resp.inServer && !ent->client->menu && (ent->client->next_try < level.framenum))
	{
		
		if(ent->client->bIsCamera)
			safe_centerprintf(ent,"You are in Chase Cam mode.\nPress Fire to follow a different player\n\nPress TAB for the game menu\nOR\nType \"join\" at the console");
		else
			safe_centerprintf(ent,"You are in Observer mode.\nPress TAB for the game menu\nOR\nType \"join\" at the console");
		
		ent->client->next_try = level.framenum + 170;

	}
	if (ent->client->resp.spectator && (ent->client->next_try < level.framenum))
	{
		
		safe_centerprintf(ent,"You are in Spectator mode.\nType \"spectator 0\" at the console\nto rejoin the game");
		
		ent->client->next_try = level.framenum + 220;

	}


	if(timelimit->value && !ent->client->showscores)
	{
		if (k2_timeleft > 0)
			ent->client->ps.stats[STAT_TIMELEFT] = k2_timeleft + 1;
		else
			ent->client->ps.stats[STAT_TIMELEFT] = k2_timeleft;
	}
	else
		ent->client->ps.stats[STAT_TIMELEFT] = 0;

	//Capturelimit/Fraglimit
	if (ctf->value && capturelimit->value && !ent->client->showscores)
		ent->client->ps.stats[STAT_FRAGSLEFT] = k2_capsleft;
	else if (!ctf->value && fraglimit->value  && !ent->client->showscores)
		ent->client->ps.stats[STAT_FRAGSLEFT] = k2_fragsleft;
	else 
		ent->client->ps.stats[STAT_FRAGSLEFT] = 0;	
		
	
	//Rank
	if(!ctf->value)
		ent->client->ps.stats[STAT_RANK] = ent->client->rank;
	
	//if hasPowerup is true, alternate display every other second
	
	//Key Timers - Alternate with Quad/Pent/Enviro timer
	if (ent->client->k2_key_framenum > level.framenum)
	{
		if(hasPowerup)
		{
			if ( level.framenum & 8 ) {

				ent->client->ps.stats[STAT_KEY_ICON] = 0;
				ent->client->ps.stats[STAT_KEYTIMER] = 0;
				return;
			}
			else
			{
				ent->client->ps.stats[STAT_TIMER_ICON] = 0;
				ent->client->ps.stats[STAT_TIMER] = 0;
			}
		}
	
		if(K2_IsRegen(ent))
		{
			ent->client->ps.stats[STAT_KEY_ICON] = gi.imageindex ("k_bluekey");
			ent->client->ps.stats[STAT_KEYTIMER] = (ent->client->k2_key_framenum - level.framenum)/10;
		}
		else if(K2_IsHaste(ent))
		{
			ent->client->ps.stats[STAT_KEY_ICON] = gi.imageindex ("k_redkey");
			ent->client->ps.stats[STAT_KEYTIMER] = (ent->client->k2_key_framenum - level.framenum)/10;
		}
		else if(K2_IsFutility(ent))
		{
			ent->client->ps.stats[STAT_KEY_ICON] = gi.imageindex ("k_security");
			ent->client->ps.stats[STAT_KEYTIMER] = (ent->client->k2_key_framenum - level.framenum)/10;
		}
		else if(K2_IsInfliction(ent))
		{
			ent->client->ps.stats[STAT_KEY_ICON] = gi.imageindex ("k_datacd");
			ent->client->ps.stats[STAT_KEYTIMER] = (ent->client->k2_key_framenum - level.framenum)/10;
		}
		else if(K2_IsStealth(ent))
		{
			ent->client->ps.stats[STAT_KEY_ICON] = gi.imageindex ("k_powercube");
			ent->client->ps.stats[STAT_KEYTIMER] = (ent->client->k2_key_framenum - level.framenum)/10;
		}
		else if(K2_IsAnti(ent))
		{
			ent->client->ps.stats[STAT_KEY_ICON] = gi.imageindex ("k_dataspin");
			ent->client->ps.stats[STAT_KEYTIMER] = (ent->client->k2_key_framenum - level.framenum)/10;
		}
		else if(K2_IsHoming(ent))
		{
			ent->client->ps.stats[STAT_KEY_ICON] = gi.imageindex ("k_comhead");
			ent->client->ps.stats[STAT_KEYTIMER] = (ent->client->k2_key_framenum - level.framenum)/10;
		}
		else if (K2_IsBFK(ent))
		{
			ent->client->ps.stats[STAT_KEY_ICON] = gi.imageindex ("k_pyramid");
			ent->client->ps.stats[STAT_KEYTIMER] = (ent->client->k2_key_framenum - level.framenum)/10;
		}

	}
	else
	{
		ent->client->ps.stats[STAT_KEY_ICON] = 0;
		ent->client->ps.stats[STAT_KEYTIMER] = 0;
	}

	ent->client->ps.stats[STAT_CTF_ID_VIEW] = 0;
	if (ent->client->resp.id_state)
		CTFSetIDView(ent);

}

void K2_ChaseCam(edict_t *ent, pmenu_t *p)
{
	int i;
	edict_t *e;

	if (ent->client->chase_target) {
		ent->client->chase_target = NULL;
		K2Menu_Close(ent);
		return;
	}

	for (i = 1; i <= maxclients->value; i++) {
		e = g_edicts + i;
		if (e->inuse && e->solid != SOLID_NOT && !e->bot_client) {
			ent->client->chase_target = e;
			K2Menu_Close(ent);
			ent->client->update_chase = true;
			break;
		}
	}
}

qboolean K2_StartClient(edict_t *ent)
{
	if(ent->client->resp.inServer)
		return false;

	ent->movetype = MOVETYPE_NOCLIP;
	ent->solid = SOLID_NOT;
	ent->svflags |= SVF_NOCLIENT;
	ent->client->ps.gunindex = 0;
	ent->client->resp.inServer=false;
	gi.linkentity (ent);

	//K2_OpenJoinMenu(ent);
	K2_OpenWelcomeMenu(ent);
	return true;
		
}
void botAddPlayer(edict_t *ent);
void K2EnterGame(edict_t *ent, pmenu_t *p)
{
	//char *s;
	
	//Check if player in cam mode 
	if(ent->client->bIsCamera)
	{
		ent->client->bIsCamera = false;
		botAddPlayer(ent);
	}
	
	K2Menu_Close(ent);

	ent->svflags &= ~SVF_NOCLIENT;
	ent->client->resp.inServer=true;
	PutClientInServer (ent);
	
	// add a teleportation effect
	ent->s.event = EV_PLAYER_TELEPORT;
	
	// hold in place briefly
	ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
	ent->client->ps.pmove.pm_time = 14;

	my_bprintf (PRINT_HIGH, "%s has entered the Fragfest\n", ent->client->pers.netname);
	
}

void	DummyThink(edict_t*ent)
{
	return;
}

void K2_FixGSLogFile(void)
{
	char	buffer1[256];
	char	buffer2[256];
	int		i, j, lines=0;
	char	*pdest;
	cvar_t  *filename	= gi.cvar( "stdlogname", "StdLog.log", CVAR_SERVERINFO );
	FILE	*StdLogFile	= NULL;
	FILE	*tempFile	= NULL;

	char	*pName		= "StdLog.log";
	char	*tName		= "TempStd.log";
	
	qboolean isbad = false;

	//If file does not exist, return;
	
	// Open Files
    if( filename )
		pName = filename->string;
    
	StdLogFile = fopen(pName,"r+t" );
	tempFile   = fopen(tName,"a+t");

    if( !StdLogFile )
    {
         //File does not exist, so everything is cool?
		return;
    }

	if( !tempFile )
    {
         printf("Couldn't open temporary file %s...logging OFF\n", tName );
		 gi.cvar_forceset("stdlogfile","0");
		 fclose(StdLogFile);
		 return;
    }

	printf("Verifying %s\n",pName);

	//Count lines
	while(!feof(StdLogFile))
	{
		i=0;
		fscanf(StdLogFile, "%c", &buffer1[i]);
		while ((buffer1[i] != '\n') && (i < 255))
		{
			i++;
			fscanf(StdLogFile, "%c", &buffer1[i]);

			if (buffer1[i] == '\n')
				break;
		}
		buffer1[i+1] = '\0';
		
		if(strchr(buffer1,'\t'))
		{
			lines++;
			strcpy(buffer2,buffer1);
			strcpy(buffer1,"");
		}
		else
			break;
	}

	//Check the last line..if good GameEnd, write all the lines
	//Else, write up to lines-1
	pdest = strstr(buffer2,"\t\tGameEnd");
	if(pdest)
		gi.dprintf("GameEnd is good\n");
	else
	{
		printf("GameEnd is bad..adjusted\n");
		isbad=true;
		lines--;
	}

	//Ok, let's re-read and output all the lines
	fseek(StdLogFile,0,0);
	for(j=0;j<lines;j++)
	{
		i=0;
		fscanf(StdLogFile, "%c", &buffer1[i]);
		while ((buffer1[i] != '\n') && (i < 255))
		{
			i++;
			fscanf(StdLogFile, "%c", &buffer1[i]);

			if (buffer1[i] == '\n')
				break;
		}
		buffer1[i+1] = '\0';
		
		fprintf(tempFile,"%s",buffer1);
	}
	if (isbad)
		fprintf(tempFile,"\t\tGameEnd\t\t\t0.0\n");
		
	fclose(tempFile);
	fclose(StdLogFile);

	remove(pName);
	if (rename(tName,pName) == 0)
		gi.dprintf("%s verify completed\n",pName);
	else
		gi.dprintf("%s verify failed\n",pName);
}

void K2_Regeneration(edict_t *ent)
{

	//K2:Begin - These vars are for armor checking for REGENERATION Key
	int				index;
	gitem_t			*armor_item;
	gitem_armor_t	*armor_info;
	gclient_t		*client;

	client = ent->client;

	//K2:End

	//K2:Begin
	//REGENERATION Key -  Armor and Health Regen
	if ( K2_IsRegen(ent) || K2_IsBFK(ent) )
	{
		//Check to see if client has armor
		index = ArmorIndex (ent);
		if (index)
		{
			armor_item = GetItemByIndex(index);
			armor_info = (gitem_armor_t *)armor_item->info;
		}
		
			
		if (ent->client->k2_regeneration_time <= level.framenum)
			{
		//Play the sound if we're gonna regenerate something
			if ( (ent->health < ent->max_health) || (index && client->pers.inventory[index] < armor_info->max_count) )
				gi.sound(ent,CHAN_ITEM,gi.soundindex("items/s_health.wav"), 1, ATTN_STATIC, 0);
					
						
			//REGENERATE the health
			if ( ent->health < ent->max_health )
			{	
				ent->health += 5;
				if (ent->health > ent->max_health) 
					ent->health = ent->max_health;
			}
				            
			//REGENERATE the armor
			if ( index && client->pers.inventory[index] < armor_info->max_count)
			{
					if (client->pers.inventory[index] < armor_info->max_count)
							client->pers.inventory[index] += 5;
					if (client->pers.inventory[index] > armor_info->max_count) 
								client->pers.inventory[index] = armor_info->max_count;
				
			}
		
			ent->client->k2_regeneration_time = level.framenum + 5;

		}
	}

}

void K2_KeyExpiredCheck(edict_t *ent)
{
	//Check for expired keys
	if ( ent->client->key && ((ent->client->k2_key_framenum - level.framenum) <= 0) )
	{
		
		K2_RemoveKeyFromInventory(ent);
		K2_SpawnKey(NULL,ent->client->key,0);
		K2_ResetClientKeyVars(ent);
	}
}



void		K2_ClearPrevOwner(edict_t *self)
{

	edict_t *ent = NULL;
	//Find all the keys laying around and check it's prev_owner.
	//If prev_owner == self, set it to NULL

	while ((ent = findradius(ent,self->s.origin,10000)) != NULL)
	{
		if (!ent->item)
			continue;

		if (!ent->prev_owner)
			continue;
		
		if (ent->prev_owner == self)
			ent->prev_owner = NULL;
	}


}

void K2_LogPlayerIP(char *userinfo)
{
    FILE *logfile;
	char	*ip, *name;
	char filename[50]="";
    cvar_t *gamed = gi.cvar ("game","baseq2", CVAR_SERVERINFO );
		
	sprintf ( filename, "%s/connect.log", gamed->string );
       
    // Open File
            
    logfile = fopen( filename, "a+t" );

    if( NULL == logfile )
    {
		gi.dprintf("Couldn't open %s", filename);
		return;
    }

	ip = Info_ValueForKey (userinfo, "ip");
	name = Info_ValueForKey (userinfo, "name");


	fprintf(logfile,"%s/%s\n",name,ip);
	
	if( NULL != logfile )
    {
        fclose( logfile );
    }

    
}

void	K2ApplyHasteSound(edict_t *ent)
{

	//K2:Begin - Also make the haste sound if needed
	if ( K2_IsHaste(ent) || K2_IsBFK(ent) ) 
		gi.sound(ent, CHAN_AUTO, gi.soundindex("boss3/BS3ATCK2.WAV"), 0.8, ATTN_NORM, 0);
	//K2:End
}
