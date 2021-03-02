#include "g_local.h"


#define TRIGGER_MONSTER		0x01
#define TRIGGER_NOT_PLAYER	0x02
#define TRIGGER_TRIGGERED	0x04
#ifdef ROGUE //- some of these are mine, some id's. I added the define's.
#define TRIGGER_TOGGLE		0x08
#endif //ROGUE

void InitTrigger (edict_t *self)
{
	if (!VectorCompare (self->s.angles, vec3_origin))
		G_SetMovedir (self->s.angles, self->movedir);

	self->solid = SOLID_TRIGGER;
	self->movetype = MOVETYPE_NONE;
	gi.setmodel (self, self->model);
	self->svflags = SVF_NOCLIENT;
}


// the wait time has passed, so set back up for another activation
void multi_wait (edict_t *ent)
{
	ent->nextthink = 0;
}


// the trigger was just activated
// ent->activator should be set to the activator so it can be held through a delay
// so wait for the delay time before firing
void multi_trigger (edict_t *ent)
{
	if (ent->nextthink)
		return;		// already been triggered

	G_UseTargets (ent, ent->activator);

	if (ent->wait > 0)	
	{
		ent->think = multi_wait;
		ent->nextthink = level.time + ent->wait;
	}
	else
	{	// we can't just remove (self) here, because this is a touch function
		// called while looping through area links...
		ent->touch = NULL;
		ent->nextthink = level.time + FRAMETIME;
		ent->think = G_FreeEdict;
	}
}

void Use_Multi (edict_t *ent, edict_t *other, edict_t *activator)
{
#ifdef ROGUE
	if(ent->spawnflags & TRIGGER_TOGGLE)
	{
		if(ent->solid == SOLID_TRIGGER)
			ent->solid = SOLID_NOT;
		else
			ent->solid = SOLID_TRIGGER;
		gi.linkentity (ent);
	}
	else
#endif //ROGUE
	{
		ent->activator = activator;
		multi_trigger (ent);
	}
}

void Touch_Multi (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if(other->client)
	{
		if (self->spawnflags & 2)
			return;
	}
	else if (other->svflags & SVF_MONSTER)
	{
		if (!(self->spawnflags & 1))
			return;
	}
	else
		return;

	if (!VectorCompare(self->movedir, vec3_origin))
	{
		vec3_t	forward;

		AngleVectors(other->s.angles, forward, NULL, NULL);
		if (_DotProduct(forward, self->movedir) < 0)
			return;
	}

	self->activator = other;
	multi_trigger (self);
}

/*QUAKED trigger_multiple (.5 .5 .5) ? MONSTER NOT_PLAYER TRIGGERED
Variable sized repeatable trigger.  Must be targeted at one or more entities.
If "delay" is set, the trigger waits some time after activating before firing.
"wait" : Seconds between triggerings. (.2 default)
sounds
1)	secret
2)	beep beep
3)	large switch
4)
set "message" to text string
*/
void trigger_enable (edict_t *self, edict_t *other, edict_t *activator)
{
	self->solid = SOLID_TRIGGER;
	self->use = Use_Multi;
	gi.linkentity (self);
}

void SP_trigger_multiple (edict_t *ent)
{
	if (ent->sounds == 1)
		ent->noise_index = gi.soundindex ("misc/secret.wav");
	else if (ent->sounds == 2)
		ent->noise_index = gi.soundindex ("misc/talk.wav");
	else if (ent->sounds == 3)
		ent->noise_index = gi.soundindex ("misc/trigger1.wav");
	
	if (!ent->wait)
		ent->wait = 0.2;
	ent->touch = Touch_Multi;
	ent->movetype = MOVETYPE_NONE;
	ent->svflags |= SVF_NOCLIENT;


	if (ent->spawnflags & (TRIGGER_TRIGGERED
#ifdef ROGUE
				| TRIGGER_TOGGLE)
#endif //ROGUE
			)
	{
		ent->solid = SOLID_NOT;
		ent->use = trigger_enable;
	}
	else
	{
		ent->solid = SOLID_TRIGGER;
		ent->use = Use_Multi;
	}

	if (!VectorCompare(ent->s.angles, vec3_origin))
		G_SetMovedir (ent->s.angles, ent->movedir);

	gi.setmodel (ent, ent->model);
	gi.linkentity (ent);
}


/*QUAKED trigger_once (.5 .5 .5) ? x x TRIGGERED
Triggers once, then removes itself.
You must set the key "target" to the name of another object in the level that has a matching "targetname".

If TRIGGERED, this trigger must be triggered before it is live.

sounds
 1)	secret
 2)	beep beep
 3)	large switch
 4)

"message"	string to be displayed when triggered
*/

void SP_trigger_once(edict_t *ent)
{
	// make old maps work because I messed up on flag assignments here
	// triggered was on bit 1 when it should have been on bit 4
	if (ent->spawnflags & 1)
	{
		vec3_t	v;

		VectorMA (ent->mins, 0.5, ent->size, v);
		ent->spawnflags &= ~1;
		ent->spawnflags |= 4;
		gi.dprintf("fixed TRIGGERED flag on %s at %s\n", ent->classname, vtos(v));
	}

	ent->wait = -1;
	SP_trigger_multiple (ent);
}

/*QUAKED trigger_relay (.5 .5 .5) (-8 -8 -8) (8 8 8)
This fixed size trigger cannot be touched, it can only be fired by other events.
*/
void trigger_relay_use (edict_t *self, edict_t *other, edict_t *activator)
{
#ifdef TRIGGER_COUNTING
	if (self->style) self->count = self->style;
	else self->count = other->count;
#endif
	G_UseTargets (self, activator);
}

void SP_trigger_relay (edict_t *self)
{
	self->use = trigger_relay_use;
}


/*
==============================================================================

trigger_key

==============================================================================
*/

/*QUAKED trigger_key (.5 .5 .5) (-8 -8 -8) (8 8 8)
A relay trigger that only fires it's targets if player has the proper key.
Use "item" to specify the required key, for example "key_data_cd"
*/
void trigger_key_use (edict_t *self, edict_t *other, edict_t *activator)
{
	int			index;

	if (!self->item)
		return;
	if (!activator->client)
		return;

	index = ITEM_INDEX(self->item);
	if (!activator->client->pers.inventory[index])
	{
		if (level.time < self->touch_debounce_time)
			return;
		self->touch_debounce_time = level.time + 5.0;
		gi.centerprintf (activator, "You need the %s", self->item->pickup_name);
		gi.sound (activator, CHAN_AUTO, gi.soundindex ("misc/keytry.wav"), 1, ATTN_NORM, 0);
		return;
	}

	gi.sound (activator, CHAN_AUTO, gi.soundindex ("misc/keyuse.wav"), 1, ATTN_NORM, 0);
	if (coop->value)
	{
		int		player;
		edict_t	*ent;

		if (strcmp(self->item->classname, "key_power_cube") == 0)
		{
			int	cube;

			for (cube = 0; cube < 8; cube++)
				if (activator->client->pers.power_cubes & (1 << cube))
					break;
			for (player = 1; player <= game.maxclients; player++)
			{
				ent = &g_edicts[player];
				if (!ent->inuse)
					continue;
				if (!ent->client)
					continue;
				if (ent->client->pers.power_cubes & (1 << cube))
				{
					ent->client->pers.inventory[index]--;
					ent->client->pers.power_cubes &= ~(1 << cube);
				}
			}
		}
		else
		{
			for (player = 1; player <= game.maxclients; player++)
			{
				ent = &g_edicts[player];
				if (!ent->inuse)
					continue;
				if (!ent->client)
					continue;
				ent->client->pers.inventory[index] = 0;
			}
		}
	}
	else
	{
		activator->client->pers.inventory[index]--;
	}

	G_UseTargets (self, activator);

	self->use = NULL;
}

void SP_trigger_key (edict_t *self)
{
	if (!st.item)
	{
		gi.dprintf("no key item for trigger_key at %s\n", vtos(self->s.origin));
		return;
	}
	self->item = FindItemByClassname (st.item);

	if (!self->item)
	{
		gi.dprintf("item %s not found for trigger_key at %s\n", st.item, vtos(self->s.origin));
		return;
	}

	if (!self->target)
	{
		gi.dprintf("%s at %s has no target\n", self->classname, vtos(self->s.origin));
		return;
	}

	gi.soundindex ("misc/keytry.wav");
	gi.soundindex ("misc/keyuse.wav");

	self->use = trigger_key_use;
}


/*
==============================================================================

trigger_counter

==============================================================================
*/

/*QUAKED trigger_counter (.5 .5 .5) ? nomessage
Acts as an intermediary for an action that takes multiple inputs.

If nomessage is not set, t will print "1 more.. " etc when triggered and "sequence complete" when finished.

After the counter has been triggered "count" times (default 2), it will fire all of it's targets and remove itself.
*/

void trigger_counter_use(edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->count == 0)
		return;
	
	self->count--;

	if (self->count)
	{
		if (! (self->spawnflags & 1))
		{
			gi.centerprintf(activator, "%i more to go...", self->count);
			gi.sound (activator, CHAN_AUTO, gi.soundindex ("misc/talk1.wav"), 1, ATTN_NORM, 0);
		}
		return;
	}
	
	if (! (self->spawnflags & 1))
	{
		gi.centerprintf(activator, "Sequence completed!");
		gi.sound (activator, CHAN_AUTO, gi.soundindex ("misc/talk1.wav"), 1, ATTN_NORM, 0);
	}
	self->activator = activator;
	multi_trigger (self);
}

void SP_trigger_counter (edict_t *self)
{
	self->wait = -1;
	if (!self->count)
		self->count = 2;

	self->use = trigger_counter_use;
}


/*
==============================================================================

trigger_always

==============================================================================
*/

/*QUAKED trigger_always (.5 .5 .5) (-8 -8 -8) (8 8 8)
This trigger will always fire.  It is activated by the world.
*/
void SP_trigger_always (edict_t *ent)
{
	// we must have some delay to make sure our use targets are present
	if (ent->delay < 0.2)
		ent->delay = 0.2;
	G_UseTargets(ent, ent);
}


/*
==============================================================================

trigger_push

==============================================================================
*/

#define PUSH_ONCE			0x01
#ifdef ROGUE
#define PUSH_START_OFF	0x02
#define PUSH_SILENT		0x04
#endif //ROGUE

static int windsound;

void trigger_push_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (strcmp(other->classname, "grenade") == 0)
	{
		VectorScale (self->movedir, self->speed * 10, other->velocity);
	}
	else if (other->health > 0)
	{	
		VectorScale (self->movedir, self->speed * 10, other->velocity);

		if (other->client)
		{
			// don't take falling damage immediately from this
			VectorCopy (other->velocity, other->client->oldvelocity);
			if (other->fly_sound_debounce_time < level.time)
			{
				other->fly_sound_debounce_time = level.time + 1.5;
				gi.sound (other, CHAN_AUTO, windsound, 1, ATTN_NORM, 0);
			}
		}
	}
	if (self->spawnflags & PUSH_ONCE)
	G_FreeEdict (self);
}

#ifdef ROGUE
void trigger_push_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->solid == SOLID_NOT)
		self->solid = SOLID_TRIGGER;
	else
		self->solid = SOLID_NOT;
	gi.linkentity (self);
}
#endif //ROGUE

#ifdef XATRIX
// RAFAEL

/*QUAKED trigger_push (.5 .5 .5) ? PUSH_ONCE PUSH_PLUS PUSH_RAMP
Pushes the player
"speed"  defaults to 1000
"wait"  defaults to 10 must use PUSH_PLUS  used for on
*/

void trigger_push_active (edict_t *self);

void trigger_effect (edict_t *self)
{
	vec3_t	origin;
	vec3_t	size;
	int		i;
	
	VectorScale (self->size, 0.5, size);
	VectorAdd (self->absmin, size, origin);
	
	for (i=0; i<10; i++)
	{
		origin[2] += (self->speed * 0.01) * (i + random());
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_TUNNEL_SPARKS);
		gi.WriteByte (1);
		gi.WritePosition (origin);
		gi.WriteDir (vec3_origin);
		gi.WriteByte (0x74 + (rand()&7));
		gi.multicast (self->s.origin, MULTICAST_PVS);
	}

}

void trigger_push_inactive (edict_t *self)
{
	if (self->delay > level.time)
	{
		self->nextthink = level.time + 0.1;
	}
	else
	{
		self->touch = trigger_push_touch;
		self->think = trigger_push_active;
		self->nextthink = level.time + 0.1;
		self->delay = self->nextthink + self->wait;  
	}
}

void trigger_push_active (edict_t *self)
{
	if (self->delay > level.time)
	{
		self->nextthink = level.time + 0.1;
		trigger_effect (self);
	}
	else
	{
		self->touch = NULL;
		self->think = trigger_push_inactive;
		self->nextthink = level.time + 0.1;
		self->delay = self->nextthink + self->wait;  
	}
}

void SP_trigger_push (edict_t *self)
{
	InitTrigger (self);
	windsound = gi.soundindex ("misc/windfly.wav");
	self->touch = trigger_push_touch;
	
	if (self->spawnflags & 2)
	{
		if (!self->wait)
			self->wait = 10;
  
		self->think = trigger_push_active;
		self->nextthink = level.time + 0.1;
		self->delay = self->nextthink + self->wait;
	}

	if (!self->speed)
		self->speed = 1000;

#ifdef ROGUE
	if(self->targetname)		// toggleable
	{
		self->use = trigger_push_use;
		if(self->spawnflags & PUSH_START_OFF)
			self->solid = SOLID_NOT;
	}
	else if(self->spawnflags & PUSH_START_OFF)
	{
		gi.dprintf ("trigger_push is START_OFF but not targeted.\n");
		self->svflags = 0;
		self->touch = NULL;
		self->solid = SOLID_BSP;
		self->movetype = MOVETYPE_PUSH;
	}
#endif //ROGUE

	gi.linkentity (self);

}

#else //XATRIX

/*QUAKED trigger_push (.5 .5 .5) ? PUSH_ONCE
Pushes the player
"speed"		defaults to 1000
*/
void SP_trigger_push (edict_t *self)
{
	InitTrigger (self);
	windsound = gi.soundindex ("misc/windfly.wav");
	self->touch = trigger_push_touch;
	if (!self->speed)
		self->speed = 1000;

#ifdef ROGUE
	if(self->targetname)		// toggleable
	{
		self->use = trigger_push_use;
		if(self->spawnflags & PUSH_START_OFF)
			self->solid = SOLID_NOT;
	}
	else if(self->spawnflags & PUSH_START_OFF)
	{
		gi.dprintf ("trigger_push is START_OFF but not targeted.\n");
		self->svflags = 0;
		self->touch = NULL;
		self->solid = SOLID_BSP;
		self->movetype = MOVETYPE_PUSH;
	}
#endif //ROGUE

	gi.linkentity (self);
}

#endif //XATRIX


/*
==============================================================================

trigger_hurt

==============================================================================
*/

/*QUAKED trigger_hurt (.5 .5 .5) ? START_OFF TOGGLE SILENT NO_PROTECTION SLOW
Any entity that touches this will be hurt.

It does dmg points of damage each server frame

SILENT			supresses playing the sound
SLOW			changes the damage rate to once per second
NO_PROTECTION	*nothing* stops the damage

"dmg"			default 5 (whole numbers only)

*/
void hurt_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->solid == SOLID_NOT)
		self->solid = SOLID_TRIGGER;
	else
		self->solid = SOLID_NOT;
	gi.linkentity (self);

	if (!(self->spawnflags & 2))
		self->use = NULL;
}


void hurt_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	int		dflags;

	if (!other->takedamage)
		return;

	if (self->timestamp > level.time)
		return;

	if (self->spawnflags & 16)
		self->timestamp = level.time + 1;
	else
		self->timestamp = level.time + FRAMETIME;

	if (!(self->spawnflags & 4))
	{
		if ((level.framenum % 10) == 0)
			gi.sound (other, CHAN_AUTO, self->noise_index, 1, ATTN_NORM, 0);
	}

	if (self->spawnflags & 8)
		dflags = DAMAGE_NO_PROTECTION;
	else
		dflags = 0;
	T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, self->dmg, dflags, MOD_TRIGGER_HURT);
}

void SP_trigger_hurt (edict_t *self)
{
	InitTrigger (self);

	self->noise_index = gi.soundindex ("world/electro.wav");
	self->touch = hurt_touch;

	if (!self->dmg)
		self->dmg = 5;

	if (self->spawnflags & 1)
		self->solid = SOLID_NOT;
	else
		self->solid = SOLID_TRIGGER;

	if (self->spawnflags & 2)
		self->use = hurt_use;

	gi.linkentity (self);
}


/*
==============================================================================

trigger_gravity

==============================================================================
*/

/*QUAKED trigger_gravity (.5 .5 .5) ?
Changes the touching entites gravity to
the value of "gravity".  1.0 is standard
gravity for the level.
*/

#ifdef ROGUE
void trigger_gravity_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->solid == SOLID_NOT)
		self->solid = SOLID_TRIGGER;
	else
		self->solid = SOLID_NOT;
	gi.linkentity (self);
}
#endif //ROGUE

void trigger_gravity_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	other->gravity = self->gravity;
}

void SP_trigger_gravity (edict_t *self)
{
	if (st.gravity == 0)
	{
		gi.dprintf("trigger_gravity without gravity set at %s\n", vtos(self->s.origin));
		G_FreeEdict  (self);
		return;
	}

	InitTrigger (self);
#ifdef ROGUE
	self->gravity = atof(st.gravity);

	if(self->spawnflags & 1)				// TOGGLE
		self->use = trigger_gravity_use;

	if(self->spawnflags & 2)				// START_OFF
	{
		self->use = trigger_gravity_use;
		self->solid = SOLID_NOT;
	}
#else //ROGUE
	self->gravity = atoi(st.gravity);
#endif //ROGUE
	self->touch = trigger_gravity_touch;

	gi.linkentity (self);
}


/*
==============================================================================

trigger_monsterjump

==============================================================================
*/

/*QUAKED trigger_monsterjump (.5 .5 .5) ?
Walking monsters that touch this will jump in the direction of the trigger's angle
"speed" default to 200, the speed thrown forward
"height" default to 200, the speed thrown upwards
*/

void trigger_monsterjump_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other->flags & (FL_FLY | FL_SWIM) )
		return;
	if (other->svflags & SVF_DEADMONSTER)
		return;
	if ( !(other->svflags & SVF_MONSTER))
		return;

// set XY even if not on ground, so the jump will clear lips
	other->velocity[0] = self->movedir[0] * self->speed;
	other->velocity[1] = self->movedir[1] * self->speed;
	
	if (!other->groundentity)
		return;
	
	other->groundentity = NULL;
	other->velocity[2] = self->movedir[2];
}

void SP_trigger_monsterjump (edict_t *self)
{
	if (!self->speed)
		self->speed = 200;
	if (!st.height)
		st.height = 200;
	if (self->s.angles[YAW] == 0)
		self->s.angles[YAW] = 360;
	InitTrigger (self);
	self->touch = trigger_monsterjump_touch;
	self->movedir[2] = st.height;
}

#ifdef TRIGGER_LOG

/*
==============================================================================

trigger_log

==============================================================================
*/

/*QUAKED trigger_log (.5 .5 .5) ? nomessage
writes a message to the log file the first time triggered
can be retriggered after not being triggered for 0.2 seconds

"message"		message to print

*/

#define TRIGGER_STATE_ACTIVE			0
#define TRIGGER_STATE_NONACTIVE		1

void trigger_log_reset(edict_t *self)
{
	self->count = TRIGGER_STATE_ACTIVE;
} //end of the function trigger_log_reset

void trigger_log_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	//only clients
	if (!other->client) return;
	//
	self->nextthink = level.time + FRAMETIME * 2;
	self->think = trigger_log_reset;
	//if active output message
	if (self->count == TRIGGER_STATE_ACTIVE)
	{
		Log_WriteTimeStamped("%s", self->message);
	} //end if
	self->count = TRIGGER_STATE_NONACTIVE;
} //end of the function trigger_log_touch

void SP_trigger_log(edict_t *self)
{
	InitTrigger(self);

	self->count = TRIGGER_STATE_ACTIVE;
	self->touch = trigger_log_touch;
} //end of the function SP_trigger_log

#endif //TRIGGER_LOG

#ifdef TRIGGER_COUNTING
/*
==============================================================================

trigger_counting

==============================================================================
*/

/*QUAKED trigger_counting (.5 .5 .5) ? nomessage
Acts as an intermediary for an action that takes multiple inputs.

"count"			initial count value (default 2)
"targetname"	doors with the same "target" field will be set in the correct state
"style"			state the door will be set to when "count" reached zero
					the door will be set out of of this state when "count" is unequal zero
					possible "style" values: STATE_TOP STATE_BOTTOM

if the counter has counted down to zero, it will set the set the target into the given state "style"
*/

#define	STATE_TOP			0
#define	STATE_BOTTOM		1

//other is for instance the button activating
//other->count is used to determine to count up or down
void trigger_counting_use (edict_t *self, edict_t *other, edict_t *activator)
{
	edict_t *t;

	//add the count of the other
	self->count += other->count;

	if (self->target)
	{
		t = NULL;
		while ((t = G_Find (t, FOFS(targetname), self->target)))
		{
			//if it is another trigger counting
			if (!Q_stricmp(t->classname, "trigger_counting"))
			{
				if (self->count) t->count++;
				else t->count--;
			} //end if
			else
			{
				//
				if (!t->use) continue;
				//set the door in ther correct state
				if (!self->count)
				{
					if (t->moveinfo.state != self->style)
					{
						t->use(t, self, activator);
					} //end if
				} //end if
				else
				{
					if (t->moveinfo.state == self->style)
					{
					t->use(t, self, activator);
					} //end if
				} //end else
			} //end else
		} //end while
	} //end if
} //end of the function trigger_count_target

void SP_trigger_counting(edict_t *self)
{
	if (!self->target)
	{
		gi.dprintf("trigger_counting without target\n");
	} //end if
	if (self->style != STATE_TOP && self->style != STATE_BOTTOM)
	{
		gi.dprintf("trigger_count with invalid style\n");
		self->style = STATE_TOP;
	} //end if
	self->use = trigger_counting_use;
} //end of the function SP_trigger_counting

#endif //TRIGGER_COUNTING
