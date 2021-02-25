// g_weapon.c

#include "g_local.h"
#include "b_player.h"

extern float xyspeed;
extern	int	jacket_armor_index;
extern	int	combat_armor_index;
extern	int	body_armor_index;

void weapon_reload (edict_t *ent);

void P_ProjectSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result)
{
	vec3_t	_distance;

	VectorCopy (distance, _distance);
	if (client->pers.hand == LEFT_HANDED)
		_distance[1] *= -1;
	else if (client->pers.hand == CENTER_HANDED)
		_distance[1] = 0;
	G_ProjectSource (point, _distance, forward, right, result);
}


/*
===============
PlayerNoise

Each player can have two noise objects associated with it:
a personal noise (jumping, pain, weapon firing), and a weapon
target noise (bullet wall impacts)

Monsters that don't directly see the player can move
to a noise in hopes of seeing the player from there.
===============
*/
void PlayerNoise(edict_t *who, vec3_t where, int type)
{
	edict_t		*noise;

	if (type == PNOISE_WEAPON)
	{
		if (who->client->silencer_shots)
		{
			who->client->silencer_shots--;
			return;
		}
	}

	if (deathmatch->value)
		return;

	if (who->flags & FL_NOTARGET)
		return;


	if (!who->mynoise)
	{
		noise = G_Spawn();
		noise->classname = "player_noise";
		VectorSet (noise->mins, -8, -8, -8);
		VectorSet (noise->maxs, 8, 8, 8);
		noise->owner = who;
		noise->svflags = SVF_NOCLIENT;
		who->mynoise = noise;

		noise = G_Spawn();
		noise->classname = "player_noise";
		VectorSet (noise->mins, -8, -8, -8);
		VectorSet (noise->maxs, 8, 8, 8);
		noise->owner = who;
		noise->svflags = SVF_NOCLIENT;
		who->mynoise2 = noise;
	}

	if (type == PNOISE_SELF || type == PNOISE_WEAPON)
	{
		noise = who->mynoise;
		level.sound_entity = noise;
		level.sound_entity_framenum = level.framenum;
	}
	else // type == PNOISE_IMPACT
	{
		noise = who->mynoise2;
		level.sound2_entity = noise;
		level.sound2_entity_framenum = level.framenum;
	}

	VectorCopy (where, noise->s.origin);
	VectorSubtract (where, noise->maxs, noise->absmin);
	VectorAdd (where, noise->maxs, noise->absmax);
	noise->teleport_time = level.time;
	gi.linkentity (noise);
}


qboolean Pickup_Weapon (edict_t *ent, edict_t *other)
{
	int			index;
	gitem_t		*ammo;

	index = ITEM_INDEX(ent->item);

	if ( ( ((int)(dmflags->value) & DF_WEAPONS_STAY) || coop->value) 
		&& other->client->pers.inventory[index])
		{
		if (!(ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM) ) )
			return false;	// leave the weapon for others to pickup
		}

	other->client->pers.inventory[index]++;

	if (!(ent->spawnflags & DROPPED_ITEM) )
		{
		// Don't give ammo for picking up staff
		if (strcmp(ent->classname, "weapon_staff") && ent->item)
		   {
			// give them some ammo with it
			ammo = FindItem (ent->item->ammo);
			if ( (int)dmflags->value & DF_INFINITE_AMMO )
				Add_Ammo (other, ammo, 1000);
			else
				Add_Ammo (other, ammo, ammo->quantity);
		   }

		if (! (ent->spawnflags & DROPPED_PLAYER_ITEM) )
			{
			if (deathmatch->value)
			{
				if ((int)(dmflags->value) & DF_WEAPONS_STAY)
					ent->flags |= FL_RESPAWN;
				else
					SetRespawn (ent, 30);
			}
			if (coop->value)
				ent->flags |= FL_RESPAWN;
			}
		}

	if (other->client->pers.weapon != ent->item && 
		(other->client->pers.inventory[index] == 1) &&
		( !deathmatch->value || other->client->pers.weapon == FindItem("hammer") ) )
		other->client->newweapon = ent->item;

	return true;
}


/*
===============
ChangeWeapon

The old weapon has been dropped all the way, so make the new one
current
===============
*/

void ShowGun(edict_t *ent)
{
	char heldmodel[128], armormodel[128], s[128], skin[128];
	int len;


	if (!ent->client->pers.weapon)
	   {
		ent->s.modelindex2 = 0;
		return;
	   }

	strcpy(heldmodel, Info_ValueForKey (ent->client->pers.userinfo, "skin"));
	for (len = 0; heldmodel[len]; len++)
	    {
		 if (heldmodel[len] == '/')
          {
          strcpy(s, &heldmodel[len+1]);
		    heldmodel[len] = 0;
          }
	    }
   if (heldmodel[len - 2] == '_')
      {
      len -= 2;
      heldmodel[len] = 0;
      }
   if (!strcmp(ent->client->pers.weapon->icon, "w_hammer"))
      strcat(heldmodel, "_h/");	
   //else if (!strcmp(ent->client->pers.weapon->icon, "w_staff"))
      //strcat(heldmodel, "_h/");	
   else if (!strcmp(ent->client->pers.weapon->icon, "w_u23k2"))
      strcat(heldmodel, "_g/");	
   else if (!strcmp(ent->client->pers.weapon->icon, "w_wasp"))
      strcat(heldmodel, "_p/");	
   else if (!strcmp(ent->client->pers.weapon->icon, "w_u23k"))
      strcat(heldmodel, "_p/");
   else /*if (!strcmp(ent->client->pers.weapon->icon, "w_u23k"))*/
      strcat(heldmodel, "/");	
   strcpy(skin, heldmodel);
   strcat(skin, s);
   gi.configstring (CS_PLAYERSKINS+(ent-g_edicts-1), 
      va("%s\\%s", ent->client->pers.netname, skin));
	//gi.dprintf ("%s\n", heldmodel);
	//gi.dprintf ("%s\n", skin);
	//ent->s.modelindex2 = gi.modelindex(heldmodel);	

   strcpy(armormodel, "players/");
   strcat(armormodel, heldmodel);
   strcpy(heldmodel, armormodel);   
	//gi.dprintf("%s - %s\n", heldmodel, armormodel);
   //strcat(heldmodel, "weapon.md2");	
   strcat(heldmodel, ent->client->pers.weapon->icon);	
	strcat(heldmodel, ".md2");
   ent->s.modelindex2 = gi.modelindex(heldmodel);
	if (ent->client->pers.inventory[body_armor_index] > 0)
      {
      strcat(armormodel, "armor.md2");	
      ent->s.modelindex3 = gi.modelindex(armormodel);
      }
	else if (ent->client->pers.inventory[combat_armor_index] > 0)
      {
      strcat(armormodel, "armor.md2");	
      ent->s.modelindex3 = gi.modelindex(armormodel);
      }
   else if (ent->client->pers.inventory[jacket_armor_index] > 0)
      {
      strcat(armormodel, "armor.md2");	
      ent->s.modelindex3 = gi.modelindex(armormodel);		
      }
}

void ChangeWeapon (edict_t *ent)
{
	ent->client->pers.lastweapon = ent->client->pers.weapon;
	ent->client->pers.weapon = ent->client->newweapon;
	ent->client->newweapon = NULL;

   ent->status &= ~STATUS_IR;
   if (ent->client->pers.weapon)
		{
      if (strcmp(ent->client->pers.weapon->pickup_name, "Assimilator"))
			{
			if (ent->client->locked)
				{
				ent->client->locked->s.renderfx &= ~RF_SHELL_RED;
				gi.linkentity(ent->client->locked);				
				ent->client->locked = NULL;
				}
			}
      if (!strcmp(ent->client->pers.weapon->pickup_name, "Rifle"))
         ent->client->machinegun_shots = 1;
		else
         ent->client->machinegun_shots = 0;
		if (ent->client->pers.weapon->ammo)
			ent->client->ammo_index = ITEM_INDEX(FindItem(ent->client->pers.weapon->ammo));
		else
			ent->client->ammo_index = 0;
		}
	else
	   {
		ent->client->ps.gunindex = 0;
		return;
	   }

	ent->client->weaponstate = WEAPON_ACTIVATING;
	ent->client->ps.gunframe = 0;
   ent->client->weapon_mode = 0;
   if (!ent->client->camera_model)
	   ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);
	ShowGun(ent);	
}

/*
=================
NoAmmoWeaponChange
=================
*/
void NoAmmoWeaponChange (edict_t *ent)
{
	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("Clip"))])
	{
      if (ent->client->pers.inventory[ITEM_INDEX(FindItem("u23k2"))])
         {
		   ent->client->newweapon = FindItem ("u23k2");
         return;
         }
	   if (ent->client->pers.inventory[ITEM_INDEX(FindItem("u23k"))])
	      {
		   ent->client->newweapon = FindItem ("u23k");
		   return;
	      }		
	}

   ent->client->newweapon = FindItem ("hammer");
}

/*
=================
Think_Weapon

Called by ClientBeginServerFrame and ClientThink
=================
*/
void Think_Weapon (edict_t *ent)
{
	// if just died, put the weapon away
	if (ent->health < 1)
	   {
		ent->client->newweapon = NULL;
		ChangeWeapon (ent);
	   }
/*
	if (ent->client->locked)
		if (ent->client->locked->health > 0)
			{
			if (!(ent->client->locked->s.renderfx & RF_SHELL_RED))
				{
				ent->client->locked->s.renderfx |= RF_SHELL_RED;
				gi.linkentity(ent->client->locked);
				}
			}
		else
			{
			ent->client->locked->s.renderfx &= ~RF_SHELL_RED;
			ent->client->locked = NULL;
			gi.linkentity(ent->client->locked);
			}	
*/

	// call active weapon think routine
	if (ent->client->pers.weapon && ent->client->pers.weapon->weaponthink)
	   {
		is_quad = (ent->client->quad_framenum > level.framenum);
		if (ent->client->silencer_shots)
			is_silenced = MZ_SILENCED;
		else
			is_silenced = 0;
		ent->client->pers.weapon->weaponthink (ent);
	   }
}


/*
================
Use_Weapon

Make the weapon ready if there is ammo
================
*/
void Use_Weapon (edict_t *ent, gitem_t *item)
{
	int			ammo_index;
	gitem_t		*ammo_item;

	// see if we're already using it
	if (item == ent->client->pers.weapon)
		return;

	if (item->ammo && !g_select_empty->value && !(item->flags & IT_AMMO))
		{
		ammo_item = FindItem(item->ammo);
		ammo_index = ITEM_INDEX(ammo_item);

		if (!ent->client->pers.inventory[ammo_index])
		   {
			gi.cprintf (ent, PRINT_HIGH, "No %s for %s.\n", ammo_item->pickup_name, item->pickup_name);
			return;
		   }

		if (ammo_index && !ent->client->pers.inventory[ammo_index])
		   {
			gi.cprintf (ent, PRINT_HIGH, "Not enough %s for %s.\n", ammo_item->pickup_name, item->pickup_name);
			return;
		   }
		}

	// change to this weapon when down
   ShowGun(ent);
	ent->client->newweapon = item;
}



/*
================
Drop_Weapon
================
*/
void Drop_Weapon (edict_t *ent, gitem_t *item)
{
	int		index;

	if ((int)(dmflags->value) & DF_WEAPONS_STAY)
		return;

	index = ITEM_INDEX(item);
	// see if we're already using it
	if ( ((item == ent->client->pers.weapon) || (item == ent->client->newweapon))&& (ent->client->pers.inventory[index] == 1) )
	{
		gi.cprintf (ent, PRINT_HIGH, "Can't drop current weapon\n");
		return;
	}

	Drop_Item (ent, item);
	ent->client->pers.inventory[index]--;
}


/*
================
Weapon_Generic

A generic function to handle the basics of weapon thinking
================
*/
#define FRAME_FIRE_FIRST		(FRAME_ACTIVATE_LAST + 1)
#define FRAME_IDLE_FIRST		(FRAME_FIRE_LAST + 1)
#define FRAME_DEACTIVATE_FIRST	(FRAME_IDLE_LAST + 1)
#define FRAME_FIRE2_FIRST	(FRAME_DEACTIVATE_LAST + 1)
#define FRAME_FIRE3_FIRST	(FRAME_FIRE2_LAST + 1)

void Weapon_Generic (edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent))
{
	int		n;

  if (ent->client->reload_time == -1)
     {
     weapon_reload (ent);
     return;
     }

	if (ent->client->weaponstate == WEAPON_DROPPING)
	   {
		if (ent->client->ps.gunframe == FRAME_DEACTIVATE_LAST)
		   {
			ChangeWeapon (ent);
			return;
		   }
		ent->client->ps.gunframe++;
		return;
	   }

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	   {
		if (ent->client->ps.gunframe == FRAME_ACTIVATE_LAST)
		   {
			ent->client->weaponstate = WEAPON_READY;
			ent->client->ps.gunframe = FRAME_IDLE_FIRST;
			return;
		   }
		ent->client->ps.gunframe++;
		return;
	   }

	if ((ent->client->newweapon) && (ent->client->weaponstate != WEAPON_FIRING))
	   {
		ent->client->weaponstate = WEAPON_DROPPING;
		ent->client->ps.gunframe = FRAME_DEACTIVATE_FIRST;
		return;
	   }

	if (ent->client->weaponstate == WEAPON_READY)
	   {
		if (((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
		   {
         if (!ent->client->ammo_index || ent->client->pers.inventory[ent->client->ammo_index])
            {
			   ent->client->latched_buttons &= ~BUTTON_ATTACK;
			   ent->client->ps.gunframe = FRAME_FIRE_FIRST;
			   ent->client->weaponstate = WEAPON_FIRING;
			   // start the animation
			   ent->client->anim_priority = ANIM_ATTACK;
			   if (ent->waterlevel > 2)
               {
  			      ent->s.frame = FRAME_SwimShoot_start - 1;
   		      ent->client->anim_end = FRAME_SwimShoot_end;
               }
			   else if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
      		   {
				   ent->s.frame = FRAME_CrouchShoot_start - 1;
				   ent->client->anim_end = FRAME_CrouchShoot_end;
			      }
            else if (ent->client->sidemove > 0)
               {
               ent->s.frame = FRAME_RightShoot_start;
   	         ent->client->anim_end = FRAME_RightShoot_end;
               }
            else if (ent->client->sidemove < 0)
               {
	            ent->s.frame = FRAME_LeftShoot_start;
		         ent->client->anim_end = FRAME_LeftShoot_end;
               }
   		   else if (xyspeed > 225 && ent->groundentity)
               {
      	      ent->s.frame = FRAME_RunShoot_start - 1;
	   	      ent->client->anim_end = FRAME_RunShoot_end;
               }
            else
               {  
   			   ent->s.frame = FRAME_Shoot_start - 1;
				   ent->client->anim_end = FRAME_Shoot_end;
	   	      }
		      }
         }
		else
		   {
			if (ent->client->ps.gunframe == FRAME_IDLE_LAST)
			   {
				ent->client->ps.gunframe = FRAME_IDLE_FIRST;
				return;
			   }

			if (pause_frames)
				for (n = 0; pause_frames[n]; n++)
					if (ent->client->ps.gunframe == pause_frames[n])
						if (rand()&15)
							return;
			ent->client->ps.gunframe++;
			return;
		   }
	   }

	if (ent->client->weaponstate == WEAPON_FIRING)
	   {
		for (n = 0; fire_frames[n]; n++)
			if (ent->client->ps.gunframe == fire_frames[n])
            {				
			   fire (ent);
			   break;
            }

		if (!fire_frames[n])
			ent->client->ps.gunframe++;

		if (ent->client->ps.gunframe == FRAME_IDLE_FIRST+1)
			ent->client->weaponstate = WEAPON_READY;
	   }
}


void Weapon_Generic2 (edict_t *ent, 
                      int FRAME_ACTIVATE_LAST, 
                      int FRAME_FIRE_LAST, 
                      int FRAME_IDLE_LAST, 
                      int FRAME_DEACTIVATE_LAST, 
                      int FRAME_FIRE2_LAST, 
                      int *pause_frames, int *fire_frames, 
                      void (*fire)(edict_t *ent), 
                      void (*fire2)(edict_t *ent))
{
	int		n;

  if (ent->client->reload_time == -1)
     {
     weapon_reload (ent);
     return;
     }

	if (ent->client->weaponstate == WEAPON_DROPPING)
	   {
		if (ent->client->ps.gunframe == FRAME_DEACTIVATE_LAST)
		   {
			ChangeWeapon (ent);
			return;
		   }
		ent->client->ps.gunframe++;
		return;
	   }

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	   {
		if (ent->client->ps.gunframe == FRAME_ACTIVATE_LAST)
		   {
			ent->client->weaponstate = WEAPON_READY;
			ent->client->ps.gunframe = FRAME_IDLE_FIRST;
			return;
		   }
		ent->client->ps.gunframe++;
		return;
	   }

	if ((ent->client->newweapon) && (ent->client->weaponstate != WEAPON_FIRING))
	   {
		ent->client->weaponstate = WEAPON_DROPPING;
		ent->client->ps.gunframe = FRAME_DEACTIVATE_FIRST;
		return;
	   }

	if (ent->client->weaponstate == WEAPON_READY)
	   {
		if (((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
		   {
         if (!ent->client->ammo_index || ent->client->pers.inventory[ent->client->ammo_index])
            {
            if (rand() & 1)
      		   ent->client->ps.gunframe = FRAME_FIRE_FIRST;
            else
               ent->client->ps.gunframe = FRAME_FIRE2_FIRST;
	   	   ent->client->weaponstate = WEAPON_FIRING;
		      // start the animation
			   ent->client->anim_priority = ANIM_ATTACK;
			   if (ent->waterlevel > 2)
               {
   		      ent->s.frame = FRAME_SwimShoot_start - 1;
	   		   ent->client->anim_end = FRAME_SwimShoot_end;
               }
			   else if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
   				{
				   ent->s.frame = FRAME_CrouchShoot_start - 1;
				   ent->client->anim_end = FRAME_CrouchShoot_end;
				   }
            else if (ent->client->sidemove > 0)
               {
               ent->s.frame = FRAME_RightShoot_start;
			      ent->client->anim_end = FRAME_RightShoot_end;
               }
            else if (ent->client->sidemove < 0)
               {
			      ent->s.frame = FRAME_LeftShoot_start;
			      ent->client->anim_end = FRAME_LeftShoot_end;
               }
			   else if (xyspeed > 225 && ent->groundentity)
               {
		         ent->s.frame = FRAME_RunShoot_start - 1;
			      ent->client->anim_end = FRAME_RunShoot_end;
               }
            else
               {  
			      ent->s.frame = FRAME_Shoot_start - 1;
				   ent->client->anim_end = FRAME_Shoot_end;
				   }
			   }
         }
	   else
	      {
			if (ent->client->ps.gunframe == FRAME_IDLE_LAST)
			   {
				ent->client->ps.gunframe = FRAME_IDLE_FIRST;
				return;
			   }

			if (pause_frames)
				for (n = 0; pause_frames[n]; n++)
					if (ent->client->ps.gunframe == pause_frames[n])
						if (rand()&15)
							return;
			ent->client->ps.gunframe++;
			return;
		   }
	   }

	if (ent->client->weaponstate == WEAPON_FIRING)
	   {
		for (n = 0; fire_frames[n]; n++)
			if (ent->client->ps.gunframe == fire_frames[n])
            {  
            if (ent->client->ps.gunframe < FRAME_IDLE_LAST)
			      fire (ent);
            else
               fire2(ent);
			   break;
            }

		if (!fire_frames[n])
			ent->client->ps.gunframe++;

		if (ent->client->ps.gunframe == FRAME_IDLE_FIRST + 1)
			ent->client->weaponstate = WEAPON_READY;
		else if (ent->client->ps.gunframe == FRAME_FIRE2_LAST + 1)
         {
         ent->client->ps.gunframe = FRAME_IDLE_FIRST + 1;
			ent->client->weaponstate = WEAPON_READY;
         }
	   }
}


void Weapon_Generic3 (edict_t *ent, 
                      int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, 
                      int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, 
                      int FRAME_FIRE2_LAST, int FRAME_FIRE3_LAST, 
                      int *pause_frames, int *fire_frames, 
                      void (*fire)(edict_t *ent), 
                      void (*fire2)(edict_t *ent), 
                      void (*fire3)(edict_t *ent))
{
	int		n;

  if (ent->client->reload_time == -1)
     {
     weapon_reload (ent);
     return;
     }

	if (ent->client->weaponstate == WEAPON_DROPPING)
	   {
		if (ent->client->ps.gunframe == FRAME_DEACTIVATE_LAST)
		   {
			ChangeWeapon (ent);
			return;
		   }
		ent->client->ps.gunframe++;
		return;
	   }

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	   {
		if (ent->client->ps.gunframe == FRAME_ACTIVATE_LAST)
		   {
			ent->client->weaponstate = WEAPON_READY;
			ent->client->ps.gunframe = FRAME_IDLE_FIRST;
			return;
		   }
		ent->client->ps.gunframe++;
		return;
	   }

	if ((ent->client->newweapon) && (ent->client->weaponstate != WEAPON_FIRING))
	   {
		ent->client->weaponstate = WEAPON_DROPPING;
		ent->client->ps.gunframe = FRAME_DEACTIVATE_FIRST;
		return;
	   }

	if (ent->client->weaponstate == WEAPON_READY)
	   {
		if (((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
		   {
         if (!ent->client->ammo_index || ent->client->pers.inventory[ent->client->ammo_index])
            {
			   ent->client->latched_buttons &= ~BUTTON_ATTACK;
            switch (rand () & 3)
               {
               case 0:
               case 3:
   		         ent->client->ps.gunframe = FRAME_FIRE3_FIRST;
                  break;
               case 1:
                  ent->client->ps.gunframe = FRAME_FIRE_FIRST;
                  break;
               case 2:
                  ent->client->ps.gunframe = FRAME_FIRE2_FIRST;
                  break;
               }
   		   ent->client->weaponstate = WEAPON_FIRING;
	   	   // start the animation
			   ent->client->anim_priority = ANIM_ATTACK;
			   if (ent->waterlevel > 2)
               {
   	         ent->s.frame = FRAME_SwimShoot_start - 1;
	   	      ent->client->anim_end = FRAME_SwimShoot_end;
               }
   		   else if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
   	   	   {
				   ent->s.frame = FRAME_CrouchShoot_start - 1;
				   ent->client->anim_end = FRAME_CrouchShoot_end;
			      }
            else if (ent->client->sidemove > 0)
               {
               ent->s.frame = FRAME_RightShoot_start;
		         ent->client->anim_end = FRAME_RightShoot_end;
               }
            else if (ent->client->sidemove < 0)
               {
			      ent->s.frame = FRAME_LeftShoot_start;
			      ent->client->anim_end = FRAME_LeftShoot_end;
               }
            else if (xyspeed > 225 && ent->groundentity)
               {
  			      ent->s.frame = FRAME_RunShoot_start - 1;
 		         ent->client->anim_end = FRAME_RunShoot_end;
               }
            else
               {  
			      ent->s.frame = FRAME_Shoot_start - 1;
   		      ent->client->anim_end = FRAME_Shoot_end;
	   	      }
		      }
         }
		else
		   {
			if (ent->client->ps.gunframe == FRAME_IDLE_LAST)
			   {
				ent->client->ps.gunframe = FRAME_IDLE_FIRST;
				return;
			   }

			if (pause_frames)
				for (n = 0; pause_frames[n]; n++)
					if (ent->client->ps.gunframe == pause_frames[n])
						if (rand()&15)
							return;
			ent->client->ps.gunframe++;
			return;
		   }
	   }

	if (ent->client->weaponstate == WEAPON_FIRING)
	   {
		for (n = 0; fire_frames[n]; n++)
			if (ent->client->ps.gunframe == fire_frames[n])
            {  
            if (ent->client->ps.gunframe < FRAME_IDLE_LAST)
			      fire (ent);
            else if (ent->client->ps.gunframe < FRAME_FIRE2_LAST)
			      fire2 (ent);
            else
               fire3(ent);
			   break;
            }

		if (!fire_frames[n])
			ent->client->ps.gunframe++;

		if (ent->client->ps.gunframe == FRAME_IDLE_FIRST + 1)
			ent->client->weaponstate = WEAPON_READY;
		else if (ent->client->ps.gunframe == FRAME_FIRE2_LAST + 1)
         {
         ent->client->ps.gunframe = FRAME_IDLE_FIRST + 1;
			ent->client->weaponstate = WEAPON_READY;
         }
		else if (ent->client->ps.gunframe == FRAME_FIRE3_LAST + 1)
         {
         ent->client->ps.gunframe = FRAME_IDLE_FIRST + 1;
			ent->client->weaponstate = WEAPON_READY;
         }
	   }
}
