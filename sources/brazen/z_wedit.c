#include "g_local.h"
#include "m_player.h"

void CheckCrossHair(edict_t *ent)
{
        vec3_t forward, right, offset, start, end, pos;
	edict_t *foundEnt = NULL;
	trace_t	tr;
	
	VectorSet(offset, 0, 0, ent->viewheight);
        AngleVectors (ent->client->v_angle, forward, right, NULL);
        P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorMA (start, 8192, forward, end);
	
	tr = gi.trace (start, NULL, NULL, end, ent, MASK_ALL);

	if (tr.ent && tr.ent->classname)
	{
		if (Q_stricmp(tr.ent->classname, "worldspawn") != 0)
			foundEnt = tr.ent;
	}
	/*
	if (!foundEnt)
	{
		edict_t	*item = NULL;
		float best, dist;
		
		best = 999;

		while ((item = findradius(item, tr.endpos, 16)) != NULL)
		{
			if (item->svflags & SVF_MONSTER)
				continue;
			if (item->solid != SOLID_TRIGGER)
				continue;
			VectorSubtract (tr.endpos, item->s.origin, pos);
			dist = VectorLength (pos);
			if (dist < best)
			{
				best = dist;
				foundEnt = item;
			}
		}
	}
	*/

	if (foundEnt)
	{
		char *targetname = "none";
		char *target = "none";
		char *message = "none";
		char sflags[384];
		char *model = "";

		Com_sprintf (sflags, sizeof(sflags), "(spawnflags %i)\n", foundEnt->spawnflags);
		
		if (foundEnt->spawnflags & SPAWNFLAG_NOT_DEATHMATCH)
			strcat(sflags, "*NOT DEATHMATCH*");
		if ((foundEnt->spawnflags & SPAWNFLAG_NOT_EASY)
		 && (foundEnt->spawnflags & SPAWNFLAG_NOT_MEDIUM)
		  && (foundEnt->spawnflags & SPAWNFLAG_NOT_HARD))
			strcat(sflags, "DEATHMATCH ONLY");
		else
		{
			if (foundEnt->spawnflags & SPAWNFLAG_NOT_EASY)
				strcat(sflags, "*NOT EASY*");
			if (foundEnt->spawnflags & SPAWNFLAG_NOT_MEDIUM)
				strcat(sflags, "*NOT MED*");
			if (foundEnt->spawnflags & SPAWNFLAG_NOT_HARD)
				strcat(sflags, "*NOT HARD*");
		}
				
		VectorCopy(foundEnt->s.origin, pos);

		if (foundEnt->target)
			target = foundEnt->target;

		if (foundEnt->targetname)
			targetname = foundEnt->targetname;
		if (foundEnt->model)
			model = foundEnt->model;
			
		if (foundEnt->svflags & SVF_MONSTER)
		{
			char *item = "none";
			
			if (foundEnt->item != NULL)
				item = foundEnt->item->classname;

			gi.centerprintf (ent, "foundEnt = %s\n origin = %s\n sFlags = %s\n item = %s\ntarget = %s\ntargetname = %s\n", foundEnt->classname, vtos(pos), sflags, item, target, targetname);
			return;
		}
		else if (foundEnt->message)
			message = foundEnt->message;

		gi.centerprintf (ent, "foundEnt = %s\n origin = %s\nsFlags = %s\n model = %s\n target = %s\ntargetname = %s\nmessage = %s\n", foundEnt->classname, vtos(pos), sflags, model, target, targetname, message);
		return;
	}

	vectoangles(tr.plane.normal, pos);
	gi.centerprintf (ent, "X-hair pos = %s\n normal = %s\n self angles =%s\n", vtos(tr.endpos), vtos(pos), vtos(ent->client->v_angle));
}

/*
=================
Think_WEdit

=================
*/
void Cmd_Noclip_f (edict_t *ent);

void Think_WEdit(edict_t *ent)
{
        int newState = -1;
        int newFrame = -1;
        qboolean bAttack = ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK);
        qboolean bAltAttack = ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ALT_ATTACK);
        
        switch(ent->client->weaponstate)
        {
        case W_READY:
                if (ent->client->newRightWeapon > -1)
                {
                        newFrame = 53;
                        newState = W_RIGHT_DROPPING;
                }                
                else if (ent->client->newLeftWeapon > -1)
                {
                        ent->client->ps.gunframe = 9;
                        ChangeLeftWeapon(ent);
                        return;
                }
                else if (bAttack)
                {
                        ent->client->latched_buttons &= ~BUTTON_ATTACK;
			ent->client->ps.gunframe = 5;
			newState = W_FIRING;
			CheckCrossHair(ent);
                }
                else if (bAltAttack)
                {
                        ent->client->latched_buttons &= ~BUTTON_ALT_ATTACK;
			ent->client->ps.gunframe = 5;
			newState = W_ALT_FIRING;
			Cmd_Noclip_f (ent);
                }
                else if (level.time - ent->last_fire > IDLE_DELAY)
                {
                        newFrame = 10;
                        newState = W_IDLE;
                }
                else
                        newFrame = 9;
                break;

        case W_IDLE:
                if (ent->client->newRightWeapon > -1)
                {
                        newFrame = 53;
                        newState = W_RIGHT_DROPPING;
                }                
                else if (ent->client->newLeftWeapon > -1)
                {
                        newFrame = 9;
                        ChangeLeftWeapon(ent);
                        return;
                }
                else if (bAttack || bAltAttack)
                {
                        ent->client->weaponstate = W_READY;
			ent->client->ps.gunframe = 9;
                        Think_WEdit(ent);
                        return;
                }
                break;

        default:
                break;
        }

        switch(ent->client->ps.gunframe)
        {
        case 4: // ready now, so think again straight away in case we want to fire fast
                ent->client->weaponstate = W_READY;// do nothing... we're ready, not idle
		ent->client->ps.gunframe = 9;
                Think_WEdit(ent);

		if (!(ent->flags & FL_GODMODE))
			ent->flags ^= FL_GODMODE;

		if (!(ent->flags & FL_NOTARGET))
			ent->flags ^= FL_NOTARGET;
                return;
                break;

	case 8:
                if (bAttack || bAltAttack)
                	return;
                	
                ent->client->weaponstate = W_READY;// do nothing... we're ready, not idle
		ent->client->ps.gunframe = 9;
                break;

	case 9:
		break;
                
        case 52:
                if (random() < 0.5)
                        newFrame = 10;
                else
                {
                        newFrame = 9;
                        newState = W_READY;
                }
                break;

        case 55:
                if (ent->client->newRightWeapon > -1)
                        ChangeRightWeapon(ent);
                break;

        default:
                if (newFrame == -1)
                        ent->client->ps.gunframe++;
                break;
        }

        if (newFrame != -1)
                ent->client->ps.gunframe = newFrame;

        if (newState != -1)
                ent->client->weaponstate = newState;
}
