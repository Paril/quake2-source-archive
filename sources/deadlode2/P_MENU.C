#include "g_local.h"

int FindMenu (edict_t *ent, int id)
{
	int i;
	for (i = 0; i < MENU_STACK_SIZE; i++)
		if (ent->client->menu[i] && ent->client->menu[i]->id == id)
			return i;
	return -1;
}

void FreeEntry (pmenu_t *p)
{
	if (p->text) {
		gi.TagFree(p->text);
		p->text = NULL;
	}
	if (p->target) {
		gi.TagFree(p->target);
		p->target = NULL;
	}
	if (p->pic_s) {
		gi.TagFree(p->pic_s);
		p->pic_s = NULL;
	}
	if (p->pic_b) {
		gi.TagFree(p->pic_b);
		p->pic_b = NULL;
	}

	memset(p, 0, sizeof(pmenu_t));
}

void PMenu_Open(edict_t *ent, int id, int *param, void *data)
{
	pmenuhnd_t *hnd;
	int oldstart=0, oldcur, i;

	if (!ent->client)
		return;

	// Check for stack space
	if ((ent->client->curmenu >= MENU_STACK_SIZE-1)
		&& ent->client->menu[ent->client->curmenu]) {
		gi.cprintf(ent, PRINT_HIGH, "WARNING: Menu out of stack space.  Unable to display.\n");
		return;
	}

	hnd = ent->client->menu[ent->client->curmenu];
	if (hnd)
	{
		// If stack position is occupied, and new menu is not the same, increment the stack pointer 
		if (hnd->id != id)
			ent->client->curmenu++;
		else	// otherwise free the old menu data
		{
			if (hnd->entries)
			{
				for (i = 0; i < hnd->num; i++)
					FreeEntry(hnd->entries+i);
				gi.TagFree(hnd->entries);
			}
			gi.TagFree(hnd);
		}
	}


	hnd = gi.TagMalloc(sizeof(*hnd), TAG_LEVEL);
	memset(hnd, 0, sizeof(*hnd));
	hnd->oldstart = 0;
	hnd->id = id;
//	if (param)
//		memcpy(&(hnd->param), param, sizeof(int) * 5);
	if (param)
		memcpy(hnd->param, param, sizeof(hnd->param));
	hnd->data = data;
	oldcur = hnd->cur;
	hnd->cur = -1;
	switch (id)
	{
	case ID_JOIN:
		hnd->UpdateFunc = ShowTeamSelectionMenu;
		break;
	case ID_MAIN:
		hnd->UpdateFunc = ShowMainMenu;
		break;
	case ID_PLAYER:
		hnd->UpdateFunc = ShowPlayerMenu;
		break;
	case ID_CONFIG:
		hnd->UpdateFunc = ShowConfigMenu;
		break;
	case ID_INVENTORY:
		hnd->UpdateFunc = ShowInventoryMenu;
		break;
	case ID_SHOP:
		hnd->UpdateFunc = ShowShopMenu;
		break;
	case ID_WEAPONS:
		hnd->UpdateFunc = ShowWeaponsMenu;
		break;
	case ID_WEAPONSTATS:
		hnd->UpdateFunc = ShowWeaponStats;
		break;
	case ID_AMMOTYPES:
		hnd->UpdateFunc = ShowAmmoTypesMenu;
		break;
	case ID_AMMOVARIANTS:
		hnd->UpdateFunc = ShowAmmoVariantsMenu;
		break;
	case ID_AMMOSTATS:
		hnd->UpdateFunc = ShowAmmoStats;
		break;
	case ID_AMMOQUANTITY:
		hnd->UpdateFunc = ShowAmmoQuantityMenu;
		break;
	case ID_SUPPLIES:
		hnd->UpdateFunc = ShowSuppliesMenu;
		break;
	case ID_SUPPLYSTATS:
		hnd->UpdateFunc = ShowSupplyStats;
		break;
	case ID_SUPPLYQUANTITY:
		hnd->UpdateFunc = ShowSupplyQuantityMenu;
		break;
	case ID_PERSONALOBJECTS:
		hnd->UpdateFunc = ShowPersonalMenu;
		break;
	case ID_TEAMOBJECTS:
		hnd->UpdateFunc = ShowTeamMenu;
		break;
	case ID_SELECTOBJECTS:
		hnd->UpdateFunc = ShowSelectObjectsMenu;
		break;
	case ID_OBJECTPROPERTIES:
		hnd->UpdateFunc = ShowObjectPropertiesMenu;
		break;
	case ID_PROFILES:
		hnd->UpdateFunc = ShowProfilesMenu;
		break;
	case ID_RADIOMESSAGES:
		hnd->UpdateFunc = ShowRadioMenu;
		break;
	case ID_BUILDINGINFO:
		hnd->UpdateFunc = ShowBuildingMenu;
		break;
	case ID_TRIGGERINFO:
		hnd->UpdateFunc = ShowTriggerMenu;
		break;
	case ID_CONSTRUCTION:
		hnd->UpdateFunc = ShowBuildMenu;
		break;
	case ID_MAPVOTING:
		hnd->UpdateFunc = ShowMapVoteMenu;
		break;
	case ID_SELECTUNIFORM:
		hnd->UpdateFunc = ShowUniformSelectionMenu;
		break;
	case ID_STATS:
		hnd->UpdateFunc = ShowPlayerStatsMenu;
		break;
	case ID_ALTMENU:
		hnd->UpdateFunc = ShowAlternateMenu;
		break;
	case ID_CREDITS:
		hnd->UpdateFunc = ShowCreditsMenu;
		break;
	case ID_MOTD:
		hnd->UpdateFunc = ShowMOTDMenu;
		break;
	default:
		hnd->UpdateFunc = ShowTargetMenu;
	}
	ent->client->menu[ent->client->curmenu] = hnd;
	PMenu_Update(ent);
}

void PMenu_Close(edict_t *ent, qboolean show_lower)
{
	int i;
	pmenuhnd_t *hnd;

	if (!ent->inuse || !ent->client->pers.connected)
		return;

	hnd = ent->client->menu[ent->client->curmenu];
	if (!hnd)
		return;

	if (hnd->entries) {
		for (i = 0; i < hnd->num; i++)
			FreeEntry(hnd->entries+i);
		gi.TagFree(hnd->entries);
	}
	gi.TagFree(hnd);

	ent->client->menu[ent->client->curmenu] = NULL;

	if (ent->client->curmenu > 0) {
		ent->client->curmenu--;
		if (show_lower && ent->client->menu[ent->client->curmenu])
			PMenu_Update(ent);
	}
}

void PMenu_CloseAll(edict_t *ent)
{
	while (ent->client->menu[ent->client->curmenu])
		PMenu_Close(ent, false);
}

void PMenu_Update(edict_t *ent)
{
	char string[1400];
	int i, start, end;		// i is the current entry number
							// start is the first non-header entry to display
							// end is the last non-head entry to display
	int num=0, cnt=0, ln=0; // num is the number of selectable lines
							// cnt is the number of non-header/trailer lines
							// ln is the actual line to draw the menu item at
	pmenu_t *p;
	int x, xoff;
	pmenuhnd_t *hnd;
	char *t, *xloc;
	qboolean alt = false;

	if (ent->menu_debounce_time > level.time)
		return;

	if (!ent->client->menu[ent->client->curmenu]) {
		gi.dprintf("PMenu_update:  ent has no menu\n");
		return;
	}
	
	hnd = ent->client->menu[ent->client->curmenu];
	// dynamic menus have changing menu items,
	// so completely refresh the menu every update
	if (hnd)
	{
		// Free old menu items
		for (i = 0; i < hnd->num; i++)
			FreeEntry(&hnd->entries[i]);
		if (hnd->entries) {
			gi.TagFree(hnd->entries);
			hnd->entries = NULL;
		}
		// Get new menu items
		hnd->UpdateFunc(ent, hnd);
		hnd = ent->client->menu[ent->client->curmenu];
		// Are there any entries for this menu?
		if (!hnd)
			return;
		if (!hnd->entries) {
			PMenu_Close(ent, true);
			return;
		}
	}
	if (hnd->cur - hnd->oldstart > 14)
		start = hnd->cur - 14;
	else if (hnd->cur - hnd->oldstart < 2)
		start = max(hnd->cur - 2, 0);
	else
		start = hnd->oldstart;
	if (start < 0) start = 0;
	hnd->oldstart = start;
	end = min(start + 16, hnd->num);

	memset(string, 0, 1400);
	if (!ent->client->prefs.no_menu_background)
		if (ent->client->prefs.inmenu) {
			switch (ent->client->prefs.menu_pos & MENU_POS_VERTICAL) {
			case MENU_POS_TOP:	sprintf(string + strlen(string), "yt 8 "); break;
			default:			sprintf(string + strlen(string), "yv 8 "); break;
			}
			switch (ent->client->prefs.menu_pos & MENU_POS_HORIZONTAL) {
			case MENU_POS_LEFT:		sprintf(string + strlen(string), "xl 32 "); break;
			case MENU_POS_RIGHT:	sprintf(string + strlen(string), "xr -288 "); break;
			default:				sprintf(string + strlen(string), "xv 32 "); break;
			}

			sprintf(string + strlen(string), "picn inventory ");
		}

	//for (i = 0, p = hnd->entries; i < hnd->num; i++, p++) {
	for (i = start, p = hnd->entries+start; i < end; i++, p++) {
		if (!p->text || !*(p->text))
			continue; // blank line

		t = p->text;
		if (*t == '*') {
			alt = true;
			t++;
		}
		if (hnd->cur < 0 && (p->SelectFunc || p->target))
			hnd->cur = i;

		//DEADLODE - Menu Off (displays in lower right of screen as a single line)
		if (ent->client->prefs.inmenu == false) {
			if (hnd->cur != i)
				continue;
			else
			{
//				ent->client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex(p->pic_b);
				x = strlen(t)*8;
				if (x > 152) x = 152;
				sprintf(string, "xr -%d yb -36 string \"%.19s\" ", x, t);
				if (DL_strcmp(string, ent->client->oldmenu, -1, true)) {
					DL_strcpy(ent->client->oldmenu, string, 1400);
					gi.WriteByte (svc_layout);
					gi.WriteString (string);
					gi.unicast (ent, true);
				}
				ent->menu_debounce_time = level.time + (ent->client->ping / 250);
				return;
			}
		//DEADLODE
		}

		switch (ent->client->prefs.menu_pos & MENU_POS_VERTICAL) {
		case MENU_POS_TOP:	sprintf(string + strlen(string), "yt %d ", 32 + (i-start) * 8); break;
		default:			sprintf(string + strlen(string), "yv %d ", 32 + (i-start) * 8); break;
		}
				
		if (p->align == PMENU_ALIGN_CENTER)
			x = 196/2 - strlen(t)*4 + 48;
		else if (p->align == PMENU_ALIGN_RIGHT)
			x = 48 + (196 - strlen(t)*8);
		else
			x = 48;
		x += p->xofs;

		switch (ent->client->prefs.menu_pos & MENU_POS_HORIZONTAL) {
		case MENU_POS_LEFT:		xloc = "xl", xoff = 0; break;
		case MENU_POS_RIGHT:	xloc = "xr", xoff = -288; break;
		default:				xloc = "xv", xoff = 0; break;
		}

		if ((p->SelectFunc || p->target) && num < 10)
		{
			num++;
			sprintf(string + strlen(string), "%s %d ", xloc, x + xoff);
			if (hnd->cur == i)
				sprintf(string + strlen(string), "string2 \"%d.\x0d %s\" ", num, t);
			else if (alt)
				sprintf(string + strlen(string), "string2 \"%d.  %s\" ", num, t);
			else
				sprintf(string + strlen(string), "string \"%d.  %s\" ", num < 10 ? num : 0, t);
			if (p->pic_s)
				sprintf (string + strlen(string), "%s %d picn %s ", xloc, (x+FONT_WIDTH*3)+xoff, p->pic_s);
		}
		else
		{
			x += FONT_WIDTH*2;
			sprintf(string + strlen(string), "%s %d ", xloc, (x - ((hnd->cur == i) ? 8 : 0))+xoff);
			if (hnd->cur == i)
				sprintf(string + strlen(string), "string2 \"\x0d %s\" ", t);
			else if (alt)
				sprintf(string + strlen(string), "string2 \" %s\" ", t);
			else
				sprintf(string + strlen(string), "string \" %s\" ", t);
			if (p->pic_s)
				sprintf (string + strlen(string), "%s %d picn %s ", xloc, x+xoff, p->pic_s);
		}
		alt = false;
	}

	if (DL_strcmp(string, ent->client->oldmenu, -1, true)) {
		DL_strcpy(ent->client->oldmenu, string, 1400);
		gi.WriteByte (svc_layout);
		gi.WriteString (string);
		gi.unicast (ent, true);
	}
	ent->menu_debounce_time = level.time + (ent->client->ping / 250);
}

void PMenu_Next(edict_t *ent)
{
	pmenuhnd_t *hnd;
	int i;
	pmenu_t *p;

	if (!ent->client->menu[ent->client->curmenu]) {
		gi.dprintf("PMenu_next:  ent has no menu\n");
		return;
	}

	hnd = ent->client->menu[ent->client->curmenu];

	if (hnd->cur < 0)
		return; // no selectable entries

	i = hnd->cur;
	p = hnd->entries + hnd->cur;
	do {
		i++, p++;
		if (i >= hnd->num)
			i = 0, p = hnd->entries;
		if (p->SelectFunc || p->target)
			break;
	} while (i != hnd->cur);

	hnd->cur = i;

//	PMenu_Update(ent);
}

void PMenu_Prev(edict_t *ent)
{
	pmenuhnd_t *hnd;
	int i;
	pmenu_t *p;

	if (!ent->client->menu[ent->client->curmenu]) {
		gi.dprintf("PMenu_Prev:  ent has no menu\n");
		return;
	}

	hnd = ent->client->menu[ent->client->curmenu];

	if (hnd->cur < 0)
		return; // no selectable entries

	i = hnd->cur;
	p = hnd->entries + hnd->cur;
	do {
		if (i <= 0) {
			i = hnd->num - 1;
			p = hnd->entries + i;
		} else
			i--, p--;
		if (p->SelectFunc || p->target)
			break;
	} while (i != hnd->cur);

	hnd->cur = i;

//	PMenu_Update(ent);
}

void PMenu_Select(edict_t *ent)
{
	pmenuhnd_t *hnd;
	pmenu_t *p;

	if (!ent->client->menu[ent->client->curmenu]) {
		gi.dprintf("PMenu_Select:  ent has no menu\n");
		return;
	}

	hnd = ent->client->menu[ent->client->curmenu];

	if (hnd->cur < 0)
		return; // no selectable entries

	p = hnd->entries + hnd->cur;
	if (p->SelectFunc)
		p->SelectFunc(ent, p);
	else if (p->target) {
		((edict_t *)(hnd->data))->target = p->target;
		G_UseTargets(hnd->data, ent);
	}
}

void PMenu_AltSelect(edict_t *ent)
{
	pmenuhnd_t *hnd;
	pmenu_t *p;

	if (!ent->client->menu[ent->client->curmenu]) {
		gi.dprintf("PMenu_Select:  ent has no menu\n");
		return;
	}

	hnd = ent->client->menu[ent->client->curmenu];

	if (hnd->cur < 0)
		return; // no selectable entries

	p = hnd->entries + hnd->cur;
	if (p->AltSelectFunc)
		p->AltSelectFunc(ent, p);
}

