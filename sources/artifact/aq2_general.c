
// **************************
// Artifact Quake 2 main code
// **************************

// "Artifact" and "Artifact Quake 2" are (c) Copyright 2000 Nick "Singe" Bousman and Glenn "Griphis" Saaiman

// Do not modify and redistribute any AQ2 materials without Singe's or Griphis's permission.
// Do not modify and redistribute this source without providing publically the modified source.
// Do not modify and redistribute this code without giving Singe approrpiate credit!
// Do not modify and redistribute pictures or models from this patch without giving Griphis appropriate credit!

// Some code was inspired by tutorials and code of others. More about this is in the artifact.txt that comes with this patch.

// This file contains all the general function calls used by all artifacts, users, and the system itself to handle the artifacts.

// **************************************************************************

#include "g_local.h"

// Instantiate variables needed.
cvar_t *AQ_PlayerMax;
cvar_t *AQ_Mortality;
cvar_t *AQ_MaxArtifacts;
cvar_t *AQ_SpawnAllAtOnce;
cvar_t *AQ_MigrationTime;
cvar_t *AQ_TypeInvisible;
cvar_t *AQ_Droptime;
cvar_t *AQ_AllArtifactsEffect;
cvar_t *AQ_XRandom;

// Initialises AQ2 variables upon DLL load.
void AQ_GlobalInit()
{
   FILE *theFile;
   cvar_t *game = gi.cvar("game", "", 0);
   char *motd = &globals.AQ_motd;
   char inData[256];
   char fileName[MAX_QPATH];
   int n, limit;

   // No artifacts banned by default
   // No artifacts picked yet
   for (n=0; n<MAX_ARTIFACT_TYPES; n++)
      globals.AQ_Artifact_Banned[n] = globals.AQ_Artifact_Picked[n] = 0;

   // What's the banlist file?
   strcpy (fileName, game->string);
   strcat (fileName, "/aq2_banlist.txt");

   // Try to read banned artifacts from the banlist file
   if (theFile = fopen(fileName, "r"))
   {
      // While not at end of file
      while (fgets(inData, 256, theFile) != NULL)
      {
         // If the line isn't a comment, scan it
         if (inData[0] != '#')
         {
            // Cut off the trailing line feed
            n=-1; while (inData[++n] != '\n'); inData[n] = (char)0;
            // Compare with known artifact names and ban matching
            for (n=0; n<MAX_ARTIFACT_TYPES; n++)
               if (Q_stricmp(inData, AQ_SName_Of(n)) == 0) AQ_Ban(n);
         }
      }
      fclose(theFile);
   }

   // Make the "picked" list match the banlist. MUST be done regardless of XRandom being on or not
   for (n=0; n<MAX_ARTIFACT_TYPES; n++)
      globals.AQ_Artifact_Picked[n] = globals.AQ_Artifact_Banned[n];
   globals.AQ_Picked_Num = globals.AQ_Banned_Num;

   // Initialize the MOTD
   motd[0] = '\0';
   strcat(motd, "Αςτιζαγτ Ρυαλε ²\nv4.b (beta) \n\n");
   strcat(motd, "Authors: Σιξηε ¦ Ηςιπθισ\n");
   strcat(motd, "θττπΊ――σιξηε®τεμεζςαηηεδ®γον\n\n");
   strcat(motd, "Type \"αρ­νοτδ\" to see this again.\n");
   strcat(motd, "Type \"αρ­θεμπ\" for detailed info.\n");

   // What's the MOTD file?
   strcpy (fileName, game->string);
   strcat (fileName, "/aq2_motd.txt");

   // Try to read the motd
   if (theFile = fopen(fileName, "r"))
   {
      strcat(motd, "\n\n");
      // While not at end of file
      while ((fgets(inData, 80, theFile) != NULL)&&(strlen(motd) < 1024))
      {
         // If the line isn't a comment, scan it
         if (inData[0] != '#')
            strcat (motd, inData);
      }
      fclose(theFile);
   }

   // Get/set console variable defaults
   AQ_MaxArtifacts = gi.cvar ("aq_existmax", "16", CVAR_SERVERINFO|CVAR_LATCH);
   AQ_MigrationTime = gi.cvar ("aq_migrate", "256", CVAR_SERVERINFO|CVAR_LATCH);
   AQ_PlayerMax = gi.cvar ("aq_pickupmax", "1", CVAR_SERVERINFO|CVAR_LATCH);
   AQ_Mortality = gi.cvar ("aq_mortality", "1", CVAR_SERVERINFO|CVAR_LATCH);  //TODO: return this to 0, MAYBE
   AQ_SpawnAllAtOnce = gi.cvar ("aq_spawnall", "1", CVAR_SERVERINFO|CVAR_LATCH);
   AQ_TypeInvisible = gi.cvar ("aq_itype", "0", CVAR_SERVERINFO|CVAR_LATCH);
   AQ_Droptime = gi.cvar ("aq_droptime", "30", CVAR_SERVERINFO|CVAR_LATCH);
   AQ_AllArtifactsEffect = gi.cvar ("aq_allpassive", "1", CVAR_SERVERINFO|CVAR_LATCH);
   AQ_XRandom = gi.cvar ("aq_xrandom", "0", CVAR_SERVERINFO|CVAR_LATCH);
}

// Initializes stuff at the beginning of a level.
void AQ_LevelInit()
{
   int n, aIndex0;

   // TRUE randomization;
   srand(time(NULL));

   // Force console variables to be within set boundaries
   if ((float)AQ_MigrationTime->value < 0) gi.cvar_forceset (AQ_MigrationTime->name, "0");
   if ((float)AQ_MigrationTime->value < 8 && (float)AQ_MigrationTime->value > 0) gi.cvar_forceset (AQ_MigrationTime->name, "8");
   if ((int)AQ_MaxArtifacts->value < 1) gi.cvar_forceset (AQ_MaxArtifacts->name, "1");
   if ((int)AQ_MaxArtifacts->value > 256) gi.cvar_forceset (AQ_MaxArtifacts->name, "256");
   if ((int)AQ_PlayerMax->value > MAX_ARTIFACT_TYPES) gi.cvar_forceset (AQ_PlayerMax->name, "37"); // Remember, number needs changing
   if ((int)AQ_PlayerMax->value < 1) gi.cvar_forceset (AQ_PlayerMax->name, "1");                   // every time new artifact added!
   if ((int)AQ_Mortality->value > 2) gi.cvar_forceset (AQ_Mortality->name, "2");
   if ((int)AQ_Mortality->value < 0) gi.cvar_forceset (AQ_Mortality->name, "0");
   if ((int)AQ_SpawnAllAtOnce->value > 1) gi.cvar_forceset (AQ_SpawnAllAtOnce->name, "1");
   if ((int)AQ_SpawnAllAtOnce->value < 0) gi.cvar_forceset (AQ_SpawnAllAtOnce->name, "0");
   if ((int)AQ_TypeInvisible->value > 1) gi.cvar_forceset (AQ_TypeInvisible->name, "1");
   if ((int)AQ_TypeInvisible->value < 0) gi.cvar_forceset (AQ_TypeInvisible->name, "0");
   if ((int)AQ_Droptime->value < -1) gi.cvar_forceset (AQ_Droptime->name,"-1");
   if ((int)AQ_AllArtifactsEffect->value > 1) gi.cvar_forceset (AQ_AllArtifactsEffect->name, "1");
   if ((int)AQ_AllArtifactsEffect->value < 0) gi.cvar_forceset (AQ_AllArtifactsEffect->name, "0");
   if ((int)AQ_XRandom->value > 1) gi.cvar_forceset (AQ_XRandom->name, "1");
   if ((int)AQ_XRandom->value < 0) gi.cvar_forceset (AQ_XRandom->name, "0");

   // Assign big names in CS_GENERAL...
   for (n=0; n<MAX_ARTIFACT_TYPES; n++)
      gi.configstring (CS_GENERAL+n, AQ_Name_Of(n));

   // There are no artifacts in the level
   level.AQ_CurrentNum = 0;

   // Artifacts aren't in the level yet;
   level.AQ_Artifacts_In = 0;

   // Begin spawning artifacts in, say, 1 second
//   level.AQ_Artifact_Timer = level.time + 1;
   level.AQ_Artifact_Timer = level.time + 1024; // It's really big cause the FIRST client to appear will make the magic happen.

   // Remake the inventory data regarding artifacts
//   aIndex0 = ITEM_INDEX(FindItem ("Artifact Hook"));
//   for (n = aIndex0; n < aIndex0+MAX_ARTIFACT_TYPES; n++)
//   {
//      itemlist[n].quantity = n-aIndex0;
//      itemlist[n].icon = AQ_Icon_Of (n-aIndex0);
//      itemlist[n].pickup_name = AQ_SName_Of (n-aIndex0);
//   }   
//TODO: "Gamemap" won't work if I rename here. Find out WHERE I can rename to get it working.
// In the meantime, I have to put a whole bunch of shit manually into the g_items.c declaration
// of item types. This means that if I change artifact constant (ex, AQ_HASTE) values here, 
// icon names here, or short artifact names here, I must remember to change those there too.
}

// Things to do later at worldspawn. Only precacheing right now.
void AQ_WorldSpawn()
{
   int itype = (int)AQ_TypeInvisible->value;
   int n;

   // Precache things that're always used
   gi.soundindex ("artifact/tink3.wav");
   gi.soundindex ("flyer/flyatck1.wav");

   // Questionable things...
   gi.soundindex ("misc/tele1.wav");
   gi.soundindex ("weapons/grenlr1b.wav");
   gi.modelindex ("models/artifact/a_null/tris.md2");

   // Precache the generic pickup if itype is on
   if (itype) gi.modelindex ("models/pickup/artifact/pickup/a_artifact/tris.md2");

   // These precaches are based on the banlist, so we only load what we NEED to load.
   // Invisible type is counted for too, so we CAN save memory by not loading pickup models
   if (globals.AQ_Artifact_Banned[AQ_FLIGHT] == 0)
   {
      if (!itype) gi.modelindex ("models/artifact/pickup/a_flight/tris.md2");
      gi.soundindex ("floater/fltsrch1.wav");
   }
   if (globals.AQ_Artifact_Banned[AQ_GWELL] == 0)
   {
      if (!itype) gi.modelindex ("models/artifact/pickup/a_gwell/tris.md2");
      gi.modelindex ("models/artifact/gwell/tris.md2");
      gi.modelindex ("models/artifact/gwell2/tris.md2");
      gi.soundindex ("weapons/rockfly.wav");
   }
   if (globals.AQ_Artifact_Banned[AQ_RADIO] == 0)
   {
      if (!itype) gi.modelindex ("models/artifact/pickup/a_radio/tris.md2");
      gi.modelindex ("models/artifact/glow/tris.md2");
      gi.modelindex ("models/artifact/can/tris.md2");
      gi.soundindex ("artifact/gcount.wav");
      gi.soundindex ("artifact/clank.wav");
   }
   if (globals.AQ_Artifact_Banned[AQ_REGEN] == 0)
   {
      if (!itype) gi.modelindex ("models/artifact/pickup/a_regen/tris.md2");
      gi.soundindex ("artifact/heart.wav");
   }
   if (globals.AQ_Artifact_Banned[AQ_HEALTH] == 0)
   {
      if (!itype) gi.modelindex ("models/artifact/pickup/a_health/tris.md2");
   }
   if (globals.AQ_Artifact_Banned[AQ_CLOAK] == 0)
   {
      if (!itype) gi.modelindex ("models/artifact/pickup/a_cloak/tris.md2");
      gi.modelindex ("models/artifact/eyes/tris.md2");
      gi.soundindex ("artifact/inv1.wav");
   }
   if (globals.AQ_Artifact_Banned[AQ_RECALL] == 0)
   {
      if (!itype) gi.modelindex ("models/artifact/pickup/a_recall/tris.md2");
   }
   if (globals.AQ_Artifact_Banned[AQ_SHUB] == 0)
   {
      if (!itype) gi.modelindex ("models/artifact/pickup/a_shub/tris.md2");
      gi.modelindex ("models/artifact/shub/tris.md2");
   }
   if (globals.AQ_Artifact_Banned[AQ_SPIKY] == 0)
   {
      if (!itype) gi.modelindex ("models/artifact/pickup/a_spiky/tris.md2");
      gi.modelindex ("models/artifact/spiky/tris.md2");
      gi.modelindex ("models/artifact/spike/tris.md2");
   }
   if (globals.AQ_Artifact_Banned[AQ_FREEZE] == 0)
   {
      if (!itype) gi.modelindex ("models/artifact/pickup/a_freeze/tris.md2");
      gi.modelindex ("models/artifact/ice/tris.md2");
      gi.soundindex ("world/brkglas.wav");
      gi.soundindex ("world/airhiss1.wav");
      gi.soundindex ("mutant/mutatck2.wav");
      gi.soundindex ("misc/fhit3.wav");
   }
   if (globals.AQ_Artifact_Banned[AQ_DEATH] == 0)
   {
      if (!itype) gi.modelindex ("models/artifact/pickup/a_death/tris.md2");
      gi.modelindex ("models/artifact/dspot/tris.md2");
      gi.soundindex ("artifact/inv1.wav");
      gi.soundindex ("artifact/inv3.wav");
      gi.soundindex ("artifact/buzzhorn.wav");
   }
   if (globals.AQ_Artifact_Banned[AQ_CAMO] == 0)
   {
      if (!itype) gi.modelindex ("models/artifact/pickup/a_camo/tris.md2");
   }
   if (globals.AQ_Artifact_Banned[AQ_HASTE] == 0)
   {
      if (!itype) gi.modelindex ("models/artifact/pickup/a_haste/tris.md2");
      gi.soundindex ("artifact/godz.wav");
      gi.soundindex ("artifact/bionic.wav");
   }
   if (globals.AQ_Artifact_Banned[AQ_BERSERK] == 0)
   {
      if (!itype) gi.modelindex ("models/artifact/pickup/a_berserk/tris.md2");
   }
   if (globals.AQ_Artifact_Banned[AQ_ORB] == 0)
   {
      if (!itype) gi.modelindex ("models/artifact/pickup/a_orb/tris.md2");
      gi.modelindex ("models/artifact/orb/tris.md2");
      gi.modelindex ("models/artifact/orb2/tris.md2");
      gi.soundindex ("artifact/blipdrip.wav");
   }
   if (globals.AQ_Artifact_Banned[AQ_EPA] == 0)
   {
      if (!itype) gi.modelindex ("models/artifact/pickup/a_epa/tris.md2");
   }
   if (globals.AQ_Artifact_Banned[AQ_BUFFY] == 0)
   {
      if (!itype) gi.modelindex ("models/artifact/pickup/a_buffy/tris.md2");
      gi.soundindex ("artifact/wpain.wav");
   }
   if (globals.AQ_Artifact_Banned[AQ_HOLOGRAM] == 0)
   {
      if (!itype) gi.modelindex ("models/artifact/pickup/hologram/a_hologram/tris.md2");
      if (!itype) gi.modelindex ("models/artifact/pickup/hologram/tris.md2");
      gi.soundindex ("world/spark6.wav");
      gi.soundindex ("misc/comp_up.wav");
   }
   if (globals.AQ_Artifact_Banned[AQ_RESIST] == 0)
   {
      if (!itype) gi.modelindex ("models/artifact/pickup/a_resist/tris.md2");
      gi.soundindex ("items/protect3.wav");
   }
   if (globals.AQ_Artifact_Banned[AQ_STRENGTH] == 0)
   {
      if (!itype) gi.modelindex ("models/artifact/pickup/a_strength/tris.md2");
      gi.soundindex ("items/damage3.wav");
   }
   if (globals.AQ_Artifact_Banned[AQ_SHIELD] == 0)
   {
      if (!itype) gi.modelindex ("models/artifact/pickup/shield2/tris.md2");
      if (!itype) gi.modelindex ("models/artifact/pickup/shield2/a_shield/tris.md2");
      gi.modelindex ("models/artifact/shield/tris.md2");
      gi.soundindex ("items/protect2.wav");
   }
   if (globals.AQ_Artifact_Banned[AQ_HELPER] == 0)
   {
      if (!itype) gi.modelindex ("models/artifact/pickup/pokeball/tris.md2");
      if (!itype) gi.modelindex ("models/artifact/pickup/pokeball/tankemon/tris.md2");
//TODO: Problem, If I precache any Tank stuff, I get "refusing to download a path with .." twice on every level load
//      gi.modelindex ("models/monsters/tank/tris.md2");
//      gi.soundindex ("tank/tnkpain2.wav");
//      gi.soundindex ("tank/tnkdeth2.wav");
//      gi.soundindex ("tank/tnkidle1.wav");
//      gi.soundindex ("tank/death.wav");
//      gi.soundindex ("tank/step.wav");
//      gi.soundindex ("tank/tnkatck4.wav");
//      gi.soundindex ("tank/tnkatck5.wav");
//      gi.soundindex ("tank/sight1.wav");
//      gi.soundindex ("tank/tnkatck1.wav");
//      gi.soundindex ("tank/tnkatk2a.wav");
//      gi.soundindex ("tank/tnkatk2b.wav");
//      gi.soundindex ("tank/tnkatk2c.wav");
//      gi.soundindex ("tank/tnkatk2d.wav");
//      gi.soundindex ("tank/tnkatk2e.wav");
//      gi.soundindex ("tank/tnkatck3.wav");
   }
   if (globals.AQ_Artifact_Banned[AQ_KAMIKAZE] == 0)
   {
      if (!itype) gi.modelindex ("models/artifact/pickup/a_kamikaze/tris.md2");
      gi.modelindex ("models/artifact/divine/tris.md2");
      gi.soundindex ("world/quake.wav");
      gi.soundindex ("artifact/boom.wav");
   }
   if (globals.AQ_Artifact_Banned[AQ_ACTION] == 0)
   {
      if (!itype) gi.modelindex ("models/artifact/pickup/a_action/tris.md2");
   }
   if (globals.AQ_Artifact_Banned[AQ_IMPACT] == 0)
   {
      if (!itype) gi.modelindex ("models/artifact/pickup/a_impact/tris.md2");
      gi.modelindex ("models/artifact/sword/tris.md2");
      gi.soundindex ("chick/chkatck4.wav");
   }
   if (globals.AQ_Artifact_Banned[AQ_WEIRD] == 0)
   {
      if (!itype) gi.modelindex ("models/artifact/pickup/a_weird/tris.md2");
      gi.modelindex ("models/artifact/weird/tris.md2");
      gi.soundindex ("artifact/orb2.wav");
   }
   if (globals.AQ_Artifact_Banned[AQ_RECALL2] == 0)
   {
      if (!itype) gi.modelindex ("models/artifact/pickup/a_recall2/tris.md2");
   }
   if (globals.AQ_Artifact_Banned[AQ_VENG] == 0)
   {
      if (!itype) gi.modelindex ("models/artifact/pickup/a_veng/tris.md2");
   }
   if (globals.AQ_Artifact_Banned[AQ_JUMPY] == 0)
   {
      if (!itype) gi.modelindex ("models/artifact/pickup/a_jumpy/tris.md2");
   }
   if (globals.AQ_Artifact_Banned[AQ_ZAP] == 0)
   {
      if (!itype) gi.modelindex ("models/artifact/pickup/a_zap/tris.md2");
      gi.modelindex ("models/monsters/parasite/segment/tris.md2");
      gi.modelindex ("models/artifact/lightning/light1/tris.md2");
      gi.modelindex ("models/artifact/lightning/light2/tris.md2");
      gi.modelindex ("models/artifact/lightning/light3/tris.md2");
      gi.soundindex ("artifact/lhit.wav");
      gi.soundindex ("artifact/lstart.wav");
      gi.soundindex ("world/l_hum1.wav");
   }
   if (globals.AQ_Artifact_Banned[AQ_REPULSE] == 0)
   {
      if (!itype) gi.modelindex ("models/artifact/pickup/a_repulse/tris.md2");
   }
   if (globals.AQ_Artifact_Banned[AQ_FIREWALK] == 0)
   {
      if (!itype) gi.modelindex ("models/artifact/pickup/a_firewalk/tris.md2");
      if (!itype) gi.modelindex ("models/artifact/pickup/hologram/a_hologram/tris.md2");
      gi.modelindex ("models/artifact/flame/tris.md2");
      gi.soundindex ("player/lava_in.wav");
      gi.soundindex ("player/fry.wav");
   }
   if (globals.AQ_Artifact_Banned[AQ_BLINK] == 0)
   {
      if (!itype) gi.modelindex ("models/artifact/pickup/a_blink/tris.md2");
      gi.soundindex ("artifact/pop.wav");
   }

   // Precache icons
   for (n=0; n<MAX_ARTIFACT_TYPES; n++)
      if (globals.AQ_Artifact_Banned[n] == 0)
         gi.imageindex (AQ_Icon_Of(n));
}

// Initialises AQ2 player variables at spawn and respawn
void AQ_ClientInit(gclient_t *client)
{
      int n;

      client->pers.AQ_State = 0;
      client->pers.AQ_SNum = 0;
      client->pers.AQ_Held = 0;
      client->pers.AQ_Equipped = NO_ARTIFACT;
      for (n = 0; n < MAX_ARTIFACT_TYPES; n++)
      {
         client->pers.AQ_Info[n].held = 0;
         client->pers.AQ_Info[n].pTime = 0;
         client->pers.AQ_Info[n].dTime = 0;
         client->pers.AQ_Info[n].aEnt = NULL;
         client->pers.AQ_Info[n].aInt = 0;
         client->pers.AQ_Info[n].aFloat = 0;
         client->pers.AQ_Info[n].aTime = 0;
         VectorClear(client->pers.AQ_Info[n].aVector);
         client->pers.inventory[ITEM_INDEX(FindItem(AQ_SName_Of(n)))] = 0;
      }
}

// Deals with spawning all artifacts at the start of the game, and adding some into the mix later on.
void AQ_Level_Think()
{
   if (level.AQ_Artifact_Timer > level.time) return;

   // If the initial spawning(s) haven't occured, deal with this.
   if (!level.AQ_Artifacts_In)
   {
      level.AQ_Artifacts_In = 1;
      // If you're supposed to spawn all of 'em at once
      if ((int)AQ_SpawnAllAtOnce->value)
         AQ_Spawn_Many_Artifacts((int)AQ_MaxArtifacts->value);
   }

   // Are there not enough artifacts in the level? Spawn one.
   if (level.AQ_CurrentNum < (int)AQ_MaxArtifacts->value)
      AQ_Spawn_Many_Artifacts (1);

   // Think again, later
   level.AQ_Artifact_Timer = level.time + AQ_LEVELTHINK;
   return;
}

// Returns a random artifact type, taking the banned artifact list into account
// Also handles the "exclusive random features"
int AQ_Get_Random_Type()
{
   int n, numLeft, newType;

   // Obtain a random type
   newType = (int)(random()*MAX_ARTIFACT_TYPES);

   if (globals.AQ_Picked_Num == MAX_ARTIFACT_TYPES) // Refresh the list!
   {
      for (n=0; n<MAX_ARTIFACT_TYPES; n++)
         globals.AQ_Artifact_Picked[n] = globals.AQ_Artifact_Banned[n];
      globals.AQ_Picked_Num = globals.AQ_Banned_Num;
   }

   // If XRandom is off, the picked list is the same as the banned list
   // If it's on, we do special things to change the picked list

   // If the type chosen was already picked/banned, do the "special" selection method.
   if (globals.AQ_Artifact_Picked[newType])
   {
      numLeft = 0;

//TODO: can't I just use globals.AQ_Picked_Num?

      // Count up the number of still-permitted types
      for (n=0; n<MAX_ARTIFACT_TYPES; n++)
         if (!globals.AQ_Artifact_Picked[n]) numLeft++;

      // Pick a new random number within the range of types left;
      newType = (int)(random()*numLeft);

      // Find the index of that random number
      n = 0;
      while (newType > 0 || globals.AQ_Artifact_Picked[n])
      {
         if (!globals.AQ_Artifact_Picked[n]) newType--;
         n++;
      }

      // Assign the new number
      newType = n;
   }

   if ((int)AQ_XRandom->value == 1) // If XRandom is on, remove this picked artifact from the list
   {
      globals.AQ_Artifact_Picked[newType] = 1;
      globals.AQ_Picked_Num++;
   }

   return newType;
}

// Finds a random entity for an artifact to spawn from
void AQ_Get_Random_Location (vec3_t luckySpot)
{
   edict_t *luckyEntity;

   luckyEntity = &g_edicts[0];

   while (!(luckyEntity->solid == SOLID_TRIGGER))
      luckyEntity = &(g_edicts[(int)(1+random() * (globals.num_edicts-1))]);

   VectorCopy ((luckyEntity->s.origin), luckySpot);
}
//TODO: Make this more varied and faster.

// spawns many random artifacts at once
void AQ_Spawn_Many_Artifacts (int volume)
{
   int     n, newType;
   vec3_t  randVel, randPos;

   level.AQ_CurrentNum += volume;
  
   for (n = 0; n < volume; n++)
   {
      // New artifact's type
      newType = AQ_Get_Random_Type();

      // New artifact's location
      AQ_Get_Random_Location (randPos);

      // New artifact's velocity
      randVel[0] = -500+random()*1000;
      randVel[1] = -500+random()*1000;
      randVel[2] = 100+random()*500;

      AQ_Spawn_Artifact (newType, randVel, randPos);
   }
}

// Warps an artifact elsewhere
void AQ_Warp (edict_t *ent)
{
   vec3_t newSpot, newVel;

   newVel[0] = -500+random()*1000;
   newVel[1] = -500+random()*1000;
   newVel[2] = 100+random()*500;
   AQ_Get_Random_Location (newSpot);

   VectorCopy (newVel, ent->velocity);
   VectorCopy (newSpot, ent->s.origin);
   VectorCopy (newSpot, ent->s.old_origin);
}

// Called by thinking artifact objects. Moves 'em to a new location.
void AQ_Artifact_Think (edict_t *ent)
{
   AQ_Warp (ent);

   // Sometimes this think is set by artifacts spawned from dead players, even if artifacts shouldn't migrate
   if ((float)AQ_MigrationTime->value > 0)
      ent->nextthink = level.time + (float)AQ_MigrationTime->value + random()*20 - 10;
   else
      ent->think = NULL;
}

// called when an artifact object is touched, preferably by a player, maybe by a wall
void AQ_Artifact_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
   // if hitting a wall, make a bouncy sound
   if (other == world)
   {
      // Check for artifacts which go TINK!TINK!TINK!TINK!TINK! and move them if it gets to be too much.
      if (ent->health > 20) { ent->health = 0; AQ_Warp (ent); return; }
      ent->health++;

      gi.sound (ent, CHAN_AUTO, gi.soundindex("artifact/tink3.wav"), 1, ATTN_STATIC, 0);
      return;
   }

   // if the impact happened inside lava or slime, warp this artifact
   if (gi.pointcontents (ent->s.origin) == CONTENTS_LAVA ||
       gi.pointcontents (ent->s.origin) == CONTENTS_SLIME ||
       gi.pointcontents (ent->s.origin) == CONTENTS_SOLID)
   { AQ_Warp (ent); return; }

   // player pickup!
   if (other->client)
   {
      // If it's too soon
      if (ent->delay > level.time) return;

      // If already holding this artifact
      if (other->client->pers.AQ_Info[(ent->style)].held) return;

      // If already holding as many artifacts as allowed
      if ((int)AQ_PlayerMax->value <= other->client->pers.AQ_Held) return;

      // If artifacts are mortal or immortal, put the artifact on death row.
      if (((int)AQ_Mortality->value == 0)||((int)AQ_Mortality->value == 1))
      {
         // We don't kill it right off. We want it to make a noise and not hog a player sound channel.
         ent->classname = "aq_dead_artifact";
         ent->touch = NULL;
         ent->think = G_FreeEdict;
         ent->nextthink = 1 + level.time;
         ent->s.modelindex = 0;

         // This is just for weirdness right now. Kill the ambient sound
         ent->s.sound = 0;
      }

      // If artifacts are mortal, decrement the level artifact count.
      if ((int)AQ_Mortality->value == 0)
         level.AQ_CurrentNum--;

      // Pickup noise
      gi.sound (ent, CHAN_AUTO, gi.soundindex("flyer/flyatck1.wav"), 1, ATTN_NORM, 0);

      // Do the REAL magic...
      AQ_Pickup (other, ent->style);

      return;
   }
}

// Spawns an artifact somewhere, of a type, with a direction to move in.
// Used in both the random spawning of artifacts throughout the level, and the dropping of an artifact.
// It returns edict_t* so special properties can be set on the spawned item outside of this function.
edict_t* AQ_Spawn_Artifact (int type, vec3_t aVelocity, vec3_t aPosition)
{
   edict_t *artifact;

   // create the artifact and give it data
   artifact = G_Spawn();
   VectorCopy (aPosition, artifact->s.origin);
   VectorCopy (aPosition, artifact->s.old_origin);
   VectorCopy (aVelocity, artifact->velocity);
   artifact->movetype = MOVETYPE_BOUNCE;
   artifact->solid = SOLID_TRIGGER;
   artifact->flags = DROPPED_ITEM;
   VectorSet(artifact->mins, -15, -15, -15);
   VectorSet(artifact->maxs, 15, 15, 15);
   artifact->touch = AQ_Artifact_Touch;
   artifact->think = AQ_Artifact_Think;
   if ((float)AQ_MigrationTime->value > 0) // This little bit is for whether or not an artifact migrates
      artifact->nextthink = level.time + (float)AQ_MigrationTime->value + random()*20 - 10;
   else
      artifact->think = NULL;
   artifact->classname = "aq_artifact";
   artifact->health = 0;               // "health" is a tick counter. If above 20 ticks, warp.
   artifact->style = type;             // "style" represents what kind of artifact is used.
   artifact->delay = level.time + 1.5; // "delay" is used so players can't IMMEDIATELY pick up spawned artifacts
   artifact->s.effects = EF_ROTATE;
   artifact->s.renderfx = RF_GLOW;
   artifact->s.modelindex = gi.modelindex ("models/artifact/pickup/a_artifact/tris.md2");

   if ((int)AQ_TypeInvisible->value == 0) // Player can distinguish artifact types
   {
      // Custom models...
           if (type == AQ_FLIGHT) artifact->s.modelindex = gi.modelindex ("models/artifact/pickup/a_flight/tris.md2");
      else if (type == AQ_GWELL) artifact->s.modelindex = gi.modelindex ("models/artifact/pickup/a_gwell/tris.md2");
      else if (type == AQ_RADIO) artifact->s.modelindex = gi.modelindex ("models/artifact/pickup/a_radio/tris.md2");
      else if (type == AQ_REGEN) artifact->s.modelindex = gi.modelindex ("models/artifact/pickup/a_regen/tris.md2");
      else if (type == AQ_HEALTH) artifact->s.modelindex = gi.modelindex ("models/artifact/pickup/a_health/tris.md2");
      else if (type == AQ_CLOAK) artifact->s.modelindex = gi.modelindex ("models/artifact/pickup/a_cloak/tris.md2");
      else if (type == AQ_RECALL) artifact->s.modelindex = gi.modelindex ("models/artifact/pickup/a_recall/tris.md2");
      else if (type == AQ_SHUB) artifact->s.modelindex = gi.modelindex ("models/artifact/pickup/a_shub/tris.md2");
      else if (type == AQ_SPIKY) artifact->s.modelindex = gi.modelindex ("models/artifact/pickup/a_spiky/tris.md2");
      else if (type == AQ_FREEZE) artifact->s.modelindex = gi.modelindex ("models/artifact/pickup/a_freeze/tris.md2");
      else if (type == AQ_DEATH) artifact->s.modelindex = gi.modelindex ("models/artifact/pickup/a_death/tris.md2");
      else if (type == AQ_CAMO) artifact->s.modelindex = gi.modelindex ("models/artifact/pickup/a_camo/tris.md2");
      else if (type == AQ_HASTE) artifact->s.modelindex = gi.modelindex ("models/artifact/pickup/a_haste/tris.md2");
      else if (type == AQ_BERSERK) artifact->s.modelindex = gi.modelindex ("models/artifact/pickup/a_berserk/tris.md2");
      else if (type == AQ_ORB) artifact->s.modelindex = gi.modelindex ("models/artifact/pickup/a_orb/tris.md2");
      else if (type == AQ_EPA) artifact->s.modelindex = gi.modelindex ("models/artifact/pickup/a_epa/tris.md2");
      else if (type == AQ_HELPER) artifact->s.modelindex = gi.modelindex ("models/artifact/pickup/pokeball/tris.md2");
      else if (type == AQ_IMPACT) artifact->s.modelindex = gi.modelindex ("models/artifact/pickup/a_impact/tris.md2");
      else if (type == AQ_ACTION) artifact->s.modelindex = gi.modelindex ("models/artifact/pickup/a_action/tris.md2");
      else if (type == AQ_RESIST) artifact->s.modelindex = gi.modelindex ("models/artifact/pickup/a_resist/tris.md2");
      else if (type == AQ_RECALL2) artifact->s.modelindex = gi.modelindex ("models/artifact/pickup/a_recall2/tris.md2");
      else if (type == AQ_HOLOGRAM) artifact->s.modelindex = gi.modelindex ("models/artifact/pickup/hologram/tris.md2");
      else if (type == AQ_SHIELD) artifact->s.modelindex = gi.modelindex ("models/artifact/pickup/shield2/tris.md2");
      else if (type == AQ_KAMIKAZE) artifact->s.modelindex = gi.modelindex ("models/artifact/pickup/a_kamikaze/tris.md2");
      else if (type == AQ_BUFFY) artifact->s.modelindex = gi.modelindex ("models/artifact/pickup/a_buffy/tris.md2");
      else if (type == AQ_STRENGTH) artifact->s.modelindex = gi.modelindex ("models/artifact/pickup/a_strength/tris.md2");
      else if (type == AQ_VENG) artifact->s.modelindex = gi.modelindex ("models/artifact/pickup/a_veng/tris.md2");
      else if (type == AQ_JUMPY) artifact->s.modelindex = gi.modelindex ("models/artifact/pickup/a_jumpy/tris.md2");
      else if (type == AQ_ZAP) artifact->s.modelindex = gi.modelindex ("models/artifact/pickup/a_zap/tris.md2");
      else if (type == AQ_REPULSE) artifact->s.modelindex = gi.modelindex ("models/artifact/pickup/a_repulse/tris.md2");
      else if (type == AQ_WEIRD) artifact->s.modelindex = gi.modelindex ("models/artifact/pickup/a_weird/tris.md2");
      else if (type == AQ_FIREWALK) artifact->s.modelindex = gi.modelindex ("models/artifact/pickup/a_firewalk/tris.md2");
      else if (type == AQ_BLINK) artifact->s.modelindex = gi.modelindex ("models/artifact/pickup/a_blink/tris.md2");
      else artifact->s.modelindex = gi.modelindex ("models/artifact/pickup/a_artifact/tris.md2");

      // Specific effects for certain models.
      if (type == AQ_FIREWALK)
      {
         artifact->s.effects |= EF_SPHERETRANS;
         artifact->s.modelindex2 = gi.modelindex ("models/artifact/pickup/hologram/a_hologram/tris.md2");
      }
      else if (type == AQ_HELPER)
      {
         artifact->s.effects |= EF_SPHERETRANS;
         artifact->s.modelindex2 = gi.modelindex ("models/artifact/pickup/pokeball/tankemon/tris.md2");
      }
      else if (type == AQ_HOLOGRAM)
      {
         artifact->s.effects |= EF_SPHERETRANS;
         artifact->s.modelindex2 = gi.modelindex ("models/artifact/pickup/hologram/a_hologram/tris.md2");
      }
      else if (type == AQ_SHIELD)
      {
         artifact->s.effects |= EF_SPHERETRANS;
         artifact->s.modelindex2 = gi.modelindex ("models/artifact/pickup/shield2/a_shield/tris.md2");
      }
   }

   gi.linkentity (artifact); // Do the magic
   return artifact;
}

// Removes all artifacts from a player when they die (maybe later also resurrects with Phoenix)
void AQ_Player_Die (edict_t *ent)
{
   int n;

   if ((int)AQ_Mortality->value == 1) // If artifacts are immortal, we need to spew 'em out
   {
      edict_t *artifact;
      vec3_t  randVel;

      for (n = 0; n < MAX_ARTIFACT_TYPES; n++)
         if (ent->client->pers.AQ_Info[n].held == 1)
         {
            randVel[0] = -200+random()*400;
            randVel[1] = -200+random()*400;
            randVel[2] = 300+random()*300;
            artifact = AQ_Spawn_Artifact (n, randVel, ent->s.origin);
            artifact->think = AQ_Artifact_Think;
            artifact->nextthink = level.time + 5 + random()*5;
         }
   }

   // No more effects and info, dammnit.
   ent->client->pers.AQ_SNum = 0;
   ent->client->pers.AQ_Held = 0;
   ent->client->pers.AQ_Equipped = NO_ARTIFACT;
   for (n=0; n<MAX_ARTIFACT_TYPES; n++)
      if (ent->client->pers.AQ_Info[n].held == 1)
      {
         if (ent->client->pers.AQ_Info[n].on)
         AQ_Passive_Shutdown (ent, n);
         ent->client->pers.AQ_Info[n].held = 0;
      }
}

// Returns the name of a given type of artifact. Used in many things
char* AQ_Name_Of (int type)
{
   if (type == NO_ARTIFACT) return "None";
   if (type == AQ_FLIGHT)   return "Flight Artifact";
   if (type == AQ_GWELL)    return "Gravity Well";
   if (type == AQ_REGEN)    return "Regeneration Artifact";
   if (type == AQ_RADIO)    return "Artifact of Radioactivity";
   if (type == AQ_HEALTH)   return "Artifact of Good Health";
   if (type == AQ_RESIST)   return "Resistance Artifact";
   if (type == AQ_HASTE)    return "Haste Artifact";
   if (type == AQ_EPA)      return "Environmental Protection Artifact";
   if (type == AQ_BERSERK)  return "Berserker Artifact";
   if (type == AQ_RECALL)   return "Recall Artifact";
   if (type == AQ_ZAP)      return "Artifact Electric";
   if (type == AQ_CLOAK)    return "Cloaking Artifact";
   if (type == AQ_KAMIKAZE) return "Artifact of Divine Wind";
   if (type == AQ_DEATH)    return "Death Spots";
   if (type == AQ_HOLOGRAM) return "Holo-Artifact";
   if (type == AQ_ORB)      return "Orb of Death";
   if (type == AQ_BLINK)    return "Blink Artifact";
   if (type == AQ_CAMO)     return "Camouflage Artifact";
   if (type == AQ_JUMPY)    return "Artifact of Uncontrollable Jumping";
   if (type == AQ_ACTION)   return "Action Movie Arsenal";
   if (type == AQ_BUFFY)    return "Vampiric Artifact";
   if (type == AQ_SHIELD)   return "Shielding Artifact";
   if (type == AQ_STRENGTH) return "Strength Artifact";
   if (type == AQ_SWITCH)   return "Stats Switcher";
   if (type == AQ_FREEZE)   return "Ice Traps";
   if (type == AQ_TELE)     return "The Teleport Shuffle";
   if (type == AQ_IMPACT)   return "Impact Artifact";
   if (type == AQ_VENG)     return "Artifact of Vengeance";
   if (type == AQ_SHUB)     return "Shub Hat";
   if (type == AQ_REPULSE)  return "Repulsion Artifact";
   if (type == AQ_PHASED)   return "Phased Artifact";
   if (type == AQ_HELPER)   return "Tank Helper";
   if (type == AQ_RECALL2)  return "DUAL Recall Artifact";
   if (type == AQ_WEIRD)    return "Artifact of Weirdness";
   if (type == AQ_PHOENIX)  return "Phoenix Artifact";
   if (type == AQ_SPIKY)    return "Floating Spiky Death Cloud";
   if (type == AQ_FIREWALK) return "Artifact of Fire Walking";
   return "Unknown";
}

// Returns a shortened name of a given type of artifact. Used mostly for the inventory.
char* AQ_SName_Of (int type)
{
   if (type == NO_ARTIFACT) return "None";
   if (type == AQ_FLIGHT)   return "Flight";
   if (type == AQ_GWELL)    return "Gravity Well";
   if (type == AQ_REGEN)    return "Regen";
   if (type == AQ_RADIO)    return "Radioactive";
   if (type == AQ_HEALTH)   return "Good Health";
   if (type == AQ_RESIST)   return "Resist";
   if (type == AQ_HASTE)    return "Haste";
   if (type == AQ_EPA)      return "EPA";
   if (type == AQ_BERSERK)  return "Berserk";
   if (type == AQ_RECALL)   return "Recall";
   if (type == AQ_ZAP)      return "Electric";
   if (type == AQ_CLOAK)    return "Cloaking";
   if (type == AQ_KAMIKAZE) return "Kamikaze";
   if (type == AQ_DEATH)    return "Death Spots";
   if (type == AQ_HOLOGRAM) return "Holo";
   if (type == AQ_ORB)      return "Death Orb";
   if (type == AQ_BLINK)    return "Blink";
   if (type == AQ_CAMO)     return "Camouflage";
   if (type == AQ_JUMPY)    return "Jumping";
   if (type == AQ_ACTION)   return "Action";
   if (type == AQ_BUFFY)    return "Vampire";
   if (type == AQ_SHIELD)   return "Shielding";
   if (type == AQ_STRENGTH) return "Strength";
   if (type == AQ_SWITCH)   return "Switch";
   if (type == AQ_FREEZE)   return "Ice Traps";
   if (type == AQ_TELE)     return "Shuffle";
   if (type == AQ_IMPACT)   return "Impact";
   if (type == AQ_VENG)     return "Vengeance";
   if (type == AQ_SHUB)     return "Shub Hat";
   if (type == AQ_REPULSE)  return "Repulsion";
   if (type == AQ_PHASED)   return "Phased";
   if (type == AQ_HELPER)   return "Tank Helper";
   if (type == AQ_RECALL2)  return "Dual Recall";
   if (type == AQ_WEIRD)    return "Weirdness";
   if (type == AQ_PHOENIX)  return "Phoenix";
   if (type == AQ_SPIKY)    return "Death Cloud";
   if (type == AQ_FIREWALK) return "Fire Walk";
   return "Unknown";
}

// Returns the filename of the icon that represents the artifact of type "type".
// Believe it or not, this is actually used more than once.
char* AQ_Icon_Of (int type)
{
   if (type == AQ_FLIGHT)   return "a_flight";
   if (type == AQ_GWELL)    return "a_gwell";
   if (type == AQ_REGEN)    return "a_regen";
   if (type == AQ_RADIO)    return "a_radio";
   if (type == AQ_HEALTH)   return "a_health";
   if (type == AQ_RESIST)   return "a_resist";
   if (type == AQ_HASTE)    return "a_haste";
   if (type == AQ_EPA)      return "a_epa";
   if (type == AQ_BERSERK)  return "a_berserk";
   if (type == AQ_RECALL)   return "a_recall";
   if (type == AQ_ZAP)      return "a_zap";
   if (type == AQ_CLOAK)    return "a_cloak";
   if (type == AQ_KAMIKAZE) return "a_kamikaze";
   if (type == AQ_DEATH)    return "a_death";
   if (type == AQ_HOLOGRAM) return "a_hologram";
   if (type == AQ_ORB)      return "a_orb";
   if (type == AQ_BLINK)    return "a_blink";
   if (type == AQ_CAMO)     return "a_camo";
   if (type == AQ_JUMPY)    return "a_jumpy";
   if (type == AQ_ACTION)   return "a_action";
   if (type == AQ_BUFFY)    return "a_buffy";
   if (type == AQ_SHIELD)   return "a_shield";
   if (type == AQ_STRENGTH) return "a_strength";
   if (type == AQ_SWITCH)   return "a_switch";
   if (type == AQ_FREEZE)   return "a_freeze";
   if (type == AQ_TELE)     return "a_tele";
   if (type == AQ_IMPACT)   return "a_impact";
   if (type == AQ_VENG)     return "a_veng";
   if (type == AQ_SHUB)     return "a_shub";
   if (type == AQ_REPULSE)  return "a_repulse";
   if (type == AQ_PHASED)   return "a_phased";
   if (type == AQ_HELPER)   return "a_helper";
   if (type == AQ_RECALL2)  return "a_recall2";
   if (type == AQ_WEIRD)    return "a_weird";
   if (type == AQ_PHOENIX)  return "a_phoenix";
   if (type == AQ_SPIKY)    return "a_spiky";
   if (type == AQ_FIREWALK) return "a_firewalk";
}

char* AQ_Desc_Of(int type)
{
   if (type == AQ_FLIGHT)
      return "Fly around as you please. Pretty self-explanatory.\n";
   if (type == AQ_GWELL)
      return "Press \"The Button\" first to drop a Gravity Well. Gravity Wells suck players and flying items inward. Gravity wells damage anything that gets too close. Press \"The Button\" when Gravity Well is deployed to recall it. If you are killed, Gravity Well will dissapear after about 30 seconds.\n";
   if (type == AQ_REGEN)
      return "Your health regenerates at 5 units per 1.5 seconds. If you've got armour, it too regenerates at 5 units per 1.5 seconds.\n";
   if (type == AQ_RADIO)
      return "Press \"The Button\" to set a radiation point, which in 5 seconds will emit high amounts of radiation for 45 seconds. Anything in that area will be damaged while in visible range of said point. A radiation point cannot be set in view of another radiation point. If you're killed, a radiation point is automatically set where you fell.\n";
   if (type == AQ_HEALTH)
      return "Your upper limit on health is 666. Feel free to grab just about as many health boxes as you like. Or perhaps an adrenaline...\n";
   if (type == AQ_RESIST)
      return "All damage you take is halved.\n";
   if (type == AQ_HASTE)
      return "Rate of fire is dramatically increased. Hyperblaster shots move faster. Press \"The Button\" to perform a super jump.\n";
   if (type == AQ_EPA)
      return "Slime and lava regenerate your health. Water regenerates health slowly. If stuck in liquid, press \"The Button\" to jump out. You've also got immunity to to falling damage, crushing damage, ice traps, death spots, radiation, death orbs, artifact electric attacks, and so on.\n";
   if (type == AQ_BERSERK)
      return "You deal 6x damage, but soak up 3x damage.\n";
   if (type == AQ_RECALL)
      return "Press \"The Button\" a first time to set recall point. Press \"The Button\" while on the recall point to erase it. Press \"The Button\" anywhere else to teleport to the recall point.\n";
   if (type == AQ_ZAP)
      return "Electrification! Any enemy within a certain radius to you will be zapped, providing you can spare some cells to power it. Water BAD!\n";
   if (type == AQ_CLOAK)
      return "Walk around as a hard-to-see pair of eyes. When firing or when damaged, cloaking is reduced to faint transparency. Weapons are silenced, standard pickups make no sound for you, and if you're careful you shall make no footsteps.\n";
   if (type == AQ_KAMIKAZE)
      return "Press \"The Button\" to begin a three click countdown, after which you explode with enough force to vaporize anybody in visible range. No frags are lost for this method of suicide. If you die by other means, you'll explode, but with not quite AS much force as it could be.\n";
   if (type == AQ_DEATH)
      return "Press \"The Button\" to create a death spot and lose 10 health. The first person to touch a death spot, dies. Death spots explode on their own after a while. You may have a maximum of 5 Death Spots out at once.\n";
   if (type == AQ_HOLOGRAM)
      return "Press \"The Button\" to drop a holographic decoy image of yourself. Each image dissapears in about 30 seconds. You may have a maximum of 5 of these out at once.\n";
   if (type == AQ_ORB)
      return "A menacing orb rotates around you, severely bashing anybody who comes in contact (except yourself). If you die, it'll bounce around aimlessly for a bit by means of Artificial Stupidity.\n";
   if (type == AQ_BLINK)
      return "Press \"The Button\" to randomly teleport. Doesn't always work.\n";
   if (type == AQ_CAMO)
      return "You're fully invisible as long as you don't move or shoot. If you do either of these, you're simply very transparent.\n";
   if (type == AQ_JUMPY)
      return "Any enemy in visual range will jump uncontrollably like a giddy schoolgirl.\n";
   if (type == AQ_ACTION)
      return "All weapons are given upon pickup. All ammunition recharges slowly.\n";
   if (type == AQ_BUFFY)
      return "Any damage you dish out is given back to you in health. 1/5th effective versus non-sentient entities.\n";
   if (type == AQ_SHIELD)
      return "Press \"The Button\", and invincibility is granted for a few seconds. This ability takes a few seconds to recharge after each use.\n";
   if (type == AQ_STRENGTH)
      return "You deal double damage.\n";
   if (type == AQ_SWITCH)
      return "Press \"The Button\" while aiming at another player. If their stats are better regarding weapons, health, items, and ammo, those appropriate stats will be switched.\n";
   if (type == AQ_FREEZE)
      return "Press \"The Button\" to set an ice trap. The first enemy to run into an ice trap will be frozen for 12 seconds. Said enemy is unfrozen in a few seconds or when killed.\n";
   if (type == AQ_TELE)
      return "Aim in a direction you wish to go and press \"The Button\". If possible, you will be teleported to the spot you were aiming at. If you aim at an enemy, positions will be switched. 10% chance of telefrag.\n";
   if (type == AQ_IMPACT)
      return "Run head-on into an enemy to slam them up against, whatever. Hurts a lot.\n";
   if (type == AQ_VENG)
      return "All damage you take is dealt right back at your attacker.\n";
   if (type == AQ_SHUB)
      return "Huge rotating monster image is placed on head.\n";
   if (type == AQ_REPULSE)
      return "Automatically repel players and various floating objects.\n";
   if (type == AQ_PHASED)
      return "Walk through some walls, but hard to control.\n";
   if (type == AQ_HELPER)
      return "Tank materializes to fight on your side. Press \"The Button\" to teleport 'im to your location. If others deal enough damage to 'im, he'll teleport back to you automatically.\n";
   if (type == AQ_RECALL2)
      return "Press \"The Button\" the first time to teleport back to where the artifact was picked up. Press \"The Button\" at any other time to teleport to where you last teleported from.\n";
   if (type == AQ_WEIRD)
      return "Tiny bubbles float off you. Press \"The Button\" to blow one in the direction you're facing. If killed, you release a few automatically. Players who touch the bubbles will experience various odd, annoying effects for a while.\n";
   if (type == AQ_PHOENIX)
      return "Three extra lives are granted. Instead of respawning elsewhere after death, you respawn where you died with health fully restored, inventory and weapons intact, and 10 seconds of lovely Quad power.\n";
   if (type == AQ_SPIKY)
      return "A big spiked cloud floats above your head, dropping little pieces of harmful chaff on the ground.\n";
   if (type == AQ_FIREWALK)
      return "When running, a harmful trail of fire is left on the ground where you've passed. Helps in keeping people from chasing you.\n";

   return "This artifact has no description, so you'll have to figure out its function on your own time.\n";
}
//TODO: Format these to a length of 37 characters

// Deals with stuff that needs dealin' when the player picks up a new artifact.
int AQ_Pickup (edict_t *ent, int type)
{
   gclient_t *client = ent->client;

   // Don't pick up stuff twice. This is almost exclusively just for the give command.
   if (client->pers.AQ_Info[type].held == 1) return 0;

   // Bonus flash
   client->bonus_alpha = 0.25;

   // Informational things
   client->pers.AQ_Held++;
   client->pers.AQ_Info[type].held = 1;
   client->pers.AQ_Info[type].pTime = level.time;
   if ((float)AQ_Droptime->value >= 0)
      client->pers.AQ_Info[type].dTime = level.time + (float)AQ_Droptime->value; // Dropability

   // Ceremonial clearing of data
   client->pers.AQ_Info[type].aInt = 0;
   client->pers.AQ_Info[type].aFloat = 0.0;
   client->pers.AQ_Info[type].aVector[0] = 0.0;
   client->pers.AQ_Info[type].aVector[1] = 0.0;
   client->pers.AQ_Info[type].aVector[2] = 0.0;
   client->pers.AQ_Info[type].aTime = 0.0;
   client->pers.AQ_Info[type].aEnt = NULL;

   // If all passive effects is on, turn on the passive effect
   if ((int)AQ_AllArtifactsEffect->value == 1)
      AQ_Passive_Start (ent, type);

   // If the player didn't have any artifact equipped, equip this one.
   if (client->pers.AQ_Equipped == NO_ARTIFACT)
      AQ_Equip (ent, type);
   
   // Do shit required to let the player inventory proper mesh with artifact data
   client->pers.inventory[ITEM_INDEX(FindItem(AQ_SName_Of (type)))]++;
   // Make the artifact show up in the pickup area of the HUD
   client->ps.stats[STAT_PICKUP_ICON] = gi.imageindex(AQ_Icon_Of (type));
   client->ps.stats[STAT_PICKUP_STRING] = CS_GENERAL + type;
   client->pickup_msg_time = level.time + 3;

   // Some artifacts (like death spots) need data that's initialized ONLY on pickup, not when equipping/unequipping.
   AQ_Special_Activate(ent, type);

   // The only point of a return value is so the "give" command can know if you already have something.
   return 1;
}

// Called when a player equips an artifact
void AQ_Equip (edict_t *ent, int type)
{
   // If the player tries to equip an already equipped artifact, activate The Button
   if (ent->client->pers.AQ_Equipped == type)
   { AQ_Active_Effect (ent); return; }

   // If all passive effects is off...
   if ((int)AQ_AllArtifactsEffect->value == 0)
   {
      if (ent->client->pers.AQ_Equipped != NO_ARTIFACT) // If player had an artifact already equipped, kill old passive.
         AQ_Passive_Shutdown (ent, ent->client->pers.AQ_Equipped);
      AQ_Passive_Start (ent, type); // Start the new passive
   }
   // Otherwise all passive is on, and we don't need to change anything here

   // Change the equipped artifact.
   ent->client->pers.AQ_Equipped = type;

   // Set up special status num for the artifact on the HUD
   AQ_HUD_Num (ent);
}

// Depending on what artifact is equipped, a special number will appear next to it on the HUD
void AQ_HUD_Num (edict_t *ent)
{
   AQ_Held_Artifact *artifact = &(ent->client->pers.AQ_Info[ent->client->pers.AQ_Equipped]);
   gclient_t *client = ent->client;
   client->pers.AQ_SNum = 0;

   if (client->pers.AQ_Equipped == AQ_KAMIKAZE) client->pers.AQ_SNum = artifact->aEnt->health;
   if (client->pers.AQ_Equipped == AQ_GWELL) client->pers.AQ_SNum = artifact->aInt;
   if (client->pers.AQ_Equipped == AQ_CAMO) client->pers.AQ_SNum = artifact->aInt;
   if (client->pers.AQ_Equipped == AQ_RECALL) if (VectorLength(artifact->aVector)) client->pers.AQ_SNum = 1;
   if (client->pers.AQ_Equipped == AQ_DEATH) client->pers.AQ_SNum = artifact->aInt;
   if (client->pers.AQ_Equipped == AQ_FREEZE) client->pers.AQ_SNum = artifact->aInt;
   if (client->pers.AQ_Equipped == AQ_HOLOGRAM) client->pers.AQ_SNum = artifact->aInt;
   if (client->pers.AQ_Equipped == AQ_CLOAK) client->pers.AQ_SNum = artifact->aInt;
   if (client->pers.AQ_Equipped == AQ_HELPER) client->pers.AQ_SNum = artifact->aEnt->health;
   if (client->pers.AQ_Equipped == AQ_RADIO) client->pers.AQ_SNum = artifact->aInt;
   if (client->pers.AQ_Equipped == AQ_ZAP) client->pers.AQ_SNum = artifact->aInt;
}

// When a player equips a rune via the inventory proper "use" command
void AQ_InvUse (edict_t *ent, gitem_t *item)
{
   AQ_Equip (ent, item->quantity); // Send the command over to AQ_Equip
}

// When a player drops a rune via the inventory proper "drop" command
void AQ_Drop (edict_t *ent, gitem_t *item)
{
   gclient_t *client = ent->client;
   vec3_t    forward, right, offset;
   vec3_t    newVel;

   // Deal with dropability
   if ((float)AQ_Droptime->value < 0)
   {
      gi.cprintf (ent, PRINT_HIGH, "Artifacts are not dropable.\n");
      return;
   }
   else if (client->pers.AQ_Info[item->quantity].dTime > level.time)
   {
      gi.cprintf (ent, PRINT_HIGH, "You cannot drop your %s for %i more seconds.\n",
       AQ_SName_Of(item->quantity), (int)(client->pers.AQ_Info[item->quantity].dTime - level.time));
      return;
   }

   // Shut down passive effects if it was equipped or all passive effects were on
   if (ent->client->pers.AQ_Equipped == item->quantity || (int)AQ_AllArtifactsEffect->value == 1)
      AQ_Passive_Shutdown (ent, item->quantity);

   // You now hold nothing if the discarded artifact was the one equipped
   if (ent->client->pers.AQ_Equipped == item->quantity)
      ent->client->pers.AQ_Equipped = NO_ARTIFACT;

   // If artifacts are immortal, time for spewing
   if ((int)AQ_Mortality->value == 1)
   {
      AngleVectors (client->v_angle, forward, right, NULL);
	VectorSet (offset, 24, 0, -16);
	VectorScale (forward, 100, newVel);
	newVel[2] = 300;
      AQ_Spawn_Artifact (item->quantity, newVel, ent->s.origin);
   } else

   // For ghost artifacts and mortal artifacts, mention something was successfully discarded
   if ((int)AQ_Mortality->value == 0 || (int)AQ_Mortality->value == 2)
      gi.cprintf (ent, PRINT_HIGH, "%s discarded.\n", AQ_Name_Of (item->quantity));

   // You hold one less artifact.
   client->pers.AQ_Held--;
   // The artifact itself, is no longer held.
   client->pers.AQ_Info[item->quantity].held = 0;
   // Your inventory of that artifact is removed.
   client->pers.inventory[ITEM_INDEX (FindItem (AQ_SName_Of (item->quantity)))]--;

   ValidateSelectedItem (ent);
}

// Prints information about the artifact you have equipped, or another if specified
void AQ_Info_Held (edict_t *ent)
{
   gclient_t *client = ent->client;
   int   toPrint = client->pers.AQ_Equipped;
   char* arg;
   int   n;

   arg = gi.args();
   for (n=0; n<MAX_ARTIFACT_TYPES; n++)
      if (Q_stricmp (arg, AQ_SName_Of(n)) == 0 || Q_stricmp (arg, AQ_Name_Of(n)) == 0)
         toPrint = n;

   if (toPrint == NO_ARTIFACT) return;

   gi.cprintf (ent, PRINT_HIGH, "\n%s (%s):\n   %s\n", AQ_Name_Of(toPrint), AQ_SName_Of(toPrint), AQ_Desc_Of(toPrint));
}

// Prints generic information about the kind of AQ2 server running
void AQ_Info_Server (edict_t *ent)
{                                                         
   gi.cprintf (ent, PRINT_HIGH, "\nΑΡ²Ί Σεςφες Ιξζοςνατιοξ\n\n");
   if (level.AQ_CurrentNum == 1)
      gi.cprintf (ent, PRINT_HIGH, "­ current #: %i artifact currently in level\n", level.AQ_CurrentNum);
   else
      gi.cprintf (ent, PRINT_HIGH, "­ current #: %i artifacts currently in level\n", level.AQ_CurrentNum);
   gi.cprintf (ent, PRINT_HIGH, "­ aq_pickupmax: %i (per player)\n", (int)AQ_PlayerMax->value);
   gi.cprintf (ent, PRINT_HIGH, "­ aq_existmax: %i (in level at once)\n", (int)AQ_MaxArtifacts->value);
   if (AQ_Mortality->value == 0)
      gi.cprintf (ent, PRINT_HIGH, "­ aq_mortality: 0 (mortal)\n");
   else if (AQ_Mortality->value == 1)
      gi.cprintf (ent, PRINT_HIGH, "­ aq_mortality: 1 (immortal)\n");
   else if (AQ_Mortality->value == 2)
      gi.cprintf (ent, PRINT_HIGH, "­ aq_mortality: 2 (ghosts)\n");
   if (AQ_SpawnAllAtOnce->value == 0)
      gi.cprintf (ent, PRINT_HIGH, "­ aq_spawnall: 0\n");
   else if (AQ_SpawnAllAtOnce->value == 1)
      gi.cprintf (ent, PRINT_HIGH, "­ aq_spawnall: 1\n");
   if (AQ_MigrationTime->value == 0)
      gi.cprintf (ent, PRINT_HIGH, "­ aq_migrate: 0 (no migration)\n");
   else if (AQ_MigrationTime->value > 0)
      gi.cprintf (ent, PRINT_HIGH, "­ aq_migrate: %i seconds\n", (int)AQ_MigrationTime->value);
   if (AQ_TypeInvisible->value == 0)
      gi.cprintf (ent, PRINT_HIGH, "­ aq_itype: 0\n");
   else if (AQ_TypeInvisible->value == 1)
      gi.cprintf (ent, PRINT_HIGH, "­ aq_itype: 1\n");
   if (AQ_Droptime->value < 0)
      gi.cprintf (ent, PRINT_HIGH, "­ aq_droptime: %i (undroppable)\n", (int)AQ_Droptime->value);
   else if (AQ_Droptime->value == 0)
      gi.cprintf (ent, PRINT_HIGH, "­ aq_droptime: 0 (instantly droppable)\n");
   else if (AQ_Droptime->value == 1)
      gi.cprintf (ent, PRINT_HIGH, "­ aq_droptime: 1 second\n");
   else if (AQ_Droptime->value > 0)
      gi.cprintf (ent, PRINT_HIGH, "­ aq_droptime: %i seconds\n", (int)AQ_Droptime->value);
   if (AQ_AllArtifactsEffect->value == 0)
      gi.cprintf (ent, PRINT_HIGH, "­ aq_allpassive: 0\n");
   else if (AQ_AllArtifactsEffect->value == 1)
      gi.cprintf (ent, PRINT_HIGH, "­ aq_allpassive: 1\n");
   if (AQ_XRandom->value == 0)
      gi.cprintf (ent, PRINT_HIGH, "­ aq_xrandom: 0\n");
   else if (AQ_XRandom->value == 1)
      gi.cprintf (ent, PRINT_HIGH, "­ aq_xrandom: 1\n");
//TODO: add "xrandom" to help and documentation

   gi.cprintf (ent, PRINT_HIGH, "\n");
}

// Lists artifacts that are banned and artifacts that aren't
void AQ_Info_BanList (edict_t *ent)
{
   int n;

   gi.cprintf (ent, PRINT_HIGH, "\nΑΡ²Ί Βαξξεδ Αςτιζαγτ Μιστ\n\n");
   if (!globals.AQ_Banned_Num)
      { gi.cprintf (ent, PRINT_HIGH, "No artifacts banned.\n"); return; }
   gi.cprintf (ent, PRINT_HIGH, "Ξοτ ΒαξξεδΊ\n");
   for (n=0; n<MAX_ARTIFACT_TYPES; n++)
      if (!globals.AQ_Artifact_Banned[n])
         gi.cprintf (ent, PRINT_HIGH, " ­ %s\n", AQ_SName_Of (n));
   gi.cprintf (ent, PRINT_HIGH, "\n");
   gi.cprintf (ent, PRINT_HIGH, "ΒαξξεδΊ\n");
   for (n=0; n<MAX_ARTIFACT_TYPES; n++)
      if (globals.AQ_Artifact_Banned[n])
         gi.cprintf(ent, PRINT_HIGH, " ­ %s\n", AQ_SName_Of (n));
   gi.cprintf (ent, PRINT_HIGH, "\n");
}

// Prints assorted helpful information, depending on what was page was requested
void AQ_Info_Help (edict_t *ent)
{
   gclient_t *client = ent->client;
   char* arg = gi.args();

   if (Q_stricmp (arg, "play") == 0)
     gi.cprintf(ent, PRINT_HIGH, "\nΘοχ το ΠμαωΊ\n In Artifact Quake, various artifacts pop up in random spots in the level. When a player picks up one an artifact, they obtain one or more special abilities or characteristics. These remain until the player is killed or intentionally discards the artifact.\n On some servers, more than one artifact can be held. On others, only one artifact may be held at a time.\n\n");

   else if (Q_stricmp (arg, "abilities") == 0)
      gi.cprintf(ent, PRINT_HIGH, "\nΘοχ Αςτιζαγτ Αβιμιτιεσ ΧοςλΊ\n There are two kinds of abilities artifacts give. Active, and passive. Some artfiacts give only one kind of ability, some give both.\n Active abilities need to be intentionally activated by the person holding the artifact. Passive abilities happen automatically.\n On some servers, all artifacts in a player's inventory will exhibit their passive effects. On other servers, an artifact needs to be equipped to exhibit its passive effect. To use an active ability, the artifact needs to be equipped.\n\n");

   else if (Q_stricmp (arg, "use") == 0)
      gi.cprintf(ent, PRINT_HIGH, "\nΘοχ το Υσε Ωους ΑςτιζαγτΊ\n For passive abilities of an artifact, nothing specific has to be done. If equipped (or if simply in your inventory, on some servers), all passive effects of an artifact will occur.\n For active abilities, one must first equip the desired artifact, then use the \"αρ­υσε\" or \"αρ­βυττοξ\" command to activate the ability.\n To equip an artifact, select the desired artifact in your inventory and press the normal \"υσε\" button.\n If you press the normal \"υσε\" button when the inventory selection is the same as your equipped artifact, it will be treated as if the \"αρ­υσε\" button was pressed.\n If no artifact is equipped, the next artifact to be picked up will be equipped automatically.\n To drop an artifact, simply use the normal \"δςοπ\" button when the artifact is selected in your inventory, or use the \"αρ­δςοπ\" command to drop whatever artifact is currently equipped.\n\n");

   else if (Q_stricmp (arg, "commands") == 0)
      gi.cprintf(ent, PRINT_HIGH, "\nΜιστ οζ ΓονναξδσΊ\n Commands are used by typing them in at the console, or by binding them to a key and simply pressing that key.\n To bind a key at the console, type \"βιξδ <λεω> <γονναξδ>\" where <λεω> is the key you want to bind the command, <γονναξδ>, to.\n αρ­υσε: Attempt to use active power of your equipped artifact.\n αρ­βυττοξ: Attempt to use active power of your equipped artifact.\n αρ­δςοπ: Discard/drop your equipped artifact. (Regardless of your inventory selection)\n αρ­ιξζο: Display information about your equipped artifact.\n αρ­ιξζο <ξανε>: Display information about the <ξανε> artifact\n αρ­σεςφες: Display server settings.\n αρ­βαξμιστ: Display a list of which artifacts have been banned from the server.\n αρ­νοτδ: Display the server's Message Of The Day\n αρ­θεμπ: Print a generic help screen, or other help screens as specified.\n αρ­ηιφε <ξανε>: Give yourself the <ξανε> artifact. (If cheats are allowed)\n\n");

   else if (Q_stricmp (arg, "server1") == 0)
      gi.cprintf(ent, PRINT_HIGH, "\nΣεςφες Σεττιξησ ΔεσγςιπτιοξσΊ\n Servers can be set to provide a variety of different types of Artifact games. Below lists all the AQ2 server variables, and what their values mean.\n αρίεψιστναψ: Maximum number of artifact in play.\n αρίνιηςατε: Seconds until an artifact moves to a new location. Zero means no movement.\n αρίπιγλυπναψ: Maximum number of artifacts each player can hold.\n αρίνοςταμιτω: If 0, artifacts dissapear when removed from a player. If 1, artifacts pop out of a player when removed. If 2, artifacts don't dissapear from ground when picked up.\n\n");

   else if (Q_stricmp (arg, "server2") == 0)
      gi.cprintf(ent, PRINT_HIGH, "\nΣεςφες Σεττιξησ ΔεσγςιπτιοξσΊ ¨γοξτ©\n αρίσπαχξαμμ: If 1, artifacts are spawned all at level start. If 0, artifacts are gradually spawned.\n αρίιτωπε: If 1, all artifact pickup models are the same. If 0, all pickup models look distinctly different.\n αρίδςοπτινε: Seconds a player must wait before discarding an artifact. No wait if 0. Undroppable if -1.\n αρίαμμπασσιφε: If 1, all passive abilities of artifacts in inventory are exhibited. If 0, only the artifact equipped will exhibit passive abilities.\nαρίψςαξδον: If 0, artifact types are chosen randomly. If 1, the same artifact type won't be chosen twice until all other artifact types have been chosen.\n\n");

   else if (Q_stricmp (arg, "about") == 0)
      gi.cprintf(ent, PRINT_HIGH, "\nΑβουτ Αςτιζαγτ Ρυαλε ²Ί\n Artifact Quake 2 is a patch by Nick \"Singe\" Bousman and Glenn \"Griphis\" Saaiman. The concept of \"artifacts\" or \"runes\" in deathmatch started with Zoid's original Quake CTF. Artifact Quake 2 and all materials for such are Copyright (c) 2000 by Nick Bousman and Glenn Saaiman. \nΑΡ² ΔεσιηξΊ Nick Bousman & Glenn Saaiman\nΑΡ² ΓοδεΊ Nick Bousman\nΑΡ² ΝοδεμσΊ Glenn Saaiman\nΑΡ² ΗςαπθιγσΊ Glenn Saaiman\n\nΘονε ΠαηεΊ http://singe.telefragged.com/artifact/\nΦεςσιοξΊ 4.b\nΔατεδΊ 12-8-99\n\n");

   else gi.cprintf(ent, PRINT_HIGH, "\n   … Χεμγονε το Αςτιζαγτ Ρυαλε ²΅ …\n Below is a list of help topics. Access them by typing \"αρ­θεμπ <τοπιγ>\"\n\nπμαω = How to Play\nαβιμιτιεσ = How Artifact Abilities Work\nυσε = How to Use Your Artifact\nγονναξδσ = List of Commands\nσεςφες± = Server Settings Descriptions\nσεςφες² = More Server Settings Descriptions\nαβουτ = About Artifact Quake 2\n\n");
}

// Ban an artifact from spawning
void AQ_Ban (int type)
{
   if (globals.AQ_Artifact_Banned[type] == 1) return;
   if (globals.AQ_Banned_Num == MAX_ARTIFACT_TYPES-1)
      { gi.bprintf(PRINT_HIGH, "Cannot ban ALL artifacts.\n"); return; }
   gi.bprintf(PRINT_HIGH, "%s has been banned.\n", AQ_Name_Of (type));
   globals.AQ_Artifact_Banned[type] = 1;
   globals.AQ_Banned_Num++;
}

// Unban an artifact from spawning
void AQ_Unban (int type)
{
   if (globals.AQ_Artifact_Banned[type] == 0) return;
   gi.bprintf (PRINT_HIGH, "%s has been unbanned.\n", AQ_Name_Of (type));
   globals.AQ_Artifact_Banned[type] = 0;
   globals.AQ_Banned_Num--;
}

// Handles banning/unbanning from the console
void AQ_Con_Banlist (int doBan)
{
   char* arg;
   int   n;

   // Not the right number of arguments
   if (gi.argc() < 3) return;

   arg = gi.args();
   if (doBan)
      arg += strlen ("aq-ban ");
   else
      arg += strlen ("aq-unban ");

   for (n=0; n<MAX_ARTIFACT_TYPES; n++)
      if (Q_stricmp (arg, AQ_SName_Of(n)) == 0 || Q_stricmp (arg, AQ_Name_Of(n)) == 0)
      {
         if (doBan)
            AQ_Ban (n);
         else
            AQ_Unban (n);
      }
}

// Lets a player give themselves one or more artifact.
void AQ_Give (edict_t *ent)
{
   char* arg;
   int   n, n2 = 0;

   if (deathmatch->value && !sv_cheats->value)
   {
      gi.cprintf (ent, PRINT_HIGH, "No cheaty for you! Server must be set with cheats = 1.\n");
      return;
   }

   // Not the right number of arguments
   if (gi.argc() < 2) return;

   arg = gi.args();

   if (Q_stricmp (arg, "all") == 0) // Give all
   {
      for (n=0; n<MAX_ARTIFACT_TYPES; n++)
         if (AQ_Pickup (ent, n))
	      n2++;
      if (n2)
         gi.sound(ent, CHAN_ITEM, gi.soundindex ("flyer/flyatck1.wav"), 1, ATTN_NORM, 0);
   }
   else
      for (n=0; n<MAX_ARTIFACT_TYPES; n++)
         if (Q_stricmp (arg, AQ_SName_Of(n)) == 0 || Q_stricmp (arg, AQ_Name_Of(n)) == 0)
         {
            if (AQ_Pickup(ent, n))
               gi.sound(ent, CHAN_ITEM, gi.soundindex ("flyer/flyatck1.wav"), 1, ATTN_NORM, 0);
         }
}

// Initialization of data ONLY at pickup (rather than at every passive activation/shutdown)
void AQ_Special_Activate (edict_t *ent, int type)
{
   gclient_t *client = ent->client;
   AQ_Held_Artifact *artifact = &(ent->client->pers.AQ_Info[type]);

   if (type == NO_ARTIFACT) return;

   // Clear initial recall point
   else if (type == AQ_RECALL) VectorClear (artifact->aVector);

   // Set initial recall point
   else if (type == AQ_RECALL2) Recall2Init (ent);

   // Initialize data, so people can't equip/deequip to get around shield recharge.
   else if (type == AQ_SHIELD)
   {
      artifact->aInt = 0;
      artifact->pTime = 0;
   }
}

// I don't like plats making shit explode. All of that is rerouted here.
void AQ_BecomeExplosion1 (edict_t *ent)
{
   // ARTIFACT QUAKE 2: I don't like how stuff blows up when a plat's blocked. It's fine for
   // dropped ammo alone, but for everything else in AQ2, stuff gets fucked up. Let's avoid this.
   if (strcmp(ent->classname, "aq_artifact") == 0) { AQ_Warp(ent); return; }
   if (strcmp(ent->classname, "aq_dead_artifact") == 0) { G_FreeEdict(ent); return; }
   if (strcmp(ent->classname, "aq_death_baby") == 0) { G_FreeEdict(ent); return; }
   if (!(ent->flags & IT_POWERUP)) { return;}

   BecomeExplosion1 (ent);
}

// When a client disconnects, I want EVERYTHING they owned to be removed.
void AQ_Remove_Playthings (edict_t *owner)
{
   edict_t *unluckyEntity = NULL;
   int      n;

   for (n = 0; n < globals.num_edicts; n++)
   {
      if (g_edicts[n].owner == owner)
         G_FreeEdict(&g_edicts[n]);
   }
}

// For coop and single-player level changes, this starts up all the passive effects. Or should
void AQ_Client_Startup (edict_t *ent)
{
   int n;

   for (n = 0; n < MAX_ARTIFACT_TYPES; n++)
      if (ent->client->pers.AQ_Info[n].on == 1)
      {
         AQ_Special_Activate(ent, n);
         AQ_Passive_Start (ent, n);
      }
}

// This is used in coop and single-player games. It'll clear out EVERY temporary AQ entity thing so they don't
// screw when somebody returns to the level later.
void AQ_Remove_ALL_Playthings()
{
   int      i;
   edict_t *ent;

   for (i=0; i<game.maxclients; i++)
   {
      ent = &g_edicts[1+i];

      if (!ent->inuse)
         continue;
	AQ_Remove_Playthings (ent);
   }
}

// Finds the distance to the closest teleport destination. For Death Spots and such.
float AQ_Nearest_TeleDest(vec3_t location)
{
   edict_t *ent = NULL;
   float closest = 1024;
   float length = 1024;
   vec3_t holder;

   while ((ent = G_Find(ent, FOFS(classname), "misc_teleporter_dest")) != NULL)
   {
      VectorSubtract (ent->s.origin, location, holder);
      length = VectorLength (holder);
      if (length < closest) closest = length;
   }
   return closest;
}

// Finds the distance to the closest deathmatch spawn point. For Death Spots and such.
float AQ_Nearest_DMSpawn(vec3_t location)
{
   edict_t *ent = NULL;
   float closest = 1024;
   float length = 1024;
   vec3_t holder;

   while ((ent = G_Find(ent, FOFS(classname), "info_player_deathmatch")) != NULL)
   {
      VectorSubtract (ent->s.origin, location, holder);
      length = VectorLength (holder);
      if (length < closest) closest = length;
   }
   return closest;
}








