#include "g_local.h"

pmenu_t k2joinmenu[] = {
	{ "*Quake II",			PMENU_ALIGN_CENTER, NULL, NULL },
	//{ "*Keys2",	PMENU_ALIGN_CENTER, NULL, NULL },
	{ "*" GAMEVERSION,							PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL, NULL },
	{ "Enter the Fragfest",		PMENU_ALIGN_LEFT, NULL, K2EnterGame },
	{ "Q2 Camera",		PMENU_ALIGN_LEFT, NULL, K2MenuCameraCommand },
	{ "Help/Info",		PMENU_ALIGN_LEFT, NULL, K2_OpenHelpMenu },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Credits",			PMENU_ALIGN_LEFT, NULL, K2Credits },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Use [ and ] to move cursor",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "ENTER to select",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "ESC to Exit Menu",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "(TAB to Return)",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	
};

pmenu_t k2creditsmenu[] = {
	{ "*Quake II",						PMENU_ALIGN_CENTER, NULL, NULL },
	{ "*Keys2 Credits",							PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,								PMENU_ALIGN_CENTER, NULL, NULL },
	{ "*Programming",					PMENU_ALIGN_LEFT, NULL, NULL }, 
	{ "Rich 'K2Guy' Shetina",				PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,								PMENU_ALIGN_CENTER, NULL, NULL },
	{ "*Playtesting", 					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Fuk'n_Hostile_K2 and the",				PMENU_ALIGN_LEFT, NULL, NULL },
	{ "K2 Klan",					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Viking and Bambi",				PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Galactus and Capt. Sky",								PMENU_ALIGN_LEFT, NULL, NULL },
	{ "*Additional Credits", 					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "id Software (Quake2)",				PMENU_ALIGN_LEFT, NULL, NULL },
	{ "'Zoid' (CTF)",				PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Expert DM (Hook)",				PMENU_ALIGN_LEFT, NULL, NULL },
	{ "'Ridah' (Eraser)",				PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,								PMENU_ALIGN_CENTER, NULL, NULL },
	{ "Return to Main Menu",			PMENU_ALIGN_CENTER, NULL, K2ReturnToMain }
};

pmenu_t k2welcomemenu[] = {
	{ "*Welcome to Keys2!",						PMENU_ALIGN_CENTER, NULL, NULL },
	{ "mode",						PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,								PMENU_ALIGN_CENTER, NULL, NULL },
	{ "*This server is running",								PMENU_ALIGN_CENTER, NULL, NULL },
	{ "*" GAMEVERSION,					PMENU_ALIGN_CENTER, NULL, NULL }, 
	{ "*www.planetquake.com/keys2",				PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,								PMENU_ALIGN_CENTER, NULL, NULL },
	{ "motd1", 					PMENU_ALIGN_CENTER, NULL, NULL },
	{ "motd2",				PMENU_ALIGN_CENTER, NULL, NULL },
	{ "motd3",					PMENU_ALIGN_CENTER, NULL, NULL },
	{ "motd4",				PMENU_ALIGN_CENTER, NULL, NULL },
	{ "motd5",				PMENU_ALIGN_CENTER, NULL, NULL },
	{ "motd6",				PMENU_ALIGN_CENTER, NULL, NULL },
	{ "motd7",				PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,								PMENU_ALIGN_CENTER, NULL, NULL },
	{ "Go To Join Menu",			PMENU_ALIGN_CENTER, NULL, K2ReturnToMain }
};


pmenu_t k2helpmenu[] = {
	{ "*Quake II",			PMENU_ALIGN_CENTER, NULL, NULL },
	{ "*Keys2 Help Menu",	PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL, NULL },
	{ "The Keys",		PMENU_ALIGN_LEFT, NULL, K2_OpenKeysMenu },
	{ "The Weapons",		PMENU_ALIGN_LEFT, NULL, K2_OpenWeaponsMenu },
	{ "Client Commands",		PMENU_ALIGN_LEFT, NULL, K2_OpenCommandsMenu },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Use [ and ] to move cursor",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "ENTER to select",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Return to Main Menu",			PMENU_ALIGN_LEFT, NULL, K2ReturnToMain },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL }
};

pmenu_t k2keysmenu[] = {
	{ "*Keys2 - The Keys",			PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL, NULL },
	{ "*Regeneration",		PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Health/Armor Regenerate",		PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "*Haste",		PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Weapons fire three",		PMENU_ALIGN_LEFT, NULL, NULL },
	{ "times the normal rate",		PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "*Infliction",		PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Weapons do three",		PMENU_ALIGN_LEFT, NULL, NULL },
	{ "times the normal damage",		PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Next Page",	PMENU_ALIGN_CENTER, NULL, K2_OpenKeysMenu2 },
	{ "Return to Help Menu",PMENU_ALIGN_CENTER, NULL, K2ReturnToHelp },
	{ "Return to Main Menu",PMENU_ALIGN_CENTER, NULL, K2ReturnToMain }
};

pmenu_t k2keysmenu2[] = {
	{ "*Keys2 - The Keys",			PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL, NULL },
	{ "*Futility",		PMENU_ALIGN_LEFT, NULL, NULL },
	{ "You take only 1/4",		PMENU_ALIGN_LEFT, NULL, NULL },
	{ "the normal damage",		PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "*Homing",		PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Rockets,Lasers,and",		PMENU_ALIGN_LEFT, NULL, NULL },
	{ "GibGun home in on targets",PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "*Stealth",		PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Virtually Invisible,",		PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Virtually Silent,",PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Immune from Homing",		PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Next Page",	PMENU_ALIGN_CENTER, NULL, K2_OpenKeysMenu3 },
	{ "Return to Help Menu",PMENU_ALIGN_CENTER, NULL, K2ReturnToHelp },
	{ "Return to Main Menu",PMENU_ALIGN_CENTER, NULL, K2ReturnToMain }
};
	
pmenu_t k2keysmenu3[] = {
	{ "*Keys2 - The Keys",			PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL, NULL },
	{ "*BFK",		PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Combined powers of",		PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Haste,Infliction,Regen,",PMENU_ALIGN_LEFT, NULL, NULL },
	{ "and Futility",PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "*AntiKey",		PMENU_ALIGN_LEFT, NULL, NULL },
	{ "You can take someone's key",		PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Immune from Homing,",PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Enemies feel your pain",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "You get health when",		PMENU_ALIGN_LEFT, NULL, NULL },
	{ "you hurt others",PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Return to Help Menu",PMENU_ALIGN_CENTER, NULL, K2ReturnToHelp },
	{ "Return to Main Menu",PMENU_ALIGN_CENTER, NULL, K2ReturnToMain },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL }
};

		
pmenu_t k2weaponsmenu[] = {
	{ "*Keys2 - The Weapons",			PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL, NULL },
	{ "*Gibgun",					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Toggle with Blaster",					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "*Flash Grenades",					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Toggle with Grenade",					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Launcher or flash command",					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "*Fire Grenades",					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Toggle with Grenade",					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Launcher",					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "*Freeze Grenades",					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Toggle with Grenade",					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Launcher or freeze command",					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "*Fire and Drunk Rockets",					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Toggle with Rocket",					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Launcher or drunk command",					PMENU_ALIGN_LEFT, NULL, NULL },
	//{ NULL,					PMENU_ALIGN_CENTER, NULL, NULL },
	{ "Return to Help Menu",			PMENU_ALIGN_CENTER, NULL, K2ReturnToHelp },
	{ "Return to Main Menu",			PMENU_ALIGN_CENTER, NULL, K2ReturnToMain }
};

pmenu_t k2commandsmenu[] = {
	{ "*Keys2 - Client Commands",			PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL, NULL },
	{ "*+hook",					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Grappling Hook",					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "*+feign (or feign)",					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Pretend to die",					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "*drop key",					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Drop your current key",					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "*take_key (or take)",					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Take someone's key",					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "*flash",					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Toggle flash handgrenades",					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "*id",					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Toggle player id",					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "*keys2 help",					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Display Keys2 Help/Info",					PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Return to Help Menu",			PMENU_ALIGN_CENTER, NULL, K2ReturnToHelp }
};

void K2Credits(edict_t *ent, pmenu_t *p)
{
	K2Menu_Close(ent);
	K2Menu_Open(ent, k2creditsmenu, -1, sizeof(k2creditsmenu) / sizeof(pmenu_t));
}

void K2_OpenWelcomeMenu(edict_t *ent)
{
	
	K2UpdateWelcomeMenu(ent);
	K2Menu_Open(ent, k2welcomemenu, 0, sizeof(k2welcomemenu) / sizeof(pmenu_t));

}

void K2_OpenJoinMenu(edict_t *ent)
{
	K2UpdateJoinMenu(ent);
	K2Menu_Open(ent, k2joinmenu, 0, sizeof(k2joinmenu) / sizeof(pmenu_t));
}

void K2Menu_Open(edict_t *ent, pmenu_t *entries, int cur, int num)
{
	pmenuhnd_t *hnd;
	pmenu_t *p;
	int i;

	if (!ent->client)
		return;

	if (ent->client->menu) {
		gi.dprintf("warning, ent already has a menu\n");
		K2Menu_Close(ent);
	}

	hnd = malloc(sizeof(*hnd));

	hnd->entries = entries;
	hnd->num = num;

	if (cur < 0 || !entries[cur].SelectFunc) {
		for (i = 0, p = entries; i < num; i++, p++)
			if (p->SelectFunc)
				break;
	} else
		i = cur;

	if (i >= num)
		hnd->cur = -1;
	else
		hnd->cur = i;

	ent->client->showscores = true;
	ent->client->inmenu = true;
	ent->client->menu = hnd;
	
	K2Menu_Update(ent);
	gi.unicast (ent, true);
}

void K2Menu_Close(edict_t *ent)
{
	if (!ent->client->menu)
		return;

	free(ent->client->menu);
	ent->client->menu = NULL;
	ent->client->showscores = false;
}

void K2Menu_Update(edict_t *ent)
{
	char string[1400];
	int i;
	pmenu_t *p;
	int x;
	pmenuhnd_t *hnd;
	char *t;
	qboolean alt = false;

	if (!ent->client->menu) {
		gi.dprintf("warning:  ent has no menu\n");
		return;
	}

	hnd = ent->client->menu;

	strcpy(string, "xv 32 yv 8 picn inventory ");

	for (i = 0, p = hnd->entries; i < hnd->num; i++, p++) {
		if (!p->text || !*(p->text))
			continue; // blank line
		t = p->text;
		if (*t == '*') {
			alt = true;
			t++;
		}
		sprintf(string + strlen(string), "yv %d ", 32 + i * 8);
		if (p->align == PMENU_ALIGN_CENTER)
			x = 196/2 - strlen(t)*4 + 64;
		else if (p->align == PMENU_ALIGN_RIGHT)
			x = 64 + (196 - strlen(t)*8);
		else
			x = 64;

		sprintf(string + strlen(string), "xv %d ",
			x - ((hnd->cur == i) ? 8 : 0));

		if (hnd->cur == i)
			sprintf(string + strlen(string), "string2 \"\x0d%s\" ", t);
		else if (alt)
			sprintf(string + strlen(string), "string2 \"%s\" ", t);
		else
			sprintf(string + strlen(string), "string \"%s\" ", t);
		alt = false;
	}

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}

void K2Menu_Next(edict_t *ent)
{
	pmenuhnd_t *hnd;
	int i;
	pmenu_t *p;

	if (!ent->client->menu) {
		gi.dprintf("warning:  ent has no menu\n");
		return;
	}

	hnd = ent->client->menu;

	if (hnd->cur < 0)
		return; // no selectable entries

	i = hnd->cur;
	p = hnd->entries + hnd->cur;
	do {
		i++, p++;
		if (i == hnd->num)
			i = 0, p = hnd->entries;
		if (p->SelectFunc)
			break;
	} while (i != hnd->cur);

	hnd->cur = i;

	K2Menu_Update(ent);
	gi.unicast (ent, true);
}

void K2Menu_Prev(edict_t *ent)
{
	pmenuhnd_t *hnd;
	int i;
	pmenu_t *p;

	if (!ent->client->menu) {
		gi.dprintf("warning:  ent has no menu\n");
		return;
	}

	hnd = ent->client->menu;

	if (hnd->cur < 0)
		return; // no selectable entries

	i = hnd->cur;
	p = hnd->entries + hnd->cur;
	do {
		if (i == 0) {
			i = hnd->num - 1;
			p = hnd->entries + i;
		} else
			i--, p--;
		if (p->SelectFunc)
			break;
	} while (i != hnd->cur);

	hnd->cur = i;

	K2Menu_Update(ent);
	gi.unicast (ent, true);
}

void K2Menu_Select(edict_t *ent)
{
	pmenuhnd_t *hnd;
	pmenu_t *p;

	if (!ent->client->menu) {
		gi.dprintf("warning:  ent has no menu\n");
		return;
	}

	hnd = ent->client->menu;

	if (hnd->cur < 0)
		return; // no selectable entries

	p = hnd->entries + hnd->cur;

	if (p->SelectFunc)
		p->SelectFunc(ent, p);
}

void K2ReturnToMain(edict_t *ent, pmenu_t *p)
{
	K2Menu_Close(ent);
	
	if(ent->client->resp.inServer)
		return;

	if(ctf->value)
		CTFOpenJoinMenu(ent);
	else
		K2_OpenJoinMenu(ent);
}

void K2ReturnToHelp(edict_t *ent, pmenu_t *p)
{
	K2Menu_Close(ent);
	K2_OpenHelpMenu(ent, p);
}

void K2_OpenHelpMenu(edict_t *ent, pmenu_t *p)
{
	K2Menu_Close(ent);
	K2Menu_Open(ent, k2helpmenu, 0, sizeof(k2helpmenu) / sizeof(pmenu_t));
}

void K2_OpenKeysMenu(edict_t *ent, pmenu_t *p)
{
	K2Menu_Close(ent);
	K2Menu_Open(ent, k2keysmenu, 0, sizeof(k2keysmenu) / sizeof(pmenu_t));
}

void K2_OpenWeaponsMenu(edict_t *ent, pmenu_t *p)
{
	K2Menu_Close(ent);
	K2Menu_Open(ent, k2weaponsmenu, 0, sizeof(k2weaponsmenu) / sizeof(pmenu_t));
}

void K2_OpenCommandsMenu(edict_t *ent, pmenu_t *p)
{
	K2Menu_Close(ent);
	K2Menu_Open(ent, k2commandsmenu, 0, sizeof(k2commandsmenu) / sizeof(pmenu_t));
}

void K2_OpenKeysMenu2(edict_t *ent, pmenu_t *p)
{
	K2Menu_Close(ent);
	K2Menu_Open(ent, k2keysmenu2, 0, sizeof(k2keysmenu2) / sizeof(pmenu_t));
}

void K2_OpenKeysMenu3(edict_t *ent, pmenu_t *p)
{
	K2Menu_Close(ent);
	K2Menu_Open(ent, k2keysmenu3, 0, sizeof(k2keysmenu3) / sizeof(pmenu_t));
}

int K2UpdateWelcomeMenu(edict_t *ent)
{
	if(ctf->value)
		k2welcomemenu[1].text = "(CTF Mode)";
	else
		k2welcomemenu[1].text = "(DM Mode)";

	
	k2welcomemenu[7].text = motd1->string;
	k2welcomemenu[8].text = motd2->string;
	k2welcomemenu[9].text = motd3->string;
	k2welcomemenu[10].text = motd4->string;
	k2welcomemenu[11].text = motd5->string;
	k2welcomemenu[12].text = motd6->string;
	k2welcomemenu[13].text = motd7->string;
	
		
	return 1;

}

int K2UpdateJoinMenu(edict_t *ent)
{
	static char levelname[32];
		
	if (ent->client->bIsCamera)
		k2joinmenu[5].text = NULL;
	else
		k2joinmenu[5].text = "Q2 Camera";

	levelname[0] = '*';
	if (g_edicts[0].message)
		strncpy(levelname+1, g_edicts[0].message, sizeof(levelname) - 2);
	else
		strncpy(levelname+1, level.mapname, sizeof(levelname) - 2);
	levelname[sizeof(levelname) - 1] = 0;

	k2joinmenu[2].text = levelname;
	
	return 1;

}

void K2MenuCameraCommand(edict_t *ent, pmenu_t *p)
{

	if (ent->client->bIsCamera)
		CameraCmd(ent,"off");
	else
		CameraCmd(ent,"on");
	
}