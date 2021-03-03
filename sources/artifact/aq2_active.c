
// ************************************
// Artifact Quake 2 active effects code
// ************************************

// "Artifact" and "Artifact Quake 2" are (c) Copyright 2000 Nick "Singe" Bousman and Glenn "Griphis" Saaiman

// Do not modify and redistribute any AQ2 materials without Singe's or Griphis's permission.
// Do not modify and redistribute this source without providing publically the modified source.
// Do not modify and redistribute this code without giving Singe approrpiate credit!
// Do not modify and redistribute pictures or models from this patch without giving Griphis appropriate credit!

// The code in this section handles anything in Artifact Quake 2 that deals
// active effects of artifacts. This is almost exclusively things that
// happen when The Button is pressed.

// **************************************************************************

#include "g_local.h"

//TODO: why does this function have to go after where it's called? crashes otherwise. this is the prototype, since it's going after
void AQ_LaserThinkGo (edict_t *ent);

// A player pressed The Button. Activate the appropriate effect.
void AQ_Active_Effect (edict_t *ent)
{
   gclient_t *client = ent->client;
   AQ_Held_Artifact *artifact = &(ent->client->pers.AQ_Info[ent->client->pers.AQ_Equipped]);

   if (client->pers.AQ_Equipped == NO_ARTIFACT) return; // If no artifact is equipped

   // Activate or recall Gravity Well 
   else if (client->pers.AQ_Equipped == AQ_GWELL) AQ_GWell (ent);

   // Set or teleport to Recall point
   else if (client->pers.AQ_Equipped == AQ_RECALL) AQ_Recall (ent);

   // Toggle between two Recall points
   else if (client->pers.AQ_Equipped == AQ_RECALL2) AQ_Recall2 (ent);

   // Toggle between two Recall points
   else if (client->pers.AQ_Equipped == AQ_KAMIKAZE) AQ_Explode_Start (ent);

   // Set a Death Spot
   else if (client->pers.AQ_Equipped == AQ_DEATH) AQ_Death_Spot (ent);

   // Set an Ice Trap
   else if (client->pers.AQ_Equipped == AQ_FREEZE) AQ_Ice_Trap (ent);

   // Place a hologram
   else if (client->pers.AQ_Equipped == AQ_HOLOGRAM) AQ_Hologram (ent);

   // If Haste, do a haste jump
   else if (client->pers.AQ_Equipped == AQ_HASTE) AQ_Haste_Jump (ent);

   // If Shield, activate the shield (or try)
   else if (client->pers.AQ_Equipped == AQ_SHIELD) AQ_Shield_Activate (ent);

   // If EPA, perform an enviro-jump
   else if (client->pers.AQ_Equipped == AQ_EPA) AQ_EPA_Jump (ent);

   // If Orb, switch modes
   else if (client->pers.AQ_Equipped == AQ_ORB) AQ_Toggle_Orb (ent);

   // If Tank Helper, teleport him to self
   else if (client->pers.AQ_Equipped == AQ_HELPER) AQ_Teleport_Tank (ent);

   // If Radioactive, try to set a point
   else if (client->pers.AQ_Equipped == AQ_RADIO) AQ_Spawn_Radio_Cannister (ent);

   // If Flight, change the speed setting
   else if (client->pers.AQ_Equipped == AQ_FLIGHT) AQ_Change_Flight_Speed (ent);

   // If Blink, teleport player to a random location
   else if (client->pers.AQ_Equipped == AQ_BLINK) AQ_Blink (ent);

   // If Weird, push off a bubble
   else if (client->pers.AQ_Equipped == AQ_WEIRD) AQ_Bubble_Push (ent);

   // If teleport shuffle, do that thing
   else if (client->pers.AQ_Equipped == AQ_TELE) AQ_Shuffle (ent);
}

// Effects ******************************************

// Make the gravity well appear, make the gravity well dissapear.
void AQ_GWell (edict_t *ent)
{
   AQ_Held_Artifact *artifact = &(ent->client->pers.AQ_Info[AQ_GWELL]);

   if (artifact->pTime > level.time) return;

   if (artifact->aInt == 0)
   {
      ent->client->pers.AQ_SNum = artifact->aInt = 1;
      AQ_GWell_Spawn (ent);
      artifact->pTime = level.time + 0.5;
   }
   else if (artifact->aInt == 1)
   {
      ent->client->pers.AQ_SNum = artifact->aInt = 0;
      gi.centerprintf (ent, "Ž Gravity Well recalled Ž");
      AQ_GWell_Kill (artifact->aEnt);
      artifact->pTime = level.time + 2;
   }
}

void AQ_GWell_Spawn (edict_t *ent)
{
   edict_t *gwell;
   vec3_t   aVel;
   vec3_t   pos;
   trace_t trace;

   gwell = G_Spawn();

   VectorCopy (ent->s.origin, pos);
   pos[2] += AQ_HAT;
   trace = gi.trace (ent->s.origin, vec3_origin, vec3_origin, pos, ent, MASK_SOLID);
   VectorCopy(trace.endpos, pos);
   VectorCopy (pos, gwell->s.origin);
   VectorCopy (aVel, gwell->avelocity);
   VectorClear (gwell->velocity);
   VectorSet (gwell->mins, 0, 0, 0);
   VectorSet (gwell->maxs, 0, 0, 0);
   gwell->movetype = MOVETYPE_FLY;
   gwell->solid = SOLID_TRIGGER;
   gwell->owner = ent;
   gwell->touch = AQ_GWell_Touch;
   gwell->nextthink = level.time;
   gwell->think = AQ_GWell_Think;
   gwell->classname = "aq_gravity_well";
   gwell->s.effects = EF_TRACKER | EF_SPHERETRANS;
   gwell->s.renderfx = 0;
   gwell->s.modelindex = gi.modelindex ("models/artifact/gwell/tris.md2");
   gwell->s.modelindex2 = gi.modelindex ("models/artifact/gwell2/tris.md2");
   gwell->s.sound = gi.soundindex ("weapons/rockfly.wav");
   gwell->s.frame = 0;
   gwell->delay = 0; // If zero, do nothing. Otherwise, kill if less than level.time.
                     // Used for making it die a while after owner death (otherwise it'd die instantly).
   VectorSet (gwell->avelocity, 720-random()*360, 720-random()*360, 720-random()*360);
   gi.linkentity (gwell);

   // The effect the gwell exhibits when placed
   gi.WriteByte (svc_temp_entity);
   gi.WriteByte (TE_NUKEBLAST);
   gi.WritePosition (pos);
   gi.multicast (pos, MULTICAST_PVS);

   // Sound of player setting a gwell
   gi.sound (ent, CHAN_AUTO, gi.soundindex ("misc/tele1.wav"), 1, ATTN_NORM, 0);

   ent->client->pers.AQ_Info[AQ_GWELL].aEnt = gwell;
}

void AQ_GWell_Kill (edict_t *ent)
{
   ent->classname = "aq_dead_gwell";
   ent->s.effects = 0;
   ent->s.renderfx = 0;
   ent->s.sound = 0;
   ent->touch = NULL;
   ent->think = G_FreeEdict;
   ent->nextthink = 1 + level.time;
   ent->s.modelindex = 0;

   gi.WriteByte (svc_temp_entity);
   gi.WriteByte (TE_TRACKER_EXPLOSION);
   gi.WritePosition (ent->s.origin);
   gi.multicast (ent->s.origin, MULTICAST_PVS);

   gi.sound (ent, CHAN_AUTO, gi.soundindex ("misc/tele1.wav"), 1, ATTN_NORM, 0);
}

// Where the Gravity Well actually sucks people up
void AQ_GWell_Think (edict_t *ent)
{
   edict_t *victim = NULL;
   float    distMax = 700;
   float    dist;
   vec3_t   dOrg, force;

   if (ent->nextthink > level.time) return;

   // Cycle gravity well frames
   if (ent->s.frame < 5)
      ent->s.frame++;
   else
      ent->s.frame = 0;

   // Do some SUCKIN' !
   while ((victim = findradius(victim, ent->s.origin, distMax)) != NULL)
   {
      if (victim == ent->owner || victim == ent) continue;

	if (victim->movetype == MOVETYPE_NONE ||
          victim->solid == SOLID_NOT ||
          victim->solid == SOLID_BSP ||
          victim->s.renderfx & RF_GLOW ||
          OnSameTeam(victim, ent->owner))
         continue;

      VectorSubtract (ent->s.origin, victim->s.origin, dOrg);
      dist = VectorLength (dOrg);
      VectorNormalize (dOrg);
      VectorScale (dOrg, 0.42, force);
      VectorScale (force, (distMax-dist), force);
      VectorAdd (victim->velocity, force, victim->velocity);

      // If the owner dies, the well doesn't go away as if he had just de-equipped. This checks for that.
      if (ent->delay)
         if (ent->delay < level.time)
         {
            AQ_GWell_Kill (ent);
            return;
         }
   }

   ent->nextthink = level.time + 0.1;
}

void AQ_GWell_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
   if (other == world) return;
   if (other == ent->owner) return;
   if (other->takedamage == DAMAGE_NO) return;
   if (ent->air_finished > level.time) return;

   T_Damage (other, ent, ent->owner, vec3_origin, ent->s.origin, vec3_origin, 1, 0, 0, MOD_AQ_GRAVITY_WELL);

   ent->air_finished = level.time + 0.2; // Don't hurt people CONSTANTLY
}

// Sets or erases the recall point, or teleports the player to it
void AQ_Recall (edict_t *ent)
{
   AQ_Held_Artifact *artifact = &(ent->client->pers.AQ_Info[AQ_RECALL]);
   vec3_t temp;

   if (artifact->aInt > level.time) return;
   artifact->aInt = level.time + 0.2;

   if (!VectorLength(artifact->aVector)) // If initially setting the vector...
   {
      VectorCopy (ent->s.origin, artifact->aVector);
      gi.centerprintf (ent, "Ž Recall point set Ž");
      ent->client->pers.AQ_SNum = 1;
      return;
   }

   if (gi.pointcontents (artifact->aVector) == CONTENTS_SOLID) // If the point has solid crap in it, erase it.
   {
      VectorClear (artifact->aVector);
      gi.centerprintf (ent, "Ž Recall point in solid rock. Erased. Ž");
      ent->client->pers.AQ_SNum = 0;
      return;
   }

   VectorSubtract(artifact->aVector, ent->s.origin, temp);
   if (VectorLength(temp) < 20) // If you're really near your recall point...
   {
      VectorClear (artifact->aVector);
      gi.centerprintf (ent, "Ž Recall point erased Ž");
      ent->client->pers.AQ_SNum = 0;
      return;
   }

   // Time to teleport to recall point! :)

   gi.unlinkentity (ent); // Unlink player to not interfere with Killbox
   VectorCopy (ent->s.origin, temp); // Temporarily remember the old spot
   VectorCopy (artifact->aVector, ent->s.origin);
   VectorCopy (artifact->aVector, ent->s.old_origin);
   ent->s.origin[2] += 10;
    // clear the velocity
   VectorClear (ent->velocity); //TODO: Why doesn't this clear the velocity?
    // draw the teleport splash at both spots
   ent->s.event = EV_PLAYER_TELEPORT;
   AQ_TeleTempEnt(temp); // Faking a temp entity
    // kill anything at the destination
   KillBox (ent);
   gi.linkentity (ent);
}

// Teleports player to where they picked up or equipped the artifact, or the last place they teleported from
void AQ_Recall2 (edict_t *ent)
{
   AQ_Held_Artifact *artifact = &(ent->client->pers.AQ_Info[AQ_RECALL2]);
   vec3_t temp;

   if (artifact->aInt > level.time) return;
   artifact->aInt = level.time + 0.2;

   if (gi.pointcontents (artifact->aVector) == CONTENTS_SOLID) // If the point has solid crap in it, erase it.
   {
      VectorCopy (ent->s.origin, artifact->aVector);
      gi.centerprintf (ent, "Ž Recall point in solid rock. Erased. Ž");
      return;
   }

   VectorCopy (ent->s.origin, temp);

   gi.unlinkentity (ent); // Unlink player to not interfere with Killbox
   VectorCopy (artifact->aVector, ent->s.origin);
   VectorCopy (artifact->aVector, ent->s.old_origin);
   ent->s.origin[2] += 10;
    // clear the velocity
   VectorClear (ent->velocity); //TODO: Why doesn't this clear the velocity?
    // draw the teleport splash at both spots
   ent->s.event = EV_PLAYER_TELEPORT;
   AQ_TeleTempEnt(temp); // Faking a temp entity
    // kill anything at the destination
   KillBox (ent);
   gi.linkentity (ent);

   VectorCopy (temp, artifact->aVector);
}

AQ_TeleTempEnt (vec3_t origin)
{
   edict_t *temp;

   temp = G_Spawn();

   VectorCopy (origin, temp->s.old_origin);
   VectorCopy (origin, temp->s.origin);
   VectorClear (temp->avelocity);
   VectorClear (temp->velocity);
   VectorClear (temp->mins);
   VectorClear (temp->maxs);
   temp->movetype = MOVETYPE_NONE;
   temp->solid = SOLID_NOT;
   temp->owner = NULL;
   temp->touch = NULL;
   temp->think = G_FreeEdict;
   temp->nextthink = level.time + 1.5;
   temp->classname = "aq_temp_tele";
   temp->s.modelindex = 0;
   gi.linkentity (temp);

   temp->s.event = EV_PLAYER_TELEPORT;
}

// Modified from Quake 2's built in target_earthquake_think so it'll go away when done
void AQ_target_earthquake_think (edict_t *ent)
{
   int i;
   edict_t *e;

   if (ent->last_move_time < level.time)
   {
      gi.positioned_sound (ent->s.origin, ent, CHAN_AUTO, ent->noise_index, 1.0, ATTN_NONE, 0);
      ent->last_move_time = level.time + 0.5;
   }

   for (i=1, e=g_edicts+i; i < globals.num_edicts; i++,e++)
   {
      if (!e->inuse)
         continue;
      if (!e->client)
         continue;
      if (!e->groundentity)
         continue;

      e->groundentity = NULL;
      e->velocity[0] += crandom()* 150;
      e->velocity[1] += crandom()* 150;
      e->velocity[2] = ent->speed * (100.0 / e->mass);
   }





   if (level.time < ent->timestamp)
      ent->nextthink = level.time + FRAMETIME;
   else
      G_FreeEdict(ent);
}

// Spawns an intangible entity that does an earthquake thingy.
void AQ_Earthquake (edict_t *ent)
{
   edict_t *quake;

   // Spawn the earthquake thingy
   quake = G_Spawn();
   VectorCopy (ent->s.origin, quake->s.origin);
   VectorCopy (ent->s.origin, quake->s.old_origin);
   VectorClear (quake->mins);
   VectorClear (quake->maxs);
   quake->movetype = MOVETYPE_NONE;
   quake->solid = SOLID_NOT;
   quake->owner = ent;
   quake->think = AQ_target_earthquake_think;
   quake->nextthink = level.time + FRAMETIME;
   quake->classname = "aq_quake";
   quake->noise_index = gi.soundindex ("world/quake.wav");
   quake->count = 4; // Duration
   quake->speed = ((float)ent->health/(float)ent->max_health) * 300; // Severity
   quake->timestamp = level.time + quake->count;
   quake->last_move_time = 0;

   gi.linkentity (quake);
}

AQ_Explode_Think (edict_t *ent)
{
   // The regular thinking func for following the owner
   AQ_Bomb_Think (ent);

   // Only access this loop if it's one of the count times
   if (ent->delay <= level.time)
   {
      ent->delay = level.time + 0.5;
      if (ent->health > 0)
      {
         gi.sound (ent, CHAN_AUTO, gi.soundindex("weapons/grenlr1b.wav"), 1, ATTN_NORM, 0);
         AQ_HUD_Num (ent->owner);
         ent->health--;
      }
      else
         AQ_Explode(ent->owner);
   }
}

// The effect for Divine Wind going off.
AQ_Explode (edict_t *ent)
{
   AQ_Held_Artifact *artifact = &(ent->client->pers.AQ_Info[AQ_KAMIKAZE]);

   // Kill everybody
   T_RadiusDamage(artifact->aEnt, ent, 8192, ent, 8192, MOD_AQ_KAMIKAZE);

   // Earthquake! }:> (Happens early so we can use the health)
   AQ_Earthquake(ent);

   // This keeps Shielding freaks from messing up the KamiKaze.
   if (ent->client->pers.AQ_Info[AQ_SHIELD].on && ent->client->pers.AQ_Info[AQ_SHIELD].aInt) ent->takedamage = DAMAGE_AIM;

   // Kill self.
   T_Damage(ent, artifact->aEnt, ent, artifact->aEnt->s.origin, ent->s.origin, vec3_origin, 8192, 0, DAMAGE_NO_PROTECTION, MOD_AQ_KAMIKAZE);

   // No more grenade thingy.
   G_FreeEdict(artifact->aEnt);

   // Blast effect I'm temporarily using
   gi.WriteByte (svc_temp_entity);
   gi.WriteByte (TE_NUKEBLAST);
   gi.WritePosition (ent->s.origin);
   gi.multicast (ent->s.origin, MULTICAST_PVS);

   // Kickass explosion effect
   AQ_Big_Boom (ent->s.origin);
}

// When a player kills somebody who's holding Divine Wind, a smaller explosion happens
void AQ_Death_Explode (edict_t *ent)
{
   AQ_Held_Artifact *artifact = &(ent->client->pers.AQ_Info[AQ_KAMIKAZE]);

   // Kill nearby people
   T_RadiusDamage(ent, ent, 256, ent, 512, MOD_AQ_KAMIKAZE2);

   // No more grenade thingy.
   G_FreeEdict(artifact->aEnt);

   // Same effect as going off manually
   AQ_Big_Boom (ent->s.origin);

   return;
}

// Set in motion the mechanisms of divine wind
void AQ_Explode_Start (edict_t *ent)
{
   // The way this works is, this function makes the grenade above the
   // player's head count down until explosion. We don't HAVE to use the
   // grenade, but since it automatically dissapears in all circumstances
   // in which the countdown should stop (player disconnects, dies, etc),
   // it's very convenient to use as the countdown timer.

   AQ_Held_Artifact *artifact = &(ent->client->pers.AQ_Info[AQ_KAMIKAZE]);

   if (artifact->aInt == 1) return;

   artifact->aInt = 1;
   artifact->aEnt->think = AQ_Explode_Think;
   ent->client->pers.AQ_SNum = artifact->aEnt->health = 3; // The time 'til explosion   
   AQ_Explode_Think (artifact->aEnt);   
}

void AQ_Big_Boom_Think (edict_t *ent)
{
   if (ent->health > 0)
   {
      switch (ent->health)
      {
         case 4: ent->s.skinnum = 0; break;
         case 3: ent->s.renderfx = RF_TRANSLUCENT; break;
         case 2: ent->s.renderfx = 0; ent->s.effects = EF_SPHERETRANS; break;
      }
      ent->health--;
      if (ent->health > 1) ent->s.frame++;
      else ent->s.frame = 0;
      ent->nextthink = level.time + FRAMETIME;
   }
   else // remain a bit to continue sound effect
   {
      G_FreeEdict(ent);
   }
}

// Makes a big silent explosion effect (for Divine Wind)
void AQ_Big_Boom (vec3_t location)
{
   edict_t *bomb;
   edict_t *light;

   // The actual bomb
   bomb = G_Spawn();
   VectorCopy (location, bomb->s.old_origin);
   VectorCopy (location, bomb->s.origin);
   if (random()*2 < 1)
      VectorSet (bomb->avelocity, 0, 100+random()*600, 0);
   else
      VectorSet (bomb->avelocity, 0, -100-random()*600, 0);
   bomb->movetype = MOVETYPE_FLY;
   bomb->solid = SOLID_NOT;
   bomb->think =  AQ_Big_Boom_Think;
   bomb->nextthink = level.time + FRAMETIME;
   bomb->classname = "aq_big_boom";
   bomb->s.modelindex = gi.modelindex ("models/artifact/divine/tris.md2");
   bomb->s.effects = 0;
   bomb->s.renderfx = 0;
   bomb->s.frame = 0;


   bomb->s.skinnum = 1;
   bomb->health = 5;
   gi.linkentity (bomb);

   // Semi-BOOM! sound
//   gi.sound (bomb, CHAN_AUTO, gi.soundindex("weapons/rocklx1a.wav"), 1, ATTN_NONE, 0);
   gi.sound (bomb, CHAN_AUTO, gi.soundindex("artifact/boom.wav"), 1, ATTN_NORM, 0);
//TODO: make people's screen flash

   // Not all effects flags mesh with each other, so this'll light up the explosion.
   light = G_Spawn();

   VectorCopy (location, light->s.old_origin);
   VectorCopy (location, light->s.origin);
   light->movetype = MOVETYPE_NONE;
   light->solid = SOLID_NOT;
   light->think =  G_FreeEdict;
   light->nextthink = level.time + 0.6;
   light->classname = "aq_big_boom_light";
   light->s.modelindex = gi.modelindex ("models/artifact/a_null/tris.md2");
   light->s.effects = EF_TAGTRAIL;
   light->health = 5;
   gi.linkentity (light);
}

// Death Spot blows up after a time
void AQ_Kill_Death_Spot (edict_t *ent)
{
   AQ_Held_Artifact *artifact = &(ent->owner->client->pers.AQ_Info[AQ_DEATH]);

   // Owner of a Death Spot is allocated another Death Spot
   if (artifact->on) artifact->aInt++;
   AQ_HUD_Num (ent->owner);

   // Make the explosion of death
   T_RadiusDamage (ent, ent->owner, 120, ent->owner, 120, MOD_AQ_DEATH_SPOT2);

   // Death Spot dies with an explosion effect
   gi.WriteByte (svc_temp_entity);
   gi.WriteByte (TE_ROCKET_EXPLOSION);
   gi.WritePosition (ent->s.origin);
   gi.multicast (ent->s.origin, MULTICAST_PHS);

   G_FreeEdict (ent);
}

// Death Spot gets touched by something
void AQ_Death_Spot_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
   AQ_Held_Artifact *artifact = &(ent->owner->client->pers.AQ_Info[AQ_DEATH]);

   if (other == world) return;
   if (other->takedamage == DAMAGE_NO) return;

   // Owner of a Death Spot is allocated another Death Spot
   if (artifact->on) artifact->aInt++;
   AQ_HUD_Num (ent->owner);

   // Kill guy who touched
   T_Damage (other, ent, ent->owner, vec3_origin, ent->s.origin, vec3_origin, 8192, 0, 0, MOD_AQ_DEATH_SPOT);

   // Dissapear in a second. Make a sound
   gi.sound (ent, CHAN_AUTO, gi.soundindex ("artifact/buzzhorn.wav"), 1, ATTN_NORM, 0);
   ent->s.modelindex = gi.modelindex ("models/artifact/a_null/tris.md2");
   ent->classname = "aq_dead_death_spot";
   ent->think = G_FreeEdict;
   ent->touch = NULL;
   ent->nextthink = level.time + 1;
}

// Death Spot arms itself
void AQ_Arm_Death_Spot (edict_t *ent)
{
   VectorClear(ent->avelocity);
   ent->solid = SOLID_TRIGGER;
   ent->touch = AQ_Death_Spot_Touch;
   ent->think = AQ_Kill_Death_Spot;
   ent->nextthink = level.time + 120;
   ent->s.effects = EF_SPHERETRANS;
   ent->s.sound = gi.soundindex ("artifact/inv3.wav");

   gi.sound (ent, CHAN_AUTO, gi.soundindex ("artifact/inv1.wav"), 1, ATTN_NORM, 0);
}

// Spawn a Death Spot
void AQ_Death_Spot (edict_t *ent)
{
   AQ_Held_Artifact *artifact = &(ent->client->pers.AQ_Info[AQ_DEATH]);
   edict_t *spot;

   if (artifact->aInt < 1)
   {
      gi.centerprintf (ent, "Ž No Death Spots left, you masochist. Ž");
      return;
   }

   //  Don't let people lay these too close to a spawn point or teleport dest. That's just impolite.
   if (AQ_Nearest_DMSpawn(ent->s.origin) < 100)
   {
      gi.centerprintf (ent, "Can't drop a Death Spot.\nŽ Too close to spawn point. Ž\nCheapass!");
      return;
   }
   if (AQ_Nearest_TeleDest(ent->s.origin) < 100)
   {
      gi.centerprintf (ent, "Can't drop a Death Spot.\nŽ Too close to teleport destination. Ž\nCheapass!");
      return;
   }

   // Owner of Death Spot is deallocated a Death Spot
   artifact->aInt--;
   ent->client->pers.AQ_SNum = artifact->aInt;

   // Spawn the Death Spot
   spot = G_Spawn();
   VectorCopy (ent->s.origin, spot->s.old_origin);
   VectorCopy (ent->s.origin, spot->s.origin);
   VectorSet (spot->avelocity, random()*720-360, random()*720-360, random()*720-360);
   VectorSet (spot->mins, -10, -10, -10);
   VectorSet (spot->maxs, 10, 10, 10);
   VectorClear (spot->velocity);
   spot->movetype = MOVETYPE_FLY;
   spot->solid = SOLID_NOT;
   spot->owner = ent;
   spot->touch = NULL;
   spot->think = AQ_Arm_Death_Spot;
   spot->nextthink = level.time + 5;
   spot->classname = "aq_death_spot";
   spot->s.modelindex = gi.modelindex ("models/artifact/dspot/tris.md2");
   spot->s.effects = EF_TRACKERTRAIL;
   spot->s.origin[2] += 10;
   gi.linkentity (spot);

   // Hurt the owner
   T_Damage(ent, ent, ent, vec3_origin, ent->s.origin, vec3_origin, 10, 0, 0, MOD_AQ_DEATH_SPOT3);

   gi.sound (spot, CHAN_AUTO, gi.soundindex ("weapons/grenlr1b.wav"), 1, ATTN_NORM, 0);
}

// Ice Trap finally dies without ever being touched
void AQ_Kill_Ice_Trap (edict_t *ent)
{
   AQ_Held_Artifact *artifact = &(ent->owner->client->pers.AQ_Info[AQ_FREEZE]);

   // Owner of Ice Trap gets allocated another Ice Trap
   if (artifact->on) artifact->aInt++;
   AQ_HUD_Num (ent->owner);

   G_FreeEdict(ent);
}

// If the Ice Trap touches the world while guy is frozen, stop moving
void AQ_Ice_Trap_Frozen_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
   if (other == world)
      VectorClear(ent->velocity);
   if (other->svflags & SVF_MONSTER)
      VectorClear(other->velocity);
}

// Ice Trap keeps a guy frozen
void AQ_Ice_Trap_Frozen (edict_t *ent)
{
   int n;
   ent->nextthink = level.time + FRAMETIME;

   // If delay is over, or guy is dead, free 'im.
   if (ent->delay <= level.time || ent->enemy->deadflag || !ent->enemy || ent->health < 1)
   {
      // Restore some data
      if (ent->enemy)
      {
         VectorCopy(ent->pos2, ent->enemy->velocity); // Restore the old velocity
         if (ent->enemy->client) // If a client...
            ent->enemy->client->pers.AQ_State &= ~AQ_STATE_FROZEN;
      }

      // Getting rid of the Ice Trap entity
      ent->touch = NULL;
      ent->solid = SOLID_NOT;
      ent->movetype = MOVETYPE_NONE;
      ent->s.frame = 0;
      ent->s.modelindex = gi.modelindex ("models/artifact/a_null/tris.md2");
      ent->classname = "aq_dead_ice_trap";
      ent->think = G_FreeEdict;
      ent->nextthink = level.time + 1;
      gi.sound (ent, CHAN_AUTO, gi.soundindex ("world/brkglas.wav"), 1, ATTN_NORM, 0);
      return;
   }

   // Make guy trapped in ice trap STAY there
   ent->enemy->s.frame = ent->health;
   VectorCopy(ent->s.origin, ent->enemy->s.origin);
   VectorCopy(ent->s.old_origin, ent->enemy->s.old_origin);
   VectorCopy(ent->s.angles, ent->enemy->s.angles);
   VectorCopy(ent->velocity, ent->enemy->velocity);
   // Things to specifically do to monsters
   if (ent->enemy->svflags & SVF_MONSTER)
   {
      ent->enemy->monsterinfo.nextframe = ent->health; //TODO: Why doesn't this freeze monsters?!
   }
   // Things to specifically do to clients
   if (ent->enemy->client)
   {
      VectorSet(ent->enemy->client->damage_blend, 0.2, 0.5, 1);
      ent->enemy->client->damage_alpha = 0.3;
   }

   // Handling the visual effects of the Ice Trap
   if (ent->s.frame < 4) ent->s.frame++; // Increase the frame
   else if (ent->delay - level.time == 9) // Deal with rendered effects
   {
      ent->s.effects &= ~EF_COLOR_SHELL;
      ent->s.renderfx &= ~RF_SHELL_BLUE;
   }
   else if (ent->delay - level.time == 6)
      ent->s.renderfx |= RF_TRANSLUCENT;
   else if (ent->delay - level.time == 3)
   {
      ent->s.renderfx &= ~RF_TRANSLUCENT;
      ent->s.effects |= EF_SPHERETRANS;
   }
}

// Something touches an Ice Trap
void AQ_Ice_Trap_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
   AQ_Held_Artifact *artifact = &(ent->owner->client->pers.AQ_Info[AQ_FREEZE]);
   vec3_t            padding;

   if (other == world) return;
   if (other->takedamage == DAMAGE_NO) return;
   if (other->client)
      if (other->client->pers.AQ_State & AQ_STATE_FROZEN) return;

   // Owner of Ice Trap gets allocated another Ice Trap
   if (artifact->on) artifact->aInt++;
   AQ_HUD_Num (ent->owner);

   // If the guy has EPA, just kill the Ice Trap
   if (other->client && other->client->pers.AQ_Info[AQ_EPA].on)
   {
      ent->touch = NULL;
      ent->solid = SOLID_NOT;
      ent->movetype = MOVETYPE_NONE;
      ent->s.frame = 0;
      ent->s.modelindex = gi.modelindex ("models/artifact/a_null/tris.md2");
      ent->classname = "aq_dead_ice_trap";
      ent->think = G_FreeEdict;
      ent->nextthink = level.time + 1;
      gi.sound (ent, CHAN_AUTO, gi.soundindex ("world/brkglas.wav"), 1, ATTN_NORM, 0);
      return;
   }

   // Do stuff for personal appearence
   ent->enemy = other;
   ent->touch = AQ_Ice_Trap_Frozen_Touch;
   ent->think = AQ_Ice_Trap_Frozen;
   ent->nextthink = level.time;
   ent->delay = level.time + 12;
   ent->s.modelindex = gi.modelindex ("models/artifact/ice/tris.md2");
   ent->s.effects = EF_BLUEHYPERBLASTER|EF_COLOR_SHELL;
   ent->s.renderfx = RF_SHELL_BLUE;
   VectorCopy (ent->enemy->s.origin, ent->s.origin);
   VectorCopy (ent->enemy->s.origin, ent->s.old_origin);

   // Make the Ice Trap nice and large so people don't get stuck in walls
   VectorCopy (other->mins, ent->mins);
   VectorCopy (other->maxs, ent->maxs);
   VectorSet (padding, 2, 2, 2);
   VectorSubtract(padding, ent->mins, ent->mins);
   VectorAdd(padding, ent->maxs, ent->maxs);

   // Store data of guy trapped
   ent->health = other->s.frame;                // health holds frame number
   VectorCopy (other->s.origin, ent->pos1);      // pos1 holds origin
   VectorCopy (other->velocity, ent->pos2);      // pos2 holds velocity
   VectorCopy (other->s.angles, ent->s.angles);  // angles holds angles
   // Store client data, if the trapped guy is a client
   if (other->client)
   {
      VectorCopy(other->client->ps.viewangles, ent->movedir); // movedir holds viewangles
      ent->max_health = ent->enemy->client->ps.gunindex; // max_health holds gun index
      other->client->pers.AQ_State |= AQ_STATE_FROZEN;
      // Tell the owner the good news and the victim the bad news :)
      if (other != ent->owner)
      {
         gi.centerprintf (other, "Ž Frozen by %s's Ice Trap Ž", ent->owner->client->pers.netname);
         gi.cprintf (ent->owner, PRINT_HIGH, "Ž %s is caught in your Ice Trap! Ž\n", other->client->pers.netname);
      } else
         gi.centerprintf (ent->owner, "Ž Frozen by your own damn Ice Trap! Ž");
   }

   gi.sound (ent, CHAN_AUTO, gi.soundindex ("world/airhiss1.wav"), 1, ATTN_NORM, 0);
}

// Ice Trap's sittin' around, occasionally giving off little particles
void AQ_Ice_Trap_Waiting (edict_t *ent)
{
   if (ent->delay <= level.time) { AQ_Kill_Ice_Trap (ent); return; }
   if (random()*2 < 1)
   {
      gi.WriteByte (svc_temp_entity);
      gi.WriteByte (TE_LASER_SPARKS);
      gi.WriteByte (5);
      gi.WritePosition (ent->s.origin);
      gi.WriteDir (vec3_origin);
      gi.WriteByte (176+(int)(random()*6));
      gi.multicast (ent->s.origin, MULTICAST_PVS);
   }
   ent->nextthink = level.time + 0.5;
}

// Ice Trap arms itself
void AQ_Arm_Ice_Trap (edict_t *ent)
{
   VectorClear(ent->avelocity);
   ent->solid = SOLID_TRIGGER;
   ent->touch = AQ_Ice_Trap_Touch;
   ent->think = AQ_Ice_Trap_Waiting;
   ent->delay = level.time + 120;
   ent->nextthink = level.time;
   ent->s.effects &= ~EF_BLUEHYPERBLASTER;
   ent->s.frame--;

   VectorCopy(ent->s.origin, ent->pos1); // Just for the ambient effect

   gi.sound (ent, CHAN_AUTO, gi.soundindex ("mutant/mutatck2.wav"), 1, ATTN_NORM, 0);
}

// Spawn an Ice Trap
void AQ_Ice_Trap (edict_t *ent)
{
   AQ_Held_Artifact *artifact = &(ent->client->pers.AQ_Info[AQ_FREEZE]);
   edict_t *trap;

   if (artifact->aInt < 1)
   {
      gi.centerprintf(ent, "Ž Damn you're cold! Ž\nŽ No more Ice Traps for you! Ž");
      return;
   }

   //  Don't let people lay these too close to a spawn point or teleport dest. That's just impolite.
   if (AQ_Nearest_DMSpawn(ent->s.origin) < 100)
   {
      gi.centerprintf (ent, "Can't drop an Ice Trap.\nŽ Too close to spawn point. Ž\nCheapass!");
      return;
   }
   if (AQ_Nearest_TeleDest(ent->s.origin) < 100)
   {
      gi.centerprintf (ent, "Can't drop an Ice Trap.\nŽ Too close to teleport destination. Ž\nCheapass!");
      return;
   }

   // Guy who owns Ice Trap is deallocated an Ice Trap
   artifact->aInt--;
   ent->client->pers.AQ_SNum = artifact->aInt;

   trap = G_Spawn();
   VectorCopy (ent->s.origin, trap->s.old_origin);
   VectorCopy (ent->s.origin, trap->s.origin);
   VectorSet (trap->avelocity, random()*720-360, random()*720-360, random()*720-360);
   VectorSet (trap->mins, -10, -10, -10);
   VectorSet (trap->maxs, 10, 10, 10);
   VectorClear (trap->velocity);
   trap->movetype = MOVETYPE_FLY;
   trap->solid = SOLID_NOT;
   trap->owner = ent;
   trap->touch = NULL;
   trap->think = AQ_Arm_Ice_Trap;
   trap->nextthink = level.time + 5;
   trap->classname = "aq_ice_trap";
   trap->s.frame = 1;
   trap->s.modelindex = gi.modelindex ("models/artifact/ice/tris.md2");
   trap->s.effects = EF_SPHERETRANS | EF_BLUEHYPERBLASTER;
   trap->s.origin[2] += 10;
   gi.linkentity (trap);

   gi.sound (trap, CHAN_AUTO, gi.soundindex ("misc/fhit3.wav"), 1, ATTN_NORM, 0);
}

// Flicker and die
void AQ_Holo_Flicker (edict_t *ent)
{
   if (random()*2 < 1)
   {
      ent->s.effects = EF_SPHERETRANS;
      ent->s.renderfx = 0;
   }
   else if (random()*2 < 1)
   {
      ent->s.effects = 0;
      ent->s.renderfx = RF_TRANSLUCENT;
   }
   else
   {
      ent->s.effects = 0;
      ent->s.renderfx = 0;
   }
   ent->s.effects |= EF_IONRIPPER;

   ent->nextthink = level.time + 0.1;

   if (ent->delay <= level.time)
      G_FreeEdict(ent);
}

// Follow the owner's movement
void AQ_Holo_Think (edict_t *ent)
{
   VectorCopy (ent->owner->s.angles, ent->s.angles);
   ent->s.angles[1] += ent->health;
   ent->s.frame = ent->owner->s.frame;
   ent->s.effects = ent->owner->s.effects;
   ent->s.renderfx = ent->owner->s.renderfx;
   ent->s.skinnum = ent->owner->s.skinnum;
   ent->s.modelindex = ent->owner->s.modelindex;
   ent->s.modelindex2 = ent->owner->s.modelindex2;

   ent->nextthink = level.time + FRAMETIME;

   // Flicker and die if time is up
   if (ent->delay <= level.time)
   {
       // Allocate owner another hologram
      if (ent->owner->client->pers.AQ_Info[AQ_HOLOGRAM].on) ent->owner->client->pers.AQ_Info[AQ_HOLOGRAM].aInt++;
      AQ_HUD_Num (ent->owner);
       // Start flickering
      ent->think = AQ_Holo_Flicker;
      ent->nextthink = level.time;
      ent->delay = level.time + 2;
      ent->classname = "aq_dead_hologram";
       // Sparking sound
      gi.sound (ent, CHAN_AUTO, gi.soundindex ("world/spark6.wav"), 1, ATTN_NORM, 0);
   }
}

// Spawn a Hologram
void AQ_Hologram (edict_t *ent)
{
   AQ_Held_Artifact *artifact = &(ent->client->pers.AQ_Info[AQ_HOLOGRAM]);
   edict_t *holo;

   if (artifact->aInt < 1)
   {
      gi.centerprintf(ent, "Ž No more fakes! Ž");
      return;
   }

   // Guy who owns Holo-Artfiact is deallocated a hologram
   artifact->aInt--;
   ent->client->pers.AQ_SNum = artifact->aInt;

   holo = G_Spawn();
   VectorCopy (ent->s.origin, holo->s.old_origin);
   VectorCopy (ent->s.origin, holo->s.origin);
   VectorClear (holo->mins);
   VectorClear (holo->maxs);
   holo->movetype = MOVETYPE_NONE;
   holo->solid = SOLID_NOT;
   holo->owner = ent;
   holo->touch = NULL;
   holo->think = AQ_Holo_Think;
   holo->nextthink = level.time;
   holo->classname = "aq_hologram";
   holo->delay = level.time + 30;
   holo->health = random()*360;
   holo->s.modelindex = ent->s.modelindex;
   holo->s.modelindex2 = ent->s.modelindex2;
   gi.linkentity (holo);

   gi.sound (holo, CHAN_AUTO, gi.soundindex ("misc/comp_up.wav"), 1, ATTN_NORM, 0);
}

// Does a super jump.
void AQ_Haste_Jump (edict_t *ent)
{
   AQ_Held_Artifact *artifact = &(ent->client->pers.AQ_Info[AQ_HASTE]);
   vec3_t forward;

   // If too soon, return.
   if (artifact->aTime > level.time) return;

   // If in air, return.
   if (!ent->groundentity && !ent->waterlevel) return;

   // Can't Haste-jump while ducking.
   if (ent->client->ps.pmove.pm_flags & PMF_DUCKED) return;

   AngleVectors (ent->client->v_angle, forward, NULL, NULL);
   VectorScale (forward, 1024, forward);

   // Don't stick on the ground
   ent->groundentity = NULL;

   VectorAdd (ent->velocity, forward, ent->velocity);
   artifact->aFloat = level.time + 3; // How long immune to falling damage.
   artifact->aTime = level.time + 0.7; // How long until you can "Haste jump" again.

   gi.sound (ent, CHAN_VOICE, gi.soundindex ("artifact/bionic.wav"), 1, ATTN_NORM, 0);
}

// Thinking func for the blobby shield thing
void AQ_Shield_Orb_Think (edict_t *ent)
{
   VectorCopy (ent->owner->s.origin, ent->s.origin);
   if (ent->s.frame == 0)
   {
      if (random()*2 < 1)
         ent->s.frame = 1;
      else
         ent->s.frame = 2;
   }
   else if (ent->s.frame == 1)
   {
      if (random()*2 < 1)
         ent->s.frame = 0;
      else
         ent->s.frame = 2;
   }
   else // s.frame == 2
   {
      if (random()*2 < 1)
         ent->s.frame = 0;
      else
         ent->s.frame = 1;
   }
   ent->nextthink = level.time + FRAMETIME;

   if (ent->owner->client)
   {
      VectorSet(ent->owner->client->damage_blend, 1, 1, 0);
      ent->owner->client->damage_alpha = 0.3;
   }
}

// Spawn the invincible shield
void AQ_Shield_Spawn (edict_t *ent)
{
   AQ_Held_Artifact *artifact = &(ent->client->pers.AQ_Info[AQ_SHIELD]);
   edict_t *shield;

   // Spawn the shield
   shield = G_Spawn();
   VectorCopy (ent->s.origin, shield->s.old_origin);
   VectorCopy (ent->s.origin, shield->s.origin);
   VectorSet (shield->s.angles, random()*360, random()*360, random()*360);
   VectorSet (shield->avelocity, 32-random()*32, 32-random()*64, 32-random()*64);
   VectorClear (shield->mins);
   VectorClear (shield->maxs);
   shield->movetype = MOVETYPE_FLY;
   shield->solid = SOLID_NOT;
   shield->owner = ent;
   shield->touch = NULL;
   shield->think = AQ_Shield_Orb_Think;
   shield->nextthink = level.time;
   shield->classname = "aq_shield";
   shield->delay = level.time + 3; // The time the player is invisible.
   shield->s.modelindex = gi.modelindex ("models/artifact/shield/tris.md2");
   shield->s.renderfx = RF_TRANSLUCENT;
   gi.linkentity (shield);

   artifact->aEnt = shield;
}

// Called when you unequip, die, or run out of time
void AQ_Shield_Deactivate (edict_t *ent)
{
   AQ_Held_Artifact *artifact = &(ent->client->pers.AQ_Info[AQ_SHIELD]);

   if (!artifact->aInt) return; // If not currently invincible, just return

   // Take damage again
   if (!ent->deadflag)
      ent->takedamage = DAMAGE_AIM;

   // You're no longer invincible
   artifact->aInt = 0;
   artifact->pTime = level.time + 3;

   // Kill the bubble sphere around you
   G_FreeEdict(artifact->aEnt);
}

// Called when you press The Button on the shield.
void AQ_Shield_Activate (edict_t *ent)
{
   AQ_Held_Artifact *artifact = &(ent->client->pers.AQ_Info[AQ_SHIELD]);

   if (artifact->aInt) return; // If invincible, return.
   if (artifact->pTime > level.time) // If not yet time to reactivate, return.
      { gi.centerprintf (ent, "Ž Recharging. Ž"); return; }

   // Here's the shield orb!
   AQ_Shield_Spawn (ent);

   // No longer take damage
   ent->takedamage = DAMAGE_NO;

   // How long you'll be invincible and the fact that you are
   artifact-> aInt = 1;
   artifact-> pTime = level.time + 3;

   // Invincible sound
   gi.sound (ent, CHAN_AUTO, gi.soundindex ("items/protect2.wav"), 1, ATTN_NORM, 0);
}

// The infamous Enviro-Jump
void AQ_EPA_Jump (edict_t *ent)
{
   AQ_Held_Artifact *artifact = &(ent->client->pers.AQ_Info[AQ_EPA]);
   vec3_t forward;

   // If too soon, return.
   if (artifact->aTime > level.time) return;

   // If not in liquid
   if (!ent->waterlevel) return;

   // Can't Enviro-jump while ducking.
   if (ent->client->ps.pmove.pm_flags & PMF_DUCKED) return;

   AngleVectors (ent->client->v_angle, forward, NULL, NULL);
   VectorScale (forward, 1024, forward);

   VectorAdd (ent->velocity, forward, ent->velocity);
   artifact->aTime = level.time + 0.5; // How long until you can Enviro-jump again.

   gi.sound (ent, CHAN_VOICE, gi.soundindex ("artifact/bionic.wav"), 1, ATTN_NORM, 0);
}

// Just toggles Orb modes and gives a little message. :)
void AQ_Toggle_Orb (edict_t *ent)
{
   AQ_Held_Artifact *artifact = &(ent->client->pers.AQ_Info[AQ_ORB]);
   if (artifact->aInt == 1)
   {
      artifact->aInt = 0;
      gi.centerprintf (ent, "Ž Orb: Locked orbit mode Ž"); return;
   }
   else
   {
      artifact->aInt = 1;
      gi.centerprintf (ent, "Ž Orb: Liberated orbit mode Ž"); return;
   }
}

// Brings your Tank Helper back to yourself
void AQ_Teleport_Tank (edict_t *ent)
{
   AQ_Held_Artifact *artifact = &(ent->client->pers.AQ_Info[AQ_HELPER]);
   edict_t *tank = artifact->aEnt;

   gi.unlinkentity (tank); // Why am I unlinking?
   AQ_TeleTempEnt(tank->s.origin); // Faking a temp entity
    // Redo angles and position
   VectorCopy(ent->s.origin, tank->s.origin);
   VectorCopy(ent->s.origin, tank->s.old_origin);
   VectorCopy(ent->s.angles, tank->s.angles);
   tank->s.angles[0] = 0;
   tank->s.event = EV_PLAYER_TELEPORT; // Teleport thingy in new area!
   gi.linkentity (tank);
}

// Think function of spinning cannister splitting open
void AQ_Cannister_Think (edict_t *ent)
{
   ent->nextthink = level.time + FRAMETIME;

   if (ent->health > 1)
   {
      ent->health--;
      ent->s.frame++;
   }
   else
      G_FreeEdict(ent);
}

// Damage players in view
void AQ_Radio_Think (edict_t *ent)
{
   AQ_Held_Artifact *artifact = &(ent->owner->client->pers.AQ_Info[AQ_RADIO]);
   edict_t *victim = NULL;
   vec3_t normal;

   // Free the spot if time is up
   if (ent->delay < level.time)
   {
      // Owner of Radioactive gets allocated another point
      if (artifact->on) artifact->aInt++;
      AQ_HUD_Num (ent->owner);

      gi.WriteByte (svc_temp_entity);
      gi.WriteByte (TE_BFG_EXPLOSION);
      gi.WritePosition (ent->s.origin);
      gi.multicast (ent->s.origin, MULTICAST_PHS);

      G_FreeEdict (ent);
      return;
   }

   while ((victim = findradius(victim, ent->s.origin, 2048)) != NULL)
   {
      // Damaging people with radiation
	if (victim->takedamage && visible (ent, victim))
      {
         VectorSubtract(ent->s.origin, victim->s.origin, normal);
         VectorNormalize(normal);
         // Give owner health (if radio thing is equipped)
         if (victim == ent->owner && artifact->on)
         {
            if (victim->health < victim->max_health)
            {
               victim->health += 1;
               gi.cprintf (victim, PRINT_HIGH, "Ž Receiving radioactive POWER! Ž\n");
            }
         } else // Damage everybody else!
         {
            T_Damage (victim, ent, ent->owner, vec3_origin, victim->s.origin, normal, 5, 0, 0, MOD_AQ_RADIOACTIVE);
            if (victim->client && !victim->client->pers.AQ_Info[AQ_EPA].on)
               gi.cprintf (victim, PRINT_HIGH, "Ž This area is radioactive! Ž\n");

         }
      }
   }
   ent->nextthink = level.time + 0.8;
}

// Creates the radiation point
void AQ_Spawn_Radio (edict_t *ent)
{
   edict_t *flashvictim = NULL;
   edict_t *radio;

   ent->classname = "aq_radio_cannister";
   ent->think = AQ_Cannister_Think;
   ent->nextthink = level.time;

   radio = G_Spawn();
   VectorSet (radio->s.angles, random()*360, random()*360, random()*360);
   VectorCopy (ent->s.origin, radio->s.origin);
   VectorCopy (ent->s.origin, radio->s.old_origin);
   VectorClear (radio->mins);
   VectorClear (radio->maxs);
   radio->movetype = MOVETYPE_FLY;
   radio->solid = SOLID_TRIGGER; // Needs to be solid somehow to be detected in a trace
   radio->touch = NULL;
   radio->owner = ent->owner;
   radio->touch = NULL;
   radio->think = AQ_Radio_Think;
   radio->nextthink = level.time;
   radio->classname = "aq_radio_point";
   radio->s.effects = EF_BFG;
   radio->s.modelindex = gi.modelindex ("models/artifact/glow/tris.md2");
   radio->s.sound = gi.soundindex ("artifact/gcount.wav");
   radio->delay = level.time + 45; // Duration
   radio->s.origin[2] -= 5;
   VectorSet (radio->avelocity, 360-random()*720, random()*720, random()*720);
   gi.linkentity (radio);

   // Make flash for people looking this way
   while ((flashvictim = findradius(flashvictim, ent->s.origin, 2048)) != NULL)
   {
      if (flashvictim->client && infront (flashvictim, ent) && visible(flashvictim, ent))
      {
         VectorSet(flashvictim->client->damage_blend, 1, 1, 1);
         flashvictim->client->damage_alpha = 1;
      }
   }

   // Radiation point activation sound
   gi.sound (ent, CHAN_AUTO, gi.soundindex ("weapons/noammo.wav"), 1, ATTN_NORM, 0);
   //TODO: better sound?
}

// Checks to see if a radiation point is in view of specified point
int AQ_Radio_InView (edict_t *ent)
{
   edict_t *spot = NULL;
   int found = 0;

   while ((spot = findradius(spot, ent->s.origin, 2048)) != NULL && !found)
      if (strcmp(spot->classname, "aq_radio_point") == 0 && visible(ent, spot))
         found++;

   return found;
}

// Spawns the initial edict that'll "become" a radiation point
void AQ_Spawn_Radio_Cannister (edict_t *ent)
{
   AQ_Held_Artifact *artifact = &(ent->client->pers.AQ_Info[AQ_RADIO]);
   edict_t *rcannister;

   if (artifact->aInt < 1)
   {
      if (!ent->deadflag) gi.centerprintf (ent, "Ž You are not an atomic playboy. Ž\nŽ Only three radiation points allowed. Ž");
      return;
   }

   if (AQ_Radio_InView (ent) && !ent->deadflag) // If a point is already in view and player isn't dead
   {
      gi.centerprintf (ent, "Ž This area is already irradiated. Ž");
      return;
   }

   // Owner of Radioactive gets deallocated another point
   artifact->aInt--;
   AQ_HUD_Num (ent);

   rcannister = G_Spawn();
   VectorCopy (ent->s.origin, rcannister->s.origin);
   VectorCopy (ent->s.origin, rcannister->s.old_origin);
   VectorClear (rcannister->mins);
   VectorClear (rcannister->maxs);
   rcannister->movetype = MOVETYPE_NONE;
   rcannister->solid = SOLID_TRIGGER; // Needs to be solid somehow to be detected in a trace
   rcannister->touch = NULL;
   rcannister->owner = ent;
   rcannister->think = AQ_Spawn_Radio;
   rcannister->nextthink = level.time + 5;
   rcannister->classname = "aq_radio_point";
   rcannister->s.effects = EF_SPINNINGLIGHTS;
   rcannister->s.modelindex = gi.modelindex ("models/artifact/can/tris.md2");
   rcannister->s.frame = 0;
   rcannister->s.origin[2] += 10;
   rcannister->health = 13; // Number of frames.
   gi.linkentity (rcannister);

   if (!ent->deadflag) gi.centerprintf (ent, "Ž 5 seconds to reach Ž\nŽ minimum safe distance. Ž");

   // Radiation point setting sound
   gi.sound (rcannister, CHAN_AUTO, gi.soundindex ("artifact/clank.wav"), 1, ATTN_NORM, 0);
}

// Changes the "speed" setting on flying and sends a message to the player
void AQ_Change_Flight_Speed (edict_t *ent)
{
   AQ_Held_Artifact *artifact = &(ent->client->pers.AQ_Info[AQ_FLIGHT]);

   // Can only change once a second
   if (artifact->pTime > level.time) return;

   // Shift from Regular -> Speedy -> Drifty -> Regular -> etc
   if (artifact->aInt == 0)
   {
      ent->client->pers.AQ_State |= AQ_STATE_NOSHOOT;
      artifact->aInt = 1;
      artifact->aFloat = 1000;
      gi.cprintf (ent, PRINT_HIGH, "Ž Speedy Mode Ž\n");
   } else
   if (artifact->aInt == 1)
   {
      ent->client->pers.AQ_State &= ~AQ_STATE_NOSHOOT;
      artifact->aEnt->s.sound = 0;
      artifact->aInt = 2;
      artifact->aFloat = 80;
      gi.cprintf (ent, PRINT_HIGH, "Ž Drifty Mode Ž\n");
   } else
   // artifact->aInt == 2
   {
      ent->client->pers.AQ_State &= ~AQ_STATE_NOSHOOT;
      artifact->aEnt->s.sound = gi.soundindex ("floater/fltsrch1.wav");
      artifact->aInt = 0;
      artifact->aFloat = 350;
      gi.cprintf (ent, PRINT_HIGH, "Ž Regular Flight Ž\n");
   }

   artifact->pTime = level.time + 1;
}

//TODO: put this in the prototype section
edict_t* AQ_Bubble_Spawn (edict_t *ent);

// Pushes off a weird bubble
void AQ_Bubble_Push (edict_t *ent)
{
   AQ_Held_Artifact *artifact = &(ent->client->pers.AQ_Info[AQ_WEIRD]);
   edict_t* newBubble;
   vec3_t forward;
   
   // pTime's being used to measure random bubble pushing, so aTime will be used to measure how often you can shoot them
   if (artifact->aTime > level.time) return;

   // Spawn a bubble, but push it off a bit
   newBubble = AQ_Bubble_Spawn (ent);
   AngleVectors (ent->client->v_angle, forward, NULL, NULL);
   VectorScale (forward, 130, forward);
   gi.unlinkentity (newBubble);
   VectorCopy (forward, newBubble->velocity);
   newBubble->nextthink = level.time + 0.7;   
   newBubble->s.origin[2] += 20;
   gi.linkentity (newBubble);

   artifact->aTime = level.time + 1;
   artifact->pTime = level.time + 2;
}

// Judges whether or not the specific location can fit ent
int AQ_IsGoodSpot (vec3_t pos, edict_t *ent)
{
   int    isGood;
   vec3_t locCheck;

   // Is it solid?
   isGood = 1;
   if (!(gi.pointcontents (pos) & MASK_MONSTERSOLID))
   {
      locCheck[0] = pos[0] + ent->mins[0];
      locCheck[1] = pos[1] + ent->mins[1];
      locCheck[2] = pos[2] + ent->mins[2];
      if (gi.pointcontents (locCheck) & MASK_MONSTERSOLID) isGood = 0;
      locCheck[0] = pos[0] + ent->mins[0];
      locCheck[1] = pos[1] + ent->mins[1];
      locCheck[2] = pos[2] + ent->maxs[2];
      if (gi.pointcontents (locCheck) & MASK_MONSTERSOLID) isGood = 0;
      locCheck[0] = pos[0] + ent->mins[0];
      locCheck[1] = pos[1] + ent->maxs[1];
      locCheck[2] = pos[2] + ent->mins[2];
      if (gi.pointcontents (locCheck) & MASK_MONSTERSOLID) isGood = 0;
      locCheck[0] = pos[0] + ent->mins[0];
      locCheck[1] = pos[1] + ent->maxs[1];
      locCheck[2] = pos[2] + ent->maxs[2];
      if (gi.pointcontents (locCheck) & MASK_MONSTERSOLID) isGood = 0;
      locCheck[0] = pos[0] + ent->maxs[0];
      locCheck[1] = pos[1] + ent->mins[1];
      locCheck[2] = pos[2] + ent->mins[2];
      if (gi.pointcontents (locCheck) & MASK_MONSTERSOLID) isGood = 0;
      locCheck[0] = pos[0] + ent->maxs[0];
      locCheck[1] = pos[1] + ent->mins[1];
      locCheck[2] = pos[2] + ent->maxs[2];
      if (gi.pointcontents (locCheck) & MASK_MONSTERSOLID) isGood = 0;
      locCheck[0] = pos[0] + ent->maxs[0];
      locCheck[1] = pos[1] + ent->maxs[1];
      locCheck[2] = pos[2] + ent->mins[2];
      if (gi.pointcontents (locCheck) & MASK_MONSTERSOLID) isGood = 0;
      locCheck[0] = pos[0] + ent->maxs[0];
      locCheck[1] = pos[1] + ent->maxs[1];
      locCheck[2] = pos[2] + ent->maxs[2];
      if (gi.pointcontents (locCheck) & MASK_MONSTERSOLID) isGood = 0;
   }
   else 
      isGood = 0; 

   return isGood;
}

// Locates a valid random location in the level to blink to
vec3_t* AQ_BlinkLoc (edict_t *ent)
{
   vec3_t gotoLoc;
   int n, isGood;
   
   // Find a location
   for (n=0;n<1000;n++)
   {
      // Find the location
      VectorCopy (ent->s.origin, gotoLoc);
      gotoLoc[0] += 1024-random()*2048;
      gotoLoc[1] += 1024-random()*2048;
      gotoLoc[2] += 1024-random()*2048;

      if (AQ_IsGoodSpot(gotoLoc, ent)) n = 1000;
   }
   
   if (!isGood) return NULL; else return &gotoLoc;
}

// Takes player to a random location
void AQ_Blink (edict_t *ent)
{
   AQ_Held_Artifact *artifact = &(ent->client->pers.AQ_Info[AQ_BLINK]);
   vec3_t* gotoLoc;
   edict_t *popper;
   vec3_t temp;

   // Only blink so often
   if (artifact->pTime > level.time) return;

   // Can only blink if a value is found
   gotoLoc = AQ_BlinkLoc(ent);
   if (!gotoLoc) return;

   // Actually move the player
   VectorCopy (ent->s.origin, temp);
   VectorCopy ((*gotoLoc), ent->s.origin);
   VectorCopy ((*gotoLoc), ent->s.old_origin);
   VectorClear (ent->velocity);

   // ALL just to make a little "pop" sound
   popper = G_Spawn();
   VectorCopy (temp, popper->s.origin);
   VectorCopy (temp, popper->s.old_origin);
   popper->movetype = MOVETYPE_NONE;
   popper->solid = SOLID_NOT;
   popper->touch = NULL;
   popper->owner = ent;
   popper->think = G_FreeEdict;
   popper->nextthink = level.time + 0.3;
   popper->classname = "aq_sound_popper";
   popper->s.modelindex = gi.modelindex ("models/artifact/a_null/tris.md2");
   gi.linkentity (popper);
   gi.sound (popper, CHAN_AUTO, gi.soundindex ("artifact/pop.wav"), 1, ATTN_NORM, 0);

   artifact->pTime = level.time + 0.2;
   artifact->aTime = level.time + 2; // aTime is for falling damage prevention
}

// Do the teleport shuffle thing
void AQ_Shuffle (edict_t *ent)
{
   AQ_Held_Artifact *artifact = &(ent->client->pers.AQ_Info[AQ_TELE]);
   vec3_t forward, aBit, gotoLoc, diff;
   trace_t trace;
   edict_t *laser;
   int stillBad = 1;
   int count = 0;

   // Only shuffle so often
   if (artifact->pTime > level.time) return;

   VectorClear (aBit);
   AngleVectors (ent->client->v_angle, forward, NULL, NULL);
   VectorScale(forward, 10, aBit);
   VectorScale(forward, 2048, forward);
   VectorAdd(forward, ent->s.origin, forward);

   // Trace to wall
   trace = gi.trace (ent->s.origin, vec3_origin, vec3_origin, forward, ent, MASK_SOLID);
   VectorCopy (trace.endpos, gotoLoc);   

   // Move back along the trace until a valid teleport-to location is found
   while (stillBad)
   {
      VectorSubtract (ent->s.origin, gotoLoc, diff);

      // If we traced all the way back to the player..
      if (VectorLength (diff) <= 20)
      {
         stillBad = 0;
         VectorCopy (ent->s.origin, gotoLoc);
      }
      else // Keep tracing
      {
         // If the current location found is valid to teleport to..
         if (AQ_IsGoodSpot (gotoLoc, ent))
            stillBad = 0;
         else // Still keep tracing
         {
            VectorSubtract (gotoLoc, aBit, gotoLoc);
            count ++;
         }

         // Prevention of infinite loops. TODO: need this?
         if (count++ > 200)
         {
            stillBad = 0;
            VectorCopy (ent->s.origin, gotoLoc);
         }
      }
   }

   // If a location was found, teleport to it!
   if (!(VectorCompare (ent->s.origin, gotoLoc)))
   {
      // Spawn the shuffle laser
      laser = G_Spawn();
      VectorCopy (ent->s.origin, laser->s.old_origin);
      VectorCopy (ent->s.origin, laser->s.origin);
      laser->movetype = MOVETYPE_FLY;
      laser->solid = SOLID_NOT;
      laser->touch = NULL;
      laser->owner = ent;
      laser->think = AQ_LaserThinkGo;
      laser->nextthink = level.time + FRAMETIME;
      laser->classname = "aq_shuffle_laser";
      laser->s.modelindex = 1;
      laser->s.frame = 16;
      laser->s.renderfx = RF_BEAM|RF_TRANSLUCENT;
      laser->s.skinnum = 0xf3f3f1f1;
      gi.linkentity (laser);

      AQ_TeleTempEnt(gotoLoc); // Faking a temp entity
      ent->s.event = EV_PLAYER_TELEPORT;

      VectorCopy (gotoLoc, ent->s.origin);
      VectorCopy (gotoLoc, ent->s.old_origin);

      artifact->pTime = level.time + 0.2;
   }
}

// Make the laser move then dissapear
void AQ_LaserThinkGo (edict_t *ent)
{
   VectorCopy (ent->owner->s.origin, ent->s.origin);
   ent->think = G_FreeEdict;
   ent->nextthink = level.time + FRAMETIME;
}





