#include "g_local.h"

void CheckBanMenu (edict_t *ent, cvar_t *ban_cvar, int menunumber)
{
	if (ban_cvar->value)
	{
		ent->client->current_menu[menunumber].text = "";
		ent->client->current_menu[menunumber].SelectFunc = NULL;
	}
}

void CheckLevelForMonster (edict_t *ent, char *menuname, int playerlevel, int classnumber)
{
	if (!(int)(stroggflags->value) & SF_RPG_MODE)
		return;

	if (Q_stricmp (menuname, "groundrose") == 0)
	{
		if (playerlevel < 18) // Death Tank
		{
			ent->client->current_menu[4].text = "";
			ent->client->current_menu[4].SelectFunc = NULL;
		}
		if (playerlevel < 13) // Laser Gladiator
		{
			ent->client->current_menu[5].text = "";
			ent->client->current_menu[5].SelectFunc = NULL;
		}
		if (playerlevel < 17) // Stormtrooper Soldier
		{
			ent->client->current_menu[6].text = "";
			ent->client->current_menu[6].SelectFunc = NULL;
		}
		if (playerlevel < 10) // Male Traitor
		{
			ent->client->current_menu[7].text = "";
			ent->client->current_menu[7].SelectFunc = NULL;
		}
		if (playerlevel < 10) // Female Traitor
		{
			ent->client->current_menu[8].text = "";
			ent->client->current_menu[8].SelectFunc = NULL;
		}
		if (playerlevel < 30) // Hypertank
		{
			ent->client->current_menu[9].text = "";
			ent->client->current_menu[9].SelectFunc = NULL;
		}
	}
	if (Q_stricmp (menuname, "fly_rose") == 0)
	{
		if (playerlevel < 13) // Flyer Hornet
		{
			ent->client->current_menu[3].text = "";
			ent->client->current_menu[3].SelectFunc = NULL;
		}
	}
	if (Q_stricmp (menuname, "swimmenu") == 0)
	{
		if (playerlevel < 5) // Shark
		{
			ent->client->current_menu[3].text = "";
			ent->client->current_menu[3].SelectFunc = NULL;
		}
	}
	if (Q_stricmp (menuname, "ground2") == 0)
	{
		if (playerlevel < 8) // Iron Maiden
		{
			ent->client->current_menu[3].text = "";
			ent->client->current_menu[3].SelectFunc = NULL;
		}
		if (playerlevel < 7) // Brain
		{
			ent->client->current_menu[4].text = "";
			ent->client->current_menu[4].SelectFunc = NULL;
		}
		if (playerlevel < 7) // Parasite
		{
			ent->client->current_menu[9].text = "";
			ent->client->current_menu[9].SelectFunc = NULL;
		}
		if (playerlevel < 9) // Mutant
		{
			ent->client->current_menu[5].text = "";
			ent->client->current_menu[5].SelectFunc = NULL;
		}
		if (playerlevel < 12) // Tank Commander
		{
			ent->client->current_menu[6].text = "";
			ent->client->current_menu[6].SelectFunc = NULL;
		}
		if (playerlevel < 20) // JORG
		{
			ent->client->current_menu[7].text = "";
			ent->client->current_menu[7].SelectFunc = NULL;
		}
		if (playerlevel < 22) // Makron
		{
			ent->client->current_menu[8].text = "";
			ent->client->current_menu[8].SelectFunc = NULL;
		}
	}
	if (Q_stricmp (menuname, "groundtr") == 0)
	{
		if (playerlevel < 12) // Gekk
		{
			ent->client->current_menu[3].text = "";
			ent->client->current_menu[3].SelectFunc = NULL;
		}
		if (playerlevel < 8) // BETA Soldier
		{
			ent->client->current_menu[4].text = "";
			ent->client->current_menu[4].SelectFunc = NULL;
		}
		if (playerlevel < 12) // BETA Gladiator
		{
			ent->client->current_menu[5].text = "";
			ent->client->current_menu[5].SelectFunc = NULL;
		}
		if (playerlevel < 11) // BETA Brain
		{
			ent->client->current_menu[6].text = "";
			ent->client->current_menu[6].SelectFunc = NULL;
		}
		if (playerlevel < 13) // BETA Iron Maiden
		{
			ent->client->current_menu[7].text = "";
			ent->client->current_menu[7].SelectFunc = NULL;
		}
		if (playerlevel < 16) // BETA Supertank
		{
			ent->client->current_menu[8].text = "";
			ent->client->current_menu[8].SelectFunc = NULL;
		}
	}
	if (Q_stricmp (menuname, "groundgz") == 0)
	{
		if (playerlevel < 9) // Stalker
		{
			ent->client->current_menu[3].text = "";
			ent->client->current_menu[3].SelectFunc = NULL;
		}
		if (playerlevel < 14) // Medic Commander
		{
			ent->client->current_menu[4].text = "";
			ent->client->current_menu[4].SelectFunc = NULL;
		}
		if (playerlevel < 26) // Black Widow (Walk)
		{
			ent->client->current_menu[5].text = "";
			ent->client->current_menu[5].SelectFunc = NULL;
		}
		if (playerlevel < 28) // Black Widow (Spider)
		{
			ent->client->current_menu[6].text = "";
			ent->client->current_menu[6].SelectFunc = NULL;
		}
	}
	if (Q_stricmp (menuname, "groundmenu") == 0)
	{
		if (playerlevel < 0) // Soldier
		{
			ent->client->current_menu[3].text = "";
			ent->client->current_menu[3].SelectFunc = NULL;
		}
		if (playerlevel < 2) // Enforcer
		{
			ent->client->current_menu[4].text = "";
			ent->client->current_menu[4].SelectFunc = NULL;
		}
		if (playerlevel < 6) // Gunner
		{
			ent->client->current_menu[5].text = "";
			ent->client->current_menu[5].SelectFunc = NULL;
		}
		if (playerlevel < 5) // Berserk
		{
			ent->client->current_menu[6].text = "";
			ent->client->current_menu[6].SelectFunc = NULL;
		}
		if (playerlevel < 8) // Medic
		{
			ent->client->current_menu[7].text = "";
			ent->client->current_menu[7].SelectFunc = NULL;
		}
		if (playerlevel < 9) // Gladiator
		{
			ent->client->current_menu[8].text = "";
			ent->client->current_menu[8].SelectFunc = NULL;
		}
		if (playerlevel < 11) // Tank
		{
			ent->client->current_menu[9].text = "";
			ent->client->current_menu[9].SelectFunc = NULL;
		}
		if (playerlevel < 14) // Supertank
		{
			ent->client->current_menu[10].text = "";
			ent->client->current_menu[10].SelectFunc = NULL;
		}
	}
	if (Q_stricmp (menuname, "flymenu") == 0)
	{
		if (playerlevel < 0) // Flyer
		{
			ent->client->current_menu[3].text = "";
			ent->client->current_menu[3].SelectFunc = NULL;
		}
		if (playerlevel < 8) // Icarus
		{
			ent->client->current_menu[4].text = "";
			ent->client->current_menu[4].SelectFunc = NULL;
		}
		if (playerlevel < 6) // Technician
		{
			ent->client->current_menu[5].text = "";
			ent->client->current_menu[5].SelectFunc = NULL;
		}
		if (playerlevel < 15) // Biggun
		{
			ent->client->current_menu[6].text = "";
			ent->client->current_menu[6].SelectFunc = NULL;
		}
	}
	if (Q_stricmp (menuname, "flytr") == 0)
	{
		if (playerlevel < 10) // Repair bot
		{
			ent->client->current_menu[4].text = "";
			ent->client->current_menu[4].SelectFunc = NULL;
		}
	}
	if (Q_stricmp (menuname, "flygz") == 0)
	{
		if (playerlevel < 17) // Carrier
		{
			ent->client->current_menu[4].text = "";
			ent->client->current_menu[4].SelectFunc = NULL;
		}
		if (playerlevel < 11) // Daedalus
		{
			ent->client->current_menu[5].text = "";
			ent->client->current_menu[5].SelectFunc = NULL;
		}
	}
}

// Paril, Menus

pmenu_t loginmenu[] = {
	{ "*Login Menu",		PMENU_ALIGN_CENTER, NULL },
	{ "StroggDM",		PMENU_ALIGN_CENTER, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ "Login",	PMENU_ALIGN_LEFT, PlayerData_Login }, 
	{ "Create new Character",				PMENU_ALIGN_LEFT, PlayerData_Create },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },   
	{ NULL,				PMENU_ALIGN_LEFT, NULL }, 
	{ "Read the readme",PMENU_ALIGN_LEFT, NULL }, 
	{ "for any help.",	PMENU_ALIGN_LEFT, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ "Close",			PMENU_ALIGN_LEFT, CloseThisMenu }
};

pmenu_t mainmenu[] = {
	{ "*Main Menu",		PMENU_ALIGN_CENTER, NULL },
	{ "StroggDM",		PMENU_ALIGN_CENTER, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ "Join the game",	PMENU_ALIGN_LEFT, OpenClassMenu }, 
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },   
	{ NULL,				PMENU_ALIGN_LEFT, NULL }, 
	{ "Read the readme",PMENU_ALIGN_LEFT, NULL }, 
	{ "for any help.",	PMENU_ALIGN_LEFT, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ "Close",			PMENU_ALIGN_LEFT, CloseThisMenu }
};

pmenu_t classtypemenu[] = {
	{ "*ClassType Menu",		PMENU_ALIGN_CENTER, NULL },
	{ "StroggDM",		PMENU_ALIGN_CENTER, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ "Select a Classtype",				PMENU_ALIGN_LEFT, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL }, 
	{ "Ground",				PMENU_ALIGN_LEFT, OpenGroundMenu },
	{ "Flying",				PMENU_ALIGN_LEFT, OpenFlyMenu },
	{ "Swimming",				PMENU_ALIGN_LEFT, OpenSwimMenu },   
	{ NULL,				PMENU_ALIGN_LEFT, NULL }, 
	{ "Read the readme",PMENU_ALIGN_LEFT, NULL }, 
	{ "for any help.",	PMENU_ALIGN_LEFT, NULL },
	{ "StroggDM V7.0",				PMENU_ALIGN_LEFT, NULL },
	{ "Close",			PMENU_ALIGN_LEFT, CloseThisMenu }
};


pmenu_t ground_pick[] = {
	{ "*ClassType Menu",		PMENU_ALIGN_CENTER, NULL },
	{ "StroggDM",		PMENU_ALIGN_CENTER, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL }, 
	{ "Original Quake2",				PMENU_ALIGN_LEFT, OpenGroundQ2MenuGround },
	{ "Ground Zero",				PMENU_ALIGN_LEFT, OpenGroundGZMenuGround },
	{ "The Reckoning",				PMENU_ALIGN_LEFT, OpenGroundTRMenuGround },   
	{ "Roseville Monsters",				PMENU_ALIGN_LEFT, OpenGroundRoseMenu }, 
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL }, 
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ "Back",			PMENU_ALIGN_LEFT, GoBack }
};



pmenu_t groundrose[] = {
	{ "*ClassType Menu",		PMENU_ALIGN_CENTER, NULL },
	{ "StroggDM",		PMENU_ALIGN_CENTER, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ "Death Tank",				PMENU_ALIGN_LEFT, BeDeathTank }, 
	{ "Laser Gladiator",				PMENU_ALIGN_LEFT, BeLaserGladiator },
	{ "Stormtrooper Soldier",				PMENU_ALIGN_LEFT, BeSoldierStorm },
	{ "Male Traitor",				PMENU_ALIGN_LEFT, BeMaleTraitor }, 
	{ "Female Traitor",				PMENU_ALIGN_LEFT, BeFemaleTraitor },
	{ "Hypertank",				PMENU_ALIGN_LEFT, BeHypertank },
	{ NULL,				PMENU_ALIGN_LEFT, NULL }, 
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ "Back",			PMENU_ALIGN_LEFT, GoBack }
};

pmenu_t fly_pick[] = {
	{ "*ClassType Menu",		PMENU_ALIGN_CENTER, NULL },
	{ "StroggDM",		PMENU_ALIGN_CENTER, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL }, 
	{ "Original Quake2",				PMENU_ALIGN_LEFT, OpenGroundQ2MenuFly },
	{ "Ground Zero",				PMENU_ALIGN_LEFT, OpenGroundGZMenuFly },
	{ "The Reckoning",				PMENU_ALIGN_LEFT, OpenGroundTRMenuFly },   
	{ "Roseville Monsters",				PMENU_ALIGN_LEFT, OpenFlyRoseMenu }, 
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL }, 
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ "Back",			PMENU_ALIGN_LEFT, GoBack }
};



pmenu_t fly_rose[] = {
	{ "*ClassType Menu",		PMENU_ALIGN_CENTER, NULL },
	{ "StroggDM",		PMENU_ALIGN_CENTER, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ "Flyer Hornet",				PMENU_ALIGN_LEFT, BeFlyerHornet }, 
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ "Back",			PMENU_ALIGN_LEFT, GoBack }
};



pmenu_t swimmenu[] = {
	{ "*SwimType Menu",		PMENU_ALIGN_CENTER, NULL },
	{ "StroggDM",		PMENU_ALIGN_CENTER, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ "Barracuda Shark",				PMENU_ALIGN_LEFT, BeShark },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ "Back",			PMENU_ALIGN_LEFT, GoBack }
};



pmenu_t ground2[] = {
	{ "*GroundType Menu",		PMENU_ALIGN_CENTER, NULL },
	{ "StroggDM",		PMENU_ALIGN_CENTER, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ "Iron Maiden",				PMENU_ALIGN_LEFT, BeChick },
	{ "Brain",				PMENU_ALIGN_LEFT, BeBrain},
	{ "Mutant",				PMENU_ALIGN_LEFT, BeMutant },
	{ "Tank Commander",				PMENU_ALIGN_LEFT, BeCTank },
	{ "JORG",				PMENU_ALIGN_LEFT, BeJorg },
	{ "Makron",				PMENU_ALIGN_LEFT, BeMakron },
	{ "Parasite",				PMENU_ALIGN_LEFT, BeParasite },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
//	{ "Next",				PMENU_ALIGN_LEFT, GoToGround3Menu },
	{ "Back",			PMENU_ALIGN_LEFT, OpenGroundMenu }
};



pmenu_t groundtr[] = {
	{ "*GroundType Menu",		PMENU_ALIGN_CENTER, NULL },
	{ "StroggDM",		PMENU_ALIGN_CENTER, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ "Gekk",				PMENU_ALIGN_LEFT, BeGekk },
	{ "BETA Soldier",				PMENU_ALIGN_LEFT, BeBETASoldier },
	{ "BETA Gladiator",				PMENU_ALIGN_LEFT, BeBETAGlad },
	{ "BETA Brain",				PMENU_ALIGN_LEFT, BeBETABrain },
	{ "BETA Iron Maiden",				PMENU_ALIGN_LEFT, BeBETAMaiden },
	{ "BETA SuperTank",				PMENU_ALIGN_LEFT, BeBETASupertank },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	//	{ "Next",				PMENU_ALIGN_LEFT, GoToGround4Menu },
	{ "Back",			PMENU_ALIGN_LEFT, OpenGroundMenu }
};




pmenu_t groundgz[] = {
	{ "*GroundType Menu",		PMENU_ALIGN_CENTER, NULL },
	{ "StroggDM",		PMENU_ALIGN_CENTER, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ "Stalker",				PMENU_ALIGN_LEFT, BeStalker },
	{ "Medic Commander",				PMENU_ALIGN_LEFT, BeMedicCommander },
	{ "Black Widow (Walk)",				PMENU_ALIGN_LEFT, BeBlackWidow },
	{ "Black Widow (Spider)",				PMENU_ALIGN_LEFT, BeBlackWidow2 },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ "Back",			PMENU_ALIGN_LEFT, OpenGroundMenu }
};

pmenu_t ground3[] = {
	{ "*GroundType Menu",		PMENU_ALIGN_CENTER, NULL },
	{ "StroggDM",		PMENU_ALIGN_CENTER, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ "Stalker",				PMENU_ALIGN_LEFT, BeStalker },
	{ "Medic Commander",				PMENU_ALIGN_LEFT, BeMedicCommander },
	{ "Black Widow (Walk)",				PMENU_ALIGN_LEFT, BeBlackWidow },
	{ "Black Widow (Spider)",				PMENU_ALIGN_LEFT, BeBlackWidow2 },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ "Back",			PMENU_ALIGN_LEFT, OpenGroundMenu }
};

pmenu_t ground4[] = {
	{ "*GroundType Menu",		PMENU_ALIGN_CENTER, NULL },
	{ "StroggDM",		PMENU_ALIGN_CENTER, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ "Stalker",				PMENU_ALIGN_LEFT, BeStalker },
	{ "Medic Commander",				PMENU_ALIGN_LEFT, BeMedicCommander },
	{ "Black Widow (Walk)",				PMENU_ALIGN_LEFT, BeBlackWidow },
	{ "Black Widow (Spider)",				PMENU_ALIGN_LEFT, BeBlackWidow2 },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ "Back",			PMENU_ALIGN_LEFT, OpenGroundMenu }
};



pmenu_t groundmenu[] = {
	{ "*GroundType Menu",		PMENU_ALIGN_CENTER, NULL },
	{ "StroggDM",		PMENU_ALIGN_CENTER, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ "Soldier",				PMENU_ALIGN_LEFT, BeSoldier },
	{ "Enforcer",				PMENU_ALIGN_LEFT, BeEnforcer },
	{ "Gunner",				PMENU_ALIGN_LEFT, BeGunner },
	{ "Berserk",				PMENU_ALIGN_LEFT, BeBerserk },
	{ "Medic",				PMENU_ALIGN_LEFT, BeMedic },
	{ "Gladiator",PMENU_ALIGN_LEFT, BeGladiator }, 
	{ "Tank",				PMENU_ALIGN_LEFT, BeTank },   
	{ "Supertank",	PMENU_ALIGN_LEFT, BeSupertank },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ "Next",			PMENU_ALIGN_LEFT, GoToGround2Menu }
};



pmenu_t flymenu[] = {
	{ "*FlyType Menu",		PMENU_ALIGN_CENTER, NULL },
	{ "StroggDM",		PMENU_ALIGN_CENTER, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ "Flyer",				PMENU_ALIGN_LEFT, BeFlyer },
	{ "Icarus",				PMENU_ALIGN_LEFT, BeIcarus },
	{ "Floater",				PMENU_ALIGN_LEFT, BeFloater },
	{ "Boss2",				PMENU_ALIGN_LEFT, BeBoss2 },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ "Back",				PMENU_ALIGN_LEFT, GoBack },
	{ "Close",			PMENU_ALIGN_LEFT, CloseThisMenu }
};



pmenu_t flytr[] = {
	{ "*FlyType Menu",	PMENU_ALIGN_CENTER, NULL },
	{ "StroggDM",		PMENU_ALIGN_CENTER, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ "Repair bot",		PMENU_ALIGN_LEFT, BeFixbot },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ "Back",			PMENU_ALIGN_LEFT, GoBack },
	{ "Close",			PMENU_ALIGN_LEFT, CloseThisMenu }
};



pmenu_t flygz[] = {
	{ "*FlyType Menu",	PMENU_ALIGN_CENTER, NULL },
	{ "StroggDM",		PMENU_ALIGN_CENTER, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ "Carrier",		PMENU_ALIGN_LEFT, BeCarrier },
	{ "Daedalus",		PMENU_ALIGN_LEFT, BeDaedalus },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ "Back",			PMENU_ALIGN_LEFT, GoBack },
	{ "Close",			PMENU_ALIGN_LEFT, CloseThisMenu }
};

// "Be"s
void BeSoldier (edict_t *ent, pmenuhnd_t *p)
{
	if (ban_soldier->value)
	{
		safe_cprintf (ent, PRINT_HIGH, "Sorry, that monster is banned.\n");
		return;
	}
    ent->client->resp.class = 1;
    EndObserverMode_Walking(ent);
}
void BeEnforcer (edict_t *ent, pmenuhnd_t *p)
{
	if (ban_enforcer->value)
	{
		safe_cprintf (ent, PRINT_HIGH, "Sorry, that monster is banned.\n");
		return;
	}
    ent->client->resp.class = 2;
    EndObserverMode_Walking(ent);
}
void BeTank (edict_t *ent, pmenuhnd_t *p)
{
	if (ban_tank->value)
	{
		safe_cprintf (ent, PRINT_HIGH, "Sorry, that monster is banned.\n");
		return;
	}
    ent->client->resp.class = 3;
    EndObserverMode_Walking(ent);
}
void BeMedic (edict_t *ent, pmenuhnd_t *p)
{
	if (ban_medic->value)
	{
		safe_cprintf (ent, PRINT_HIGH, "Sorry, that monster is banned.\n");
		return;
	}
    ent->client->resp.class = 4;
    EndObserverMode_Walking(ent);
}
void BeMedicCommander (edict_t *ent, pmenuhnd_t *p)
{
	if (ban_mediccommander->value)
	{
		safe_cprintf (ent, PRINT_HIGH, "Sorry, that monster is banned.\n");
		return;
	}
    ent->client->resp.class = 28;
    EndObserverMode_Walking(ent);
}
void BeGunner (edict_t *ent, pmenuhnd_t *p)
{
	if (ban_gunner->value)
	{
		safe_cprintf (ent, PRINT_HIGH, "Sorry, that monster is banned.\n");
		return;
	}
    ent->client->resp.class = 5;
    EndObserverMode_Walking(ent);
}
void BeGladiator (edict_t *ent, pmenuhnd_t *p)
{
	if (ban_gladiator->value)
	{
		safe_cprintf (ent, PRINT_HIGH, "Sorry, that monster is banned.\n");
		return;
	}
    ent->client->resp.class = 6;
    EndObserverMode_Walking(ent);
}
void BeSupertank (edict_t *ent, pmenuhnd_t *p)
{
	if (ban_supertank->value)
	{
		safe_cprintf (ent, PRINT_HIGH, "Sorry, that monster is banned.\n");
		return;
	}
    ent->client->resp.class = 7;
    EndObserverMode_Tread(ent);
}
void BeFlyer (edict_t *ent, pmenuhnd_t *p)
{
	if (ban_flyer->value)
	{
		safe_cprintf (ent, PRINT_HIGH, "Sorry, that monster is banned.\n");
		return;
	}
    ent->client->resp.class = 8;
    EndObserverMode_Flying(ent);
}
void BeFixbot (edict_t *ent, pmenuhnd_t *p)
{
	if (ban_fixbot->value)
	{
		safe_cprintf (ent, PRINT_HIGH, "Sorry, that monster is banned.\n");
		return;
	}
    ent->client->resp.class = 32;
    EndObserverMode_Flying(ent);
}
void BeShark (edict_t *ent, pmenuhnd_t *p)
{
	if (ban_shark->value)
	{
		safe_cprintf (ent, PRINT_HIGH, "Sorry, that monster is banned.\n");
		return;
	}
    ent->client->resp.class = 17;
    EndObserverMode_Walking(ent);
}
void BeFloater (edict_t *ent, pmenuhnd_t *p)
{
	if (ban_floater->value)
	{
		safe_cprintf (ent, PRINT_HIGH, "Sorry, that monster is banned.\n");
		return;
	}
    ent->client->resp.class = 16;
    EndObserverMode_Flying(ent);
}
void BeBerserk (edict_t *ent, pmenuhnd_t *p)
{
	if (ban_berserk->value)
	{
		safe_cprintf (ent, PRINT_HIGH, "Sorry, that monster is banned.\n");
		return;
	}
    ent->client->resp.class = 9;
    EndObserverMode_Walking(ent);
}
void BeChick (edict_t *ent, pmenuhnd_t *p)
{
	if (ban_ironmaiden->value)
	{
		safe_cprintf (ent, PRINT_HIGH, "Sorry, that monster is banned.\n");
		return;
	}
    ent->client->resp.class = 10;
    EndObserverMode_Walking(ent);
}
void BeBoss2 (edict_t *ent, pmenuhnd_t *p)
{
	if (ban_boss2->value)
	{
		safe_cprintf (ent, PRINT_HIGH, "Sorry, that monster is banned.\n");
		return;
	}
    ent->client->resp.class = 11;
    EndObserverMode_Flying(ent);
}
void BeCarrier (edict_t *ent, pmenuhnd_t *p)
{
	if (ban_carrier->value)
	{
		safe_cprintf (ent, PRINT_HIGH, "Sorry, that monster is banned.\n");
		return;
	}
    ent->client->resp.class = 29;
    EndObserverMode_Flying(ent);
}
void BeBlackWidow (edict_t *ent, pmenuhnd_t *p)
{
	if (ban_widow1->value)
	{
		safe_cprintf (ent, PRINT_HIGH, "Sorry, that monster is banned.\n");
		return;
	}
    ent->client->resp.class = 30;
    EndObserverMode_Walking(ent);
}
void BeBlackWidow2 (edict_t *ent, pmenuhnd_t *p)
{
	if (ban_widow2->value)
	{
		safe_cprintf (ent, PRINT_HIGH, "Sorry, that monster is banned.\n");
		return;
	}
    ent->client->resp.class = 31;
    EndObserverMode_Walking(ent);
}
void BeBrain (edict_t *ent, pmenuhnd_t *p)
{
	if (ban_brain->value)
	{
		safe_cprintf (ent, PRINT_HIGH, "Sorry, that monster is banned.\n");
		return;
	}
    ent->client->resp.class = 12;
    EndObserverMode_Walking(ent);
}
void BeMutant (edict_t *ent, pmenuhnd_t *p)
{
	if (ban_mutant->value)
	{
		safe_cprintf (ent, PRINT_HIGH, "Sorry, that monster is banned.\n");
		return;
	}
    ent->client->resp.class = 13;
    EndObserverMode_Walking(ent);
}
void BeParasite (edict_t *ent, pmenuhnd_t *p)
{
	if (ban_parasite->value)
	{
		safe_cprintf (ent, PRINT_HIGH, "Sorry, that monster is banned.\n");
		return;
	}
	//safe_cprintf (ent, PRINT_HIGH, "I'm sorry, parasite is being removed from StroggDM.\nHe is not functioning correctly.\nMake way for a new monster!\n");
	//return;
    ent->client->resp.class = 40;
    EndObserverMode_Walking(ent);
}
void BeCTank (edict_t *ent, pmenuhnd_t *p)
{
	if (ban_ctank->value)
	{
		safe_cprintf (ent, PRINT_HIGH, "Sorry, that monster is banned.\n");
		return;
	}
	//safe_cprintf (ent, PRINT_HIGH, "I'm sorry, parasite is being removed from StroggDM.\nHe is not functioning correctly.\nMake way for a new monster!\n");
	//return;
    ent->client->resp.class = 41;
    EndObserverMode_Walking(ent);
}
void BeJorg (edict_t *ent, pmenuhnd_t *p)
{
	if (ban_jorg->value)
	{
		safe_cprintf (ent, PRINT_HIGH, "Sorry, that monster is banned.\n");
		return;
	}
    ent->client->resp.class = 18;
    EndObserverMode_Walking(ent);
}
void BeMakron (edict_t *ent, pmenuhnd_t *p)
{
	if (ban_makron->value)
	{
		safe_cprintf (ent, PRINT_HIGH, "Sorry, that monster is banned.\n");
		return;
	}
    ent->client->resp.class = 19;
    EndObserverMode_Walking(ent);
}
void BeIcarus (edict_t *ent, pmenuhnd_t *p)
{
	if (ban_icarus->value)
	{
		safe_cprintf (ent, PRINT_HIGH, "Sorry, that monster is banned.\n");
		return;
	}
    ent->client->resp.class = 15;
    EndObserverMode_Flying(ent);
}
// MP1
void BeBETASoldier (edict_t *ent, pmenuhnd_t *p)
{
	if (ban_betasoldier->value)
	{
		safe_cprintf (ent, PRINT_HIGH, "Sorry, that monster is banned.\n");
		return;
	}
    ent->client->resp.class = 20;
    EndObserverMode_Walking(ent);
}
void BeBETABrain (edict_t *ent, pmenuhnd_t *p)
{
	if (ban_betabrain->value)
	{
		safe_cprintf (ent, PRINT_HIGH, "Sorry, that monster is banned.\n");
		return;
	}
    ent->client->resp.class = 22;
    EndObserverMode_Walking(ent);
}
void BeBETAMaiden (edict_t *ent, pmenuhnd_t *p)
{
	if (ban_betaironmaiden->value)
	{
		safe_cprintf (ent, PRINT_HIGH, "Sorry, that monster is banned.\n");
		return;
	}
    ent->client->resp.class = 23;
    EndObserverMode_Walking(ent);
}
void BeBETASupertank (edict_t *ent, pmenuhnd_t *p)
{
	if (ban_betasupertank->value)
	{
		safe_cprintf (ent, PRINT_HIGH, "Sorry, that monster is banned.\n");
		return;
	}
    ent->client->resp.class = 24;
    EndObserverMode_Tread(ent);
}
void BeBETAGlad (edict_t *ent, pmenuhnd_t *p)
{
	if (ban_betagladiator->value)
	{
		safe_cprintf (ent, PRINT_HIGH, "Sorry, that monster is banned.\n");
		return;
	}
    ent->client->resp.class = 21;
    EndObserverMode_Walking(ent);
}
void BeGekk (edict_t *ent, pmenuhnd_t *p)
{
	if (ban_gekk->value)
	{
		safe_cprintf (ent, PRINT_HIGH, "Sorry, that monster is banned.\n");
		return;
	}
    ent->client->resp.class = 25;
    EndObserverMode_Walking(ent);
}
// MP2
void BeDaedalus (edict_t *ent, pmenuhnd_t *p)
{
	if (ban_daedalus->value)
	{
		safe_cprintf (ent, PRINT_HIGH, "Sorry, that monster is banned.\n");
		return;
	}
    ent->client->resp.class = 26;
    EndObserverMode_Flying(ent);
}
void BeStalker (edict_t *ent, pmenuhnd_t *p)
{
	if (ban_stalker->value)
	{
		safe_cprintf (ent, PRINT_HIGH, "Sorry, that monster is banned.\n");
		return;
	}
    ent->client->resp.class = 27;
    EndObserverMode_Walking(ent);
}
// "Be"s
// Roseville Mission Packs
void BeDeathTank (edict_t *ent, pmenuhnd_t *p)
{
	if (ban_deathtank->value)
	{
		safe_cprintf (ent, PRINT_HIGH, "Sorry, that monster is banned.\n");
		return;
	}

    ent->client->resp.class = 33;
    EndObserverMode_Walking(ent);
}

void BeLaserGladiator (edict_t *ent, pmenuhnd_t *p)
{
	if (ban_lasergladiator->value)
	{
		safe_cprintf (ent, PRINT_HIGH, "Sorry, that monster is banned.\n");
		return;
	}

    ent->client->resp.class = 34;
    EndObserverMode_Walking(ent);
}

void BeFlyerHornet (edict_t *ent, pmenuhnd_t *p)
{
	if (ban_flyerhornet->value)
	{
		safe_cprintf (ent, PRINT_HIGH, "Sorry, that monster is banned.\n");
		return;
	}
    ent->client->resp.class = 35;
    EndObserverMode_Flying(ent);
}

void BeSoldierStorm (edict_t *ent, pmenuhnd_t *p)
{
	if (ban_stormt->value)
	{
		safe_cprintf (ent, PRINT_HIGH, "Sorry, that monster is banned.\n");
		return;
	}

    ent->client->resp.class = 36;
    EndObserverMode_Walking(ent);
}

void BeMaleTraitor (edict_t *ent, pmenuhnd_t *p)
{
	if (ban_traitorm->value)
	{
		safe_cprintf (ent, PRINT_HIGH, "Sorry, that monster is banned.\n");
		return;
	}

    ent->client->resp.class = 37;
    EndObserverMode_Walking(ent);
}

void BeFemaleTraitor (edict_t *ent, pmenuhnd_t *p)
{
	if (ban_traitorf->value)
	{
		safe_cprintf (ent, PRINT_HIGH, "Sorry, that monster is banned.\n");
		return;
	}

    ent->client->resp.class = 38;
    EndObserverMode_Walking(ent);
}
void BeHypertank (edict_t *ent, pmenuhnd_t *p)
{
	if (ban_supertank->value)
	{
		safe_cprintf (ent, PRINT_HIGH, "Sorry, that monster is banned.\n");
		return;
	}
    ent->client->resp.class = 39;
    EndObserverMode_Tread(ent);
}

void CloseThisMenu (edict_t *ent, pmenuhnd_t *p)
{
	PMenu_CloseIt (ent);
}

void GoBackMain (edict_t *ent, pmenuhnd_t *p)
{
	PMenu_CloseIt (ent);
}

void OpenGroundQ2MenuGround (edict_t *ent, pmenuhnd_t *p)
{
	memset (ent->client->current_menu, 0, sizeof(pmenu_t));
	memcpy(ent->client->current_menu, groundmenu, sizeof(groundmenu));
	
	if ((int)(stroggflags->value) & SF_RPG_MODE)
		CheckLevelForMonster (ent, "groundmenu", ent->client->resp.lvl, ent->client->resp.class);

	CheckBanMenu (ent, ban_soldier, 3);
	CheckBanMenu (ent, ban_enforcer, 4);
	CheckBanMenu (ent, ban_gunner, 5);
	CheckBanMenu (ent, ban_berserk, 6);
	CheckBanMenu (ent, ban_medic, 7);
	CheckBanMenu (ent, ban_gladiator, 8);
	CheckBanMenu (ent, ban_tank, 9);
	CheckBanMenu (ent, ban_supertank, 10);

	PMenu_CloseIt (ent);
	PMenu_Open(ent, /*groundmenu*/ent->client->current_menu, -1, sizeof(/*groundmenu*/ent->client->current_menu) / sizeof(pmenu_t), NULL);
}

void OpenGroundGZMenuGround (edict_t *ent, pmenuhnd_t *p)
{
	memset (ent->client->current_menu, 0, sizeof(pmenu_t));
	memcpy(ent->client->current_menu, groundgz, sizeof(groundgz));

	if ((int)(stroggflags->value) & SF_RPG_MODE)
		CheckLevelForMonster (ent, "groundgz", ent->client->resp.lvl, ent->client->resp.class);

	CheckBanMenu (ent, ban_stalker, 3);
	CheckBanMenu (ent, ban_mediccommander, 4);
	CheckBanMenu (ent, ban_widow1, 5);
	CheckBanMenu (ent, ban_widow2, 6);

	PMenu_CloseIt (ent);

	PMenu_Open(ent, /*flymenu*/ent->client->current_menu, -1, sizeof(/*flymenu*/ent->client->current_menu) / sizeof(pmenu_t), NULL);
}

void OpenGroundRoseMenu (edict_t *ent, pmenuhnd_t *p)
{
	memset (ent->client->current_menu, 0, sizeof(pmenu_t));
	memcpy(ent->client->current_menu, groundrose, sizeof(groundrose));

	if ((int)(stroggflags->value) & SF_RPG_MODE)
		CheckLevelForMonster (ent, "groundrose", ent->client->resp.lvl, ent->client->resp.class);

	CheckBanMenu (ent, ban_deathtank, 4);
	CheckBanMenu (ent, ban_lasergladiator, 5);
	CheckBanMenu (ent, ban_stormt, 6);
	CheckBanMenu (ent, ban_traitorm, 7);
	CheckBanMenu (ent, ban_traitorf, 8);
	CheckBanMenu (ent, ban_hypertank, 9);

	PMenu_CloseIt (ent);

	PMenu_Open(ent, /*flymenu*/ent->client->current_menu, -1, sizeof(/*flymenu*/ent->client->current_menu) / sizeof(pmenu_t), NULL);
}


void OpenGroundTRMenuFly (edict_t *ent, pmenuhnd_t *p)
{
	memset (ent->client->current_menu, 0, sizeof(pmenu_t));
	memcpy(ent->client->current_menu, flytr, sizeof(flytr));

	if ((int)(stroggflags->value) & SF_RPG_MODE)
		CheckLevelForMonster (ent, "flytr", ent->client->resp.lvl, ent->client->resp.class);

	CheckBanMenu (ent, ban_fixbot, 4);

	PMenu_CloseIt (ent);

	PMenu_Open(ent, /*flymenu*/ent->client->current_menu, -1, sizeof(/*flymenu*/ent->client->current_menu) / sizeof(pmenu_t), NULL);
}

void OpenGroundQ2MenuFly (edict_t *ent, pmenuhnd_t *p)
{
	memset (ent->client->current_menu, 0, sizeof(pmenu_t));
	memcpy(ent->client->current_menu, flymenu, sizeof(flymenu));

	if ((int)(stroggflags->value) & SF_RPG_MODE)
		CheckLevelForMonster (ent, "flymenu", ent->client->resp.lvl, ent->client->resp.class);

	CheckBanMenu (ent, ban_flyer, 3);
	CheckBanMenu (ent, ban_icarus, 4);
	CheckBanMenu (ent, ban_floater, 5);
	CheckBanMenu (ent, ban_boss2, 6);

	PMenu_CloseIt (ent);

	PMenu_Open(ent, /*flymenu*/ent->client->current_menu, -1, sizeof(/*flymenu*/ent->client->current_menu) / sizeof(pmenu_t), NULL);
}

void OpenGroundGZMenuFly (edict_t *ent, pmenuhnd_t *p)
{
	memset (ent->client->current_menu, 0, sizeof(pmenu_t));
	memcpy(ent->client->current_menu, flygz, sizeof(flytr));

	if ((int)(stroggflags->value) & SF_RPG_MODE)
		CheckLevelForMonster (ent, "flygz", ent->client->resp.lvl, ent->client->resp.class);

	CheckBanMenu (ent, ban_carrier, 4);
	CheckBanMenu (ent, ban_daedalus, 5);

	PMenu_CloseIt (ent);

	PMenu_Open(ent, /*flymenu*/ent->client->current_menu, -1, sizeof(/*flymenu*/ent->client->current_menu) / sizeof(pmenu_t), NULL);
}

void OpenGroundTRMenuGround (edict_t *ent, pmenuhnd_t *p)
{
	memset (ent->client->current_menu, 0, sizeof(pmenu_t));
	memcpy(ent->client->current_menu, groundtr, sizeof(groundtr));

	if ((int)(stroggflags->value) & SF_RPG_MODE)
		CheckLevelForMonster (ent, "groundtr", ent->client->resp.lvl, ent->client->resp.class);

	CheckBanMenu (ent, ban_gekk, 4);
	CheckBanMenu (ent, ban_betasoldier, 5);
	CheckBanMenu (ent, ban_betagladiator, 6);
	CheckBanMenu (ent, ban_betabrain, 7);
	CheckBanMenu (ent, ban_betaironmaiden, 8);
	CheckBanMenu (ent, ban_betasupertank, 9);

	PMenu_CloseIt (ent);

	PMenu_Open(ent, /*flymenu*/ent->client->current_menu, -1, sizeof(/*flymenu*/ent->client->current_menu) / sizeof(pmenu_t), NULL);
}

void OpenGroundMain (edict_t *ent, pmenuhnd_t *p)
{
	memset (ent->client->current_menu, 0, sizeof(pmenu_t));
	memcpy(ent->client->current_menu, ground_pick, sizeof(ground_pick));

	PMenu_CloseIt (ent);

	PMenu_Open(ent, /*flymenu*/ent->client->current_menu, -1, sizeof(/*flymenu*/ent->client->current_menu) / sizeof(pmenu_t), NULL);
}
void OpenGroundMenu (edict_t *ent, pmenuhnd_t *p)
{
	memset (ent->client->current_menu, 0, sizeof(pmenu_t));
	memcpy(ent->client->current_menu, ground_pick, sizeof(ground_pick));

	PMenu_CloseIt (ent);

	PMenu_Open(ent, /*flymenu*/ent->client->current_menu, -1, sizeof(/*flymenu*/ent->client->current_menu) / sizeof(pmenu_t), NULL);
}

void GoToGround2Menu (edict_t *ent, pmenuhnd_t *p)
{
	memset (ent->client->current_menu, 0, sizeof(pmenu_t));
	memcpy(ent->client->current_menu, ground2, sizeof(ground2));

	if ((int)(stroggflags->value) & SF_RPG_MODE)
		CheckLevelForMonster (ent, "ground2", ent->client->resp.lvl, ent->client->resp.class);

	CheckBanMenu (ent, ban_ironmaiden, 3);
	CheckBanMenu (ent, ban_brain, 4);
	CheckBanMenu (ent, ban_mutant, 5);
	CheckBanMenu (ent, ban_ctank, 6);
	CheckBanMenu (ent, ban_jorg, 7);
	CheckBanMenu (ent, ban_makron, 8);

	PMenu_CloseIt (ent);

	PMenu_Open(ent, /*flymenu*/ent->client->current_menu, -1, sizeof(/*flymenu*/ent->client->current_menu) / sizeof(pmenu_t), NULL);
}

void GoToGround3Menu (edict_t *ent, pmenuhnd_t *p)
{
	memset (ent->client->current_menu, 0, sizeof(pmenu_t));
	memcpy(ent->client->current_menu, ground3, sizeof(ground3));

	PMenu_CloseIt (ent);

	PMenu_Open(ent, /*flymenu*/ent->client->current_menu, -1, sizeof(/*flymenu*/ent->client->current_menu) / sizeof(pmenu_t), NULL);
}
/*void GoToGround4Menu (edict_t *ent, pmenuhnd_t *p)
{
	PMenu_CloseIt (ent);
	PMenu_Open(ent, ground4, -1, sizeof(ground4) / sizeof(pmenu_t), NULL);
}*/
void GoBack (edict_t *ent, pmenuhnd_t *p)
{
	memset (ent->client->current_menu, 0, sizeof(pmenu_t));
	memcpy(ent->client->current_menu, mainmenu, sizeof(mainmenu));

	PMenu_CloseIt (ent);

	PMenu_Open(ent, /*flymenu*/ent->client->current_menu, -1, sizeof(/*flymenu*/ent->client->current_menu) / sizeof(pmenu_t), NULL);
}

void OpenFlyMenu (edict_t *ent, pmenuhnd_t *p)
{
	memset (ent->client->current_menu, 0, sizeof(pmenu_t));
	memcpy(ent->client->current_menu, fly_pick, sizeof(fly_pick));

	PMenu_CloseIt (ent);

	PMenu_Open(ent, /*flymenu*/ent->client->current_menu, -1, sizeof(/*flymenu*/ent->client->current_menu) / sizeof(pmenu_t), NULL);
}

void OpenFlyRoseMenu (edict_t *ent, pmenuhnd_t *p)
{
	memset (ent->client->current_menu, 0, sizeof(pmenu_t));
	memcpy(ent->client->current_menu, fly_rose, sizeof(fly_rose));

	if ((int)(stroggflags->value) & SF_RPG_MODE)
		CheckLevelForMonster (ent, "fly_rose", ent->client->resp.lvl, ent->client->resp.class);

	CheckBanMenu (ent, ban_flyerhornet, 3);

	PMenu_CloseIt (ent);

	PMenu_Open(ent, /*flymenu*/ent->client->current_menu, -1, sizeof(/*flymenu*/ent->client->current_menu) / sizeof(pmenu_t), NULL);
}

void OpenSwimMenu (edict_t *ent, pmenuhnd_t *p)
{
	memset (ent->client->current_menu, 0, sizeof(pmenu_t));
	memcpy(ent->client->current_menu, swimmenu, sizeof(swimmenu));

	if ((int)(stroggflags->value) & SF_RPG_MODE)
		CheckLevelForMonster (ent, "swimmenu", ent->client->resp.lvl, ent->client->resp.class);

	CheckBanMenu (ent, ban_shark, 3);

	PMenu_CloseIt (ent);

	PMenu_Open(ent, /*flymenu*/ent->client->current_menu, -1, sizeof(/*flymenu*/ent->client->current_menu) / sizeof(pmenu_t), NULL);
}

void OpenClassMenu (edict_t *ent, pmenuhnd_t *p)
{
	memset (ent->client->current_menu, 0, sizeof(pmenu_t));
	memcpy(ent->client->current_menu, classtypemenu, sizeof(classtypemenu));

	PMenu_CloseIt (ent);

	PMenu_Open(ent, /*flymenu*/ent->client->current_menu, -1, sizeof(/*flymenu*/ent->client->current_menu) / sizeof(pmenu_t), NULL);
}
void OpenMainMenus (edict_t *ent)
{
	memset (ent->client->current_menu, 0, sizeof(pmenu_t));
	memcpy(ent->client->current_menu, mainmenu, sizeof(mainmenu));

	PMenu_CloseIt (ent);

	PMenu_Open(ent, /*flymenu*/ent->client->current_menu, -1, sizeof(/*flymenu*/ent->client->current_menu) / sizeof(pmenu_t), NULL);
}
void OpenMainMenu (edict_t *ent, pmenuhnd_t *p)
{
	memset (ent->client->current_menu, 0, sizeof(pmenu_t));
	memcpy(ent->client->current_menu, mainmenu, sizeof(mainmenu));

	PMenu_CloseIt (ent);

	PMenu_Open(ent, /*flymenu*/ent->client->current_menu, -1, sizeof(/*flymenu*/ent->client->current_menu) / sizeof(pmenu_t), NULL);
}
// RPG, Lawl.
void OpenLoginMenu (edict_t *ent, pmenuhnd_t *p)
{
	memset (ent->client->current_menu, 0, sizeof(pmenu_t));
	memcpy(ent->client->current_menu, loginmenu, sizeof(loginmenu));

	PMenu_CloseIt (ent);

	PMenu_Open(ent, /*flymenu*/ent->client->current_menu, -1, sizeof(/*flymenu*/ent->client->current_menu) / sizeof(pmenu_t), NULL);
}


pmenu_t partymenu[] = {
	{ "*Party Menu",		PMENU_ALIGN_CENTER, NULL },
	{ "StroggDM",		PMENU_ALIGN_CENTER, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL }, 
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },   
	{ NULL,				PMENU_ALIGN_LEFT, NULL }, 
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },   
	{ NULL,				PMENU_ALIGN_LEFT, NULL }, 
	{ NULL,				PMENU_ALIGN_LEFT, NULL }, 
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ "Close",			PMENU_ALIGN_LEFT, CloseThisMenu }
};

void ShowPartyMenuParties (edict_t *ent, pmenu_t current_menu[1536])
{
	int i;
	edict_t *foundent;
	int p = 2;

	// Go through the list, let's find all the parties!
	for (i=0 ; i<maxclients->value ; i++)
	{
		foundent = g_edicts + 1 + i;

		if (foundent->client->resp.party.has_party) // Does this client own a party?
		{
			p++;

			current_menu[p].text = strdup(foundent->client->resp.party.name);
			// Paril, FIXME
			//currentmenu[p].SelectFunc = ;
		}
	}
}

void OpenPartyMenu (edict_t *ent, pmenuhnd_t *p)
{
	memset (ent->client->current_menu, 0, sizeof(pmenu_t));
	memcpy(ent->client->current_menu, partymenu, sizeof(partymenu));

	PMenu_CloseIt (ent);

	ShowPartyMenuParties (ent, ent->client->current_menu);

	PMenu_Open(ent, /*flymenu*/ent->client->current_menu, -1, sizeof(/*flymenu*/ent->client->current_menu) / sizeof(pmenu_t), NULL);
}