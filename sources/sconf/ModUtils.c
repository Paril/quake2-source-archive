// ModUtils.c
//
// Written by Two Pole Software
// a.k.a. Rohn, SteQve, and Mr. X
//
// programmed by SteQve

#include "g_local.h"
#include "ModUtils.h"

// Slightly adapted from Q_strncasecmp.
int StrBeginsWith (char *s1, char *s2)
	{
	int		c1, c2, max = 999999;
	
	do
	{
		c1 = *s1++;
		c2 = *s2++;
			if (!c1) return(1); // Reached end of search string
		if (!max--)
			return 1;		// strings are equal until end point
		
		if (c1 != c2)
		{
			if (c1 >= 'a' && c1 <= 'z')
				c1 -= ('a' - 'A');
			if (c2 >= 'a' && c2 <= 'z')
				c2 -= ('a' - 'A');
			if (c1 != c2)
				return 0;		// strings not equal
		}
	} while (c1);
	
	return 1;		// strings are equal
}


void SetPlayerShellColor (edict_t *ent, int color)
{
	// This cancels out effects of powerups, whose color is set via
	// s.renderfx and not s.effects.  This may be a little overkill.
	ent->s.effects = 0;
	if (color == UTIL_SHELL_COLOR_NONE)
	{
		ent->s.renderfx &= ~UTIL_SHELL_COLOR_WHITE;
		ent->s.effects &= ~EF_COLOR_SHELL;
	}
	else {
		// "erase" any other colors that were set
		ent->s.renderfx &= ~UTIL_SHELL_COLOR_WHITE;
		ent->s.renderfx |= color;
		ent->s.effects |= EF_COLOR_SHELL;
	}
}

// Hack for Q2 3.14 (and 3.12 and 3.13)...  It appears the shell color effects
// variable has been changed.  *So*, just flash between blue, green,
// and red - temporarily, of course.
void SetPlayerShellColor314Hack (edict_t *ent, int color)
{

	/*
	if ((! PlayerHasShellEffects(ent)) ||
		(level.framenum % 2 == 1))
	{
		// Flash Green, since it's only used when an It is being 
		// hit.  If It has power armor, then... well.... use red
		
	}
	*/

	// This cancels out effects of powerups, whose color is set via
	// s.renderfx and not s.effects.  This may be a little overkill.
	ent->s.effects = 0;
	if (color == UTIL_SHELL_COLOR_NONE)
	{
		ent->s.renderfx &= ~UTIL_SHELL_COLOR_WHITE;
		ent->s.effects &= ~EF_COLOR_SHELL;
	}
	else {
		// "erase" any other colors that were set
		ent->s.renderfx &= ~UTIL_SHELL_COLOR_WHITE;
		ent->s.renderfx |= color;
		ent->s.effects |= EF_COLOR_SHELL;
	}
}

int PlayerHasShellEffects (edict_t *ent)
{
	int		pa_type;
	int		remaining;
	int has_effects = 0;

	if (!ent) return (0);

	// Adapted from G_SetClientEffects in p_view.c
	if (ent->powerarmor_time > level.time)
	{
		pa_type = PowerArmorType (ent);
		if (pa_type == POWER_ARMOR_SCREEN) return(1);
		else if (pa_type == POWER_ARMOR_SHIELD) return(1);
	}
	if (ent->client->quad_framenum > level.framenum)
	{
		remaining = ent->client->quad_framenum - level.framenum;
		if (remaining > 30 || (remaining & 4) )
			return(1);
	}
	if (ent->client->invincible_framenum > level.framenum)
	{
		remaining = ent->client->invincible_framenum - level.framenum;
		if (remaining > 30 || (remaining & 4) )
			return(1);
	}

	// show cheaters!!!
	if (ent->flags & FL_GODMODE) return(1);

	return(0);

}

// Change: what if clients have "non-printable" characters in their names?
// e.g. they've got green letters
edict_t *FindClientByNetname (char *name)
{
	int n;
	edict_t *player;

	// Code obtained from PlayersRangeFromSpot in p_client.c
	for (n = 1; n <= maxclients->value; n++)
	{
		player = &g_edicts[n];
		if (!player->inuse)
			continue;
		if (!player->client)
			continue;
		if (!player->client->pers.netname)
			continue;
		if (!Q_strcasecmp(name, player->client->pers.netname))
			return(player);
	}
	return(NULL);
}

edict_t *FindClientByMatchingNetname (char *name)
{
	int n, match_count;
	edict_t *player, *match_player;
	char *match;

	match_count = 0;
	// Code obtained from PlayersRangeFromSpot in p_client.c
	for (n = 1; n <= maxclients->value; n++)
	{
		player = &g_edicts[n];
		if (!player->inuse)
			continue;
		if (!player->client)
			continue;
		if (!player->client->pers.netname)
			continue;
		match = strstr(player->client->pers.netname, name);

		if (match != NULL) {
			match_player = player;
			match_count++;
		}
	}
	if (match_count == 1)
		return(match_player);
	else return(NULL);
}


int IsActiveClient (edict_t *ent)
{
	if ((!ent->inuse) || (!ent->client))
		return(0);
	else return(1);
}

// level.players counts those who connected, not the ones who are active;
// plus, sometimes it seems to go wacky, but maybe that's just me.
int CountActiveClients ()
{
	int n, count;
	edict_t *player;

	count = 0;
	for (n = 1; n <= maxclients->value; n++)
	{
		player = &g_edicts[n];
		if (!IsActiveClient(player))
			continue;
		count++;
	}
	return(count);
}

// level.players counts those who connected, not the ones who are active;
// plus, sometimes it seems to go wacky, but maybe that's just me.
int CountConnectedClients ()
{
	int n, count;
	edict_t *player;

	count = 0;
	for (n = 1; n <= maxclients->value; n++)
	{
		player = &g_edicts[n];
		if (!player->inuse)
			continue;
		count++;
	}
	return(count);
}

// Could use a better random number generator...  do srand().
edict_t *GetRandomClient ()
{
	int n, x, r, tot;
	edict_t *player;

	tot = CountActiveClients();
	if (! tot) {
		// why would this happen?
		DPRINT0("ModUtils: no clients for random client?\n");
		return(NULL);
	}

	// Errrrr... isn't there some count of the number of clients somewhere?
	r = rand() % tot;
	x = 0;
	for (n = 1; n <= maxclients->value; n++)
	{
		player = &g_edicts[n];
		if (!player->inuse)
			continue;
		if (!player->client)
			continue;
		if (x == r) return(player);
		x++;
	}
	DPRINT0("ModUtils: no clients for random client?\n");
	return(NULL);
}

// DESTRUCTIVELY converts an entire string to bold text.
void MakeBoldString (char *text)
{
	while (*text)
		if (*text == '\n') 
			text++;
		else
			*(text++) = *text + 0x80;
	return;
}

// DESTRUCTIVE!!!
void FormatBoldString (char *text)
{
	int t, i, do_bold;

	t=0;
	do_bold = 0;
	for (i=0; text[i] != '\0'; i++)
	{
		// WOW!!!!  Check out that inefficiency!!!!  I am DANGEROUS!!!
		if (StrBeginsWith("<b>", text+i))
		{
			do_bold = 1;
			i += 2;
		}
		else if (StrBeginsWith("</b>", text+i))
		{
			do_bold = 0;
			i += 3;
		}
		else if (do_bold)
		{
			if (*(text+i) == '\n')
				text[t++] = *(text+i);
			else
				text[t++] = *(text+i) + 0x80;
		}
		else {
			text[t++] = *(text+i);
		}
	}
	text[t] = '\0';
}
