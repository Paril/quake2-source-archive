//
// QDEVELS.H - cool support functions for Quake II development
// Version 1.0 (last updated Jan 6, 1998)
// 
// Published at http://www.planetquake.com/qdevels
// Code by various authors, released by SumFuka@planetquake.com
//
// Please browse qdevels.c for function descriptions.


#define for_each_player(JOE_BLOGGS,INDEX)				\
for(INDEX=1;INDEX<=maxclients->value;INDEX++)			\
	if ((JOE_BLOGGS=&g_edicts[i]) && JOE_BLOGGS->inuse)

#define rndnum(y,z) floor(((random()*(z-(y+1)))+y))
#define midnum(y,z) ((y+z)/2)

#define rip_ent(x) while((rip_ent=G_Find(NULL,FOFS(classname),x)) != NULL)
