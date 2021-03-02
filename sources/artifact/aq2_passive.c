
// *************************************
// Artifact Quake 2 passive effects code
// *************************************

// "Artifact" and "Artifact Quake 2" are (c) Copyright 2000 Nick "Singe" Bousman and Glenn "Griphis" Saaiman

// Do not modify and redistribute any AQ2 materials without Singe's or Griphis's permission.
// Do not modify and redistribute this source without providing publically the modified source.
// Do not modify and redistribute this code without giving Singe approrpiate credit!
// Do not modify and redistribute pictures or models from this patch without giving Griphis appropriate credit!

// The code in this section handles anything in Artifact Quake 2 that deals with
// passive effects of artifacts.

// **************************************************************************

#include "g_local.h"

// Starts up the passive effect of a single artifact
void AQ_Passive_Start (edict_t *ent, int type)
{
   gclient_t *client = ent->client;
   AQ_Held_Artifact *artifact = &(ent->client->pers.AQ_Info[type]);
   ent->client->pers.AQ_Info[type].on = 1; // The effects of this artifact are now counted

   // If Good Health, up the max_health. :)
   if (type == AQ_HEALTH) { ent->max_health = 666; return; }

   // If Camo, initialize some data
   else if (type == AQ_CAMO) { artifact->aInt = 0; artifact->aFloat = 0; }

   // If Gravity Well, initialize some data
   else if (type == AQ_GWELL) artifact->aInt = 0;

   // If Shub Hat, spawn the shub hat
   else if (type == AQ_SHUB) AQ_Shub_Spawn (ent);

   // If Floating Spiky Death Cloud, spawn the thing
   else if (type == AQ_SPIKY) AQ_Spiky_Spawn (ent);

   // If Kamikaze, make that grenade appear
   else if (type == AQ_KAMIKAZE) AQ_Bomb_Spawn (ent);

   // If Death Spots, find all of your death spots in the level to see how many you can set.
   else if (type == AQ_DEATH) AQ_Death_Spot_Init (ent);

   // If Ice Traps, find all of your ice traps in the level to see how many you can set.
   else if (type == AQ_FREEZE) AQ_Ice_Trap_Init (ent);

   // If Holo-Artifact, find all of your holograms in the level to see how many you can set.
   else if (type == AQ_HOLOGRAM) AQ_Hologram_Init (ent);

   // If Flight, activate the little trail thingy underneath you.
   else if (type == AQ_FLIGHT) AQ_Flight_Trail (ent);

   // If Action Movie Arsenal, grant all weapons.
   else if (type == AQ_ACTION) AQ_Give_Weapons (ent);

   // If Vengeance, spawn the buddy-head.
   else if (type == AQ_VENG) AQ_Head_Spawn (ent);

   // If Cloaking, cloak.
   else if (type == AQ_CLOAK) AQ_Cloak (ent);

   // If Death Orb, make a Death Orb.
   else if (type == AQ_ORB) AQ_Death_Orb_Spawn (ent);

   // If Shield, clear the info about being invincible.
   else if (type == AQ_SHIELD) artifact->aInt = 0;

   // If Tank Helper, spawn Tank
   else if (type == AQ_HELPER) AQ_Tank_Spawn (ent);

   // If Radioactive, find all of your radiation points in the level to see how many you can set.
   else if (type == AQ_RADIO) AQ_Radio_Init (ent);

   // If Impact, spawn the sword indicator.
   else if (type == AQ_IMPACT) AQ_Sword_Spawn (ent);

   // If Electric, spawn the zappy thing.
   else if (type == AQ_ZAP) AQ_Zap_Spawn (ent);

   // If firewalk, clear the ent reference
   else if (type == AQ_FIREWALK) artifact->aEnt = NULL;
}

// Shuts down the passive effect of a single artifact
void AQ_Passive_Shutdown (edict_t *ent, int type)
{
   gclient_t *client = ent->client;
   AQ_Held_Artifact *artifact = &(ent->client->pers.AQ_Info[type]);
   ent->client->pers.AQ_Info[type].on = 0; // The effects of this artifact are no longer counted

   // If Good Health, return max_health to normal
   if (type == AQ_HEALTH)
   {
      ent->max_health = 100;
      if (ent->health > ent->max_health) ent->health = ent->max_health;
      return;
   } //TODO: Put in its own function?

   // If Camo, turn off the invisiblity
   else if (type == AQ_CAMO) AQ_Camo_Stop (ent);

   // If Gravity Well, turn off the gravity well (or give it a death sentence)
   else if (type == AQ_GWELL)
   {
      if (artifact->aInt)
      {
         if (ent->health < 1)
            artifact->aEnt->delay = level.time + 20;
         else
            AQ_GWell_Kill (artifact->aEnt);
      }
   } //TODO: Put in its own function?

   // If Shub, turn off the Shub hat
   else if (type == AQ_SHUB) AQ_Shub_Kill (artifact->aEnt);

   // If Spiky Death cloud, kill the Spiky Death cloud
   else if (type == AQ_SPIKY) AQ_Spiky_Kill (artifact->aEnt);

   // If Kamikaze, make that grenade dissapear
   else if (type == AQ_KAMIKAZE && !ent->deadflag) G_FreeEdict (artifact->aEnt);

   // If Flight, kill the trail entity underneath you and remove no-fire status if necessary.
   else if (type == AQ_FLIGHT)
   {
      if (ent->client->pers.AQ_State & AQ_STATE_NOSHOOT)
         ent->client->pers.AQ_State &= ~AQ_STATE_NOSHOOT;
      G_FreeEdict (artifact->aEnt);
   } //TODO: Put in its own function?

   // If Vengeance, kill the buddy-head.
   else if (type == AQ_VENG) G_FreeEdict (artifact->aEnt);

   // If Cloaking, uncloak and remove silencer shots.
   else if (type == AQ_CLOAK) { AQ_UnCloak (ent); ent->client->silencer_shots = 0; }

   // If Death Orb, kill the Orb or send it off bouncing.
   else if (type == AQ_ORB)
   {
      if (ent->deadflag) artifact->aEnt->air_finished = level.time + 20;
      AQ_Kill_Death_Orb (artifact->aEnt);
      artifact->aEnt = NULL;
   }

   // If Shield, deactivate.
   else if (type == AQ_SHIELD) AQ_Shield_Deactivate (ent);

   // If Tank Helper, remove Tank
   else if (type == AQ_HELPER) AQ_Remove_Tank (artifact->aEnt);

   // If Radioactive, drop a cannister of shit
   else if (type == AQ_RADIO && ent->deadflag) AQ_Spawn_Radio_Cannister (ent);

   // If Impact, make that sword dissapear
   else if (type == AQ_IMPACT) G_FreeEdict (artifact->aEnt);

   // If Electric, make that zappy thing dissapear
   else if (type == AQ_ZAP) G_FreeEdict (artifact->aEnt);

   // If weirdness, and you died, then spawn some things
   else if (type == AQ_WEIRD) AQ_Weird_Death (ent);

   // If firewalk, clear the ent reference
   else if (type == AQ_FIREWALK) artifact->aEnt = NULL;
}

// Handles the constant, ongoing passive effects
void AQ_Passive_Think (edict_t *ent)
{
   gclient_t *client = ent->client;
   int n;

   // Scan through ALL passive effects, or just the passive effect of equipped artifact.
   // Which of the two choices that will happen, is determined by the "all passive" cvar.

   if ((int)AQ_AllArtifactsEffect->value == 0) // Do just the equipped passive effect.
      AQ_Passive_Effect (ent, client->pers.AQ_Equipped);
   else
   if ((int)AQ_AllArtifactsEffect->value == 1) // Do ALL passive effects!
      for (n=0; n<MAX_ARTIFACT_TYPES; n++)
         if (client->pers.AQ_Info[n].held == 1)
            AQ_Passive_Effect (ent, n);
}

// Actually sends the execution of an ongoing passive effect to the right function.
void AQ_Passive_Effect (edict_t *ent, int type)
{
   if (type == NO_ARTIFACT) return;
   else if (type == AQ_REGEN) AQ_Regen (ent);
   else if (type == AQ_BERSERK) AQ_Berserk (ent);
   else if (type == AQ_CAMO) AQ_Camo (ent);
   else if (type == AQ_ACTION) AQ_Give_Ammo (ent);
   else if (type == AQ_REPULSE) AQ_Repel (ent);
   else if (type == AQ_CLOAK) AQ_Cloak_Think (ent);
   else if (type == AQ_SHIELD) AQ_Shield_Passive (ent);
   else if (type == AQ_HELPER) AQ_Tank_Update (ent);
   else if (type == AQ_IMPACT) AQ_Impact_Think (ent);
   else if (type == AQ_JUMPY) AQ_Jumpy_Think (ent);
   else if (type == AQ_ZAP) AQ_Electric_Think (ent);
   else if (type == AQ_FIREWALK) AQ_FireWalk_Think (ent);
   else if (type == AQ_WEIRD) AQ_Weird_Think (ent);
}

// Effects ******************************************

// Makes a tiny red flash near the health box picked up when player has good health
void AQ_Health_Flash (edict_t *ent)
{
   gi.WriteByte (svc_muzzleflash);
   gi.WriteShort (ent-g_edicts);
   gi.WriteByte (MZ_NUKE1);
   gi.multicast (ent->s.origin, MULTICAST_PVS);
}

// Recharges health and armour for regen
AQ_Regen (edict_t *ent)
{
   AQ_Held_Artifact *artifact = &(ent->client->pers.AQ_Info[AQ_REGEN]);
   int armor_index;

   if (artifact->pTime > level.time) return;

   if (ent->health < ent->max_health)
   {
      ent->health += 5;
      if (ent->health > ent->max_health) ent->health = ent->max_health;

      gi.sound (ent, CHAN_AUTO, gi.soundindex("artifact/heart.wav"), 0.5, ATTN_NORM, 0);
   }
   armor_index = ArmorIndex (ent);
   if (armor_index && ent->client->pers.inventory[armor_index] < 200) // That 200 number is COMPLETELY arbitrary
      ent->client->pers.inventory[armor_index] += 5;

   artifact->pTime = level.time + 1;
}

// Haste sound
AQ_Haste (edict_t *ent)
{
   AQ_Held_Artifact *artifact = &(ent->client->pers.AQ_Info[AQ_HASTE]);

   if (artifact->pTime > level.time) return;

   if (ent->client->weaponstate == WEAPON_FIRING)
   {
      gi.sound (ent, CHAN_AUTO, gi.soundindex ("artifact/godz.wav"), 1, ATTN_NORM, 0);   
      artifact->pTime = level.time + 2;
   }
}

// Resist sound
AQ_Resist (edict_t *ent)
{
   AQ_Held_Artifact *artifact = &(ent->client->pers.AQ_Info[AQ_RESIST]);

   if (artifact->pTime > level.time) return;

   gi.WriteByte (svc_muzzleflash);
   gi.WriteShort (ent-g_edicts);
   gi.WriteByte (MZ_NUKE2);
   gi.multicast (ent->s.origin, MULTICAST_PVS);

   gi.sound (ent, CHAN_AUTO, gi.soundindex ("items/protect3.wav"), 1, ATTN_NORM, 0);   
   artifact->pTime = level.time + 2.1;
}

// Strength sound
void AQ_Strength (edict_t *ent)
{
   AQ_Held_Artifact *artifact = &(ent->client->pers.AQ_Info[AQ_STRENGTH]);

   if (artifact->pTime > level.time) return;

   if (ent->client->weaponstate == WEAPON_FIRING)
   {
      gi.sound (ent, CHAN_AUTO, gi.soundindex ("items/damage3.wav"), 1, ATTN_NORM, 0);   
      artifact->pTime = level.time + 1.1;
   }
}

// Berserk effect
void AQ_Berserk (edict_t *ent)
{
   AQ_Held_Artifact *artifact = &(ent->client->pers.AQ_Info[AQ_BERSERK]);
   vec3_t temp;

   if (artifact->pTime > level.time) return;

   VectorSet(temp, 4-random()*8, 4-random()*8, 8-random()*16);
   VectorAdd(ent->s.origin, temp, temp);

   gi.WriteByte (svc_temp_entity);
   gi.WriteByte (TE_BLOOD);
   gi.WritePosition (temp);
   gi.WriteDir (vec3_origin);
   gi.multicast (ent->s.origin, MULTICAST_PVS);

   VectorSet(ent->client->kick_angles, random()*5 - 2.5, random()*5 - 2.5, random()*5 - 2.5);

   artifact->pTime = level.time + 0.3;
}

// Deactivation of camo
AQ_Camo_Stop (edict_t *ent)
{
   AQ_Held_Artifact *artifact = &(ent->client->pers.AQ_Info[AQ_CAMO]);

   if (artifact->aInt)
   {
      artifact->aInt = 0;
      ent->flags &= ~FL_NOTARGET;
      ent->svflags &= ~SVF_NOCLIENT;
   }
}

// Thinking for camoflague artifact. Invisible, not invisible, etc...
AQ_Camo (edict_t *ent)
{
   AQ_Held_Artifact *artifact = &(ent->client->pers.AQ_Info[AQ_CAMO]);

   if (artifact->pTime > level.time) return;

   if (artifact->aInt == 0 && !VectorLength(ent->velocity))
   { // Make invisible
      artifact->aInt = 1;
      AQ_HUD_Num (ent);
      ent->flags |= FL_NOTARGET;
      ent->svflags |= SVF_NOCLIENT;
   }
   else
   if (artifact->aInt == 1 && (VectorLength(ent->velocity) || ent->client->weaponstate == WEAPON_FIRING || artifact->aFloat))
   { // Make visible
      artifact->aInt = 0;
      AQ_HUD_Num (ent);
      ent->flags &= ~FL_NOTARGET;
      ent->svflags &= ~SVF_NOCLIENT;
      artifact->pTime = level.time + 0.3;
   }

   // Make kinda dark if invisible
   if (artifact->aInt)
   {
      VectorSet(ent->client->damage_blend, 0, 0, 0);
      ent->client->damage_alpha = 0.2;
   }
}

// Shub Hat's thinking func
void AQ_Shub_Think (edict_t *ent)
{
   VectorCopy (ent->owner->s.origin, ent->s.origin);
   ent->nextthink = level.time + FRAMETIME;
}

// Spawn a Shub Hat
void AQ_Shub_Spawn (edict_t *ent)
{
   edict_t *shub;

   shub = G_Spawn();
   VectorCopy (ent->s.origin, shub->s.origin);
   VectorCopy (ent->s.origin, shub->s.old_origin);
   VectorClear (shub->avelocity);
   VectorClear (shub->velocity);
   VectorClear (shub->mins);
   VectorClear (shub->maxs);
   shub->movetype = MOVETYPE_FLY;
   shub->solid = SOLID_NOT;
   shub->owner = ent;
   shub->touch = NULL;
   shub->think = AQ_Shub_Think;
   shub->nextthink = level.time;
   shub->classname = "aq_shub_hat";
   shub->s.modelindex = gi.modelindex ("models/artifact/shub/tris.md2");
   if (random()*2 < 1)
      shub->avelocity[1] = 90;
   else
      shub->avelocity[1] = -90;
   gi.linkentity (shub);

   ent->client->pers.AQ_Info[AQ_SHUB].aEnt = shub;
}

// Remove a Shub Hat
void AQ_Shub_Kill (edict_t *ent)
{
   G_FreeEdict (ent);
}

// Blow shit up!
void AQ_Spiker_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
   vec3_t dir, norm;

   if (other == world) 
   {
        if (VectorLength(ent->velocity))
        {
           VectorClear(ent->avelocity);
           VectorClear(ent->velocity);
           VectorCopy(plane->normal, ent->s.angles);
           ent->s.angles[1] = random()*360;
        }
        return;
   }
   if (other == ent->owner) return;
   if (other->takedamage == DAMAGE_NO) return;

   VectorSet (dir, random()*10-5, random()*10-5, 0);
   VectorAdd (dir, ent->s.origin, dir);
   VectorSet (norm, 0, 0, -1);

   T_Damage (other, ent, ent->owner, dir, ent->s.origin, norm, 20, 20, 0, MOD_AQ_DEATH_CLOUD);

   BecomeExplosion1 (ent);
}

// Spawn func for the things the spikey death cloud spits out
void AQ_Spawn_Baby_Spiker (edict_t *ent)
{
   edict_t *baby;

   baby = G_Spawn();
   VectorCopy (ent->s.origin, baby->s.old_origin);
   VectorCopy (ent->s.origin, baby->s.origin);
   VectorSet (baby->velocity, random()*500-250, random()*500-250, random()*100-80);
   VectorSet (baby->avelocity, random()*720-360, random()*720-360, random()*720-360);
   VectorSet (baby->mins, -5, -5, -4);
   VectorSet (baby->maxs, 5, 5, 4);
   baby->movetype = MOVETYPE_TOSS;
   baby->solid = SOLID_TRIGGER;
   baby->owner = ent->owner;
   baby->touch = AQ_Spiker_Touch;
   baby->think = G_FreeEdict;
   baby->nextthink = level.time + 10;
   baby->classname = "aq_death_baby";
   baby->s.modelindex = gi.modelindex ("models/artifact/spike/tris.md2");
   gi.linkentity (baby);
}

// Floating Death Thing's thinking func
void AQ_Spiky_Think (edict_t *ent)
{
   VectorCopy (ent->owner->s.origin, ent->s.origin);
   ent->s.origin[2] += AQ_HAT;

   // Occasionally fire a semi-grenade thing.
   if (random()*10 < 1)
      AQ_Spawn_Baby_Spiker (ent);

   if (ent->s.frame++ == 2) ent->s.frame = 0;

   ent->nextthink = level.time + FRAMETIME;
}

// Spawn function for Floating Spiky Death Cloud
void AQ_Spiky_Spawn (edict_t *ent)
{
   edict_t *thing;

   thing = G_Spawn();
   VectorSet (thing->avelocity, random()*400-200, random()*400-200, random()*400-200);
   VectorCopy (ent->s.origin, thing->s.old_origin);
   VectorCopy (ent->s.origin, thing->s.origin);
   VectorClear (thing->velocity);
   VectorClear (thing->mins);
   VectorClear (thing->maxs);
   thing->movetype = MOVETYPE_FLY;
   thing->solid = SOLID_NOT;
   thing->owner = ent;
   thing->touch = NULL;
   thing->think = AQ_Spiky_Think;
   thing->nextthink = level.time;
   thing->classname = "aq_death_cloud";
   thing->s.modelindex = gi.modelindex ("models/artifact/spiky/tris.md2");
   gi.linkentity (thing);

   ent->client->pers.AQ_Info[AQ_SPIKY].aEnt = thing;
}

// Remove a Floating Death Thing
void AQ_Spiky_Kill (edict_t *ent)
{
   G_FreeEdict (ent);
}

// Set the first point for Dual Recall
void Recall2Init (edict_t *ent)
{
   AQ_Held_Artifact *artifact = &(ent->client->pers.AQ_Info[AQ_RECALL2]);
   VectorCopy (ent->s.origin, artifact->aVector);
}

// Kamikaze grenade think. Follow the player
void AQ_Bomb_Think (edict_t *ent)
{
   VectorCopy (ent->owner->s.origin, ent->s.origin);
   ent->s.origin[2] += AQ_HAT;
   ent->nextthink = level.time + FRAMETIME;
}

// Spawn the bomb that follows the player around when they've got kamikaze
void AQ_Bomb_Spawn (edict_t *ent)
{
   AQ_Held_Artifact *artifact = &(ent->client->pers.AQ_Info[AQ_KAMIKAZE]);
   edict_t *bomb;

   artifact->aInt = 0; // This is so you can't keep "activating" explosions. Only one.

   bomb = G_Spawn();
   VectorSet (bomb->avelocity, random()*400-200, random()*400-200, random()*400-200);
   VectorCopy (ent->s.origin, bomb->s.old_origin);
   VectorCopy (ent->s.origin, bomb->s.origin);
   bomb->s.old_origin[2] += AQ_HAT;
   bomb->s.origin[2] += AQ_HAT;
   VectorClear (bomb->velocity);
   VectorClear (bomb->mins);
   VectorClear (bomb->maxs);
   bomb->movetype = MOVETYPE_FLY;
   bomb->solid = SOLID_NOT;
   bomb->owner = ent;

   bomb->touch = NULL;
   bomb->think = AQ_Bomb_Think;
   bomb->nextthink = level.time;
   bomb->classname = "aq_bomb";
   bomb->health = 0;
   bomb->s.effects = EF_GRENADE;
   bomb->s.modelindex = gi.modelindex ("models/objects/grenade/tris.md2");
   gi.linkentity (bomb);

   artifact->aEnt = bomb;
}

// Find all death spots belonging to ent and give ent the appropriate number of death spots allowed to set.
void AQ_Death_Spot_Init (edict_t *ent)
{
   AQ_Held_Artifact *artifact = &(ent->client->pers.AQ_Info[AQ_DEATH]);
   edict_t *spot = NULL;
   int num = 5;

   while ((spot = G_Find(spot, FOFS(classname), "aq_death_spot")) != NULL)
      if (spot->owner == ent) num--;

   artifact->aInt = num;
}

// Find all ice traps belonging to ent and give ent the appropriate number of ice traps allowed to set.
void AQ_Ice_Trap_Init (edict_t *ent)
{
   AQ_Held_Artifact *artifact = &(ent->client->pers.AQ_Info[AQ_FREEZE]);
   edict_t *trap = NULL;
   int num = 6;

   while ((trap = G_Find(trap, FOFS(classname), "aq_ice_trap")) != NULL)
      if (trap->owner == ent) num--;

   artifact->aInt = num;
}

// Find all ice traps belonging to ent and give ent the appropriate number of ice traps allowed to set.
void AQ_Hologram_Init (edict_t *ent)
{
   AQ_Held_Artifact *artifact = &(ent->client->pers.AQ_Info[AQ_HOLOGRAM]);
   edict_t *holo = NULL;
   int num = 5;

   while ((holo = G_Find(holo, FOFS(classname), "aq_hologram")) != NULL)
      if (holo->owner == ent) num--;

   artifact->aInt = num;
}

// Find all radiation points belonging to ent and give ent the appropriate number of ice traps allowed to set.
void AQ_Radio_Init (edict_t *ent)
{
   AQ_Held_Artifact *artifact = &(ent->client->pers.AQ_Info[AQ_RADIO]);
   edict_t *radio = NULL;
   int num = 3;

   while ((radio = G_Find(radio, FOFS(classname), "aq_radio_point")) != NULL)
      if (radio->owner == ent) num--;

   artifact->aInt = num;
}

// Flight trail think. Follow the player
void AQ_Trail_Think (edict_t *ent)
{
   VectorCopy (ent->owner->s.origin, ent->s.origin);
   ent->s.origin[2] -= 20;
   ent->nextthink = level.time + FRAMETIME;
}

// Creates an entity which follows underneath the player and leaves little white sparks while flying.
void AQ_Flight_Trail (edict_t *ent)
{
   AQ_Held_Artifact *artifact = &(ent->client->pers.AQ_Info[AQ_FLIGHT]);
   edict_t *trail;

   // Um, this is just here for flight speed. It has nothing to do with the flight trail.
   if (artifact->aFloat != 80 && artifact->aFloat != 350 && artifact->aFloat != 1000)
   {
      artifact->aFloat = 350;
      artifact->aInt = 0;
   }

   trail = G_Spawn();

   VectorCopy (ent->s.origin, trail->s.old_origin);
   VectorCopy (ent->s.origin, trail->s.origin);
   VectorClear (trail->mins);
   VectorClear (trail->maxs);
   trail->movetype = MOVETYPE_FLY;
   trail->solid = SOLID_NOT;
   trail->owner = ent;
   trail->touch = NULL;
   trail->think = AQ_Trail_Think;
   trail->nextthink = level.time;
   trail->classname = "aq_flight_trail";
   trail->s.effects = EF_GRENADE;
   trail->s.modelindex = gi.modelindex ("models/artifact/a_null/tris.md2");
   trail->s.sound = gi.soundindex ("floater/fltsrch1.wav");
   gi.linkentity (trail);

   artifact->aEnt = trail;   
}

// Give all weapons
void AQ_Give_Weapons (edict_t *ent)

{
   int* machinegun = &ent->client->pers.inventory[ITEM_INDEX(FindItem ("Machinegun"))];
   int* chaingun = &ent->client->pers.inventory[ITEM_INDEX(FindItem ("Chaingun"))];
   int* shotgun = &ent->client->pers.inventory[ITEM_INDEX(FindItem ("Shotgun"))];
   int* doubleshotgun = &ent->client->pers.inventory[ITEM_INDEX(FindItem ("Super Shotgun"))];
   int* rocketlauncher = &ent->client->pers.inventory[ITEM_INDEX(FindItem ("Rocket Launcher"))];
   int* grenadelauncher = &ent->client->pers.inventory[ITEM_INDEX(FindItem ("Grenade Launcher"))];
   int* hyperblaster = &ent->client->pers.inventory[ITEM_INDEX(FindItem ("HyperBlaster"))];
   int* bfg10k = &ent->client->pers.inventory[ITEM_INDEX(FindItem ("BFG10K"))];
   int* railgun = &ent->client->pers.inventory[ITEM_INDEX(FindItem ("Railgun"))];
   int gave = 0;

   if (!*machinegun) { *machinegun += 1; gave++; }
   if (!*chaingun) { *chaingun += 1; gave++; }
   if (!*shotgun) { *shotgun += 1; gave++; }
   if (!*doubleshotgun) { *doubleshotgun += 1; gave++; }
   if (!*rocketlauncher) { *rocketlauncher += 1; gave++; }
   if (!*grenadelauncher) { *grenadelauncher += 1; gave++; }
   if (!*hyperblaster) { *hyperblaster += 1; gave++; }
   if (!*bfg10k) { *bfg10k += 1; gave++; }
   if (!*railgun) { *railgun += 1; gave++; }
	
   if (gave)
      gi.sound (ent, CHAN_AUTO, gi.soundindex ("misc/w_pkup.wav"), 1, ATTN_NORM, 0);
}

// Recharge ammo :)
void AQ_Give_Ammo (edict_t *ent)
{
   int* bullets = &ent->client->pers.inventory[ITEM_INDEX(FindItem ("bullets"))];
   int* shells = &ent->client->pers.inventory[ITEM_INDEX(FindItem ("shells"))];
   int* rockets = &ent->client->pers.inventory[ITEM_INDEX(FindItem ("rockets"))];
   int* grenades = &ent->client->pers.inventory[ITEM_INDEX(FindItem ("grenades"))];
   int* cells = &ent->client->pers.inventory[ITEM_INDEX(FindItem ("cells"))];
   int* slugs = &ent->client->pers.inventory[ITEM_INDEX(FindItem ("slugs"))];

   if (ent->client->pers.AQ_Info[AQ_ACTION].pTime > level.time) return;

   // Increase all the ammo and check boundaries.
   if (*bullets < ent->client->pers.max_bullets) *bullets += 4; else
   if (*bullets > ent->client->pers.max_bullets) *bullets = ent->client->pers.max_bullets;
   if (*shells < ent->client->pers.max_shells) *shells += 2;  else
   if (*shells > ent->client->pers.max_shells) *shells = ent->client->pers.max_shells;
   if (*rockets < ent->client->pers.max_rockets) *rockets += 1;
   if (*grenades < ent->client->pers.max_grenades) *grenades += 1;
   if (*cells < ent->client->pers.max_cells) *cells += 4;  else
   if (*cells > ent->client->pers.max_cells) *cells = ent->client->pers.max_cells;
   if (*slugs < ent->client->pers.max_slugs) *slugs += 1;

   ent->client->pers.AQ_Info[AQ_ACTION].pTime = level.time + 2;
}

// The player's repulsive. Eww! Icky!
void AQ_Repel (edict_t *ent)
{
   edict_t *victim = NULL;
   float    distMax = 400;
   float    dist;
   vec3_t   dOrg, force;

   if (ent->client->pers.AQ_Info[AQ_REPULSE].pTime > level.time) return;

   while ((victim = findradius(victim, ent->s.origin, distMax)) != NULL)
   {
	if (victim->movetype == MOVETYPE_NONE ||
          victim->solid == SOLID_NOT ||
          victim->solid == SOLID_BSP ||
          victim->s.renderfx & RF_GLOW ||
          OnSameTeam(victim, ent))
         continue;

      VectorSubtract (ent->s.origin, victim->s.origin, dOrg);
      dist = VectorLength (dOrg);
      VectorNormalize (dOrg);
      VectorScale (dOrg, 0.84, force);
      VectorScale (force, (distMax-dist), force);
      VectorSubtract (victim->velocity, force, victim->velocity);
   }

   ent->client->pers.AQ_Info[AQ_REPULSE].pTime = level.time + FRAMETIME;
}

// Floating gibbed head's think routine.
void AQ_Heady_Think (edict_t *ent)
{
   vec3_t difference;
   float dist;

    // Find exact difference between head and owner
   VectorSubtract (ent->owner->s.origin, ent->s.origin, difference);
   dist = VectorLength (difference);
    // Turn it into a direction
   VectorNormalize (difference);
   VectorScale (difference, dist, difference);
    // Add to the speed
   VectorAdd (ent->velocity, difference, ent->velocity);
    // If you're too far away, make head exactly as far away as it needs to be
   if (dist > 100)
   {
      dist = 100;
      VectorNormalize (difference);
      VectorScale (difference, dist, difference);      
      VectorSubtract (ent->owner->s.origin, difference, ent->s.origin);
   }
    // If the speed is too big, decrease it.
   if (VectorLength (ent->velocity) > 800)
   {
      VectorNormalize (ent->velocity);
      VectorScale (ent->velocity, 800, ent->velocity);
   }
    // Don't stick to the ground. :)
   if (ent->groundentity)
   {
      VectorSet (ent->avelocity, random()*720-360, random()*720-360, random()*720-360);
      ent->groundentity = NULL;
   }
    // Make it flash red if it does vengeance damage  (Do we need the 2nd condition?)
   if (ent->owner && ent->owner->client->pers.AQ_Info[AQ_VENG].on && ent->owner->client->pers.AQ_Info[AQ_VENG].aTime > level.time)
   {
      if (ent->owner->client->pers.AQ_Info[AQ_VENG].aTime > level.time + 0.2)
      {
         ent->s.effects |= EF_COLOR_SHELL;
         ent->s.renderfx |= RF_SHELL_RED;
      } else {
         ent->s.effects &= ~EF_COLOR_SHELL;
         ent->s.renderfx &= ~RF_SHELL_RED;
      }
   }

   ent->nextthink = level.time + FRAMETIME;
}

// Make a floating head to follow the player around
void AQ_Head_Spawn (edict_t *ent)
{
   AQ_Held_Artifact *artifact = &(ent->client->pers.AQ_Info[AQ_VENG]);

   edict_t *heady;

   heady = G_Spawn();
   VectorCopy (ent->s.origin, heady->s.old_origin);
   VectorCopy (ent->s.origin, heady->s.origin);
   VectorSet (heady->avelocity, random()*720-360, random()*720-360, random()*720-360);
   VectorClear (heady->mins);
   VectorClear (heady->maxs);
   heady->movetype = MOVETYPE_FLY;
   heady->solid = SOLID_NOT;
   heady->owner = ent;
   heady->touch = NULL;
   heady->think = AQ_Heady_Think;
   heady->nextthink = level.time;
   heady->classname = "aq_vengeance_head";
   heady->s.effects |= EF_GIB;
   heady->s.modelindex = gi.modelindex ("models/objects/gibs/head2/tris.md2");
   gi.linkentity (heady);

   artifact->aEnt = heady;
}

// Do damage to people who did damage to you. :)
void AQ_VengBack (edict_t *ent, edict_t *attacker, int damage, int knockback, int origMOD)
{
   vec3_t diff;

   // Don't damage the player for damaging himself.
   if (attacker == ent) return;

   if (attacker->health < 1) return;
   if (attacker->takedamage == DAMAGE_NO) return;

   // Two Vengeances cancel each other out.
   if (attacker->client && attacker->client->pers.AQ_Info[AQ_VENG].on) return;

   VectorSubtract(attacker->s.origin, ent->s.origin, diff);

   T_Damage(attacker, ent, ent, diff, attacker->s.origin, vec3_origin, damage, knockback, 0, MOD_AQ_VENG);
   meansOfDeath = origMOD; // To keep the client obits accurate

   // This lets the buddy head know when to flash
   ent->client->pers.AQ_Info[AQ_VENG].aTime = level.time + 0.5;
}

// Cloak the guy with Cloaking
void AQ_Cloak (edict_t *ent)
{
   AQ_Held_Artifact *artifact = &(ent->client->pers.AQ_Info[AQ_CLOAK]);

   if (artifact->aInt) return;

   // Keeps track of invisibility
   artifact->aInt = 1;
   AQ_HUD_Num (ent);

   // Become eyes
   ent->s.modelindex = gi.modelindex ("models/artifact/eyes/tris.md2");
   ent->s.modelindex2 = 0;

   // Just for monsters :)
   ent->flags |= FL_NOTARGET;

   // Play the invisible sound
   gi.sound (ent, CHAN_AUTO, gi.soundindex ("artifact/inv1.wav"), 0.5, ATTN_NORM, 0);
}

// Uncloak the guy with Cloaking
void AQ_UnCloak (edict_t *ent)
{
   AQ_Held_Artifact *artifact = &(ent->client->pers.AQ_Info[AQ_CLOAK]);

   // Time until you cloak up again
   artifact->pTime = level.time + 3;

   if (!artifact->aInt) return;

   // No longer cloaked
   artifact->aInt = 0;
   AQ_HUD_Num (ent);

   // Use the regular models
   ent->s.modelindex = 255;
   ent->s.modelindex2 = 255;

   // Just for monsters :)
   ent->flags &= ~FL_NOTARGET;

   // Play the, um, un-invisible sound
   gi.sound (ent, CHAN_AUTO, gi.soundindex ("misc/spawn1.wav"), 0.5, ATTN_NORM, 0);
}

// Handles the re-cloaking
void AQ_Cloak_Think (edict_t *ent)
{
   AQ_Held_Artifact *artifact = &(ent->client->pers.AQ_Info[AQ_CLOAK]);

   if (artifact->aInt && ent->s.frame != 0) ent->s.frame = 0;

   if (!ent->client->silencer_shots) ent->client->silencer_shots = 50; // Keep upping player's silencer shots
   if (artifact->pTime > level.time) return; // If not time to recloak, return.
   if (artifact->aInt) // If already cloaked, make dark, return.
   {
      VectorSet(ent->client->damage_blend, 0, 0, 0);
      ent->client->damage_alpha = 0.3;
      return; 
   }

   AQ_Cloak(ent);
}

// Thinking for the free-range Death Orb
void AQ_Free_Orb_Think (edict_t *ent)
{
   // Keep shell on self.   
   VectorCopy (ent->s.origin, ent->enemy->s.origin);
   VectorCopy (ent->velocity, ent->enemy->velocity);

   // If stuck on the ground, redo all the angles and such, and make bounce.
   if (ent->groundentity)
   {
      ent->groundentity = NULL;
      VectorSet (ent->velocity, 400-random()*800, 400-random()*800, 100+random()*500); 
      VectorSet (ent->avelocity, random()*720-360, random()*720-360, random()*720-360);
      VectorSet (ent->enemy->s.angles, random()*360, random()*360, random()*360);
      VectorClear (ent->enemy->avelocity);
      if (random()*2 < 1) ent->enemy->avelocity[1] = 1000; else ent->enemy->avelocity[1] = -1000;
   }

   // Hurt people
   T_RadiusDamage(ent, ent->owner, 40, ent->owner, 80, MOD_AQ_DEATH_ORB2);

   ent->nextthink = level.time + FRAMETIME;

   // Finally make dissapear, if time
   if (ent->air_finished <= level.time)
   {
      gi.WriteByte (svc_temp_entity);
      gi.WriteByte (TE_BFG_BIGEXPLOSION);
      gi.WritePosition (ent->s.origin);
      gi.multicast (ent->s.origin, MULTICAST_PVS);

      G_FreeEdict (ent->enemy); // Kill the shell
      G_FreeEdict (ent); // Kill the orb
   }
}

// Kill the Death Orb
void AQ_Kill_Death_Orb (edict_t *ent)
{
   G_FreeEdict (ent->target_ent); // Kill the axis.

   if (ent->air_finished > level.time) // If there's a "delay" before killing the orb, make it bounce around.
   {
      ent->think = AQ_Free_Orb_Think;
      ent->nextthink = level.time;
      ent->movetype = MOVETYPE_BOUNCE;
      ent->touch = NULL;
      ent->s.effects |= EF_COLOR_SHELL;
      ent->s.renderfx |= RF_SHELL_GREEN;
      return;
   } else
   {
      G_FreeEdict (ent->enemy); // Kill the shell
      G_FreeEdict (ent); // Kill the orb
   }
}

// Thinking for the rotating Death Orb
void AQ_Orb_Think (edict_t *ent)
{
   vec3_t  spot;
   trace_t trace;
   float dist;

   // Green flashness.
   if (ent->delay <= level.time)
   {
      if (ent->health == 0) // Flash
      {
         ent->health = 1;
         ent->s.effects |= EF_COLOR_SHELL;
         ent->s.renderfx |= RF_SHELL_GREEN;
         ent->delay = level.time + FRAMETIME;
         gi.sound(ent, CHAN_AUTO, gi.soundindex ("artifact/blipdrip.wav"), 0.1, ATTN_NORM, 0);
      }
      else                  // Un-flash
      {
         ent->health = 0;
         ent->s.effects &= ~EF_COLOR_SHELL;
         ent->s.renderfx &= ~RF_SHELL_GREEN;
         ent->delay = level.time + FRAMETIME + random()*0.5;
      }
   }

   // Put axis on top of the player
   VectorCopy (ent->owner->s.origin, ent->target_ent->s.origin);
   ent->target_ent->s.origin[2] += 10;

   if (ent->owner->client->pers.AQ_Info[AQ_ORB].aInt == 1) // If in the "new" death orb mode...
   {
       // Find exact difference between orb and owner
      VectorSubtract (ent->owner->s.origin, ent->s.origin, spot);
      dist = VectorLength (spot);
       // Turn it into a direction
      VectorNormalize (spot);
      VectorScale (spot, dist, spot);
       // Add to the speed
      VectorAdd (ent->velocity, spot, ent->velocity);
       // If you're too far away, make orb exactly as far away as it needs to be
      if (dist > 300)
      {
         dist = 300;
         VectorNormalize (spot);
         VectorScale (spot, dist, spot);      
         VectorSubtract (ent->owner->s.origin, spot, ent->s.origin);
      }
       // If the speed is too big, decrease it.
      if (VectorLength (ent->velocity) > 800)
      {
         VectorNormalize (ent->velocity);
         VectorScale (ent->velocity, 800, ent->velocity);
      }
       // Don't stick to the ground. :)
      if (ent->groundentity)
      {
         VectorSet (ent->avelocity, random()*720-360, random()*720-360, random()*720-360);
         ent->groundentity = NULL;
         VectorSet (ent->avelocity, random()*720-360, random()*720-360, random()*720-360);
         VectorSet (ent->enemy->s.angles, random()*360, random()*360, random()*360);

         VectorClear (ent->enemy->avelocity);
         if (random()*2 < 1) ent->enemy->avelocity[1] = 1000; else ent->enemy->avelocity[1] = -1000;
      }
       // Keep the shell in place
      VectorCopy (ent->s.origin, ent->enemy->s.origin);
      VectorCopy (ent->velocity, ent->enemy->velocity);
   } else
   { // Otherwise, just make it rotate around the player
       // Put self (Death Orb) in proper place
      AngleVectors(ent->target_ent->s.angles, spot, NULL, NULL);
      VectorScale(spot, 90, spot);
      VectorAdd(spot, ent->target_ent->s.origin, spot);
      trace = gi.trace (ent->target_ent->s.origin, NULL, NULL, spot, ent, MASK_SOLID);
      VectorCopy(trace.endpos, spot);
      VectorCopy(spot, ent->s.origin);
       // Keep the shell in place
      VectorCopy (spot, ent->enemy->s.origin);
   }

   // Hurt people
   T_RadiusDamage(ent, ent->owner, 40, ent->owner, 80, MOD_AQ_DEATH_ORB);

   ent->nextthink = level.time + FRAMETIME;
}

// Spawns an Orb of Death
void AQ_Death_Orb_Spawn (edict_t *ent)
{
   AQ_Held_Artifact *artifact = &(ent->client->pers.AQ_Info[AQ_ORB]);
   edict_t *orb, *axis, *shell;

   orb = G_Spawn(); // The orb. Handles damage and effects and placement
   axis = G_Spawn(); // Invisible axis on player about which the orb rotates around. Does nothing.
   shell = G_Spawn(); // Transparent shell around orb. Does nothing.

   VectorCopy (ent->s.origin, orb->s.old_origin);
   VectorCopy (ent->s.origin, orb->s.origin);
   VectorSet (orb->avelocity, random()*720-360, random()*720-360, random()*720-360);
   VectorClear (orb->mins);
   VectorClear (orb->maxs);
   orb->movetype = MOVETYPE_FLY;
   orb->solid = SOLID_NOT;
   orb->owner = ent;
   orb->touch = NULL;
   orb->think = AQ_Orb_Think;
   orb->nextthink = level.time;
   orb->classname = "aq_death_orb";
   orb->health = 0;
   orb->s.effects = 0;
   orb->s.renderfx = 0;
   orb->s.modelindex = gi.modelindex ("models/artifact/orb/tris.md2");
   gi.linkentity (orb);

   VectorCopy (ent->s.origin, axis->s.old_origin);
   VectorCopy (ent->s.origin, axis->s.origin);
   VectorClear (axis->s.angles);
   VectorClear (axis->avelocity);
   if (random()*2 < 1)
      axis->avelocity[1] = 50;
   else
      axis->avelocity[1] = -50;
   axis->s.angles[1] = random()*360;
   VectorClear (axis->mins);
   VectorClear (axis->maxs);
   axis->movetype = MOVETYPE_FLY;
   axis->solid = SOLID_NOT;
   axis->owner = ent;
   axis->touch = NULL;
   axis->classname = "aq_death_orb_axis";
   gi.linkentity (axis);

   VectorCopy (ent->s.origin, shell->s.old_origin);
   VectorCopy (ent->s.origin, shell->s.origin);
   VectorSet (shell->s.angles,random()*360, random()*360, random()*360);
   VectorClear (shell->avelocity);
   if (random()*2 < 1)
      shell->avelocity[1] = 1000;
   else
      shell->avelocity[1] = -1000;
   VectorClear (shell->mins);
   VectorClear (shell->maxs);
   shell->movetype = MOVETYPE_FLY;
   shell->solid = SOLID_NOT;
   shell->owner = ent;
   shell->touch = NULL;
   shell->think = NULL;
   shell->classname = "aq_death_orb_shell";
   shell->s.modelindex = gi.modelindex ("models/artifact/orb2/tris.md2");
   shell->s.effects |= EF_SPHERETRANS | EF_COLOR_SHELL;
   shell->s.renderfx |= RF_SHELL_GREEN;
   gi.linkentity (shell);

   orb->target_ent = axis;
   orb->enemy = shell;
   axis->target_ent = orb;
   shell->target_ent = orb;

   artifact->aEnt = orb;
}

// Gives player health when they did damage, and sets up for a red flash.
void AQ_Vampire (edict_t *ent, edict_t *victim, int amount)
{
   AQ_Held_Artifact *artifact = &(ent->client->pers.AQ_Info[AQ_BUFFY]);

   if (ent->health >= ent->max_health) return;
   if (ent == victim) return; // Don't give health for self-damage
   if (victim->owner && victim->owner == ent) return; // Can't damage your own guys to get health. (Tank Helper)
   if (victim->svflags & SVF_MONSTER) amount = amount / 5.0; // If it's a monster, you only get 1/5th the amount.
   // Add the health
   ent->health += amount;
   if (ent->health > ent->max_health) ent->health = ent->max_health;
   // Sound!
   if (artifact->aTime < level.time)
      gi.sound(ent, CHAN_AUTO, gi.soundindex ("artifact/wpain.wav"), 1, ATTN_NORM, 0);
   // Triggers p_view to make the red shell flash
   artifact->aTime = level.time + 2*FRAMETIME;
}

// Passive function called with Shielding.
void AQ_Shield_Passive (edict_t *ent)
{
   AQ_Held_Artifact *artifact = &(ent->client->pers.AQ_Info[AQ_SHIELD]);

   if (artifact->aInt) // If invincible
   {
      if (artifact->pTime <= level.time) // If time to deactivate
         AQ_Shield_Deactivate (ent);
   }
//TODO: Merge this with the HUD number
}

// Environmental protection gives players health when in slime, water, or lava.
void AQ_EPA_Liquid(edict_t *ent)
{
   // check for "drowning"
   if (ent->waterlevel == 3)
   {
      // if out of air, start getting health
      if (ent->air_finished < level.time && ent->client->next_drown_time < level.time && ent->health < ent->max_health)
      { // !drown!
         ent->client->next_drown_time = level.time + 1;

         // get more health the longer underwater
         ent->dmg += 2;

         if (ent->dmg > 15)
            ent->dmg = 15;

         // play a health sound
         gi.sound (ent, CHAN_VOICE, gi.soundindex("items/l_health.wav"), 1, ATTN_NORM, 0);

         ent->health += ent->dmg;
         if (ent->health > ent->max_health) ent->health = ent->max_health;
      }
   }
   else
   {
      ent->air_finished = level.time + 12;
      ent->dmg = 2;
   }

   // check for sizzle "damage"
   if (ent->waterlevel && (ent->watertype&(CONTENTS_LAVA|CONTENTS_SLIME)))
   {
      if (ent->watertype & CONTENTS_LAVA)
      {
         if (ent->health < ent->max_health
          && ent->pain_debounce_time <= level.time)
         {
            if (rand()&1)
               gi.sound (ent, CHAN_VOICE, gi.soundindex("items/l_health.wav"), 1, ATTN_NORM, 0);
            else
               gi.sound (ent, CHAN_VOICE, gi.soundindex("items/n_health.wav"), 1, ATTN_NORM, 0);
            ent->pain_debounce_time = level.time + 0.5;
         }
         ent->health += 3*ent->waterlevel;
         if (ent->health > ent->max_health) ent->health = ent->max_health;
      }

      if (ent->watertype & CONTENTS_SLIME)
      {
         ent->health += ent->waterlevel;
         if (ent->health > ent->max_health) ent->health = ent->max_health;
      }

   }
}

// Clears the damage of things EPA grants immunity to.
int AQ_EPA_Damage(edict_t *targ, edict_t *attacker, edict_t *inflictor, int damageStart, int mod)
{
   int damage = damageStart;

   // If it were some sort of falling, blaster, etc, damage...
   if (attacker == world) damage = 0;

   // Most definitely make kamikaze kill the guy, even if it's yourself.
   if (inflictor && strcmp (inflictor->classname, "aq_bomb") == 0) return damage;

   // No splash damage :)
   if (attacker == targ) damage = 0;

   // No crush damage
   if (mod == MOD_CRUSH) damage = 0;

   // If it were some sort of AQ2-related damage...
   if (inflictor && (strcmp (inflictor->classname, "aq_death_orb") == 0 ||
                     strcmp (inflictor->classname, "aq_death_baby") == 0 ||
                     strcmp (inflictor->classname, "aq_radio_point") == 0 ||
                     strcmp (inflictor->classname, "aq_gravity_well") == 0 ||
                     strcmp (inflictor->classname, "aq_death_spot") == 0 ||
                     strcmp (inflictor->classname, "target_laser") == 0))
      damage = 0;

   // This will activate a white color shell
   if (damage == 0)
      targ->client->pers.AQ_Info[AQ_EPA].aTime = level.time + 2*FRAMETIME;

   return damage;
}

// Needed to reset movement after death teleport
extern mmove_t tank_move_stand;

void AQ_tank_thud (edict_t *self)
{
	gi.sound (self, CHAN_BODY, gi.soundindex ("tank/tnkdeth2.wav"), 1, ATTN_NORM, 0);
}

// Remove Tank immediately. Used after player death induced Tank deaths as well as unequipping.
void AQ_Remove_Tank (edict_t *self)
{
   AQ_TeleTempEnt (self->s.origin);
   G_FreeEdict (self);
}

// If Tank was just shot up and killed, return to owner
void AQ_Tank_Defeated (edict_t *self)
{
   gi.unlinkentity (self);
   self->deadflag = DEAD_NO;
   self->takedamage = DAMAGE_YES;
   AQ_TeleTempEnt(self->s.origin); // Faking a temp entity
   self->health = 750; // More health
    // Redo angles and position
   VectorCopy(self->owner->s.origin, self->s.origin);
   VectorCopy(self->owner->s.origin, self->s.old_origin);
   VectorCopy(self->owner->s.angles, self->s.angles);
   self->s.angles[0] = 0;
   self->s.skinnum &= ~1; // Fix his skin back
   self->s.event = EV_PLAYER_TELEPORT; // Teleport thingy in new area!
   self->monsterinfo.currentmove = &tank_move_stand;
   gi.linkentity (self);

   gi.centerprintf (self->owner, "Ž Your Tank Helper was defeated Ž");
}

// When Tank dies, you get these animation frames and death.
mframe_t aq_tank_frames_death1 [] =
{ ai_move, -7, NULL, ai_move, -2, NULL, ai_move, -2, NULL, ai_move, 1, NULL, ai_move, 3, NULL, 	ai_move, 6, NULL, ai_move, 1, NULL, ai_move, 1, NULL, ai_move, 2, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, -2, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, -3, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, -4, NULL, ai_move, -6, NULL, ai_move, -4, NULL, ai_move, -5, NULL, ai_move, -7, NULL, ai_move, -15, AQ_tank_thud, ai_move, -5, NULL, ai_move, 0, NULL, ai_move, 0, NULL, ai_move, 0, NULL};
mmove_t aq_tank_death = {222, 253, aq_tank_frames_death1, AQ_Tank_Defeated};
AQ_Tank_Die (edict_t *ent)
{
   gi.sound (ent, CHAN_VOICE, gi.soundindex ("tank/death.wav"), 1, ATTN_NORM, 0);
   ent->deadflag = DEAD_DYING;
   ent->takedamage = DAMAGE_NO;
   ent->monsterinfo.currentmove = &aq_tank_death; // Completely dead!
}

// Spawns a Tank Helper
AQ_Tank_Spawn (edict_t *ent)
{
   AQ_Held_Artifact *artifact = &(ent->client->pers.AQ_Info[AQ_HELPER]);
   edict_t *tank;
   vec3_t forward, temp;

   tank = G_Spawn();

   // Make Tank appear on player
   VectorCopy(ent->s.origin, tank->s.origin);
   VectorCopy(ent->s.origin, tank->s.old_origin);
   VectorCopy(ent->s.angles, tank->s.angles);
   tank->s.angles[0] = 0;

   // Link link link
   artifact->aEnt = tank;
   tank->owner = ent;
   tank->classname = "aq_tank_helper";

   SP_monster_tank (tank);

   // Different "death" ting
   tank->die = AQ_Tank_Die;

   if (random()*2 < 1) tank->s.skinnum = 2;
}

// Updates the display of Tank's health and keeps track of who he should attack
void AQ_Tank_Update (edict_t *ent)
{
   AQ_Held_Artifact *artifact = &(ent->client->pers.AQ_Info[AQ_HELPER]);
   edict_t *tank = artifact->aEnt;
   edict_t *target = NULL;
   edict_t *oldtarget = NULL;
   vec3_t  diff;
   float   dist;

   // Update displayed health
   if (ent->client->pers.AQ_Info[AQ_HELPER].aEnt->health) AQ_HUD_Num (ent);

   // Not time yet
   if (artifact->pTime > level.time) return;
   // If Tank already has a VISIBLE enemy, we don't need to worry about looking for one.
   if (tank->enemy && tank->enemy != ent && tank->enemy != world && visible(tank, tank->enemy)) return;

   oldtarget = tank->enemy;
   tank->enemy = NULL;
   while ((target = findradius(target, tank->s.origin, 1024)) != NULL && !tank->enemy)
   {
	if (target->takedamage != DAMAGE_NO &&                   // If it takes damage,
          !target->deadflag &&                                 // and isn't dead,
          target != ent &&                                     // and isn't the client which is Tank's owner,
          target != tank &&                                    // and isn't tank himself,
          (target->client || target->svflags & SVF_MONSTER) && // and is either a client or a monster...
          visible (tank, target))                              // and is visible
         tank->enemy = target;
   }

   // If no enemy was found, set some crap
   if (tank->enemy == NULL)
   {
      // If Tank had an enemy out of range, just look for the old one
      if (oldtarget && (oldtarget->client || oldtarget->svflags & SVF_MONSTER) && oldtarget != ent)
         tank->enemy = oldtarget;
      // If Tank didn't have an enemy, just make Tank walk to his owner.
      else
      {
         VectorSubtract (tank->s.origin, ent->s.origin, diff);
         dist = VectorLength (diff);
         if (dist > 100) // Owner is far
            tank->enemy = ent;
         else // Owner is close
            tank->enemy = world; 
      }
   }

   artifact->pTime = level.time + 0.5;
}

// Impact sword's thinking func
void AQ_Sword_Think (edict_t *ent)
{
   VectorCopy (ent->owner->s.origin, ent->s.origin);
   VectorCopy (ent->owner->s.angles, ent->s.angles);
   ent->s.origin[2] += AQ_HAT;

   ent->nextthink = level.time + FRAMETIME;
}

// Spawn an Impact sword
void AQ_Sword_Spawn (edict_t *ent)
{
   edict_t *sword;

   sword = G_Spawn();
   VectorCopy (ent->s.origin, sword->s.origin);
   VectorCopy (ent->s.origin, sword->s.old_origin);
   VectorClear (sword->avelocity);
   VectorClear (sword->velocity);
   VectorClear (sword->mins);
   VectorClear (sword->maxs);
   sword->movetype = MOVETYPE_FLY;
   sword->solid = SOLID_NOT;
   sword->owner = ent;
   sword->touch = NULL;
   sword->think = AQ_Sword_Think;
   sword->nextthink = level.time;
   sword->classname = "aq_impact_hat";
   sword->s.modelindex = gi.modelindex ("models/artifact/sword/tris.md2");
   gi.linkentity (sword);

   ent->client->pers.AQ_Info[AQ_IMPACT].aEnt = sword;
}

// Look for people to bash!
void AQ_Impact_Think (edict_t *ent)
{
   AQ_Held_Artifact *artifact = &(ent->client->pers.AQ_Info[AQ_IMPACT]);
   edict_t          *touch[MAX_EDICTS], *hit;
   int              i, num;

   if (artifact->aTime > level.time) return;

   num = gi.BoxEdicts (ent->absmin, ent->absmax, touch, MAX_EDICTS, AREA_SOLID);

   for (i=0 ; i<num ; i++)
   {
      hit = touch[i];
      if (hit->takedamage && hit != ent)
         AQ_Impact_Touch (ent, hit);
   }
}

// Bash people!
void AQ_Impact_Touch (edict_t *self, edict_t *other)
{
   AQ_Held_Artifact *artifact = &(self->client->pers.AQ_Info[AQ_IMPACT]);
   vec3_t diff, push, dir, normal;
   float diffSpeed;

   // If you aren't moving, don't do this.
   if (!VectorLength(self->velocity)) return;

   // No touch ground and push up a little
   other->groundentity = NULL;
   other->velocity[2] += 300;

   // Add velocity to the guy who was hit
   VectorSubtract(self->velocity, other->velocity, diff);
   diffSpeed = VectorLength(diff);
   VectorSubtract(other->s.origin, self->s.origin, push);
   VectorNormalize(push);
   VectorScale(push, diffSpeed*2, push);
   VectorAdd(other->velocity, push, other->velocity);

   // Bash!
   VectorCopy(push, dir);
   VectorNormalize(dir);
   T_Damage(other, self, self, dir, other->s.origin, vec3_origin, (int)(diffSpeed/7.0), 0, 0, MOD_AQ_IMPACT);

   // Bashing sound!
   gi.sound (self, CHAN_AUTO, gi.soundindex ("chick/chkatck4.wav"), 1, ATTN_NORM, 0);

   artifact->aTime = level.time + 0.5;
}

// Make people viewable range jump, when appropriate
void AQ_Jumpy_Think (edict_t *self)
{
   AQ_Held_Artifact *artifact = &(self->client->pers.AQ_Info[AQ_JUMPY]);
   edict_t *victim = NULL;
   float    distMax = 1024;
   trace_t  trace;

   // If it's not time to make people jump, don't.
   if (artifact->pTime > level.time) return;

   // Check massive viewable radius, and make things jump

   // Do some JUMPIN' !
   while ((victim = findradius(victim, self->s.origin, distMax)) != NULL)
   {
      if (victim == self) continue;

	if (victim->movetype == MOVETYPE_NONE ||
          victim->solid == SOLID_NOT ||
          victim->solid == SOLID_BSP ||
          victim->takedamage == DAMAGE_NO ||
          victim->deadflag != DEAD_NO ||
          OnSameTeam(victim, self))
         continue;

      // Can we see 'im?
      trace = gi.trace(self->s.origin, vec3_origin, vec3_origin, victim->s.origin, self, MASK_SOLID);
      if (trace.fraction < 1.0) continue;

      // EPA types aren't effected
      if (victim->client && victim->client->pers.AQ_Info[AQ_EPA].on) continue;

      // If 'e's on the ground, make him jump.
      if (victim->groundentity)
      {
         victim->groundentity = NULL;
         victim->velocity[2] = victim->velocity[2] + 300;

         // If it's a client, make 'im do a jumping noise
         if (victim->client)
         {
            gi.sound(victim, CHAN_VOICE, gi.soundindex("*jump1.wav"), 1, ATTN_NORM, 0);
            PlayerNoise(victim, victim->s.origin, PNOISE_SELF);
         }
      }
   }

   // Don't jump 
   artifact->pTime = level.time + 0.1;
}

// Zappy thing for electric artifact's thinking func
void AQ_Zap_Think (edict_t *ent)
{
   VectorCopy (ent->owner->s.origin, ent->s.origin);
   VectorCopy (ent->owner->s.angles, ent->s.angles);
   VectorSet (ent->s.angles, random()*360, random()*360, random()*360);
   ent->s.origin[2] += 20;

   // Choose a different lightning bolt thing, or none if there's no cells
   if (ent->owner->client->pers.AQ_Info[AQ_ZAP].aInt < 1)
   {
      // Change to nothing if no cells
      if (ent->s.modelindex != gi.modelindex ("models/artifact/a_null/tris.md2"))
         ent->s.modelindex = gi.modelindex ("models/artifact/a_null/tris.md2");
   }
   else
   {
      // Choose different appearences
      if (random() > 0.5)
      {
         if (ent->s.modelindex == gi.modelindex ("models/artifact/lightning/light1/tris.md2"))
            ent->s.modelindex = gi.modelindex ("models/artifact/lightning/light2/tris.md2");
         else if (ent->s.modelindex == gi.modelindex ("models/artifact/lightning/light2/tris.md2"))
            ent->s.modelindex = gi.modelindex ("models/artifact/lightning/light1/tris.md2");
         else // ent->s.modelindex == gi.modelindex ("models/artifact/lightning/light3/tris.md2")
            ent->s.modelindex = gi.modelindex ("models/artifact/lightning/light1/tris.md2");
      } else
      {
         if (ent->s.modelindex == gi.modelindex ("models/artifact/lightning/light1/tris.md2"))
            ent->s.modelindex = gi.modelindex ("models/artifact/lightning/light3/tris.md2");
         else if (ent->s.modelindex == gi.modelindex ("models/artifact/lightning/light2/tris.md2"))
            ent->s.modelindex = gi.modelindex ("models/artifact/lightning/light3/tris.md2");
         else // ent->s.modelindex == gi.modelindex ("models/artifact/lightning/light3/tris.md2")
            ent->s.modelindex = gi.modelindex ("models/artifact/lightning/light2/tris.md2");
      }
   }

   ent->nextthink = level.time + FRAMETIME;
}

// Spawn a lightning thing for the player
void AQ_Zap_Spawn (edict_t *ent)
{
   edict_t *zap;
   int     *cells = &(ent->client->pers.inventory[ITEM_INDEX(FindItem ("cells"))]);

   zap = G_Spawn();
   VectorCopy (ent->s.origin, zap->s.origin);
   VectorCopy (ent->s.origin, zap->s.old_origin);
   VectorClear (zap->avelocity);
   VectorClear (zap->velocity);
   VectorClear (zap->mins);
   VectorClear (zap->maxs);
   zap->movetype = MOVETYPE_FLY;
   zap->solid = SOLID_NOT;
   zap->owner = ent;
   zap->touch = NULL;
   zap->think = AQ_Zap_Think;
   zap->nextthink = level.time;
   zap->classname = "aq_zap_hat";
   zap->s.modelindex = gi.modelindex ("models/artifact/lightning/light1/tris.md2");
   zap->s.sound = gi.soundindex ("world/l_hum1.wav");
   zap->s.renderfx = RF_FULLBRIGHT;
   gi.linkentity (zap);

   ent->client->pers.AQ_Info[AQ_ZAP].aEnt = zap;
   ent->client->pers.AQ_Info[AQ_ZAP].aInt = *cells;
}

// Radius damage for those only in water. Lighning damage
// This is an exact copy of the T_RadiusDamage func, but with the "must be in water" condition added.
void T_LightningDamage (edict_t *inflictor, edict_t *attacker, float damage, edict_t *ignore, float radius)
{
	float	points;
	edict_t	*ent = NULL;
	vec3_t	v;
	vec3_t	dir;

	while ((ent = findradius(ent, inflictor->s.origin, radius)) != NULL)
	{
		if (ent == ignore)
			continue;
		if (!ent->takedamage)
			continue;
		if (ent->client && !ent->waterlevel) continue;

		VectorAdd (ent->mins, ent->maxs, v);
		VectorMA (ent->s.origin, 0.5, v, v);
		VectorSubtract (inflictor->s.origin, v, v);
		points = damage - 0.5 * VectorLength (v);
		if (ent == attacker)
			points = points * 0.5;
		if (points > 0)
		{
			if (CanDamage (ent, inflictor))
			{
				VectorSubtract (ent->s.origin, inflictor->s.origin, dir);
				T_Damage (ent, inflictor, attacker, dir, inflictor->s.origin, vec3_origin, (int)points, (int)points, DAMAGE_RADIUS, MOD_AQ_ELECTRIC2);
			}
		}
	}
}

// Look for people to zap! (And flash sometimes)
void AQ_Electric_Think (edict_t *ent)
{
   AQ_Held_Artifact *artifact = &(ent->client->pers.AQ_Info[AQ_ZAP]);
   edict_t *victim = NULL;
   edict_t *trueVic = NULL;
   float    distMax = 300;
   float    dist = distMax;
   float    leastDist = distMax;
   vec3_t   dOrg;
   trace_t  trace;
   int     *cells;

   // Make occasional flashes
   if (random()*100 > 70)
   {
      gi.WriteByte (svc_muzzleflash);
      gi.WriteShort (ent-g_edicts);
      gi.WriteByte (MZ_NUKE4);
      gi.multicast (ent->s.origin, MULTICAST_PVS);
   }

   // Jumped-in-water check
   if (ent->waterlevel > 1 && (ent->watertype&(CONTENTS_WATER|CONTENTS_SLIME)))
   {
      cells = &(ent->client->pers.inventory[ITEM_INDEX(FindItem ("cells"))]);
      if (*cells > 0)
      {
         T_LightningDamage(ent, ent, 20*(*cells), NULL, 20*(*cells));
         *cells = 0;
         gi.sound (ent, CHAN_AUTO, gi.soundindex("artifact/lstart.wav"), 1.0, ATTN_NORM, 0);

         artifact->aInt = *cells;
         AQ_HUD_Num (ent);

         return;
      }
   }

   // If it's not time to make people get zapped, don't.
   if (artifact->pTime > level.time) return;

   // Update HUD cause we ALWAYS update the HUD
   // Also, if player doesn't have enough cells, they can't zap anyone
   cells = &(ent->client->pers.inventory[ITEM_INDEX(FindItem ("cells"))]);
   if (artifact->aInt != *cells)
   {
      artifact->aInt = *cells;
      AQ_HUD_Num (ent);
   }
   if (*cells < 1) return;

   // Check massive viewable radius, and zap shit
   // Find people to hurt!
   while ((victim = findradius(victim, ent->s.origin, distMax)) != NULL)
   {
      if (victim == ent) continue;
	if (victim->takedamage == DAMAGE_NO) continue;
      if (victim->client && victim->client->pers.AQ_Info[AQ_EPA].on) continue;
      if (victim->owner && victim->owner == ent) continue;
      if (OnSameTeam(victim, ent)) continue;

      // Can we see 'im?
      trace = gi.trace(ent->s.origin, vec3_origin, vec3_origin, victim->s.origin, ent, MASK_SOLID);
      if (trace.fraction < 1.0) continue;

      // EPA types aren't effected
      if (victim->client && victim->client->pers.AQ_Info[AQ_EPA].on) continue;

      // Less far away?
      VectorSubtract(victim->s.origin, ent->s.origin, dOrg);
      dist = VectorLength(dOrg);
      if (dist <= leastDist) {trueVic = victim; leastDist = dist;}
   }
   victim = trueVic;

   if (victim)
   {
      // Flash effects
      gi.WriteByte (svc_muzzleflash);
      gi.WriteShort (ent-g_edicts);
      gi.WriteByte (MZ_NUKE8);
      gi.multicast (victim->s.origin, MULTICAST_PVS);
      gi.WriteByte (svc_muzzleflash);
      gi.WriteShort (victim-g_edicts);
      gi.WriteByte (MZ_NUKE8);
      gi.multicast (victim->s.origin, MULTICAST_PVS);
      // Beam effect
      gi.WriteByte (svc_temp_entity);
      gi.WriteByte (TE_PARASITE_ATTACK);
      gi.WriteShort (ent - g_edicts);
      gi.WritePosition (artifact->aEnt->s.origin);
      gi.WritePosition (trace.endpos);
      gi.multicast (ent->s.origin, MULTICAST_PVS);
      // Damage
      T_Damage(victim, ent, ent, vec3_origin, victim->s.origin, vec3_origin, 20, 5, 0, MOD_AQ_ELECTRIC);
      // Sound
      gi.sound (ent, CHAN_AUTO, gi.soundindex("artifact/lhit.wav"), 1.0, ATTN_NORM, 0);
      *cells -= 1; // Less cells!
      artifact->aInt = *cells;
      AQ_HUD_Num (ent); // Show the number on the display
   }

   // Don't zap ALL the time 
   artifact->pTime = level.time + 0.5;
}

void AQ_Fire_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
   if (other->takedamage == DAMAGE_NO ||
       other == ent->owner ||
       other->solid == SOLID_NOT ||
       other->solid == SOLID_BSP ||
       OnSameTeam(other, ent->owner)) return;
 
   gi.sound(other, CHAN_AUTO, gi.soundindex ("player/lava_in.wav"), 1, ATTN_NORM, 0);
   T_Damage(other, ent, ent->owner, vec3_origin, other->s.origin, vec3_origin, 25, 10, 0, MOD_AQ_FIREWALK);
   G_FreeEdict (ent);
}

void AQ_Fire_Think (edict_t *ent)
{
   if (ent->s.frame++ > 13) ent->s.frame = 12; 
   ent->nextthink = level.time + FRAMETIME;
   if (ent->delay < level.time) G_FreeEdict(ent);
}

// Spawns some fire when you use walk
void AQ_Fire_Spawn (edict_t *ent)
{
   AQ_Held_Artifact *artifact = &(ent->client->pers.AQ_Info[AQ_FIREWALK]);
   edict_t *fire;
   vec3_t   dir;

   fire = G_Spawn();
   VectorCopy (ent->s.origin, fire->s.origin);
   VectorCopy (ent->s.origin, fire->s.old_origin);
   fire->s.origin[2] -= 26;
   fire->s.old_origin[2] -= 26;
   VectorClear (fire->avelocity);
   VectorClear (fire->velocity);
   VectorSet (fire->mins, -17, -17, -10);
   VectorSet (fire->maxs, 17, 17, 22);
   fire->s.angles[1] = random()*360;
   fire->movetype = MOVETYPE_NONE;
   fire->solid = SOLID_TRIGGER;
   fire->owner = ent;
   fire->touch = AQ_Fire_Touch;
   fire->think = AQ_Fire_Think;
   fire->nextthink = level.time;
   fire->classname = "aq_fire";
   fire->s.modelindex = gi.modelindex ("models/artifact/flame/tris.md2");
   fire->s.sound = gi.soundindex ("player/fry.wav");
   fire->s.effects = EF_IONRIPPER;
   fire->s.renderfx = RF_TRANSLUCENT;
   fire->delay = level.time + 1.3;
   gi.linkentity (fire);

   if (artifact->aEnt)
      artifact->aEnt->s.sound = 0;
   artifact->aEnt = fire;
}

// Thinking func for fire walker
void AQ_FireWalk_Think (edict_t *ent)
{
   AQ_Held_Artifact *artifact = &(ent->client->pers.AQ_Info[AQ_FIREWALK]);
   if (artifact->pTime > level.time ||
       !VectorLength(ent->velocity) ||
       !(ent->groundentity) ||
       ent->waterlevel > 0)
      return;

   AQ_Fire_Spawn (ent);
   artifact->pTime = level.time + FRAMETIME;
}

// Remove the effects of weirdness
void AQ_UnWeird (edict_t *ent)
{
   ent->target_ent->client->pers.AQ_State &= ~AQ_STATE_WEIRD;

   // Fix values if needed
   if (ent->health == 4) ent->target_ent->client->ps.fov = ent->max_health;
   if (ent->health == 6) ent->target_ent->client->pers.AQ_State &= ~AQ_STATE_NOSHOOT;

   G_FreeEdict (ent);
}

// Continue inflicting weirdness!
void AQ_Bubble_Weirding (edict_t *ent)
{
   if (ent->delay < level.time || ent->target_ent->health < 1 || !ent->target_ent || !ent->target_ent->client)
   {
      AQ_UnWeird (ent);
      return;
   }
   VectorCopy (ent->target_ent->s.origin, ent->s.origin);

   // Weird mode: UNCONTROLLABLE JUMPING!
   if (ent->health == 0)
   {
      if (ent->target_ent->groundentity)
      {
         ent->target_ent->groundentity = NULL;
         ent->target_ent->velocity[2] = ent->target_ent->velocity[2] + 300;

         // If it's a client, make 'im do a jumping noise
         if (ent->target_ent->client)
         {
            gi.sound(ent->target_ent, CHAN_VOICE, gi.soundindex("*jump1.wav"), 1, ATTN_NORM, 0);
            PlayerNoise(ent->target_ent, ent->target_ent->s.origin, PNOISE_SELF);
         }
      }      
   }
   // Weird mode: DRUNKEN STUMBLE!
   else if (ent->health == 1)
   {
      VectorSet(ent->target_ent->client->kick_angles, 2-random()*4, 2-random()*4, 2-random()*4);
      if (VectorLength(ent->target_ent->velocity) > 100 && ent->target_ent->groundentity)
      {
         ent->target_ent->velocity[0] += 450-random()*800;
         ent->target_ent->velocity[1] += 450-random()*800;
      }
   }
   // Weird mode: WOBBLY HEAD!
   else if (ent->health == 2)
   {
      VectorSet(ent->target_ent->client->kick_angles, 20-random()*40, 20-random()*40, 20-random()*40);
   }
   // Weird mode: LETHARGY!
//TODO: improve this mode
   else if (ent->health == 3)
   {
      if (ent->target_ent->velocity[0] > 10 || ent->target_ent->velocity[0] < -10) ent->target_ent->velocity[0] *= 0.5;
      if (ent->target_ent->velocity[1] > 10 || ent->target_ent->velocity[1] < -10) ent->target_ent->velocity[1] *= 0.5;
      if (ent->groundentity) {ent->groundentity = NULL; VectorClear(ent->target_ent->velocity); ent->target_ent->velocity[2] += 10;}
   }
   // Weird mode: SHROOM MODE!
   else if (ent->health == 4)
   {
      if (ent->gib_health)
      {
         ent->target_ent->client->ps.fov += 20;
         if (ent->target_ent->client->ps.fov+20 > 180) ent->gib_health = 0;
      } else {
         ent->target_ent->client->ps.fov -= 20;
         if (ent->target_ent->client->ps.fov-20 < 10) ent->gib_health = 1;
      }
   }
   // Weird mode: FERRET SHOCK!
   else if (ent->health == 5)
   {
      int   i, f, r, g, b;
      ent->gib_health = anglemod (ent->gib_health + 10);
      i = (int)(((float)ent->gib_health) / 60);
      f = (ent->gib_health)%60;
      if (i == 0) {r = 60;   g = f;    b = 0;}
      if (i == 1) {r = 60-f; g = 60;   b = 0;}
      if (i == 2) {r = 0;    g = 60;   b = f;}
      if (i == 3) {r = 0;    g = 60-f; b = 60;}
      if (i == 4) {r = f;    g = 0;    b = 60;}
      if (i == 5) {r = 60;   g = 0;    b = 60-f;}
      VectorSet(ent->target_ent->client->damage_blend, (float)r/60, (float)g/60, (float)b/60);
      ent->target_ent->client->damage_alpha = 0.8;
   }
   // Weird mode: PACIFISM!
   else if (ent->health == 6)
   {
      if (!(ent->target_ent->client->pers.AQ_State & AQ_STATE_NOSHOOT))
         ent->target_ent->client->pers.AQ_State |= AQ_STATE_NOSHOOT;
   }
   // Weird mode: STOOD UP TOO FAST MODE
   else if (ent->health == 7)
   {
      if (VectorLength (ent->target_ent->velocity))
      {
         VectorSet(ent->target_ent->client->damage_blend, 0, 0, 0);
         ent->target_ent->client->damage_alpha = 1.0;
      }
   }

   ent->nextthink = level.time + FRAMETIME;
}

// Returns weirdness names 
char* AQ_Weird_Name (int type)
{
   if (type == 0) return "UNCONTROLLABLE JUMPING!";
   if (type == 1) return "DRUNKEN STUMBLE!";
   if (type == 2) return "WOBBLY HEAD!";
   if (type == 3) return "LETHARGY!";
   if (type == 4) return "SHROOM MODE!";
   if (type == 5) return "FERRET SHOCK!";
   if (type == 6) return "PACIFISM!";
   if (type == 7) return "STOOD UP TOO FAST MODE!";
   return "Unknown mode!";
   //TODO: make more modes!
   //TODO: fix lethargy!
}

// Touch someone and inflict weirdness!
void AQ_Bubble_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
   if (!other->client ||
       other == ent->owner ||
       other->client->pers.AQ_State & AQ_STATE_WEIRD ||
       other->client->pers.AQ_Info[AQ_EPA].on ||
       OnSameTeam(other, ent->owner))
   {
      if (ent->groundentity) { ent->groundentity = NULL; ent->velocity[2] += 50; }
      return;
   }

   other->client->pers.AQ_State |= AQ_STATE_WEIRD;

//TODO: remove :)
   if (other->health > 8) ent->health = random()*8; else ent->health = other->health;
//   ent->health = random()*8;

   // Change bubble as necessary
   ent->solid = SOLID_NOT;
   ent->touch = NULL;
   ent->think = AQ_Bubble_Weirding;
   ent->nextthink = level.time;
   ent->s.sound = gi.soundindex ("artifact/orb2.wav");
   ent->s.modelindex = gi.modelindex ("models/artifact/a_null/tris.md2");
   ent->delay = level.time + 15;
   ent->target_ent = other;

   // Save the fov
   if (ent->health == 4)
   {
      ent->max_health = ent->target_ent->client->ps.fov;
      ent->gib_health = 1;
   }
   // Start at a random hue
   if (ent->health == 5) ent->gib_health = random()*360;

   // Alert the owner that a guy was hit
   if (ent->owner && ent->owner->client)
      gi.cprintf (ent->owner, PRINT_HIGH, "Ž You inflicted %s with %s Ž\n", ent->target_ent->client->pers.netname, AQ_Weird_Name (ent->health));
   // Alert the victim!
   gi.centerprintf (ent->target_ent, "Ž Weirdness hit! Ž\nŽ %s Ž", AQ_Weird_Name (ent->health));
}

// Wander around, I say to the bubble
void AQ_Bubble_Think (edict_t *ent)
{
   if (ent->groundentity) ent->groundentity = NULL;
  
   ent->velocity[0] += 8 - random()*16;
   ent->velocity[1] += 8 - random()*16;
   ent->velocity[2] += 8 - random()*16;

   ent->nextthink = level.time + 0.2;
   if (ent->delay < level.time) G_FreeEdict(ent);
}

// Spawn a weirdness bubble
edict_t* AQ_Bubble_Spawn (edict_t *ent)
{
   edict_t *bubble;

   bubble = G_Spawn();
   VectorCopy (ent->s.origin, bubble->s.origin);
   VectorCopy (ent->s.origin, bubble->s.old_origin);
   VectorSet (bubble->velocity, 20-random()*60, 30-random()*60, 30-random()*60);
   VectorSet (bubble->avelocity, 360-random()*720, 360-random()*720, 360-random()*720);
   VectorSet (bubble->mins, -5, -5, -5);
   VectorSet (bubble->maxs, 5, 5, 5);
   bubble->movetype = MOVETYPE_FLY;
   bubble->solid = SOLID_TRIGGER;
   bubble->owner = ent;
   bubble->touch = AQ_Bubble_Touch;
   bubble->think = AQ_Bubble_Think;
   bubble->nextthink = level.time;
   bubble->classname = "aq_bubble";
   bubble->s.modelindex = gi.modelindex ("models/artifact/weird/tris.md2");
   bubble->s.effects = EF_SPHERETRANS;
   bubble->s.renderfx = RF_FULLBRIGHT;
   bubble->delay = level.time + 4 + random()*3;
   gi.linkentity (bubble);

   return bubble;
}

// Occasionally spawn some bubbles
void AQ_Weird_Think (edict_t *ent)
{
  AQ_Held_Artifact *artifact = &(ent->client->pers.AQ_Info[AQ_WEIRD]);

  if (artifact->pTime > level.time) return;
  AQ_Bubble_Spawn (ent);
  artifact->pTime = level.time + 0.3 + random()*1;
}

// When you die with weirdness, a bunch of bubbles come off
void AQ_Weird_Death (edict_t *ent)
{
   int n;
   // If actually dead
   if (ent->client && ent->health < 1)
      for (n=0;n<5;n++) AQ_Bubble_Spawn (ent);
}



