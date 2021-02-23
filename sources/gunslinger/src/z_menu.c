#include "g_local.h"

void spectator_respawn (edict_t *ent);
/*
typedef struct menu_s {
	char			*string;
	int				align;
	int				value;
	qboolean		choosable;
	struct	menu_s	*next;
} menu_t;

typedef struct player_menu_s {
	qboolean	showmenu;
	qboolean	update;
	void		(*create_menu)(edict_t *ent);
	menu_t		*start;
	menu_t		*end;
	int			selection;
	int			total;
	qboolean	transmitted;
	void		(*usr_menu_sel)(edict_t *ent, int choice);
} player_menu_t;
*/

void RPS_MenuOpen(edict_t *ent, void (*create_menu)(edict_t *ent), void (*usr_menu_sel)(edict_t *ent, int choice), qboolean update, int delay) {
	player_menu_t	*pm=&ent->client->menu_data;
	pm->showmenu=true;
	pm->update=update;
	if (pm->start) {
		RPS_MenuFree(ent);
	}
	pm->create_menu=create_menu;
	pm->selection=-1;
	pm->total=0;
	pm->usr_menu_sel=usr_menu_sel;
	if (delay) {
		pm->displayframe=level.framenum+delay;
	} else {
		RPS_MenuUpdate(ent);
		gi.unicast (ent, true);
	}
}

void RPS_MenuAdd(edict_t *ent, char *string, int align, int value, qboolean choice) {
	player_menu_t	*pm=&ent->client->menu_data;
	menu_t			*here;

	here=malloc(sizeof(*here));
	here->string=malloc(strlen(string)+5);
	strcpy(here->string, string);
	here->align=align;
	here->value=value;
	here->next=NULL;

	if (pm->start) {
		pm->end->next=here;
		pm->end=here;
	} else {
		pm->start=here;
		pm->end=here;
	}

	here->choosable=choice;
	if (choice) {
		pm->total++;
		// If this is the first choice, it is the default
		if (pm->selection==-1)
			pm->selection=0;
	}
}

void RPS_MenuTransmit(edict_t *ent) {
	// Menu stats: width=28, height=19
	player_menu_t	*pm=&ent->client->menu_data;
	menu_t			*here=pm->start;
	char			data[MAXSTRLEN];
	int				i=-1, x=0, y=26;
	char			tempstr[256];

//	debugmsg("Transmitting\n");
	if (!here) {
		RPS_MenuClose(ent);
		return;
	}

	sprintf(data, "xv 32 yv 8 picn inventory ");
	while (here) {
		if (here->align & ALIGN_LEFT) {
			x=48;
			if (here->choosable)
				x+=8;
		} else if (here->align & ALIGN_CENTER) {
			x=160-(strlen(here->string)*4);
		} else if (here->align &ALIGN_RIGHT) {
			x=272-(strlen(here->string)*8);
			if (here->choosable)
				x-=8;
		}
		strcpy(tempstr,here->string);
		if (here->choosable) {
			i++;
			if (pm->selection == i) {
				debugmsg("Current choice is %i\n", here->value);
				RPS_AddToString(data, va("xv 48 yv %i string2 \">\" xv 264 string2 \"<\" ", y), MAXSTRLEN);
			} else {
				RPS_ColorText(tempstr, false);
			}
		} else {
			RPS_ColorText(tempstr, false);
		}
//		if (here->align & ALIGN_TITLE)
//			RPS_AddToString(data, va("xv %i yv 10 string \"%s\" ", x, here->string), MAXSTRLEN);
//		else {
		RPS_AddToString(data, va("xv %i yv %i string \"%s\" ", x, y, tempstr), MAXSTRLEN);
		y+=8;
//		}
		if (here->align & HALF_SPACE)
			y+=4;
		if (here->align & FULL_SPACE)
			y+=8;
		if (here->align & NO_SPACE)
			y-=8;
		here=here->next;
	}

	ent->client->showscores = true;
	ent->client->showinventory = false;
	ent->client->menu_data.showmenu = true;
	pm->transmitted=true;

	gi.WriteByte (svc_layout);
	gi.WriteString (data);
//	debugmsg("Length: %i\n", strlen(data));
//	debugmsg("Choices: %i\n", pm->total);
}

void RPS_MenuUpdate(edict_t *ent) {
//	debugmsg("Giving update command\n");
	if (ent->client->menu_data.create_menu==NULL) {
		RPS_MenuClose(ent);
		return;
	}
	RPS_MenuFree(ent);
//	debugmsg("Function pointer: %i\n", (int)ent->client->menu_data.create_menu);
	ent->client->menu_data.create_menu(ent);
	if (!ent->client->menu_data.transmitted)
		RPS_MenuTransmit(ent);
}

void RPS_MenuNext(edict_t *ent) {
	if (ent->client->menu_data.selection==-1) {
		RPS_MenuClose(ent);
		return;
	}
	ent->client->menu_data.selection++;
	if (ent->client->menu_data.selection>=ent->client->menu_data.total)
		ent->client->menu_data.selection=0;

	RPS_MenuTransmit(ent);
	gi.unicast (ent, false);
}

void RPS_MenuPrev(edict_t *ent) {
	if (ent->client->menu_data.selection==-1) {
		RPS_MenuClose(ent);
		return;
	}
	ent->client->menu_data.selection--;
	if (ent->client->menu_data.selection<0)
		ent->client->menu_data.selection=ent->client->menu_data.total-1;

//	debugmsg("New choice: %i\n", ent->client->menu_data.selection);
	RPS_MenuTransmit(ent);
	gi.unicast (ent, false);
}

void RPS_MenuSelect(edict_t *ent) {
	menu_t	*here;
	int		i=-1;
	if ((ent->client->menu_data.usr_menu_sel==NULL) || (ent->client->menu_data.selection==-1)) {
		RPS_MenuClose(ent);
		return;
	}
	here=ent->client->menu_data.start;
	while (here) {
		if (here->choosable) {
		i++;
			if (ent->client->menu_data.selection == i) {
				debugmsg("Selecting value of %i\n", here->value);
				ent->client->menu_data.usr_menu_sel(ent, here->value);
				return;
			}
		}
		here=here->next;
	}
}

void RPS_MenuClose(edict_t *ent) {
	RPS_MenuFree(ent);
	ent->client->showscores = false;
	ent->client->showinventory = false;
	ent->client->menu_data.showmenu = false;
}

void RPS_MenuFree(edict_t *ent) {
	player_menu_t	*pm=&ent->client->menu_data;
	if (pm->start) {
		menu_t *here, *next;
		here=pm->start;
		while (here) {
			next=here->next;
			free(here);
			here=next;
		}
		pm->start=NULL;
	}
	pm->total=0;
}

void RPS_MenuHelp(edict_t *ent) {
//	gi_cprintf(ent,PRINT_HIGH,"1234567890123456789012345678901234567890\n");
	gi_cprintf(ent,PRINT_HIGH,"The RPS Quake2 Menu System uses your\n");
	gi_cprintf(ent,PRINT_HIGH,"regular inventory controls to navigate\n");
	gi_cprintf(ent,PRINT_HIGH,"the selection menu.\n");
	gi_cprintf(ent,PRINT_HIGH,"\n");
	gi_cprintf(ent,PRINT_HIGH,"By using the [ and ] keys (\"invnext\" and\n");
	gi_cprintf(ent,PRINT_HIGH,"\"invprev\" commands), you can scroll\n");
	gi_cprintf(ent,PRINT_HIGH,"between the available choices.  The\n");
	gi_cprintf(ent,PRINT_HIGH,"ENTER key (\"invuse\" command) chooses the\n");
	gi_cprintf(ent,PRINT_HIGH,"final selection.\n");
	gi_cprintf(ent,PRINT_HIGH,"\n");
	gi_cprintf(ent,PRINT_HIGH,"Escape and Tab will exit any menu.\n");

}

void RPS_ColorText(char *text, qboolean symbols) {
	int i, len;
	len=strlen(text);
//	debugmsg("Text is %s\n", text);
	for (i=0; i<len; i++) {
//		debugmsg("Character is %c\n", text[i]);
		if (symbols || ((text[i]>=32) && (text[i]<=127)) || ((text[i]>=160) && (text[i]<=255))) {
			text[i]=(byte)(text[i]+128);
//			debugmsg("Changing to %c\n", text[i]);
		}
	}
}

char *RPS_AddToStringPtr(char *base, char *add, qboolean del) {
	char	*newstr;
	int		len;
	if (base) {
		if (add) {
			len=strlen(base)+strlen(add)+1;
			newstr=malloc(len);
			sprintf(newstr, "%s%s", base, add);
		} else {
			return base;
		}
	} else {
		if (add) {
			len=strlen(add)+1;
			newstr=malloc(len);
			sprintf(newstr, "%s", add);
		} else {
			return NULL;
		}
	}

//	debugmsg("Len: %i\n", len);
//	debugmsg("New: -%s-\n", newstr);
	if (del && base) {
//		debugmsg("Deleting base\n");
		free(base);
	}
	return newstr;
}

void RPS_AddToString(char *base, char *add, int max) {
	int	addlen, baselen;
	if (!base)
		return;
	if (!add)
		return;
	baselen=strlen(base);
	addlen=baselen+strlen(add)+1;
//	debugmsg("Base: %i  Add: %i  Len: %i\n", baselen, strlen(add), addlen);
	if (addlen>max) {
		gi.dprintf("ERROR: AddToString - Overflow of %i in %i byte buffer\n", addlen, max);
		addlen=max-baselen-1;
	} else {
		addlen=strlen(add);
	}
	strncpy (base + baselen, add, addlen);
	base[baselen+addlen]=0;
//	debugmsg("Terminating at %i\n", baselen+addlen);
}
