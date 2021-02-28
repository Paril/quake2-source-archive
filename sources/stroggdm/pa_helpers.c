/*
Quake2 Helper Functions
		By Paril


These functions allow quicker access to game import stuff.

I will explain each on the way.
*/

#include "g_local.h" // General include

/*
==================
Remake of gi.sound, allowed for easier use.
Used like so:

channel:

0 = CHAN_AUTO // Doesn't override
1 = CHAN_WEAPON // Weapon channel
2 = CHAN_VOICE // Voice channel
3 =	CHAN_ITEM // Item channel
4 = CHAN_BODY // Body channel

attentuation:

0 =	ATTN_NONE // Full volume, entire level
1 = ATTN_NORM // Normal, medium radius
2 =	ATTN_IDLE // A bit lower than normal
3 = ATTN_STATIC // Sound gets lower with very small radiuses

Should be enough. Usage example:

sound (ent, 0, "weapons", "lstart", 1);

OR

sound (ent, CHAN_AUTO, "weapons", "lstart", ATTN_NORM);
==================
*/
void sound (edict_t *ent, int channel, char *folder, char *file, int attentuation)
{
	if (channel > 4) // If channel is greater an allowed
		channel = 4;
	if (attentuation > 3) // If attentuation is greater an allowed
		attentuation = 3;
	gi.sound(ent, channel, gi.soundindex(va("%s/%s.wav", folder, file)), 1, attentuation, 0);
}

/*
==================
Remake of gi.cprintf, allowed for easier use.
Used like so:

printlevel:

0 = PRINT_LOW // At pickup item location
1 = PRINT_MEDIUM // Death messages
2 = PRINT_HIGH // Critical messages (almost same as PRINT_MEDIUM, stays longer I think)
3 =	PRINT_CHAT // Chat message

Should be enough. Usage example:

cprint (ent, 2, "Welcome!");

OR

cprint (ent, PRINT_HIGH, "Welcome!");
==================
*/
void cprint (edict_t *ent, int printlevel, char *text)
{
	if (printlevel > 3)
		printlevel = 3;
	safe_cprintf (ent, printlevel, "%s\n", text);
}

/*
==================
Remake of gi.centerprintf, allowed for easier use.
Used like so:

centerprint (ent, "Welcome!");
==================
*/
void centerprint (edict_t *ent, char *text)
{
	safe_centerprintf (ent, "%s\n", text);
}

/*
==================
Remake of gi.dprintf, allowed for easier use.
ONLY USED FOR SPECIAL MESSAGES DEALING USUALLY
WITH MISPLACED ENTITIES OR SO! USE BPRINT FOR 
MESSAGE LIKE "Welcome to my server!"
Used like so:

dprintf ("Placing entities...");
==================
*/
void dprint (edict_t *ent, char *text)
{
	gi.dprintf ("%s\n", text);
}

/*
==================
Remake of gi.bprintf, allowed for easier use.
Used like so:

printlevel:

0 = PRINT_LOW // At pickup item location
1 = PRINT_MEDIUM // Death messages
2 = PRINT_HIGH // Critical messages (almost same as PRINT_MEDIUM, stays longer I think)
3 =	PRINT_CHAT // Chat message

Should be enough. Usage example:

bprint (2, "Welcome!");

OR

bprint (PRINT_HIGH, "Welcome!");
==================
*/
void bprint (edict_t *ent, int printlevel, char *text)
{
	if (printlevel > 3)
		printlevel = 3;
	safe_bprintf (printlevel, "%s\n", text);
}


/*
==================
Remake of gi.trace, allowed for easier use.
Used like so:

simple_trace (ent, 60);
==================
*/
trace_t simple_trace (edict_t *trace_shooter, int distance)
{
	vec3_t  start;
	vec3_t  forward;
	vec3_t  end;
	trace_t tr;

	VectorCopy(trace_shooter->s.origin, start);
	start[2] += trace_shooter->viewheight;
	AngleVectors(trace_shooter->client->v_angle, forward, NULL, NULL);
	VectorMA(start, distance, forward, end);

	tr = gi.trace (start, NULL, NULL, end, trace_shooter, MASK_SHOT);

	return tr;
}


/*
==================
New function
By Paril

This function, used best in
conjunction with simple_trace, can be used
to push or pull the *pushed entity.

Usage (Without trace)

Push_Pull (ent, 250) // Push
Push_Pull (ent, -250) // Pull

Usage (With trace, Starting from declairations)

trace_t tr;

tr.ent = (simple_trace (ent, 150));
Push_Pull (tr.ent, 700);
==================
*/
void Push_Pull (edict_t *vict, int power)
{
	vec3_t  forward;

   if ( vict && ((vict->svflags & SVF_MONSTER) || (vict->client)) )
   {
       VectorScale(forward, power, forward);
       VectorAdd(forward, vict->velocity, vict->velocity);
   }
}


/*
==================
New function
By Paril

This function simplifies dealing
damage to others.

Usage:

DoDamage (other, rocket, ent, 50, 170, DAMAGE_NO_ARMOR, MOD_ROCKET); // Example
==================
*/
void DoDamage (edict_t *target, edict_t *inflictor, edict_t *attacker, int damage, int knockback, int extra_flags, int mod)
{
	T_Damage (target, inflictor, attacker, vec3_origin, target->s.origin, vec3_origin, damage, knockback, extra_flags, mod);
}

