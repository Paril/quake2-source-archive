//====================================================================
//=============== GLOBAL SUBSTITUTIONS MADE UPON COMPILE =============
//====================================================================

// Some Control Structure Global Substitutions
#define for_each_client for(i=0;i<maxclients->value;i++)
#define for_each_item for(i=1;i<=MAX_ITEMS;i++)

// Cvars..
#define CVAR_FRAGLIMIT fraglimit->value
#define CVAR_DEATHMATCH deathmatch->value
#define CVAR_DEDICATED dedicated->value
#define CVAR_SKILL_LEVEL skill->value
#define CVAR_TIMELIMIT timelimit->value
#define CVAR_MAXCLIENTS maxclients->value
#define CVAR_MAX_ENTITIES maxentities->value
#define CVAR_SELECT_EMPTY_GUN g_select_empty->value

// Some server variables
#define CHEATS_FLAG sv_cheats->value
#define MAXVELOCITY sv_maxvelocity->value
#define WORLD_GRAVITY sv_gravity->value

//====================================================================

// gitem_t
#define ENTS_ITEM_QUANTITY ent->item->quantity
#define ENTS_ITEM_USE ent->item->use
#define ENTS_ITEM_FLAG ent->item->flags
#define ENTS_ITEM_PICKUP ent->item->pickup
#define ITEM_PICKUP_SOUND ent->item->pickup_sound
#define ENTS_ITEM_TAG ent->item->tag
#define ITEM_WORLD_MODEL ent->item->world_model
#define ENTS_ITEM_AMMO ent->item->ammo
#define ENTS_ITEM_ICON ent->item->icon
#define ENTS_ITEM_INFO ent->item->info

//====================================================================

// Misc Model definitions.
#define BOMB_MODEL "models/objects/bomb/tris.md2"
#define ROCKET_MODEL "models/objects/rocket/tris.md2"
#define GRENADE_MODEL "models/objects/grenade/tris.md2"
#define HGRENADE_MODEL "models/objects/grenade2/tris.md2"
#define STROGG_SHIP_MODEL "models/ships/viper/tris.md2"

// Body Gibs Models.
#define GIB_SM_MEAT_MODEL "models/objects/gibs/sm_meat/tris.md2"
#define GIB_ARM_MODEL "models/objects/gibs/arm/tris.md2"
#define GIB_LEG_MODEL "models/objects/gibs/leg/tris.md2"
#define GIB_BONE_MODEL "models/objects/gibs/bone/tris.md2"
#define GIB_BONE2_MODEL "models/objects/gibs/bone2/tris.md2"
#define GIB_CHEST_MODEL "models/objects/gibs/chest/tris.md2"
#define GIB_SKULL_MODEL "models/objects/gibs/skull/tris.md2"
#define GIB_HEAD_MODEL "models/objects/gibs/head/tris.md2"
#define GIB_HEAD2_MODEL "models/objects/gibs/head2/tris.md2"

// Debris Gibs Models.
#define DEBRIS1_MODEL "models/objects/debris1/tris.md2"
#define DEBRIS2_MODEL "models/objects/debris2/tris.md2"
#define DEBRIS3_MODEL "models/objects/debris3/tris.md2"

// Misc World Models
#define BANNER_MODEL "models/objects/banner/tris.md2"
#define DMSPAWN_MODEL "models/objects/dmspot/tris.md2"
#define LASER_MODEL "models/objects/laser/tris.md2"
#define SILENCER_MODEL "models/items/silencer/tris.md2"

//====================================================================
//============= Global SoundIndex Substitutions ======================
//====================================================================

// Misc Sound Indices
#define WIND_SOUND gi.soundindex("misc/windfly.wav")
#define LASFLY_SOUND gi.soundindex("misc/lasfly.wav")
#define SECRET_SOUND gi.soundindex("misc/secret.wav")
#define TALK_SOUND gi.soundindex("misc/talk.wav")
#define TALK1_SOUND gi.soundindex("misc/talk1.wav")
#define POWER1_SOUND gi.soundindex("misc/power1.wav")
#define POWER2_SOUND gi.soundindex("misc/power2.wav")
#define TRIGGER1_SOUND gi.soundindex("misc/trigger1.wav")
#define FHIT3_SOUND gi.soundindex("misc/fhit3.wav")
#define PC_UP_SOUND gi.soundindex("misc/pc_up.wav")
#define H2OHIT1_SOUND gi.soundindex("misc/h2ohit1.wav")
#define UDEATH_SOUND gi.soundindex("misc/udeath.wav")

// World Sound Indices
#define AMB10_SOUND gi.soundindex("world/amb10.wav")
#define LAND_SOUND gi.soundindex("world/land.wav")
#define PILOT1_SOUND gi.soundindex("world/pilot1.wav")
#define PILOT2_SOUND gi.soundindex("world/pilot2.wav")
#define PILOT3_SOUND gi.soundindex("world/pilot3.wav")
#define FLYBY1_SOUND gi.soundindex("world/flyby1.wav")
#define ELECTRO_SOUND gi.soundindex("world/electro.wav")
#define MOVING_WATER_SOUND gi.soundindex("world/mov_watr.wav")
#define STOP_WATER_SOUND gi.soundindex("world/stp_watr.wav")

// Item Sound Indices
#define PICKUP_SOUND gi.soundindex("items/pkup.wav")
#define RESPAWN1_SOUND gi.soundindex("items/respawn1.wav")
#define AIROUT_SOUND gi.soundindex("items/airout.wav")
#define DAMAGE_SOUND gi.soundindex("items/damage.wav")
#define DAMAGE2_SOUND gi.soundindex("items/damage2.wav")
#define DAMAGE3_SOUND gi.soundindex("items/damage3.wav")
#define PROTECT_SOUND gi.soundindex("items/protect.wav")
#define PROTECT2_SOUND gi.soundindex("items/protect2.wav")
#define PROTECT4_SOUND gi.soundindex("items/protect4.wav")

// Weapon Sound Indices
#define NO_AMMO_SOUND gi.soundindex("weapons/noammo.wav")
#define HANDGRENADE1B_SOUND gi.soundindex("weapons/hgrena1b.wav")
#define HANDGRENADE1A_SOUND gi.soundindex("weapons/hgrenb1a.wav")
#define HANDGRENADE1C_SOUND gi.soundindex("weapons/hgrenc1b.wav")
#define GRENADE_TOSS_SOUND gi.soundindex("weapons/hgrent1a.wav")
#define HANDGRENADE2_SOUND gi.soundindex("weapons/hgrenb2a.wav")
#define GRENADE_BOUNCE_SOUND gi.soundindex("weapons/grenlb1b.wav")
#define HYPERBLASTER1_SOUND gi.soundindex("weapons/hyprbl1a.wav")
#define HYPERBLASTER2_SOUND gi.soundindex("weapons/hyprbd1a.wav")
#define CHAINGUN1_SOUND gi.soundindex("weapons/chngnu1a.wav")
#define CHAINGUN2_SOUND gi.soundindex("weapons/chngnd1a.wav")
#define CHAINGUN3_SOUND gi.soundindex("weapons/chngnl1a.wav")
#define ROCKET_FLY_SOUND gi.soundindex("weapons/rockfly.wav")
#define RG_HUM_SOUND gi.soundindex("weapons/rg_hum.wav")
#define BFG_HUM_SOUND gi.soundindex("weapons/bfg_hum.wav")
#define BFG_EXPLODE_SOUND gi.soundindex("weapons/bfg__x1b.wav")
#define BFG_LAUNCH_SOUND gi.soundindex("weapons/bfg__l1a.wav")
#define LASER_SOUND gi.soundindex("weapons/laser2.wav")

// Player Generic Sound Indices
#define ENTER_WATER_SOUND gi.soundindex("player/watr_in.wav")
#define EXIT_WATER_SOUND gi.soundindex("player/watr_out.wav")
#define UNDER_WATER_SOUND gi.soundindex("player/watr_un.wav")
#define WATER_GASP1_SOUND gi.soundindex("player/gasp1.wav")
#define WATER_GASP2_SOUND gi.soundindex("player/gasp2.wav")
#define WATER_DROWN1_SOUND gi.soundindex("player/drown1.wav")
#define U_BREATHING1_SOUND gi.soundindex("player/u_breath1.wav")
#define U_BREATHING2_SOUND gi.soundindex("player/u_breath2.wav")
#define BURNING1_SOUND gi.soundindex("player/burn1.wav")
#define BURNING2_SOUND gi.soundindex("player/burn2.wav")
#define LAVA_SCREAM_SOUND gi.soundindex("player/lava_in.wav")
#define LAVA1_SOUND gi.soundindex("player/lava1.wav")
#define LAVA2_SOUND gi.soundindex("player/lava2.wav")
#define FRYING_SOUND gi.soundindex("player/fry.wav")

// Player (Male/Female) Sound Indices
#define DEATH1_SOUND gi.soundindex("*death1.wav")
#define DEATH2_SOUND gi.soundindex("*death2.wav")
#define DEATH3_SOUND gi.soundindex("*death3.wav")
#define DEATH4_SOUND gi.soundindex("*death4.wav")
#define FALLING1_SOUND gi.soundindex("*fall1.wav")
#define FALLING2_SOUND gi.soundindex("*fall2.wav")
#define GURPING1_SOUND gi.soundindex("*gurp1.wav")
#define GURPING2_SOUND gi.soundindex("*gurp2.wav")
#define JUMPING1_SOUND gi.soundindex("*jump1.wav")
#define PAIN25_1_SOUND gi.soundindex("*pain25_1.wav")
#define PAIN25_2_SOUND gi.soundindex("*pain25_2.wav")
#define PAIN50_1_SOUND gi.soundindex("*pain50_1.wav")
#define PAIN50_2_SOUND gi.soundindex("*pain50_2.wav")
#define PAIN75_1_SOUND gi.soundindex("*pain75_1.wav")
#define PAIN75_2_SOUND gi.soundindex("*pain75_2.wav")
#define PAIN100_1_SOUND gi.soundindex("*pain100_1.wav")
#define PAIN100_2_SOUND gi.soundindex("*pain100_2.wav")

// Sound randomization
#define RANDOM_PAIN_SOUND gi.soundindex(va("*pain%i_%i.wav",l,r))
#define RANDOM_DEATH_SOUND gi.soundindex(va("*death%i.wav",(rand()%4)+1))

// ??
#define INFANTRY_INFLIES1_SOUND gi.soundindex("infantry/inflies1.wav")

//====================================================================
//====== SOME IF STATEMENT FLAG CHECK SUBSTITUTIONS ==================
//====================================================================

// Ents Damage Flags
#define NO_ARMOR_PROTECTION (dflags & DAMAGE_NO_ARMOR)
#define DAMAGE_FROM_ENERGY_WEAPON (dflags & DAMAGE_ENERGY)
#define DAMAGE_FROM_BULLETS (dflags & DAMAGE_BULLET)
#define DAMAGE_IS_RADIUS_DAMAGE (dflags & DAMAGE_RADIUS)
#define NO_KICK_FROM_DAMAGE (dflags & DAMAGE_NO_KNOCKBACK)
#define NO_DAMAGE_PROTECTION (dflags & DAMAGE_NO_PROTECTION)

//====================================================================

// Deathmatch Flags - cvar declared in g_main.c
#define ENABLE_TEAMPLAY ((int)dmflags->value & (DF_MODELTEAMS|DF_SKINTEAMS))
#define PLAY_TEAMS_BY_MODEL ((int)dmflags->value & DF_MODELTEAMS)
#define PLAY_TEAMS_BY_SKIN ((int)dmflags->value & DF_SKINTEAMS)
#define ALLOW_INFINITE_AMMO ((int)dmflags->value & DF_INFINITE_AMMO)
#define DMFLAGS_SET_FOR_NO_HEALTH ((int)dmflags->value & DF_NO_HEALTH)
#define NO_DAMAGE_FROM_FRIENDLY_FIRE ((int)dmflags->value & DF_NO_FRIENDLY_FIRE)
#define ENABLE_FIXED_FOV ((int)dmflags->value & DF_FIXED_FOV)
#define DMFLAGS_SET_INSTANT_ITEMS ((int)dmflags->value & DF_INSTANT_ITEMS)
#define DMFLAGS_SET_NO_ARMOR_ALLOWED ((int)dmflags->value & DF_NO_ARMOR)
#define ENABLE_WEAPONS_STAY ((int)dmflags->value & DF_WEAPONS_STAY)
#define DMFLAGS_SET_NO_ITEMS ((int)dmflags->value & DF_NO_ITEMS)
#define DMFLAGS_SET_SAME_LEVEL ((int)dmflags->value & DF_SAME_LEVEL)
#define ALLOW_LEVEL_EXIT ((int)dmflags->value & DF_ALLOW_EXIT)
#define ALLOW_QUAD_DROP ((int)dmflags->value & DF_QUAD_DROP)
#define ENABLE_SPAWN_FARTHEST ((int)dmflags->value & DF_SPAWN_FARTHEST)
#define ENABLE_FORCED_RESPAWN ((int)dmflags->value & DF_FORCE_RESPAWN)

//====================================================================

// Checking if Ent can be damaged
#define ENT_CANNOT_TAKE_DAMAGE ent->takedamage=DAMAGE_NO

// Checking if Ent is Monster
#define ENT_IS_NOT_MONSTER !(ent->svflags & SVF_MONSTER)

// Checking ents/players if standing or crouching.
#define ENT_CURRENTLY_IS_CROUCHED (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
#define PLAYER_CURRENTLY_IS_CROUCHED (player->client->ps.pmove.pm_flags & PMF_DUCKED)

// movetype flags
#define ENT_HAS_NOCLIP_ON (ent->movetype == MOVETYPE_NOCLIP)
#define ENT_HAS_NOCLIP_OFF (ent->movetype != MOVETYPE_NOCLIP)
#define MOVETYPE_NOT_FLYMISSILE (ent->movetype != MOVETYPE_FLYMISSILE)

// Misc State Flags.
#define ENT_SET_FOR_NOTARGET (ent->flags & FL_NOTARGET)
#define ENT_SET_FOR_GODMODE (ent->flags & FL_GODMODE)
#define ENT_SET_FOR_TEAMSLAVE (ent->flags & FL_TEAMSLAVE)
#define ENT_SET_FOR_POWERARMOR (ent->flags & FL_POWER_ARMOR)
#define ENT_SET_FOR_RESPAWN (ent->flags & FL_RESPAWN)
#define FLYING_TYPE_ENTITY (ent->flags & FL_FLY)
#define SWIMMING_TYPE_ENTITY (ent->flags & FL_SWIM)
#define ENT_ON_PARTIAL_GROUND (ent->flags & FL_PARTIALGROUND)
#define SWIMMING_OR_FLYING_TYPE_ENTITY (ent->flags & (FL_FLY|FL_SWIM))

// important client defines
#define ITEM_IN_ENTS_INVENTORY ent->client->pers.inventory[index]
#define ITEM_NOT_IN_ENTS_INVENTORY !ent->client->pers.inventory[index]
