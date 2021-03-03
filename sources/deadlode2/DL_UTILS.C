#include "g_local.h"
#include "dl_player.h"

char *motdLines[255];
int numMOTDLines = 0;

ppmframes_t s_standFrames [] = {
	{"AIM",		FRAME_s_aimetc_stnd1,	FRAME_s_aimetc_stnd10},
	{"HIP",		FRAME_s_hip_stnd1,		FRAME_s_hip_stnd10},
	{"PISTOL",	FRAME_s_pstletc_stn1,	FRAME_s_pstletc_stn10},
	{"LASER",	FRAME_s_lsr_stnd1,		FRAME_s_lsr_stnd10},
	{NULL,		0,						0}
};

ppmframes_t s_runFrames [] = {
	{"AIM",		FRAME_s_aimetc_run1,	FRAME_s_aimetc_run6},
	{"HIP",		FRAME_s_hipetc_run1,	FRAME_s_hipetc_run6},
	{"MACH",	FRAME_s_machetc_run1,	FRAME_s_machetc_run6},
	{"PISTOL",	FRAME_s_pstletc_run1,	FRAME_s_pstletc_run6},
	{"LASER",	FRAME_s_lsr_run1,		FRAME_s_lsr_run6},
	{NULL,		0,						0}
};

ppmframes_t s_attackFrames [] = {
	{"AIM",		FRAME_s_aimetc_att1,	FRAME_s_aimetc_att5},
	{"PUMP",	FRAME_s_pump_att1,		FRAME_s_pump_att7},
	{"HIP",		FRAME_s_hip_att1,		FRAME_s_hip_att2},
	{"PISTOL",	FRAME_s_pstletc_att1,	FRAME_s_pstletc_att5},
	{"LASER",	FRAME_s_lsr_att1,		FRAME_s_lsr_att2},
	{"GUSTAV",	FRAME_s_gustav_att1,	FRAME_s_gustav_att5},
	{"LAW",		FRAME_s_law_att1,		FRAME_s_law_att5},
	{NULL,		0,						0}
};

ppmframes_t s_painFrames [] = {
	{"AIM",		FRAME_s_aimetc_pain1,	FRAME_s_aimetc_pain6},
	{"PISTOL",	FRAME_s_pstletc_pai1,	FRAME_s_pstletc_pai8},
	{"LASER",	FRAME_s_lsr_pain1,		FRAME_s_lsr_pain7},
	{NULL,		0,						0}
};

ppmframes_t s_deathFrames [] = {
	{"AIM",		FRAME_s_aimetc_die1,	FRAME_s_aimetc_die10},
	{"PISTOL",	FRAME_s_pstletc_die1,	FRAME_s_pstletc_die13},
	{"LASER",	FRAME_s_lsr_die1,		FRAME_s_lsr_die13},
	{NULL,		0,						0}
};

ppmframes_t jumpFrames [] = {
	{"AIM",		FRAME_s_aimetc_jump1,	FRAME_s_aimetc_jump6},
	{"PISTOL",	FRAME_s_pstletc_jum1,	FRAME_s_pstletc_jum6},
	{"LASER",	FRAME_s_lsr_jump1,		FRAME_s_lsr_jump6},
	{NULL,		0,						0}
};

ppmframes_t c_standFrames [] = {
	{"AIM",		FRAME_c_aimetc_stnd1,	FRAME_c_aimetc_stnd10},
	{"PISTOL",	FRAME_c_pstletc_stn1,	FRAME_c_pstletc_stn10},
	{"LASER",	FRAME_c_lsr_stnd1,		FRAME_c_lsr_stnd10},
	{NULL,		0,						0}
};

ppmframes_t c_runFrames [] = {
	{"AIM",		FRAME_c_aimetc_run1,	FRAME_c_aimetc_run6},
	{"PISTOL",	FRAME_c_pstletc_run1,	FRAME_c_pstletc_run6},
	{"LASER",	FRAME_c_lsr_run1,		FRAME_c_lsr_run6},
	{NULL,		0,						0}
};

ppmframes_t c_attackFrames [] = {
	{"AIM",		FRAME_c_aim_att1,		FRAME_c_aim_att5},
	{"PUMP",	FRAME_c_pump_att1,		FRAME_c_pump_att9},
	{"HIP",		FRAME_c_hip_att1,		FRAME_c_hip_att2},
	{"PISTOL",	FRAME_c_pstletc_att1,	FRAME_c_pstletc_att5},
	{"LASER",	FRAME_c_lsr_att1,		FRAME_c_lsr_att2},
	{"GUSTAV",	FRAME_c_gustav_att1,	FRAME_c_gustav_att5},
	{"LAW",		FRAME_c_law_att1,		FRAME_c_law_att5},
	{NULL,		0,						0}
};

ppmframes_t c_painFrames [] = {
	{"AIM",		FRAME_c_aimetc_pain1,	FRAME_c_aimetc_pain8},
	{"PISTOL",	FRAME_c_pstletc_pai1,	FRAME_c_pstletc_pai6},
	{"LASER",	FRAME_c_lsr_pain1,		FRAME_c_lsr_pain6},
	{NULL,		0,						0}
};

ppmframes_t c_deathFrames [] = {
	{"AIM",		FRAME_c_aimetc_die1,	FRAME_c_aimetc_die9},
	{"PISTOL",	FRAME_c_pstletc_die1,	FRAME_c_pstletc_die9},
	{"LASER",	FRAME_c_lsr_die1,		FRAME_c_lsr_die6},
	{NULL,		0,						0}
};

	
ident_t identSet[] = {
	{"PLAYERS",			ASSIGN_LEAST_PLAYERS},
	{"SCORE",			ASSIGN_LEAST_SCORE},
	{"RANDOM",			ASSIGN_RANDOM},

	{"NOENCUMBRANCE",	PM_NOENCUMBRANCE},
	{"TEAMSCORES",		PM_TEAMSCORES},
	{"FORCESKINS",		PM_FORCESKINS},
	{"TEAMDAMAGE",		PM_TEAMDAMAGE},
	{"ELIMINATION",		PM_ELIMINATION},
	{"INFINITEAMMO",	PM_INFINITEAMMO},
	{"NORELOADS",		PM_NORELOADS},
	{"DEFENSERETURN",	PM_DEFENSERETURN},
	{"HALFDAMAGE",		PM_HALFDAMAGE},

	{"PELLET",			FT_PELLET},
	{"FLIGHT",			FT_FLIGHT},
	{"TOSS",			FT_TOSS},

	{"RIFLE",			WS_RIFLE},
	{"PISTOL",			WS_PISTOL},
	{"HIP",				WS_HIP},
	{"PUMP",			WS_PUMP},
	{"SHOULDER",		WS_SHOULDER},
	{"SHOULDER_ALT",	WS_SHOULDER_ALT},
	{"HEAD_TARGET",		WS_HEAD_TARGET},

	{"DETONATE",		TT_DETONATE},
	{"BOUNCE",			TT_BOUNCE},
	{"STICK",			TT_STICK},
	{"DET_ONFLESH",		TT_DET_TOUCH},
	{"DET_TIMER",		TT_DET_TTL},
	{"DET_SKY",			TT_DET_SKY},

	{"DRUNK",			FE_DRUNK},
	{"HOMING",			FE_HOMING},
	{"BFG",				FE_BFG},

	{"SEMI",			WT_SEMI},
	{"AUTO",			WT_AUTO},
	{"BURST",			WT_BURST},
	{"NOIDLEPAUSE",		WT_NOIDLEPAUSE},
	{"NOTUSABLE",		WT_NOTUSABLE},
	{"NOAMMO",			WT_NOAMMO},
	{"COUNTRELOAD",		WT_COUNTRELOAD},

	{"NONE",			BE_NONE},
	{"SOUND",			BE_SOUND},
	{"MODEL",			BE_MODEL},
	{"MUZZLEFLASH",		BE_MUZZLEFLASH},
	{"TEMPENTITY",		BE_TEMPENTITY},
	{"PUSH",			BE_PUSH},
	{"TINT",			BE_TINT},
	{"CONFUSION",		BE_CONFUSION},
	{"BLIND",			BE_BLIND},
	{"DAMAGE",			BE_DAMAGE},

	{"RADIUS",			BE_RADIUS},
	{"LOOP",			BE_LOOP},

	{"FLARE",			BE_FLARE},
	{"SMOKE",			BE_SMOKE},
	{"FLASH",			BE_FLASH},
	{"CONCUSSION",		BE_CONCUSSION},
	{"RADIUSDAMAGE",	BE_RADIUSDAMAGE},
	{"SHRAPNEL",		BE_SHRAPNEL},
	{"MIRV",			BE_MIRV},
	{"SPINNER",			BE_SPINNER},
	{"GRAVITY",			BE_GRAVITY},
	{"AFFECTSENSOR",	BE_AFFECTSENSOR},
	{"QUAKE",			BE_QUAKE},
	{"DRONE",			BE_DRONE},

	{"IMPACT",			DAMAGE_IMPACT},
	{"PIERCE",			DAMAGE_PIERCE},
	{"ENERGY",			DAMAGE_ENERGY},
	{"BLAST",			DAMAGE_BLAST},
	{"IGNORE_ARMOR",	DAMAGE_NO_ARMOR},
	{"NO_KNOCKBACK",	DAMAGE_NO_KNOCKBACK},
	{"NO_PROTECTION",	DAMAGE_NO_PROTECTION},
	{"IGNORE_HITLOC",	DAMAGE_NO_HITLOC},

	{"WEAPON",			ITEM_WEAPON},
	{"AMMO",			ITEM_AMMO},
	{"SUPPLY",			ITEM_SUPPLY},

	{"EF_ROTATE",		EF_ROTATE},
	{"EF_GIB",			EF_GIB},
	{"EF_BLASTER",		EF_BLASTER},
	{"EF_ROCKET",		EF_ROCKET},
	{"EF_GRENADE",		EF_GRENADE},
	{"EF_HYPERBLASTER",	EF_HYPERBLASTER},
	{"EF_BFG",			EF_BFG},
	{"EF_COLOR_SHELL",	EF_COLOR_SHELL},
	{"EF_POWERSCREEN",	EF_POWERSCREEN},
	{"EF_ANIM01",		EF_ANIM01},
	{"EF_ANIM23",		EF_ANIM23},
	{"EF_ANIM_ALL",		EF_ANIM_ALL},
	{"EF_ANIM_ALLFAST",	EF_ANIM_ALLFAST},
	{"EF_FLIES",		EF_FLIES},
	{"EF_QUAD",			EF_QUAD},
	{"EF_PENT",			EF_PENT},
	{"EF_TELEPORTER",	EF_TELEPORTER},
	{"EF_FLAG1",		EF_FLAG1},
	{"EF_FLAG2",		EF_FLAG2},
	{"EF_IONRIPPER",	EF_IONRIPPER},
	{"EF_GREENGIB",		EF_GREENGIB},
	{"EF_BLUEHYPERBLASTER", EF_BLUEHYPERBLASTER},
	{"EF_SPINNINGLIGHTS", EF_SPINNINGLIGHTS},
	{"EF_PLASMA",		EF_PLASMA},
	{"EF_TRAP",			EF_TRAP},
	{"EF_TRACKER",		EF_TRACKER},
	{"EF_DOUBLE",		EF_DOUBLE},
	{"EF_SPHERETRANS",	EF_SPHERETRANS},
	{"EF_TAGTRAIL",		EF_TAGTRAIL},
	{"EF_HALF_DAMAGE",	EF_HALF_DAMAGE},
	{"EF_TRACKERTRAIL",	EF_TRACKERTRAIL},

	{"TRANSLUCENT",		RF_TRANSLUCENT},
	{"RED_SHELL",		RF_SHELL_RED},
	{"GREEN_SHELL",		RF_SHELL_GREEN},
	{"BLUE_SHELL",		RF_SHELL_BLUE},
	{"FULL_BRIGHT",		RF_FULLBRIGHT},

	{"FLASHLIGHT",		SUP_FLASHLIGHT},
	{"GRAPPLE",			SUP_GRAPPLE},
	{"GRENADE",			SUP_GRENADE},
	{"TIMER",			SUP_TIMER},
	{"TRIPWIRE",		SUP_TRIPWIRE},
	{"PROXY",			SUP_PROXY},
	{"VIDSENSOR",		SUP_VIDSENSOR},
	{"MSGJAMMER",		SUP_MSGJAMMER},
	{"TOOL",			SUP_TOOL},
	{"ARMOUR",			SUP_ARMOUR},
	{"SIGHT",			SUP_SIGHT},
	{"UNIFORM",			SUP_UNIFORM},
	{"MEDIKIT",			SUP_MEDIKIT},
	{"SABOTAGE",		SUP_SABOTAGE},

	{"TOOL_BUILD",		TOOL_BUILD},
	{"TOOL_REPAIR",		TOOL_REPAIR},

	{"MOUNTED",			SF_MOUNTED},
	{"INFRARED",		SF_INFRARED},
	{"ID_NAME",			SF_ID_NAME},
	{"ID_WEAPON",		SF_ID_WEAPON},
	{"ID_TEAM",			SF_ID_TEAM},
	{"ID_IFF",			SF_ID_IFF},
	{"ID_RANGE",		SF_ID_RANGE},
	{"ID_STARLIGHT",	SF_ID_STARLIGHT},
	
	{"EFXBOX",			BLDG_EFXBOX},
	{"CAMERA",			BLDG_CAMERA},
	{"REMOTE_TRIGGER",	BLDG_REMOTE_TRIGGER},
	{"MOUNT",			BLDG_MOUNT},
	{"GUN",				BLDG_GUN},
	{"MSGDECODER",		BLDG_MSGDECODER},
	{"LASERGATE",		BLDG_LASERGATE},
	{"SPOTLIGHT",		BLDG_SPOTTER},

	{"ATTACH_FLOORS",		BLDG_ATTACH_FLOORS},
	{"ATTACH_WALLS",		BLDG_ATTACH_WALLS},
	{"ATTACH_CEILINGS",		BLDG_ATTACH_CEILINGS},
	{"ATTACH_PLAYERS",		BLDG_ATTACH_PLAYERS},
	{"ANGLES_SURFACE",		BLDG_ANGLES_SURFACE},
	{"YAW_FACEPLAYER",		BLDG_YAW_FACEPLAYER},
	{"PITCH_FULL",			BLDG_PITCH_FULL},
	{"PITCH_PARTIAL",		BLDG_PITCH_PARTIAL},
	{"DEFAULT_OFF",			BLDG_DEFAULT_OFF},
	{"ALLOW_PLAYERMOVE",	BLDG_ALLOW_PLAYERMOVE},
	{"ALLOW_PLAYERROTATE",	BLDG_ALLOW_PLAYERROTATE},
	{"ALLOW_PLAYERPILOT",	BLDG_ALLOW_PLAYERPILOT},
	{"USE_ON_DESTROY",		BLDG_USE_ON_DESTROY},
	
	{"LASER_RED",		0xF2F2F0F0},
	{"LASER_GREEN",		0xD0D1D2D3},
	{"LASER_BLUE",		0xF3F3F1F1},
	{"LASER_YELLOW",	0xDCDDDEDF},
	{"LASER_BLACK",		0xE0E1E2E3},

	{"ARMOUR_SPARKS",	ARMOUR_SPARKS},
	{"ARMOUR_SHELL",	ARMOUR_SHELL},
	{"ARMOUR_SHIELD",	ARMOUR_SHIELD},
	{"ARMOUR_FADE",		ARMOUR_FADE},

	{"LEGS",		LOC_LEGS},
	{"CHEST",		LOC_CHEST},
	{"HEAD",		LOC_HEAD},

	{"TE_GUNSHOT",					TE_GUNSHOT},
	{"TE_BLOOD",					TE_BLOOD},
	{"TE_BLASTER",					TE_BLASTER},
	{"TE_RAILTRAIL",				TE_RAILTRAIL},
	{"TE_SHOTGUN",					TE_SHOTGUN},
	{"TE_EXPLOSION1",				TE_EXPLOSION1},
	{"TE_EXPLOSION2",				TE_EXPLOSION2},
	{"TE_ROCKET_EXPLOSION",			TE_ROCKET_EXPLOSION},
	{"TE_GRENADE_EXPLOSION",		TE_GRENADE_EXPLOSION},
	{"TE_SPARKS",					TE_SPARKS},
	{"TE_SPLASH",					TE_SPLASH},
	{"TE_BUBBLETRAIL",				TE_BUBBLETRAIL},
	{"TE_SCREEN_SPARKS",			TE_SCREEN_SPARKS},
	{"TE_SHIELD_SPARKS",			TE_SHIELD_SPARKS},
	{"TE_BULLET_SPARKS",			TE_BULLET_SPARKS},
	{"TE_LASER_SPARKS",				TE_LASER_SPARKS},
	{"TE_PARASITE_ATTACK",			TE_PARASITE_ATTACK},
	{"TE_ROCKET_EXPLOSION_WATER",	TE_ROCKET_EXPLOSION_WATER},
	{"TE_GRENADE_EXPLOSION_WATER",	TE_GRENADE_EXPLOSION_WATER},
	{"TE_MEDIC_CABLE_ATTACK",		TE_MEDIC_CABLE_ATTACK},
	{"TE_BFG_EXPLOSION",			TE_BFG_EXPLOSION},
	{"TE_BFG_BIGEXPLOSION",			TE_BFG_BIGEXPLOSION},
	{"TE_BOSSTPORT",				TE_BOSSTPORT},
	{"TE_BFG_LASER",				TE_BFG_LASER},
	{"TE_GRAPPLE_CABLE",			TE_GRAPPLE_CABLE},
	{"TE_WELDING_SPARKS",			TE_WELDING_SPARKS},

	{"TE_GREENBLOOD",				TE_GREENBLOOD},
	{"TE_BLUEHYPERBLASTER",			TE_BLUEHYPERBLASTER},
	{"TE_PLASMA_EXPLOSION",			TE_PLASMA_EXPLOSION},
	{"TE_TUNNEL_SPARKS",			TE_TUNNEL_SPARKS},

	{"TE_BLASTER2",					TE_BLASTER2},
	{"TE_RAILTRAIL2",				TE_RAILTRAIL2},
	{"TE_FLAME",					TE_FLAME},
	{"TE_LIGHTNING",				TE_LIGHTNING},
	{"TE_DEBUGTRAIL",				TE_DEBUGTRAIL},
	{"TE_PLAIN_EXPLOSION",			TE_PLAIN_EXPLOSION},
	{"TE_FLASHLIGHT",				TE_FLASHLIGHT},
	{"TE_FORCEWALL",				TE_FORCEWALL},
	{"TE_HEATBEAM",					TE_HEATBEAM},
	{"TE_MONSTER_HEATBEAM",			TE_MONSTER_HEATBEAM},
	{"TE_STEAM",					TE_STEAM},
	{"TE_BUBBLETRAIL2",				TE_BUBBLETRAIL2},
	{"TE_MOREBLOOD",				TE_MOREBLOOD},
	{"TE_HEATBEAM_SPARKS",			TE_HEATBEAM_SPARKS},
	{"TE_HEATBEAM_STEAM",			TE_HEATBEAM_STEAM},
	{"TE_CHAINFIST_SMOKE",			TE_CHAINFIST_SMOKE},
	{"TE_ELECTRIC_SPARKS",			TE_ELECTRIC_SPARKS},
	{"TE_TRACKER_EXPLOSION",		TE_TRACKER_EXPLOSION},
	{"TE_TELEPORT_EFFECT",			TE_TELEPORT_EFFECT},
	{"TE_DBALL_GOAL",				TE_DBALL_GOAL},
	{"TE_WIDOWBEAMOUT",				TE_WIDOWBEAMOUT},
	{"TE_NUKEBLAST",				TE_NUKEBLAST},
	{"TE_WIDOWSPLASH",				TE_WIDOWSPLASH},
	{"TE_EXPLOSION1_BIG",			TE_EXPLOSION1_BIG},
	{"TE_EXPLOSION1_NP",			TE_EXPLOSION1_NP},
	{"TE_FLECHETTE",				TE_FLECHETTE},

	{"DESTROY",						AS_DESTROY},
	{"TIMEDDISABLE",				AS_TIMEDDISABLE},
	{"RANDOMDISABLE",				AS_RANDOMDISABLE},
	{"TARGETORIGIN",				AS_TARGETORIGIN},
	{"TARGETRANDOM",				AS_TARGETRANDOM},

	{"SWAPORIGINS",					EFF_SWAPORIGINS},

	{"",			0}
};

field_t worldmods[] =
{
	{"pushed",	MOD_PUSHED,		F_LSTRING},
	{"fall",	MOD_FALLING,	F_LSTRING},
	{"water",	MOD_WATER,		F_LSTRING},
	{"slime",	MOD_SLIME,		F_LSTRING},
	{"lava",	MOD_LAVA,		F_LSTRING},
	{"crush",	MOD_CRUSH,		F_LSTRING},
	{"telefrag",MOD_TELEFRAG,	F_LSTRING},
	{"suicide",	MOD_SUICIDE,	F_LSTRING},
	{"explosive",	MOD_EXPLOSIVE,	F_LSTRING},
	{"barrel",	MOD_BARREL,		F_LSTRING},
	{"bomb",	MOD_BOMB,		F_LSTRING},
	{"exit",	MOD_EXIT,		F_LSTRING},
	{"laser",	MOD_TARGET_LASER,	F_LSTRING},
	{"hurt",	MOD_TRIGGER_HURT,	F_LSTRING},
	{"blaster",	MOD_TARGET_BLASTER,	F_LSTRING},
	{"grapple",	MOD_GRAPPLE,	F_LSTRING},

	{NULL, 0, F_INT}
};

typedef struct
{
	int			ident;
	int			version;

	int			skinwidth;
	int			skinheight;
	int			framesize;		// byte size of each frame

	int			num_skins;
	int			num_xyz;
	int			num_st;			// greater than num_xyz for seams
	int			num_tris;
	int			num_glcmds;		// dwords in strip/fan command list
	int			num_frames;

	int			ofs_skins;		// each skin is a MAX_SKINNAME string
	int			ofs_st;			// byte offset from start for stverts
	int			ofs_tris;		// offset for dtriangles
	int			ofs_frames;		// offset for first frame
	int			ofs_glcmds;	
	int			ofs_end;		// end of file

} mdl_head;

typedef struct
{
	float		scale[3];		// multiply byte verts by this
	float		translate[3];	// then add this
	char		name[16];		// frame name from grabbing
} mdl_frame;

int FindMD2(const char *model) {
PFILE *FHnd;

	if ((FHnd = pfopen(model, "rb")) == NULL)
		return 0;
	pfclose(FHnd);
	return 1;
}

int GetMD2FrameSet(const char *model, const char *frameset, int *start, int *end) {
PFILE *FHnd;
mdl_head mdlHead;
mdl_frame mdlFrame;
char	*num;

int i, cur=0, min=999, max=-1;

	*start = 0;
	*end = 0;

	if ((FHnd = pfopen(model, "rb")) == NULL) {
		gi.dprintf("Unable to locate file %s.\n", model);
		return 0;
	}

	pfread(&mdlHead, 1, sizeof(mdl_head), FHnd);
	pfseek(FHnd, mdlHead.ofs_frames, SEEK_SET);

	for (i=0; i<mdlHead.num_frames; i++) {
		pfread(&mdlFrame, 1, sizeof(mdl_frame), FHnd);
		pfseek(FHnd, sizeof(int) * mdlHead.num_xyz, SEEK_CUR);
		if (!DL_strcmp(mdlFrame.name, frameset, strlen(frameset), false)) {
			num = mdlFrame.name + strlen(frameset);
			cur = atoi(num);
			if (start && (cur < min)) {
				min = cur;
				*start = i;
			}
			if (end && (cur > max)) {
				max = cur;
				*end = i;
			}
		}
	}

	pfclose(FHnd);
	return cur;
}

void GetPPMFrameSet(gclient_t *client, int ppmState, int *start, int *end) {
char *animset = NULL, anim[32];
ppmframes_t *fs, *p;
int i;

	if (ppmState < 10)
		if (client->ps.pmove.pm_flags & PMF_DUCKED)
			ppmState += 10;

	switch(ppmState) {
	case PPM_IDLE:		if (client->curweap) animset = ITEMWEAP(client->curweap)->ppmIdle_stand; fs = &s_standFrames[0]; break;
	case PPM_RUN:		if (client->curweap) animset = ITEMWEAP(client->curweap)->ppmRun_stand; fs = &s_runFrames[0]; break;
	case PPM_ATTACK:	if (client->curweap) animset = ITEMWEAP(client->curweap)->ppmAttack_stand; fs = &s_attackFrames[0]; break;
	case PPM_PAIN:		if (client->curweap) animset = ITEMWEAP(client->curweap)->ppmPain_stand; fs = &s_painFrames[0]; break;
	case PPM_DIE:		if (client->curweap) animset = ITEMWEAP(client->curweap)->ppmDie_stand; fs = &s_deathFrames[0]; break;
	case PPM_C_IDLE:	if (client->curweap) animset = ITEMWEAP(client->curweap)->ppmIdle_crouch; fs = &c_standFrames[0]; break;
	case PPM_C_RUN:		if (client->curweap) animset = ITEMWEAP(client->curweap)->ppmRun_crouch; fs = &c_runFrames[0]; break;
	case PPM_C_ATTACK:	if (client->curweap) animset = ITEMWEAP(client->curweap)->ppmAttack_crouch; fs = &c_attackFrames[0]; break;
	case PPM_C_PAIN:	if (client->curweap) animset = ITEMWEAP(client->curweap)->ppmPain_crouch; fs = &c_painFrames[0]; break;
	case PPM_C_DIE:		if (client->curweap) animset = ITEMWEAP(client->curweap)->ppmDie_crouch; fs = &c_deathFrames[0]; break;
	case PPM_JUMP:		if (client->curweap) animset = ITEMWEAP(client->curweap)->ppmJump; fs = &jumpFrames[0]; break;
	}

	if (animset) {
		if ((client->vw_index && (client->vw_index != client->ppm_oldVWeapModel)) ||
			(ppmState != client->ppm_oldState) ||
			(client->ppm_nextChange < level.time)) {
			i = random() * sub_count(animset);
			sub_string(animset, anim, i);

			DL_strcpy(client->ppm_oldSet, anim, 31);
			client->ppm_nextChange = level.time + ((random() * 15.0) + 5.0);
			client->ppm_oldState = ppmState;
			client->ppm_oldVWeapModel = client->vw_index;
		} else
			DL_strcpy(anim, client->ppm_oldSet, 31);

		p = fs;
		while(fs->name && DL_strcmp(fs->name, anim, -1, false))
			fs++;
		if (DL_strcmp(fs->name, anim, -1, false))
			fs = p;
	}

	*start = fs->s_frame;
	*end = fs->e_frame;
}

typedef struct {
	unsigned long int	riffID;
	unsigned long int	fileSize;
	unsigned long int	waveID;
	unsigned long int	chunkID;
	unsigned long int	nchunkSize;
	unsigned short int	wfmtTag;
	unsigned short int	nChannels;
	unsigned long int	nSamplesPerSec;
	unsigned long int	nAvgBytesPerSec;
	unsigned short int	nBlockAlign;
	unsigned short int	nBitsPerSample;
	unsigned long int	dataID;
	unsigned long int	dataSize;
} wav_head;

float GetWAVLength(const edict_t *ent, const char *wavfile) {
char skin[255], *p = NULL;
wav_head wavHead;
PFILE *FHnd;
	// Get the skin type, but loose the skin name/color
			
	if (wavfile[0] == '*') {
		if (ent->client) {
			DL_strcpy(skin, Info_ValueForKey(ent->client->pers.userinfo, "skin"), -1);
			p = strrchr(skin, '/');
		}
		if (p && (p != skin)) p[0] = 0;
		else DL_strcpy(skin, "male", -1);

		FHnd = pfopen(va("players/%s/%s", skin, &wavfile[1]), "rb");
	} else 
		FHnd = pfopen(wavfile, "rb");

	if (FHnd) {
		pfread(&wavHead, 1, sizeof(wav_head), FHnd);
		pfclose(FHnd);

		return (float)((wavHead.dataSize / (wavHead.nBitsPerSample / 8)) / wavHead.nChannels) / (float)(wavHead.nSamplesPerSec);
	}
	return 0;
}

int TempEntType(int te) {
	switch(te) {
	case TE_EXPLOSION1:
	case TE_EXPLOSION2:
	case TE_ROCKET_EXPLOSION:
	case TE_GRENADE_EXPLOSION:
	case TE_ROCKET_EXPLOSION_WATER:
	case TE_GRENADE_EXPLOSION_WATER:
	case TE_BOSSTPORT:
	case TE_BFG_EXPLOSION:

	case TE_DBALL_GOAL:
	case TE_PLAIN_EXPLOSION:
	case TE_NUKEBLAST:
	case TE_TRACKER_EXPLOSION:
	case TE_WIDOWBEAMOUT:
	case TE_WIDOWSPLASH:
	case TE_EXPLOSION1_BIG:
	case TE_EXPLOSION1_NP:
	case TE_CHAINFIST_SMOKE:

	case TE_FLASHLIGHT:
	case TE_PLASMA_EXPLOSION:
		return (1);
	case TE_SHIELD_SPARKS:
	case TE_SCREEN_SPARKS:
	case TE_SPARKS:
	case TE_BULLET_SPARKS:
	case TE_BLOOD:
	case TE_SHOTGUN:
	case TE_GUNSHOT:
	case TE_BLASTER:
	case TE_SPLASH:
	case TE_LASER_SPARKS:

	case TE_ELECTRIC_SPARKS:
	case TE_MOREBLOOD:
	case TE_STEAM:
	case TE_FLECHETTE:
	case TE_HEATBEAM_SPARKS:
	case TE_HEATBEAM_STEAM:
	case TE_BLASTER2:
		return (2);
	case TE_BUBBLETRAIL:
	case TE_RAILTRAIL:
	case TE_BFG_LASER:
	case TE_MEDIC_CABLE_ATTACK:
	case TE_PARASITE_ATTACK:
	case TE_GRAPPLE_CABLE:

	case TE_DEBUGTRAIL:
	case TE_FORCEWALL:
	case TE_LIGHTNING:
	case TE_BUBBLETRAIL2:
	case TE_RAILTRAIL2:
	case TE_HEATBEAM:
		return (3);
	default:
		return (0);
	}
}

void CastExplosion(int te, const edict_t *ent, vec3_t origin) {
	// TE_EXPLOSION1
	// TE_EXPLOSION2
	// TE_ROCKET_EXPLOSION
	// TE_GRENADE_EXPLOSION
	// TE_ROCKET_EXPLOSION_WATER
	// TE_GRENADE_EXPLOSION_WATER
	// TE_BOSSTPORT
	// TE_BFG_EXPLOSION
	// TE_TELEPORT_EFFECT

	// TE_DBALL_GOAL
	// TE_PLAIN_EXPLOSION
	// TE_NUKEBLAST
	// TE_TRACKER_EXPLOSION
	// TE_WIDOWBEAMOUT
	// TE_WIDOWSPLASH
	// TE_EXPLOSION1_BIG
	// TE_EXPLOSION1_NP
	// TE_CHAINFIST_SMOKE

	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(te);

	if (te == TE_WIDOWBEAMOUT)
		gi.WriteShort(20001); // or 20002 (unknown)

	gi.WritePosition(origin);

	if (te == TE_FLASHLIGHT)
		gi.WriteShort (ent - g_edicts);
}

void CastFountain (int te, vec3_t origin, vec3_t normal, int count, int color, int misc) {
	// TE_SHIELD_SPARKS
	// TE_SCREEN_SPARKS
	// TE_SPARKS
	// TE_BULLET_SPARKS
	// TE_BLOOD
	// TE_SHOTGUN
	// TE_GUNSHOT
	// TE_BLASTER

	// TE_SPLASH
	// TE_LASER_SPARKS

	// TE_ELECTRIC_SPARKS
	// TE_MOREBLOOD
	// TE_STEAM
	// TE_FLECHETTE
	// TE_HEATBEAM_SPARKS
	// TE_HEATBEAM_STEAM
	// TE_BLASTER2

	// TE_BLUEHYPERBLASTER

	if (count > 255) count = 255;
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (te);

	if (te == TE_STEAM)
		gi.WriteShort (1);		// unique ID?  

	if (te == TE_SPLASH || te == TE_LASER_SPARKS || te == TE_STEAM)
		gi.WriteByte (count);

	gi.WritePosition (origin);
	gi.WriteDir (normal);

	if (te == TE_SPLASH || te == TE_LASER_SPARKS || te == TE_STEAM)
		gi.WriteByte (color);

	if (te == TE_STEAM) {
		gi.WriteShort (misc);	// Speed ??
		gi.WriteLong ((int)10000);		// Timer/Duration (optional)
	}
}

void CastTrail (int te, const edict_t *ent, const edict_t *targ, vec3_t origin, vec3_t dest, vec3_t offset, int misc) {
vec3_t	off, start, end;
	// TE_BUBBLETRAIL
	// TE_RAILTRAIL
	// TE_BFG_LASER

	// TE_MEDIC_CABLE_ATTACK
	// TE_PARASITE_ATTACK
	// TE_GRAPPLE_CABLE

	// TE_DEBUGTRAIL
	// TE_FORCEWALL
	// TE_LIGHTNING
	// TE_BUBBLETRAIL2
	// TE_RAILTRAIL2
	// TE_HEATBEAM

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (te);

	if (te == TE_MEDIC_CABLE_ATTACK || te == TE_PARASITE_ATTACK || te == TE_GRAPPLE_CABLE || te == TE_LIGHTNING || te == TE_HEATBEAM)
		gi.WriteShort (ent - g_edicts);

	if (te == TE_LIGHTNING)
		gi.WriteShort (targ - g_edicts);

	VectorCopy(origin, start);
	VectorCopy(dest, end);
	if (te == TE_MEDIC_CABLE_ATTACK || te == TE_PARASITE_ATTACK) {
		VectorSubtract(end, start, off);
		VectorNormalize(off);
		VectorMA(start, 16, off, start);
		VectorMA(end, 16, off, end);
	}

	gi.WritePosition (start);
	gi.WritePosition (end);

	if (te == TE_GRAPPLE_CABLE) {
		VectorSubtract(origin, ent->s.origin, off);
		gi.WritePosition(off);
	}

	if (te == TE_FORCEWALL)
		gi.WriteByte(misc);
}

//	gi.WriteByte (svc_temp_entity);
//	gi.WriteByte (TE_BLOOD_FOUNTAIN);
//	gi.WritePosition (origin);
//	gi.WritePosition (velocity);
//	gi.WriteShort (50);
//	gi.multicast (self->s.origin, MULTICAST_ALL);


//	gi.WriteByte (svc_temp_entity);
//	gi.WriteByte (TE_FLAME);
//	gi.WriteShort(ent - g_edicts);
//	gi.WriteShort(6);
//	gi.WritePosition (start);
//	gi.WritePosition (flame->s.origin);
//	gi.WritePosition (flame->flameinfo.pos1);
//	gi.WritePosition (flame->flameinfo.pos2);
//	gi.WritePosition (flame->flameinfo.pos3);
//	gi.WritePosition (flame->flameinfo.pos4);
//	gi.multicast (flame->s.origin, MULTICAST_PVS);

void MCastTempEnt(int te, const edict_t *ent, const edict_t *targ, vec3_t origin, vec3_t normal, vec3_t offset, int count, int color, int misc, int mcast) {
	switch(TempEntType(te)) {
	case 1:	CastExplosion(te, ent, origin); break;
	case 2:	CastFountain(te, origin, normal, count, color, misc); break;
	case 3:	CastTrail(te, ent, targ, origin, normal, offset, misc);	break;
	}
	gi.multicast (origin, mcast);
}

void UCastTempEnt(int te, const edict_t *ent, const edict_t *targ, vec3_t origin, vec3_t normal, vec3_t offset, int count, int color, int misc, const edict_t *utarg) {
	switch(TempEntType(te)) {
	case 1:	CastExplosion(te, ent, origin); break;
	case 2:	CastFountain(te, origin, normal, count, color, misc); break;
	case 3:	CastTrail(te, ent, targ, origin, normal, offset, misc);	break;
	}
	gi.unicast (utarg, false);
}

void MuzzleFlash (const edict_t *ent, int flash, multicast_t who) {
	// must intercept muzzleflashes because
	// the player ent might not be sent to client (SVF_NOCLIENT)
	if (!ent)
		return;

	if (ent->client)
		if (ent->client->dummy)
			ent = ent->client->dummy;
	
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (flash);
	gi.multicast (ent->s.origin, who);
}

void MuzzleFlash2(const edict_t *ent, int flash, multicast_t who) {
	// must intercept muzzleflashes because
	// the player ent might not be sent to client (SVF_NOCLIENT)
	if (!ent)
		return;
		
	if (ent->client)
		if (ent->client->dummy)
			ent = ent->client->dummy;
	
	gi.WriteByte (svc_muzzleflash2);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (flash);
	gi.multicast (ent->s.origin, who);
}

#define svc_sound 9

#define MASK_VOLUME 			1
#define MASK_ATTENUATION 		2
#define MASK_POSITION 			4
#define MASK_ENTITY_CHANNEL 	8
#define MASK_TIMEOFS 			16	

void unicastSound(const edict_t *player, int soundIndex, float volume) {
int mask = MASK_ENTITY_CHANNEL;

	if (volume != 1.0)
		mask |= MASK_VOLUME;

	gi.WriteByte(svc_sound);
	gi.WriteByte((byte)mask);
	gi.WriteByte((byte)soundIndex);
	if (mask & MASK_VOLUME) {
		gi.WriteByte((byte)(volume * 255));
	}

	gi.WriteShort(((player - g_edicts - 1) << 3) + CHAN_NO_PHS_ADD);
	//gi.WriteShort(((player - g_edicts) << 3) + CHAN_NO_PHS_ADD);

	gi.unicast (player, false);
}


// Find the far-side of an object and returns its thinkness, returns -1 if maxdist is reached
float DL_FindFarSide (vec3_t origin, vec3_t dir, vec3_t fside, float maxdist) {
vec3_t	pos;
trace_t	tr;

	VectorNormalize(dir);
	VectorMA(origin, maxdist, dir, pos);
	if (gi.pointcontents(pos) & MASK_SOLID)
		return -1.0;

	tr = gi.trace(pos, NULL, NULL, origin, NULL, MASK_SOLID);
	VectorCopy(tr.endpos, fside);
	return (1.0 - tr.fraction) * maxdist;
}


// changed to allow camera-based & flash grenade blending
void DL_CalcBlend (edict_t *ent)
{
	int		contents;
	vec3_t	vieworg;
	int		remaining;

	ent->client->ps.blend[0] = ent->client->ps.blend[1] = 
		ent->client->ps.blend[2] = ent->client->ps.blend[3] = 0;

	if (ent->client->camera && ent->client->viewcam)
		VectorCopy (ent->client->camera->s.origin, vieworg);
	else
	{
		// add for contents
		VectorAdd (ent->s.origin, ent->client->ps.viewoffset, vieworg);
	}
	contents = gi.pointcontents (vieworg);
	if (contents & (CONTENTS_LAVA|CONTENTS_SLIME|CONTENTS_WATER) )
		ent->client->ps.rdflags |= RDF_UNDERWATER;
	else
		ent->client->ps.rdflags &= ~RDF_UNDERWATER;

	if (contents & (CONTENTS_SOLID|CONTENTS_LAVA))
		SV_AddBlend (1.0, 0.3, 0.0, 0.6, ent->client->ps.blend);
	else if (contents & CONTENTS_SLIME)
		SV_AddBlend (0.0, 0.1, 0.05, 0.6, ent->client->ps.blend);
	else if (contents & CONTENTS_WATER)
		SV_AddBlend (0.5, 0.3, 0.2, 0.4, ent->client->ps.blend);

	// Flash grenade
	if (ent->client->flash_alpha > 0) {
		SV_AddBlend (1, 1, 1, ent->client->flash_alpha, ent->client->ps.blend);
		ent->client->flash_alpha -= ent->client->flash_fade;
	}
	if (ent->client->flash_alpha < 0) {
		ent->client->flash_alpha = 0;
		ent->client->flash_fade = 0;
	} else if (ent->client->flash_alpha > 2)
		ent->client->flash_alpha = 2;

	// Night Vision
	if (ent->client->night_vision) {
		ent->client->ps.rdflags |= RDF_IRGOGGLES;
		SV_AddBlend (0, 0.5, 0, 0.25, ent->client->ps.blend);
	} else
		if (ent->client->ps.rdflags & RDF_IRGOGGLES)
			ent->client->ps.rdflags &= ~RDF_IRGOGGLES;

	ent->client->night_vision = false;

	if (ent->client->dummy)
		return;

	// Concussion grenade - view blacks out, opposite to flash, but shorted duration
	// Don't tint/effect clients viewing cameras, since thier body may not be near the camera
	// and the camera itself is immune to concussion effects.
	if (ent->client->blackout_alpha > 0) {
		SV_AddBlend (0, 0, 0, ent->client->blackout_alpha, ent->client->ps.blend);
		ent->client->blackout_alpha -= ent->client->blackout_fade;
	}
	if (ent->client->blackout_alpha < 0) {
		ent->client->blackout_alpha = 0;
		ent->client->blackout_fade = 0;
	} else if (ent->client->blackout_alpha > 2)
		ent->client->blackout_alpha = 2;

	// Smokes/Poisons/Other - Tint screen specified color, and allow fade out
	if (ent->client->tint_alpha > 0) {
		SV_AddBlend (ent->client->tint[0], ent->client->tint[1], ent->client->tint[2],
			ent->client->tint_alpha, ent->client->ps.blend);
		ent->client->tint_alpha -= ent->client->tint_fade;
	}
	if (ent->client->tint_alpha < 0) {
		ent->client->tint_alpha = 0;
		ent->client->tint_fade = 0;
	} else if (ent->client->tint_alpha > 2)
		ent->client->tint_alpha = 2;

	// add for powerups
	if (ent->client->quad_framenum > level.framenum)
	{
		remaining = ent->client->quad_framenum - level.framenum;
		if (remaining == 30)	// beginning to fade
			gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage2.wav"), 1, ATTN_NORM, 0);
		if (remaining > 30 || (remaining & 4) )
			SV_AddBlend (0, 0, 1, 0.08, ent->client->ps.blend);
	}
	else if (ent->client->invincible_framenum > level.framenum)
	{
		remaining = ent->client->invincible_framenum - level.framenum;
		if (remaining == 30)	// beginning to fade
			gi.sound(ent, CHAN_ITEM, gi.soundindex("items/protect2.wav"), 1, ATTN_NORM, 0);
		if (remaining > 30 || (remaining & 4) )
			SV_AddBlend (1, 1, 0, 0.08, ent->client->ps.blend);
	}
	else if (ent->client->enviro_framenum > level.framenum)
	{
		remaining = ent->client->enviro_framenum - level.framenum;
		if (remaining == 30)	// beginning to fade
			gi.sound(ent, CHAN_ITEM, gi.soundindex("items/airout.wav"), 1, ATTN_NORM, 0);
		if (remaining > 30 || (remaining & 4) )
			SV_AddBlend (0, 1, 0, 0.08, ent->client->ps.blend);
	}
	else if (ent->client->breather_framenum > level.framenum)
	{
		remaining = ent->client->breather_framenum - level.framenum;
		if (remaining == 30)	// beginning to fade
			gi.sound(ent, CHAN_ITEM, gi.soundindex("items/airout.wav"), 1, ATTN_NORM, 0);
		if (remaining > 30 || (remaining & 4) )
			SV_AddBlend (0.4, 1, 0.4, 0.04, ent->client->ps.blend);
	}

	// add for damage
	if (ent->client->damage_alpha > 0)
		SV_AddBlend (ent->client->damage_blend[0],ent->client->damage_blend[1]
		,ent->client->damage_blend[2], ent->client->damage_alpha, ent->client->ps.blend);

	if (ent->client->bonus_alpha > 0)
		SV_AddBlend (0.85, 0.7, 0.3, ent->client->bonus_alpha, ent->client->ps.blend);

	// drop the damage value
	ent->client->damage_alpha -= 0.06;
	if (ent->client->damage_alpha < 0)
		ent->client->damage_alpha = 0;

	// drop the bonus value
	ent->client->bonus_alpha -= 0.1;
	if (ent->client->bonus_alpha < 0)
		ent->client->bonus_alpha = 0;
}

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

int IsIdent(const char *ident)
{
	int i;

	for (i = 0; identSet[i].name[0] && DL_strcmp(identSet[i].name, ident, -1, false); i++);
	if (!identSet[i].name[0])
		return 1;
	return 0;
}

int DL_GetIdentValue(const char *ident, int *result) {
int i;

	if (isdigit(ident[0])) {
		*result = strtol(ident, NULL, 0);
		return 1;
	}

	for (i=0; identSet[i].name[0] && DL_strcmp(identSet[i].name, ident, -1, false); i++);
	if (!identSet[i].name[0]) {
		*result = 0.0;
		return 0;
	}
	*result = identSet[i].value;
	return 1;
}

void DL_StrToType(const char *value, byte *b, field_t *f) {
vec3_t	vec;
float	v;
double	temp;
char	**str;

	switch (f->type) {
	case F_LSTRING:
		str = (char **)b;
		if (*str)
			gi.TagFree(*str);
		*str = DL_strcpy(NULL, value, -1);
		break;
	case F_VECTOR:
	case F_ANGLE:
		sscanf (value, "%f %f %f", &vec[0], &vec[1], &vec[2]);
		if (f->type == F_ANGLE)
		{
			vec[0] = anglemod(vec[0]);
			vec[1] = anglemod(vec[1]);
			vec[2] = anglemod(vec[2]);
		}
		((float *)b)[0] = vec[0];
		((float *)b)[1] = vec[1];
		((float *)b)[2] = vec[2];
		break;
	case F_INT:
		if(evaluate(value, &temp, 0, 0, 0))
			gi.dprintf("Error evaluating %s = %s\n", f->name, value);
		else
			*(int *)b = (int)temp;
		break;
	case F_FLOAT:
		*(float *)b = atof(value);
		v = *(float *)b;
		break;
	case F_ANGLEHACK:
		v = atof(value);
		((float *)b)[0] = 0;
		((float *)b)[1] = v;
		((float *)b)[2] = 0;
		break;
	case F_IGNORE:
		break;
	}
}

void ReadIniFile(const char *filename, void *dest, int elementsize, int numelements, field_t *fields, char *err_exceeded)
{
	IniFile	*IHnd;
	char	*section, *value, buf[256], **str;
	field_t	*f;
	int		i, j, nameofs;
	byte	*base, *b;

	if ((IHnd = Ini_ReadIniFile(filename)) == NULL) {
		gi.dprintf("Error: Unable to read from %s\n", filename);
		return;
	} else
		gi.dprintf("Reading %s...", filename);

	nameofs = 0;
	for (f = fields; f->name; f++)
	{
		if (!DL_strcmp(f->name, "name", -1, false))
			nameofs = f->ofs;
	}

	for (i = 0; i < IHnd->number_of_sections; i++)
	{
		section = IHnd->section_names[i];
		for (j = 0; j <= numelements; j++)
		{
			// no slots available
			if (j >= numelements)
			{
				gi.dprintf("Warning: %s limit of %d exceeded\n", err_exceeded, numelements);
				Ini_FreeIniFile(IHnd);
				return;
			}
			base = (byte *)dest + (elementsize*j);
			str = (char **)(base + nameofs);
			if (!*str || !DL_strcmp(section, *str, -1, false))
				break;
		}
		if (!*str)
			*str = DL_strcpy(NULL, section, -1);

		for (f = fields; f->name; f++)
		{
			if (f->arraysize)
			{
				for (j = 0; j < f->arraysize; j++)
				{
					b = base + f->ofs + (j * f->elementsize);
					// ini arrays start with 1 instead of 0
					Com_sprintf(buf, sizeof(buf), "%s[%d]", f->name, j+1);
					value = Ini_ReadString(IHnd, section, buf, NULL);
					if (!value || !value[0])
						break;
					DL_StrToType(value, b, f);
				}
			}
			else
			{
				b = base + f->ofs;
				value = Ini_ReadString(IHnd, section, f->name, NULL);
				if (!value || !value[0])
					continue;
				DL_StrToType(value, b, f);
			}
		}
	}
	gi.dprintf("(%d of %d), ", IHnd->number_of_sections, numelements);
	Ini_FreeIniFile(IHnd);
	gi.dprintf("Done\n");
}

void ReadAmmoIniFile(const char *infile, qboolean clean)
{
	char	filename[MAX_QPATH];
	int		i;

	sprintf(filename, "%s/%s", dlsys.gamedir->string, infile);

	if (clean) memset(&ammo[0], 0, sizeof(ammo_t) * MAX_MUNITIONS);
	else memset(&ammo[0], 0, sizeof(ammo_t) * MAX_INI_MUNITIONS);
	
	for (i = 0; i < MAX_WEAPONS; i++) {
		ammo[i].pellet_count = 1;
	}

	ReadIniFile(filename, ammo, sizeof(ammo[0]), MAX_INI_MUNITIONS, dl_ammoflds, "Ammo Variant");

	for (i = 0; i < MAX_MUNITIONS; i++)
		if (ammo[i].name)	gi.configstring(CS_AMMO_VARS+i, ammo[i].name);
}

void ReadWeaponsIniFile(const char *infile, qboolean clean)
{
	char	filename[MAX_QPATH];
	int		i, start=0, end=0;

	sprintf(filename, "%s/%s", dlsys.gamedir->string, infile);
	
	if (clean) memset(&weapons[0], 0, sizeof(weap_t) * MAX_WEAPONS);
	else memset(&weapons[0], 0, sizeof(weap_t) * MAX_INI_WEAPONS);

	// only do this once; sets defaults for all weapons
	for (i = 0; i < MAX_WEAPONS; i++) {
		weapons[i].clip_size = 1;
		weapons[i].rnds_chambered = 1;
	}

	ReadIniFile(filename, weapons, sizeof(weapons[0]), MAX_INI_WEAPONS, dl_weapflds, "Weapon");

	for (i = 0; i < MAX_WEAPONS; i++)
	{
		if (weapons[i].name)
		{
			gi.configstring(CS_WEAPONS+i, weapons[i].name);
			if (weapons[i].view_model) {
				if (!FindMD2(weapons[i].view_model)) {
					// if no model is found, dump the weapon from game play
					gi.dprintf("Model not found: %s\n", weapons[i].view_model);
					memset(&weapons[i], 0, sizeof(weapons[i]));
					continue;
				}
				if (!weapons[i].frame_raise_num) {
					start = end = 0;
					GetMD2FrameSet(weapons[i].view_model, "active", &start, &end);
					weapons[i].frame_raise = start;
					weapons[i].frame_raise_num = end-start;
				}
	
				if (!weapons[i].frame_fire_num) {
					start = end = 0;
					GetMD2FrameSet(weapons[i].view_model, "pow", &start, &end);
					weapons[i].frame_fire = start;
					weapons[i].frame_fire_num = end-start;
				}

				if (!weapons[i].frame_idle_num) {
					start = end = 0;
					GetMD2FrameSet(weapons[i].view_model, "idle", &start, &end);
					weapons[i].frame_idle = start;
					weapons[i].frame_idle_num = end-start;
				}

				if (!weapons[i].frame_drop_num) {	// ID's modelers can spell, and use "putway"
					start = end = 0;
					GetMD2FrameSet(weapons[i].view_model, "putway", &start, &end);
					weapons[i].frame_drop = start;
					weapons[i].frame_drop_num = end-start;
				}
				if (!weapons[i].frame_drop_num) {	// For our own models, check for "putaway"
					start = end = 0;
					GetMD2FrameSet(weapons[i].view_model, "putaway", &start, &end);
					weapons[i].frame_drop = start;
					weapons[i].frame_drop_num = end-start;
				}
				if (!weapons[i].frame_reload_num) {
					start = end = 0;
					GetMD2FrameSet(weapons[i].view_model, "reload", &start, &end);
					weapons[i].frame_reload = start;
					weapons[i].frame_reload_num = end-start;
				}
			}
		}
	}
}

void ReadEffectsIniFile(const char *infile, qboolean clean)
{
	char	filename[MAX_QPATH];

	sprintf(filename, "%s/%s", dlsys.gamedir->string, infile);
	
	if (clean) memset(&effects[0], 0, sizeof(effect_t) * MAX_EFFECTS);
	else memset(&effects[0], 0, sizeof(effect_t) * MAX_INI_EFFECTS);

	ReadIniFile(filename, effects, sizeof(effects[0]), MAX_INI_EFFECTS, dl_efxflds, "Effects");
}

void ReadSuppliesIniFile(const char *infile, qboolean clean)
{
	char	filename[MAX_QPATH];

	sprintf(filename, "%s/%s", dlsys.gamedir->string, infile);
	
	if (clean) memset(supplies, 0, sizeof(supply_t) * MAX_SUPPLIES);
	else memset(supplies, 0, sizeof(supply_t) * MAX_INI_SUPPLIES);

	ReadIniFile(filename, supplies, sizeof(supplies[0]), MAX_INI_SUPPLIES, dl_supflds, "Supply");
}

void ReadBuildingsIniFile(const char *infile, qboolean clean)
{
	char	filename[MAX_QPATH];

	sprintf(filename, "%s/%s", dlsys.gamedir->string, infile);
	
	if (clean) memset(buildings, 0, sizeof(building_t) * MAX_BUILDINGS);
	else memset(buildings, 0, sizeof(building_t) * MAX_INI_BUILDINGS);

	ReadIniFile(filename, buildings, sizeof(buildings[0]), MAX_INI_BUILDINGS, dl_bldflds, "Building");
}

// Small util function for ValidateINIEntries that simplifies Effects checking
int CheckEffects(const char *name, const char *efx) {
char buff[32];
int i, j;

	j = sub_count(efx);
	for (i = 0; i < j; i++) {
		if (GetEffectByName (sub_string(efx, buff, i)) == NULL) {
			gi.dprintf ("%s: No such effect %s\n", name, buff);
			return 0;
		}
	}
	return 1;
}

// Make all structs have valid data
// Also, precache models
void ValidateIniEntries(void) {
int i, j;

	gi.imageindex("ammo/a_generic_s");
	gi.imageindex("ammo/a_generic");
	gi.imageindex("weapons/w_generic_s");
	gi.imageindex("weapons/w_generic");

	// Ammo variants
	for (i = 0; i < MAX_MUNITIONS; i++) {
		if (!ammo[i].name)
			break;
		
		if (!ammo[i].base_name)
		{
			gi.dprintf("%s has no base ammo\n", ammo[i].name);
			gi.TagFree(ammo[i].name);
			ammo[i].name = NULL;
		}

		if (ammo[i].effect) CheckEffects(ammo[i].name, ammo[i].effect);
		
		// Precaches
		if (ammo[i].sound) gi.soundindex (ammo[i].sound);
		if (ammo[i].model) gi.modelindex (ammo[i].model);
		if (ammo[i].pic_s) gi.imageindex (ammo[i].pic_s);
		if (ammo[i].pic_b) gi.imageindex (ammo[i].pic_b);
	}

	// Weapons
	for (i = 0; i < MAX_WEAPONS; i++) {
		if (!weapons[i].name)
			break;

		// check clipsize
		if (weapons[i].clip_size < weapons[i].rnds_chambered)
			weapons[i].clip_size = weapons[i].rnds_chambered;

		// Precaches
		if (weapons[i].pic_s) gi.imageindex(weapons[i].pic_s);
		else weapons[i].pic_s = "weapons/w_generic_s";
		if (weapons[i].pic_b) gi.imageindex(weapons[i].pic_b);
		else weapons[i].pic_b = "weapons/w_generic";

		if (weapons[i].snd_raise) gi.soundindex(weapons[i].snd_raise);
		if (weapons[i].snd_fire) gi.soundindex(weapons[i].snd_fire);
		if (weapons[i].snd_idle) gi.soundindex(weapons[i].snd_idle);
		if (weapons[i].snd_drop) gi.soundindex(weapons[i].snd_drop);
		if (weapons[i].snd_reload) gi.soundindex(weapons[i].snd_reload);
		if (weapons[i].view_model) gi.modelindex(weapons[i].view_model);
	}

	// Effects
	for (i = 0; i < MAX_EFFECTS; i++) {
		if (!effects[i].name)
			break;

		// Precaches
		if (effects[i].model) gi.modelindex(effects[i].model);
	}

	// Supplies
	for (i = 0; i < MAX_SUPPLIES; i++) {
		if (!supplies[i].name)
			break;

//		if (supplies[i].ammo && GetBaseAmmoByName(supplies[i].ammo) == NULL)
//			gi.dprintf ("%s: No such base ammo %s\n", supplies[i].name, supplies[i].ammo);

		// Precaches
		if (supplies[i].pic_s) gi.imageindex(supplies[i].pic_s);
		if (supplies[i].pic_b) gi.imageindex(supplies[i].pic_b);
		if (supplies[i].s_use) gi.soundindex(supplies[i].s_use);
		if (supplies[i].model) gi.modelindex(supplies[i].model);

		if (supplies[i].effect)
			CheckEffects(supplies[i].name, supplies[i].effect);
	}

	// Buildings
	for (i = 0; i < MAX_BUILDINGS; i++) {
		if (!buildings[i].name)
			break;

		for (j = 0; j < MAX_BLDG_WEAPONS; j++)
		{
			if (buildings[i].weapon[j].name && !GetWeaponByName(buildings[i].weapon[j].name))
				gi.dprintf("%s: No such weapon %s\n", buildings[i].name, buildings[i].weapon[j].name);
		}
//		if (buildings[i].weapon1 && GetWeaponByName(buildings[i].weapon1) == NULL)
//			gi.dprintf ("%s: No such weapon %s\n", buildings[i].name, buildings[i].weapon1);
//		if (buildings[i].weapon2 && GetWeaponByName(buildings[i].weapon2) == NULL)
//			gi.dprintf ("%s: No such weapon %s\n", buildings[i].name, buildings[i].weapon2);
//		if (buildings[i].weapon3 && GetWeaponByName(buildings[i].weapon3) == NULL)
//			gi.dprintf ("%s: No such weapon %s\n", buildings[i].name, buildings[i].weapon3);
//		if (buildings[i].weapon4 && GetWeaponByName(buildings[i].weapon4) == NULL)
//			gi.dprintf ("%s: No such weapon %s\n", buildings[i].name, buildings[i].weapon4);

		if (buildings[i].effect)
			CheckEffects(buildings[i].name, buildings[i].effect);

		if (buildings[i].upgrade && GetBuildingByName(buildings[i].upgrade) == NULL)
			gi.dprintf ("%s: No such building upgrade %s\n", buildings[i].name, buildings[i].upgrade);

		// Precaches
		if (buildings[i].pic_s)		gi.imageindex(buildings[i].pic_s);
		if (buildings[i].pic_b)		gi.imageindex(buildings[i].pic_b);
		if (buildings[i].s_start)	gi.soundindex(buildings[i].s_start);
		if (buildings[i].s_done)	gi.soundindex(buildings[i].s_done);
		if (buildings[i].s_idle)	gi.soundindex(buildings[i].s_idle);
		if (buildings[i].s_die)		gi.soundindex(buildings[i].s_die);
		if (buildings[i].model)		gi.modelindex(buildings[i].model);
	}
}
int blue_targ_sup, blue_sup;
int red_targ_sup, red_sup;
int LoadMOTD(const char *filename);

void DeadLodeInit(qboolean full) {
char filename[MAX_QPATH], *value;
double temp;
int i;
static int game_init = false;

    memset(&dlsys, 0, sizeof(dlsys_t));
	dlsys.num_teams = 2;

	gi.dprintf("------ Deadlode2 Initializing ------\n");

	// CTF Map support - resupply pads.
	red_targ_sup = red_sup = 0;
	blue_targ_sup = blue_sup = 0;
	
	// setup uservars
	gi.cvar("passkey", "", CVAR_USERINFO);

	dlsys.gamedir = gi.cvar ("game", "baseq2", CVAR_SERVERINFO | CVAR_LATCH);
	dlsys.inidir = gi.cvar ("inidir", ".", CVAR_SERVERINFO | CVAR_LATCH);

	// if (dlsys.sysIni)
	//	Ini_FreeIniFile(dlsys.sysIni); // Free old INI file, just in case

	sprintf(filename, "%s/%s/system.ini", dlsys.gamedir->string, dlsys.inidir->string);
	if ((dlsys.sysIni = Ini_ReadIniFile(filename)) == NULL) {
		gi.dprintf("Error: Unable to read from %s\n", filename);
		return;
	}

	// respawn_delay
	dlsys.respawn_delay = Ini_ReadDouble(dlsys.sysIni, "System", "RespawnDelay", 0.0);

	// map voting
	value = Ini_ReadString(dlsys.sysIni, "System", "MapVoting", "on");
	if (value && !DL_strcmp(value, "off", -1, false)) dlsys.map_voting = false;
	else dlsys.map_voting = true;

	// Force MOTD display
	value = Ini_ReadString(dlsys.sysIni, "System", "ForceMOTD", "off");
	if (value && !DL_strcmp(value, "off", -1, false)) dlsys.force_motd = false;
	else dlsys.force_motd = true;

	// custom classes
	value = Ini_ReadString(dlsys.sysIni, "System", "PlayerProfiles", "on");
	if (value && !DL_strcmp(value, "off", -1, false)) dlsys.player_profiles = false;
	else dlsys.player_profiles = true;

	// auto team
	value = Ini_ReadString(dlsys.sysIni, "System", "AutoTeam", "off");
	if (!value || !DL_strcmp(value, "off", -1, false))
		dlsys.auto_team = false;
	else if (!DL_GetIdentValue(value, &dlsys.auto_team)) {
		gi.dprintf("AutoTeams: %s is not a valid option. AutoTeams disabled.\n", value);
		dlsys.auto_team = false;
	}

	// Play Modes - No weight, cheats, etc..
	value = Ini_ReadString(dlsys.sysIni, "System", "PlayModes", NULL);
	if (value) {
		if(!evaluate(value, &temp, 0, 0, 0))
			dlsys.playmode = (int)temp;
		else {
			gi.dprintf("PlayModes: Invalid Identifier: %s\n", value);
			dlsys.playmode = 0;
		}
	} else
		dlsys.playmode = 0;


	for (i=0; i<MAX_TEAMS; i++) {
		dlsys.teams[i].team_no = i+1;
		switch (i) {
		case 0:
				dlsys.teams[i].team_name =	Ini_ReadString(dlsys.sysIni, "Team 1", "Name",    "Red");
				dlsys.teams[i].pic_s =		Ini_ReadString(dlsys.sysIni, "Team 1", "pic_s",   "team1/i_team_s");
				dlsys.teams[i].pic_b =		Ini_ReadString(dlsys.sysIni, "Team 1", "pic_b",   "team1/i_team");
				dlsys.teams[i].banner =		Ini_ReadString(dlsys.sysIni, "Team 1", "banner",  "models/team1/banner.md2");
				dlsys.teams[i].sbanner =	Ini_ReadString(dlsys.sysIni, "Team 1", "small_banner",  "models/team1/banner_small.md2");
				dlsys.teams[i].skin =		Ini_ReadString(dlsys.sysIni, "Team 1", "skin",    "dl2_red");
				dlsys.teams[i].ctfflag =	Ini_ReadString(dlsys.sysIni, "Team 1", "ctfflag", "players/male/flag1.md2");
				dlsys.teams[i].capwav =		Ini_ReadString(dlsys.sysIni, "Team 1", "capturewav", NULL);
			break;
		case 1:
				dlsys.teams[i].team_name =	Ini_ReadString(dlsys.sysIni, "Team 2", "Name",    "Blue");
				dlsys.teams[i].pic_s =		Ini_ReadString(dlsys.sysIni, "Team 2", "pic_s",   "team2/i_team_s");
				dlsys.teams[i].pic_b =		Ini_ReadString(dlsys.sysIni, "Team 2", "pic_b",   "team2/i_team");
				dlsys.teams[i].banner =		Ini_ReadString(dlsys.sysIni, "Team 2", "banner",  "models/team2/banner.md2");
				dlsys.teams[i].sbanner =	Ini_ReadString(dlsys.sysIni, "Team 2", "small_banner",  "models/team2/banner_small.md2");
				dlsys.teams[i].skin =		Ini_ReadString(dlsys.sysIni, "Team 2", "skin",    "dl2_blue");
				dlsys.teams[i].ctfflag =	Ini_ReadString(dlsys.sysIni, "Team 2", "ctfflag", "players/male/flag2.md2");
				dlsys.teams[i].capwav =		Ini_ReadString(dlsys.sysIni, "Team 2", "capturewav", NULL);
			break;
		case 2:
				dlsys.teams[i].team_name =	Ini_ReadString(dlsys.sysIni, "Team 3", "Name", "Green");
				dlsys.teams[i].pic_s =		Ini_ReadString(dlsys.sysIni, "Team 3", "pic_s","team3/i_team_s");
				dlsys.teams[i].pic_b =		Ini_ReadString(dlsys.sysIni, "Team 3", "pic_b","team3/i_team");
				dlsys.teams[i].banner =		Ini_ReadString(dlsys.sysIni, "Team 3", "banner",  "models/team3/banner.md2");
				dlsys.teams[i].sbanner =	Ini_ReadString(dlsys.sysIni, "Team 3", "small_banner",  "models/team3/banner_small.md2");
				dlsys.teams[i].skin =		Ini_ReadString(dlsys.sysIni, "Team 3", "skin", "dl2_green");
			break;
		case 3:
				dlsys.teams[i].team_name =	Ini_ReadString(dlsys.sysIni, "Team 4", "Name", "Yellow");
				dlsys.teams[i].pic_s =		Ini_ReadString(dlsys.sysIni, "Team 4", "pic_s","team4/i_team_s");
				dlsys.teams[i].pic_b =		Ini_ReadString(dlsys.sysIni, "Team 4", "pic_b","team4/i_team");
				dlsys.teams[i].banner =		Ini_ReadString(dlsys.sysIni, "Team 4", "banner",  "models/team4/banner.md2");
				dlsys.teams[i].sbanner =	Ini_ReadString(dlsys.sysIni, "Team 4", "small_banner",  "models/team4/banner_small.md2");
				dlsys.teams[i].skin =		Ini_ReadString(dlsys.sysIni, "Team 4", "skin", "dl2_yellow");
			break;
		/*
		case 4:	dlsys.teams[i].team_name = "Purple"; break;
		case 5:	dlsys.teams[i].team_name = "Aqua"; break;
		case 6:	dlsys.teams[i].team_name = "White"; break;
		case 7:	dlsys.teams[i].team_name = "Black"; break;
		*/
		default: dlsys.teams[i].team_name = "Unkown";
				dlsys.teams[i].pic_s = "t_none_s";
				dlsys.teams[i].pic_b = "t_none_b";
				dlsys.teams[i].skin = "";
			break;
		}
		gi.configstring(CS_TEAM_NAMES+i, dlsys.teams[i].team_name);
	}

/*
	if (!game_init) {
		game_init = true;
		return;
	}
*/
	ReadAmmoIniFile(va("%s/munitions.ini", dlsys.inidir->string), full);
	ReadWeaponsIniFile(va("%s/weapons.ini", dlsys.inidir->string), full);
	ReadEffectsIniFile(va("%s/effects.ini", dlsys.inidir->string), full);
	ReadSuppliesIniFile(va("%s/supplies.ini", dlsys.inidir->string), full);
	ReadBuildingsIniFile(va("%s/buildings.ini", dlsys.inidir->string), full);

	LoadMOTD(va("%s/motd.txt", dlsys.inidir->string));

	ValidateIniEntries();
}

char *ReadEntFile(const char *mapname) {
char	filename[64], *entset;
FILE	*FHnd;
int		setsize;

	sprintf(filename, "%s/maps/%s.ent", dlsys.gamedir->string, mapname);
	if ((FHnd = fopen(filename, "rb")) == NULL)
		return NULL;

	// Get file size of entity set
	fseek(FHnd, 0, SEEK_END);
	setsize = ftell(FHnd);
	fseek(FHnd, 0, SEEK_SET);

	entset = gi.TagMalloc(setsize, TAG_LEVEL);
	if (fread(entset, 1, setsize, FHnd) != setsize) {
		gi.TagFree(entset);
		fclose(FHnd);
		return NULL;
	}

	gi.dprintf("NOTICE: Using alternate entity set: %s\n", filename);

	fclose(FHnd);
	return entset;
}

void WriteEntFile(const char *mapname, const char *entities) {
char	filename[64];
FILE	*FHnd;
int		setsize;

	setsize = strlen(entities);
	sprintf(filename, "%s/maps/%s.ent", dlsys.gamedir->string, mapname);
	if ((FHnd = fopen(filename, "wt")) == NULL) {
		gi.dprintf("WARNING: Error creating entity file.\n");
		return;
	}

	if (fwrite(entities, 1, setsize, FHnd) != setsize)
		gi.dprintf("WARNING: Error writing entity set to disk.\n");

	fclose(FHnd);
}

// Sticky functions
// Keeps an entity relative to another entity, preserving
// angle offsets and such
void VectorRotate(vec3_t in, vec3_t angles, vec3_t out) {
	float cv, sv, angle, tv;

	VectorCopy(in, out);
	angle = (-angles[PITCH]) * M_PI / 180;
	cv = cos(angle);
	sv = sin(angle);
	tv = (out[0] * cv) - (out[2] * sv);
	out[2] = (out[2] * cv) + (out[0] * sv);
	out[0] = tv;

	angle = (angles[YAW]) * M_PI / 180;
	cv = cos(angle);
	sv = sin(angle);
	tv = (out[0] * cv) - (out[1] * sv);
	out[1] = (out[1] * cv) + (out[0] * sv);
	out[0] = tv;

	angle = (angles[ROLL]) * M_PI / 180;
	cv = cos(angle);
	sv = sin(angle);
	tv = (out[1] * cv) - (out[2] * sv);
	out[2] = (out[2] * cv) + (out[1] * sv);
	out[1] = tv;
}

void VectorUnrotate(vec3_t in, vec3_t angles, vec3_t out) {
	float cv, sv, angle, tv;

	VectorCopy(in, out);
	angle = (-angles[ROLL]) * M_PI / 180;
	cv = cos(angle);
	sv = sin(angle);
	tv = (out[1] * cv) - (out[2] * sv);
	out[2] = (out[2] * cv) + (out[1] * sv);
	out[1] = tv;

	angle = (-angles[YAW]) * M_PI / 180;
	cv = cos(angle);
	sv = sin(angle);
	tv = (out[0] * cv) - (out[1] * sv);
	out[1] = (out[1] * cv) + (out[0] * sv);
	out[0] = tv;

	angle = (angles[PITCH]) * M_PI / 180;
	cv = cos(angle);
	sv = sin(angle);
	tv = (out[0] * cv) - (out[2] * sv);
	out[2] = (out[2] * cv) + (out[0] * sv);
	out[0] = tv;
}

void BBoxRotate(vec3_t in, vec3_t angles, vec3_t minOut, vec3_t maxOut) {
vec3_t	vi;
int i;

	for (i=0; i<3; i++) {
		VectorClear(vi);
		vi[i] = in[i];
		VectorRotate(vi, angles, vi);
		AddPointToBounds(vi, minOut, maxOut);
	}
}

void stuck_prethink (edict_t *self)
{
	vec3_t temp, vec;
	edict_t *other;

	other = self->rotate_target;
	if (!other->inuse) {
		self->rotate_target = NULL;
		self->prethink = NULL;
		return;
	}

	VectorRotate(self->orot1, other->s.angles, temp);
	VectorRotate(self->orot2, other->s.angles, vec);
	VectorAdd(other->s.origin, temp, self->s.origin);
	VectorSubtract(vec, temp, vec);
	// special hack for buildings
	if (self->building && self->rotate_target->building)
		return;
	vectoangles(vec, self->s.angles);

	// gi.linkentity(self);
}

void Calc_StuckOffset(edict_t *self, edict_t *other)
{
	vec3_t forward;
	vec3_t mins, maxs;
	int i;

	VectorSubtract(self->s.origin, other->s.origin, forward);
	VectorUnrotate(forward, other->s.angles, self->orot1);
	AngleVectors(self->s.angles, forward, NULL, NULL);
	VectorMA(self->s.origin, 64, forward, forward);
	VectorSubtract(forward, other->s.origin, forward);
	VectorUnrotate(forward, other->s.angles, self->orot2);
	VectorUnrotate(self->mins, other->s.angles, mins);
	VectorUnrotate(self->maxs, other->s.angles, maxs);
	// Rotating can make maxs lower than mins
	for (i = 0; i < 3; i++)
	{
		self->rotate_mins[i] = min(mins[i], maxs[i]);
		self->rotate_maxs[i] = max(mins[i], maxs[i]);
	}
}

void stick(edict_t *self, edict_t *other)
{
	VectorClear(self->velocity);
	VectorClear(self->avelocity);
	
	if (other != g_edicts)
	{
		Calc_StuckOffset(self, other);
		self->rotate_target = other;
		self->prethink = stuck_prethink;
	}
	else
		self->prethink = NULL;
}

qboolean LineOfSight (vec3_t start, vec3_t end, int mask)
{
	trace_t tr;

	tr = gi.trace (start, NULL, NULL, end, NULL, mask);
	if (tr.fraction == 1.0)
		return true;
	return false;
}

void ProjectClient (vec3_t dest, edict_t *ent, qboolean offhand)
{
	vec3_t offset, forward, right;
	
	if (!ent->client)
	{
		VectorCopy (ent->s.origin, dest);
		return;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	if ((ent->client->pers.hand == LEFT_HANDED && !offhand) ||
		(ent->client->pers.hand == RIGHT_HANDED && offhand))
		offset[1] *= -1;
	else if (ent->client->pers.hand == CENTER_HANDED)
		offset[1] = 0;
	G_ProjectSource (ent->s.origin, offset, forward, right, dest);
}

float AngleDiff (float angle1, float angle2)
{
	float diff;

	angle1 = anglemod(angle1);
	angle2 = anglemod(angle2);

	diff = fabs(angle1 - angle2);
	if (diff <= 180)
		return diff;
	return fabs(diff - 360);
}

void AnglesDiff(vec3_t angle1, vec3_t angle2, vec3_t angleOut) {
int i;

	for (i=0; i<3; i++)
		angleOut[i] = AngleDiff(angle1[i], angle2[i]);
}

void AnglesMod(vec3_t in, vec3_t out)
{
	out[0] = anglemod(in[0]);
	out[1] = anglemod(in[1]);
	out[2] = anglemod(in[2]);
}

#define FIELD_CONSTANT	45000.0	// detemined by max-effective range with an fov of 90 (500units * 90)

// InFOV returns non-zero if the targ is within the origin's FOV.
int InFOV(vec3_t origin, vec3_t dir, vec3_t targ, float fov, qboolean dof) {
vec3_t	diff, dirToTarg;
int i, inView = 0;
float distToTarg, minFocus, maxFocus;

	VectorSubtract(targ, origin, diff);
	vectoangles(diff, dirToTarg);
	AnglesDiff(dir, dirToTarg, diff);

	for (i=0; i<3; i++) {
		if (diff[i] <= (fov / 2))
			inView++;
	}

	if (inView < 3)
		return 0;

	VectorSubtract(targ, origin, diff);
	distToTarg = VectorLength(diff);

	if (dof) {
		minFocus = ((90/fov) * 50) / 2.0;
		maxFocus = ((4500/fov) * 50) / 4.0;

		if (distToTarg < minFocus)
			return 0;
		if (distToTarg > maxFocus)
			return 0;
	}

	return (int)distToTarg;
}

// FindInFOV searches the surrounding area for the centermost entity in the FoV
edict_t *FindInFOV(vec3_t origin, vec3_t dir, float fov, int maxdist) {
vec3_t	diff, dirToTarg;
int i, inView;
float nearest=999, offcenter;
edict_t	*g, *targ;

	g = targ = NULL;
	while (g = findradius(g, origin, maxdist)) {
		if (!g->inuse)
			continue;

		VectorSubtract(g->s.origin, origin, diff);
		vectoangles(diff, dirToTarg);
		AnglesDiff(dir, dirToTarg, diff);

		for (i=inView=0; i<3; i++) {
			if (diff[i] <= (fov / 2))
				inView++;
		}
		if (inView < 3)
			continue;

		offcenter = VectorLength(diff);
		if (offcenter < nearest) {
			nearest = offcenter;
			targ = g;
		}
	}

	return targ;
}

void DL_ChangeAngles (vec3_t start, vec3_t ideal_angles, vec3_t angle_speed, vec3_t dest) {
float	ideal;
float	current;
float	move;
float	speed;
int		i;

	for (i = 0; i < 3; i++)
	{
		current = anglemod(start[i]);
		ideal = anglemod(ideal_angles[i]);

		if (current == ideal)
			continue;
		
		move = ideal - current;
//		gi.dprintf("Diff %d: %f\n", i, move);
		speed = angle_speed[i];
		if (ideal > current)
		{
			if (move >= 180)
				move = move - 360;
		}
		else
		{
			if (move <= -180)
				move = move + 360;
		}
		if (move > 0)
		{
			if (move > speed)
				move = speed;
		}
		else
		{
			if (move < -speed)
				move = -speed;
		}
	
		dest[i] = anglemod (current + move);
	}
}

int sub_count(const char *in) {
int i = 0;

	if (!in || !in[0])
		return 0;

	for (;(in=strstr(in,"|")) != NULL; in++, i++);
	return i+1;
}

char *sub_string(const char *in, char *out, int n) {
char *dup, *str, *t;
int i = 0;

	if (!in || !in[0])
		return NULL;

	t = str = dup = DL_strcpy(NULL, in, -1);
	while ((t=strstr(t,"|")) != NULL) {
		*t++ = 0;
		if (--n == 0)
			str = t;
	}
	DL_strcpy(out, str, -1);
	gi.TagFree(dup);
	return out;
}

char *strnsub(const char *pszString,
			const char *pszPattern,
			const char *pszReplacement,
			int iMaxLength)
{
	char  *pszSubstring, *pszTmpSubstring ;
	int   iPatternLength, iReplacementLength ;
	
	/*-------------------------------------------------*/
	
	pszTmpSubstring = pszSubstring = pszString ;
	iPatternLength = strlen(pszPattern) ;
	iReplacementLength = strlen(pszReplacement) ;
	
	if ( NULL == ( pszSubstring = strstr(pszString, pszPattern ) ) )
	{                                   /* No match found               */
		return( NULL );
	}

	if ( 0 == DL_strcmp( pszPattern, pszReplacement,-1, true) )
		return( 0 ) ;                 /* Pattern == replacement: loop */
	
	if ( ( strlen( pszString ) + ( iReplacementLength - iPatternLength ) )
		>= iMaxLength )
		
		/* Enough space for replacement? */
		
		return( NULL ) ;
	
	pszTmpSubstring = (char *) calloc(iMaxLength, sizeof(char)) ;
	
	/* Buy some workspace      */
	
	if ( NULL == pszTmpSubstring )            /* No memory left          */
		return( NULL ) ;
	
	DL_strcpy( pszTmpSubstring, pszSubstring + iPatternLength, -1 ) ;
	
	/* If there was space left */
	
	while ( iReplacementLength-- )
	{     /* Copy replacement        */
		*pszSubstring++ = *pszReplacement++ ;
	}
	
	DL_strcpy( pszSubstring, pszTmpSubstring, -1 ) ;
	
	/* Add old stuff back      */
	
	free( pszTmpSubstring ) ;
	
	return( pszSubstring - iPatternLength ) ;
	
	/* Return pointer to change*/
}

int GetHitLocation (vec3_t point, edict_t *ent)
{
	vec3_t hLoc;

	VectorSubtract(point, ent->s.origin, hLoc);
	if (ent->maxs[2] <= 4)
	{
		// target is crouching
		if (hLoc[2] > 0) {
			return LOC_HEAD;
		}
		else if (hLoc[2] < -8) {
			return LOC_LEGS;
		}
	}
	else
	{
		// target is standing
		if (hLoc[2] > 20) {
			return LOC_HEAD;
		}
		else if (hLoc[2] < 8) {
			return LOC_LEGS;
		}
	}
	return LOC_CHEST;
}

char *MeansOfDeath (const char *in, const char *victim, const char *killer, const char *device, const char *weapname, const char *ammoname, const char *basename, const char *locname, qboolean female) {
static char	string[256];

	// pick a random substring
	sub_string(in, string, random() * sub_count(in));

	// bad way to do this...
	if (victim) strnsub(string, "%v", victim,	254);
	if (killer) strnsub(string, "%k", killer,	254);
	if (device) strnsub(string, "%d", device,	254);
	if (basename) strnsub(string, "%b", basename,	254);
	if (ammoname) strnsub(string, "%a", ammoname,	254);
	if (weapname) strnsub(string, "%w", weapname,	254);
	if (female) {
		strnsub(string, "%g", "her",	254);
		strnsub(string, "%p", "her",	254);
	} else {
		strnsub(string, "%g", "him",	254);
		strnsub(string, "%p", "his",	254);
	}
	if (locname) strnsub(string, "%l", locname,	254);

	return string;
}

char *GetDeathMessage(int mod, const char *section, const char *entry) {
char modSection[128];

	switch (mod) {
	case MOD_KILLOTHER:			sprintf(modSection, "Frag:%s", section); break;
	case MOD_KILLTEAM:			sprintf(modSection, "Scag:%s", section); break;
	case MOD_KILLSELF:			sprintf(modSection, "Suicide:%s", section); break;
	case MOD_BLDG_KILLOTHER:	sprintf(modSection, "BldgFrag:%s", section); break;
	case MOD_BLDG_KILLOWNER:	sprintf(modSection, "BldgOwner:%s", section); break;
	case MOD_BLDG_KILLTEAM:		sprintf(modSection, "BldgScag:%s", section); break;
	case MOD_BLDG_KILLSELF:		sprintf(modSection, "BldgSuicide:%s", section); break;
	case MOD_WORLD:
	default:					sprintf(modSection, "World"); break;
	}

	return Ini_ReadString(dlsys.sysIni, modSection, entry, NULL);
}

void SearchDeadBodies(edict_t *self) {
edict_t *g, *body = NULL;
float	fDist, closest = 64;
vec3_t	vDist;
item_t	*ours, *found;
char	*name;
int		moved, max, t;

	if (!self->client)
		return;

	for (g = &g_edicts[(int)maxclients->value + 1]; g <= &g_edicts[(int)maxclients->value + BODY_QUEUE_SIZE + 1]; g++) {
		if (!g->takedamage)
			continue;
		VectorSubtract(self->s.origin, g->s.origin, vDist);
		fDist = VectorLength(vDist);
		if (fDist < closest)
			closest = fDist, body = g;
	}

	// no bodies found
	if (!body)
		return;

	// Search for anything on this body which is part of your current profile.
	for (found = body->inventory; found; found = found->next) {
		name = GetItemName(found);
		if (FindSection(self->client->prefs.IHnd, name) >= 0) {
			if (ours = FindItemByName(self, name, found->itemtype)) {
				max = Ini_ReadInteger(self->client->prefs.IHnd, name, "quantity", 1);
				t = Ini_ReadInteger(self->client->prefs.IHnd, name, "maximum", max);
				if (t > max) max = t;
				moved =	max - ours->quantity;
				if (moved > 0) {
					if ((found->itemtype == ITEM_AMMO) && (moved > 10))
						moved = 10;
						// move ammo in 10 round sets
					else if ((found->itemtype == ITEM_SUPPLY) &&
					   ((ITEMSUPPLY(found)->type == SUP_ARMOUR) ||
						(ITEMSUPPLY(found)->type == SUP_FLASHLIGHT) ||
						(ITEMSUPPLY(found)->type == SUP_TOOL)) );
						// move the all at once
					else
						moved = 1;

					MoveToInventory(body, found, moved, self);
					return;
				}
			}
		}
	}
}

void stuffcmd(const edict_t *ent, const char *s) 	
{
   	gi.WriteByte (11);	        
	gi.WriteString (s);
    gi.unicast (ent, true);	
}


char *PicNParseMsg (const char *in, const char *a_player, const char *a_team, const char *o_team) {
static char	string[256], ap_upper[32], at_upper[32], ot_upper[32];
int i;

	// pick a random substring
	sub_string(in, string, random() * sub_count(in));

	// bad way to do this...
	if (!a_player) a_player = "";
	if (!a_team) a_team = "";
	if (!o_team) o_team = "";

	// Player name
	memset(ap_upper, 0, 32);
	for (i=0; a_player[i] && (i < 32); i++)
		ap_upper[i] = toupper(a_player[i]);

	strnsub(string, "%p", a_player,	254);
	strnsub(string, "%P", ap_upper,	254);

	// Team Name
	memset(at_upper, 0, 32);
	for (i=0; a_team[i] && (i < 32); i++)
		at_upper[i] = toupper(a_team[i]);

	strnsub(string, "%t", a_team,	254);
	strnsub(string, "%T", at_upper,	254);

	// Owning Team
	memset(ot_upper, 0, 32);
	for (i=0; o_team[i] && (i < 32); i++)
		ot_upper[i] = toupper(o_team[i]);

	strnsub(string, "%k", o_team,	254);
	strnsub(string, "%K", ot_upper,	254);

	return string;
}

void RecountTeamPlayers(void) {
int i;
edict_t *g;

	for (i = 0; i < MAX_TEAMS; i++)
		dlsys.teams[i].players = 0;

	// Recount team players
	for (g = &g_edicts[1]; g <= &g_edicts[(int)maxclients->value]; g++) {
		if (!g->client)
			continue;
		if (!g->client->pers.connected)
			continue;
		if (!g->client->resp.team)
			continue;

		g->client->resp.team->players++;
	}
}

team_t *AssignToTeam(void) {
int i, fewest = 999;
team_t *t = NULL;

	if (dlsys.auto_team == ASSIGN_LEAST_PLAYERS) {
		for (i = 0; i < dlsys.num_teams; i++) {
			if (dlsys.teams[i].players < fewest) {
				fewest = dlsys.teams[i].players;
				t = &dlsys.teams[i];
			}
		}
	} else if (dlsys.auto_team == ASSIGN_LEAST_SCORE) {
		for (i = 0; i < dlsys.num_teams; i++) {
			if (dlsys.teams[i].score < fewest) {
				fewest = dlsys.teams[i].score;
				t = &dlsys.teams[i];
			}
		}
	} else if (dlsys.auto_team == ASSIGN_RANDOM) {
		i = random() * dlsys.num_teams;
		t = &dlsys.teams[i];
	}

	return t;
}
			
qboolean DL_StartClient(edict_t *ent)
{
	if (dlsys.playmode & PM_ELIMINATION) {
		if (ent->client->resp.team && ent->client->resp.profile[0] && ent->client->resp.lives--)
			return false;
	} else if (ent->client->resp.team && ent->client->resp.profile[0])
		return false;

	// start as 'observer'
	ent->movetype = MOVETYPE_NOCLIP;
	ent->solid = SOLID_NOT;
	ent->svflags |= SVF_NOCLIENT;
	ent->client->ps.gunindex = 0;
	gi.linkentity (ent);

	if (dlsys.playmode & PM_ELIMINATION)
		if (!ent->client->resp.lives)
			return true;

	if (dlsys.force_motd)
		PMenu_Open(ent, ID_MOTD, NULL, NULL);
	else
		PMenu_Open(ent, ID_MAIN, NULL, NULL);
	return true;
}

char *DL_AssignSkin(edict_t *ent, char *s)
{
	int playernum = ent-g_edicts-1;
	char *p;
	char t[64];
	static char skin[MAX_QPATH];

	Com_sprintf(t, sizeof(t), "%s", s);

	p = strrchr(t, '/');
	if (p && (p != t)) p[0] = 0;
	else DL_strcpy(t, "male", -1);

	if (dlsys.playmode & PM_FORCESKINS)	// force the player to use the default male skin
		DL_strcpy(t, "male", -1);

	if (ent->team) DL_strcpy(skin, va("%s\\%s/%s", ent->client->pers.netname, t, ent->team->skin), -1);
	else DL_strcpy(skin, va("%s\\%s", ent->client->pers.netname, s), -1);

	gi.configstring (CS_PLAYERSKINS+playernum, skin);
	return skin;
}


int DL_strcmp(const char *s1, const char *s2, int len, qboolean casesen) {
	if ((s1 && !s2) || (!s1 && s2)) return -1;
	if (!s1 && !s2) return 0;
	if (s1 == s2) return 0;

	while(len && *s1 && *s2 && (casesen ? *s1 == *s2 : toupper(*s1) == toupper(*s2)))
		s1++, s2++, len--;

	if (!len || (casesen ? *s1 == *s2 : toupper(*s1) == toupper(*s2))) return 0;
	else return -1;
}

char *DL_strcpy(char *dest, const char *src, int max) {
char *tmp;
int len;

	if (!src)
		return NULL;

	if (!dest) {	// allocate a block of memory for the string
		len = strlen(src)+1;
		if ((max > 0) && (max < len))
			len = max + 1;
		dest = gi.TagMalloc(len, TAG_LEVEL);
	}		

	tmp = dest;
	while (*src && max) { 
		*tmp = *src;
		tmp++, src++, max--;
	}

	*tmp = 0;

	return dest;
}

qboolean DL_FileExist(const char *filename) {
PFILE *FHnd;
	if ((FHnd = pfopen(filename, "rb")) != NULL) {
		pfclose(FHnd);
		return true;
	}
	return false;
}

int LoadMOTD(const char *filename) {
PFILE *FHnd;
char *t, *buff;
int size, len, stop = false;

	if ((FHnd = pfopen(filename, "rt")) != NULL) {
		pfseek(FHnd, 0L, SEEK_END);
		size = pftell(FHnd);
		pfseek(FHnd, 0L, SEEK_SET);

		numMOTDLines = 8;
		buff = (char *)gi.TagMalloc(size+1, TAG_LEVEL);
		t = buff;
		while (pfgets(t, buff+size-t, FHnd))
		{
			len = strlen(t);
			if (t[len-1] == '\n')		// was a whole line read in?
			{
				if (len > 32)
					t[32] = '\0';		// truncate long lines to 32 chars
				else
					t[len-1] = '\0';	// remove newline

				motdLines[numMOTDLines++] = t;
				t += strlen(t)+1;		// move t past terminating \0 for next line
			}
		}
		pfclose(FHnd);

		return true;
	}
	return false;
}

// Tagged Memory Functions
// These emulate Quake2's functions, but allow the debugger to see what's going on
typedef struct tagmem_s
{
	int		tag;
	void	*data;
	int		size;	// for debugging
	struct	tagmem_s *next;
} tagmem_t;

static tagmem_t *tag_head = NULL;

int	tag_game_num = 0;
int tag_game_size = 0;
int	tag_level_num = 0;
int tag_level_size = 0;

void *DL_TagMalloc(int size, int tag)
{
	tagmem_t	*t;

	if (tag == TAG_GAME)
	{
		tag_game_num++;
		tag_game_size += size;
		//gi.dprintf("DL_TagMalloc: TAG_GAME  num: %5d   size: %5d\n", tag_game_num, tag_game_size);
	}
	else if (tag == TAG_LEVEL)
	{
		tag_level_num++;
		tag_level_size += size;
		//gi.dprintf("DL_TagMalloc: TAG_LEVEL num: %5d   size: %5d\n", tag_level_num, tag_level_size);
	}
	else
		assert(0);

	t = malloc(sizeof(tagmem_t));
	t->tag = tag;
	t->data = malloc(size);
	t->size = size;
	t->next = tag_head;
	tag_head = t;
	
	// clear data structure
	// memset(t->data, 0, t->size);

	return t->data;
}

void DL_TagFree(void *block)
{
	tagmem_t	*t, *prev;

	for (t = tag_head, prev = NULL; t; prev = t, t = t->next)
	{
		if (t->data == block)
		{
			if (prev)
				prev->next = t->next;
			else
				tag_head = t->next;

			if (t->tag == TAG_GAME)
			{
				tag_game_num--;
				tag_game_size -= t->size;
				//gi.dprintf("DL_TagFree:   TAG_GAME  num: %5d   size: %5d\n", tag_game_num, tag_game_size);
			}
			else if (t->tag == TAG_LEVEL)
			{
				tag_level_num--;
				tag_level_size -= t->size;
				//gi.dprintf("DL_TagFree:   TAG_LEVEL num: %5d   size: %5d\n", tag_level_num, tag_level_size);
			}
			else
				assert(0);

			free(t->data);
			free(t);
			return;
		}
	}
	assert(0);
}

void DL_FreeTags(int tag)
{
	tagmem_t	*t, *prev, *next;

	for (t = tag_head, prev = NULL; t; t = next)
	{
		next = t->next;
		if (t->tag == tag)
		{
			if (prev)
				prev->next = t->next;
			else
				tag_head = t->next;

			if (tag == TAG_GAME)
			{
				tag_game_num--;
				tag_game_size -= t->size;
			}
			else if (tag == TAG_LEVEL)
			{
				tag_level_num--;
				tag_level_size -= t->size;
			}
			else
				assert(0);

			free(t->data);
			free(t);
		}
		else
			prev = t;
	}
	
/*	if (tag == TAG_GAME)
		gi.dprintf("DL_FreeTags:  TAG_GAME  num: %5d   size: %5d\n", tag_game_num, tag_game_size);
	else if (tag == TAG_LEVEL)
		gi.dprintf("DL_FreeTags:  TAG_LEVEL num: %5d   size: %5d\n", tag_level_num, tag_level_size);
	else
		assert(0);
*/
}

void DL_DebugLog(char *string) {
FILE *FHnd;

	if ((FHnd = fopen("dl2debug.log", "at")) == NULL)
		return;

	fputs(string, FHnd);
	fclose(FHnd);
}