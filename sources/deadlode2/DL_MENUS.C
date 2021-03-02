#include "g_local.h"

pmenu_t pCurMenu[64];

#define SHOW_COMPATIBLE		0
#define SHOW_ALL			1

void OpenPlayerMenu(edict_t *ent, pmenu_t *p);
void OpenConfigMenu(edict_t *ent, pmenu_t *p);
void OpenAmmoStats(edict_t *ent, pmenu_t *p);
void OpenWeaponStats(edict_t *ent, pmenu_t *p);
void OpenAmmoVariantsMenu(edict_t *ent, pmenu_t *p);
void OpenAmmoTypesMenu(edict_t *ent, pmenu_t *p);
void OpenWeaponsMenu(edict_t *ent, pmenu_t *p);
void OpenInventoryMenu(edict_t *ent, pmenu_t *p);
void OpenAmmoQuantityMenu(edict_t *ent, pmenu_t *p);
void OpenSuppliesMenu(edict_t *ent, pmenu_t *p);
void OpenSupplyStats(edict_t *ent, pmenu_t *p);
void OpenSupplyQuantityMenu(edict_t *ent, pmenu_t *p);
void OpenProfilesMenu(edict_t *ent, pmenu_t *p);
void OpenPersonalMenu(edict_t *ent, pmenu_t *p);
void OpenObjectPropertiesMenu (edict_t *ent, pmenu_t *p);
void OpenMapVoteMenu(edict_t *ent, pmenu_t *p);
void OpenTeamSelectionMenu(edict_t *ent, pmenu_t *p);
void OpenPlayerStatsMenu(edict_t *ent, pmenu_t *p);
void OpenAlternateMenu(edict_t *ent, pmenu_t *p);
void OpenCreditsMenu(edict_t *ent, pmenu_t *p);
void OpenMOTDMenu(edict_t *ent, pmenu_t *p);

void ResetProfile(edict_t *ent, pmenu_t *p);

void SelectItem(edict_t *ent, pmenu_t *p);
void AddAmmoToInventory(edict_t *ent, pmenu_t *p);
void AddWeaponToInventory(edict_t *ent, pmenu_t *p);
void AddSupplyToInventory(edict_t *ent, pmenu_t *p);
void PreviousMenu(edict_t *ent, pmenu_t *p);
void ExitMenu(edict_t *ent, pmenu_t *p);
void SelectSystemProfile(edict_t *ent, pmenu_t *p);
void SelectTeamCamera (edict_t *ent, pmenu_t *p);
void SelectObject_Personal (edict_t *ent, pmenu_t *p);
void SelectObject_Team (edict_t *ent, pmenu_t *p);
void DestroyObject (edict_t *ent, pmenu_t *p);
void ChangeTeamVisible (edict_t *ent, pmenu_t *p);
void Menu_BuildingRepair(edict_t *ent, pmenu_t *p);
void Menu_BuildingRotate(edict_t *ent, pmenu_t *p);
void Menu_BuildingDestroy(edict_t *ent, pmenu_t *p);
void Menu_BuildingUse(edict_t *ent, pmenu_t *p);
void Menu_TriggerReclaim(edict_t *ent, pmenu_t *p);
void Menu_TriggerDestroy(edict_t *ent, pmenu_t *p);
void Menu_SelectUniform(edict_t *ent, pmenu_t *p);


pmenu_t MakeMenuItem(char *mformat, ...) {
/*******
format chars:
	T = Display Text			(char *)
	B = Big Pic					(char *)
	S = Small Pic				(char *)
	A = Argument				(void *)
	F = Display Function		(void (*SelectFunc)(edict_t *ent, struct pmenu_s *entry))
	G = Alt Display Function	(void (*AltSelectFunc)(edict_t *ent, struct pmenu_s *entry))
	C = Center Alignment		(none)
	R = Right Alignment			(none)
	O = X Offset				(int)
	P = Parameter (upto 5)		(int)
	H = always display at top   (none)
	L = always display at bottom (none)
	N = Target (targetnames)	(char *)
*******/
pmenu_t p;
va_list	argptr;
int parmcnt=0;
void *TempFunc;
	
	memset(&p, 0, sizeof(p));

	va_start (argptr, mformat);
	while (mformat && *mformat) {
		switch (*mformat) {
		case 'T': p.text = ED_NewString(va_arg(argptr, char *)); break;
		case 'B': p.pic_b = ED_NewString(va_arg(argptr, char *)); break;
		case 'S': p.pic_s = ED_NewString(va_arg(argptr, char *)); break;
		case 'N': p.target = ED_NewString(va_arg(argptr, char *)); break;
		case 'A': p.arg = va_arg(argptr, void *); break;
		case 'F': 
			TempFunc = va_arg(argptr, void *);
			p.SelectFunc = (void (*)(edict_t *,struct pmenu_s *))(TempFunc);
			break;
		case 'G': 
			TempFunc = va_arg(argptr, void *);
			p.AltSelectFunc = (void (*)(edict_t *,struct pmenu_s *))(TempFunc);
			break;

		case 'O': p.xofs = va_arg(argptr, int); break;
		case 'P': p.param[parmcnt++] = va_arg(argptr, int); break;

		case 'C': p.align = PMENU_ALIGN_CENTER; break;
		case 'R': p.align = PMENU_ALIGN_RIGHT; break;

		case 'H': p.flags = 0x01; break;	// PMENU_TYPE_HEADER
		case 'L': p.align = 0x02; break;	// PMENU_TYPE_TRAILER
		}
		mformat++;
	}
	va_end (argptr);

	if ((p.SelectFunc == ExitMenu) || (p.SelectFunc == PreviousMenu)) {
		if (!p.pic_s) p.pic_s = ED_NewString("m_back_s");
		if (!p.pic_b) p.pic_b = ED_NewString("m_back");
	}

	return p;	
}

void PreviousMenu(edict_t *ent, pmenu_t *p) {
	PMenu_Close(ent, true);
}

void ExitMenu(edict_t *ent, pmenu_t *p) {
	PMenu_CloseAll(ent);
}

void ShowMainMenu (edict_t *ent, pmenuhnd_t *hnd) {
int i = 0;

	pCurMenu[i++] = MakeMenuItem("TS", "  ", "empty_w_tile");
	pCurMenu[i++] = MakeMenuItem("T", "*  DeadLode 2 Main Menu");
	pCurMenu[i++] = MakeMenuItem(NULL);
	pCurMenu[i++] = MakeMenuItem(NULL);
	pCurMenu[i++] = MakeMenuItem("T", "Select");

	if (!(ent->movetype != MOVETYPE_NOCLIP && ent->deadflag != DEAD_DEAD)) {
		if (!dlsys.auto_team)
			pCurMenu[i++] = MakeMenuItem("TF", "  Team Selection", OpenTeamSelectionMenu);
//		pCurMenu[i++] = MakeMenuItem("TF", "  Class Profiles", OpenProfilesMenu);
//		pCurMenu[i++] = MakeMenuItem("TF", "  Armoury", OpenShopMenu);
	} else {
		pCurMenu[i++] = MakeMenuItem("TF", "  Inventory", OpenInventoryMenu);
		if (ent->client->prefs.adv_menus) {
			pCurMenu[i++] = MakeMenuItem("TF", "  Personal Objects", OpenPersonalMenu);
			pCurMenu[i++] = MakeMenuItem("TF", "  Team Objects", OpenTeamMenu);
		}
		pCurMenu[i++] = MakeMenuItem(NULL);
//		pCurMenu[i++] = MakeMenuItem("TF", "  Player Statistics", OpenPlayerStatsMenu);
//		pCurMenu[i++] = MakeMenuItem("TF", "  Armoury", OpenShopMenu);
	}
	pCurMenu[i++] = MakeMenuItem("TF", "  Class Profiles", OpenProfilesMenu);

	// Display "Player Settings" menu only if the player has (or can make) a profile
	if (ent && ent->client && ent->client->prefs.IHnd)
		pCurMenu[i++] = MakeMenuItem("TF", "  Player Settings", OpenPlayerMenu);

	if (dlsys.map_voting)
		pCurMenu[i++] = MakeMenuItem("TF", "  Map Voting", OpenMapVoteMenu);
	pCurMenu[i++] = MakeMenuItem("TF", "  DeadLode2 Credits", OpenCreditsMenu);

	if (!ent->client->prefs.shown_motd)
		pCurMenu[i++] = MakeMenuItem("TF", "  Message Of The Day", OpenMOTDMenu);
	pCurMenu[i++] = MakeMenuItem(NULL);

	pCurMenu[i++] = MakeMenuItem("TF", "Exit", ExitMenu);

	hnd->entries = gi.TagMalloc(sizeof(*hnd->entries)*i, TAG_LEVEL);
	memcpy(hnd->entries, pCurMenu, i*sizeof(*hnd->entries));
	hnd->num = i;
}

void OpenMainMenu(edict_t *ent, pmenu_t *p) {
	if (level.intermissiontime)
		PMenu_Open(ent, ID_MAPVOTING, p->param, p->arg);
	else
		PMenu_Open(ent, ID_MAIN, p->param, p->arg);
}

void ShowPlayerMenu (edict_t *ent, pmenuhnd_t *hnd) {
int i = 0;

	if (!ent || !ent->client || !ent->client->prefs.IHnd)
		return;

	pCurMenu[i++] = MakeMenuItem("TS", "  ", "empty_w_tile");
	pCurMenu[i++] = MakeMenuItem("T", "*  DeadLode 2 Player Menu");
	pCurMenu[i++] = MakeMenuItem(NULL);
	pCurMenu[i++] = MakeMenuItem(NULL);

	pCurMenu[i++] = MakeMenuItem("TF", "  Customize Profile", OpenShopMenu);
	pCurMenu[i++] = MakeMenuItem("TF", "  Player Statistics", OpenPlayerStatsMenu);
	pCurMenu[i++] = MakeMenuItem("TF", "  Profile Settings", OpenConfigMenu);
	pCurMenu[i++] = MakeMenuItem(NULL);

	if (ent->client->curmenu)
		pCurMenu[i++] = MakeMenuItem("TF", "Go back", PreviousMenu);
	pCurMenu[i++] = MakeMenuItem("TF", "Exit", ExitMenu);

	hnd->entries = gi.TagMalloc(sizeof(*hnd->entries)*i, TAG_LEVEL);
	memcpy(hnd->entries, pCurMenu, i*sizeof(*hnd->entries));
	hnd->num = i;
}

void OpenPlayerMenu(edict_t *ent, pmenu_t *p) {
	PMenu_Open(ent, ID_PLAYER, p->param, p->arg);
}


void ChangeConfig(edict_t *ent, pmenu_t *p) {
IniFile *IHnd;
int i;

	if (!ent || !ent->client || !ent->client->prefs.IHnd)
		return;

	IHnd = ent->client->prefs.IHnd;

	switch(p->param[0]) {
	case 1:	ent->client->prefs.adv_menus = p->param[1]; break;
	case 2:	ent->client->prefs.no_menu_background = p->param[1]; break;
	case 3:	i = p->param[1];
			if (i > (MENU_POS_RIGHT|MENU_POS_TOP))
				i = MENU_POS_CENTER;
			else if (i > (MENU_POS_RIGHT))
				i = MENU_POS_RIGHT|MENU_POS_TOP;
			else if (i > (MENU_POS_LEFT|MENU_POS_TOP))
				i = MENU_POS_RIGHT;
			else if (i > MENU_POS_LEFT)
				i = MENU_POS_LEFT|MENU_POS_TOP;
			else if (i > MENU_POS_TOP)
				i = MENU_POS_LEFT;
			else if (i > MENU_POS_CENTER)
				i = MENU_POS_TOP;
			else
				i = MENU_POS_CENTER;

			ent->client->prefs.menu_pos = i;
			break;
	case 4:	ent->client->prefs.auto_profile = p->param[1]; break;
	}

	DL_WritePlayerStats(ent);
}

void ShowConfigMenu(edict_t *ent, pmenuhnd_t *hnd) {
int i = 0;

	if (!ent || !ent->client || !ent->client->prefs.IHnd)
		return;

	pCurMenu[i++] = MakeMenuItem("TS", "*  DeadLode 2 Player", "empty_w_tile");
	pCurMenu[i++] = MakeMenuItem("T",  "*  Profile Settings");
	pCurMenu[i++] = MakeMenuItem(NULL);
	pCurMenu[i++] = MakeMenuItem(NULL);

	pCurMenu[i++] = MakeMenuItem("TFPP", va("Advanced Menus    %s", ent->client->prefs.adv_menus ? "ON" : "OFF"), ChangeConfig, 1, !ent->client->prefs.adv_menus);
	pCurMenu[i++] = MakeMenuItem("TFPP", va("Menu Background   %s", ent->client->prefs.no_menu_background ? "OFF" : "ON"), ChangeConfig, 2, !ent->client->prefs.no_menu_background);
	pCurMenu[i++] = MakeMenuItem("TFPP", "Menu Position", ChangeConfig, 3, ent->client->prefs.menu_pos+1);
	pCurMenu[i++] = MakeMenuItem(NULL);
	pCurMenu[i++] = MakeMenuItem("TFPP", va("AutoProfile       %s", ent->client->prefs.auto_profile ? "ON" : "OFF"), ChangeConfig, 4, !ent->client->prefs.auto_profile);
	pCurMenu[i++] = MakeMenuItem(NULL);

	if (ent->client->curmenu)
		pCurMenu[i++] = MakeMenuItem("TF", "Go back", PreviousMenu);
	pCurMenu[i++] = MakeMenuItem("TF", "Exit", ExitMenu);

	hnd->entries = gi.TagMalloc(sizeof(*hnd->entries)*i, TAG_LEVEL);
	memcpy(hnd->entries, pCurMenu, i*sizeof(*hnd->entries));
	hnd->num = i;
}

void OpenConfigMenu(edict_t *ent, pmenu_t *p) {
	PMenu_Open(ent, ID_CONFIG, p->param, p->arg);
}

void ShowShopMenu(edict_t *ent, pmenuhnd_t *hnd) {
int i = 0;

	pCurMenu[i++] = MakeMenuItem("TS", "  ", "empty_w_tile");
	pCurMenu[i++] = MakeMenuItem("T", "*  DeadLode 2 Armoury");
	pCurMenu[i++] = MakeMenuItem(NULL);
	pCurMenu[i++] = MakeMenuItem(NULL);

	pCurMenu[i++] = MakeMenuItem("T", "Select");
	pCurMenu[i++] = MakeMenuItem("TFP", "  Weapons", OpenWeaponsMenu, 1);
	pCurMenu[i++] = MakeMenuItem("TFP", "  Munitions", OpenAmmoTypesMenu, 1);
	pCurMenu[i++] = MakeMenuItem("TFP", "  Supplies", OpenSuppliesMenu, 1);
	pCurMenu[i++] = MakeMenuItem(NULL);
	pCurMenu[i++] = MakeMenuItem("TF", "  Reset Profile", ResetProfile);
	pCurMenu[i++] = MakeMenuItem(NULL);

	if (ent->client->curmenu)
		pCurMenu[i++] = MakeMenuItem("TF", "Go back", PreviousMenu);
	pCurMenu[i++] = MakeMenuItem("TF", "Exit", ExitMenu);

	hnd->entries = gi.TagMalloc(sizeof(*hnd->entries)*i, TAG_LEVEL);
	memcpy(hnd->entries, pCurMenu, i*sizeof(*hnd->entries));
	hnd->num = i;
}

void ResetProfile(edict_t *ent, pmenu_t *p) {
	DL_Cmd_ClearProfile(ent);
}

void OpenShopMenu(edict_t *ent, pmenu_t *p) {
	PMenu_Open(ent, ID_SHOP, p->param, p->arg);
}

//
// Team Selection
//

void JoinTeam(edict_t *ent, pmenu_t *p) {
// DEADLODE
	if (ent->client->resp.team)
		ent->client->resp.team->players--;

	// Players lose thier buildings when switchign teams...
	ReleaseBuildings(ent);

	ent->client->resp.team = &(dlsys.teams[p->param[0] - 1]);
	ent->client->resp.team->players++;
	ent->team = ent->client->resp.team;
// DEADLODE
	gi.bprintf(PRINT_HIGH, "%s joined team %s.\n",
		ent->client->pers.netname, dlsys.teams[p->param[0]-1].team_name);

	// Spawn player into game, only if profile and team are selected
	if (ent->client->resp.profile[0]) {
		ent->svflags &= ~SVF_NOCLIENT;
		// Close menu system, and start player
		PMenu_CloseAll(ent);

		PutClientInServer (ent);
		// add a teleportation effect
		ent->s.event = EV_PLAYER_TELEPORT;
		// hold in place briefly
		ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
		ent->client->ps.pmove.pm_time = 14;
	} else
		PMenu_Close(ent, true);
}

void ShowTeamSelectionMenu(edict_t *ent, pmenuhnd_t *hnd) {
int		i=0, j=0, t;

	pCurMenu[i++] = MakeMenuItem("TS", "  ", "empty_w_tile");
	pCurMenu[i++] = MakeMenuItem("T", "*  Team Selection");
	pCurMenu[i++] = MakeMenuItem(NULL);
	pCurMenu[i++] = MakeMenuItem(NULL);

	for (t=0; t<dlsys.num_teams; t++) {
		pCurMenu[i++] = MakeMenuItem("TFA", va("Team %s", dlsys.teams[t].team_name), JoinTeam, t+1);
		pCurMenu[i++] = MakeMenuItem("TR", va("*Players: %2d  Score: %3d", dlsys.teams[t].players, dlsys.teams[t].score));
	}
	pCurMenu[i++] = MakeMenuItem(NULL);

	if (ent->client->curmenu)
		pCurMenu[i++] = MakeMenuItem("TF", "Go back", PreviousMenu);
	pCurMenu[i++] = MakeMenuItem("TF", "Exit", ExitMenu);

	hnd->entries = gi.TagMalloc(sizeof(*hnd->entries)*i, TAG_LEVEL);
	memcpy(hnd->entries, pCurMenu, i*sizeof(*hnd->entries));
	hnd->num = i;
}

void OpenTeamSelectionMenu(edict_t *ent, pmenu_t *p) {
	PMenu_Open(ent, ID_JOIN, p->param, p->arg);
}

//
// Personal Objects
//
// team = 0: show personal objects only
// team = 1: show team objects only
// team = 2: show personal and team objects
int ShowObjects(family_t *family, edict_t *ent, int count, edict_t *trig, int xofs, int team, void (*SelectFunc)(edict_t *ent, struct pmenu_s *entry))
{
	int		i;
	object_t *obj;
	char	buf[64];

	if (!family || !family->children)
		return count;

	for (i = 0; obj = family->children[i]; i++)
	{
		if ((obj->owner == ent && (team == 0 || team == 2)) || (team && obj->team_visible))
		{
			Com_sprintf(buf, sizeof(buf), "%s (%s)", obj->name, obj->type_name);
			pCurMenu[count++] = MakeMenuItem("TSBFOPP", buf, obj->pic_s, obj->pic_b, SelectFunc, xofs, obj, trig);
			// xofs += 8;	// indent children only if we are visible
		}
		count = ShowObjects(&obj->family, ent, count, trig, xofs + 8, team, SelectFunc);
	}
	return count;
}

void ShowPersonalMenu(edict_t *ent, pmenuhnd_t *hnd) {
int		i=0, cur=0, j=0;

	pCurMenu[i++] = MakeMenuItem("TS", "  ", "empty_w_tile");
	pCurMenu[i++] = MakeMenuItem("T", "*  Personal Buildings");
	pCurMenu[i++] = MakeMenuItem(NULL);

	i = ShowObjects(&ent->client->resp.team->objects, ent, i, NULL, 0, 0, OpenObjectPropertiesMenu);
	pCurMenu[i++] = MakeMenuItem(NULL);

	if (ent->client->curmenu)
		pCurMenu[i++] = MakeMenuItem("TF", "Go back", PreviousMenu);
	pCurMenu[i++] = MakeMenuItem("TF", "Exit", ExitMenu);

	hnd->entries = gi.TagMalloc(sizeof(*hnd->entries)*i, TAG_LEVEL);
	memcpy(hnd->entries, pCurMenu, i*sizeof(*hnd->entries));
	hnd->num = i;
}

void OpenPersonalMenu(edict_t *ent, pmenu_t *p) {
	PMenu_Open(ent, ID_PERSONALOBJECTS, p->param, p->arg);
}

void ShowObjectPropertiesMenu (edict_t *ent, pmenuhnd_t *hnd) {
object_t *obj;
int		i=0, j=0;

	if (!hnd->param[0])
		return;

	obj = (object_t *)hnd->param[0];
	pCurMenu[i++] = MakeMenuItem("TS", "*  Object properties for", "empty_w_tile");
	pCurMenu[i++] = MakeMenuItem("T", va("*  %s", obj->name));
	pCurMenu[i++] = MakeMenuItem(NULL);
	pCurMenu[i++] = MakeMenuItem(NULL);

//	pCurMenu[i++] = MakeMenuItem("T", va("Name: %s", obj->name));
	pCurMenu[i++] = MakeMenuItem("T", va("Type: %s", obj->type_name));
	pCurMenu[i++] = MakeMenuItem(NULL);

//	if (obj->entity->building && (obj->entity->building->flags & BLDG_ALLOW_PLAYERDESTROY))
		pCurMenu[i++] = MakeMenuItem("TFA", va("Destroy %s", obj->name), DestroyObject, obj);
	pCurMenu[i++] = MakeMenuItem(NULL);

	if (obj->owner == ent)
	{
		if (obj->team_visible)
			pCurMenu[i++] = MakeMenuItem("TFA", "Remove from team objects", ChangeTeamVisible, obj);
		else
			pCurMenu[i++] = MakeMenuItem("TFA", "Add to team objects", ChangeTeamVisible, obj);
		pCurMenu[i++] = MakeMenuItem(NULL);
	}

	if (ent->client->curmenu)
		pCurMenu[i++] = MakeMenuItem("TF", "Go back", PreviousMenu);
	pCurMenu[i++] = MakeMenuItem("TF", "Exit", ExitMenu);

	hnd->entries = gi.TagMalloc(sizeof(*hnd->entries)*i, TAG_LEVEL);
	memcpy(hnd->entries, pCurMenu, i*sizeof(*hnd->entries));
	hnd->num = i;
}

void OpenObjectPropertiesMenu(edict_t *ent, pmenu_t *p) {
	PMenu_Open(ent, ID_OBJECTPROPERTIES, p->param, p->arg);
}

void DestroyObject (edict_t *ent, pmenu_t *p)
{
	object_t *obj;
	int i;

	obj = (object_t *)p->arg;
	PMenu_Close(ent, true);

	if (!obj)
		return;

	if (!obj->entity)
		return;

	if (obj->entity->die)
	{
		obj->entity->die (obj->entity, ent, ent, 0, vec3_origin);
		return;
	}

	G_FreeEdict(obj->entity);

	for (i = 0; i < MAX_TEAMS; i++)
		RemoveFromTeamObjects(FindTeamObject(&dlsys.teams[i].objects, TMOFS(entity), (int)ent));
}

void ChangeTeamVisible (edict_t *ent, pmenu_t *p)
{
	object_t	*obj;
	
	obj = (object_t *)p->arg;
	if (!obj)
		return;

	obj->team_visible ^= true;
}

//
// Team Objects
//

void ShowTeamMenu(edict_t *ent, pmenuhnd_t *hnd) {
int		i=0, cur=0, j=0;
team_t	*team;

	team = ent->client->resp.team;

	pCurMenu[i++] = MakeMenuItem("TS", "  ", "empty_w_tile");
	pCurMenu[i++] = MakeMenuItem("T", "*  Team Buildings");
	pCurMenu[i++] = MakeMenuItem(NULL);
	pCurMenu[i++] = MakeMenuItem(NULL);

	i = ShowObjects(&team->objects, ent, i, NULL, 0, 1, SelectObject_Team);
	
	pCurMenu[i++] = MakeMenuItem(NULL);

	if (ent->client->curmenu)
		pCurMenu[i++] = MakeMenuItem("TF", "Go back", PreviousMenu);
	pCurMenu[i++] = MakeMenuItem("TF", "Exit", ExitMenu);

	hnd->entries = gi.TagMalloc(sizeof(*hnd->entries)*i, TAG_LEVEL);
	memcpy(hnd->entries, pCurMenu, i*sizeof(*hnd->entries));
	hnd->num = i;

}

void OpenTeamMenu(edict_t *ent, pmenu_t *p) {
	PMenu_Open(ent, ID_TEAMOBJECTS, p->param, p->arg);
}

void SelectTeamCamera (edict_t *ent, pmenu_t *p) {
	if (!p->arg)
		return;

	Camera_select(ent, (edict_t *)p->arg);
}

// SelectTeamObject was already taken..
void SelectObject_Team (edict_t *ent, pmenu_t *p) {
	object_t *obj;

	if (!p->arg)
		return;

	obj = (object_t *)p->param[0];
	switch(obj->type)
	{
	case BLDG_CAMERA:
		Camera_select (ent, obj->entity);
		break;
	case BLDG_GUN:
	case BLDG_MOUNT:
	default:
		break;
	}
}

//
// Player/Team Object Selection (mid-game menu, not accessible via main menu)
//
void SelectTeamObject (edict_t *ent, pmenu_t *p) {
edict_t		*trig, *targ;
object_t	*obj;

	obj	= (object_t *)p->param[0];
	trig = (edict_t *)p->param[1];
	PMenu_Close(ent, true);

	if (!trig || !obj)
		return;

	targ = obj->entity;

	if (trig->sensors.parent)
		RemoveChild(&((edict_t *)trig->sensors.parent)->sensors, trig);
	trig->target_ent = targ;
	trig->sensors.parent = obj->entity;
	AddChild(&targ->sensors, trig);
	if (trig->supply)
		gi.cprintf(ent, PRINT_HIGH, "%s set to trigger \"%s\" (%s)\n", trig->supply->name, obj->name, obj->type_name);
}

void ShowSelectObjectsMenu(edict_t *ent, pmenuhnd_t *hnd) {
int		i=0, cur=0, j=0;
team_t	*team;
edict_t	*trig;

	if (!hnd->data)
		return;
	trig = (edict_t *)hnd->data;

	team = ent->client->resp.team;
	if (team->objects.children && team->objects.children[0])
	{
		pCurMenu[i++] = MakeMenuItem("TC", "*Buildings");
		pCurMenu[i++] = MakeMenuItem(NULL);

		i = ShowObjects(&team->objects, ent, i, trig, 0, 2, SelectTeamObject);

		pCurMenu[i++] = MakeMenuItem(NULL);
	} else {
		pCurMenu[i++] = MakeMenuItem("TC", "*No buildings");
		pCurMenu[i++] = MakeMenuItem(NULL);
	}
	
	if (ent->client->curmenu)
		pCurMenu[i++] = MakeMenuItem("TF", "Go back", PreviousMenu);
	pCurMenu[i++] = MakeMenuItem("TF", "Exit", ExitMenu);

	hnd->entries = gi.TagMalloc(sizeof(*hnd->entries)*i, TAG_LEVEL);
	memcpy(hnd->entries, pCurMenu, i*sizeof(*hnd->entries));
	hnd->num = i;
}

void OpenSelectObjectsMenu(edict_t *ent, pmenu_t *p) {
	PMenu_Open(ent, ID_SELECTOBJECTS, p->param, p->arg);
}

//
// INVENTORY
//
void OpenInventoryMenu(edict_t *ent, pmenu_t *p) {
	PMenu_Open(ent, ID_INVENTORY, p->param, p->arg);
}

void ShowInventoryMenu(edict_t *ent, pmenuhnd_t *hnd) {
int numitems, i=0, j=0;
item_t	*curitem;
char *name, *pic_b, *pic_s;
// void (*Func)(edict_t *ent, struct pmenu_s *entry);

	numitems = CountUniqueItems(ent);

	pCurMenu[i++] = MakeMenuItem("TS", "  ", "empty_w_tile");
	pCurMenu[i++] = MakeMenuItem("T", "*  Inventory");
	pCurMenu[i++] = MakeMenuItem(NULL);
	pCurMenu[i++] = MakeMenuItem(NULL);

	pCurMenu[i++] = MakeMenuItem("TC", va("*Inventory Weight: %0.3f", GetInventoryWeight(ent)));
	pCurMenu[i++] = MakeMenuItem(NULL);

	if (hnd->param[0] != ITEM_WEAPON) pCurMenu[i++] = MakeMenuItem("TFP", "Weapons", OpenInventoryMenu, ITEM_WEAPON);
	if (hnd->param[0] != ITEM_AMMO) pCurMenu[i++] = MakeMenuItem("TFP", "Munitions", OpenInventoryMenu, ITEM_AMMO);
	if (hnd->param[0] != ITEM_SUPPLY) pCurMenu[i++] = MakeMenuItem("TFP", "Supplies", OpenInventoryMenu, ITEM_SUPPLY);
	pCurMenu[i++] = MakeMenuItem(NULL);

	for (curitem = ent->inventory; curitem != NULL; curitem=curitem->next) {
		if (hnd->param[0] && (curitem->itemtype != hnd->param[0]))
			continue;

		name = GetItemName(curitem);
		if (curitem->itemtype == ITEM_WEAPON) {
			pic_b = ((weap_t *)(curitem->item))->pic_b;
			pic_s = ((weap_t *)(curitem->item))->pic_s;
		} else if (curitem->itemtype == ITEM_AMMO) {
			pic_b = ((ammo_t *)curitem->item)->pic_b;
			pic_s = ((ammo_t *)curitem->item)->pic_s;
		} else if (curitem->itemtype == ITEM_SUPPLY) {
			pic_b = ((supply_t *)curitem->item)->pic_b;
			pic_s = ((supply_t *)curitem->item)->pic_s;
		} 
		pCurMenu[i++] = MakeMenuItem("TFGASB", va("%3d %s", curitem->quantity, name ? name : "Unknown"), SelectItem, OpenAlternateMenu, curitem, name ? pic_s : NULL, name ? pic_b : NULL);
	}
	pCurMenu[i++] = MakeMenuItem(NULL);

	if (ent->client->curmenu)
		pCurMenu[i++] = MakeMenuItem("TF", "Go back", PreviousMenu);
	pCurMenu[i++] = MakeMenuItem("TF", "Exit", ExitMenu);
	
	hnd->entries = gi.TagMalloc(sizeof(*hnd->entries)*i, TAG_LEVEL);
	memcpy(hnd->entries, pCurMenu, i*sizeof(*hnd->entries));
	hnd->num = i;
}

void SelectItem(edict_t *ent, pmenu_t *p) {
item_t	*item;

	if (!ent || !p || !p->arg)
		return;

	item = p->arg;
	if (!item->quantity) {
		gi.cprintf(ent, PRINT_HIGH, "You don't have any %s's in your inventory.\n", GetItemName(item));
		return;
	}
	switch(item->itemtype) {
	case ITEM_WEAPON:	ent->client->nextweap = item; break;
	case ITEM_AMMO:		ent->client->nextammo = ITEMAMMO(item); break;
	case ITEM_SUPPLY:	Supply_Use(ent, (supply_t *)item->item); break;
	default:			gi.cprintf(ent, PRINT_HIGH, "%s is not a usable item.\n", GetItemName(item));
	}
}

//
// Ammunition
//
void ShowAmmoTypesMenu(edict_t *ent, pmenuhnd_t *hnd) {
int numitems, i=0, unique=0, k;
item_t	*curitem;
char *namelist[MAX_MUNITIONS];

	// clear list
	for (numitems=0; numitems < MAX_MUNITIONS; numitems++)
		namelist[numitems] = NULL;

	if (hnd->param[1] == SHOW_ALL) {
		pCurMenu[i++] = MakeMenuItem("TS", "*  Available DeadLode 2", "ammo/a_generic");
		pCurMenu[i++] = MakeMenuItem("T", "*  munition types");
		pCurMenu[i++] = MakeMenuItem(NULL);
		pCurMenu[i++] = MakeMenuItem(NULL);

		// build list of unique base munitions
		for (numitems=0; numitems < MAX_MUNITIONS && ammo[numitems].base_name; numitems++) {
			for (k = 0; k < unique; k++) {
				if (!DL_strcmp(ammo[numitems].base_name, namelist[k], -1, false))
					break;
			}
			if (DL_strcmp(ammo[numitems].base_name, namelist[k], -1, false)) {
				namelist[unique++] = ammo[numitems].base_name;
				pCurMenu[i++] = MakeMenuItem("TFASBP", ammo[numitems].base_name, OpenAmmoVariantsMenu, ammo[numitems].base_name, ammo[numitems].pic_s, ammo[numitems].pic_b, hnd->param[0]);
			}
		}
		pCurMenu[i++] = MakeMenuItem(NULL);

		pCurMenu[i++] = MakeMenuItem("TFPP", "Show compatible types", OpenAmmoTypesMenu, hnd->param[0], 0);
	} else {
		pCurMenu[i++] = MakeMenuItem("TS", "*  DeadLode 2 compatible", "ammo/a_generic");
		pCurMenu[i++] = MakeMenuItem("T", "*  munition types");
		pCurMenu[i++] = MakeMenuItem(NULL);
		pCurMenu[i++] = MakeMenuItem(NULL);

		// build list of unique base munitions
		for (numitems=0; numitems < MAX_MUNITIONS && ammo[numitems].base_name; numitems++) {
			for (k = 0; k < unique; k++) {
				if (!DL_strcmp(ammo[numitems].base_name, namelist[k], -1, false))
					break;
			}
			if (DL_strcmp(ammo[numitems].base_name, namelist[k], -1, false)) {
				namelist[unique++] = ammo[numitems].base_name;
				// Fixme!  This should use the player profile, not the inventory
				for (curitem = ent->inventory; curitem != NULL; curitem=curitem->next) {
					if (curitem->itemtype == ITEM_WEAPON) {
						if (!DL_strcmp(ITEMWEAP(curitem)->base_ammo, ammo[numitems].base_name, -1, false))
							pCurMenu[i++] = MakeMenuItem("TFASBP", ammo[numitems].base_name, OpenAmmoVariantsMenu, ammo[numitems].base_name, ammo[numitems].pic_s, ammo[numitems].pic_b, hnd->param[0]);
					}
				}
			}
		}

		if (i <= 4) {
			pCurMenu[i++] = MakeMenuItem("TC", "There are no");
			pCurMenu[i++] = MakeMenuItem("TC", "compabitle munitions");
		}
		pCurMenu[i++] = MakeMenuItem(NULL);

		pCurMenu[i++] = MakeMenuItem("TFPP", "Show all types", OpenAmmoTypesMenu, hnd->param[0], SHOW_ALL);
	}

	if (ent->client->curmenu)
		pCurMenu[i++] = MakeMenuItem("TF", "Go back", PreviousMenu);
	pCurMenu[i++] = MakeMenuItem("TF", "Exit", ExitMenu);
	
	hnd->entries = gi.TagMalloc(sizeof(*hnd->entries)*i, TAG_LEVEL);
	memcpy(hnd->entries, pCurMenu, i*sizeof(*hnd->entries));
	hnd->num = i;
}

void OpenAmmoTypesMenu(edict_t *ent, pmenu_t *p) {
	PMenu_Open(ent, ID_AMMOTYPES, p->param, p->arg);
}

void ShowAmmoVariantsMenu(edict_t *ent, pmenuhnd_t *hnd) {
int		i=0, j=0, numitems=0;
char	*ammo_name;

	if (!hnd->data)
		return;
	ammo_name = hnd->data;

	pCurMenu[i++] = MakeMenuItem("TS", "*  Munition variants for", NULL);
	pCurMenu[i++] = MakeMenuItem("T", va("*  %s", ammo_name));
	pCurMenu[i++] = MakeMenuItem(NULL);
	pCurMenu[i++] = MakeMenuItem(NULL);

	for (j=0; j < MAX_MUNITIONS; j++) {
		if (ammo[j].base_name) {
			if (DL_strcmp(ammo[j].base_name, ammo_name, -1, false))
				continue;
			pCurMenu[i++] = MakeMenuItem("TFASBP", ammo[j].name, OpenAmmoStats, &ammo[j], ammo->pic_s, ammo->pic_b, hnd->param[0]);
		}
	}
	pCurMenu[i++] = MakeMenuItem(NULL);

	if (ent->client->curmenu)
		pCurMenu[i++] = MakeMenuItem("TF", "Go back", PreviousMenu);
	pCurMenu[i++] = MakeMenuItem("TF", "Exit", ExitMenu);

	hnd->entries = gi.TagMalloc(sizeof(*hnd->entries)*i, TAG_LEVEL);
	memcpy(hnd->entries, pCurMenu, i*sizeof(*hnd->entries));
	hnd->num = i;
}

void OpenAmmoVariantsMenu(edict_t *ent, pmenu_t *p) {
	PMenu_Open(ent, ID_AMMOVARIANTS, p->param, p->arg);
}

void ShowAmmoStats(edict_t *ent, pmenuhnd_t *hnd) {
int i = 0;
ammo_t	*ammo;

	if (!hnd->data)
		return;
	ammo = (ammo_t *)hnd->data;

	pCurMenu[i++] = MakeMenuItem("TS", "*  Munition stats for", ammo->pic_b);
	pCurMenu[i++] = MakeMenuItem("T", va("*  %s", ammo->name));
	pCurMenu[i++] = MakeMenuItem(NULL);
	pCurMenu[i++] = MakeMenuItem(NULL);

	pCurMenu[i++] = MakeMenuItem("T", va("Type:     %s", ammo->base_name));
	pCurMenu[i++] = MakeMenuItem("T", va("Weight:   %0.1f", (float)ammo->weight));
	pCurMenu[i++] = MakeMenuItem("T", va("Damage Mod: %+d", (int)(ammo->damage)));
	if (ammo->ftype == FT_PELLET) pCurMenu[i++] = MakeMenuItem("T", va("Range Mod:  %+d", (int)(ammo->range)));
	else pCurMenu[i++] = MakeMenuItem("T", va("Speed Mod:  %+d", (int)(ammo->range)));
	pCurMenu[i++] = MakeMenuItem(NULL);

	if (hnd->param[0])
		pCurMenu[i++] = MakeMenuItem("TFASB", "Add to Inventory", OpenAmmoQuantityMenu, ammo, "m_additem_s", "m_additem");
	if (ent->client->curmenu)
		pCurMenu[i++] = MakeMenuItem("TF", "Go back", PreviousMenu);
	pCurMenu[i++] = MakeMenuItem("TF", "Exit", ExitMenu);

	hnd->entries = gi.TagMalloc(sizeof(*hnd->entries)*i, TAG_LEVEL);
	memcpy(hnd->entries, pCurMenu, i*sizeof(*hnd->entries));
	hnd->num = i;
}

void OpenAmmoStats(edict_t *ent, pmenu_t *p) {
	PMenu_Open(ent, ID_AMMOSTATS, p->param, p->arg);
}

void ShowAmmoQuantityMenu(edict_t *ent, pmenuhnd_t *hnd) {
ammo_t *amm;
int		i = 0, tot=0;

	if (!hnd->data)
		return;
	amm = (ammo_t *)hnd->data;

	pCurMenu[i++] = MakeMenuItem("TS", "*  Select ammo quantity for", amm->pic_b);
	pCurMenu[i++] = MakeMenuItem("T", va("*  %s", amm->name));
	pCurMenu[i++] = MakeMenuItem(NULL);
	pCurMenu[i++] = MakeMenuItem(NULL);

	if (ent->client->prefs.IHnd)
		tot = Ini_ReadInteger(ent->client->prefs.IHnd, amm->name, "Quantity", 0); 

	pCurMenu[i++] = MakeMenuItem("T", va("%0.1f Total Weight", GetProfileWeight(ent->client->prefs.IHnd, 1)));
	pCurMenu[i++] = MakeMenuItem("T", va("%3d %s", tot, amm->name ? amm->name : "Unknown"));
	pCurMenu[i++] = MakeMenuItem(NULL);


	if (amm->weight < 0.15)
		pCurMenu[i++] = MakeMenuItem("TFAP", "100 round box", AddAmmoToInventory, hnd->data, tot + 100);
	if (amm->weight < 0.35)
		pCurMenu[i++] = MakeMenuItem("TFAP", " 50 round box", AddAmmoToInventory, hnd->data, tot + 50);
	if (amm->weight < 0.75)
		pCurMenu[i++] = MakeMenuItem("TFAP", " 20 round box", AddAmmoToInventory, hnd->data, tot + 20);
	if (amm->weight < 2.00)
		pCurMenu[i++] = MakeMenuItem("TFAP", "  5 round box", AddAmmoToInventory, hnd->data, tot + 5);
	pCurMenu[i++] = MakeMenuItem("TFAP", "  1 round", AddAmmoToInventory, hnd->data, tot + 1);
	pCurMenu[i++] = MakeMenuItem(NULL);

	if (ent->client->curmenu)
		pCurMenu[i++] = MakeMenuItem("TF", "Go back", PreviousMenu);
	pCurMenu[i++] = MakeMenuItem("TF", "Exit", ExitMenu);

	hnd->entries = gi.TagMalloc(sizeof(*hnd->entries)*i, TAG_LEVEL);
	memcpy(hnd->entries, pCurMenu, i*sizeof(*hnd->entries));
	hnd->num = i;
}

void OpenAmmoQuantityMenu(edict_t *ent, pmenu_t *p) {
	PMenu_Open(ent, ID_AMMOQUANTITY, p->param, p->arg);
}

void AddAmmoToInventory(edict_t *ent, pmenu_t *p) {
//	AddToInventory(ent, ITEM_AMMO, p->arg, 0, p->flags, 0, NULL, NULL);
	AddItemToProfile(ent, ((ammo_t *)(p->arg))->name, ITEM_AMMO, p->param[0], 0, 0, NULL, NULL);
}

//
// Weapons
//
void ShowWeaponsMenu(edict_t *ent, pmenuhnd_t *hnd) {
int		i=0, j=0, k, numCat=0;
char	*categories[32];

	pCurMenu[i++] = MakeMenuItem("TS", "*  Available DeadLode 2", "supplies/i_tools");
	if (hnd->data) pCurMenu[i++] = MakeMenuItem("T", va("*  %s", (char *)hnd->data));
	else pCurMenu[i++] = MakeMenuItem("T", "*  Weapons");
	pCurMenu[i++] = MakeMenuItem(NULL);
	pCurMenu[i++] = MakeMenuItem(NULL);

	numCat = 0;
	categories[0] = NULL;
	for (j=0; j < MAX_WEAPONS; j++) {
		if (!weapons[j].name)
			continue;
		if (weapons[j].flags & WT_NOTUSABLE)
			continue;
		if (DL_strcmp(weapons[j].category, hnd->data, -1, false)) {
			for (k=0; k < numCat; k++)
				if (!DL_strcmp(weapons[j].category, categories[k], -1, false))
					break;
			if (DL_strcmp(weapons[j].category, categories[k], -1, false)) {
				categories[numCat++] = weapons[j].category;
				categories[numCat] = NULL;
			}
			continue;
		}

		pCurMenu[i++] = MakeMenuItem("TFASBP", weapons[j].name, OpenWeaponStats, &weapons[j], weapons[j].pic_s, weapons[j].pic_b, hnd->param[0]);
	}

	pCurMenu[i++] = MakeMenuItem(NULL);
	for (j=0; j<numCat; j++)
		pCurMenu[i++] = MakeMenuItem("TFPA", categories[j], OpenWeaponsMenu, hnd->param[0], categories[j]);

	if (hnd->data)
		pCurMenu[i++] = MakeMenuItem("TFP", "Miscellaneous", OpenWeaponsMenu, hnd->param[0]);
	pCurMenu[i++] = MakeMenuItem(NULL);

	if (ent->client->curmenu)
		pCurMenu[i++] = MakeMenuItem("TF", "Go back", PreviousMenu);
	pCurMenu[i++] = MakeMenuItem("TF", "Exit", ExitMenu);

	hnd->entries = gi.TagMalloc(sizeof(*hnd->entries)*i, TAG_LEVEL);
	memcpy(hnd->entries, pCurMenu, i*sizeof(*hnd->entries));
	hnd->num = i;
}

void OpenWeaponsMenu(edict_t *ent, pmenu_t *p) {
	PMenu_Open(ent, ID_WEAPONS, p->param, p->arg);
}

void ShowWeaponStats(edict_t *ent, pmenuhnd_t *hnd) {
int i = 0;
weap_t	*weap;
char	*other;

	if (!hnd->data)
		return;
	weap = (weap_t *)hnd->data;

	pCurMenu[i++] = MakeMenuItem("TS", "*  Weapon stats for", weap->pic_b);
	pCurMenu[i++] = MakeMenuItem("T", va("*  %s", weap->name));
	pCurMenu[i++] = MakeMenuItem(NULL);
	pCurMenu[i++] = MakeMenuItem(NULL);


	if (weap->category) pCurMenu[i++] = MakeMenuItem("T", va("Type:     %s", weap->category));
	else pCurMenu[i++] = MakeMenuItem("T", "Type:     Miscellaneous");
	pCurMenu[i++] = MakeMenuItem("T", va("Weight:   %0.1f", (float)weap->weight));
	pCurMenu[i++] = MakeMenuItem("T", va("Ammo:      %s", weap->base_ammo));
	pCurMenu[i++] = MakeMenuItem("T", va("Damage:    %d", (int)(weap->damage_mod)));
	if (ammo->ftype == FT_PELLET) pCurMenu[i++] = MakeMenuItem("T", va("Range:     %d", (int)(weap->range_mod)));
	else pCurMenu[i++] = MakeMenuItem("T", va("Speed:     %d", (int)(weap->range_mod)));
	pCurMenu[i++] = MakeMenuItem("T", va("Accuracy:  %d", (int)(weap->accuracy)));

	if (weap->size <= 0.01) other = "Stability: Excellent";
	else if (weap->size <= 0.05) other = "Stability: Good";
	else if (weap->size <= 0.5) other = "Stability: Average";
	else other = "Stability: Poor";
	pCurMenu[i++] = MakeMenuItem("T", other);
	pCurMenu[i++] = MakeMenuItem(NULL);

	if (!(weap->flags & WT_NOAMMO)) {
		pCurMenu[i++] = MakeMenuItem("TFA", "View compatible munitions", OpenAmmoVariantsMenu, weap->base_ammo);
		pCurMenu[i++] = MakeMenuItem(NULL);
	}
	if (hnd->param[0])
		pCurMenu[i++] = MakeMenuItem("TFASB", "Add to Inventory", AddWeaponToInventory, weap, "m_additem_s", "m_additem");

	if (ent->client->curmenu)
		pCurMenu[i++] = MakeMenuItem("TF", "Go back", PreviousMenu);
	pCurMenu[i++] = MakeMenuItem("TF", "Exit", ExitMenu);

	hnd->entries = gi.TagMalloc(sizeof(*hnd->entries)*i, TAG_LEVEL);
	memcpy(hnd->entries, pCurMenu, i*sizeof(*hnd->entries));
	hnd->num = i;
}

void OpenWeaponStats(edict_t *ent, pmenu_t *p) {
	PMenu_Open(ent, ID_WEAPONSTATS, p->param, p->arg);
}

void AddWeaponToInventory(edict_t *ent, pmenu_t *p) {
weap_t	*weap;
item_t	*item;
int		num = 0;

	weap = ((weap_t *)(p->arg));

	if (weap->use_count) {
		item = FindItemByName(ent, weap->name, ITEM_WEAPON);
		if (item) num = item->quantity + 1;
	}

	AddItemToProfile(ent, weap->name, ITEM_WEAPON, 0, 0, 0, NULL, NULL);
	PMenu_Close(ent, false);

	if (!(weap->flags & WT_NOAMMO)) {
		p->param[0] = 1; // setting param[0] = 1 allows the player to add ammo from this screen.
		PMenu_Open(ent, ID_AMMOVARIANTS, p->param, weap->base_ammo);
	}
}

//
// Supplies
//
void ShowSuppliesMenu(edict_t *ent, pmenuhnd_t *hnd) {
int		i=0, j=0, k, numCat=0;
char	*categories[32];

	pCurMenu[i++] = MakeMenuItem("TS", "*  Available DeadLode 2", "supplies/i_tools");
	if (hnd->data) pCurMenu[i++] = MakeMenuItem("T", va("*  %s", (char *)hnd->data));
	else pCurMenu[i++] = MakeMenuItem("T", "*  Supplies and Equipment");
	pCurMenu[i++] = MakeMenuItem(NULL);
	pCurMenu[i++] = MakeMenuItem(NULL);

	numCat = 0;
	categories[0] = NULL;
	for (j=0; j < MAX_SUPPLIES; j++) {
		if (!supplies[j].name)
			continue;
		if (supplies[j].type & SUP_GOAL)
			continue;
		if (DL_strcmp(supplies[j].category, hnd->data, -1, false)) {
			for (k=0; k < numCat; k++)
				if (!DL_strcmp(supplies[j].category, categories[k], -1, false))
					break;
			if (DL_strcmp(supplies[j].category, categories[k], -1, false)) {
				categories[numCat++] = supplies[j].category;
				categories[numCat] = NULL;
			}
			continue;
		}

		pCurMenu[i++] = MakeMenuItem("TFASBP", supplies[j].name, OpenSupplyStats, &supplies[j], supplies[j].pic_s, supplies[j].pic_b, hnd->param[0]);
	}

	pCurMenu[i++] = MakeMenuItem(NULL);
	for (j=0; j<numCat; j++)
		pCurMenu[i++] = MakeMenuItem("TFPA", categories[j], OpenSuppliesMenu, hnd->param[0], categories[j]);

	if (hnd->data)
		pCurMenu[i++] = MakeMenuItem("TFP", "Miscellaneous", OpenSuppliesMenu, hnd->param[0]);
	pCurMenu[i++] = MakeMenuItem(NULL);

	if (ent->client->curmenu)
		pCurMenu[i++] = MakeMenuItem("TF", "Go back", PreviousMenu);
	pCurMenu[i++] = MakeMenuItem("TF", "Exit", ExitMenu);

	hnd->entries = gi.TagMalloc(sizeof(*hnd->entries)*i, TAG_LEVEL);
	memcpy(hnd->entries, pCurMenu, i*sizeof(*hnd->entries));
	hnd->num = i;
}

void OpenSuppliesMenu(edict_t *ent, pmenu_t *p) {
	PMenu_Open(ent, ID_SUPPLIES, p->param, p->arg);
}

void ShowSupplyStats(edict_t *ent, pmenuhnd_t *hnd) {
supply_t	*sup;
int			i = 0;

	if (!hnd->data)
		return;
	sup = (supply_t *)hnd->data;

	pCurMenu[i++] = MakeMenuItem("TS", "*  Supply stats for", sup->pic_b);
	pCurMenu[i++] = MakeMenuItem("T", va("*  %s", sup->name));
	pCurMenu[i++] = MakeMenuItem(NULL);
	pCurMenu[i++] = MakeMenuItem(NULL);

	// pCurMenu[i++] = MakeMenuItem("T", va("Name:     %s", sup->name));
	if (sup->category)	pCurMenu[i++] = MakeMenuItem("T", va("Type:     %s", sup->category));
	else pCurMenu[i++] = MakeMenuItem("T", "Type:     Miscellaneous");
	pCurMenu[i++] = MakeMenuItem("T", va("Weight:   %0.1f", (float)(sup->weight)));
	pCurMenu[i++] = MakeMenuItem(NULL);

	if (hnd->param[0]) {
		if (sup->type & (SUP_ARMOUR|SUP_FLASHLIGHT|SUP_TOOL))
			pCurMenu[i++] = MakeMenuItem("TFASB", "Add to Inventory", OpenSupplyQuantityMenu, sup, "m_additem_s", "m_additem");
		else
			pCurMenu[i++] = MakeMenuItem("TFASB", "Add to Inventory", AddSupplyToInventory, sup, "m_additem_s", "m_additem");
	}
	if (ent->client->curmenu)
		pCurMenu[i++] = MakeMenuItem("TF", "Go back", PreviousMenu);
	pCurMenu[i++] = MakeMenuItem("TF", "Exit", ExitMenu);

	hnd->entries = gi.TagMalloc(sizeof(*hnd->entries)*i, TAG_LEVEL);
	memcpy(hnd->entries, pCurMenu, i*sizeof(*hnd->entries));
	hnd->num = i;
}

void OpenSupplyStats(edict_t *ent, pmenu_t *p) {
	PMenu_Open(ent, ID_SUPPLYSTATS, p->param, p->arg);
}

void ShowSupplyQuantityMenu(edict_t *ent, pmenuhnd_t *hnd) {
supply_t	*sup;
int		i = 0, tot = 0;

	if (!hnd->data)
		return;
	sup = (supply_t *)hnd->data;

	pCurMenu[i++] = MakeMenuItem("TS", "*  Select supply quantity", sup->pic_b);
	pCurMenu[i++] = MakeMenuItem("T", va("*  for %s", sup->name ? sup->name : "Unknown"));
	pCurMenu[i++] = MakeMenuItem(NULL);
	pCurMenu[i++] = MakeMenuItem(NULL);

	if (ent->client->prefs.IHnd)
		tot = Ini_ReadInteger(ent->client->prefs.IHnd, sup->name, "Quantity", 0); 

	pCurMenu[i++] = MakeMenuItem("T", va("%0.1f Total Weight", GetProfileWeight(ent->client->prefs.IHnd, 1)));
	pCurMenu[i++] = MakeMenuItem("T", va("%3d %s", tot, sup->name ? sup->name : "Unknown"));
	pCurMenu[i++] = MakeMenuItem(NULL);

	if (sup->weight < 0.15)
		pCurMenu[i++] = MakeMenuItem("TFAP", "100 units", AddSupplyToInventory, hnd->data, tot+100);
	if (sup->weight < 0.35)
		pCurMenu[i++] = MakeMenuItem("TFAP", " 50 units", AddSupplyToInventory, hnd->data, tot+50);
	if (sup->weight < 0.75)
		pCurMenu[i++] = MakeMenuItem("TFAP", " 20 units", AddSupplyToInventory, hnd->data, tot+20);
	if (sup->weight < 2.00)
		pCurMenu[i++] = MakeMenuItem("TFAP", "  5 units", AddSupplyToInventory, hnd->data, tot+5);
	pCurMenu[i++] = MakeMenuItem("TFAP", "  1 unit", AddSupplyToInventory, hnd->data, tot+1);
	pCurMenu[i++] = MakeMenuItem(NULL);

	if (ent->client->curmenu)
		pCurMenu[i++] = MakeMenuItem("TF", "Go back", PreviousMenu);
	pCurMenu[i++] = MakeMenuItem("TF", "Exit", ExitMenu);

	hnd->entries = gi.TagMalloc(sizeof(*hnd->entries)*i, TAG_LEVEL);
	memcpy(hnd->entries, pCurMenu, i*sizeof(*hnd->entries));
	hnd->num = i;
}

void OpenSupplyQuantityMenu(edict_t *ent, pmenu_t *p) {
	PMenu_Open(ent, ID_SUPPLYQUANTITY, p->param, p->arg);
}

void AddSupplyToInventory(edict_t *ent, pmenu_t *p) {
int tot;
	if (!p->param[0]) {
		tot = Ini_ReadInteger(ent->client->prefs.IHnd, ((supply_t *)(p->arg))->name, "Quantity", 0); 
		AddItemToProfile(ent, ((supply_t *)(p->arg))->name, ITEM_SUPPLY, tot + 1, 0, 0, NULL, NULL);
		PMenu_Close(ent, true);
	} else
		AddItemToProfile(ent, ((supply_t *)(p->arg))->name, ITEM_SUPPLY, p->param[0], 0, 0, NULL, NULL);
}

//
// System Profiles
//
void ShowProfilesMenu(edict_t *ent, pmenuhnd_t *hnd) {
int		numitems, j=0, i=0;
char	**entries;


	numitems = Ini_NumberOfEntries(dlsys.sysIni, "Profiles");

	pCurMenu[i++] = MakeMenuItem("TS", "  ", "empty_w_tile");
	pCurMenu[i++] = MakeMenuItem("T", "*  System Profiles");
	pCurMenu[i++] = MakeMenuItem(NULL);
	pCurMenu[i++] = MakeMenuItem(NULL);
	if (numitems == -1) {
		pCurMenu[i++] = MakeMenuItem(NULL);
		pCurMenu[i++] = MakeMenuItem("TC", "No Profiles Defined");
		pCurMenu[i++] = MakeMenuItem(NULL);
	} else {
		if (dlsys.player_profiles) {
			//	sprintf(filename, "profiles/players/%s.ini", ValidateProfileName(ent->client->pers.netname, pName));
			//	if (DL_FileExist(filename)) {
			if (ent->client->prefs.IHnd) {
				pCurMenu[i++] = MakeMenuItem("TFP", "Player Profile", SelectSystemProfile, 1);
				pCurMenu[i++] = MakeMenuItem(NULL);
			}
		}
		entries = Ini_SectionEntries(dlsys.sysIni, "Profiles");
		for (j=0; j<numitems; j++)
			if (DL_strcmp(Ini_ReadString(dlsys.sysIni, "Profiles", entries[j], NULL), "on", -1, false))
				pCurMenu[i++] = MakeMenuItem("T", entries[j]);
			else
				pCurMenu[i++] = MakeMenuItem("TF", entries[j], SelectSystemProfile);
	}
	pCurMenu[i++] = MakeMenuItem(NULL);

	if (ent->client->curmenu)
		pCurMenu[i++] = MakeMenuItem("TF", "Go back", PreviousMenu);
	pCurMenu[i++] = MakeMenuItem("TF", "Exit", ExitMenu);

	hnd->entries = gi.TagMalloc(sizeof(*hnd->entries)*i, TAG_LEVEL);
	memcpy(hnd->entries, pCurMenu, i*sizeof(*hnd->entries));
	hnd->num = i;

}

void OpenProfilesMenu(edict_t *ent, pmenu_t *p) {
	PMenu_Open(ent, ID_PROFILES, p->param, p->arg);
}

void SelectSystemProfile(edict_t *ent, pmenu_t *p) {
int	param;

	if (!p || !p->text)
		return;

	param = p->param[0];

	if (!ent->client) {
		gi.dprintf("Non-client tried to select a profile\n");
		return;
	}

	if (!param)
		DL_strcpy(ent->client->resp.next_profile, p->text, 31);
	else
		memset(ent->client->resp.next_profile, 255, 32);

	// Only spawn in player if they are finished with profile/team selections
	if (!(ent->movetype != MOVETYPE_NOCLIP && ent->deadflag != DEAD_DEAD)) {
		// make this profile change immediate
		DL_strcpy(ent->client->resp.profile, ent->client->resp.next_profile, -1);
		if (!param)
			gi.bprintf(PRINT_HIGH, "%s is playing as a %s.\n", ent->client->pers.netname, ent->client->resp.profile);
		else
			gi.bprintf(PRINT_HIGH, "%s is playing with his custom profile.\n", ent->client->pers.netname);

		// don't spawn in, if player hasn't selected a team yet
		if (!ent->client->resp.team)
			return;

		// Close menu system, and start player
		PMenu_CloseAll(ent);

		PutClientInServer (ent);
		// add a teleportation effect
		ent->s.event = EV_PLAYER_TELEPORT;
		// hold in place briefly
		ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
		ent->client->ps.pmove.pm_time = 14;
	} else {
		if (!param) // && *ent->client->resp.next_profile && (*ent->client->resp.next_profile != 255))
			gi.cprintf(ent, PRINT_HIGH, "You will play as a %s, when you respawn next.\n", ent->client->resp.next_profile);
		else
			gi.cprintf(ent, PRINT_HIGH, "You will play using your personal profile, when you respawn next.\n");
		PMenu_Close(ent, true);
	}
}

void SelectRadioMessage(edict_t *ent, pmenu_t *p) {
	if (!ent || !p)
		return;

	Radio_Use(ent, (char *)p->arg);
}

void ShowRadioMenu(edict_t *ent, pmenuhnd_t *hnd) {
char	*file, **entries;
int		numitems, j=0, i=0;

	numitems = Ini_NumberOfEntries(dlsys.sysIni, "Radio Messages");

	pCurMenu[i++] = MakeMenuItem(NULL);
	pCurMenu[i++] = MakeMenuItem("TS", "*  Radio Messages", "empty_w_tile");
	pCurMenu[i++] = MakeMenuItem(NULL);
	pCurMenu[i++] = MakeMenuItem(NULL);
	if (numitems == -1) {
		pCurMenu[i++] = MakeMenuItem(NULL);
		pCurMenu[i++] = MakeMenuItem("TC", "No Radio Messages Defined");
		pCurMenu[i++] = MakeMenuItem(NULL);
	} else {
		entries = Ini_SectionEntries(dlsys.sysIni, "Radio Messages");
		for (j=0; j<numitems; j++) {
			file = Ini_ReadString(dlsys.sysIni, "Radio Messages", entries[j], NULL);
			pCurMenu[i++] = MakeMenuItem("TFA", entries[j], SelectRadioMessage, file);
		}	
	}
	pCurMenu[i++] = MakeMenuItem(NULL);

	if (ent->client->curmenu)
		pCurMenu[i++] = MakeMenuItem("TF", "Go back", PreviousMenu);
	pCurMenu[i++] = MakeMenuItem("TF", "Exit", ExitMenu);

	hnd->entries = gi.TagMalloc(sizeof(*hnd->entries)*i, TAG_LEVEL);
	memcpy(hnd->entries, pCurMenu, i*sizeof(*hnd->entries));
	hnd->num = i;
}

void OpenRadioMenu(edict_t *ent, pmenu_t *p) {
	PMenu_Open(ent, ID_RADIOMESSAGES, p->param, p->arg);
}

void OpenBuildMenu(edict_t *ent, pmenu_t *p) {
	PMenu_Open(ent, ID_CONSTRUCTION, p->param, p->arg);
}

void ShowBuildMenu(edict_t *ent, pmenuhnd_t *hnd) {
int			i=0, j=0, k, numCat=0, tools=0, ttypes=0;
char		*categories[32];
item_t		*item;
supply_t	*tool;

	if (!ent || ent->deadflag)
		return;

	// Check the player for the required tools
	for (item = ent->inventory; item; item = item->next) {
		tool = ITEMSUPPLY(item);
		if ((item->itemtype == ITEM_SUPPLY) && (tool->type == SUP_TOOL)) {
			tools |= tool->flags;
			ttypes |= tool->tool;
		}
	}

	pCurMenu[i++] = MakeMenuItem("TS", "*  Available DeadLode 2", "supplies/i_tools");
	if (hnd->data) pCurMenu[i++] = MakeMenuItem("T", va("*  %s", (char *)hnd->data));
	else pCurMenu[i++] = MakeMenuItem("T", "*  Buildings");
	pCurMenu[i++] = MakeMenuItem(NULL);
	pCurMenu[i++] = MakeMenuItem(NULL);
/*
	pCurMenu[i++] = MakeMenuItem("TC", "For the purposed of debugging");
	pCurMenu[i++] = MakeMenuItem("TC", "building has been temporalily");
	pCurMenu[i++] = MakeMenuItem("TC", "disabled on this server.");
	pCurMenu[i++] = MakeMenuItem(NULL);
	pCurMenu[i++] = MakeMenuItem("TC", "Sorry for the Inconvinence.");
	pCurMenu[i++] = MakeMenuItem("TC", "Hang Tight though, it'll be");
	pCurMenu[i++] = MakeMenuItem("TC", "back soon!");

*/
	numCat = 0;
	categories[0] = NULL;
	for (j=0; j < MAX_BUILDINGS; j++) {
		if (!buildings[j].name)
			continue;
		if (CanBuildBuilding(ent, &buildings[j]))
			continue;
		if (DL_strcmp(buildings[j].category, hnd->data, -1, false)) {
			for (k=0; k < numCat; k++)
				if (!DL_strcmp(buildings[j].category, categories[k], -1, false))
					break;
			if (DL_strcmp(buildings[j].category, categories[k], -1, false)) {
				categories[numCat++] = buildings[j].category;
				categories[numCat] = NULL;
			}
			continue;
		}

		pCurMenu[i++] = MakeMenuItem("TFAP", buildings[j].name, SelectBuilding, &buildings[j], hnd->param[0]);
	}

	pCurMenu[i++] = MakeMenuItem(NULL);
	for (j=0; j<numCat; j++)
		pCurMenu[i++] = MakeMenuItem("TFPA", categories[j], OpenBuildMenu, hnd->param[0], categories[j]);

	if (hnd->data)
		pCurMenu[i++] = MakeMenuItem("TFP", "Miscellaneous", OpenBuildMenu, hnd->param[0]);
	pCurMenu[i++] = MakeMenuItem(NULL);

	if (ent->client->curmenu)
		pCurMenu[i++] = MakeMenuItem("TF", "Go back", PreviousMenu);
	pCurMenu[i++] = MakeMenuItem("TF", "Exit", ExitMenu);

	hnd->entries = gi.TagMalloc(sizeof(*hnd->entries)*i, TAG_LEVEL);
	memcpy(hnd->entries, pCurMenu, i*sizeof(*hnd->entries));
	hnd->num = i;
}

void ShowBuildingMenu(edict_t *ent, pmenuhnd_t *hnd) {
char		*stateName[4] = {"Disabled","Offline","Online", "Active"};
int			i=0, j=0, cur=0, pos;
qboolean	nextto = false;
edict_t		*bldg, *child;
float		dist;
vec3_t		diff;

	if (!ent || ent->deadflag || !hnd || !hnd->data)
		return;

	// specified ent is not a building, or building was destroyed
	// Close menu, and return gracefully
	bldg = (edict_t *)hnd->data;
	if (!bldg->building) {
		PMenu_Close(ent, true);
		return;
	}

	VectorSubtract(ent->s.origin, bldg->s.origin, diff);
	dist = VectorLength(diff);
	if (dist < 64) nextto = true;
//	pos = InFOV(ent->s.origin, ent->s.angles, bldg->s.origin, 90, true);
//	if (pos && pos < 64)
//		nextto = true;

	pCurMenu[i++] = MakeMenuItem("TS", "*  Maintenance menu for", "supplies/i_tools");
	pCurMenu[i++] = MakeMenuItem("T", va("*  %s", bldg->building->name));
	pCurMenu[i++] = MakeMenuItem(NULL);
	pCurMenu[i++] = MakeMenuItem(NULL);


	pCurMenu[i++] = MakeMenuItem("T", va("Health: %3d%% (%d)", (int)(((float)bldg->health / (float)bldg->building->min_points) * 100.0), bldg->health));
	pCurMenu[i++] = MakeMenuItem("T", va("Status: %s", stateName[bldg->state]));
	pCurMenu[i++] = MakeMenuItem(NULL);

	if (nextto) {
		pCurMenu[i++] = MakeMenuItem("TFAP", "Repair", Menu_BuildingRepair, bldg, 1);

		if (bldg->use)
			pCurMenu[i++] = MakeMenuItem("TFA", "Use", Menu_BuildingUse, bldg);

		if ((bldg->building->type & BLDG_MOUNT) && (bldg->building->flags & BLDG_ALLOW_PLAYERROTATE))
			pCurMenu[i++] = MakeMenuItem("TFA", "Rotate", Menu_BuildingRotate, bldg);

		// pCurMenu[i++] = MakeMenuItem("TFA", "Dismantle", Menu_BuildingRepair, bldg, 1);

		if (bldg->master == ent)
			pCurMenu[i++] = MakeMenuItem("TFA", "Destroy", Menu_BuildingDestroy, bldg);
	}
	pCurMenu[i++] = MakeMenuItem(NULL);

	if (bldg->family.parent && ((edict_t *)bldg->family.parent)->building)
		pCurMenu[i++] = MakeMenuItem("TFA", "Select parent", OpenBuildingMenu, bldg->family.parent);

//	if (bldg->goalentity && bldg->goalentity->building)
//		pCurMenu[i++] = MakeMenuItem("TFA", "Select child", OpenBuildingMenu, bldg->goalentity);
	if (bldg->family.children && bldg->family.children[0])
	{
		pCurMenu[i++] = MakeMenuItem(NULL);
		pCurMenu[i++] = MakeMenuItem("T", "*Attachments:");
		for (j = 0; child = bldg->family.children[j]; j++)
		{
			if (!child->building)
				continue;

			pCurMenu[i++] = MakeMenuItem("TOFA", child->building->name, FONT_WIDTH, OpenBuildingMenu, child);
		}
	}
	if (bldg->sensors.children && bldg->sensors.children[0])
	{
		pCurMenu[i++] = MakeMenuItem(NULL);
		pCurMenu[i++] = MakeMenuItem("T", "*Sensors:");
		for (j = 0; child = bldg->sensors.children[j]; j++)
		{
			if (!child->supply)
				continue;

			pCurMenu[i++] = MakeMenuItem("TOFA", child->supply->name, FONT_WIDTH, OpenTriggerMenu, child);
		}
	}


	pCurMenu[i++] = MakeMenuItem(NULL);
	if (ent->client->curmenu)
		pCurMenu[i++] = MakeMenuItem("TF", "Go back", PreviousMenu);
	pCurMenu[i++] = MakeMenuItem("TF", "Exit", ExitMenu);

	pos = FindMenu(ent, ID_BUILDINGINFO);
	if (pos >= 0) cur = ent->client->menu[pos]->cur;

	hnd->entries = gi.TagMalloc(sizeof(*hnd->entries)*i, TAG_LEVEL);
	memcpy(hnd->entries, pCurMenu, i*sizeof(*hnd->entries));
	hnd->num = i;
}

void OpenBuildingMenu(edict_t *ent, pmenu_t *p) {
	PMenu_Open(ent, ID_BUILDINGINFO, p->param, p->arg);
}

void Menu_BuildingRepair(edict_t *ent, pmenu_t *p) {
edict_t *bldg, *tbldg;
item_t	*item;
supply_t *supl;
vec3_t	start;

	// Fixme: make repair time-based, given the amount
	// of damage the building has taken
	// Repair Time is: (buildtime * 2) * (maxhealth / health)
	// ie: repairs take 2x as long as to build a new one.
	// A building at 50% health takes same time to repair
	// as it would to build a new one...  BUT, building
	// a new one may require more time in attachments, supplies, etc
	if (!p || !p->arg)
		return;

	bldg = (edict_t *)p->arg;
	if (!bldg->building)
		return;

	VectorCopy(ent->s.origin, start);
	start[2] += ent->viewheight;
	tbldg = FindInFOV(start, ent->client->v_angle, 90, 64);
	if (tbldg != bldg)
		return;

	// Find a suitable tool to repair building with
	for (item = ent->inventory, supl = NULL; item; item = item->next) {
		supl = ITEMSUPPLY(item);
		if ((item->itemtype == ITEM_SUPPLY) && (supl->type == SUP_TOOL))
			if ((supl->tool & TOOL_REPAIR) && (supl->flags & bldg->building->req_tools))
				break;
		supl = NULL;
	}

	Tool_Use(ent, supl);
}

// Rotating a mount only changes rotation angles, not the actual mount.
// Activating this causes the mount/turret to center in on the players
// viewpoint, aiming at the players target.
void Menu_BuildingRotate(edict_t *ent, pmenu_t *p) {
edict_t *bldg;
int i;
vec3_t	offset;

	if (!p || !p->arg)
		return;

	bldg = (edict_t *)p->arg;
//	PMenu_Close(ent, true); 

	if (!bldg->building)
		return;

	VectorSubtract(ent->client->ps.viewangles, bldg->s.angles, offset);
	VectorSubtract(offset, bldg->building->attach[0].angles, offset);
//	AnglesMod(offset, offset);

	for (i = 0; i < MAX_BLDG_ATTACH; i++) {
		if (bldg->attach[i])
			VectorSubtract(offset, bldg->attach[i]->ideal_angles, bldg->attach[i]->rotate_offset);
	}
}

void Menu_BuildingDestroy(edict_t *ent, pmenu_t *p) {
edict_t *bldg;

	if (!p || !p->arg)
		return;

	bldg = (edict_t *)p->arg;
	PMenu_Close(ent, true); 

	if (!bldg->building)
		return;

	if (bldg->die)
		bldg->die(bldg, ent, ent, 0, vec3_origin);
}

void Menu_BuildingUse(edict_t *ent, pmenu_t *p) {
edict_t *bldg;

	if (!p || !p->arg)
		return;

	bldg = (edict_t *)p->arg;
	if (!bldg->building)
		return;

	if (bldg->use) {
		// Release the previously used item
		if (ent->use_this)
			ent->use_this->use_this = NULL;
		ent->use_this = bldg;
		bldg->use(bldg, ent, ent);
	}
}

void OpenTriggerMenu(edict_t *ent, pmenu_t *p)
{
	PMenu_Open(ent, ID_TRIGGERINFO, p->param, p->arg);
}

void ShowTriggerMenu(edict_t *ent, pmenuhnd_t *hnd)
{
	edict_t	*trig;
	int		i = 0, pos;
	qboolean nextto;
	char	*status;

	if (!hnd->data)
		return;

	trig = hnd->data;
	if (!trig->supply)
	{
		PMenu_Close(ent, true);
		return;
	}

	pos = InFOV(ent->s.origin, ent->s.angles, trig->s.origin, 90, true);
	nextto = false;
	if (pos && pos < 64)
		nextto = true;

	pCurMenu[i++] = MakeMenuItem("TS", "*  Properties for", trig->supply->pic_b);
	pCurMenu[i++] = MakeMenuItem("T", va("*  %s", trig->supply->name));
	pCurMenu[i++] = MakeMenuItem(NULL);
	pCurMenu[i++] = MakeMenuItem(NULL);

	pCurMenu[i++] = MakeMenuItem("T", va("Health: %3d", trig->health));

	if (trig->supply->type == SUP_TIMER)
		status = va("Time remaining: %d", (int)(trig->timestamp - level.time));
	else if (trig->touch_debounce_time > level.time)
		status = "Status: Active";
	else if (trig->flash_time > level.time)
		status = "Status: Disabled";
	else if (trig->target_ent)
		status = "Status: Online";
	else
		status = "Status: Offline";
	pCurMenu[i++] = MakeMenuItem("T", status);
	pCurMenu[i++] = MakeMenuItem(NULL);

	if (nextto && (trig->master == ent)) {
//		if (trig->use)
//			pCurMenu[i++] = MakeMenuItem("TFA", "Use", Menu_TriggerUse, trig);

		pCurMenu[i++] = MakeMenuItem("TFA", "Reclaim", Menu_TriggerReclaim, trig);
		pCurMenu[i++] = MakeMenuItem("TFA", "Destroy", Menu_TriggerDestroy, trig);
		pCurMenu[i++] = MakeMenuItem("TFA", "Change target", OpenSelectObjectsMenu, trig);
		pCurMenu[i++] = MakeMenuItem(NULL);
	}
	
	if (trig->target_ent && trig->target_ent->building)
	{
		pCurMenu[i++] = MakeMenuItem("TFA", va("Target: %s", trig->target_ent->building->name), OpenBuildingMenu, trig->target_ent);
		pCurMenu[i++] = MakeMenuItem(NULL);
	}
	if (ent->client->curmenu)
		pCurMenu[i++] = MakeMenuItem("TF", "Go back", PreviousMenu);
	pCurMenu[i++] = MakeMenuItem("TF", "Exit", ExitMenu);

	hnd->entries = gi.TagMalloc(sizeof(*hnd->entries)*i, TAG_LEVEL);
	memcpy(hnd->entries, pCurMenu, i*sizeof(*hnd->entries));
	hnd->num = i;
}

void Menu_TriggerReclaim(edict_t *ent, pmenu_t *p)
{
	edict_t	*trig;

	if (!p || !p->arg)
		return;

	trig = (edict_t *)p->arg;
	PMenu_Close(ent, true);

	if (!trig->supply)
		return;

	ReclaimTrigger(ent, trig);
}

void Menu_TriggerDestroy(edict_t *ent, pmenu_t *p)
{
	edict_t *trig;

	if (!p || !p->arg)
		return;

	trig = (edict_t *)p->arg;
	PMenu_Close(ent, true);

	if (!trig->supply)
		return;

	if (trig->die)
		trig->die(trig, ent, ent, 0, vec3_origin);
	else
		G_FreeEdict(trig);
}


void VoteForNextMap(edict_t *ent, pmenu_t *p) {
	if (!ent || !p)
		return;

	ent->client->prefs.mapvote = p->arg;
	if (!level.intermissiontime)
		PMenu_CloseAll(ent);
}

void ShowMapVoteMenu(edict_t *ent, pmenuhnd_t *hnd) {
char	*desc, **entries;
int		numitems, j=0, i=0, sec;

	pCurMenu[i++] = MakeMenuItem("TS", "*  DeadLode 2 available", "empty_w_tile");
	pCurMenu[i++] = MakeMenuItem("T", "*  server maps");
	pCurMenu[i++] = MakeMenuItem(NULL);
	pCurMenu[i++] = MakeMenuItem(NULL);

	if ((level.time - level.intermissiontime > 0.2)) {
		sec = (level.intermissiontime + 21) - level.time;
		if (sec > 0) {
			pCurMenu[i++] = MakeMenuItem("TC", va("%d seconds to vote", sec));
			pCurMenu[i++] = MakeMenuItem(NULL);
		}
	}

	numitems = Ini_NumberOfEntries(dlsys.sysIni, "Vote Maps");
	if (numitems == -1) {
		pCurMenu[i++] = MakeMenuItem(NULL);
		pCurMenu[i++] = MakeMenuItem("TC", "No Maps Defined");
		pCurMenu[i++] = MakeMenuItem(NULL);
	} else {
		entries = Ini_SectionEntries(dlsys.sysIni, "Vote Maps");
		for (j=0; j<numitems; j++) {
			desc = Ini_ReadString(dlsys.sysIni, "Vote Maps", entries[j], NULL);
			pCurMenu[i++] = MakeMenuItem("TFA", va("%-20s %2d", desc, DL_GetMapVotes(entries[j])), VoteForNextMap, entries[j]);
		}	
	}
	pCurMenu[i++] = MakeMenuItem(NULL);

	if (ent->client->curmenu)
		pCurMenu[i++] = MakeMenuItem("TF", "Go back", PreviousMenu);
	pCurMenu[i++] = MakeMenuItem("TF", "Exit", ExitMenu);

	hnd->entries = gi.TagMalloc(sizeof(*hnd->entries)*i, TAG_LEVEL);
	memcpy(hnd->entries, pCurMenu, i*sizeof(*hnd->entries));
	hnd->num = i;
}

void OpenMapVoteMenu(edict_t *ent, pmenu_t *p) {
	if (!dlsys.map_voting)
		return;

	PMenu_Open(ent, ID_MAPVOTING, p->param, p->arg);
}

// Uniform Selection Menus
void Menu_SelectUniform(edict_t *ent, pmenu_t *p) {

	// Players lose thier buildings when switching teams...
	ent->team = &(dlsys.teams[p->param[0] - 1]);
	DL_AssignSkin(ent, Info_ValueForKey(ent->client->pers.userinfo, "skin"));
	if (ent->team != ent->client->resp.team) {
		gi.cprintf(ent, PRINT_HIGH, "Your wearing an enemy uniform.\n");
		RemoveInventoryItem(ent, FindInventoryItem(ent, (supply_t *)(p->arg)), 1); 
	} else
		gi.cprintf(ent, PRINT_HIGH, "You wearing your standard uniform.\n");
}

void ShowUniformSelectionMenu(edict_t *ent, pmenuhnd_t *hnd) {
int		i=0, j=0, t;

	pCurMenu[i++] = MakeMenuItem("TS", "*  DeadLode 2 available", ((supply_t *)(hnd->data))->pic_b);
	pCurMenu[i++] = MakeMenuItem("T", "*  uniform selections");
	pCurMenu[i++] = MakeMenuItem(NULL);
	pCurMenu[i++] = MakeMenuItem(NULL);

	for (t=0; t<dlsys.num_teams; t++)
		pCurMenu[i++] = MakeMenuItem("TFPA", va("Team %s", dlsys.teams[t].team_name), Menu_SelectUniform, t+1, hnd->data);
	pCurMenu[i++] = MakeMenuItem(NULL);

	if (ent->client->curmenu)
		pCurMenu[i++] = MakeMenuItem("TF", "Go back", PreviousMenu);
	pCurMenu[i++] = MakeMenuItem("TF", "Exit", ExitMenu);

	hnd->entries = gi.TagMalloc(sizeof(*hnd->entries)*i, TAG_LEVEL);
	memcpy(hnd->entries, pCurMenu, i*sizeof(*hnd->entries));
	hnd->num = i;
}

void OpenPlayerStatsMenu(edict_t *ent, pmenu_t *p) {
	PMenu_Open(ent, ID_STATS, p->param, p->arg);
}

void ShowPlayerStatsMenu(edict_t *ent, pmenuhnd_t *hnd) {
int		i=0, j=0;
int		hr, min, sec;

	pCurMenu[i++] = MakeMenuItem("TS", "*  DeadLode 2 Player", "empty_w_tile");
	pCurMenu[i++] = MakeMenuItem("T", "*  Statistics");
	pCurMenu[i++] = MakeMenuItem(NULL);
	pCurMenu[i++] = MakeMenuItem(NULL);

	pCurMenu[i++] = MakeMenuItem("T", va("Total Kills:        %d", ent->client->prefs.stats.total_kills));
	pCurMenu[i++] = MakeMenuItem("T", va("Total Deaths:       %d", ent->client->prefs.stats.total_deaths));
	pCurMenu[i++] = MakeMenuItem("T", va("Total Suicides:     %d", ent->client->prefs.stats.total_suicides));
	pCurMenu[i++] = MakeMenuItem("T", va("Carriers Killed:    %d", ent->client->prefs.stats.carriers_killed));
	pCurMenu[i++] = MakeMenuItem("T", va("Chasers Killed:     %d", ent->client->prefs.stats.chasers_killed));
	pCurMenu[i++] = MakeMenuItem("T", va("Games Played:       %d", ent->client->prefs.stats.games));

	sec = (int)(ent->client->prefs.stats.game_time) % 60;
	min = ((int)(ent->client->prefs.stats.game_time) / 60) % 60;
	hr = ((int)(ent->client->prefs.stats.game_time) / 3600) % 60;
	pCurMenu[i++] = MakeMenuItem("T", va("Total Game Time: %d:%02d:%02d", hr, min, sec));

	sec = (int)(ent->client->prefs.stats.carry_time) % 60;
	min = ((int)(ent->client->prefs.stats.carry_time) / 60) % 60;
	hr = ((int)(ent->client->prefs.stats.carry_time) / 3600) % 60;
	pCurMenu[i++] = MakeMenuItem("T", va("Carry Time:      %d:%02d:%02d", hr, min, sec));

/*
	sprintf(tStats[j], "Empty Time:         %0.1f", ent->client->pers.stats.empty_time);
	sprintf(tStats[j], "Total Damage:       %0.1f", ent->client->pers.stats.total_damage);
	sprintf(tStats[j], "Total Skill:        %0.1f", ent->client->pers.stats.total_skill);

	sprintf(tStats[j], "Defensive Skill:    %0.1f", ent->client->pers.stats.defensive_skill);
	sprintf(tStats[j], "Offensive Skill:    %0.1f", ent->client->pers.stats.offensive_skill);
	sprintf(tStats[j], "Role Potential:     %0.1f", ent->client->pers.stats.role_potential);
	sprintf(tStats[j], "Role Effectiveness: %0.1f", ent->client->pers.stats.role_effectiveness);
	pCurMenu[i++] = MakeMenuItem(tStats[j++], NULL, NULL, PMENU_ALIGN_LEFT, NULL, NULL, 0, 0, 0);
*/
	pCurMenu[i++] = MakeMenuItem(NULL);

	if (ent->client->curmenu)
		pCurMenu[i++] = MakeMenuItem("TF", "Go back", PreviousMenu);
	pCurMenu[i++] = MakeMenuItem("TF", "Exit", ExitMenu);

	hnd->entries = gi.TagMalloc(sizeof(*hnd->entries)*i, TAG_LEVEL);
	memcpy(hnd->entries, pCurMenu, i*sizeof(*hnd->entries));
	hnd->num = i;
}

void OpenAlternateMenu(edict_t *ent, pmenu_t *p) {
	PMenu_Open(ent, ID_ALTMENU, p->param, p->arg);
}

void AltDropItem(edict_t *ent, pmenu_t *p) {
item_t *item;

	if (!p->arg)
		return;
	item = (item_t *)p->arg;
	if (RemoveInventoryItem(ent, item, p->param[0]) != item)
		PMenu_Close(ent, true);
}

void AltRemoveItem(edict_t *ent, pmenu_t *p) {
item_t *item;
supply_t *supply = NULL;
ammo_t	*ammo = NULL;
weap_t	*weap = NULL;
char *name;
int num, cur;

	if (!p->arg)
		return;

	item = (item_t *)p->arg;
	if (item->itemtype == ITEM_WEAPON) {
		weap = (weap_t *)item->item;
		name = weap->name;
	} else if (item->itemtype == ITEM_AMMO) {
		ammo = (ammo_t *)item->item;
		name = ammo->name;
	} else if (item->itemtype == ITEM_SUPPLY) {
		supply = (supply_t *)item->item;
		name = supply->name;
	} else
		return;

	num = cur = Ini_ReadInteger(ent->client->prefs.IHnd, name, "Quantity", 1);
	if (p->param[0] > 2)
		num = p->param[0];

	if (num < cur)
		Ini_WriteInteger(ent->client->prefs.IHnd, name, "Quantity", cur - num);
	else
		Ini_RemoveSection(ent->client->prefs.IHnd, name);

	PMenu_Close(ent, true);
}

void ShowAlternateMenu(edict_t *ent, pmenuhnd_t *hnd) {
item_t	*item;
supply_t *supply = NULL;
ammo_t	*ammo = NULL;
weap_t	*weap = NULL;
int		i = 0;
char	*name, *pic, *cat, *other;
float	weight;

	if (!hnd->data)
		return;
	item = (item_t *)hnd->data;
	if (item->itemtype == ITEM_WEAPON) {
		weap = (weap_t *)item->item;
		name = weap->name;
		pic = weap->pic_b;
		cat = weap->category;
		weight = weap->weight;
	} else if (item->itemtype == ITEM_AMMO) {
		ammo = (ammo_t *)item->item;
		name = ammo->name;
		pic = ammo->pic_b;
		cat = ammo->base_name;
		weight = ammo->weight;
	} else if (item->itemtype == ITEM_SUPPLY) {
		supply = (supply_t *)item->item;
		name = supply->name;
		pic = supply->pic_b;
		cat = supply->category;
		weight = supply->weight;
	} else
		return;

	pCurMenu[i++] = MakeMenuItem("TS", "*  Secondary info for", pic);
	pCurMenu[i++] = MakeMenuItem("T", va("*  %s", name));
	pCurMenu[i++] = MakeMenuItem(NULL);
	pCurMenu[i++] = MakeMenuItem(NULL);

	if (cat)	pCurMenu[i++] = MakeMenuItem("T", va("Type:       %s", cat));
	else pCurMenu[i++] = MakeMenuItem("T", "Type:       Miscellaneous");
	pCurMenu[i++] = MakeMenuItem("T", va("Weight:     %0.1f", (float)weight));

	if (weap) {
		pCurMenu[i++] = MakeMenuItem("T", va("Ammo:       %s", weap->base_ammo));
		pCurMenu[i++] = MakeMenuItem("T", va("Damage:     %d", (int)(weap->damage_mod)));
		pCurMenu[i++] = MakeMenuItem("T", va("Range:      %d", (int)(weap->range_mod)));
		pCurMenu[i++] = MakeMenuItem("T", va("Accuracy:   %d", (int)(weap->accuracy)));

		if (weap->size <= 0.01) other = "Stability:  Excellent";
		else if (weap->size <= 0.05) other = "Stability:  Good";
		else if (weap->size <= 0.5) other = "Stability:  Average";
		else other = "Stability:  Poor";
		pCurMenu[i++] = MakeMenuItem("T", other);
		pCurMenu[i++] = MakeMenuItem(NULL);
		pCurMenu[i++] = MakeMenuItem("TFA", "Add to Profile", AddWeaponToInventory, weap);
	} else if (ammo) {
		pCurMenu[i++] = MakeMenuItem("T", va("Damage Mod: %+d", (int)(ammo->damage)));
		if (ammo->ftype == FT_PELLET) pCurMenu[i++] = MakeMenuItem("T", va("Range Mod:  %+d", (int)(ammo->range)));
		else pCurMenu[i++] = MakeMenuItem("T", va("Speed Mod:  %+d", (int)(ammo->range)));
		pCurMenu[i++] = MakeMenuItem(NULL);
		pCurMenu[i++] = MakeMenuItem("TFA", "Add to Profile", OpenAmmoQuantityMenu, ammo);
	} else {
		pCurMenu[i++] = MakeMenuItem("TFA", "Add to Profile", OpenSupplyQuantityMenu, supply);
	}

	if (item->quantity) {
		pCurMenu[i++] = MakeMenuItem("TFAP", "Drop Item (single)", AltDropItem, item, 1);
		if (item->quantity > 1)
			pCurMenu[i++] = MakeMenuItem("TFAP", "Discard (all)", AltDropItem, item, item->quantity);
		pCurMenu[i++] = MakeMenuItem("TFA", "Remove from Profile", AltRemoveItem, item);
	}

	if (ent->client->curmenu)
		pCurMenu[i++] = MakeMenuItem("TF", "Go back", PreviousMenu);
	pCurMenu[i++] = MakeMenuItem("TF", "Exit", ExitMenu);

	hnd->entries = gi.TagMalloc(sizeof(*hnd->entries)*i, TAG_LEVEL);
	memcpy(hnd->entries, pCurMenu, i*sizeof(*hnd->entries));
	hnd->num = i;
}

void OpenCreditsMenu(edict_t *ent, pmenu_t *p) {
	p->param[0] = level.framenum;
	PMenu_Open(ent, ID_CREDITS, p->param, p->arg);
}

void ShowCreditsMenu(edict_t *ent, pmenuhnd_t *hnd) {
int		i=0, j=0, pos, numCredits = 60;
char *credits[] = {
"",
"",
"",
"",
"",
"",
"",
"",
"*Programming",
"  Kyle Mallory",
"  Conor Davis",
"",
"*Modeling",
"  Andrew Eglington",
"  Kyle Mallory",
"  Alistair Briggs",
"  John Sheffield",
"  James Edwards",
"  Colin Anderson",
"  Eric Ngai",
"",
"*Skins & Artwork",
"  Jake Bielanski",
"  Andrew Eglington",
"",
"*Sound Effects",
"  Eric von Rothkirch",
"  Kyle Mallory",
"  Andrew Eglington",
"  Dan Morley &",
"    NoiseCraft Studios",
"",
"*Maps",
"  John Bye",
"  Owen Marlowe",
"  Anders Gustavson",
"  Jaan-Madis Uusvali",
"  Ken Nichols",
"",
"*Map Textures",
"  David Ottval",
"  Daniel Nolan",
"  Anders Gustavson",
"",
"*Beta Testers",
"  Bryan 'Devnull' Klimek",
"  James 'Ven'Tatsu' Morgan",
"  Jean Pierre 'Optimus' Smith",
"  Patrick 'PM' Martin",
"  Richard 'Andromni' Martin",
"  Scott 'Sloath' de Petro",
"  Everyone's Friend 'NiHiLiST'",
"  Jim 'Sgt. Thundercok' Kaufman",
"",
"*Special Thanks",
"  DeadLode.com",
"  Id Software",
"",
"  All dedicated DeadLode",
"  fans across the globe."
};

	pCurMenu[i++] = MakeMenuItem("TS", "*  DeadLode 2 Game", "empty_w_tile");
	pCurMenu[i++] = MakeMenuItem("T", "*  Credits");
	pCurMenu[i++] = MakeMenuItem(NULL);
	pCurMenu[i++] = MakeMenuItem(NULL);

	pos = ((level.framenum - hnd->param[0]) / 3) % numCredits;

	for (j=0; j<9; j++)
		pCurMenu[i++] = MakeMenuItem("T", credits[(pos + j) % numCredits]);

	pCurMenu[i++] = MakeMenuItem(NULL);
	if (ent->client->curmenu)
		pCurMenu[i++] = MakeMenuItem("TF", "Go back", PreviousMenu);
	pCurMenu[i++] = MakeMenuItem("TF", "Exit", ExitMenu);

	hnd->entries = gi.TagMalloc(sizeof(*hnd->entries)*i, TAG_LEVEL);
	memcpy(hnd->entries, pCurMenu, i*sizeof(*hnd->entries));
	hnd->num = i;
}


void OpenMOTDMenu(edict_t *ent, pmenu_t *p) {
	p->param[0] = level.framenum;
	PMenu_Open(ent, ID_MOTD, p->param, p->arg);
}

void ShowMOTDMenu(edict_t *ent, pmenuhnd_t *hnd) {
int		i=0, j=0, pos = 0;

	pCurMenu[i++] = MakeMenuItem("TS", "*  DeadLode 2 Game", "empty_w_tile");
	pCurMenu[i++] = MakeMenuItem("T", "*  Message Of The Day");
	pCurMenu[i++] = MakeMenuItem(NULL);
	pCurMenu[i++] = MakeMenuItem(NULL);

	if (numMOTDLines) {
		if (numMOTDLines > 16)
			pos = ((level.framenum - hnd->param[0]) / 4) % numMOTDLines;
		else
			pos = 8;

		for (j=0; j<8; j++)
			pCurMenu[i++] = MakeMenuItem("T", motdLines[(pos + j) % numMOTDLines]);
	} else {
		pCurMenu[i++] = MakeMenuItem("TC", "No Message Of The Day");
	}

	pCurMenu[i++] = MakeMenuItem(NULL);
	if (ent->client->curmenu)
		pCurMenu[i++] = MakeMenuItem("TF", "Go back", PreviousMenu);
	pCurMenu[i++] = MakeMenuItem("TF", "Exit", ExitMenu);

	hnd->entries = gi.TagMalloc(sizeof(*hnd->entries)*i, TAG_LEVEL);
	memcpy(hnd->entries, pCurMenu, i*sizeof(*hnd->entries));
	hnd->num = i;
}
