// g_weapon.c

#include "g_local.h"
#include "m_player.h"

extern void medic_cable_attack (edict_t *self);

// TYPE OF MONSTER ---- HEALTH
#define M_SOLDIERLT 0 // 20
#define M_SOLDIER 1 // 30
#define M_SOLDIERSS 2 // 40
#define M_FLIPPER 3 // 50
#define M_FLYER 4 // 50
#define M_INFANTRY 5 // 100
#define M_INSANE 6 // 100 - Crazy Marine
#define M_GUNNER 7 // 175
#define M_CHICK 8 // 175
#define M_PARASITE 9 // 175
#define M_FLOATER 10 // 200
#define M_HOVER 11 // 240
#define M_BERSERK 12 // 240
#define M_MEDIC 13 // 300
#define M_MUTANT 14 // 300
#define M_BRAIN 15 // 300
#define M_GLADIATOR 16 // 400
#define M_TANK 17 // 750
#define M_SUPERTANK 18 // 1500
#define M_BOSS2 19 // 2000
#define M_JORG 20 // 3000
#define M_MAKRON 21 // 3000
#define M_STALKER 22

// Random Number
//#define rndnum(y,z) ((random()*((z)-((y)+1)))+(y))
//======================================================
void G_Spawn_Explosion(int type, vec3_t start, vec3_t origin ) {
gi.WriteByte(svc_temp_entity);
gi.WriteByte(type);
gi.WritePosition(start);
gi.multicast(origin, MULTICAST_PVS);
}

// Monster Spawning
// For Medic Commander / Carrier

#define POWER1_SOUND gi.soundindex("misc/power1.wav")
#define POWER2_SOUND gi.soundindex("misc/power2.wav")



void SP_monster_jorg(edict_t *self);
void SP_monster_boss2(edict_t *self);
void SP_monster_berserk(edict_t *self);
void SP_monster_soldier_ss(edict_t *self);
void SP_monster_soldier(edict_t *self);
void SP_monster_brain(edict_t *self);
void SP_monster_chick(edict_t *self);
void SP_monster_flipper(edict_t *self);
void SP_monster_floater(edict_t *self);
void SP_monster_flyer(edict_t *self);
void SP_monster_gladiator(edict_t *self);
void SP_monster_hover(edict_t *self);
void SP_monster_infantry(edict_t *self);
void SP_misc_insane(edict_t *self);
void SP_monster_medic(edict_t *self);
void SP_monster_mutant(edict_t *self);
void SP_monster_parasite(edict_t *self);
void SP_monster_tank(edict_t *self);
void SP_monster_makron(edict_t *self);
void SP_monster_gunner(edict_t *self);
void SP_monster_supertank(edict_t *self);
void SP_monster_soldier_light(edict_t *self);
void SP_monster_stalker(edict_t *self);

//======================================================
// Timer entity initiates the Monsters Self-Detonation.
//
// Monster = timer->activator
// Owner = timer->owner
//======================================================
void Monster_Explode(edict_t *timer) {
vec3_t zvec={0,0,0};

// Has Monster died already?
if (!timer->activator) {
G_FreeEdict(timer);
return; }

// Maximally damage the monster entity...
T_Damage(timer->activator, timer->owner, timer->owner, zvec, timer->activator->s.origin, NULL, 5000, 1, 0,
MOD_SPLASH);

// Spawn an explosion fireball..
G_Spawn_Explosion(TE_EXPLOSION2, timer->activator->s.origin, timer->activator->s.origin);

// Do grenade-type radius damage to anybody nearby
T_RadiusDamage(timer->activator, timer->owner, 40, NULL, 200, MOD_SPLASH);

G_FreeEdict(timer);
}

//=========================================================
void Spawn_Timer(edict_t *ent) {
edict_t *timer;

timer=G_Spawn();
timer->owner=ent;
ent->mynoise2=timer; // Link back to timer..
timer->activator=ent->goalentity;
timer->takedamage=DAMAGE_NO;
timer->movetype=MOVETYPE_NONE;
timer->solid = SOLID_NOT;
VectorClear(timer->s.origin);
VectorClear(timer->mins);
VectorClear(timer->maxs);
timer->think=Monster_Explode;
timer->nextthink=level.time + 180.0; // 3 mins to Self-Destruct
gi.linkentity(timer);
}

//=========================================================
qboolean Spawn_Monster(edict_t *ent, int mtype) {
edict_t *monster;
vec3_t forward, up, torigin;
int temp;

if ((int)(stroggflags->value) & SF_MONSTER_OPP)
{
	safe_cprintf (ent, PRINT_HIGH, "Sorry, no spawning when monster opponents are on.\n");
	return false;
}
if (ent->client->resp.spawned_monsters == 6)
{
	safe_cprintf (ent, PRINT_HIGH, "You're at your limit.\n");
	return false;
}

if ((int)(stroggflags->value) & SF_NO_MONSTER_SPAWNING)
{
	safe_cprintf (ent, PRINT_HIGH, "Sorry, monster spawning is disabled.\n");
	return false;
}
// See if we can project Monster forward 100 units...
AngleVectors(ent->s.angles, forward, NULL, up);

VectorCopy(ent->s.origin, torigin);
VectorMA(torigin, 175, forward, torigin);
VectorMA(torigin, 20, up, torigin);
if (gi.pointcontents(torigin) & MASK_SHOT) {
safe_cprintf(ent,PRINT_HIGH,"Cannot project into Solid!\n");
return false; }

ent->client->resp.spawned_monsters++;


// Create basic entity stuff...
monster = G_Spawn();
monster->classname = "XMonster"; // Used for Killed() & ClientObits()
monster->activator = ent; // Link back to Owner.
monster->owner=world;   // Creator can be killed too.
monster->mtype=mtype;   // Type of monster this is (for obits)..
ent->goalentity = monster; // Link Ent to this Monster.
VectorCopy(torigin, monster->s.origin);
monster->team_owner = ent;

if (!ent->client->pers.monster1)
{
	safe_cprintf (ent, PRINT_HIGH, "Mon1\n");
	ent->client->pers.monster1 = monster;
	goto restofmonsterstuff;
}

if ((!ent->client->pers.monster2) && (ent->client->pers.monster1))
{
	safe_cprintf (ent, PRINT_HIGH, "Mon2\n");
	ent->client->pers.monster2 = monster;
	goto restofmonsterstuff;
}

if ((!ent->client->pers.monster3) && (ent->client->pers.monster2))
{
	safe_cprintf (ent, PRINT_HIGH, "Mon3\n");
	ent->client->pers.monster3 = monster;
	goto restofmonsterstuff;
}

if ((!ent->client->pers.monster4) && (ent->client->pers.monster3))
{
	safe_cprintf (ent, PRINT_HIGH, "Mon4\n");
	ent->client->pers.monster4 = monster;
	goto restofmonsterstuff;
}

if ((!ent->client->pers.monster5) && (ent->client->pers.monster4))
{
	safe_cprintf (ent, PRINT_HIGH, "Mon5\n");
	ent->client->pers.monster5 = monster;
	goto restofmonsterstuff;
}

if ((!ent->client->pers.monster6) && (ent->client->pers.monster5))
{
	safe_cprintf (ent, PRINT_HIGH, "Mon6\n");
	ent->client->pers.monster6 = monster;
	goto restofmonsterstuff;
}

restofmonsterstuff:
monster->s.angles[0] = 360;
monster->s.angles[1] = ent->s.angles[1];
monster->s.angles[2] = 360;

gi.linkentity(monster);

//
// Now the Monster Stuff..
//

skill->value=3; // Toggle Advanced AI Mode.

temp=deathmatch->value;
deathmatch->value=0; // Must = 0 to bypass quick-exit in Monster Code

// create this monster type
switch (mtype) {
case M_BERSERK: SP_monster_berserk(monster); break;
case M_BOSS2: SP_monster_boss2(monster); break;
case M_SOLDIERSS: SP_monster_soldier_ss(monster); break;
case M_JORG: SP_monster_jorg(monster); break;
//case M_BRAIN: SP_monster_brain(monster); break;
case M_CHICK: SP_monster_chick(monster); break;
case M_FLIPPER: SP_monster_flipper(monster); break;
case M_FLOATER: SP_monster_floater(monster); break;
case M_FLYER: SP_monster_flyer(monster); break;
case M_INSANE: SP_misc_insane(monster); break;
case M_GLADIATOR: SP_monster_gladiator(monster); break;
case M_HOVER: SP_monster_hover(monster); break;
case M_INFANTRY: SP_monster_infantry(monster); break;
case M_SOLDIERLT: SP_monster_soldier_light(monster); break;
case M_SOLDIER: SP_monster_soldier(monster); break;
case M_MEDIC: SP_monster_medic(monster); break;
case M_MUTANT: SP_monster_mutant(monster); break;
case M_PARASITE: SP_monster_parasite(monster); break;
case M_TANK: SP_monster_tank(monster); break;
case M_MAKRON: SP_monster_makron(monster); break;
case M_GUNNER: SP_monster_gunner(monster); break;
case M_SUPERTANK: SP_monster_supertank(monster); break;
case M_STALKER: SP_monster_stalker(monster); break;
} // end switch

deathmatch->value=temp; // Restore to previous value.

monster->monsterinfo.aiflags |= AI_BRUTAL; // Kill Everything Mode!!


return true; // Spawn successful
}



void Setup_Random_Monsters(void)
{
	edict_t *ent;

	//if (((int)stroggflags->value & SF_MONSTERS))
		//{;

	ent = G_Spawn();
	ent->nextthink = level.time + 5;
	ent->think = SpawnMonsters;
	//}
	//gi.dprintf ("Setup_Random_Monsters() complete!\n");
}

void SpawnMonsters(edict_t *ent)
{
//	gitem_t *tech;
//	edict_t *spot;

	//gi.dprintf ("SpawnMonsters() started!\n");

	while (monsters_in_map < 7)
	{
		int t;

		t = rndnum (0, 22);
		if (t == 15)
			t = 16;
		Spawn_Random_Monster (t);
	//	gi.dprintf ("Spawning a monster!\n");
	}
	if (ent)
		G_FreeEdict(ent);
	//gi.dprintf ("SpawnMonsters() complete!\n");

}

void ReSpawnMonster(edict_t *self)
{
	G_FreeEdict (self);
	monsters_in_map --;
	Spawn_Random_Monster (NULL);
}

int LevelAverage_Modify (edict_t *monster, int average)
{
	int av;

	if (average < 2)
		av = rndnum (0, 8);
	else if (average < 5)
		av = rndnum (0, 12);
	else if (average < 8)
		av = rndnum (5, 15);
	else if (average < 13)
		av = rndnum (9, 18);
	else if (average < 16)
		av = rndnum (10, 21);
	else if (average < 20)
		av = rndnum (15, 22);
	else if (average < 25)
		av = rndnum (16, 22);
	else
		av = rndnum (0, 8);

	return av;
}

void SP_monster_boss_tank (edict_t *self);
void SP_monster_boss_makron (edict_t *self);

// Spawns monsters around the map.
qboolean Spawn_Random_Monster(int mtype) 
{
	edict_t *monster;
////	vec3_t spawn_origin, spawn_angles;
////	vec3_t forward, up;
	vec3_t torigin = {0, 0, 0};
//	int temp;
	vec3_t mins = {-64, -64, 0};
	vec3_t maxs = {64, 64, 112};
	int i;
	edict_t *cl_ent;
	int alllevelsadded;
	int all;
	int average;
	edict_t *ent;
	int		count = 0;
	int		player;
	int av;

	alllevelsadded = 0;
	all = 0;
	count = 0;

//	returnandfindspawnpoint (mins, maxs, torigin);

	if (monsters_in_map > 7)
	{
		gi.dprintf ("Too many monsters in the map!\n");
		return false;
	}

	// Create basic entity stuff...
	monster = G_Spawn();
	monster->classname = "XMonster"; // Used for Killed() & ClientObits()
	monster->owner=world;   // Creator can be killed too.
	monster->mtype=mtype;   // Type of monster this is (for obits)..
	monster->nextthink = level.time + 100;
	monster->think = ReSpawnMonster;
	monster->monstermap = 255;
//	VectorCopy(torigin, monster->s.origin);

	//gi.dprintf ("Monster spawned!\n");
	monsters_in_map++;

	findspawnpoint(monster);

	if (gi.pointcontents(monster->s.origin) & MASK_SHOT) 
	{
		int t = rndnum (0, 22);

		if (t == 15)
			t = 16;
		gi.dprintf("Monster projected into solid, re-spawning\n");
		monsters_in_map --;
		Spawn_Random_Monster (t);
		return false; 
	}
	
	gi.linkentity(monster);
	
	sound (monster, CHAN_AUTO, "misc", "spawn1", ATTN_NONE);

	//
	// Now the Monster Stuff..
	//
	
	skill->value=3; // Toggle Advanced AI Mode.
	
//	temp=deathmatch->value;
//	deathmatch->value=0; // Must = 0 to bypass quick-exit in Monster Code

	for (player = 1; player <= maxclients->value; player++)
	{
		ent = &g_edicts[player];
		if (!ent->inuse)
			continue;
		if (!ent->client)
			continue;
		count++;
	}

	for (i=1 ; i<=maxclients->value ; i++)
	{
		cl_ent = &g_edicts[i];
		if (!cl_ent->inuse || !cl_ent->client)
			continue;
		if (!cl_ent->client->resp.lvl)
			continue;
		if (!cl_ent->client->resp.class)
			continue;
	
		alllevelsadded += cl_ent->client->resp.lvl;

		if (i == count) // reached last player, so we finish the average
			average = alllevelsadded / count;
	}

	if (!average)
		average = 1;

	// Paril, I have a better idea.
	// I will use the LevelAverage_Modify function to
	// spawn them instead of this place.

	av = LevelAverage_Modify (monster, average);

	switch (av) 
	{
		case M_BERSERK: SP_monster_berserk(monster); break;
		case M_BOSS2: SP_monster_boss2(monster); break;
		case M_SOLDIERSS: SP_monster_soldier_ss(monster); break;
		case M_JORG: SP_monster_jorg(monster); break;
		case M_BRAIN: SP_monster_chick(monster); break; // Paril FIXME, revert to normal brain.
		case M_CHICK: SP_monster_chick(monster); break;
		case M_FLIPPER: SP_monster_flipper(monster); break;
		case M_FLOATER: SP_monster_floater(monster); break;
		case M_FLYER: SP_monster_flyer(monster); break;
		case M_INSANE: SP_monster_chick(monster); break;
		case M_GLADIATOR: SP_monster_gladiator(monster); break;
		case M_HOVER: SP_monster_hover(monster); break;
//		case M_INFANTRY: SP_monster_oinfantry(monster); break;
		case M_INFANTRY: SP_monster_infantry(monster); break;
		case M_SOLDIERLT: SP_monster_soldier_light(monster); break;
		case M_SOLDIER: SP_monster_soldier(monster); break;
		case M_MEDIC: SP_monster_medic(monster); break;
		case M_MUTANT: SP_monster_mutant(monster); break;
		case M_PARASITE: SP_monster_parasite(monster); break;
		case M_TANK: SP_monster_tank(monster); break;
		case M_MAKRON: SP_monster_makron(monster); break;
//		case M_GUNNER: SP_monster_ogunner(monster); break;
		case M_GUNNER: SP_monster_gunner(monster); break;
		case M_SUPERTANK: SP_monster_supertank(monster); break;
		case M_STALKER: SP_monster_stalker(monster); break;
	}

	// Paril, REVISION: Only spawn a Tank Commander if the average level is atleast greator than 15
	if (average < 15 && random() < 0.003) // Very slim chance, but it can happen
	{
		SP_monster_boss_tank (monster);
		safe_bprintf (PRINT_HIGH, "A boss of type Tank Commander has spawned! Be careful everybody!\n");
		return 0;
	}
	else if (average > 14)
	{
		if (random() < 0.010)
		{
			SP_monster_boss_tank (monster);
			safe_bprintf (PRINT_HIGH, "A boss of type Tank Commander has spawned! Be careful everybody!\n");
		}
		else if (random() < 0.004)
		{
			SP_monster_boss_makron (monster);
			safe_bprintf (PRINT_HIGH, "A boss of type Makron has spawned! Be careful everybody!\n");
		}
		return 0;
	}

	gi.dprintf ("Monster of mtype %i was spawned\n", av);

	// create this monster type
/*	switch (mtype) {
	case M_BERSERK: SP_monster_berserk(monster); break;
	case M_BOSS2: SP_monster_boss2(monster); break;
	case M_SOLDIERSS: SP_monster_soldier_ss(monster); break;
	case M_JORG: SP_monster_jorg(monster); break;
		//case M_BRAIN: SP_monster_brain(monster); break;
	case M_CHICK: SP_monster_chick(monster); break;
	case M_FLIPPER: SP_monster_flipper(monster); break;
	case M_FLOATER: SP_monster_floater(monster); break;
	case M_FLYER: SP_monster_flyer(monster); break;
	case M_INSANE: SP_misc_insane(monster); break;
	case M_GLADIATOR: SP_monster_gladiator(monster); break;
	case M_HOVER: SP_monster_hover(monster); break;
	case M_INFANTRY: SP_monster_oinfantry(monster); break;
	case M_SOLDIERLT: SP_monster_soldier_light(monster); break;
	case M_SOLDIER: SP_monster_soldier(monster); break;
	case M_MEDIC: SP_monster_medic(monster); break;
	case M_MUTANT: SP_monster_mutant(monster); break;
	case M_PARASITE: SP_monster_parasite(monster); break;
	case M_TANK: SP_monster_tank(monster); break;
	case M_MAKRON: SP_monster_makron(monster); break;
	case M_GUNNER: SP_monster_ogunner(monster); break;
	case M_SUPERTANK: SP_monster_supertank(monster); break;
	case M_STALKER: SP_monster_stalker(monster); break;
	}*/ // end switch
	
	//SP_monster_boss_makron (monster);

	findspawnpoint (monster);

//	deathmatch->value=temp; // Restore to previous value.
	
	monster->monsterinfo.aiflags |= AI_BRUTAL; // Kill Everything Mode!!


	//gi.dprintf ("Finished spawning the monster!\n");
	return true; // Spawn successful
}

//====================================================
void Cmd_Monsters_f(edict_t *ent) {

if (ent->goalentity) {
Monster_Explode(ent->mynoise2);
ent->goalentity=NULL;
safe_centerprintf(ent, "MONSTER DETONATED\n");
return; }

// Pass in the Flag for the Type of Monster you want!!
if (Spawn_Monster(ent, M_BRAIN)) {
Spawn_Timer(ent);
safe_centerprintf(ent, "MOVE AWAY NOW!!\n"); }
}



// Monster Spawn


























void P_ProjectSource_Reverse(gclient_t *client, 
vec3_t point, 
vec3_t distance, 
vec3_t forward, 
vec3_t right, 
vec3_t result) 
{ 
vec3_t dist={0,0,0}; 

VectorCopy(distance, dist); 
if (client->pers.hand == RIGHT_HANDED) 
dist[1] *= -1; // Left Hand already defaulted 
else if (client->pers.hand == CENTER_HANDED) 
dist[1]= 0; 
G_ProjectSource(point, dist, forward, right, result); 
} 



qboolean	is_quad;
static byte		is_silenced;

/*
=================
NoAmmoWeaponChange
=================
*/
void NoAmmoWeaponChange (edict_t *ent)
{
	char *bestweap;

	if (ent->client->resp.class == 1)
	{
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Soldier Blaster") == 0)
		{
			bestweap = "Soldier Shotgun";
			ent->client->newweapon = FindItem (bestweap);
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Soldier Shotgun") == 0)
		{
			bestweap = "Soldier Machinegun";
			ent->client->newweapon = FindItem (bestweap);
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Soldier Machinegun") == 0)
		{
			bestweap = "Soldier Blaster";
			ent->client->newweapon = FindItem (bestweap);
		}
	}
	else if (ent->client->resp.class == 2)
	{
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Infantry Chaingun") == 0)
		{
			bestweap = "Infantry Melee";
			ent->client->newweapon = FindItem (bestweap);
		}
	}
	else if (ent->client->resp.class == 14)
	{
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Tank Blaster") == 0)
		{
			bestweap = "Tank Machinegun";
			ent->client->newweapon = FindItem (bestweap);
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Tank Machinegun") == 0)
		{
			bestweap = "Tank Rocket Launcher";
			ent->client->newweapon = FindItem (bestweap);
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Tank Rocket Launcher") == 0)
		{
			bestweap = "Tank Blaster";
			ent->client->newweapon = FindItem (bestweap);
		}
	}
	else if (ent->client->resp.class == 41)
	{
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Tank Blaster") == 0)
		{
			bestweap = "Tank Machinegun";
			ent->client->newweapon = FindItem (bestweap);
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Tank Machinegun") == 0)
		{
			bestweap = "Tank Rocket Launcher";
			ent->client->newweapon = FindItem (bestweap);
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Tank Rocket Launcher") == 0)
		{
			bestweap = "Tank Blaster";
			ent->client->newweapon = FindItem (bestweap);
		}
	}
	else if (ent->client->resp.class == 3)
	{
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Tank Blaster") == 0)
		{
			bestweap = "Tank Machinegun";
			ent->client->newweapon = FindItem (bestweap);
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Tank Machinegun") == 0)
		{
			bestweap = "Tank Rocket Launcher";
			ent->client->newweapon = FindItem (bestweap);
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Tank Rocket Launcher") == 0)
		{
			bestweap = "Tank Blaster";
			ent->client->newweapon = FindItem (bestweap);
		}
	}
	else if (ent->client->resp.class == 4)
	{
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Medic HyperBlaster") == 0)
		{
			bestweap = "Medic Healer";
			ent->client->newweapon = FindItem (bestweap);
		}
	}
	else if (ent->client->resp.class == 5)
	{
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Gunner Chaingun") == 0)
		{
			bestweap = "Gunner Grenade Launcher";
			ent->client->newweapon = FindItem (bestweap);
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Gunner Grenade Launcher") == 0)
		{
			bestweap = "Gunner Chaingun";
			ent->client->newweapon = FindItem (bestweap);
		}
	}
	else if (ent->client->resp.class == 6)
	{
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Gladiator Railgun") == 0)
		{
			bestweap = "Gladiator Spike";
			ent->client->newweapon = FindItem (bestweap);
		}
	}
	else if (ent->client->resp.class == 7)
	{
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "SuperTank Chaingun") == 0)
		{
			bestweap = "SuperTank Rocket Launcher";
			ent->client->newweapon = FindItem (bestweap);
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "SuperTank Rocket Launcher") == 0)
		{
			bestweap = "SuperTank Chaingun";
			ent->client->newweapon = FindItem (bestweap);
		}
	}
	else if (ent->client->resp.class == 8)
	{
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Flyer HyperBlaster") == 0)
		{
			bestweap = "Flyer Melee";
			ent->client->newweapon = FindItem (bestweap);
		}
	}
	else if (ent->client->resp.class == 9)
	{
		// No Weapons to run out of ammo on...
	}
	else if (ent->client->resp.class == 10)
	{
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Iron Maiden Rocket Launcher") == 0)
		{
			bestweap = "Iron Maiden Slash";
			ent->client->newweapon = FindItem (bestweap);
		}
	}
	else if (ent->client->resp.class == 11)
	{
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Boss2 Chainguns") == 0)
		{
			bestweap = "Boss2 Rocket Launcher";
			ent->client->newweapon = FindItem (bestweap);
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Boss2 Rocket Launcher") == 0)
		{
			bestweap = "Boss2 Chainguns";
			ent->client->newweapon = FindItem (bestweap);
		}
	}
	else if (ent->client->resp.class == 12)
	{
		// No Weapons to run out of ammo on...
	}
	else if (ent->client->resp.class == 13)
	{
		// Ditto
	}
	else if (ent->client->resp.class == 14)
	{
		// Egh!
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Tank Blaster") == 0)
		{
			bestweap = "Tank Machinegun";
			ent->client->newweapon = FindItem (bestweap);
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Tank Machinegun") == 0)
		{
			bestweap = "Tank Rocket Launcher";
			ent->client->newweapon = FindItem (bestweap);
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Tank Rocket Launcher") == 0)
		{
			bestweap = "Tank Blaster";
			ent->client->newweapon = FindItem (bestweap);
		}
	}
	else if (ent->client->resp.class == 41)
	{
		// Egh!
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Tank Blaster") == 0)
		{
			bestweap = "Tank Machinegun";
			ent->client->newweapon = FindItem (bestweap);
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Tank Machinegun") == 0)
		{
			bestweap = "Tank Rocket Launcher";
			ent->client->newweapon = FindItem (bestweap);
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Tank Rocket Launcher") == 0)
		{
			bestweap = "Tank Blaster";
			ent->client->newweapon = FindItem (bestweap);
		}
	}
	else if (ent->client->resp.class == 15)
	{
		// Only one weapon ><
	}
	else if (ent->client->resp.class == 16)
	{
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Floater HyperBlaster") == 0)
		{
			bestweap = "Floater Clamp";
			ent->client->newweapon = FindItem (bestweap);
		}
	}
	else if (ent->client->resp.class == 17)
	{
		// Blug
	}
	else if (ent->client->resp.class == 18)
	{
		// Only one weapon ><><><
	}
	else if (ent->client->resp.class == 19)
	{
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Makron Blaster") == 0)
		{
			bestweap = "Makron Railgun";
			ent->client->newweapon = FindItem (bestweap);
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Makron Railgun") == 0)
		{
			bestweap = "Makron BFG";
			ent->client->newweapon = FindItem (bestweap);
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Makron BFG") == 0)
		{
			bestweap = "Makron Blaster";
			ent->client->newweapon = FindItem (bestweap);
		}
	}
	else if (ent->client->resp.class == 20)
	{
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Soldier RipperGun") == 0)
		{
			bestweap = "Soldier HyperGun";
			ent->client->newweapon = FindItem (bestweap);
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Soldier HyperGun") == 0)
		{
			bestweap = "Soldier LaserGun";
			ent->client->newweapon = FindItem (bestweap);
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Soldier LaserGun") == 0)
		{
			bestweap = "Soldier RipperGun";
			ent->client->newweapon = FindItem (bestweap);
		}
	}
	else if (ent->client->resp.class == 21)
	{
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Gladiator Plasma") == 0)
		{
			bestweap = "Gladiator Spike";
			ent->client->newweapon = FindItem (bestweap);
		}
	}
	else if (ent->client->resp.class == 22)
	{
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Brain Eye Lasers") == 0)
		{
			bestweap = "BETA Brain Tentacles";
			ent->client->newweapon = FindItem (bestweap);
		}
	}
	else if (ent->client->resp.class == 23)
	{
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "BETA Iron Maiden Rocket Launcher") == 0)
		{
			bestweap = "Iron Maiden Slash";
			ent->client->newweapon = FindItem (bestweap);
		}
	}
	else if (ent->client->resp.class == 24)
	{
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "SuperTank Chaingun") == 0)
		{
			bestweap = "SuperTank Rocket Launcher";
			ent->client->newweapon = FindItem (bestweap);
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "SuperTank Rocket Launcher") == 0)
		{
			bestweap = "SuperTank Chaingun";
			ent->client->newweapon = FindItem (bestweap);
		}
	}
	else if (ent->client->resp.class == 25)
	{
		// None
	}
	else if (ent->client->resp.class == 26)
	{
		// Only one weapon
	}
	else if (ent->client->resp.class == 27)
	{
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Stalker Blaster") == 0)
		{
			bestweap = "Stalker Melee";
			ent->client->newweapon = FindItem (bestweap);
		}
	}
	else if (ent->client->resp.class == 28)
	{
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Medic Commander HyperBlaster") == 0)
		{
			bestweap = "Medic Commander Healer";
			ent->client->newweapon = FindItem (bestweap);
		}
	}
	else if (ent->client->resp.class == 29)
	{
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Carrier Chainguns") == 0)
		{
			bestweap = "Carrier Grenade Launcher";
			ent->client->newweapon = FindItem (bestweap);
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Carrier Grenade Launcher") == 0)
		{
			bestweap = "Carrier Railgun";
			ent->client->newweapon = FindItem (bestweap);
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Carrier Railgun") == 0)
		{
			bestweap = "Carrier Chainguns";
			ent->client->newweapon = FindItem (bestweap);
		}
	}
	else if (ent->client->resp.class == 30)
	{
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Widow Blaster") == 0)
		{
			bestweap = "Widow Railgun";
			ent->client->newweapon = FindItem (bestweap);
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Widow Railgun") == 0)
		{
			bestweap = "Widow Blaster";
			ent->client->newweapon = FindItem (bestweap);
		}
	}
	else if (ent->client->resp.class == 31)
	{
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Black Widow Heat Beam") == 0)
		{
			bestweap = "Black Widow Disruptor";
			ent->client->newweapon = FindItem (bestweap);
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Black Widow Disruptor") == 0)
		{
			bestweap = "Black Widow Heat Beam";
			ent->client->newweapon = FindItem (bestweap);
		}
	}
	else if (ent->client->resp.class == 32)
	{
		// None yet
	}
}
/*******mod_dist_point_to_line***********
* Simply calculates the distance from a point to a line
* point: the point for which we wish to calculate the distance
* linepoint: a point on the line
* linedir: the direction of the line
*****************************************/
float mod_dist_point_to_line(vec3_t point, vec3_t linepoint, vec3_t linedir)
{
        vec3_t temp, temp2;
 
        VectorSubtract(point, linepoint, temp);
        CrossProduct(temp, linedir, temp2);
 
        return VectorLength(temp2)/VectorLength(linedir);
}
 
/*******mod_GetLeadoffVec****************
* Author: Tim Matthews
* NOTE: This Code may be distributed and used freely as long as you give due credit to the author.
*
* Get's the direction in which to shoot at an enemy, taking into account his
* current velocity
*
*Arguments
*       self: you
*       fireorigin: the point from which the blast originates
*       rad: the radius of the bounding sphere. Only consider monsters inside this sphere
*       projspeed: the speed of the projectile being fired
*       bydist: true if judging best target by distance, otherwise test by angle
*       firedir: the direction we want to calculate
*************************************/
edict_t *mod_GetLeadoffVec(edict_t *self, vec3_t fireorigin, float rad, float projspeed, int bydist, vec3_t firedir)
{
	edict_t *other; /*any potential target besides yourself*/
	edict_t *best /*best victim so far*/;
	
	vec3_t viewvec;  /*your line of site*/
	vec3_t guessdir;
	vec3_t bestvec; /*the best guess for the direction of your blaster fire*/
	float bestdist, dist;  /*distance of the other guy and the shortest distance encountered*/
	
	
	vec3_t temp, temp2, otherdir;
	float d, t;
	double alpha, beta, rho;
	double a, b, c, t1, t2;
	
	//gi.dprintf("Checking Targets\n");
	
	AngleVectors (self->client->v_angle, viewvec, NULL, NULL); /*get the view direction*/
	
	best=NULL;
	other=findradius(NULL, fireorigin, rad); /*find something*/
	
	while (other) 
	{
		if (/*(other->svflags & SVF_MONSTER) &&*/ other->health>0 && other!=self) 
		{ /*might have to modify these*/
			if (visible(self, other) && infront(self, other)) 
			{
				/*player is in front and visible*/
				
				/*calculate lead off*/
				VectorSubtract(other->s.origin, other->s.old_origin, otherdir);
				alpha=VectorNormalize(otherdir); /*alpha = speed,  otherdir=direction vector*/
				
				if (alpha>.05 && projspeed>0) 
				{ /*if speed is significant, this value may have to be changed*/
				  d=mod_dist_point_to_line(fireorigin, other->s.origin, otherdir); /*distance from the firepoint to the
				  the line on which the enemy is running*/
				  
				  beta=projspeed; /*our projectile speed*/
				  
				  VectorSubtract(fireorigin, other->s.origin, temp);
				  CrossProduct(temp, otherdir, temp2);  //temp2 now holds the normal to a plane defined by fireorigin and other->s.origin
				  CrossProduct(temp2, otherdir, temp);
				  VectorNormalize(temp); /*temp holds the direction from the point to the line*/
				  
				  VectorScale(temp, d, guessdir);
				  VectorAdd(guessdir, fireorigin, guessdir);
				  VectorSubtract(guessdir, other->s.origin, guessdir);
				  rho=VectorLength(guessdir); /*the length from other->s.origin to the point where the perpendicular vector from fireorigin intersects*/
				  
				  /*now, a little quadratic equation solving...*/
				  a=alpha*alpha-beta*beta; 
				  b=-2*alpha*rho;
				  c=rho*rho+d*d;
				  
				  t1=(-b+sqrt(b*b-4*a*c))/(2*a);
				  t2=(-b-sqrt(b*b-4*a*c))/(2*a);
				  
				  t=(t1>=0)?t1:t2; /*positive solution is the correct one*/
				  
				  VectorScale(otherdir, t*alpha, guessdir);
				  VectorAdd(other->s.origin, guessdir, guessdir);
				  VectorSubtract(guessdir, fireorigin, guessdir); /*now we have our best guess*/
				} 
				else 
				{
					/*enemy is standing still, so just get a simple direction vector*/
					VectorSubtract(other->s.origin, fireorigin, guessdir); 
				}
				
				
				if (bydist) 
				{
					dist=guessdir[0]*guessdir[0]+guessdir[1]*guessdir[1]+guessdir[2]*guessdir[2];
					
					if (!best || dist<bestdist) 
					{
						best=other;
						VectorCopy(guessdir, bestvec);
						bestdist=dist;
					}
				} 
				else 
				{
					/*choose best as the person most in front of us*/
					VectorNormalize(guessdir);
					dist=DotProduct(viewvec, guessdir);
					
					if (best && Q_stricmp(best->classname, "hook") == 0) // don't target hooks
						best = NULL;
					
					if (ctf->value)
					{
						if (best && best->client)
						{
							if (best->client->resp.ctf_team == self->client->resp.ctf_team)
								best = NULL;
						}
						else
						{
							if (best->activator)
							{
								if (best->activator->client->resp.ctf_team == self->activator->client->resp.ctf_team)
									best = NULL;
							}
						}
					}
					
					if (!best || dist>bestdist) 
					{
						best=other;
						VectorCopy(guessdir, bestvec);
						bestdist=dist;
					}
				}
			}
		}
		other=findradius(other, self->s.origin, rad); /*find the next entity*/
	}
	
	if (!best) /*No targets aquired, so just fire forward as usual*/
		AngleVectors(self->client->v_angle, firedir, NULL, NULL);
	else 
	{
		//gi.dprintf("Target %s Aquired\n", best->classname);
		VectorCopy(bestvec, firedir);
		VectorNormalize(firedir);
	}
	
	return best;
}


void weapon_grenade_fire (edict_t *ent, qboolean held);

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

void Left_ProjectSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result)
{
	vec3_t	_distance;

	VectorCopy (distance, _distance);
	if (client->pers.hand == LEFT_HANDED)
		_distance[1] *= 8;
	else if (client->pers.hand == CENTER_HANDED)
		_distance[1] = 0;
	else if (client->pers.hand == RIGHT_HANDED)
		_distance[1] = -8;
	G_ProjectSource (point, _distance, forward, right, result);
}
void Beam_ProjectSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result)
{
	vec3_t	_distance;

	VectorCopy (distance, _distance);
	if (client->pers.hand == LEFT_HANDED)
		_distance[1] *= 12;
	else if (client->pers.hand == CENTER_HANDED)
		_distance[1] = 0;
	else if (client->pers.hand == RIGHT_HANDED)
		_distance[1] = -12;
	G_ProjectSource (point, _distance, forward, right, result);
}
void Left_Flyer_ProjectSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result)
{
	vec3_t	_distance;

	VectorCopy (distance, _distance);
	if (client->pers.hand == LEFT_HANDED)
		_distance[1] *= 14;
	else if (client->pers.hand == CENTER_HANDED)
		_distance[1] = 0;
	else if (client->pers.hand == RIGHT_HANDED)
		_distance[1] = -14;
	G_ProjectSource (point, _distance, forward, right, result);
}

void Right_Flyer_ProjectSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result)
{
	vec3_t	_distance;

	VectorCopy (distance, _distance);
	if (client->pers.hand == LEFT_HANDED)
		_distance[1] *= -14;
	else if (client->pers.hand == CENTER_HANDED)
		_distance[1] = 0;
	else if (client->pers.hand == RIGHT_HANDED)
		_distance[1] = 14;
	G_ProjectSource (point, _distance, forward, right, result);
}

void MBlaster_Fire (edict_t *ent, vec3_t g_offset, int damage, qboolean hyper, int effect)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;

	if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
	{
		ent->client->ps.gunframe = 19;
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		return;
	}

	if (is_quad)
		damage *= 4;
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 24, 8, ent->viewheight-8);
	VectorAdd (offset, g_offset, offset);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	fire_blaster (ent, start, forward, damage, 1000, effect, 2);

	gi.sound(ent, CHAN_WEAPON, gi.soundindex ("medic/medatck1.wav"), 1, ATTN_NORM, 0);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	PlayerNoise(ent, start, PNOISE_WEAPON);
}

void Temp_Medic_Blaster_Fire (edict_t *ent)
{
	int		damage;

	damage = 2;

	MBlaster_Fire (ent, vec3_origin, damage, false, EF_BLASTER);
	ent->client->ps.gunframe++;
}

void MBlasterCmd_Fire (edict_t *ent, vec3_t g_offset, int damage, qboolean hyper, int effect)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;

	if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
	{
		ent->client->ps.gunframe = 19;
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		return;
	}

	if (is_quad)
		damage *= 4;
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 24, 8, ent->viewheight-8);
	VectorAdd (offset, g_offset, offset);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	fire_blaster2 (ent, start, forward, damage, 1000, effect, hyper);

	gi.sound(ent, CHAN_WEAPON, gi.soundindex ("medic_commander/medatck1a.wav"), 1, ATTN_NORM, 0);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	PlayerNoise(ent, start, PNOISE_WEAPON);
}

void Temp_MedicCmd_Blaster_Fire (edict_t *ent)
{
	int		damage;

	damage = 4;

	MBlasterCmd_Fire (ent, vec3_origin, damage, false, EF_BLASTER);
	ent->client->ps.gunframe++;
}

void MBlaster_FireRight (edict_t *ent, vec3_t g_offset, int damage, qboolean hyper, int effect)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;

	if (is_quad)
		damage *= 4;
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 24, 8, ent->viewheight-8);
	VectorAdd (offset, g_offset, offset);
	Right_Flyer_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	fire_blaster (ent, start, forward, damage, 1000, effect, 2);
	//ent->client->ps.gunframe++;

	gi.sound(ent, CHAN_WEAPON, gi.soundindex ("flyer/flyatck3.wav"), 1, ATTN_NORM, 0);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	PlayerNoise(ent, start, PNOISE_WEAPON);
}

void MBlaster_FireLeft (edict_t *ent, vec3_t g_offset, int damage, qboolean hyper, int effect)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;

	if (is_quad)
		damage *= 4;
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 24, 8, ent->viewheight-8);
	VectorAdd (offset, g_offset, offset);
	Left_Flyer_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	fire_blaster (ent, start, forward, damage, 1000, effect, 2);
	//ent->client->ps.gunframe++;

	gi.sound(ent, CHAN_WEAPON, gi.soundindex ("flyer/flyatck3.wav"), 1, ATTN_NORM, 0);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	PlayerNoise(ent, start, PNOISE_WEAPON);
}

void Temp_Flyer_Blaster_FireRight (edict_t *ent)
{
	int		damage;

	damage = 2;

	MBlaster_FireRight (ent, vec3_origin, damage, false, EF_HYPERBLASTER);
}

void Temp_Flyer_Blaster_FireLeft (edict_t *ent)
{
	int		damage;

	damage = 2;

	MBlaster_FireLeft (ent, vec3_origin, damage, false, EF_HYPERBLASTER);
}

void GrenLaunch_Source (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result)
{
	vec3_t	_distance;

	VectorCopy (distance, _distance);
	if (client->pers.hand == LEFT_HANDED)
		_distance[1] *= 5;
	else if (client->pers.hand == CENTER_HANDED)
		_distance[1] = 0;
	else if (client->pers.hand == RIGHT_HANDED)
		_distance[1] = -5;
	G_ProjectSource (point, _distance, forward, right, result);
}

void Middle_Source (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result)
{
	vec3_t	_distance;

	VectorCopy (distance, _distance);
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
		// give them some ammo with it
		ammo = FindItem (ent->item->ammo);
		if ( (int)dmflags->value & DF_INFINITE_AMMO )
			Add_Ammo (other, ammo, 1000);
		else
			Add_Ammo (other, ammo, ammo->quantity);

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
		( !deathmatch->value || other->client->pers.weapon == FindItem("Soldier blaster") ) )
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
void ChangeWeapon (edict_t *ent)
{
	int i;

	if (ent->client->grenade_time)
	{
		ent->client->grenade_time = level.time;
		ent->client->weapon_sound = 0;
		weapon_grenade_fire (ent, false);
		ent->client->grenade_time = 0;
	}

	ent->client->pers.lastweapon = ent->client->pers.weapon;
	ent->client->pers.weapon = ent->client->newweapon;
	ent->client->newweapon = NULL;
	ent->client->machinegun_shots = 0;

	// set visible model
	if (ent->s.modelindex == 255) {
		if (ent->client->pers.weapon)
			i = ((ent->client->pers.weapon->weapmodel & 0xff) << 8);
		else
			i = 0;
		ent->s.skinnum = (ent - g_edicts - 1) | i;
	}

	if (ent->client->pers.weapon && ent->client->pers.weapon->ammo)
		ent->client->ammo_index = ITEM_INDEX(FindItem(ent->client->pers.weapon->ammo));
	else
		ent->client->ammo_index = 0;

	if (!ent->client->pers.weapon)
	{	// dead
		ent->client->ps.gunindex = 0;
		return;
	}

	ent->client->weaponstate = WEAPON_ACTIVATING;
	ent->client->ps.gunframe = 0;
        if (!ent->client->chasetoggle)
                ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);

	/*ent->client->anim_priority = ANIM_PAIN;
	if(ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
			ent->s.frame = FRAME_crpain1;
			ent->client->anim_end = FRAME_crpain4;
	}
	else
	{
			ent->s.frame = FRAME_pain301;
			ent->client->anim_end = FRAME_pain304;
			
	}*/
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
//	int			ammo_index;
//	gitem_t		*ammo_item;

	// see if we're already using it
	if (item == ent->client->pers.weapon)
		return;

	// change to this weapon when down
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
		safe_cprintf (ent, PRINT_HIGH, "Can't drop current weapon\n");
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

void Weapon_Generic2 (edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent))
{
	int		n;

	if(ent->deadflag || ent->s.modelindex != 255) // VWep animations screw up corpses
	{
		return;
	}

	if (ent->client->weaponstate == WEAPON_DROPPING)
	{
		if (ent->client->ps.gunframe == FRAME_DEACTIVATE_LAST)
		{
			ChangeWeapon (ent);
			return;
		}
		/*else if ((FRAME_DEACTIVATE_LAST - ent->client->ps.gunframe) == 4)
		{
			ent->client->anim_priority = ANIM_REVERSE;
			if(ent->client->ps.pmove.pm_flags & PMF_DUCKED)
			{
				ent->s.frame = FRAME_crpain4+1;
				ent->client->anim_end = FRAME_crpain1;
			}
			else
			{
				ent->s.frame = FRAME_pain304+1;
				ent->client->anim_end = FRAME_pain301;
				
			}
		}*/

		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	{
		if ((int)(stroggflags->value) & SF_FAST_ACT)
		{
			ent->client->weaponstate = WEAPON_READY;
			ent->client->ps.gunframe = FRAME_IDLE_FIRST;
			return;
		}
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
		if ((int)(stroggflags->value) & SF_FAST_DEACT)
		{
			ChangeWeapon (ent);
			return;
		}
		ent->client->weaponstate = WEAPON_DROPPING;
		ent->client->ps.gunframe = FRAME_DEACTIVATE_FIRST;

		/*if ((FRAME_DEACTIVATE_LAST - FRAME_DEACTIVATE_FIRST) < 4)
		{
			ent->client->anim_priority = ANIM_REVERSE;
			if(ent->client->ps.pmove.pm_flags & PMF_DUCKED)
			{
				ent->s.frame = FRAME_crpain4+1;
				ent->client->anim_end = FRAME_crpain1;
			}
			else
			{
				ent->s.frame = FRAME_pain304+1;
				ent->client->anim_end = FRAME_pain301;
				
			}
		}*/
		return;
	}

	if (ent->client->weaponstate == WEAPON_READY)
	{
		if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
		{
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
			if ((!ent->client->ammo_index) || 
				( ent->client->pers.inventory[ent->client->ammo_index] >= ent->client->pers.weapon->quantity))
			{
				ent->client->ps.gunframe = FRAME_FIRE_FIRST;
				ent->client->weaponstate = WEAPON_FIRING;

				// start the animation
				ent->client->anim_priority = ANIM_ATTACK;
				/*if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
				{
					ent->s.frame = FRAME_crattak1-1;
					ent->client->anim_end = FRAME_crattak9;
				}
				else
				{
					ent->s.frame = FRAME_attack1-1;
					ent->client->anim_end = FRAME_attack8;
				}*/
			}
			else
			{
				if (level.time >= ent->pain_debounce_time)
				{
					gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
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
			{
				for (n = 0; pause_frames[n]; n++)
				{
					if (ent->client->ps.gunframe == pause_frames[n])
					{
						if (rand()&15)
							return;
					}
				}
			}

			ent->client->ps.gunframe++;
			return;
		}
	}

	if (ent->client->weaponstate == WEAPON_FIRING)
	{
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Gunner Grenade Launcher") == 0)
		{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				if (ent->client->ps.gunframe == 8)
					ent->client->ps.gunframe = 5;
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Icarus HyperBlaster") == 0)
		{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				if (ent->client->ps.gunframe == 5)
					ent->client->ps.gunframe = 2;
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Shark Bite") == 0)
		{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				if (ent->client->ps.gunframe == 21)
					ent->client->ps.gunframe = 10;
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Shark Tail") == 0)
		{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				if (ent->client->ps.gunframe == 20)
					ent->client->ps.gunframe = 9;
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Shark Tail") == 0)
		{
			if (ent->client->ps.gunframe == 9)
			{
				ent->s.frame = 28;
				ent->client->anim_end = 40;			
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Shark Bite") == 0)
		{
			if (ent->client->ps.gunframe == 10)
			{
				ent->s.frame = 10;
				ent->client->anim_end = 19;			
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Tank Blaster") == 0)
		{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				if (ent->client->ps.gunframe == 6)
					ent->client->ps.gunframe = 3;
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Death Tank Laser Gun") == 0)
		{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				if (ent->client->ps.gunframe == 6)
					ent->client->ps.gunframe = 3;
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Carrier Chainguns") == 0)
		{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				if (ent->client->ps.gunframe == 16)
					ent->client->ps.gunframe = 9;
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Widow Blaster") == 0)
		{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				if (ent->client->ps.gunframe == 4)
					ent->client->ps.gunframe = 3;
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Widow Kick") == 0)
		{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				if (ent->client->ps.gunframe == 3)
				{
					ent->s.frame = 161;
					ent->client->anim_end = 168;
				}
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Carrier Grenade Launcher") == 0)
		{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				if (ent->client->ps.gunframe == 14)
					ent->client->ps.gunframe = 7;
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Tank Shockwave") == 0)
		{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				if (ent->client->ps.gunframe == 29)
					ent->client->ps.gunframe = 20;
			}
				if (ent->client->ps.gunframe == 3)
				{
					ent->s.frame = 76;
					ent->client->anim_end = 114;
				}
				if (ent->client->ps.gunframe == 28 && ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
				{
					ent->s.frame = 93;
					ent->client->anim_end = 114;
				}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Gunner Chaingun") == 0)
		{
			if (ent->client->ps.gunframe == 3)
			{
				ent->s.frame = 134;
				ent->client->anim_end = 142;
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Stalker Melee") == 0)
		{
			if (ent->client->ps.gunframe == 3)
			{
				ent->s.frame = 53;
				ent->client->anim_end = 65;
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Stalker Blaster") == 0)
		{
			if (ent->client->ps.gunframe == 3)
			{
				ent->s.frame = 61;
				ent->client->anim_end = 65;
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Gekk Claws") == 0)
		{
			if (ent->client->ps.gunframe == 3)
			{
				ent->s.frame = 45;
				ent->client->anim_end = 53;
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Gekk Spit") == 0)
		{
			if (ent->client->ps.gunframe == 3)
			{
				ent->s.frame = 54;
				ent->client->anim_end = 61;
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Makron BFG") == 0)
		{
			if (ent->client->ps.gunframe == 3)
			{
				ent->s.frame = 0;
				ent->client->anim_end = 7;
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Makron Railgun") == 0)
		{
			if (ent->client->ps.gunframe == 3)
			{
				ent->s.frame = 34;
				ent->client->anim_end = 49;
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Medic HyperBlaster") == 0)
		{
			if (ent->client->ps.gunframe == 3)
			{
				ent->s.frame = 183;
				ent->client->anim_end = 192;
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Medic Healer") == 0)
		{
			if (ent->client->ps.gunframe == 3)
			{
				ent->s.frame = 213;
				ent->client->anim_end = 236;
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Medic Commander Spawner") == 0)
		{
			if (ent->client->ps.gunframe == 3)
			{
				ent->s.frame = 213;
				ent->client->anim_end = 236;
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Medic Commander Healer") == 0)
		{
			if (ent->client->ps.gunframe == 3)
			{
				ent->s.frame = 213;
				ent->client->anim_end = 236;
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Medic Commander HyperBlaster") == 0)
		{
			if (ent->client->ps.gunframe == 3)
			{
				ent->s.frame = 183;
				ent->client->anim_end = 192;
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Widow Spawner") == 0)
		{
			if (ent->client->resp.class == 31)
			{
				if (ent->client->ps.gunframe == 3)
				{
					ent->s.frame = 10;
					ent->client->anim_end = 27;
				}
			}
			if (ent->client->resp.class == 30)
			{
				if (ent->client->ps.gunframe == 3)
				{
					ent->s.frame = 82;
					ent->client->anim_end = 99;
				}
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Infantry Melee") == 0)
		{

			if (ent->client->ps.gunframe == 5)
			{
				ent->s.frame = 199;
				ent->client->anim_end = 206;
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Jorg Chainguns") == 0)
		{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				if (ent->client->ps.gunframe == 6)
					ent->client->ps.gunframe = 2;
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Black Widow Heat Beam") == 0)
		{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				if (ent->client->ps.gunframe == 6)
					ent->client->ps.gunframe = 5;
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Tank Rocket Launcher") == 0)
		{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				if (ent->client->ps.gunframe == 23)
					ent->client->ps.gunframe = 20;
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Death Tank Plasma Launcher") == 0)
		{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				if (ent->client->ps.gunframe == 23)
					ent->client->ps.gunframe = 20;
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Tank Rocket Launcher") == 0)
		{
				if (ent->client->ps.gunframe == 3)
				{
					ent->s.frame = 114;
					ent->client->anim_end = 134;
				}
				if (ent->client->ps.gunframe == 13)
				{
					ent->s.frame = 134;
					ent->client->anim_end = 147;
				}
				if (ent->client->ps.gunframe == 22 && ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
				{
					ent->s.frame = 143;
					ent->client->anim_end = 167;
				}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Death Tank Plasma Launcher") == 0)
		{
				if (ent->client->ps.gunframe == 3)
				{
					ent->client->ps.gunframe = 13;
				}
				if (ent->client->ps.gunframe == 13)
				{
					ent->s.frame = 134;
					ent->client->anim_end = 147;
				}
				if (ent->client->ps.gunframe == 22 && ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
				{
					ent->s.frame = 143;
					ent->client->anim_end = 167;
				}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Mutant Claws") == 0)
		{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				if (ent->client->ps.gunframe == 12)
					ent->client->ps.gunframe = 4;
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Medic Hyperblaster") == 0)
		{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				if (ent->client->ps.gunframe == 13)
					ent->client->ps.gunframe = 12;
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Medic Commander Hyperblaster") == 0)
		{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				if (ent->client->ps.gunframe == 13)
					ent->client->ps.gunframe = 12;
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Medic Hyperblaster") == 0)
		{
			if (ent->client->ps.gunframe == 4)
			{
				Temp_Medic_Blaster_Fire (ent);
			}
			if (ent->client->ps.gunframe == 8)
			{
				Temp_Medic_Blaster_Fire (ent);
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Medic Commander Hyperblaster") == 0)
		{
			if (ent->client->ps.gunframe == 4)
			{
				Temp_MedicCmd_Blaster_Fire (ent);
			}
			if (ent->client->ps.gunframe == 8)
			{
				Temp_MedicCmd_Blaster_Fire (ent);
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Flyer Hyperblaster") == 0)
		{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				if (ent->client->ps.gunframe == 5)
					ent->client->ps.gunframe = 2;
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Flyer Hyperblaster") == 0)
		{
			if (ent->client->ps.gunframe == 2)
			{
				Temp_Flyer_Blaster_FireLeft (ent);
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Flyer Hyperblaster") == 0)
		{
			if (ent->client->ps.gunframe == 2)
			{
				Temp_Flyer_Blaster_FireRight (ent);
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Flyer Hyperblaster") == 0)
		{
			if (ent->client->ps.gunframe == 3)
			{
				Temp_Flyer_Blaster_FireLeft (ent);
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Flyer Hyperblaster") == 0)
		{
			if (ent->client->ps.gunframe == 4)
			{
				Temp_Flyer_Blaster_FireRight (ent);
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Flyer Hyperblaster") == 0)
		{
			if (ent->client->ps.gunframe == 3)
			{
				ent->s.frame = 86;
				ent->client->anim_end = 89;
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Flyer Hornet Railgun") == 0)
		{
			if (ent->client->ps.gunframe == 3)
			{
				ent->s.frame = 79;
				ent->client->anim_end = 95;
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Infantry Melee") == 0)
		{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				if (ent->client->ps.gunframe == 14)
					ent->client->ps.gunframe = 4;
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Soldier Blaster") == 0)
		{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				if (ent->client->ps.gunframe == 9)
					ent->client->ps.gunframe = 4;
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Soldier HyperGun") == 0)
		{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				if (ent->client->ps.gunframe == 9)
					ent->client->ps.gunframe = 4;
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Soldier RipperGun") == 0)
		{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				if (ent->client->ps.gunframe == 9)
					ent->client->ps.gunframe = 4;
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Gladiator Spike") == 0)
		{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				if (ent->client->ps.gunframe == 9)
					ent->client->ps.gunframe = 3;
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Gladiator Spike") == 0)
		{
			if (ent->client->ps.gunframe == 3)
			{
				ent->s.frame = 30;
				ent->client->anim_end = 37;
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Flyer Melee") == 0)
		{
			if (ent->client->ps.gunframe == 3)
			{
				ent->s.frame = 64;
				ent->client->anim_end = 77;
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "SuperTank Rocket Launcher") == 0)
		{
			if (ent->client->ps.gunframe == 3)
			{
				ent->s.frame = 27;
				ent->client->anim_end = 46;
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "BETA Brain Tentacles") == 0)
		{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				if (ent->client->ps.gunframe == 14)
					ent->client->ps.gunframe = 9;
			}
		}
		
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "BETA Brain Tentacles") == 0)
		{
			if (ent->client->ps.gunframe == 8)
			{
				gi.sound(ent, CHAN_WEAPON, gi.soundindex ("brain/Brnatck1.wav"), 1, ATTN_NORM, 0);
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "BETA Brain Tentacles") == 0)
		{
			if (ent->client->ps.gunframe == 3)
			{
				ent->s.frame = 71;
				ent->client->anim_end = 82;			
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Brain Tentacles") == 0)
		{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				if (ent->client->ps.gunframe == 14)
					ent->client->ps.gunframe = 9;
			}
		}
		
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Brain Tentacles") == 0)
		{
			if (ent->client->ps.gunframe == 8)
			{
				gi.sound(ent, CHAN_WEAPON, gi.soundindex ("brain/Brnatck1.wav"), 1, ATTN_NORM, 0);
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Brain Tentacles") == 0)
		{
			if (ent->client->ps.gunframe == 3)
			{
				ent->s.frame = 71;
				ent->client->anim_end = 82;			
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Mutant Claws") == 0)
		{
			if (ent->client->ps.gunframe == 4)
			{
				ent->s.frame = 5;
				ent->client->anim_end = 14;			
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Parasite Drain Attack") == 0)
		{
			if (ent->client->ps.gunframe == 2)
			{
				ent->s.frame = 39;
				ent->client->anim_end = 56;			
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Parasite Suck Attack") == 0)
		{
			if (ent->client->ps.gunframe == 2)
			{
				ent->s.frame = 39;
				ent->client->anim_end = 56;			
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Floater Clamp") == 0)
		{
			if (ent->client->ps.gunframe == 3)
			{
				ent->s.frame = 50;
				ent->client->anim_end = 69;			
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Floater Melee") == 0)
		{
			if (ent->client->ps.gunframe == 3)
			{
				ent->s.frame = 72;
				ent->client->anim_end = 103;			
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Brain Tentacles") == 0)
		{
			if (ent->client->ps.gunframe == 9)
			{
				ent->s.frame = 76;
				ent->client->anim_end = 82;			
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "BETA Brain Tentacles") == 0)
		{
			if (ent->client->ps.gunframe == 9)
			{
				ent->s.frame = 76;
				ent->client->anim_end = 82;			
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Tank Shockwave") == 0)
		{
			if (ent->client->ps.gunframe == 3)
			{
				gi.sound(ent, CHAN_WEAPON, gi.soundindex ("tank/Tnkatck4.wav"), 1, ATTN_NORM, 0);
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Makron Railgun") == 0)
		{
			if (ent->client->ps.gunframe == 3)
			{
				gi.sound(ent, CHAN_WEAPON, gi.soundindex ("makron/rail_up.wav"), 1, ATTN_NORM, 0);
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Shark Bite") == 0)
		{
			if (ent->client->ps.gunframe == 3)
			{
				gi.sound(ent, CHAN_WEAPON, gi.soundindex ("flipper/flpatck1.wav"), 1, ATTN_NORM, 0);
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Shark Tail") == 0)
		{
			if (ent->client->ps.gunframe == 3)
			{
				gi.sound(ent, CHAN_WEAPON, gi.soundindex ("flipper/flpatck2.wav"), 1, ATTN_NORM, 0);
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Brain Claws") == 0)
		{
			if (ent->client->ps.gunframe == 7)
			{
				gi.sound(ent, CHAN_WEAPON, gi.soundindex ("brain/melee1.wav"), 1, ATTN_NORM, 0);
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Brain Claws") == 0)
		{
			if (ent->client->ps.gunframe == 3)
			{
				ent->s.frame = 52;
				ent->client->anim_end = 70;			
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Brain Claws") == 0)
		{
			if (ent->client->ps.gunframe == 12)
			{
				gi.sound(ent, CHAN_WEAPON, gi.soundindex ("brain/melee2.wav"), 1, ATTN_NORM, 0);
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Soldier Shotgun") == 0)
		{
			if (ent->client->ps.gunframe == 10)
			{
				gi.sound(ent, CHAN_WEAPON, gi.soundindex ("infantry/infatck3.wav"), 1, ATTN_NORM, 0);
			}
		}
		//
		//
		//	Iron Maiden
		//
		//
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Iron Maiden Rocket Launcher") == 0)
		{
			if (ent->client->ps.gunframe == 3)
			{
				gi.sound(ent, CHAN_WEAPON, gi.soundindex ("chick/Chkatck1.wav"), 1, ATTN_NORM, 0);
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Iron Maiden Rocket Launcher") == 0)
		{
			if (ent->client->ps.gunframe == 3)
			{
				ent->s.frame = 0;
				ent->client->anim_end = 12;
			}
			if (ent->client->ps.gunframe == 15)
			{
				ent->s.frame = 11;
				ent->client->anim_end = 12;
			}
			if (ent->client->ps.gunframe == 16)
			{
				ent->s.frame = 11;
				ent->client->anim_end = 12;
			}
			if (ent->client->ps.gunframe == 17)
			{
				ent->s.frame = 11;
				ent->client->anim_end = 12;
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Iron Maiden Rocket Launcher") == 0)
		{
			if (ent->client->ps.gunframe == 25)
			{
				gi.sound(ent, CHAN_WEAPON, gi.soundindex ("chick/Chkatck5.wav"), 1, ATTN_NORM, 0);
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Iron Maiden Rocket Launcher") == 0)
		{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				if (ent->client->ps.gunframe == 30)
					ent->client->ps.gunframe = 17;
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Beta Iron Maiden Rocket Launcher") == 0)
		{
			if (ent->client->ps.gunframe == 3)
			{
				gi.sound(ent, CHAN_WEAPON, gi.soundindex ("chick/Chkatck1.wav"), 1, ATTN_NORM, 0);
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Beta Iron Maiden Rocket Launcher") == 0)
		{
			if (ent->client->ps.gunframe == 3)
			{
				ent->s.frame = 0;
				ent->client->anim_end = 12;
			}
			if (ent->client->ps.gunframe == 15)
			{
				ent->s.frame = 11;
				ent->client->anim_end = 12;
			}
			if (ent->client->ps.gunframe == 16)
			{
				ent->s.frame = 11;
				ent->client->anim_end = 12;
			}
			if (ent->client->ps.gunframe == 17)
			{
				ent->s.frame = 11;
				ent->client->anim_end = 12;
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Beta Iron Maiden Rocket Launcher") == 0)
		{
			if (ent->client->ps.gunframe == 25)
			{
				gi.sound(ent, CHAN_WEAPON, gi.soundindex ("chick/Chkatck5.wav"), 1, ATTN_NORM, 0);
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Beta Iron Maiden Rocket Launcher") == 0)
		{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				if (ent->client->ps.gunframe == 30)
					ent->client->ps.gunframe = 17;
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Iron Maiden Slash") == 0)
		{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				if (ent->client->ps.gunframe == 11)
					ent->client->ps.gunframe = 2;
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Iron Maiden Slash") == 0)
		{
			if (ent->client->ps.gunframe == 3)
			{
				ent->s.frame = 31;
				ent->client->anim_end = 47;
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Boss2 Chainguns") == 0)
		{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				if (ent->client->ps.gunframe == 5)
					ent->client->ps.gunframe = 3;
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Infantry Melee") == 0)
		{
			if (ent->client->ps.gunframe == 8)
			{
				gi.sound(ent, CHAN_WEAPON, gi.soundindex ("infantry/infatck2.wav"), 1, ATTN_NORM, 0);
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Berserker Club") == 0)
		{
			if (ent->client->ps.gunframe == 3)
			{
				gi.sound(ent, CHAN_WEAPON, gi.soundindex ("berserk/attack.wav"), 1, ATTN_NORM, 0);
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Berserker Spike") == 0)
		{
			if (ent->client->ps.gunframe == 3)
			{
				gi.sound(ent, CHAN_WEAPON, gi.soundindex ("berserk/attack.wav"), 1, ATTN_NORM, 0);
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Berserker Club") == 0)
		{
			if (ent->client->ps.gunframe == 3)
			{
				ent->s.frame = 85;
				ent->client->anim_end = 96;
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Berserker Spike") == 0)
		{
			if (ent->client->ps.gunframe == 3)
			{
				ent->s.frame = 76;
				ent->client->anim_end = 84;
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Gladiator Spike") == 0)
		{
			if (ent->client->ps.gunframe == 3)
			{
				gi.sound(ent, CHAN_WEAPON, gi.soundindex ("gladiator/melee1.wav"), 1, ATTN_NORM, 0);
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Fixbot Welder") == 0)
		{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				if (ent->client->ps.gunframe == 19)
					ent->client->ps.gunframe = 12;
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Gladiator Plasma") == 0)
		{
			if (ent->client->ps.gunframe == 6)
			{
				gi.sound(ent, CHAN_WEAPON, gi.soundindex ("weapons/shootplasma.wav"), 1, ATTN_NORM, 0);
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Gladiator Railgun") == 0)
		{
			if (ent->client->ps.gunframe == 3)
			{
				gi.sound(ent, CHAN_WEAPON, gi.soundindex ("gladiator/railgun.wav"), 1, ATTN_NORM, 0);
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Widow Railgun") == 0)
		{
			if (ent->client->ps.gunframe == 3)
			{
				gi.sound(ent, CHAN_WEAPON, gi.soundindex ("gladiator/railgun.wav"), 1, ATTN_NORM, 0);
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Carrier Railgun") == 0)
		{
			if (ent->client->ps.gunframe == 3)
			{
				gi.sound(ent, CHAN_WEAPON, gi.soundindex ("gladiator/railgun.wav"), 1, ATTN_NORM, 0);
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Gladiator Railgun") == 0)
		{
			if (ent->client->ps.gunframe == 3)
			{
				ent->s.frame = 46;
				ent->client->anim_end = 54;
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Gladiator Plasma") == 0)
		{
			if (ent->client->ps.gunframe == 3)
			{
				ent->s.frame = 46;
				ent->client->anim_end = 54;
			}
		}
		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Gunner Chaingun") == 0)
		{
			if (ent->client->ps.gunframe == 4)
			{
				gi.sound(ent, CHAN_WEAPON, gi.soundindex ("gunner/gunatck1.wav"), 1, ATTN_NORM, 0);
			}
		}
		for (n = 0; fire_frames[n]; n++)
		{
			if (ent->client->ps.gunframe == fire_frames[n])
			{
//ZOID
				if (!CTFApplyStrengthSound(ent))
//ZOID
				if (ent->client->quad_framenum > level.framenum)
					gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage3.wav"), 1, ATTN_NORM, 0);
//ZOID
				CTFApplyHasteSound(ent);
//ZOID
				fire (ent);
				break;
			}
		}

		if (!fire_frames[n])
			ent->client->ps.gunframe++;

		if (ent->client->ps.gunframe == FRAME_IDLE_FIRST+1)
			ent->client->weaponstate = WEAPON_READY;
	}
}

//ZOID
void Weapon_Generic (edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent))
{
	int oldstate = ent->client->weaponstate;

	Weapon_Generic2 (ent, FRAME_ACTIVATE_LAST, FRAME_FIRE_LAST, 
		FRAME_IDLE_LAST, FRAME_DEACTIVATE_LAST, pause_frames, 
		fire_frames, fire);

	// run the weapon frame again if hasted
	//if (stricmp(ent->client->pers.weapon->pickup_name, "Grapple") == 0 &&
		//ent->client->weaponstate == WEAPON_FIRING)
		//return;

	if ((CTFApplyHaste(ent) ||
		(Q_stricmp(ent->client->pers.weapon->pickup_name, "Grapple") == 0 &&
		ent->client->weaponstate != WEAPON_FIRING))
		&& oldstate == ent->client->weaponstate) {
		Weapon_Generic2 (ent, FRAME_ACTIVATE_LAST, FRAME_FIRE_LAST, 
			FRAME_IDLE_LAST, FRAME_DEACTIVATE_LAST, pause_frames, 
			fire_frames, fire);
	}
}
//ZOID


/*
======================================================================

GRENADE

======================================================================
*/

#define GRENADE_TIMER		3.0
#define GRENADE_MINSPEED	400
#define GRENADE_MAXSPEED	800

void weapon_grenade_fire (edict_t *ent, qboolean held)
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
	int		damage = 125;
	float	timer;
	int		speed;
	float	radius;

	radius = damage+40;
	if (is_quad)
		damage *= 4;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	timer = ent->client->grenade_time - level.time;
	speed = GRENADE_MINSPEED + (GRENADE_TIMER - timer) * ((GRENADE_MAXSPEED - GRENADE_MINSPEED) / GRENADE_TIMER);
	fire_grenade2 (ent, start, forward, damage, speed, timer, radius, held);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->client->grenade_time = level.time + 1.0;

	if(ent->deadflag || ent->s.modelindex != 255) // VWep animations screw up corpses
	{
		return;
	}

	if (ent->health <= 0)
		return;

	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->client->anim_priority = ANIM_ATTACK;
		ent->s.frame = FRAME_crattak1-1;
		ent->client->anim_end = FRAME_crattak3;
	}
	else
	{
		ent->client->anim_priority = ANIM_REVERSE;
		ent->s.frame = FRAME_wave08;
		ent->client->anim_end = FRAME_wave01;
	}
}

void Weapon_Grenade (edict_t *ent)
{
	if ((ent->client->newweapon) && (ent->client->weaponstate == WEAPON_READY))
	{
		ChangeWeapon (ent);
		return;
	}

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	{
		ent->client->weaponstate = WEAPON_READY;
		ent->client->ps.gunframe = 16;
		return;
	}

	if (ent->client->weaponstate == WEAPON_READY)
	{
		if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
		{
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
			if (ent->client->pers.inventory[ent->client->ammo_index])
			{
				ent->client->ps.gunframe = 1;
				ent->client->weaponstate = WEAPON_FIRING;
				ent->client->grenade_time = 0;
			}
			else
			{
				if (level.time >= ent->pain_debounce_time)
				{
					gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}
			}
			return;
		}

		if ((ent->client->ps.gunframe == 29) || (ent->client->ps.gunframe == 34) || (ent->client->ps.gunframe == 39) || (ent->client->ps.gunframe == 48))
		{
			if (rand()&15)
				return;
		}

		if (++ent->client->ps.gunframe > 48)
			ent->client->ps.gunframe = 16;
		return;
	}

	if (ent->client->weaponstate == WEAPON_FIRING)
	{
		if (ent->client->ps.gunframe == 5)
			gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/hgrena1b.wav"), 1, ATTN_NORM, 0);

		if (ent->client->ps.gunframe == 11)
		{
			if (!ent->client->grenade_time)
			{
				ent->client->grenade_time = level.time + GRENADE_TIMER + 0.2;
				ent->client->weapon_sound = gi.soundindex("weapons/hgrenc1b.wav");
			}

			// they waited too long, detonate it in their hand
			if (!ent->client->grenade_blew_up && level.time >= ent->client->grenade_time)
			{
				ent->client->weapon_sound = 0;
				weapon_grenade_fire (ent, true);
				ent->client->grenade_blew_up = true;
			}

			if (ent->client->buttons & BUTTON_ATTACK)
				return;

			if (ent->client->grenade_blew_up)
			{
				if (level.time >= ent->client->grenade_time)
				{
					ent->client->ps.gunframe = 15;
					ent->client->grenade_blew_up = false;
				}
				else
				{
					return;
				}
			}
		}

		if (ent->client->ps.gunframe == 12)
		{
			ent->client->weapon_sound = 0;
			weapon_grenade_fire (ent, false);
		}

		if ((ent->client->ps.gunframe == 15) && (level.time < ent->client->grenade_time))
			return;

		ent->client->ps.gunframe++;

		if (ent->client->ps.gunframe == 16)
		{
			ent->client->grenade_time = 0;
			ent->client->weaponstate = WEAPON_READY;
		}
	}
}


//*******************
//   FORCE G EXPLODE
//*******************
void ForceGrenadeRock_Explode (edict_t *ent)
{
	vec3_t		origin;
	int			mod;
	edict_t	*sent = NULL;
	vec3_t	v;
	vec3_t	dir;

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	//FIXME: if we are onground then raise our Z just a bit since we are a point?
	/*if (ent->enemy)
	{
		float	points;
		vec3_t	v;
		vec3_t	dir;

		VectorAdd (ent->enemy->mins, ent->enemy->maxs, v);
		VectorMA (ent->enemy->s.origin, 0.5, v, v);
		VectorSubtract (ent->s.origin, v, v);
		points = ent->dmg - 0.5 * VectorLength (v);
		VectorSubtract (ent->enemy->s.origin, ent->s.origin, dir);
		if (ent->spawnflags & 1)
			mod = MOD_HANDGRENADE;
		else
			mod = MOD_GRENADE;
		T_Damage (ent->enemy, ent, ent->owner, dir, ent->s.origin, vec3_origin, (int)points, 100, DAMAGE_RADIUS, mod);
	}*/

	if (ent->spawnflags & 2)
		mod = MOD_HELD_GRENADE;
	else
		mod = MOD_HG_SPLASH;

	//push

	while ((sent = findradius(sent, ent->s.origin, 280)) != NULL)
	{
		if (!sent->takedamage)
			continue;

		VectorAdd (sent->mins, ent->maxs, v);
		VectorMA (sent->s.origin, 0.5, v, v);
		VectorSubtract (ent->s.origin, v, v);
		
		if (CanDamage (sent, ent))
		{
			VectorSubtract (sent->s.origin, ent->s.origin, dir);
			T_Damage (sent, ent, ent->owner, dir, ent->s.origin, vec3_origin, 25+random()*5, 600, DAMAGE_RADIUS, mod);
			if (sent->client)
			{
//				sent->s.event = EV_FALLFAR;
				//sent->pain_debounce_time = level.time;	// no normal pain sound
//				sent->forcer = ent->owner;
//				sent->client->force_framenum = level.framenum + 40;
			} else if (sent->svflags & SVF_MONSTER) {
				T_Damage (sent, ent, ent->owner, dir, ent->s.origin, vec3_origin, 60, 600, DAMAGE_RADIUS, mod);
			}
		}
	}

	//(*positioned_sound) (vec3_t origin, edict_t *ent, int channel, int soundinedex, float volume, float attenuation, float timeofs)
	gi.positioned_sound(ent->s.origin,ent,CHAN_WEAPON,gi.soundindex("makron/bfg_fire.wav"),1,ATTN_NORM,0);

	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BFG_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	G_FreeEdict (ent);
}
void ForceGrenade_Explode (edict_t *ent)
{
	vec3_t		origin;
	int			mod;
	edict_t	*sent = NULL;
	vec3_t	v;
	vec3_t	dir;

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	//FIXME: if we are onground then raise our Z just a bit since we are a point?
	/*if (ent->enemy)
	{
		float	points;
		vec3_t	v;
		vec3_t	dir;

		VectorAdd (ent->enemy->mins, ent->enemy->maxs, v);
		VectorMA (ent->enemy->s.origin, 0.5, v, v);
		VectorSubtract (ent->s.origin, v, v);
		points = ent->dmg - 0.5 * VectorLength (v);
		VectorSubtract (ent->enemy->s.origin, ent->s.origin, dir);
		if (ent->spawnflags & 1)
			mod = MOD_HANDGRENADE;
		else
			mod = MOD_GRENADE;
		T_Damage (ent->enemy, ent, ent->owner, dir, ent->s.origin, vec3_origin, (int)points, 100, DAMAGE_RADIUS, mod);
	}*/

	if (ent->spawnflags & 2)
		mod = MOD_HELD_GRENADE;
	else
		mod = MOD_HG_SPLASH;

	//push

	while ((sent = findradius(sent, ent->s.origin, 280)) != NULL)
	{
		if (!sent->takedamage)
			continue;

		VectorAdd (sent->mins, ent->maxs, v);
		VectorMA (sent->s.origin, 0.5, v, v);
		VectorSubtract (ent->s.origin, v, v);
		
		if (CanDamage (sent, ent))
		{
			VectorSubtract (sent->s.origin, ent->s.origin, dir);
			T_Damage (sent, ent, ent->owner, dir, ent->s.origin, vec3_origin, 25+random()*5, 600, DAMAGE_RADIUS, mod);
			if (sent->client)
			{
//				sent->s.event = EV_FALLFAR;
				//sent->pain_debounce_time = level.time;	// no normal pain sound
//				sent->forcer = ent->owner;
//				sent->client->force_framenum = level.framenum + 40;
			} else if (sent->svflags & SVF_MONSTER) {
				T_Damage (sent, ent, ent->owner, dir, ent->s.origin, vec3_origin, 60, 600, DAMAGE_RADIUS, mod);
			}
		}
	}

	//(*positioned_sound) (vec3_t origin, edict_t *ent, int channel, int soundinedex, float volume, float attenuation, float timeofs)
	gi.positioned_sound(ent->s.origin,ent,CHAN_WEAPON,gi.soundindex("makron/bfg_fire.wav"),1,ATTN_NORM,0);

	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BFG_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	G_FreeEdict (ent);
}
//*************
// TOUCH FORCE
//*************

void ForceGrenade_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	if (!other->takedamage)
	{
		if (ent->spawnflags & 1)
		{
			if (random() > 0.5)
				gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/hgrenb1a.wav"), 1, ATTN_NORM, 0);
			else
				gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/hgrenb2a.wav"), 1, ATTN_NORM, 0);
		}
		else
		{
			gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/grenlb1b.wav"), 1, ATTN_NORM, 0);
		}
		return;
	}

	ent->enemy = other;
	ForceGrenade_Explode (ent);
}

//***************
//  FIRE ROUTINES
//*************** 

void fire_forcegrenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius)
{
	edict_t	*grenade;
	vec3_t	dir;
	vec3_t	forward, right, up;

	vectoangles (aimdir, dir);
	AngleVectors (dir, forward, right, up);

	grenade = G_Spawn();
	VectorCopy (start, grenade->s.origin);
	VectorScale (aimdir, speed, grenade->velocity);
	VectorMA (grenade->velocity, 200 + crandom() * 10.0, up, grenade->velocity);
	VectorMA (grenade->velocity, crandom() * 10.0, right, grenade->velocity);
	VectorSet (grenade->avelocity, 300, 300, 300);
	grenade->movetype = MOVETYPE_BOUNCE;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	grenade->s.effects = EF_GRENADE | 0x00400000;
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	grenade->s.modelindex = gi.modelindex ("models/objects/grenade/tris.md2");
	grenade->owner = self;
	grenade->touch = ForceGrenade_Touch;
	grenade->nextthink = level.time + timer;
	grenade->think = ForceGrenade_Explode;
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "grenade";

	gi.linkentity (grenade);
}


void fire_forcegrenade2 (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held)
{
	edict_t	*grenade;
	vec3_t	dir;
	vec3_t	forward, right, up;

	vectoangles (aimdir, dir);
	AngleVectors (dir, forward, right, up);

	grenade = G_Spawn();
	VectorCopy (start, grenade->s.origin);
	VectorScale (aimdir, speed, grenade->velocity);
	VectorMA (grenade->velocity, 200 + crandom() * 10.0, up, grenade->velocity);
	VectorMA (grenade->velocity, crandom() * 10.0, right, grenade->velocity);
	VectorSet (grenade->avelocity, 300, 300, 300);
	grenade->movetype = MOVETYPE_MORE_BOUNCE;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	grenade->s.effects = EF_GRENADE | 0x00400000;
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	grenade->s.modelindex = gi.modelindex ("models/objects/grenade2/tris.md2");
	grenade->owner = self;
	grenade->touch = ForceGrenade_Touch;
	grenade->nextthink = level.time + timer;
	grenade->think = ForceGrenade_Explode;
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "hgrenade";
	if (held)
		grenade->spawnflags = 3;
	else
		grenade->spawnflags = 1;
	grenade->s.sound = gi.soundindex("weapons/hgrenc1b.wav");

	if (timer <= 0.0)
		ForceGrenade_Explode (grenade);
	else
	{
		gi.sound (self, CHAN_WEAPON, gi.soundindex ("weapons/hgrent1a.wav"), 1, ATTN_NORM, 0);
		gi.linkentity (grenade);
	}
}

/*
======================================================================

GRENADE LAUNCHER

======================================================================
*/

void weapon_grenadelauncher_fire (edict_t *ent)
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
	int		damage = 50;
	float	radius;

	radius = damage+40;
	if (is_quad)
		damage *= 4;

	if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
	{
		ent->client->ps.gunframe = 9;
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		return;
	}

	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	GrenLaunch_Source (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	if (ent->client->pers.abilities.gunner_forcegrenades)
		fire_forcegrenade (ent, start, forward, 20, 600, 2.5, radius);
	else
		fire_grenade (ent, start, forward, damage, 600, 2.5, radius);

	gi.sound(ent, CHAN_WEAPON, gi.soundindex ("gunner/gunatck3.wav"), 1, ATTN_NORM, 0);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ2_GUNNER_GRENADE_1 | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->s.frame = 117;
	ent->client->anim_end = 128;
}

void Weapon_GrenadeLauncher (edict_t *ent)
{
	static int	pause_frames[]	= {11, 20, 0};
	static int	fire_frames[]	= {5, 0};

	Weapon_Generic (ent, 3, 10, 21, 22, pause_frames, fire_frames, weapon_grenadelauncher_fire);
}

/*
======================================================================

ROCKET

======================================================================
*/

void Weapon_RocketLauncher_Fire (edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage;
	float	damage_radius;
	int		radius_damage;

	damage = 100 + (int)(random() * 20.0);
	radius_damage = 120;
	damage_radius = 120;
	if (is_quad)
	{
		damage *= 4;
		radius_damage *= 4;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_rocket (ent, start, forward, damage, 650, damage_radius, radius_damage);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_ROCKET | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	ent->client->anim_priority = ANIM_ATTACK;
	ent->s.frame = FRAME_attack1;
	ent->client->anim_end = FRAME_attack8;


	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_RocketLauncher (edict_t *ent)
{
	static int	pause_frames[]	= {25, 33, 42, 50, 0};
	static int	fire_frames[]	= {5, 0};

	Weapon_Generic (ent, 4, 12, 50, 54, pause_frames, fire_frames, Weapon_RocketLauncher_Fire);
}


void TankBlast_Fire (edict_t *ent, vec3_t g_offset, int damage, qboolean hyper, int effect)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;

	if (is_quad)
		damage *= 4;
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 24, 8, ent->viewheight-8);
	VectorAdd (offset, g_offset, offset);
	Left_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	fire_blaster (ent, start, forward, damage, 800, effect, 3);

	gi.sound(ent, CHAN_WEAPON, gi.soundindex ("tank/tnkatck3.wav"), 1, ATTN_NORM, 0);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	ent->s.frame = 63;
	ent->client->anim_end = 66;
}

void Weapon_TankBlast_Fire (edict_t *ent)
{
	int		damage;

	damage = 30;

	TankBlast_Fire (ent, vec3_origin, damage, false, EF_BLASTER);
	ent->client->ps.gunframe++;
}

void Weapon_TankBlast (edict_t *ent)
{
	static int	pause_frames[]	= {7, 16, 0};
	static int	fire_frames[]	= {4, 0};

	Weapon_Generic (ent, 2, 6, 17, 18, pause_frames, fire_frames, Weapon_TankBlast_Fire);
}

void Weapon_TankRock_Fire (edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage;
	float	damage_radius;
	int		radius_damage;
	vec3_t v;

	damage = 100 + (int)(random() * 20.0);
	radius_damage = 50+20;
	damage_radius = 50+20;
	if (is_quad)
	{
		damage *= 4;
		radius_damage *= 4;
	}

	if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
	{
		ent->client->ps.gunframe = 29;
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		return;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	VectorSet(offset, 8, 8, ent->viewheight+4);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	if (ent->client->pers.abilities.tank_dualrockets)
	{
		v[PITCH] = ent->client->v_angle[PITCH];
		v[YAW]   = ent->client->v_angle[YAW] - 5;
		v[ROLL]  = ent->client->v_angle[ROLL];
		AngleVectors (v, forward, NULL, NULL);
        fire_rocket (ent, start, forward, 50, 550, damage_radius, radius_damage);
		v[PITCH] = ent->client->v_angle[PITCH];
		v[YAW]   = ent->client->v_angle[YAW] + 5;
		v[ROLL]  = ent->client->v_angle[ROLL];
		AngleVectors (v, forward, NULL, NULL);
	}
	fire_rocket (ent, start, forward, 50, 550, damage_radius, radius_damage);


	gi.sound(ent, CHAN_WEAPON, gi.soundindex ("tank/tnkatck1.wav"), 1, ATTN_NORM, 0);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_TankRock (edict_t *ent)
{
	static int	pause_frames[]	= {29, 38, 0};
	static int	fire_frames[]	= {14, 17, 20, 0};

	Weapon_Generic (ent, 2, 28, 39, 40, pause_frames, fire_frames, Weapon_TankRock_Fire);
}

void airburst(vec3_t origin)
{
edict_t *ent=NULL;
vec3_t start,end;
vec3_t dir;
float radius=200;
//int i;
qboolean test=false;
 
 
VectorCopy(origin, start);
// Blow backward ALL ents within 200 units...
while ((ent = findradius(ent, origin, 200)) != NULL)
{
               if (!ent->takedamage)
                       continue;
 
 
 
        VectorCopy(ent->s.origin, end);
        // subtract ent's origin from grenade's origin to get direction..
        VectorSubtract(end, start, dir);
        // Scale new velocity into ent's velocity!
        VectorScale(dir,50,dir);
        VectorAdd(dir,ent->velocity, ent->velocity);
} // end while
}
 
 
 
void fire_airburst (edict_t *self, vec3_t start, vec3_t aimdir)
{
        vec3_t         end,forward;
        trace_t        tr;
        int            mask;
 
VectorMA (start, 8192, aimdir, end);
mask = MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA;
tr = gi.trace (start, NULL, NULL, end, self, mask);
if (tr.fraction==1)
        return;
 
if ((tr.ent->svflags & SVF_MONSTER) || (tr.ent->client))
{       
AngleVectors(self->s.angles, forward, NULL, NULL);
VectorScale(forward, 400, forward);
VectorAdd(forward, tr.ent->velocity, tr.ent->velocity);
}
else if (tr.ent->solid==(SOLID_BSP|SOLID_BBOX))
{
        PlayerNoise(self, tr.endpos, PNOISE_IMPACT);
        airburst(tr.endpos);
        return;
}              
 
}


void Weapon_TankSmash_Fire (edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage;
	float	damage_radius;
	int		radius_damage;

	damage = 150 + (int)(random() * 20.0);
	radius_damage = 50+20;
	damage_radius = 50+20;
	if (is_quad)
	{
		damage *= 4;
		radius_damage *= 4;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	VectorSet(offset, 8, 8, ent->viewheight+4);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	//fire_rocket (ent, start, forward, 50, 550, damage_radius, radius_damage);
	T_RadiusDamage (ent, ent, damage, ent, 600, MOD_TANKBLAST);
    //fire_airburst (ent, start, forward); // Anyone infront gets blown back!

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BFG_BIGEXPLOSION);
	gi.WritePosition (ent->s.origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	gi.sound(ent, CHAN_WEAPON, gi.soundindex ("tank/Tnkatck5.wav"), 1, ATTN_NORM, 0);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);
}

void Weapon_TankSmash (edict_t *ent)
{
	static int	pause_frames[]	= {40, 49, 0};
	static int	fire_frames[]	= {28, 0};

	Weapon_Generic (ent, 2, 39, 50, 51, pause_frames, fire_frames, Weapon_TankSmash_Fire);
}


void Weapon_IronRock_Fire (edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage;
	float	damage_radius;
	int		radius_damage;

	damage = 100 + (int)(random() * 20.0);
	if (ent->client->pers.abilities.chick_triplespread)
	{
		radius_damage = 200+20;
		damage_radius = 200+20;
	}
	else
	{
		radius_damage = 50+20;
		damage_radius = 50+20;
	}
	if (is_quad)
	{
		damage *= 4;
		radius_damage *= 4;
	}

	if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
	{
		ent->client->ps.gunframe = 29;
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		return;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	VectorSet(offset, 8, 8, ent->viewheight+4);
	GrenLaunch_Source (ent->client, ent->s.origin, offset, forward, right, start);

	fire_rocket (ent, start, forward, 50, 550, damage_radius, radius_damage);

	gi.sound(ent, CHAN_AUTO, gi.soundindex ("chick/Chkatck2.wav"), 1, ATTN_NORM, 0);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->s.frame = 13;
	ent->client->anim_end = 31;
}

void Weapon_IronRock (edict_t *ent)
{
	static int	pause_frames[]	= {34, 43, 0};
	static int	fire_frames[]	= {18, 0};

	Weapon_Generic (ent, 2, 33, 44, 45, pause_frames, fire_frames, Weapon_IronRock_Fire);
}

/*
======================================================================

BLASTER / HYPERBLASTER

======================================================================
*/

void HoverHyper_Fire (edict_t *ent, vec3_t g_offset, int damage, qboolean hyper, int effect)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;

	if (is_quad)
		damage *= 4;
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 24, 8, ent->viewheight-8);
	VectorAdd (offset, g_offset, offset);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	fire_blaster (ent, start, forward, damage, 1000, effect, 2);

	gi.sound(ent, CHAN_WEAPON, gi.soundindex ("hover/hovatck1.wav"), 1, ATTN_NORM, 0);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	ent->s.frame = 200;
	ent->client->anim_end = 204;
}

void Weapon_HoverHyper_Fire (edict_t *ent)
{
	int		damage;

	damage = 1;

	HoverHyper_Fire (ent, vec3_origin, damage, false, EF_HYPERBLASTER);
	ent->client->ps.gunframe++;
}

void Weapon_HoverHyper (edict_t *ent)
{
	static int	pause_frames[]	= {6, 15, 0};
	static int	fire_frames[]	= {3, 4, 0};

	Weapon_Generic (ent, 2, 5, 16, 17, pause_frames, fire_frames, Weapon_HoverHyper_Fire);
}

void FlyerHyper_Fire (edict_t *ent, vec3_t g_offset, int damage, qboolean hyper, int effect)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;

	if (is_quad)
		damage *= 4;
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 24, 8, ent->viewheight-8);
	VectorAdd (offset, g_offset, offset);
	Left_Flyer_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	fire_blaster (ent, start, forward, damage, 1000, effect, 2);
	ent->client->ps.gunframe++;

	gi.sound(ent, CHAN_WEAPON, gi.soundindex ("flyer/flyatck3.wav"), 1, ATTN_NORM, 0);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	PlayerNoise(ent, start, PNOISE_WEAPON);
}

void Weapon_FlyerHyper_Fire (edict_t *ent)
{
	int		damage;

	damage = 2;

	FlyerHyper_Fire (ent, vec3_origin, damage, false, EF_HYPERBLASTER);
}

void Weapon_FlyerHyper (edict_t *ent)
{
	static int	pause_frames[]	= {6, 14, 0};
	static int	fire_frames[]	= {0};

	Weapon_Generic (ent, 1, 5, 15, 16, pause_frames, fire_frames, Weapon_FlyerHyper_Fire);
}

void FloaterHyper_Fire (edict_t *ent, vec3_t g_offset, int damage, qboolean hyper, int effect)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;

	if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
	{
		if (ent->client->ps.gunframe == 7)
			ent->client->ps.gunframe = 6;
		else
			ent->client->ps.gunframe = 6;
	}
	else
		ent->client->ps.gunframe = 8;

	if (is_quad)
		damage *= 4;
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 24, 8, ent->viewheight-8);
	VectorAdd (offset, g_offset, offset);
	Middle_Source (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	fire_blaster (ent, start, forward, damage, 1000, effect, 2);

	gi.sound(ent, CHAN_WEAPON, gi.soundindex ("floater/Fltatck1.wav"), 1, ATTN_NORM, 0);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
	ent->s.frame = 36;
	ent->client->anim_end = 44;
}

void Weapon_FloaterHyper_Fire (edict_t *ent)
{
	int		damage;

	damage = 2;

	FloaterHyper_Fire (ent, vec3_origin, damage, false, EF_HYPERBLASTER);
}

void Weapon_FloaterHyper (edict_t *ent)
{
	static int	pause_frames[]	= {8, 17, 0};
	static int	fire_frames[]	= {6, 0};

	Weapon_Generic (ent, 2, 7, 18, 19, pause_frames, fire_frames, Weapon_FloaterHyper_Fire);
}



void Blaster_Fire (edict_t *ent, vec3_t g_offset, int damage, qboolean hyper, int effect)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;

	if (is_quad)
		damage *= 4;
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 24, 8, ent->viewheight-8);
	VectorAdd (offset, g_offset, offset);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	fire_blaster (ent, start, forward, damage, 600, effect, 4);

	//gi.sound(ent, CHAN_WEAPON, gi.soundindex ("soldier/solatck2.wav"), 1, ATTN_NORM, 0);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	ent->s.frame = 16;
	ent->client->anim_end = 19;
}

void Weapon_Blaster_Fire (edict_t *ent)
{
	int		damage;
	
	damage = 18;

	Blaster_Fire (ent, vec3_origin, damage, false, EF_BLASTER);
	gi.sound(ent, CHAN_WEAPON, gi.soundindex ("soldier/solatck2.wav"), 1, ATTN_NORM, 0);
	ent->client->ps.gunframe++;
}

void Weapon_Blaster (edict_t *ent)
{
	static int	pause_frames[]	= {11, 20, 0};
	static int	fire_frames[]	= {5, 0};

	Weapon_Generic (ent, 4, 10, 21, 24, pause_frames, fire_frames, Weapon_Blaster_Fire);

	if (ent->client->pers.abilities.soldier_haste)
	{
		static int	pause_frames[]	= {11, 20, 0};
		static int	fire_frames[]	= {5, 0};

		Weapon_Generic (ent, 4, 10, 21, 24, pause_frames, fire_frames, Weapon_Blaster_Fire);
	}
}

void MedicHyper_Fire (edict_t *ent, vec3_t g_offset, int damage, qboolean hyper, int effect)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;

	if (is_quad)
		damage *= 4;
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 24, 8, ent->viewheight-8);
	VectorAdd (offset, g_offset, offset);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	fire_blaster (ent, start, forward, damage, 600, effect, 2);

	gi.sound(ent, CHAN_WEAPON, gi.soundindex ("medic/medatck1.wav"), 1, ATTN_NORM, 0);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->s.frame = 206;
	ent->client->anim_end = 212;
}

void Weapon_MedicHyper_Fire (edict_t *ent)
{
	int		damage;

	damage = 5;

	MedicHyper_Fire (ent, vec3_origin, damage, false, EF_HYPERBLASTER);
	ent->client->ps.gunframe++;
}

void Weapon_MedicHyper (edict_t *ent)
{
	static int	pause_frames[]	= {18, 27, 0};
	static int	fire_frames[]	= {12, 13, 0};

	Weapon_Generic (ent, 2, 17, 28, 29, pause_frames, fire_frames, Weapon_MedicHyper_Fire);
}

extern void medic_heal (edict_t *self);
extern void medicplain_heal (edict_t *self);

void Weapon_MedicHeal_Fire (edict_t *self)
{
        vec3_t end,forward;
        trace_t tr;
 
        VectorCopy(self->s.origin,end);
        AngleVectors (self->client->v_angle, forward, NULL, NULL);
        end[0]=end[0]+forward[0]*1000;
        end[1]=end[1]+forward[1]*1000;
        end[2]=end[2]+forward[2]*1000;
 
        tr = gi.trace (self->s.origin, NULL, NULL, end, self, MASK_SHOT);
        if(tr.ent != NULL) 
        {
               self->enemy=tr.ent;
               medicplain_heal(self);
        }

		self->client->ps.gunframe++;

	if (self->client->ps.gunframe == 9)
	{
		gi.sound (self, CHAN_AUTO, gi.soundindex("medic/medatck2.wav"), 1, ATTN_NORM, 0);
	}
	if (self->client->ps.gunframe == 19)
	{
		gi.sound (self, CHAN_AUTO, gi.soundindex("medic/medatck5.wav"), 1, ATTN_NORM, 0);
	}
}

void Weapon_MedicHeal (edict_t *ent)
{
	static int	pause_frames[]	= {27, 36, 0};
	static int	fire_frames[]	= {8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 0};

	Weapon_Generic (ent, 2, 26, 37, 38, pause_frames, fire_frames, Weapon_MedicHeal_Fire);
}

void Weapon_HyperBlaster_Fire (edict_t *ent)
{
	float	rotation;
	vec3_t	offset;
	int		effect;
	int		damage;

	ent->client->weapon_sound = gi.soundindex("weapons/hyprbl1a.wav");

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe++;
	}
	else
	{
		if (! ent->client->pers.inventory[ent->client->ammo_index] )
		{
			if (level.time >= ent->pain_debounce_time)
			{
				gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
				ent->pain_debounce_time = level.time + 1;
			}
		}
		else
		{
			rotation = (ent->client->ps.gunframe - 5) * 2*M_PI/6;
			offset[0] = -4 * sin(rotation);
			offset[1] = 0;
			offset[2] = 4 * cos(rotation);

			if ((ent->client->ps.gunframe == 6) || (ent->client->ps.gunframe == 9))
				effect = EF_HYPERBLASTER;
			else
				effect = 0;
			if (deathmatch->value)
				damage = 15;
			else
				damage = 20;
			Blaster_Fire (ent, offset, damage, true, effect);

			sound (ent, CHAN_WEAPON, "weapons", "Hyprbf1a", ATTN_NORM);
			if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
				ent->client->pers.inventory[ent->client->ammo_index]--;

			ent->client->anim_priority = ANIM_ATTACK;
			if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
			{
				ent->s.frame = FRAME_crattak1 - 1;
				ent->client->anim_end = FRAME_crattak9;
			}
			else
			{
				ent->s.frame = FRAME_attack1 - 1;
				ent->client->anim_end = FRAME_attack8;
			}
		}

		ent->client->ps.gunframe++;
		if (ent->client->ps.gunframe == 12 && ent->client->pers.inventory[ent->client->ammo_index])
			ent->client->ps.gunframe = 6;
	}

	if (ent->client->ps.gunframe == 12)
	{
		gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/hyprbd1a.wav"), 1, ATTN_NORM, 0);
		ent->client->weapon_sound = 0;
	}

}

void Weapon_HyperBlaster (edict_t *ent)
{
	static int	pause_frames[]	= {0};
	static int	fire_frames[]	= {6, 7, 8, 9, 10, 11, 0};

	Weapon_Generic (ent, 5, 20, 49, 53, pause_frames, fire_frames, Weapon_HyperBlaster_Fire);
}

void GunChain_Fire (edict_t *ent)
{
//	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			damage = 3;
	int			kick = 4;
	vec3_t		offset;

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->ps.gunframe == 12)
		ent->client->ps.gunframe = 10;
	else
		ent->client->ps.gunframe = 10;

	if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
	{
		ent->client->ps.gunframe = 13;
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		return;
	}

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_GUNNERCHAIN);

	gi.sound(ent, CHAN_WEAPON, gi.soundindex ("gunner/gunatck2.wav"), 1, ATTN_NORM, 0);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ2_GUNNER_MACHINEGUN_1 | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->client->anim_priority = ANIM_ATTACK;
	ent->s.frame = 152;
	ent->client->anim_end = 158;
}

void Weapon_GunChain (edict_t *ent)
{
	static int	pause_frames[]	= {16, 25, 0};
	static int	fire_frames[]	= {10, 11, 12, 0};

	Weapon_Generic (ent, 2, 15, 26, 28, pause_frames, fire_frames, GunChain_Fire);
}

void InfMelee_Fire (edict_t *ent)
{
//	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			damage = (5 + (rand() % 5));
	int			kick = 50;
	vec3_t		offset;

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	fire_player_melee (ent, start, forward, 60, damage, kick, 1, MOD_INFMELEE);
	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);
}

void Weapon_InfMelee (edict_t *ent)
{
	static int	pause_frames[]	= {16, 25, 0};
	static int	fire_frames[]	= {11, 0};

	Weapon_Generic (ent, 4, 15, 26, 28, pause_frames, fire_frames, InfMelee_Fire);
}

void SharkBite_Fire (edict_t *ent)
{
//	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			damage = 45;
	int			kick = 0;
	vec3_t		offset;

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	fire_player_melee_brain (ent, start, forward, 100, damage, kick, 1, MOD_FLYSPIKES);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);
}

void Weapon_SharkBite (edict_t *ent)
{
	static int	pause_frames[]	= {22, 31, 0};
	static int	fire_frames[]	= {15, 20, 0};

	Weapon_Generic (ent, 2, 21, 32, 33, pause_frames, fire_frames, SharkBite_Fire);
}


void SharkTail_Fire (edict_t *ent)
{
//	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			damage = 35;
	int			kick = 0;
	vec3_t		offset;

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	fire_player_melee_brain (ent, start, forward, 100, damage, kick, 1, MOD_FLYSPIKES);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);
}

void Weapon_SharkTail (edict_t *ent)
{
	static int	pause_frames[]	= {22, 31, 0};
	static int	fire_frames[]	= {12, 15, 0};

	Weapon_Generic (ent, 2, 21, 32, 33, pause_frames, fire_frames, SharkTail_Fire);
}

void Weapon_FixbotBlast_Fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage;
	int			kick;

	damage = 95;
	kick = 100;

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -3, ent->client->kick_origin);
	ent->client->kick_angles[0] = -3;

	VectorSet(offset, 0, 7,  ent->viewheight-12);
	Middle_Source (ent->client, ent->s.origin, offset, forward, right, start);
	fire_blaster (ent, start, forward, damage, 800, EF_BLASTER, 1);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->client->anim_priority = ANIM_ATTACK;
	ent->s.frame = 183;
	ent->client->anim_end = 187;
}


void Weapon_FixbotBlast (edict_t *ent)
{
	static int	pause_frames[]	= {16, 25, 0};
	static int	fire_frames[]	= {7, 0};

	Weapon_Generic (ent, 2, 15, 26, 27, pause_frames, fire_frames, Weapon_FixbotBlast_Fire);
}

void Weapon_FixbotWeld_Fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage;
	int			kick;

	damage = 95;
	kick = 100;

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -3, ent->client->kick_origin);
	ent->client->kick_angles[0] = -3;

	VectorSet(offset, 0, 7,  ent->viewheight-12);
	Middle_Source (ent->client, ent->s.origin, offset, forward, right, start);
	fire_weld (ent, start, forward, 75, 8, 0, 1, MOD_BLASTER);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);
}


void Weapon_FixbotWeld (edict_t *ent)
{
	static int	pause_frames[]	= {26, 35, 0};
	static int	fire_frames[]	= {12, 13, 14, 15, 16, 17, 18, 0};

	Weapon_Generic (ent, 2, 25, 36, 37, pause_frames, fire_frames, Weapon_FixbotWeld_Fire);
}


void FlyerMelee_Fire (edict_t *ent)
{
//	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			damage = 5;
	int			kick = 0;
	vec3_t		offset;

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	fire_player_melee_flyer (ent, start, forward, 100, damage, kick, 1, MOD_FLYSPIKES);
	gi.sound(ent, CHAN_WEAPON, gi.soundindex ("flyer/flyatck2.wav"), 1, ATTN_NORM, 0);
	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);
}

void Weapon_FlyerMelee (edict_t *ent)
{
	static int	pause_frames[]	= {13, 22, 0};
	static int	fire_frames[]	= {5, 9, 0};

	Weapon_Generic (ent, 2, 12, 23, 24, pause_frames, fire_frames, FlyerMelee_Fire);
}

void FlyerKamikaze_Fire (edict_t *ent)
{
//	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			damage = 5;
	int			kick = 0;
	vec3_t		offset;

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	//fire_player_melee_flyer (ent, start, forward, 100, damage, kick, 1, MOD_FLYSPIKES);
	T_RadiusDamage (ent, ent, 500, NULL, 1000, MOD_BLASTER);
	//gi.sound(ent, CHAN_WEAPON, gi.soundindex ("flyer/flyatck2.wav"), 1, ATTN_NORM, 0);
	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);
}

void Weapon_FlyerKamikaze (edict_t *ent)
{
	static int	pause_frames[]	= {13, 22, 0};
	static int	fire_frames[]	= {5, 0};

	Weapon_Generic (ent, 2, 12, 23, 24, pause_frames, fire_frames, FlyerKamikaze_Fire);
}

void GladMelee_Fire (edict_t *ent)
{
//	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			damage = (5 + (rand() % 5));
	int			kick = 50;
	vec3_t		offset;

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	fire_player_melee_glad (ent, start, forward, 83, damage, kick, 1, MOD_GLADSPIKE);
	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);
}

void Weapon_GladMelee (edict_t *ent)
{
	static int	pause_frames[]	= {10, 19, 0};
	static int	fire_frames[]	= {6, 0};

	Weapon_Generic (ent, 2, 9, 20, 21, pause_frames, fire_frames, GladMelee_Fire);
}

void BrainTentacles_Fire (edict_t *ent)
{
//	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			damage = (5 + (rand() %5));
	int			kick = -600;
	vec3_t		offset;

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	Middle_Source (ent->client, ent->s.origin, offset, forward, right, start);

	fire_player_melee_berserk (ent, start, forward, 70, damage, kick, 1, MOD_BRAINTENTACLE);
	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);
}

void Weapon_BrainTentacles (edict_t *ent)
{
	static int	pause_frames[]	= {20, 29, 0};
	static int	fire_frames[]	= {9, 10, 11, 12, 0};

	Weapon_Generic (ent, 2, 19, 30, 31, pause_frames, fire_frames, BrainTentacles_Fire);
}

void BrainMelee_Fire (edict_t *ent)
{
//	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			damage = (13 + (rand() %7));
	int			kick = 40;
	vec3_t		offset;

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	Middle_Source (ent->client, ent->s.origin, offset, forward, right, start);

	fire_player_melee_brain (ent, start, forward, 80, damage, kick, 1, MOD_BRAINCLAWS);
	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);
}

void Weapon_BrainMelee (edict_t *ent)
{
	static int	pause_frames[]	= {22, 31, 0};
	static int	fire_frames[]	= {9, 14, 0};

	Weapon_Generic (ent, 2, 21, 32, 33, pause_frames, fire_frames, BrainMelee_Fire);
}

void MutantMelee_Fire (edict_t *ent)
{
//	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			damage = (23 + (rand() %18));
	int			kick = 0;
	vec3_t		offset;

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	Middle_Source (ent->client, ent->s.origin, offset, forward, right, start);

	fire_player_melee_mutant2 (ent, start, forward, 80, damage, kick, 1, MOD_MUTSLASH);
	gi.sound(ent, CHAN_WEAPON, gi.soundindex ("mutant/Mutatck1.wav"), 1, ATTN_NORM, 0);
	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);
}

void Weapon_MutantMelee (edict_t *ent)
{
	static int	pause_frames[]	= {16, 25, 0};
	static int	fire_frames[]	= {9, 11, 0};

	Weapon_Generic (ent, 2, 15, 26, 27, pause_frames, fire_frames, MutantMelee_Fire);
}


void BerserkLeft_Fire (edict_t *ent)
{
//	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			damage = (35 + (rand() % 20));
	int			kick = 400;
	vec3_t		offset;

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	fire_player_melee_berserk (ent, start, forward, 125, damage, kick, 1, MOD_BERCLUB);
	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);
}

void Weapon_BerserkLeft (edict_t *ent)
{
	static int	pause_frames[]	= {16, 25, 0};
	static int	fire_frames[]	= {9, 0};

	Weapon_Generic (ent, 2, 15, 26, 27, pause_frames, fire_frames, BerserkLeft_Fire);
}

void IronHand_Fire (edict_t *ent)
{
//	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			damage = (10 + (rand() %6));
	int			kick = 100;
	vec3_t		offset;

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	fire_player_melee_berserk (ent, start, forward, 100, damage, kick, 1, MOD_IRONHAND);
	gi.sound(ent, CHAN_WEAPON, gi.soundindex ("chick/Chkatck3.wav"), 1, ATTN_NORM, 0);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);
}

void Weapon_IronHand (edict_t *ent)
{
	static int	pause_frames[]	= {13, 23, 0};
	static int	fire_frames[]	= {9, 0};

	Weapon_Generic (ent, 2, 12, 24, 25, pause_frames, fire_frames, IronHand_Fire);
}

void BerserkRight_Fire (edict_t *ent)
{
//	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			damage = (35 + (rand() % 20));
	int			kick = 400;
	vec3_t		offset;

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	fire_player_melee_berserk (ent, start, forward, 125, damage, kick, 1, MOD_BERSPIKE);
	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);
}

void Weapon_BerserkRight (edict_t *ent)
{
	static int	pause_frames[]	= {10, 19, 0};
	static int	fire_frames[]	= {7, 0};

	Weapon_Generic (ent, 2, 9, 20, 21, pause_frames, fire_frames, BerserkRight_Fire);
}

void Boss1Chain_Fire (edict_t *ent)
{
//	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			damage = 6;
	int			kick = 4;
	vec3_t		offset;
//	vec3_t firevec; /*Added: Direction to fire*/

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
	{
		ent->client->ps.gunframe = 4;
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		return;
	}

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	//mod_GetLeadoffVec(ent, start, 5000, 1000, false, firevec);
	fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_STANKCHAIN);

	gi.sound(ent, CHAN_WEAPON, gi.soundindex ("infantry/infatck1.wav"), 1, ATTN_NORM, 0);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ2_INFANTRY_MACHINEGUN_1 | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->client->anim_priority = ANIM_ATTACK;
	ent->s.frame = 0;
	ent->client->anim_end = 2;
}

void Weapon_Boss1Chain (edict_t *ent)
{
	static int	pause_frames[]	= {4, 13, 0};
	static int	fire_frames[]	= {2, 3, 0};

	Weapon_Generic (ent, 1, 3, 13, 14, pause_frames, fire_frames, Boss1Chain_Fire);
}





void Weapon_Boss2Rock_Fire (edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage;
	float	damage_radius;
	int		radius_damage;
	vec3_t  v;

	damage = 100 + (int)(random() * 20.0);
	radius_damage = 50+20;
	damage_radius = 50+20;
	if (is_quad)
	{
		damage *= 4;
		radius_damage *= 4;
	}

	if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
	{
		ent->client->ps.gunframe = 14;
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		return;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	VectorSet(offset, 8, 8, ent->viewheight-12);
	Middle_Source (ent->client, ent->s.origin, offset, forward, right, start);

	v[PITCH] = ent->client->v_angle[PITCH];
	v[YAW]   = ent->client->v_angle[YAW] + 2;
	v[ROLL]  = ent->client->v_angle[ROLL];
	AngleVectors (v, forward, NULL, NULL);
	fire_rocket (ent, start, forward, 50, 500, damage_radius, radius_damage);
	v[PITCH] = ent->client->v_angle[PITCH];
	v[YAW]   = ent->client->v_angle[YAW] + 1;
	v[ROLL]  = ent->client->v_angle[ROLL];
	AngleVectors (v, forward, NULL, NULL);
	fire_rocket (ent, start, forward, 50, 500, damage_radius, radius_damage);
	v[PITCH] = ent->client->v_angle[PITCH];
	v[YAW]   = ent->client->v_angle[YAW] - 1;
	v[ROLL]  = ent->client->v_angle[ROLL];
	AngleVectors (v, forward, NULL, NULL);
	fire_rocket (ent, start, forward, 50, 500, damage_radius, radius_damage);
	v[PITCH] = ent->client->v_angle[PITCH];
	v[YAW]   = ent->client->v_angle[YAW] - 2;
	v[ROLL]  = ent->client->v_angle[ROLL];
	AngleVectors (v, forward, NULL, NULL);
	fire_rocket (ent, start, forward, 50, 500, damage_radius, radius_damage);

	gi.sound(ent, CHAN_WEAPON, gi.soundindex ("tank/rocket.wav"), 1, ATTN_NORM, 0);
	gi.sound(ent, CHAN_WEAPON, gi.soundindex ("tank/rocket.wav"), 1, ATTN_NORM, 0);
	gi.sound(ent, CHAN_WEAPON, gi.soundindex ("tank/rocket.wav"), 1, ATTN_NORM, 0);
	gi.sound(ent, CHAN_WEAPON, gi.soundindex ("tank/rocket.wav"), 1, ATTN_NORM, 0);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= 4;
	ent->s.frame = 102;
	ent->client->anim_end = 109;
}

void Weapon_Boss2Rock (edict_t *ent)
{
	static int	pause_frames[]	= {15, 24, 0};
	static int	fire_frames[]	= {8, 0};

	Weapon_Generic (ent, 2, 14, 25, 26, pause_frames, fire_frames, Weapon_Boss2Rock_Fire);
}





void Weapon_Boss1Rock_Fire (edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage;
	float	damage_radius;
	int		radius_damage;

	damage = 100 + (int)(random() * 20.0);
	radius_damage = 50+20;
	damage_radius = 50+20;
	if (is_quad)
	{
		damage *= 4;
		radius_damage *= 4;
	}

	if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
	{
		ent->client->ps.gunframe = 14;
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		return;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_rocket (ent, start, forward, 50, 500, damage_radius, radius_damage);

	gi.sound(ent, CHAN_WEAPON, gi.soundindex ("tank/rocket.wav"), 1, ATTN_NORM, 0);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_Boss1Rock (edict_t *ent)
{
	static int	pause_frames[]	= {14, 23, 0};
	static int	fire_frames[]	= {3, 6, 9, 0};

	Weapon_Generic (ent, 2, 13, 24, 25, pause_frames, fire_frames, Weapon_Boss1Rock_Fire);
}


void InfChain_Fire (edict_t *ent)
{
//	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			damage = 3;
	int			kick = 4;
	vec3_t		offset;

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->ps.gunframe == 4)
		ent->client->ps.gunframe = 5;
	else
		ent->client->ps.gunframe = 5;

	if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
	{
		ent->client->ps.gunframe = 5;
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		return;
	}

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_INFCHAIN);

	gi.sound(ent, CHAN_WEAPON, gi.soundindex ("infantry/infatck1.wav"), 1, ATTN_NORM, 0);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ2_INFANTRY_MACHINEGUN_1 | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->client->anim_priority = ANIM_ATTACK;
	ent->s.frame = 194;
	ent->client->anim_end = 198;
}

void Weapon_InfChain (edict_t *ent)
{
	static int	pause_frames[]	= {6, 15, 0};
	static int	fire_frames[]	= {5, 0};

	Weapon_Generic (ent, 4, 5, 16, 18, pause_frames, fire_frames, InfChain_Fire);
}


void Boss2Chains_Fire (edict_t *ent)
{
//	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			damage = 3;
	int			kick = 4;
	vec3_t		offset;

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
	{
		ent->client->ps.gunframe = 5;
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		return;
	}

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_bullet (ent, start, forward, 6, 4, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_BOSS2CH1);
	fire_bullet (ent, start, forward, 6, 4, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_BOSS2CH2);
	ent->client->ps.gunframe++;

	gi.sound(ent, CHAN_WEAPON, gi.soundindex ("infantry/infatck1.wav"), 1, ATTN_NORM, 0);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ2_INFANTRY_MACHINEGUN_1 | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= 2;

	ent->client->anim_priority = ANIM_ATTACK;
	ent->s.frame = 84;
	ent->client->anim_end = 91;
}

void Weapon_Boss2Chains (edict_t *ent)
{
	static int	pause_frames[]	= {6, 15, 0};
	static int	fire_frames[]	= {2, 3, 4, 5, 0};

	Weapon_Generic (ent, 2, 5, 16, 17, pause_frames, fire_frames, Boss2Chains_Fire);
}

void Boss3Chains_Fire (edict_t *ent)
{
//	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			damage = 3;
	int			kick = 4;
	vec3_t		offset;

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
	{
		ent->client->ps.gunframe = 7;
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		return;
	}

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_bullet (ent, start, forward, 20, 10, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_BOSS2CH1);
	fire_bullet (ent, start, forward, 20, 10, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_BOSS2CH2);
	ent->client->ps.gunframe++;

	gi.sound(ent, CHAN_WEAPON, gi.soundindex ("berserk/xfire.wav"), 1, ATTN_NORM, 0);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ2_INFANTRY_MACHINEGUN_1 | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= 2;

	ent->client->anim_priority = ANIM_ATTACK;
	ent->s.frame = 12;
	ent->client->anim_end = 17;
}

void Weapon_Boss3Chains (edict_t *ent)
{
	static int	pause_frames[]	= {7, 16, 0};
	static int	fire_frames[]	= {2, 3, 4, 5, 6, 0};

	Weapon_Generic (ent, 2, 6, 17, 18, pause_frames, fire_frames, Boss3Chains_Fire);
}
/*
======================================================================

MACHINEGUN / CHAINGUN

======================================================================
*/

//#define rndnum(y,z) ((random()*((z)-((y)+1)))+(y))

void TankMach_Fire (edict_t *ent)
{
//	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			damage = 20;
	int			kick = 4;
	vec3_t		offset;
	float r;

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->ps.gunframe == 2)
		ent->client->ps.gunframe = 3;
	else
		ent->client->ps.gunframe = 3;

	if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
	{
		ent->client->ps.gunframe = 5;
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		return;
	}

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_TANKMACHINEGUN);

	r = rndnum (1, 6);
	gi.sound (ent, CHAN_WEAPON, gi.soundindex(va("weapons/Tnkmc%i.wav", rand()%4+1)), 1, ATTN_NORM, 0);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->client->anim_priority = ANIM_ATTACK;
	ent->s.frame = 187;
	ent->client->anim_end = 196;
}

void Weapon_TankMach (edict_t *ent)
{
	static int	pause_frames[]	= {4, 13, 0};
	static int	fire_frames[]	= {3, 0};

	Weapon_Generic (ent, 2, 3, 14, 15, pause_frames, fire_frames, TankMach_Fire);
}

void Machinegun_Fire (edict_t *ent)
{
//	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			damage;
	int			kick = 4;
	vec3_t		offset;
	vec3_t		firevec;

	if (ent->client->resp.class == 36)
		damage = 18;
	else
		damage = 5;

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->ps.gunframe == 4)
		ent->client->ps.gunframe = 5;
	else
		ent->client->ps.gunframe = 5;

	if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
	{
		ent->client->ps.gunframe = 5;
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		return;
	}

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	mod_GetLeadoffVec(ent, start, 5000, 1000, false, firevec);
	fire_bullet (ent, start, firevec, damage, kick, 200, 350, MOD_SOLDMACHINEGUN);

	gi.sound(ent, CHAN_WEAPON, gi.soundindex ("soldier/solatck3.wav"), 1, ATTN_NORM, 0);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ2_SOLDIER_MACHINEGUN_1 | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->client->anim_priority = ANIM_ATTACK;
	ent->s.frame = 40;
	ent->client->anim_end = 41;
}

void Weapon_Machinegun (edict_t *ent)
{
	static int	pause_frames[]	= {6, 15, 0};
	static int	fire_frames[]	= {5, 0};

	Weapon_Generic (ent, 4, 5, 16, 19, pause_frames, fire_frames, Machinegun_Fire);

	if (ent->client->pers.abilities.soldier_haste)
	{
		static int	pause_frames[]	= {6, 15, 0};
		static int	fire_frames[]	= {5, 0};

		Weapon_Generic (ent, 4, 5, 16, 19, pause_frames, fire_frames, Machinegun_Fire);
	}
}

void Chaingun_Fire (edict_t *ent)
{
	int			i;
	int			shots;
	vec3_t		start;
	vec3_t		forward, right, up;
	float		r, u;
	vec3_t		offset;
	int			damage;
	int			kick = 2;

	if (deathmatch->value)
		damage = 6;
	else
		damage = 8;

	if (ent->client->ps.gunframe == 5)
		gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/chngnu1a.wav"), 1, ATTN_IDLE, 0);

	if ((ent->client->ps.gunframe == 14) && !(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe = 32;
		ent->client->weapon_sound = 0;
		return;
	}
	else if ((ent->client->ps.gunframe == 21) && (ent->client->buttons & BUTTON_ATTACK)
		&& ent->client->pers.inventory[ent->client->ammo_index])
	{
		ent->client->ps.gunframe = 15;
	}
	else
	{
		ent->client->ps.gunframe++;
	}

	if (ent->client->ps.gunframe == 22)
	{
		ent->client->weapon_sound = 0;
		gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/chngnd1a.wav"), 1, ATTN_IDLE, 0);
	}
	else
	{
		ent->client->weapon_sound = gi.soundindex("weapons/chngnl1a.wav");
	}

	ent->client->anim_priority = ANIM_ATTACK;
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->s.frame = FRAME_crattak1 - (ent->client->ps.gunframe & 1);
		ent->client->anim_end = FRAME_crattak9;
	}
	else
	{
		ent->s.frame = FRAME_attack1 - (ent->client->ps.gunframe & 1);
		ent->client->anim_end = FRAME_attack8;
	}

	if (ent->client->ps.gunframe <= 9)
		shots = 1;
	else if (ent->client->ps.gunframe <= 14)
	{
		if (ent->client->buttons & BUTTON_ATTACK)
			shots = 2;
		else
			shots = 1;
	}
	else
		shots = 3;

	if (ent->client->pers.inventory[ent->client->ammo_index] < shots)
		shots = ent->client->pers.inventory[ent->client->ammo_index];

	if (!shots)
	{
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		return;
	}

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	for (i=0 ; i<3 ; i++)
	{
		ent->client->kick_origin[i] = crandom() * 0.35;
		ent->client->kick_angles[i] = crandom() * 0.7;
	}

	for (i=0 ; i<shots ; i++)
	{
		// get start / end positions
		AngleVectors (ent->client->v_angle, forward, right, up);
		r = 7 + crandom()*4;
		u = crandom()*4;
		VectorSet(offset, 0, r, u + ent->viewheight-8);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

		fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_CHAINGUN);
	}

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte ((MZ_CHAINGUN1 + shots - 1) | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= shots;
}


void Weapon_Chaingun (edict_t *ent)
{
	static int	pause_frames[]	= {38, 43, 51, 61, 0};
	static int	fire_frames[]	= {5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 0};

	Weapon_Generic (ent, 4, 31, 61, 64, pause_frames, fire_frames, Chaingun_Fire);
}


/*
======================================================================

SHOTGUN / SUPERSHOTGUN

======================================================================
*/

void weapon_shotgun_fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage = 3;
	int			kick = 1;

	if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
	{
		ent->client->ps.gunframe = 15;
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		return;
	}

	if (ent->client->ps.gunframe == 9)
	{
		ent->client->ps.gunframe++;
		return;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -2;

	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	fire_shotgun (ent, start, forward, damage, kick, 500, 500, 19, MOD_SOLDSHOTGUN);
	
	gi.sound(ent, CHAN_WEAPON, gi.soundindex ("soldier/solatck1.wav"), 1, ATTN_NORM, 0);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ2_SOLDIER_SHOTGUN_1 | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->s.frame = 16;
	ent->client->anim_end = 29;
}

void Weapon_Shotgun (edict_t *ent)
{
	static int	pause_frames[]	= {15, 24, 0};
	static int	fire_frames[]	= {5, 9, 0};

	Weapon_Generic (ent, 4, 14, 25, 28, pause_frames, fire_frames, weapon_shotgun_fire);

	if (ent->client->pers.abilities.soldier_haste)
	{
		static int	pause_frames[]	= {15, 24, 0};
		static int	fire_frames[]	= {5, 9, 0};

		Weapon_Generic (ent, 4, 14, 25, 28, pause_frames, fire_frames, weapon_shotgun_fire);
	}
}





/*
======================================================================

RAILGUN

======================================================================
*/

void weapon_railgun_fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage;
	int			kick;

	damage = 50;
	kick = 100;

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -3, ent->client->kick_origin);
	ent->client->kick_angles[0] = -3;

	VectorSet(offset, 0, 7,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	if (ent->client->resp.class == 6)
		fire_rail (ent, start, forward, damage, kick, MOD_GLADRAIL);
	else
		fire_instlaser (ent, start, forward, 165, 35, MOD_RAILGUN);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}


void Weapon_Railgun (edict_t *ent)
{
	static int	pause_frames[]	= {16, 25, 0};
	static int	fire_frames[]	= {7, 0};

	Weapon_Generic (ent, 2, 15, 26, 27, pause_frames, fire_frames, weapon_railgun_fire);
}

void Weapon_MakronRail_Fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage;
	int			kick;

	damage = 50;
	kick = 100;

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -3, ent->client->kick_origin);
	ent->client->kick_angles[0] = -3;

	VectorSet(offset, 0, 7,  ent->viewheight-8);
	Middle_Source (ent->client, ent->s.origin, offset, forward, right, start);
	fire_rail (ent, start, forward, damage, kick, MOD_GLADRAIL);

	gi.sound(ent, CHAN_WEAPON, gi.soundindex ("weapons/RAILGF1A.WAV"), 1, ATTN_NORM, 0);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}


void Weapon_MakronRail (edict_t *ent)
{
	static int	pause_frames[]	= {19, 28, 0};
	static int	fire_frames[]	= {11, 0};

	Weapon_Generic (ent, 2, 18, 29, 30, pause_frames, fire_frames, Weapon_MakronRail_Fire);
}


/*
======================================================================

BFG10K

======================================================================
*/

void weapon_bfg_fire (edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
//	int		damage;
//	float	damage_radius;

	// cells can go down during windup (from power armor hits), so
	// check again and abort firing if we don't have enough now
	if (ent->client->pers.inventory[ent->client->ammo_index] < 20)
	{
		return;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);

	// make a big pitch kick with an inverse fall
	ent->client->v_dmg_pitch = -5;
	ent->client->v_dmg_roll = crandom()*2;
	ent->client->v_dmg_time = level.time + DAMAGE_TIME;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	Left_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_bfg (ent, start, forward, 50, 300, 300);
	gi.sound(ent, CHAN_WEAPON, gi.soundindex ("makron/bfg_fire.wav"), 1, ATTN_NORM, 0);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= 20;
}

void Weapon_BFG (edict_t *ent) 
{
	static int	pause_frames[]	= {10, 19, 0};
	static int	fire_frames[]	= {5, 0};

	Weapon_Generic (ent, 2, 9, 20, 21, pause_frames, fire_frames, weapon_bfg_fire);
} 


void MakronBlast_Fire (edict_t *ent, vec3_t g_offset, int damage, qboolean hyper, int effect)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;

	if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
	{
		if (ent->client->ps.gunframe == 2)
			ent->client->ps.gunframe = 3;
		else
			ent->client->ps.gunframe = 2;
	}

	if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
	{
		ent->client->ps.gunframe = 4;
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		return;
	}

	if (is_quad)
		damage *= 4;
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 24, 8, ent->viewheight-8);
	VectorAdd (offset, g_offset, offset);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	fire_blaster (ent, start, forward, damage, 1000, effect, 2);

	gi.sound(ent, CHAN_WEAPON, gi.soundindex ("makron/blaster.wav"), 1, ATTN_NORM, 0);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] --;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	ent->s.frame = 23;
	ent->client->anim_end = 33;
}

void Weapon_MakronBlast_Fire (edict_t *ent)
{
	int		damage;

	damage = 15;

	MakronBlast_Fire (ent, vec3_origin, damage, false, EF_BLASTER);
	ent->client->ps.gunframe++;
}

void Weapon_MakronBlast (edict_t *ent)
{
	static int	pause_frames[]	= {4, 13, 0};
	static int	fire_frames[]	= {3, 0};

	Weapon_Generic (ent, 2, 3, 14, 15, pause_frames, fire_frames, Weapon_MakronBlast_Fire);
}

//======================================================================

void FloaterMelee1_Fire (edict_t *ent)
{
//	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			damage = (30 + rand() % 6);
	int			kick = -50;
	vec3_t		offset;

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	Middle_Source (ent->client, ent->s.origin, offset, forward, right, start);

	fire_player_melee_berserk (ent, start, forward, 65, damage, kick, 1, MOD_BRAINTENTACLE);
	gi.sound(ent, CHAN_WEAPON, gi.soundindex ("floater/Fltatck3.wav"), 1, ATTN_NORM, 0);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);
}

void Weapon_FloaterMelee1 (edict_t *ent)
{
	static int	pause_frames[]	= {13, 22, 0};
	static int	fire_frames[]	= {10, 0};

	Weapon_Generic (ent, 2, 12, 23, 24, pause_frames, fire_frames, FloaterMelee1_Fire);
}

void FloaterMelee2_Fire (edict_t *ent)
{
//	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			damage = (35 + rand() % 6);
	int			kick = -50;
	vec3_t		offset;

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	Middle_Source (ent->client, ent->s.origin, offset, forward, right, start);

	fire_player_melee_berserk (ent, start, forward, 80, damage, kick, 1, MOD_BRAINTENTACLE);
	gi.sound(ent, CHAN_WEAPON, gi.soundindex ("floater/Fltatck2.wav"), 1, ATTN_NORM, 0);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);
}

void Weapon_FloaterMelee2 (edict_t *ent)
{
	static int	pause_frames[]	= {12, 21, 0};
	static int	fire_frames[]	= {10, 0};

	Weapon_Generic (ent, 2, 11, 22, 23, pause_frames, fire_frames, FloaterMelee2_Fire);
}
// Parasite
extern void parasite_drain (edict_t *self);
extern void parasite_drain_attack_health (edict_t *self);

void Weapon_Parasite_Suck_Fire (edict_t *self) 
{
        vec3_t end,forward;
        trace_t tr;
 
        VectorCopy(self->s.origin,end);
        AngleVectors (self->client->v_angle, forward, NULL, NULL);
        end[0]=end[0]+forward[0]*1000;
        end[1]=end[1]+forward[1]*1000;
        end[2]=end[2]+forward[2]*1000;
 
        tr = gi.trace (self->s.origin, NULL, NULL, end, self, MASK_SHOT);
        if(tr.ent != NULL) 
        {
               self->enemy=tr.ent;
               parasite_drain(self);
        }

		self->client->ps.gunframe++;
}

void Weapon_Parasite_Suck (edict_t *ent)
{
	static int	pause_frames[]	= {20, 29, 0};
	static int	fire_frames[]	= {4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0};

	Weapon_Generic (ent, 2, 19, 30, 31, pause_frames, fire_frames, Weapon_Parasite_Suck_Fire);
}

void Weapon_Parasite_Drain_Fire (edict_t *self) 
{
        vec3_t end,forward;
        trace_t tr;
 
        VectorCopy(self->s.origin,end);
        AngleVectors (self->client->v_angle, forward, NULL, NULL);
        end[0]=end[0]+forward[0]*1000;
        end[1]=end[1]+forward[1]*1000;
        end[2]=end[2]+forward[2]*1000;
 
        tr = gi.trace (self->s.origin, NULL, NULL, end, self, MASK_SHOT);
        if(tr.ent != NULL) 
        {
               self->enemy=tr.ent;
               parasite_drain_attack_health(self);
        }

		self->client->ps.gunframe++;
}

void Weapon_Parasite_Drain (edict_t *ent)
{
	static int	pause_frames[]	= {20, 29, 0};
	static int	fire_frames[]	= {4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0};

	Weapon_Generic (ent, 2, 19, 30, 31, pause_frames, fire_frames, Weapon_Parasite_Drain_Fire);
}

// All Mission Pack Weapons Below This Line!
// Paril
// Mission Pack 1 - Xatrix

// Beta Iron Maiden
void Weapon_BetaIronRock_Fire (edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage;
	float	damage_radius;
	int		radius_damage;

	damage = 100 + (int)(random() * 20.0);
	radius_damage = 50+20;
	damage_radius = 50+20;
	if (is_quad)
	{
		damage *= 4;
		radius_damage *= 4;
	}

	if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
	{
		ent->client->ps.gunframe = 29;
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		return;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	VectorSet(offset, 8, 8, ent->viewheight+4);
	GrenLaunch_Source (ent->client, ent->s.origin, offset, forward, right, start);

	fire_rockheat (ent, start, forward, 30, 550, damage_radius, radius_damage);

	gi.sound(ent, CHAN_AUTO, gi.soundindex ("chick/Chkatck2.wav"), 1, ATTN_NORM, 0);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->s.frame = 13;
	ent->client->anim_end = 31;
}

void Weapon_BetaIronRock (edict_t *ent)
{
	static int	pause_frames[]	= {34, 43, 0};
	static int	fire_frames[]	= {18, 0};

	Weapon_Generic (ent, 2, 33, 44, 45, pause_frames, fire_frames, Weapon_BetaIronRock_Fire);
}

// Gekk

void GekkSpit_Fire (edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage;
	float	damage_radius;
	int		radius_damage;

	damage = 100 + (int)(random() * 20.0);
	radius_damage = 50+20;
	damage_radius = 50+20;
	if (is_quad)
	{
		damage *= 4;
		radius_damage *= 4;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	VectorSet(offset, 8, 8, ent->viewheight+4);
	Middle_Source (ent->client, ent->s.origin, offset, forward, right, start);

	fire_loogie (ent, start, forward, 15, 500);

	gi.sound(ent, CHAN_AUTO, gi.soundindex ("gek/gk_atck1.wav"), 1, ATTN_NORM, 0);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);
}

void Weapon_GekkSpit (edict_t *ent)
{
	static int	pause_frames[]	= {9, 18, 0};
	static int	fire_frames[]	= {7, 0};

	Weapon_Generic (ent, 2, 8, 19, 20, pause_frames, fire_frames, GekkSpit_Fire);
}

void GekkMelee_Fire (edict_t *ent)
{
//	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			damage = (15 + (rand() %5));
	int			kick = 50;
	vec3_t		offset;

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	fire_player_melee (ent, start, forward, 60, damage, kick, 1, MOD_BLASTER);

	gi.sound(ent, CHAN_AUTO, gi.soundindex ("gek/gk_atck3.wav"), 1, ATTN_NORM, 0);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);
}

void Weapon_GekkMelee (edict_t *ent)
{
	static int	pause_frames[]	= {11, 20, 0};
	static int	fire_frames[]	= {6, 8, 0};

	Weapon_Generic (ent, 2, 10, 21, 22, pause_frames, fire_frames, GekkMelee_Fire);
}

// BETA Guards
void BetaLaser_Fire (edict_t *ent)
{
//	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			damage = 1;
	int			kick = 2;
	vec3_t		offset;
	vec3_t		firevec;

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->ps.gunframe == 4)
		ent->client->ps.gunframe = 5;
	else
		ent->client->ps.gunframe = 5;

	if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
	{
		ent->client->ps.gunframe = 5;
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		return;
	}

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	mod_GetLeadoffVec(ent, start, 5000, 1000, false, firevec);
	fire_laser (ent, start, forward, 3);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ2_SOLDIER_MACHINEGUN_1 | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->client->anim_priority = ANIM_ATTACK;
	ent->s.frame = 40;
	ent->client->anim_end = 41;
}

void Weapon_BetaLaser (edict_t *ent)
{
	static int	pause_frames[]	= {6, 15, 0};
	static int	fire_frames[]	= {5, 0};

	Weapon_Generic (ent, 4, 5, 16, 19, pause_frames, fire_frames, BetaLaser_Fire);
}

void BetaRipper_Fire (edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage;
	float	damage_radius;
	int		radius_damage;

	damage = 100 + (int)(random() * 20.0);
	radius_damage = 50+20;
	damage_radius = 50+20;
	if (is_quad)
	{
		damage *= 4;
		radius_damage *= 4;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	if (ent->client->pers.abilities.soldierb_speed)
		fire_ionripper (ent, start, forward, 6, 1500, EF_IONRIPPER);
	else
		fire_ionripper (ent, start, forward, 6, 700, EF_IONRIPPER);

	gi.sound(ent, CHAN_AUTO, gi.soundindex ("weapons/rippfire.wav"), 1, ATTN_NORM, 0);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= 2;

	ent->s.frame = 16;
	ent->client->anim_end = 19;
}

void Weapon_BetaRipper (edict_t *ent)
{
	static int	pause_frames[]	= {9, 18, 0};
	static int	fire_frames[]	= {7, 8, 0};

	Weapon_Generic (ent, 2, 8, 19, 20, pause_frames, fire_frames, BetaRipper_Fire);
}

void BetaHyper_Fire (edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage;
	float	damage_radius;
	int		radius_damage;

	damage = 100 + (int)(random() * 20.0);
	radius_damage = 50+20;
	damage_radius = 50+20;
	if (is_quad)
	{
		damage *= 4;
		radius_damage *= 4;
	}
	if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
	{
		ent->client->weapon_sound = gi.soundindex("weapons/hyprbl1a.wav");
	}
	else
		ent->client->weapon_sound = 0;

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	if (ent->client->pers.abilities.soldierb_speed)
		fire_blueblaster (ent, start, forward, 4, 1600, EF_BLUEHYPERBLASTER);
	else
		fire_blueblaster (ent, start, forward, 4, 600, EF_BLUEHYPERBLASTER);
	gi.sound (ent, CHAN_WEAPON, gi.soundindex("weapons/Tnkmc1.wav"), 1, ATTN_NORM, 0);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] --;

	ent->s.frame = 13;
	ent->client->anim_end = 19;
}

void Weapon_BetaHyper (edict_t *ent)
{
	static int	pause_frames[]	= {9, 18, 0};
	static int	fire_frames[]	= {6, 7, 8, 0};

	Weapon_Generic (ent, 2, 8, 19, 20, pause_frames, fire_frames, BetaHyper_Fire);
}

// BETA Glad

void Weapon_BetaPlasma_Fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage;
	int			kick;

	damage = 50;
	kick = 100;

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -3, ent->client->kick_origin);
	ent->client->kick_angles[0] = -3;

	VectorSet(offset, 0, 7,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	//fire_rail (ent, start, forward, damage, kick, MOD_GLADRAIL);
	fire_plasma (ent, start, forward, 100, 725, 60, 60);
	//gi.sound (ent, CHAN_WEAPON, gi.soundindex("weapons/plasshot.wav"), 1, ATTN_NORM, 0);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}


void Weapon_BetaPlasma (edict_t *ent)
{
	static int	pause_frames[]	= {16, 25, 0};
	static int	fire_frames[]	= {6, 8, 0};

	Weapon_Generic (ent, 2, 15, 26, 27, pause_frames, fire_frames, Weapon_BetaPlasma_Fire);
}

// BETA Brain


void BetaBrainTentacles_Fire (edict_t *ent)
{
//	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			damage = (5 + (rand() %5));
	int			kick = -600;
	vec3_t		offset;
	trace_t tr;
	vec3_t  end;

	VectorCopy(ent->s.origin, start);
	start[2] += ent->viewheight;
	AngleVectors(ent->client->v_angle, forward, NULL, NULL);
	VectorMA(start, 1200, forward, end);
	tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT);
	if ( tr.ent && ((tr.ent->svflags & SVF_MONSTER) || (tr.ent->client)) )
	{
        VectorScale(forward, -400, forward);
        VectorAdd(forward, tr.ent->velocity, tr.ent->velocity);
	}
	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	Middle_Source (ent->client, ent->s.origin, offset, forward, right, start);
	if (ent->client->pers.abilities.bbrain_tentacles)
		fire_player_melee_bbrain_suck (ent, start, forward, 50, 2, 0, 1, MOD_BRAINTENTACLE);
	//fire_player_melee_berserk (ent, start, forward, 70, damage, kick, 1, MOD_BRAINTENTACLE);
	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);
}

void Weapon_BetaBrainTentacles (edict_t *ent)
{
	static int	pause_frames[]	= {20, 29, 0};
	static int	fire_frames[]	= {9, 10, 11, 12, 0};

	Weapon_Generic (ent, 2, 19, 30, 31, pause_frames, fire_frames, BetaBrainTentacles_Fire);
}

void BrainLaser_Fire (edict_t *ent)
{
//	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			damage = 1;
	int			kick = 2;
	vec3_t		offset;
//	vec3_t		firevec;
	vec3_t		v;

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->ps.gunframe == 2)
		ent->client->ps.gunframe = 3;
	else
		ent->client->ps.gunframe = 3;

	if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
	{
		ent->client->ps.gunframe = 4;
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		return;
	}

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight+4);
	Middle_Source (ent->client, ent->s.origin, offset, forward, right, start);

	v[PITCH] = ent->client->v_angle[PITCH];
	v[YAW]   = ent->client->v_angle[YAW] - 2;
	v[ROLL]  = ent->client->v_angle[ROLL];
	AngleVectors (v, forward, NULL, NULL);
	fire_laser (ent, start, forward, 2);
	v[YAW]   = ent->client->v_angle[YAW] + 2;
	AngleVectors (v, forward, NULL, NULL);
	fire_laser (ent, start, forward, 2);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ2_SOLDIER_MACHINEGUN_1 | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= 2;

	ent->client->anim_priority = ANIM_ATTACK;
	ent->s.frame = 40;
	ent->client->anim_end = 41;
}

void Weapon_BrainLaser (edict_t *ent)
{
	static int	pause_frames[]	= {4, 13, 0};
	static int	fire_frames[]	= {3, 0};

	Weapon_Generic (ent, 2, 3, 14, 15, pause_frames, fire_frames, BrainLaser_Fire);
}






// MP2 - Rogue / Ground Zero Weapons!

// Daedalus
void DaedalusHyper_Fire (edict_t *ent, vec3_t g_offset, int damage, qboolean hyper, int effect)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;

	if (is_quad)
		damage *= 4;
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 24, 8, ent->viewheight-8);
	VectorAdd (offset, g_offset, offset);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	fire_blaster2 (ent, start, forward, damage, 1000, effect, hyper);

	gi.sound(ent, CHAN_WEAPON, gi.soundindex ("tank/tnkatck3.wav"), 1, ATTN_NORM, 0);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	ent->s.frame = 200;
	ent->client->anim_end = 204;
}

void Weapon_DaedalusHyper_Fire (edict_t *ent)
{
	int		damage;

	damage = 3;

	DaedalusHyper_Fire (ent, vec3_origin, damage, false, EF_BLASTER);
	ent->client->ps.gunframe++;
}

void Weapon_DaedalusHyper (edict_t *ent)
{
	static int	pause_frames[]	= {6, 15, 0};
	static int	fire_frames[]	= {3, 4, 0};

	Weapon_Generic (ent, 2, 5, 16, 17, pause_frames, fire_frames, Weapon_DaedalusHyper_Fire);
}

// Stalker
void StalkMelee_Fire (edict_t *ent)
{
//	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			damage = (23 + (rand() % 12));
	int			kick = 50;
	vec3_t		offset;

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	fire_player_melee_stalk (ent, start, forward, 60, damage, kick, 1, MOD_BLASTER);
	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);
}

void Weapon_StalkMelee (edict_t *ent)
{
	static int	pause_frames[]	= {15, 24, 0};
	static int	fire_frames[]	= {6, 12, 0};

	Weapon_Generic (ent, 2, 14, 25, 26, pause_frames, fire_frames, StalkMelee_Fire);
}

void StalkBlast_Fire (edict_t *ent, vec3_t g_offset, int damage, qboolean hyper, int effect)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;

	if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
	{
		ent->client->ps.gunframe = 18;
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		return;
	}

	if (is_quad)
		damage *= 4;
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 24, 8, ent->viewheight);
	VectorAdd (offset, g_offset, offset);
	Middle_Source (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	if (ent->client->pers.abilities.stalker_homing)
		fire_blaster2_homing (ent, start, forward, damage, 1000, effect, hyper);
	else
		fire_blaster2 (ent, start, forward, damage, 1000, effect, hyper);

	gi.sound(ent, CHAN_WEAPON, gi.soundindex ("tank/tnkatck3.wav"), 1, ATTN_NORM, 0);


	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	PlayerNoise(ent, start, PNOISE_WEAPON);
}

void Weapon_StalkBlast_Fire (edict_t *ent)
{
	int		damage;

	damage = 30;

	StalkBlast_Fire (ent, vec3_origin, damage, false, EF_BLASTER);
	ent->client->ps.gunframe++;
}

void Weapon_StalkBlast (edict_t *ent)
{
	static int	pause_frames[]	= {7, 16, 0};
	static int	fire_frames[]	= {5, 0};

	Weapon_Generic (ent, 2, 6, 17, 18, pause_frames, fire_frames, Weapon_StalkBlast_Fire);
}

// Medic Commander Hyperblaster
void MedicCmdHyper_Fire (edict_t *ent, vec3_t g_offset, int damage, qboolean hyper, int effect)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;

	if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
	{
		ent->client->ps.gunframe = 18;
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		return;
	}

	if (is_quad)
		damage *= 4;
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 24, 8, ent->viewheight-8);
	VectorAdd (offset, g_offset, offset);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	fire_blaster2 (ent, start, forward, damage, 600, effect, hyper);

	gi.sound(ent, CHAN_WEAPON, gi.soundindex ("tank/tnkatck3.wav"), 1, ATTN_NORM, 0);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->s.frame = 206;
	ent->client->anim_end = 212;
}

void Weapon_MedicCmdHyper_Fire (edict_t *ent)
{
	int		damage;

	damage = 6;

	MedicCmdHyper_Fire (ent, vec3_origin, damage, false, EF_BLASTER);
	ent->client->ps.gunframe++;
}

void Weapon_MedicCmdHyper (edict_t *ent)
{
	static int	pause_frames[]	= {18, 27, 0};
	static int	fire_frames[]	= {12, 13, 0};

	Weapon_Generic (ent, 2, 17, 28, 29, pause_frames, fire_frames, Weapon_MedicCmdHyper_Fire);
}

void Weapon_MedicCmdSpawn_Fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage;
	int			kick;
	int mon_type;

	damage = 65;
	kick = 100;

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	mon_type = rndnum (0, 22);

	if (mon_type == 0 || mon_type == 1 || mon_type == 2 || mon_type == 3)
		mon_type = 5;
	if (mon_type == 4 || mon_type == 6 || mon_type == 8 || mon_type == 9)
		mon_type = 7;
	if (mon_type == 10 || mon_type == 11)
		mon_type = 13;
	if (mon_type == 19)
		mon_type = 14;
	if (mon_type == 20 ||mon_type == 12 || mon_type == 18)
		mon_type = /*15*/14; // Brain removed.
	if (mon_type == 21)
		mon_type = 16;
	if (mon_type == 22)
		mon_type = 17;
	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -3, ent->client->kick_origin);
	ent->client->kick_angles[0] = -3;

	VectorSet(offset, 0, 7,  ent->viewheight-12);
	Middle_Source (ent->client, ent->s.origin, offset, forward, right, start);
	//fire_rail (ent, start, forward, damage, kick, MOD_GLADRAIL);

	/*if (*/Spawn_Monster(ent, /*M_FLYER*/mon_type);//) 
	//{
	//	Spawn_Timer(ent);
	//}
	gi.sound(ent, CHAN_AUTO, gi.soundindex ("medic_commander/monsterspawn1.wav"), 1, ATTN_IDLE, 0);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]-= 30;
}


void Weapon_MedicCmdSpawn (edict_t *ent)
{
	static int	pause_frames[]	= {27, 36, 0};
	static int	fire_frames[]	= {18, 0};

	Weapon_Generic (ent, 2, 26, 37, 38, pause_frames, fire_frames, Weapon_MedicCmdSpawn_Fire);
}

void Weapon_MedicCmdHeal_Fire (edict_t *self)
{
        vec3_t end,forward;
        trace_t tr;
 
        VectorCopy(self->s.origin,end);
        AngleVectors (self->client->v_angle, forward, NULL, NULL);
        end[0]=end[0]+forward[0]*1000;
        end[1]=end[1]+forward[1]*1000;
        end[2]=end[2]+forward[2]*1000;
 
        tr = gi.trace (self->s.origin, NULL, NULL, end, self, MASK_SHOT);
        if(tr.ent != NULL) 
        {
               self->enemy=tr.ent;
               medic_heal(self);
        }

		self->client->ps.gunframe++;

	if (self->client->ps.gunframe == 9)
	{
		gi.sound (self, CHAN_AUTO, gi.soundindex("medic_commander/medatck2c.wav"), 1, ATTN_NORM, 0);
	}
	if (self->client->ps.gunframe == 19)
	{
		gi.sound (self, CHAN_AUTO, gi.soundindex("medic_commander/medatck5a.wav"), 1, ATTN_NORM, 0);
	}
}


void Weapon_MedicCmdHeal (edict_t *ent)
{
	static int	pause_frames[]	= {27, 36, 0};
	static int	fire_frames[]	= {8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 0};

	Weapon_Generic (ent, 2, 26, 37, 38, pause_frames, fire_frames, Weapon_MedicCmdHeal_Fire);
}

// Carrier

void CarrierChains_Fire (edict_t *ent)
{
//	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			damage = 3;
	int			kick = 4;
	vec3_t		offset;

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
	{
		ent->client->ps.gunframe = 18;
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		return;
	}

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_bullet (ent, start, forward, 6, 4, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_BOSS2CH1);
	fire_bullet (ent, start, forward, 6, 4, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_BOSS2CH2);
	ent->client->ps.gunframe++;

	gi.sound(ent, CHAN_WEAPON, gi.soundindex ("infantry/infatck1.wav"), 1, ATTN_NORM, 0);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ2_INFANTRY_MACHINEGUN_1 | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= 2;

	ent->client->anim_priority = ANIM_ATTACK;
}

void Weapon_CarrierChains (edict_t *ent)
{
	static int	pause_frames[]	= {17, 26, 0};
	static int	fire_frames[]	= {3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 0};

	Weapon_Generic (ent, 2, 16, 27, 28, pause_frames, fire_frames, CarrierChains_Fire);
}

void Weapon_CarrierLaunch_Fire (edict_t *ent)
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
	int		damage = 85;
	float	radius;

	radius = damage+40;
	if (is_quad)
		damage *= 4;

	if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
	{
		ent->client->ps.gunframe = 19;
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		return;
	}

	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	GrenLaunch_Source (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	fire_grenade (ent, start, forward, damage, 600, 2.5, radius);

	gi.sound(ent, CHAN_WEAPON, gi.soundindex ("gunner/gunatck3.wav"), 1, ATTN_NORM, 0);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ2_GUNNER_GRENADE_1 | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->s.frame = 28;
	ent->client->anim_end = 43;
}

void Weapon_CarrierLaunch (edict_t *ent)
{
	static int	pause_frames[]	= {18, 27, 0};
	static int	fire_frames[]	= {8, 0};

	Weapon_Generic (ent, 3, 17, 28, 29, pause_frames, fire_frames, Weapon_CarrierLaunch_Fire);
}
void Weapon_CarrierRail_Fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage;
	int			kick;

	damage = 65;
	kick = 100;

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -3, ent->client->kick_origin);
	ent->client->kick_angles[0] = -3;

	VectorSet(offset, 0, 7,  ent->viewheight-12);
	Middle_Source (ent->client, ent->s.origin, offset, forward, right, start);
	fire_rail (ent, start, forward, damage, kick, MOD_GLADRAIL);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->s.frame = 13;
	ent->client->anim_end = 27;
}


void Weapon_CarrierRail (edict_t *ent)
{
	static int	pause_frames[]	= {16, 25, 0};
	static int	fire_frames[]	= {7, 0};

	Weapon_Generic (ent, 2, 15, 26, 27, pause_frames, fire_frames, Weapon_CarrierRail_Fire);
}

void Weapon_CarrierSpawn_Fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage;
	int			kick;

	damage = 65;
	kick = 100;

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -3, ent->client->kick_origin);
	ent->client->kick_angles[0] = -3;

	VectorSet(offset, 0, 7,  ent->viewheight-12);
	Middle_Source (ent->client, ent->s.origin, offset, forward, right, start);
	//fire_rail (ent, start, forward, damage, kick, MOD_GLADRAIL);

	if (Spawn_Monster(ent, M_FLYER)) 
	{
		Spawn_Timer(ent);
	}
	gi.sound(ent, CHAN_AUTO, gi.soundindex ("medic_commander/monsterspawn1.wav"), 1, ATTN_IDLE, 0);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]-= 20;

	ent->s.frame = 44;
	ent->client->anim_end = 61;
}


void Weapon_CarrierSpawn (edict_t *ent)
{
	static int	pause_frames[]	= {16, 25, 0};
	static int	fire_frames[]	= {7, 0};

	Weapon_Generic (ent, 2, 15, 26, 27, pause_frames, fire_frames, Weapon_CarrierSpawn_Fire);
}

// Widow (Walking)
void Weapon_WidowRail_Fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage;
	int			kick;

	damage = 45;
	kick = 100;

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -3, ent->client->kick_origin);
	ent->client->kick_angles[0] = -3;

	VectorSet(offset, 0, 7,  ent->viewheight-8);
	Left_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_rail (ent, start, forward, damage, kick, MOD_GLADRAIL);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->s.frame = 36;
	ent->client->anim_end = 40;
}


void Weapon_WidowRail (edict_t *ent)
{
	static int	pause_frames[]	= {11, 20, 0};
	static int	fire_frames[]	= {6, 0};

	Weapon_Generic (ent, 2, 10, 21, 22, pause_frames, fire_frames, Weapon_WidowRail_Fire);
}

void WidowBlast_Fire (edict_t *ent, vec3_t g_offset, int damage, qboolean hyper, int effect)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;

	if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
	{
		ent->client->ps.gunframe = 6;
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		return;
	}

	if (is_quad)
		damage *= 4;
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 24, 8, ent->viewheight-8);
	VectorAdd (offset, g_offset, offset);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	fire_blaster2 (ent, start, forward, damage, 1200, effect, hyper);

	gi.sound(ent, CHAN_WEAPON, gi.soundindex ("tank/tnkatck3.wav"), 1, ATTN_NORM, 0);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->s.frame = 72;
	ent->client->anim_end = 73;
}

void Weapon_WidowBlast_Fire (edict_t *ent)
{
	int		damage;

	damage = 10;

	WidowBlast_Fire (ent, vec3_origin, damage, false, EF_BLASTER);
	ent->client->ps.gunframe++;
}

void Weapon_WidowBlast (edict_t *ent)
{
	static int	pause_frames[]	= {5, 14, 0};
	static int	fire_frames[]	= {3, 4, 0};

	Weapon_Generic (ent, 2, 4, 15, 16, pause_frames, fire_frames, Weapon_WidowBlast_Fire);
}

void WidowKick_Fire (edict_t *ent)
{
//	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			damage = (18 + (rand() % 12));
	int			kick = 50;
	vec3_t		offset;

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	Middle_Source (ent->client, ent->s.origin, offset, forward, right, start);

	fire_player_melee_berserk (ent, start, forward, 60, damage, kick, 1, MOD_BLASTER);
	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);
}

void Weapon_WidowKick (edict_t *ent)
{
	static int	pause_frames[]	= {11, 20, 0};
	static int	fire_frames[]	= {6, 0};

	Weapon_Generic (ent, 2, 10, 21, 22, pause_frames, fire_frames, WidowKick_Fire);
}

// Widow 2 (Spider)

void WidowTongs_Fire (edict_t *ent)
{
//	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			damage = (56 + (rand() % 12));
	int			kick = 50;
	vec3_t		offset;

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	Middle_Source (ent->client, ent->s.origin, offset, forward, right, start);

	fire_player_melee_berserk (ent, start, forward, 89, damage, kick, 1, MOD_BLASTER);
	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	ent->s.frame = 53;
	ent->client->anim_end = 55;
}

void Weapon_WidowTongs (edict_t *ent)
{
	static int	pause_frames[]	= {10, 19, 0};
	static int	fire_frames[]	= {9, 0};

	Weapon_Generic (ent, 2, 9, 20, 21, pause_frames, fire_frames, WidowTongs_Fire);
}

void WidowBeam_Fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage;
	int			kick;

	if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
	{
		ent->client->ps.gunframe = 9;
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		return;
	}

	damage = 8;
	kick = 20;

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -3, ent->client->kick_origin);

	VectorSet(offset, 0, 7,  ent->viewheight-8);
	Middle_Source (ent->client, ent->s.origin, offset, forward, right, start);
	fire_heat (ent, start, forward, offset, damage, kick, 0);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->s.frame = 38;
	ent->client->anim_end = 46;
}


void Weapon_WidowBeam (edict_t *ent)
{
	static int	pause_frames[]	= {9, 18, 0};
	static int	fire_frames[]	= {5, 0};

	Weapon_Generic (ent, 2, 8, 19, 20, pause_frames, fire_frames, WidowBeam_Fire);
}

void WidowDisr_Fire (edict_t *self)
{
	vec3_t		forward, right;
	vec3_t		start;
	vec3_t		end;
	vec3_t		offset;
	edict_t		*enemy;
	trace_t		tr;
	int			damage;
	vec3_t		mins, maxs;

	// PMM - felt a little high at 25
	if(deathmatch->value)
		damage = 30;
	else
		damage = 45;

	VectorSet(mins, -16, -16, -16);
	VectorSet(maxs, 16, 16, 16);
	AngleVectors (self->client->v_angle, forward, right, NULL);
	VectorSet(offset, 24, 8, self->viewheight-8);
	P_ProjectSource (self->client, self->s.origin, offset, forward, right, start);

	// FIXME - can we shorten this? do we need to?
	VectorMA (start, 8192, forward, end);
	enemy = NULL;
	//PMM - doing two traces .. one point and one box.  
	tr = gi.trace (start, vec3_origin, vec3_origin, end, self, MASK_SHOT);
	if(tr.ent != world)
	{
		if(tr.ent->svflags & SVF_MONSTER || tr.ent->client)
		{
			if(tr.ent->health > 0)
				enemy = tr.ent;
		}
	}
	else
	{
		tr = gi.trace (start, mins, maxs, end, self, MASK_SHOT);
		if(tr.ent != world)
		{
			if(tr.ent->svflags & SVF_MONSTER || tr.ent->client)
			{
				if(tr.ent->health > 0)
					enemy = tr.ent;
			}
		}
	}

	VectorScale (forward, -2, self->client->kick_origin);
	self->client->kick_angles[0] = -1;

	fire_tracker (self, start, forward, damage, 1000, enemy);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (self-g_edicts);
	gi.WriteByte (MZ_TRACKER);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	PlayerNoise(self, start, PNOISE_WEAPON);

	self->client->ps.gunframe++;
	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		self->client->pers.inventory[self->client->ammo_index] -= 20;

	self->s.frame = 28;
	self->client->anim_end = 34;
}


void Weapon_WidowDisr (edict_t *ent)
{
	static int	pause_frames[]	= {9, 18, 0};
	static int	fire_frames[]	= {6, 0};

	Weapon_Generic (ent, 2, 8, 19, 20, pause_frames, fire_frames, WidowDisr_Fire);
}


void Weapon_WidowSpawn_Fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage;
	int			kick;

	damage = 65;
	kick = 100;

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -3, ent->client->kick_origin);
	ent->client->kick_angles[0] = -3;

	VectorSet(offset, 0, 7,  ent->viewheight-12);
	Middle_Source (ent->client, ent->s.origin, offset, forward, right, start);
	//fire_rail (ent, start, forward, damage, kick, MOD_GLADRAIL);

	if (Spawn_Monster(ent, M_STALKER)) 
	{
		Spawn_Timer(ent);
	}
	gi.sound(ent, CHAN_AUTO, gi.soundindex ("medic_commander/monsterspawn1.wav"), 1, ATTN_IDLE, 0);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]-= 25;
}


void Weapon_WidowSpawn (edict_t *ent)
{
	static int	pause_frames[]	= {16, 25, 0};
	static int	fire_frames[]	= {7, 0};

	Weapon_Generic (ent, 2, 15, 26, 27, pause_frames, fire_frames, Weapon_WidowSpawn_Fire);
}


// Roseville Mission Packs

// Death Tank
void DeathTankLaser_Fire (edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
//	int		damage;
//	float	damage_radius;
//	int		radius_damage;
//	vec3_t v;

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;


	VectorSet(offset, 24, 8, ent->viewheight-8);
	Left_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);


	fire_instlaser (ent, start, forward, 75, 25, MOD_BLASTER);

	gi.sound(ent, CHAN_WEAPON, gi.soundindex ("tank/tnkatck3.wav"), 1, ATTN_NORM, 0);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->s.frame = 63;
	ent->client->anim_end = 66;

	ent->client->ps.gunframe++;
}

void Weapon_DeathTankLaser (edict_t *ent)
{
	static int	pause_frames[]	= {7, 16, 0};
	static int	fire_frames[]	= {4, 0};

	Weapon_Generic (ent, 2, 6, 17, 18, pause_frames, fire_frames, DeathTankLaser_Fire);
}

void Weapon_DeathTankPlas_Fire (edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage;
//	float	damage_radius;
//	int		radius_damage;
//	vec3_t v;

	damage = 100 + (int)(random() * 20.0);

	if (is_quad)
	{
		damage *= 4;
	}

	if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
	{
		ent->client->ps.gunframe = 29;
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		return;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	VectorSet(offset, 8, 8, ent->viewheight+4);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	fire_phlas (ent, start, forward, 35, 900, 0, 0);


	gi.sound(ent, CHAN_WEAPON, gi.soundindex ("weapons/shootdplas.wav"), 1, ATTN_NORM, 0);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_DeathTankPlas (edict_t *ent)
{
	static int	pause_frames[]	= {29, 38, 0};
	static int	fire_frames[]	= {14, 17, 20, 0};

	Weapon_Generic (ent, 2, 28, 39, 40, pause_frames, fire_frames, Weapon_DeathTankPlas_Fire);
}


// Flyer Hornet
void Weapon_HornetRail_Fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage;
	int			kick;

	damage = 90;
	kick = 89;

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -3, ent->client->kick_origin);
	ent->client->kick_angles[0] = -3;

	VectorSet(offset, 0, 7,  ent->viewheight-12);
	Middle_Source (ent->client, ent->s.origin, offset, forward, right, start);
	if (ent->client->ps.gunframe == 7)
		fire_rail (ent, start, forward, damage, kick, MOD_GLADRAIL);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	if (ent->client->ps.gunframe == 3)
	{
		ent->s.frame = 79;
		ent->client->anim_end = 95;
	}
}


void Weapon_HornetRail (edict_t *ent)
{
	static int	pause_frames[]	= {16, 25, 0};
	static int	fire_frames[]	= {7, 0};

	Weapon_Generic (ent, 2, 15, 26, 27, pause_frames, fire_frames, Weapon_HornetRail_Fire);
}

void TraitChain_Fire (edict_t *ent)
{
//	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			damage = 4;
	int			kick = 6;
	vec3_t		offset;

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe = 31;
		ent->client->weaponstate = WEAPON_READY;
		return;
	}

	if (ent->client->ps.gunframe == 21)
		ent->client->ps.gunframe = 5;

	if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
	{
		ent->client->ps.gunframe = 31;
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		return;
	}

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_INFCHAIN);

	ent->client->ps.gunframe++;

	gi.sound(ent, CHAN_WEAPON, gi.soundindex ("infantry/infatck1.wav"), 1, ATTN_NORM, 0);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ2_INFANTRY_MACHINEGUN_1 | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->client->anim_priority = ANIM_ATTACK;
	ent->s.frame = FRAME_attack1 - (ent->client->ps.gunframe & 1);
	ent->client->anim_end = FRAME_attack8;
}

void Weapon_TraitChain (edict_t *ent)
{
	static int	pause_frames[]	= {38, 43, 51, 61, 0};
	static int	fire_frames[]	= {5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 0};

	Weapon_Generic (ent, 4, 31, 61, 64, pause_frames, fire_frames, TraitChain_Fire);
}





void weapon_Trait_railgun_fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage;
	int			kick;

	if (deathmatch->value)
	{	// normal damage is too extreme in dm
		damage = 100;
		kick = 200;
	}
	else
	{
		damage = 150;
		kick = 250;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -3, ent->client->kick_origin);
	ent->client->kick_angles[0] = -3;

	VectorSet(offset, 0, 7,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_rail (ent, start, forward, damage, kick, MOD_RAILGUN);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_RAILGUN | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->client->anim_priority = ANIM_ATTACK;
	ent->s.frame = FRAME_attack1;
	ent->client->anim_end = FRAME_attack8;
}


void Weapon_Trait_Railgun (edict_t *ent)
{
	static int	pause_frames[]	= {56, 0};
	static int	fire_frames[]	= {4, 0};

	Weapon_Generic (ent, 3, 18, 56, 61, pause_frames, fire_frames, weapon_Trait_railgun_fire);
}


void weapon_supershotgun_fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	vec3_t		v;
	int			damage = 6;
	int			kick = 12;

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -2;

	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	v[PITCH] = ent->client->v_angle[PITCH];
	v[YAW]   = ent->client->v_angle[YAW] - 5;
	v[ROLL]  = ent->client->v_angle[ROLL];
	AngleVectors (v, forward, NULL, NULL);
	fire_shotgun (ent, start, forward, damage, kick, DEFAULT_SHOTGUN_HSPREAD, DEFAULT_SHOTGUN_VSPREAD, DEFAULT_SSHOTGUN_COUNT/2, MOD_SSHOTGUN);
	v[YAW]   = ent->client->v_angle[YAW] + 5;
	AngleVectors (v, forward, NULL, NULL);
	fire_shotgun (ent, start, forward, damage, kick, DEFAULT_SHOTGUN_HSPREAD, DEFAULT_SHOTGUN_VSPREAD, DEFAULT_SSHOTGUN_COUNT/2, MOD_SSHOTGUN);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_SSHOTGUN | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	ent->client->anim_priority = ANIM_ATTACK;
	ent->s.frame = FRAME_attack1;
	ent->client->anim_end = FRAME_attack8;

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= 2;
}

void Weapon_SuperShotgun (edict_t *ent)
{
	static int	pause_frames[]	= {29, 42, 57, 0};
	static int	fire_frames[]	= {7, 0};

	Weapon_Generic (ent, 6, 17, 57, 61, pause_frames, fire_frames, weapon_supershotgun_fire);
}








void HypertankChain_Fire (edict_t *ent)
{
//	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			damage = 6;
	int			kick = 4;
	vec3_t		offset;
//	vec3_t firevec; /*Added: Direction to fire*/
	int rnd = rndnum (0, 5);

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
	{
		ent->client->ps.gunframe = 4;
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		return;
	}

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	//mod_GetLeadoffVec(ent, start, 5000, 1000, false, firevec);
	if (rnd < 3)
		fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_STANKCHAIN);
	else
		fire_blaster (ent, start, forward, 9, 800, EF_BLASTER, 2);

	gi.sound(ent, CHAN_WEAPON, gi.soundindex ("infantry/infatck1.wav"), 1, ATTN_NORM, 0);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ2_INFANTRY_MACHINEGUN_1 | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->client->anim_priority = ANIM_ATTACK;
	ent->s.frame = 0;
	ent->client->anim_end = 2;
}

void Weapon_HypertankChain (edict_t *ent)
{
	static int	pause_frames[]	= {4, 13, 0};
	static int	fire_frames[]	= {2, 3, 0};

	Weapon_Generic (ent, 1, 3, 13, 14, pause_frames, fire_frames, HypertankChain_Fire);
}

void Weapon_HypertankRock_Fire (edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage;
	float	damage_radius;
	int		radius_damage;
	int rnd = rndnum (0, 9);

	damage = 100 + (int)(random() * 20.0);
	radius_damage = 50+20;
	damage_radius = 50+20;
	if (is_quad)
	{
		damage *= 4;
		radius_damage *= 4;
	}

	if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
	{
		ent->client->ps.gunframe = 14;
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		return;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	if (rnd == 0 || rnd == 1 || rnd == 2)
		fire_rocket (ent, start, forward, 80, 1200, damage_radius*2, radius_damage*2);
	else if (rnd == 3 || rnd == 4 || rnd == 5)
		fire_grenade (ent, start, forward, 50, 800, 2.5, 230);
	else
		fire_bfg (ent, start, forward, 160, 500, 300);

	gi.sound(ent, CHAN_WEAPON, gi.soundindex ("tank/rocket.wav"), 1, ATTN_NORM, 0);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_HypertankRock (edict_t *ent)
{
	static int	pause_frames[]	= {14, 23, 0};
	static int	fire_frames[]	= {3, 6, 9, 0};

	Weapon_Generic (ent, 2, 13, 24, 25, pause_frames, fire_frames, Weapon_HypertankRock_Fire);
}