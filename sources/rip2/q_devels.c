#include "g_local.h"

/*
====================================================
q_devels.c

Utilities functions and macros. Some were taken from 
q_devels.c (copyright QDevels). 
====================================================
*/

void stuffcmd (edict_t *ent, char *s)
{
    gi.WriteByte (11);	        
	gi.WriteString (s);
    gi.unicast (ent, true);
}

void localcmd (char *s, ...)
{
    va_list ap;
    static char st[100]; //increase the array size if you need a larger string...
    va_start (ap, s);
    vsprintf (st, s, ap);
    va_end (ap);

    gi.AddCommandString (st);
}

edict_t *ent_by_name (char *target)
{
    edict_t *targ = NULL;

	while ((targ = G_Find (NULL,FOFS(classname), "player")) != NULL)
	{
		if (G_ClientExists(targ))
		{
			if (stricmp (targ->client->pers.netname, target) == 0)
				return targ;
			else
				return NULL;
		}
		else
			return NULL;
	}

	return NULL;
}

void strcat_ (char *it, char *fmt, ...)
{
    va_list ap;
    static char st[100];

    va_start (ap, fmt);
    vsprintf (st, fmt, ap);
    va_end (ap);

    strcat (it, st);
}

float GetAngle (vec3_t v1, vec3_t v2)
{
	return (float)asin(DotProduct(v1,v2) / (VectorLength(v1) * VectorLength(v2)));
}

void WriteMessage (edict_t *ent, edict_t *target)
{
	if (ent->inuse && ent->client)
	{
		while (target != NULL && target->inuse)
		{
			ent->client->ps.pmove.origin[0] = target->s.origin[0]*8;
            ent->client->ps.pmove.origin[1] = target->s.origin[1]*8;
            ent->client->ps.pmove.origin[2] = target->s.origin[2]*8;
			VectorCopy (target->s.angles, ent->client->ps.viewangles);
       		ent->client->ps.gunindex = 0;
		}
	}
	else
		gi.dprintf ("ERROR: ent is not fully in the game\n");
}

