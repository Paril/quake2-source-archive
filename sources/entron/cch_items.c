/**********************************************************\
(c) 1998-1999 Endor Productions. All rights reserved.
Written by Chris Hilton and Nawfel 'Topaz' Tricha. 
NOTICE: This code is given as is. There is no direct support
for anything written here. All the information/code included
here are free to use and redistribute under no resitrictions
as long as you credit Endor Productions and the Author for
any snipet of code you might find interesting.

  FILE: cch_items.c
  Description: Chris' items 

\**********************************************************/

#include "g_local.h"
#include "cch_items.h"


// Check beh_items cvar to see how to substitute Beholder weapons & other items
// into maps
// 1 = swap certain Beholder weapons for regular Q2 weapons
// 2 = randomly place Beholder weapons at every weapon spawn
// 3 = randomly place Beholder and Q2 weapons at every weapon spawn
// default = no substitution
void CheckBeholderItems(edict_t *ent)
{
	int			i;
	behsub_t	*sub;

	switch ( (int)beh_items->value )
	{
	case 0:
		return;
	case 1:
		for ( i = 0, sub = sublist; i < num_subs; i++, sub++ )
		{
			if (!strcmp(ent->classname, sub->classname))
			{	// found it
				ent->classname = sub->subname;
				return;
			}
		}
		break;
	case 2:
		if ( strncmp(ent->classname, "weapon_", 7) == 0 )
		{
			i = (int)(random() * num_behweaps);
			ent->classname = behweaplist[i];
			return;
		}
		break;
	case 3:
		if ( strncmp(ent->classname, "weapon_", 7) == 0 )
		{
			i = (int)(random() * (num_behweaps + num_q2weaps));
			if ( i < num_q2weaps )
			{
				ent->classname = q2weaplist[i];
				return;
			}
			i -= num_q2weaps;
			ent->classname = behweaplist[i];
			return;
		}
		break;
	}
}

