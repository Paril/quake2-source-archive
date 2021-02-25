//===========================================================================
// g_flare.h
//
// Marine flares.
// Originally coded by DingBat.
//
// Copyright (c), 1999 The BatCave. All Rights Reserved.
//===========================================================================

// Flare constants
#define MAX_FLARES_DEFAULT "5"
#define FLARE_BRIGHT_DEFAULT "10"
#define FLARE_DIM_DEFAULT "5"
#define FLARE_DIE_DEFAULT "5"
#define FLARE_HEALTH_DEFAULT "0"
#define FLARE_DAMAGE_DEFAULT "0"
#define FLARE_RADIUS_DEFAULT "0"
#define ENABLE_FLARE_DAMAGE_DEFAULT "1"

void validateMaxFlares();
int getMaxFlares();
void validateFlareBrightTime();
int getFlareBrightTime();
void validateFlareDimTime();
int getFlareDimTime();
void validateFlareDieTime();
int getFlareDieTime();
void validateFlareHealth();
int getFlareHealth();
void validateFlareDamage();
int getFlareDamage();
void validateFlareDamageRadius();
int getFlareDamageRadius();

static void Flare_End(edict_t *ent) ;
static void Flare_Burnout(edict_t *ent) ;
static void Flare_Die(edict_t *self, 
		      edict_t *inflictor, 
		      edict_t *attacker, 
		      int damage, 
		      vec3_t point) ;
static void Flare_Explode(edict_t *ent) ;
static void Flare_Touch(edict_t *ent, 
						edict_t *other, 
						cplane_t *plane,
						csurface_t *surf) ;
void fire_flare(edict_t *self, 
				vec3_t start, 
				vec3_t aimdir,
				int damage,
				int speed, 
				float timer, 
				float damage_radius, 
				qboolean held) ;
void Cmd_Flare_f(edict_t *ent) ;
				
void weapon_flare_fire(edict_t *ent, qboolean held) ;
void Weapon_Flare (edict_t *ent) ;


