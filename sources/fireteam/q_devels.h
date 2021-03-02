//
// QDEVELS.H - cool support functions for Quake II development
// Version 1.5 (last updated Jan 29, 1998)
//
// Published at http://www.planetquake.com/qdevels
// Code by various authors, released by SumFuka@planetquake.com
//
// Please browse qdevels.c for function descriptions.


#define for_each_player(JOE_BLOGGS,INDEX)				\
for(INDEX=1;INDEX<=maxclients->value;INDEX++)			\
	if ((JOE_BLOGGS=&g_edicts[i]) && JOE_BLOGGS->inuse)

#define rndnum(y,z) ((random()*((z)-((y)+1)))+(y))

void x_stuffcmd(edict_t *e, char *s);
edict_t *ent_by_name (char *target);
void centerprint_all (char *msg);



