#include "g_local.h"
#include "q_devels.h"
#include "m_player.h"

void Waypoint_Think (edict_t *ent)
{
       edict_t *blip = NULL;

       if (ent->selected->deadflag == DEAD_DEAD) {
          G_FreeEdict (ent);
       }
       /*
       if (ent->selected->selected != ent)
       {
          G_FreeEdict (ent);
       }
       */
       ent->think = Waypoint_Think;
       while ((blip = findradius(blip, ent->s.origin, 50)) != NULL)
       {
               if (!(blip->svflags & SVF_MONSTER))
                       continue;
               if (blip == ent->owner)
                       continue;
               if (!blip->takedamage)
                       continue;
               if (ent->selected != blip)
                       continue;
               if (blip->health <= 0)
                       continue;
               if (!visible(ent, blip))
                       continue;
        blip->goalentity = NULL;
        blip->monsterinfo.aiflags |= (AI_STAND_GROUND | AI_TEMP_STAND_GROUND);
        blip->monsterinfo.stand (blip);
        blip->enemy = NULL;
        blip->selected = NULL;
               G_FreeEdict (ent);

               break;
       }

       ent->nextthink = level.time + .1;
}

void Patrol_Think (edict_t *ent)
{
       edict_t *blip = NULL;

     /*
       if (level.time > ent->delay)
       {
          G_FreeEdict (ent);
       }
     */

        
       if (ent->selected->deadflag == DEAD_DEAD) {
          G_FreeEdict (ent);
       }

       ent->think = Patrol_Think;
       while ((blip = findradius(blip, ent->s.origin, 100)) != NULL)
       {
               if (!(blip->svflags & SVF_MONSTER))
                       continue;
               if (blip == ent->owner)
                       continue;
               if (!blip->takedamage)
                       continue;
               if (blip->health <= 0)
                       continue;
               if (ent->selected != blip)
                       continue;
               if (!blip->waypoint2)
                       continue;
               if (!blip->waypoint1)
                       continue;
               if (!visible(ent, blip))
                       continue;

       if (Q_stricmp(ent->classname, "patrol2") == 0) {
       blip->goalentity = blip->movetarget = blip->waypoint1;
        }
       if (Q_stricmp(ent->classname, "patrol1") == 0) {
       blip->goalentity = blip->movetarget = blip->waypoint2;
        }
      blip->monsterinfo.walk (blip->movetarget);
            //t„nne roinaa
               break;
       }

       ent->nextthink = level.time + .1;
}

char *ClientTeam (edict_t *ent)
{
	char		*p;
	static char	value[512];

	value[0] = 0;

	if (!ent->client)
		return value;

	strcpy(value, Info_ValueForKey (ent->client->pers.userinfo, "skin"));
	p = strchr(value, '/');
	if (!p)
		return value;

	if ((int)(dmflags->value) & DF_MODELTEAMS)
	{
		*p = 0;
		return value;
	}

	// if ((int)(dmflags->value) & DF_SKINTEAMS)
	return ++p;
}

qboolean OnSameTeam (edict_t *ent1, edict_t *ent2)
{
	char	ent1Team [512];
	char	ent2Team [512];

	if (!((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS)))
		return false;

	strcpy (ent1Team, ClientTeam (ent1));
	strcpy (ent2Team, ClientTeam (ent2));

	if (strcmp(ent1Team, ent2Team) == 0)
		return true;
	return false;
}
qboolean CheckRadius (edict_t *ent, int pituus)
{
      edict_t *blip = NULL;
      int monstercount;
	vec3_t	vieworg;

	VectorAdd (ent->s.origin, ent->client->ps.viewoffset, vieworg);

        if (gi.pointcontents(vieworg) & CONTENTS_SOLID)
        {
        gi.cprintf(ent, PRINT_HIGH, "You cannot spawn anything outside level!");
        return;
        }
      monstercount = 0;

       while ((blip = findradius(blip, ent->s.origin, pituus)) != NULL)
       {

             //  if ((blip->monsterinfo.stand != NULL) && (blip->health > 0))
               if ((blip->svflags & SVF_MONSTER) && (blip->health > 0))

               {
            gi.cprintf (ent, PRINT_HIGH, "Another monster too close\n");
               return false;
               }
       }

       while ((blip = findradius(blip, ent->s.origin, pituus * 2)) != NULL)
       {

               if ((blip->player > 0) && (blip->health > 0))
               {
            gi.cprintf (ent, PRINT_HIGH, "Player too close\n");
               return false;
               }
       }

       while ((blip = findradius(blip, ent->s.origin, 728)) != NULL)
       {
             //  if ((blip->monsterinfo.stand != NULL) && (blip->health > 0))

               if ((blip->svflags & SVF_MONSTER) && (blip->health > 0))
               {
               monstercount = monstercount + 1;
               }
     
               if (monstercount > 7) {
            gi.cprintf (ent, PRINT_HIGH, "Too many monsters in this area (exceeded nearby monster limit)\n");
               return false;

               }

       }
return true;
}

qboolean EnoughStuff (edict_t *ent, int amount, char *s, int pituus)
{
	int			index;
	gitem_t		*it;
//        char            *s;
	vec3_t	vieworg;

	VectorAdd (ent->s.origin, ent->client->ps.viewoffset, vieworg);

        if (gi.pointcontents(vieworg) & CONTENTS_SOLID)
        {
        gi.cprintf(ent, PRINT_HIGH, "You cannot spawn anything outside level!");
        return;
        }
        //s = "Cells";
	it = FindItem (s);
	index = ITEM_INDEX(it);
        if (ent->client->pers.inventory[index] < amount)
	{
            gi.cprintf (ent, PRINT_HIGH, "Not Enough Cells for that operation\n");
                return false;
	}
        else
        {
        if (CheckRadius(ent, pituus)) {
        ent->client->pers.inventory[index] -= amount;
        }
        else
        {
        return false;
        }

        }
        return true;
        
}

qboolean EnoughCells (edict_t *ent, int amount, char *s)
{
	int			index;
	gitem_t		*it;

	it = FindItem (s);
	index = ITEM_INDEX(it);
        if (ent->client->pers.inventory[index] < amount)
	{
            gi.cprintf (ent, PRINT_HIGH, "Not Enough Cells for that operation\n");
                return false;
	}
        else
        {
        ent->client->pers.inventory[index] -= amount;
        }
        return true;
        
}

qboolean EnoughStuff2 (edict_t *ent, int amount, char *s)
{
	int			index;
	gitem_t		*it;
//        char            *s;

	vec3_t	vieworg;

	VectorAdd (ent->s.origin, ent->client->ps.viewoffset, vieworg);

        if (gi.pointcontents(vieworg) & CONTENTS_SOLID)
        {
        gi.cprintf(ent, PRINT_HIGH, "You cannot spawn anything outside level!");
        return;
        }
//s = "Cells";
	it = FindItem (s);
	index = ITEM_INDEX(it);
        if (ent->client->pers.inventory[index] < amount)
	{
            gi.cprintf (ent, PRINT_HIGH, "Not Enough Cells for that operation\n");
                return false;
	}
        
        return true;
        
}


/*
=================
Cmd_id_f
=================
*/
void Cmd_id_f (edict_t *ent)
{
int j;
    char stats[500];
    vec3_t  start, forward, end;
trace_t tr;
    j = sprintf(stats, "     NAME              RANGE        \n\n");

VectorCopy(ent->s.origin, start);
    start[2] += ent->viewheight;
    AngleVectors(ent->client->v_angle, forward, NULL, NULL);
    VectorMA(start, 8192, forward, end);

    tr = gi.trace(start, NULL, NULL, end, ent,
MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA);

    if (tr.ent->client)
{
j += sprintf(stats + j, "%16s          %i              \n",
tr.ent->client->pers.netname, (int)(tr.fraction * 512));
gi.centerprintf(ent, "%s", stats);      
}

}
// End code

void Cmd_Select_f (edict_t *ent)
{
       vec3_t  start;
       vec3_t  forward;
       vec3_t  end;
       trace_t tr;
       edict_t *waypoint;

       VectorCopy(ent->s.origin, start);
       start[2] += ent->viewheight;
       AngleVectors(ent->client->v_angle, forward, NULL, NULL);
       VectorMA(start, 8192, forward, end);

    tr = gi.trace(start, NULL, NULL, end, ent,
MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA);

       //Valittu
       if ( tr.ent && (tr.ent->svflags & SVF_MONSTER))
       {
       ent->selected = tr.ent;
       }
       else
       if (tr.ent && ent->selected && (!ent->selected->selected))
       {
       if (ent->selected->health < 1) return;
       if (ent->selected->waypoint1)
       {
       G_FreeEdict (ent->selected->waypoint1);
       if (ent->selected->waypoint2) G_FreeEdict (ent->selected->waypoint2);
       ent->selected->waypoint1 = NULL;
       ent->selected->waypoint2 = NULL;
       }

       waypoint = G_Spawn();
       VectorCopy(tr.endpos, waypoint->s.origin);
       waypoint->s.origin[2] += 32;
       waypoint->delay = level.time + 45;
       waypoint->nextthink = level.time + .1;
       waypoint->think = Waypoint_Think;
       waypoint->selected = ent->selected;
       waypoint->classname = "waypoint";
       waypoint->s.modelindex = gi.modelindex ("models/objects/target/tris.md2");
       ent->selected->goalentity = ent->selected->movetarget = waypoint;
       ent->selected->selected = waypoint;
       ent->selected->monsterinfo.walk (ent->selected);
        gi.linkentity (waypoint);
       }
       else
       if (tr.ent && ent->selected && ent->selected->selected)
       {
       if (!(ent->selected->selected->svflags & SVF_MONSTER))
       G_FreeEdict (ent->selected->selected);
       ent->selected->selected = NULL;
       ent->selected->goalentity = NULL;
       ent->selected->enemy = NULL;
       ent->selected->monsterinfo.aiflags |= (AI_STAND_GROUND | AI_TEMP_STAND_GROUND);
       ent->selected->monsterinfo.stand (ent->selected);

       }

    }


void Cmd_Select2_f (edict_t *ent)
{
       vec3_t  start;
       vec3_t  forward;
       vec3_t  end;
       trace_t tr;
//       edict_t *waypoint;

       VectorCopy(ent->s.origin, start);
       start[2] += ent->viewheight;
       AngleVectors(ent->client->v_angle, forward, NULL, NULL);
       VectorMA(start, 8192, forward, end);

    tr = gi.trace(start, NULL, NULL, end, ent,
MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA);

       //Valittu
       if ( tr.ent && (tr.ent->svflags & SVF_MONSTER) && !ent->selected)
       {
       ent->selected = tr.ent;
       }
       else
       if (tr.ent && ent->selected && (tr.ent->svflags & SVF_MONSTER))
       {
       if (ent->selected->health < 1) return;
       if (ent->selected->waypoint1)
       {
       G_FreeEdict (ent->selected->waypoint1);
       G_FreeEdict (ent->selected->waypoint2);
       ent->selected->waypoint1 = NULL;
       ent->selected->waypoint2 = NULL;
       }
       ent->selected->selected = NULL;
       ent->selected->goalentity = ent->selected->movetarget = tr.ent;
       ent->selected->selected = tr.ent;
       ent->selected->monsterinfo.walk (ent->selected);


       }
       else
       if (tr.ent && ent->selected && (tr.ent == ent->selected))
       {
       ent->selected->selected = NULL;
       ent->selected->goalentity = NULL;
       ent->selected->enemy = NULL;
       ent->selected->monsterinfo.aiflags |= (AI_STAND_GROUND | AI_TEMP_STAND_GROUND);
       ent->selected->monsterinfo.stand (ent->selected);
       ent->selected = NULL;

       }

    }




void Cmd_Patrol_f (edict_t *ent)
{
       vec3_t  start;
       vec3_t  forward;
       vec3_t  end;
       trace_t tr;
       edict_t *waypoint;

       VectorCopy(ent->s.origin, start);
       start[2] += ent->viewheight;
       AngleVectors(ent->client->v_angle, forward, NULL, NULL);
       VectorMA(start, 8192, forward, end);

    tr = gi.trace(start, NULL, NULL, end, ent,
MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA);

       //Valittu
       if ( tr.ent && (tr.ent->svflags & SVF_MONSTER))
       {
       //(tr.fraction * 512)
       ent->selected = tr.ent;
       }
       else
       if (tr.ent && ent->selected && (!ent->selected->waypoint1))
       {
       if (ent->selected->health < 1) return;

       if (ent->selected && ent->selected->selected)
       {
       G_FreeEdict (ent->selected->selected);
       ent->selected->selected = NULL;
       ent->selected->goalentity = NULL;
       ent->selected->monsterinfo.aiflags |= (AI_STAND_GROUND | AI_TEMP_STAND_GROUND);
       ent->selected->monsterinfo.stand (ent->selected);
       ent->selected->enemy = NULL;

       }

       waypoint = G_Spawn();
       VectorCopy(tr.endpos, waypoint->s.origin);
       waypoint->s.origin[2] += 32;
       waypoint->selected = ent->selected;
       waypoint->delay = level.time + 145;

       waypoint->nextthink = level.time + .1;
       waypoint->think = Patrol_Think;
       waypoint->classname = "patrol1";
       waypoint->s.modelindex = gi.modelindex ("models/objects/target/tris.md2");
       ent->selected->waypoint1 = waypoint;

      // ent->selected->goalentity = ent->selected->movetarget = waypoint;
      // ent->selected->monsterinfo.walk (ent->selected);
        gi.linkentity (waypoint);
       return;
       }
       if (tr.ent && ent->selected && ent->selected->waypoint1 && (!ent->selected->waypoint2) )
       {
       if (ent->selected->health < 1) return;
       waypoint = G_Spawn();
       VectorCopy(tr.endpos, waypoint->s.origin);
       waypoint->s.origin[2] += 32;
       waypoint->selected = ent->selected;

       waypoint->delay = level.time + 145;

       waypoint->nextthink = level.time + .1;
       waypoint->think = Patrol_Think;
       waypoint->classname = "patrol2";
       waypoint->s.modelindex = gi.modelindex ("models/objects/target/tris.md2");
       ent->selected->waypoint2 = waypoint;

       ent->selected->goalentity = ent->selected->movetarget = ent->selected->waypoint1;
       ent->selected->monsterinfo.walk (ent->selected);
        gi.linkentity (waypoint);
       return;
       }

       if (tr.ent && ent->selected && ent->selected->waypoint1 && ent->selected->waypoint2 )
       {
       ent->selected->goalentity = NULL;
       ent->selected->monsterinfo.aiflags |= (AI_STAND_GROUND | AI_TEMP_STAND_GROUND);
       ent->selected->monsterinfo.stand (ent->selected);
       ent->selected->enemy = NULL;

       if (ent->selected->waypoint1->health < 1) G_FreeEdict(ent->selected->waypoint1);
       if (ent->selected->waypoint1->health < 1) G_FreeEdict(ent->selected->waypoint2);
       ent->selected->waypoint1 = NULL;
       ent->selected->waypoint2 = NULL;
       return;
       }
    }


/*
=================
Cmd_Push_f
=================
*/
void Cmd_Push_f (edict_t *ent)
{
       vec3_t  start;
       vec3_t  forward;
       vec3_t  end;
       trace_t tr;
	int			index;
	gitem_t		*it;
        char            *s;

       if (ent->timetospawn < level.time) {
       gi.sound (ent, CHAN_BODY, gi.soundindex ("misc/windfly.wav"), 1, ATTN_NORM, 0);
       ent->timetospawn = level.time + 10;
       VectorCopy(ent->s.origin, start);
       start[2] += ent->viewheight;
       AngleVectors(ent->client->v_angle, forward, NULL, NULL);
       VectorMA(start, 8192, forward, end);
       tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT);
       if ( tr.ent && ((tr.ent->svflags & SVF_MONSTER) || (tr.ent->client)) )
       {
               VectorScale(forward, 2500, forward);

               VectorAdd(forward, tr.ent->velocity, tr.ent->velocity);
       if (tr.ent->svflags & SVF_MONSTER) {
        s = "Cells";
	it = FindItem (s);
	index = ITEM_INDEX(it);
       ent->client->pers.inventory[index] += 5;
       }
       }
       }
       else
       {
        gi.cprintf (ent, PRINT_HIGH, "You need to wait for your mental energies to recharge.\n");
        s = "Cells";
	it = FindItem (s);
	index = ITEM_INDEX(it);

        ent->client->pers.inventory[index] += 5;

        }
}


void SelectNextItem (edict_t *ent, int itflags)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;

	cl = ent->client;

	if (cl->chase_target) {
		ChaseNext(ent);
		return;
	}

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (cl->pers.selected_item + i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (!(it->flags & itflags))
			continue;

		cl->pers.selected_item = index;
		return;
	}

	cl->pers.selected_item = -1;
}

void SelectPrevItem (edict_t *ent, int itflags)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;

	cl = ent->client;

	if (cl->chase_target) {
		ChasePrev(ent);
		return;
	}

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (cl->pers.selected_item + MAX_ITEMS - i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (!(it->flags & itflags))
			continue;

		cl->pers.selected_item = index;
		return;
	}

	cl->pers.selected_item = -1;
}

void ValidateSelectedItem (edict_t *ent)
{
	gclient_t	*cl;

	cl = ent->client;

	if (cl->pers.inventory[cl->pers.selected_item])
		return;		// valid

	SelectNextItem (ent, -1);
}


//=================================================================================

/*
==================
Cmd_Give_f

Give items to a client
==================
*/
void Cmd_Give_f (edict_t *ent)
{
	char		*name;
	gitem_t		*it;
	int			index;
	int			i;
	qboolean	give_all;
	edict_t		*it_ent;

	if (deathmatch->value && !sv_cheats->value)
	{
		gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	name = gi.args();

	if (Q_stricmp(name, "all") == 0)
		give_all = true;
	else
		give_all = false;

	if (give_all || Q_stricmp(gi.argv(1), "health") == 0)
	{
		if (gi.argc() == 3)
			ent->health = atoi(gi.argv(2));
		else
			ent->health = ent->max_health;
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "weapons") == 0)
	{
		for (i=0 ; i<game.num_items ; i++)
		{
			it = itemlist + i;
			if (!it->pickup)
				continue;
			if (!(it->flags & IT_WEAPON))
				continue;
			ent->client->pers.inventory[i] += 1;
		}
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "ammo") == 0)
	{
		for (i=0 ; i<game.num_items ; i++)
		{
			it = itemlist + i;
			if (!it->pickup)
				continue;
			if (!(it->flags & IT_AMMO))
				continue;
			Add_Ammo (ent, it, 1000);
		}
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "armor") == 0)
	{
		gitem_armor_t	*info;

		it = FindItem("Jacket Armor");
		ent->client->pers.inventory[ITEM_INDEX(it)] = 0;

		it = FindItem("Combat Armor");
		ent->client->pers.inventory[ITEM_INDEX(it)] = 0;

		it = FindItem("Body Armor");
		info = (gitem_armor_t *)it->info;
		ent->client->pers.inventory[ITEM_INDEX(it)] = info->max_count;

		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "Power Shield") == 0)
	{
		it = FindItem("Power Shield");
		it_ent = G_Spawn();
		it_ent->classname = it->classname;
		SpawnItem (it_ent, it);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);

		if (!give_all)
			return;
	}

	if (give_all)
	{
		for (i=0 ; i<game.num_items ; i++)
		{
			it = itemlist + i;
			if (!it->pickup)
				continue;
			if (it->flags & (IT_ARMOR|IT_WEAPON|IT_AMMO))
				continue;
			ent->client->pers.inventory[i] = 1;
		}
		return;
	}

	it = FindItem (name);
	if (!it)
	{
		name = gi.argv(1);
		it = FindItem (name);
		if (!it)
		{
			gi.cprintf (ent, PRINT_HIGH, "unknown item\n");
			return;
		}
	}

	if (!it->pickup)
	{
		gi.cprintf (ent, PRINT_HIGH, "non-pickup item\n");
		return;
	}

	index = ITEM_INDEX(it);

	if (it->flags & IT_AMMO)
	{
		if (gi.argc() == 3)
			ent->client->pers.inventory[index] = atoi(gi.argv(2));
		else
			ent->client->pers.inventory[index] += it->quantity;
	}
	else
	{
		it_ent = G_Spawn();
		it_ent->classname = it->classname;
		SpawnItem (it_ent, it);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);
	}
}


/*
==================
Cmd_God_f

Sets client to godmode

argv(0) god
==================
*/
void Cmd_God_f (edict_t *ent)
{
	char	*msg;

	if (deathmatch->value && !sv_cheats->value)
	{
		gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	ent->flags ^= FL_GODMODE;
	if (!(ent->flags & FL_GODMODE) )
		msg = "godmode OFF\n";
	else
		msg = "godmode ON\n";

	gi.cprintf (ent, PRINT_HIGH, msg);
}


/*
==================
Cmd_Notarget_f

Sets client to notarget

argv(0) notarget
==================
*/
void Cmd_Notarget_f (edict_t *ent)
{
	char	*msg;

	if (deathmatch->value && !sv_cheats->value)
	{
		gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	ent->flags ^= FL_NOTARGET;
	if (!(ent->flags & FL_NOTARGET) )
		msg = "notarget OFF\n";
	else
		msg = "notarget ON\n";

	gi.cprintf (ent, PRINT_HIGH, msg);
}


/*
==================
Cmd_Noclip_f

argv(0) noclip
==================
*/
void Cmd_Noclip_f (edict_t *ent)
{
	char	*msg;

	if (deathmatch->value && !sv_cheats->value)
	{
		gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	if (ent->movetype == MOVETYPE_NOCLIP)
	{
		ent->movetype = MOVETYPE_WALK;
		msg = "noclip OFF\n";
	}
	else
	{
		ent->movetype = MOVETYPE_NOCLIP;
		msg = "noclip ON\n";
	}

	gi.cprintf (ent, PRINT_HIGH, msg);
}


/*
==================
Cmd_Use_f

Use an inventory item
==================
*/
void Cmd_Use_f (edict_t *ent)
{
	int			index;
	gitem_t		*it;
	char		*s;

	s = gi.args();
	it = FindItem (s);
	if (!it)
	{
		gi.cprintf (ent, PRINT_HIGH, "unknown item: %s\n", s);
		return;
	}
	if (!it->use)
	{
		gi.cprintf (ent, PRINT_HIGH, "Item is not usable.\n");
		return;
	}
	index = ITEM_INDEX(it);
	if (!ent->client->pers.inventory[index])
	{
		gi.cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
		return;
	}

	it->use (ent, it);
}


/*
==================
Cmd_Drop_f

Drop an inventory item
==================
*/
void Cmd_Drop_f (edict_t *ent)
{
	int			index;
	gitem_t		*it;
	char		*s;

	s = gi.args();
	it = FindItem (s);
	if (!it)
	{
		gi.cprintf (ent, PRINT_HIGH, "unknown item: %s\n", s);
		return;
	}
	if (!it->drop)
	{
		gi.cprintf (ent, PRINT_HIGH, "Item is not dropable.\n");
		return;
	}
	index = ITEM_INDEX(it);
	if (!ent->client->pers.inventory[index])
	{
		gi.cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
		return;
	}

	it->drop (ent, it);
}


/*
=================
Cmd_Inven_f
=================
*/
void Cmd_Inven_f (edict_t *ent)
{
	int			i;
	gclient_t	*cl;

	cl = ent->client;

	cl->showscores = false;
	cl->showhelp = false;

	if (cl->showinventory)
	{
		cl->showinventory = false;
		return;
	}

	cl->showinventory = true;

	gi.WriteByte (svc_inventory);
	for (i=0 ; i<MAX_ITEMS ; i++)
	{
		gi.WriteShort (cl->pers.inventory[i]);
	}
	gi.unicast (ent, true);
}

/*
=================
Cmd_InvUse_f
=================
*/
void Cmd_InvUse_f (edict_t *ent)
{
	gitem_t		*it;

	ValidateSelectedItem (ent);

	if (ent->client->pers.selected_item == -1)
	{
		gi.cprintf (ent, PRINT_HIGH, "No item to use.\n");
		return;
	}

	it = &itemlist[ent->client->pers.selected_item];
	if (!it->use)
	{
		gi.cprintf (ent, PRINT_HIGH, "Item is not usable.\n");
		return;
	}
	it->use (ent, it);
}

/*
=================
Cmd_WeapPrev_f
=================
*/
void Cmd_WeapPrev_f (edict_t *ent)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;
	int			selected_weapon;

	cl = ent->client;

	if (!cl->pers.weapon)
		return;

	selected_weapon = ITEM_INDEX(cl->pers.weapon);

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (selected_weapon + i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (! (it->flags & IT_WEAPON) )
			continue;
		it->use (ent, it);
		if (cl->pers.weapon == it)
			return;	// successful
	}
}

/*
=================
Cmd_WeapNext_f
=================
*/
void Cmd_WeapNext_f (edict_t *ent)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;
	int			selected_weapon;

	cl = ent->client;

	if (!cl->pers.weapon)
		return;

	selected_weapon = ITEM_INDEX(cl->pers.weapon);

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (selected_weapon + MAX_ITEMS - i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (! (it->flags & IT_WEAPON) )
			continue;
		it->use (ent, it);
		if (cl->pers.weapon == it)
			return;	// successful
	}
}

/*
=================
Cmd_WeapLast_f
=================
*/
void Cmd_WeapLast_f (edict_t *ent)
{
	gclient_t	*cl;
	int			index;
	gitem_t		*it;

	cl = ent->client;

	if (!cl->pers.weapon || !cl->pers.lastweapon)
		return;

	index = ITEM_INDEX(cl->pers.lastweapon);
	if (!cl->pers.inventory[index])
		return;
	it = &itemlist[index];
	if (!it->use)
		return;
	if (! (it->flags & IT_WEAPON) )
		return;
	it->use (ent, it);
}

/*
=================
Cmd_InvDrop_f
=================
*/
void Cmd_InvDrop_f (edict_t *ent)
{
	gitem_t		*it;

	ValidateSelectedItem (ent);

	if (ent->client->pers.selected_item == -1)
	{
		gi.cprintf (ent, PRINT_HIGH, "No item to drop.\n");
		return;
	}

	it = &itemlist[ent->client->pers.selected_item];
	if (!it->drop)
	{
		gi.cprintf (ent, PRINT_HIGH, "Item is not dropable.\n");
		return;
	}
	it->drop (ent, it);
}

/*
=================
Cmd_Kill_f
=================
*/
void Cmd_Kill_f (edict_t *ent)
{
if (ent->gameleader) return;
	if((level.time - ent->client->respawn_time) < 5)
		return;
	ent->flags &= ~FL_GODMODE;
	ent->health = 0;
	meansOfDeath = MOD_SUICIDE;
	player_die (ent, ent, ent, 100000, vec3_origin);
}

/*
=================
Cmd_PutAway_f
=================
*/
void Cmd_PutAway_f (edict_t *ent)
{
	ent->client->showscores = false;
	ent->client->showhelp = false;
	ent->client->showinventory = false;
}


int PlayerSort (void const *a, void const *b)
{
	int		anum, bnum;

	anum = *(int *)a;
	bnum = *(int *)b;

	anum = game.clients[anum].ps.stats[STAT_FRAGS];
	bnum = game.clients[bnum].ps.stats[STAT_FRAGS];

	if (anum < bnum)
		return -1;
	if (anum > bnum)
		return 1;
	return 0;
}

/*
=================
Cmd_Players_f
=================
*/
void Cmd_Players_f (edict_t *ent)
{
	int		i;
	int		count;
	char	small[64];
	char	large[1280];
	int		index[256];

	count = 0;
	for (i = 0 ; i < maxclients->value ; i++)
		if (game.clients[i].pers.connected)
		{
			index[count] = i;
			count++;
		}

	// sort by frags
	qsort (index, count, sizeof(index[0]), PlayerSort);

	// print information
	large[0] = 0;

	for (i = 0 ; i < count ; i++)
	{
		Com_sprintf (small, sizeof(small), "%3i %s\n",
			game.clients[index[i]].ps.stats[STAT_FRAGS],
			game.clients[index[i]].pers.netname);
		if (strlen (small) + strlen(large) > sizeof(large) - 100 )
		{	// can't print all of them in one packet
			strcat (large, "...\n");
			break;
		}
		strcat (large, small);
	}

	gi.cprintf (ent, PRINT_HIGH, "%s\n%i players\n", large, count);
}

/*
=================
Cmd_Wave_f
=================
*/
void Cmd_Wave_f (edict_t *ent)
{
	int		i;

	i = atoi (gi.argv(1));

	// can't wave when ducked
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
		return;

	if (ent->client->anim_priority > ANIM_WAVE)
		return;

	ent->client->anim_priority = ANIM_WAVE;

	switch (i)
	{
	case 0:
		gi.cprintf (ent, PRINT_HIGH, "flipoff\n");
		ent->s.frame = FRAME_flip01-1;
		ent->client->anim_end = FRAME_flip12;
		break;
	case 1:
		gi.cprintf (ent, PRINT_HIGH, "salute\n");
		ent->s.frame = FRAME_salute01-1;
		ent->client->anim_end = FRAME_salute11;
		break;
	case 2:
		gi.cprintf (ent, PRINT_HIGH, "taunt\n");
		ent->s.frame = FRAME_taunt01-1;
		ent->client->anim_end = FRAME_taunt17;
		break;
	case 3:
		gi.cprintf (ent, PRINT_HIGH, "wave\n");
		ent->s.frame = FRAME_wave01-1;
		ent->client->anim_end = FRAME_wave11;
		break;
	case 4:
	default:
		gi.cprintf (ent, PRINT_HIGH, "point\n");
		ent->s.frame = FRAME_point01-1;
		ent->client->anim_end = FRAME_point12;
		break;
	}
}

/*
==================
Cmd_Say_f
==================
*/
void Cmd_Say_f (edict_t *ent, qboolean team, qboolean arg0)
{
	int		i, j;
	edict_t	*other;
	char	*p;
	char	text[2048];
	gclient_t *cl;

	if (gi.argc () < 2 && !arg0)
		return;

	if (!((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS)))
		team = false;

	if (team)
		Com_sprintf (text, sizeof(text), "(%s): ", ent->client->pers.netname);
	else
		Com_sprintf (text, sizeof(text), "%s: ", ent->client->pers.netname);

	if (arg0)
	{
		strcat (text, gi.argv(0));
		strcat (text, " ");
		strcat (text, gi.args());
	}
	else
	{
		p = gi.args();

		if (*p == '"')
		{
			p++;
			p[strlen(p)-1] = 0;
		}
		strcat(text, p);
	}

	// don't let text be too long for malicious reasons
	if (strlen(text) > 150)
		text[150] = 0;

	strcat(text, "\n");

	if (flood_msgs->value) {
		cl = ent->client;

        if (level.time < cl->flood_locktill) {
			gi.cprintf(ent, PRINT_HIGH, "You can't talk for %d more seconds\n",
				(int)(cl->flood_locktill - level.time));
            return;
        }
        i = cl->flood_whenhead - flood_msgs->value + 1;
        if (i < 0)
            i = (sizeof(cl->flood_when)/sizeof(cl->flood_when[0])) + i;
		if (cl->flood_when[i] && 
			level.time - cl->flood_when[i] < flood_persecond->value) {
			cl->flood_locktill = level.time + flood_waitdelay->value;
			gi.cprintf(ent, PRINT_CHAT, "Flood protection:  You can't talk for %d seconds.\n",
				(int)flood_waitdelay->value);
            return;
        }
		cl->flood_whenhead = (cl->flood_whenhead + 1) %
			(sizeof(cl->flood_when)/sizeof(cl->flood_when[0]));
		cl->flood_when[cl->flood_whenhead] = level.time;
	}

	if (dedicated->value)
		gi.cprintf(NULL, PRINT_CHAT, "%s", text);

	for (j = 1; j <= game.maxclients; j++)
	{
		other = &g_edicts[j];
		if (!other->inuse)
			continue;
		if (!other->client)
			continue;
		if (team)
		{
			if (!OnSameTeam(ent, other))
				continue;
		}
		gi.cprintf(other, PRINT_CHAT, "%s", text);
	}
}

/*
void Cmd_MonsterMove_f (edict_t *ent)
{
       vec3_t  start;
       vec3_t  forward;
       vec3_t  end;
       trace_t tr;
       edict_t *monster = NULL;

       VectorCopy(ent->s.origin, start);
       start[2] += ent->viewheight;
       AngleVectors(ent->client->v_angle, forward, NULL, NULL);
       VectorMA(start, 8192, forward, end);
       tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT);
       if ( tr.ent && (tr.ent->svflags & SVF_MONSTER))
       {
            gi.cprintf (ent, PRINT_HIGH, "Monster Awaiting move order\n");
            monster = tr.ent;
       }
       if (monster != NULL)
       {}
}

*/
void Cmd_MonsterMove_f (edict_t *ent)
{
       vec3_t  start;
       vec3_t  forward;
       vec3_t  end;
       trace_t tr;

       gi.cprintf (ent, PRINT_HIGH, "Monster Awaiting move order\n");
       VectorCopy(ent->s.origin, start);
       start[2] += ent->viewheight;
       AngleVectors(ent->client->v_angle, forward, NULL, NULL);
       VectorMA(start, 8192, forward, end);
       tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT);
       if ( tr.ent && ((tr.ent->svflags & SVF_MONSTER) || (tr.ent->client)) )
       {
               VectorScale(forward, 5000, forward);
               VectorAdd(forward, tr.ent->velocity, tr.ent->velocity);
       }
}





void Cmd_PlayerList_f(edict_t *ent)
{
	int i;
	char st[80];
	char text[1400];
	edict_t *e2;

	// connect time, ping, score, name
	*text = 0;
	for (i = 0, e2 = g_edicts + 1; i < maxclients->value; i++, e2++) {
		if (!e2->inuse)
			continue;

		Com_sprintf(st, sizeof(st), "%02d:%02d %4d %3d %s%s\n",
			(level.framenum - e2->client->resp.enterframe) / 600,
			((level.framenum - e2->client->resp.enterframe) % 600)/10,
			e2->client->ping,
			e2->client->resp.score,
			e2->client->pers.netname,
			e2->client->resp.spectator ? " (spectator)" : "");
		if (strlen(text) + strlen(st) > sizeof(text) - 50) {
			sprintf(text+strlen(text), "And more...\n");
			gi.cprintf(ent, PRINT_HIGH, "%s", text);
			return;
		}
		strcat(text, st);
	}
	gi.cprintf(ent, PRINT_HIGH, "%s", text);
}

void SpawnWhat(edict_t *ent)
{
	char		*s;
        s = gi.args();

        if (Q_stricmp(s, "quad") == 0)
        SP_boobytrap (ent, 1);
        else
        if (Q_stricmp(s, "megahealth") == 0)
        SP_boobytrap (ent, 2);
        else
        if (Q_stricmp(s, "barrel") == 0)
        SP_boobytrap (ent, 3);
        else
        SP_boobytrap (ent, 0);
}

void SpecialWhat(edict_t *ent)
{
	char		*s;
        s = gi.args();

        if (Q_stricmp(s, "quad") == 0)  {
        ent->specialselect = 1;
        gi.cprintf (ent, PRINT_HIGH, "You will spawn with perma quad when you die\n");
        }
        else
        if (Q_stricmp(s, "regeneration") == 0) {
        ent->specialselect = 2;
        gi.cprintf (ent, PRINT_HIGH, "You will regenerate yourself after you die\n");
        }
        else
        if (Q_stricmp(s, "ammo") == 0){
        ent->specialselect = 3;
        gi.cprintf (ent, PRINT_HIGH, "You will spawn with full ammo after you die\n");
        }
        else
        ent->specialselect = 0;
}


/*
=================
ClientCommand
=================
*/
void ClientCommand (edict_t *ent)
{
	char	*cmd;
        int i,lasku;
	vec3_t	vieworg;
        edict_t *client;
	gitem_t		*item;

	if (!ent->client)
		return;		// not fully in game yet

	cmd = gi.argv(0);


                if (Q_stricmp (cmd, "say") == 0)
	{
		Cmd_Say_f (ent, false, false);
		return;
	}
	if (Q_stricmp (cmd, "say_team") == 0)
	{
		Cmd_Say_f (ent, true, false);
		return;
	}

	if (Q_stricmp (cmd, "players") == 0)
	{
		Cmd_Players_f (ent);
		return;
	}

        if (Q_stricmp (cmd, "id") == 0)
	{
        Cmd_id_f (ent);
        return;
        }

        if (Q_stricmp (cmd, "score") == 0)
	{
		Cmd_Score_f (ent);
		return;
	}
	if (Q_stricmp (cmd, "help") == 0)
	{
		Cmd_Help_f (ent);
		return;
	}
        if (Q_stricmp (cmd, "special") == 0)
	{
        SpecialWhat(ent);
		return;
	}

        if (Q_stricmp (cmd, "nogameleader") == 0)
	{
        ent->nolead += 1;
        if (ent->nolead > 1) ent->nolead = 0;
        if (ent->nolead == 1) {
        gi.cprintf(ent, PRINT_HIGH, "Now you can't be selected as gameleader(This map).\n");

        }
        else
        gi.cprintf(ent, PRINT_HIGH, "Now you can be selected as gameleader.\n");

        return;
        }
      //  if (level.timeron == 3 | ((int)dmflags->value & DF_NOAUTOASSIGN))
	//RaVeN fixed typo   || was intended ?

       if (level.timeron == 3 || ((int)dmflags->value & DF_NOAUTOASSIGN))
      
	if (Q_stricmp (cmd, "gameleader") == 0)
	{
                if (ent->player == 1) {
               gi.cprintf (ent, PRINT_HIGH, "You are already a player.\n");
                return;
                }

            if (level.gameleader == 1 )
               gi.cprintf (ent, PRINT_HIGH, "There can be only one GameLeader.\n");
               else
               {
	for (i=0 ; i<maxclients->value ; i++)
	{
		client = g_edicts + 1 + i;
		if (!client->inuse)
			continue;
                gi.cprintf (client, PRINT_HIGH, "Gameleader is %s.\n",ent->client->pers.netname);

	}
        	ent->movetype = MOVETYPE_NOCLIP;
                ent->gameleader = 1;
                ent->player = 0;
                ent->s.effects |= EF_TELEPORTER;
                ent->svflags &= ~SVF_NOCLIENT;

                level.gameleader = 1;
               }
        	return;
	}

       if ((level.timeron == 3 || ((int)dmflags->value & DF_NOAUTOASSIGN)) && (ent->gameleader > 0 || ent->gameleader != 2)) 
        if (Q_stricmp (cmd, "player") == 0)
	{
        if (ent->gameleader > 0) return;
                if (ent->gameleader > 0) {
                gi.cprintf(ent, PRINT_HIGH, "You were Gameleader, changed to player.\n");
                ent->player = 1;
                ent->gameleader = 0;
                level.gameleader = 0;
                ent->svflags &= ~SVF_NOCLIENT;
                respawn (ent);
                return;
                }


                if (ent->player > 0) {
                gi.cprintf(ent, PRINT_HIGH, "You are already a player.\n");
                return;
                }
	for (i=0 ; i<maxclients->value ; i++)
	{
		client = g_edicts + 1 + i;
		if (!client->inuse)
			continue;
                        gi.cprintf(client, PRINT_HIGH, "%s is now player.\n",ent->client->pers.netname);
	}

                ent->player = 1;
                respawn (ent);
        	return;
	}




//Gameleader alkaa t„st„

        if (ent->gameleader > 0) {

        if (Q_stricmp (cmd, "glicarus") == 0)
	{


        if (ent->gameleader == 2)
        {
        gi.cprintf(ent, PRINT_HIGH, "Returning to normal state.\n");
        ent->gameleader = 1;
        ent->timetospawn = level.time + 25;
     
        item = FindItem("Cells");
        ent->client->pers.weapon = item;
        ent->client->newweapon = item;

        ent->solid = SOLID_NOT;
        ent->movetype = MOVETYPE_NOCLIP;
        ent->client->ps.gunindex = 0;
        ent->client->weapon_sound = 0;
        ent->health = 100;

        return;
        }


        if (ent->timetospawn < level.time)
        {

	VectorAdd (ent->s.origin, ent->client->ps.viewoffset, vieworg);

        if (gi.pointcontents(vieworg) & CONTENTS_SOLID)
        {
        gi.cprintf(ent, PRINT_HIGH, "You cannot spawn outside level!\n");
        return;
        }

        
        if (EnoughStuff(ent,15,"Cells", 150)) {
        ent->gameleader = 2;
        ent->health = 100;
	ent->solid = SOLID_BBOX;
        ent->movetype = MOVETYPE_WALK;
	ent->viewheight = 22;
        ent->takedamage = DAMAGE_AIM;
        item = FindItem("HyperBlaster");
        ent->client->pers.weapon = item;
        ent->client->newweapon = item;
        if (ent->client->pers.weapon == item)
        {
        ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);
        }

        if (ent->client->pers.selected_item == ITEM_INDEX(item))
        {
        ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);
        }
        ent->client->pers.selected_item = ITEM_INDEX(item);
			Think_Weapon (ent);
                        ChangeWeapon (ent);
        return;
        }
        else
        return;

        }
        else {
        lasku = ent->timetospawn - level.time;
        gi.cprintf(ent, PRINT_HIGH, "Your mental energies are recharging ,wait for : %i seconds.\n",lasku);
        return;
        }
        }

        

        if (Q_stricmp (cmd, "thup") == 0)
	{
         ent->velocity[2] = ent->velocity[2] + 30;
         if (ent->velocity[2] > 300) ent->velocity[2] = 300;
        
      //  if (ent->thrust < 100) ent->thrust += 10;
      //  gi.cprintf(ent, PRINT_HIGH, "Thrust at: %i.\n",ent->thrust);
        return;
        }

        if (Q_stricmp (cmd, "thdown") == 0)
	{
       // if (ent->thrust > -100) ent->thrust = ent->thrust - 10;
       // gi.cprintf(ent, PRINT_HIGH, "Thrust at: %i.\n",ent->thrust);
       // return;
        }

        if (Q_stricmp (cmd, "move") == 0)
        {
        Cmd_MonsterMove_f (ent);
        return;
        }
        if (Q_stricmp (cmd, "trap") == 0)
	{
        if (EnoughStuff2(ent,10,"Cells"))
        SpawnWhat(ent);
		return;
	}

        if (Q_stricmp (cmd, "teleporter") == 0)
	{
        if (EnoughStuff(ent,50,"Cells",300))
        SP_func_DoomSpawngl (ent);
		return;
	}

        if (Q_stricmp (cmd, "mine") == 0)
	{
        if (EnoughStuff2(ent,2,"Cells"))
        SP_minetrap (ent,0);
		return;
	}


        if (Q_stricmp (cmd, "push") == 0)
	{
        if (EnoughCells(ent,5,"Cells"))
        Cmd_Push_f(ent);
                return;
	}

        if (Q_stricmp (cmd, "berserk") == 0)
	{
        if (EnoughStuff(ent,5,"Cells", 175))
                SP_monster_berserk2(ent);
		return;
	}
        if (Q_stricmp (cmd, "makron") == 0)
	{
        if (EnoughStuff(ent,130,"Cells", 200))
                SP_monster_makron2(ent);
		return;
	}

        if (Q_stricmp (cmd, "jorg") == 0)
	{
        if (EnoughStuff(ent,300,"Cells", 225))
                SP_monster_jorg2(ent);
		return;
	}

        if (Q_stricmp (cmd, "flyboss") == 0)
	{
        if (EnoughStuff(ent,80,"Cells", 225))
                SP_monster_boss02(ent);
		return;
	}

        if (Q_stricmp (cmd, "icarus") == 0)
	{
        if (EnoughStuff(ent,12,"Cells", 175))
                SP_monster_hover2(ent);
		return;
	}

        if (Q_stricmp (cmd, "target") == 0)
	{
        Cmd_Select_f (ent);
        return;
        }

        if (Q_stricmp (cmd, "escort") == 0)
	{
        Cmd_Select2_f (ent);
        return;
        }

        if (Q_stricmp (cmd, "patrol") == 0)
	{
        Cmd_Patrol_f (ent);
        return;
        }

        if (Q_stricmp (cmd, "follow") == 0)
	{
          if (ent->follow == 0) {
          gi.cprintf (ent, PRINT_HIGH, "Monsters follow turned on.\n");
          ent->follow = 1;
          }
          else
          {
          gi.cprintf (ent, PRINT_HIGH, "Monsters follow turned off.\n");
          ent->follow = 0;

           }
          return;
          }

        if (Q_stricmp (cmd, "soldier_ss") == 0)
	{
        if (EnoughStuff(ent,4,"Cells", 175))
                SP_monster_soldier_ss2(ent);
                 return;
	}
        if (Q_stricmp (cmd, "flipper") == 0)
	{
        if (EnoughStuff(ent,5,"Cells", 175))
                SP_monster_flipper2(ent);
                 return;
	}
        if (Q_stricmp (cmd, "supertank") == 0)
	{
        if (EnoughStuff(ent,50,"Cells", 250))
                SP_monster_supertank2(ent);
                 return;
	}
        if (Q_stricmp (cmd, "tank") == 0)
	{
        if (EnoughStuff(ent,25,"Cells", 225))
                SP_monster_tank2(ent, 0);
                 return;
	}
        if (Q_stricmp (cmd, "tankcommander") == 0)
	{
        if (EnoughStuff(ent,35,"Cells", 230))
                SP_monster_tank2(ent, 1);
                 return;
	}
        if (Q_stricmp (cmd, "medic") == 0)
	{
        if (EnoughStuff(ent,15,"Cells", 175))
                SP_monster_medic2(ent);
                 return;
	}

        if (Q_stricmp (cmd, "mutant") == 0)
	{
        if (EnoughStuff(ent,20,"Cells", 200))
                SP_monster_mutant2(ent);
                 return;
	}

        if (Q_stricmp (cmd, "soldier") == 0)
	{
        if (EnoughStuff(ent,4,"Cells", 175))
                SP_monster_soldier2(ent);
                 return;
	}

        if (Q_stricmp (cmd, "soldier_light") == 0)
	{
        if (EnoughStuff(ent,2,"Cells", 175))
                SP_monster_soldier_light2(ent);
                 return;
	}


        if (Q_stricmp (cmd, "parasite") == 0)
	{
        if (EnoughStuff(ent,8,"Cells", 175))
                SP_monster_parasite2(ent);
                 return;
	}
        if (Q_stricmp (cmd, "enforcer") == 0)
	{
        if (EnoughStuff(ent,5,"Cells", 175))
                SP_monster_infantry2(ent);
                 return;
	}
        if (Q_stricmp (cmd, "brains") == 0)
	{
        if (EnoughStuff(ent,8,"Cells", 175))
                SP_monster_brain2(ent);
		return;
	}
        if (Q_stricmp (cmd, "chick") == 0)
        {                          
        if (EnoughStuff(ent,10,"Cells", 175))
                SP_monster_chick2(ent);
		return;
	}
        if (Q_stricmp (cmd, "gunner") == 0)
	{
        if (EnoughStuff(ent,10,"Cells", 175))
                SP_monster_gunner2(ent);
		return;
	}
        if (Q_stricmp (cmd, "gladiator") == 0)
	{
        if (EnoughStuff(ent,20,"Cells", 175))
                SP_monster_gladiator2(ent);
		return;
	}
        if (Q_stricmp (cmd, "flyer") == 0)
	{
        if (EnoughStuff(ent,6,"Cells", 175))
                SP_monster_flyer2(ent);
		return;
	}
        if (Q_stricmp (cmd, "floater") == 0)
	{
        if (EnoughStuff(ent,7,"Cells", 175))
                SP_monster_floater2(ent);
		return;
	}

 } // gamemaster sulku       

	if (level.intermissiontime)
		return;

        if (Q_stricmp (cmd, "use") == 0)
		Cmd_Use_f (ent);

        else if ((ent->player == 1) && (Q_stricmp (cmd, "drop") == 0)) {
		Cmd_Drop_f (ent);
                }


        else if (Q_stricmp (cmd, "give") == 0)
		Cmd_Give_f (ent);
	else if (Q_stricmp (cmd, "god") == 0)
		Cmd_God_f (ent);
	else if (Q_stricmp (cmd, "notarget") == 0)
		Cmd_Notarget_f (ent);
        /*
        else if (Q_stricmp (cmd, "noclip") == 0)
		Cmd_Noclip_f (ent);
        */
        else if (Q_stricmp (cmd, "inven") == 0)
		Cmd_Inven_f (ent);
	else if (Q_stricmp (cmd, "invnext") == 0)
		SelectNextItem (ent, -1);
	else if (Q_stricmp (cmd, "invprev") == 0)
		SelectPrevItem (ent, -1);
	else if (Q_stricmp (cmd, "invnextw") == 0)
		SelectNextItem (ent, IT_WEAPON);
	else if (Q_stricmp (cmd, "invprevw") == 0)
		SelectPrevItem (ent, IT_WEAPON);
	else if (Q_stricmp (cmd, "invnextp") == 0)
		SelectNextItem (ent, IT_POWERUP);
	else if (Q_stricmp (cmd, "invprevp") == 0)
		SelectPrevItem (ent, IT_POWERUP);
	else if (Q_stricmp (cmd, "invuse") == 0)
		Cmd_InvUse_f (ent);
        else if ((ent->player == 1) && Q_stricmp (cmd, "invdrop") == 0)
		Cmd_InvDrop_f (ent);
	else if (Q_stricmp (cmd, "weapprev") == 0)
		Cmd_WeapPrev_f (ent);
	else if (Q_stricmp (cmd, "weapnext") == 0)
		Cmd_WeapNext_f (ent);
	else if (Q_stricmp (cmd, "weaplast") == 0)
		Cmd_WeapLast_f (ent);
        else if (Q_stricmp (cmd, "kill") == 0)
		Cmd_Kill_f (ent);
        else if (Q_stricmp (cmd, "putaway") == 0)
		Cmd_PutAway_f (ent);
	else if (Q_stricmp (cmd, "wave") == 0)
		Cmd_Wave_f (ent);
	else if (Q_stricmp(cmd, "playerlist") == 0)
		Cmd_PlayerList_f(ent);
        else if (Q_stricmp (cmd, "maplist") == 0) 
                Cmd_Maplist_f (ent); 
        else if (Q_stricmp (cmd, "flashlight") == 0) {
        if (ent->player > 0)  {
		FL_make (ent);
                }
                }

        else	// anything that doesn't match a command will be a chat
		Cmd_Say_f (ent, false, true);
}
