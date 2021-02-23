

// Basic functions

void T_RadiusDamage2 (edict_t *attacker,vec3_t position, float damage, float radius, int mod);
void FlashLightThink(edict_t *ent);
edict_t *findradius2 (edict_t *from, vec3_t org, float rad);
qboolean TeamMembers(edict_t *p1, edict_t *p2);
qboolean infront2 (edict_t *self, edict_t *other);
qboolean infront3 (edict_t *self, edict_t *other);
qboolean infront4 (edict_t *self, edict_t *other);
qboolean TouchingLadder(edict_t *self);
void		Bot_InitNodes(void);
qboolean	Bot_LoadNodes(void);
qboolean	Bot_SaveNodes(void);
void ShowGun(edict_t *ent);	//vwep
void CreateCamera(edict_t *ent);
void CamNext(edict_t *ent);
void CamPrev(edict_t *ent);
void CameraThink(edict_t *ent, usercmd_t *ucmd);

// Weapon banning console variables

extern cvar_t	*ban_sword, *ban_chainsaw, *ban_supershotgun,
		*ban_crossbow,*ban_airgun, *ban_grenadelauncher,
		*ban_proxylauncher, *ban_rocketlauncher, *ban_hyperblaster,
		*ban_railgun, *ban_buzzsaw, *ban_vortex, *ban_defenceturret,
		*ban_rocketturret, *ban_bfg;

// Item banning console variables

extern cvar_t	*ban_grapple, *ban_bodyarmor, *ban_combatarmor, *ban_jacketarmor, 
		*ban_armorshard, *ban_powerscreen, *ban_powershield,
		*ban_ammo_grenades, *ban_ammo_flashgrenades,
		*ban_ammo_lasergrenades, *ban_ammo_poisongrenades,
		*ban_ammo_proximitymines, *ban_ammo_shells,
		*ban_ammo_explosiveshells, *ban_ammo_arrows,
		*ban_ammo_explosivearrows, *ban_ammo_poisonarrows,
		*ban_ammo_cells, *ban_ammo_rockets, *ban_ammo_homingmissiles,
		*ban_ammo_buzzes, *ban_ammo_slugs, *ban_quaddamage,
		*ban_jetpack, *ban_invulnerability, *ban_invisibility,
		*ban_silencer, *ban_rebreather, *ban_environmentsuit,
		*ban_adrenaline, *ban_health, *ban_health_small,
		*ban_health_large, *ban_health_mega;

// Startup weapon console variables

extern cvar_t	*start_sword, *start_chainsaw, *start_supershotgun, *start_crossbow,
		*start_airgun, *start_grenadelauncher, *start_proxylauncher,
		*start_rocketlauncher, *start_hyperblaster, *start_railgun,
		*start_buzzsaw, *start_bfg;

// Startup item console variables

extern cvar_t	*start_grapple, *start_defenceturret, *start_rocketturret, 
		*start_autosentry, *start_gravityvortex, *start_bodyarmor,
		*start_combatarmor, *start_jacketarmor, *start_armorshard,
		*start_powerscreen, *start_powershield, *start_ammo_grenades,
		*start_ammo_flashgrenades, *start_ammo_lasergrenades,
		*start_ammo_poisongrenades, *start_ammo_proximitymines,
		*start_ammo_shells, *start_ammo_explosiveshells,
		*start_ammo_arrows, *start_ammo_explosivearrows,
		*start_ammo_poisonarrows, *start_ammo_cells,
		*start_ammo_rockets, *start_ammo_homingmissiles,
		*start_ammo_buzzes, *start_ammo_slugs, *start_quaddamage,
		*start_jetpack, *start_invulnerability, *start_silencer,
		*start_rebreather, *start_environmentsuit;

// Item definitions

extern gitem_t	*it_shells, *it_eshells, *it_cells, *it_arrows, *it_rockets, 
		*it_homings, *it_slugs, *it_buzzes, *it_grenades,
		*it_flashgrenades, *it_lasermines, *it_poisongrenades,
		*it_proxymines, *it_ak42, *it_shotgun, *it_supershotgun,
		*it_esupershotgun, *it_crossbow, *it_grenadelauncher,
		*it_rocketlauncher, *it_hominglauncher, *it_railgun,
		*it_buzzsaw, *it_hyperblaster, *it_bfg,
		*it_flashgrenadelauncher, *it_poisongrenadelauncher,
		*it_proxyminelauncher, *it_vortex, *it_sword, *it_rturret,
		*it_poisonarrows, *it_explosivearrows, *it_poisoncrossbow,
		*it_explosivecrossbow, *it_grapple, *it_jetpack, *it_chainsaw,
		*it_health, *it_health_large, *it_health_mega, *it_lturret,
		*it_airfist;

