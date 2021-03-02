#include "g_local.h"
void MatrixJabAndHook(edict_t *ent);
void MatrixCharge(edict_t *ent);
void swoosh (edict_t *ent);

static void P_ProjectSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result)
{
	vec3_t	_distance;

	VectorCopy (distance, _distance);
	if (client->pers.hand == LEFT_HANDED)
		_distance[1] *= -1;
	else if (client->pers.hand == CENTER_HANDED)
		_distance[1] = 0;
	G_ProjectSource (point, _distance, forward, right, result);
}

void MatrixKungfu (edict_t *ent)
{
	
	MatrixJabAndHook(ent);
	MatrixCharge(ent);
}
void MatrixJabAndHook(edict_t *ent)
{
	vec3_t	forward, right;
	vec3_t  offset, start;
    trace_t tr;
    vec3_t end;
	int damage, kick, MOD, sum;
	edict_t	*target = NULL;
	edict_t *blip = NULL;
	vec3_t	targetdir, blipdir, dir, AngleToTarget;
	vec_t	speed;
	int		dist;

	if(ent->client->weaponstate == WEAPON_FIRING)
		return;
	if(ent->deadflag)
		return;
	if(ent->client->next_kungfu > level.framenum)
		return;
	
	
	while ((blip = findradius(blip, ent->s.origin, 100)) != NULL)
	{
		if (!(blip->svflags & SVF_MONSTER) && !blip->client)
			continue;
		if (blip == ent)
			continue;
		if (!blip->takedamage)
			continue;
		if (blip->health <= 0)
			continue;
		VectorSubtract(blip->s.origin, ent->s.origin, blipdir);
		blipdir[2] += 16;
		if ((target == NULL) || (VectorLength(blipdir) < VectorLength(targetdir)))
		{
			target = blip;
			VectorCopy(blipdir, targetdir);
		}
	}
	sum = fabs(ent->client->v_angle[YAW] - ent->oldvangles[YAW]);

	if(sum > 180)
		sum = 360 - sum;
	
	
//	gi.cprintf (ent, PRINT_HIGH, "YAW = %i   ",sum);
//
	if (target != NULL)
	{
		VectorSubtract(target->s.origin, ent->s.origin, targetdir);
		VectorNormalize(targetdir);
		tr = gi.trace (ent->s.origin, NULL, NULL, target->s.origin, ent, MASK_MONSTERSOLID);
		MOD = MOD_KICK;
		damage = 60;
		kick = 300;
		
		if(sum> 45)//if you spin far enough, do damage
		{
			target->velocity[2] +=100;
			T_Damage (target, ent, ent, targetdir, target->s.origin, target->s.origin, damage_spinkick->value, kick, 1, MOD_SWEEP);
			ent->matrixflip = KUNGFU;
			ent->KungFuAnim = SPINKICK;
			if(level.framenum < ent->LastHit + 15)
			{
				ent->CumulativeDamage += damage_spinkick->value;
				ent->HitStreak++;
				ent->LastHit = level.framenum;
			}
			else
			{
				ent->HitStreak = 1;
				ent->CumulativeDamage = damage_spinkick->value;
				ent->LastHit = level.framenum;
			}
			ent->KungFuAnim = SPINKICK;
			swoosh (ent);//swoosh noise

			if (ent->solid != SOLID_NOT && sum> 100)
			{
				if(IsBlocking(target))
				gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/phitf2.wav"), 1, ATTN_NORM, 0);
				else
				gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/kick1.wav"), 1, ATTN_NORM, 0);
				PlayerNoise(ent, ent->s.origin, PNOISE_SELF);
		
				ent->client->next_kungfu = level.framenum + reload_spinkick->value;
		
				return;
			}
		}
		
		
		
		
	}

	   
    // don't need to check for water

	if (ent->OLDvertbuttons <= 0 && 
		ent->vertbuttons>0 && 
		!ent->groundentity && 
		!ent->oldgroundentity) //hover kick must work even if no enemy in range.
			{
				MOD = MOD_KICK;
				damage = 20;
				kick = 1;
				swoosh (ent);//swoosh noise

				if(target != NULL && tr.fraction < 1.0 && tr.ent->takedamage && !tr.ent->deadflag && !ent->deadflag)
				{
					T_Damage (target, ent, ent, targetdir, target->s.origin, target->s.origin, damage_hoverkick->value, kick, 1, MOD_HOVER);
					if(level.framenum < ent->LastHit + 15)
					{
					ent->CumulativeDamage += damage_hoverkick->value;
					ent->HitStreak++;
					ent->LastHit = level.framenum;
					}
					else
					{
					ent->HitStreak = 1;
					ent->CumulativeDamage = damage_hoverkick->value;
					ent->LastHit = level.framenum;
					}
					ent->KungFuAnim = HOVERKICK;
				
					if (ent->solid != SOLID_NOT )
					{
						if(IsBlocking(target))
						{
							gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/phitf2.wav"), 1, ATTN_NORM, 0);
							
						}
						else
						gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/phitf1.wav"), 1, ATTN_NORM, 0);
						PlayerNoise(ent, ent->s.origin, PNOISE_SELF);
					}
				
				}
				ent->client->next_kungfu = level.framenum + reload_hoverkick->value;


					if (ent->velocity[2] > 0)
						ent->velocity[2] += 100;
					else
						ent->velocity[2] += 120;
					ent->matrixflip = KUNGFU;
					ent->KungFuAnim = HOVERKICK;
			}
	
    AngleVectors (ent->client->v_angle, forward, right, NULL);
    VectorScale (forward, 0, ent->client->kick_origin);
    VectorSet(offset, 0, 0,  ent->viewheight-4);
    P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
    
    VectorMA( start, 80, forward, end );
    
    tr = gi.trace (ent->s.origin, NULL, NULL, end, ent, MASK_MONSTERSOLID);

	if (tr.fraction < 1.0 && tr.ent->takedamage && !tr.ent->deadflag && !ent->deadflag)        
    {            
        
			if(ent->forwardbuttons<0)
				return;
			
			if (ent->OLDforwardbuttons == 0 && ent->forwardbuttons>0)
			{
				MOD = MOD_KICK;
				damage = 20;
				kick = 250;
					T_Damage (tr.ent, ent, ent, forward, tr.endpos, tr.plane.normal, damage_hook->value, kick, 1, MOD_HOOK);
					ent->matrixflip = KUNGFU;
					ent->KungFuAnim = HOOK;
					if(level.framenum < ent->LastHit + 15)
					{
					ent->CumulativeDamage += damage_hook->value;
					ent->HitStreak++;
					ent->LastHit = level.framenum;
					}
					else
					{
					ent->HitStreak = 1;
					ent->CumulativeDamage = damage_hook->value;
					ent->LastHit = level.framenum;
					}
					VectorClear(ent->velocity);
					swoosh (ent);//swoosh noise
					if (ent->solid != SOLID_NOT )
					if(IsBlocking(tr.ent))
						gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/phitf2.wav"), 1, ATTN_NORM, 0);
						else
						gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/kick1.wav"), 1, ATTN_NORM, 0);
					
					PlayerNoise(ent, ent->s.origin, PNOISE_SELF);
					ent->client->next_kungfu = level.framenum + reload_hook->value;
					ent->client->kick_angles[0] = 16;
			}
			else
			if (ent->OLDstrafebuttons <= 0 && ent->strafebuttons>0 && ent->forwardbuttons == 0)
			{
				MOD = MOD_KICK;
				damage = 10;
				kick = 50;
					T_Damage (tr.ent, ent, ent, forward, tr.endpos, tr.plane.normal, damage_jab->value, kick, 1, MOD_JAB);
					ent->matrixflip = KUNGFU;
					ent->KungFuAnim = JAB;
					if(level.framenum < ent->LastHit + 15)
					{
					ent->CumulativeDamage += damage_jab->value;
					ent->HitStreak++;
					ent->LastHit = level.framenum;
					}
					else
					{
					ent->HitStreak = 1;
					ent->CumulativeDamage = damage_jab->value;
					ent->LastHit = level.framenum;
					}
					VectorClear(ent->velocity);
					swoosh (ent);//swoosh noise
					if (ent->solid != SOLID_NOT )
					if(IsBlocking(tr.ent))
						gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/phitf2.wav"), 1, ATTN_NORM, 0);
						else
						gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/kick2.wav"), 1, ATTN_NORM, 0);
						
					PlayerNoise(ent, ent->s.origin, PNOISE_SELF);
					ent->client->next_kungfu = level.framenum + reload_jab->value;
					ent->client->kick_angles[2] = -15;
			}
			else
			if (ent->OLDstrafebuttons >= 0 && ent->strafebuttons<0 && ent->forwardbuttons == 0)
			{
				MOD = MOD_KICK;
				damage = 10;
				kick = 50;
					T_Damage (tr.ent, ent, ent, forward, tr.endpos, tr.plane.normal, damage_jab->value, kick, 1, MOD_JAB);
					ent->matrixflip = KUNGFU;
					ent->KungFuAnim = JAB;
					if(level.framenum < ent->LastHit + 15)
					{
					ent->CumulativeDamage += damage_jab->value;
					ent->HitStreak++;
					ent->LastHit = level.framenum;
					}
					else
					{
					ent->HitStreak = 1;
					ent->CumulativeDamage = damage_jab->value;
					ent->LastHit = level.framenum;
					}
					VectorClear(ent->velocity);
					swoosh (ent);//swoosh noise
					
					if (ent->solid != SOLID_NOT )
					if(IsBlocking(tr.ent))
					gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/phitf2.wav"), 1, ATTN_NORM, 0);
					else
					gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/kick.wav"), 1, ATTN_NORM, 0);
					PlayerNoise(ent, ent->s.origin, PNOISE_SELF);
					ent->client->next_kungfu = level.framenum + reload_jab->value;
					ent->client->kick_angles[2] = 15;
			}
			else
			if (ent->OLDvertbuttons < 0 && ent->vertbuttons>=0 && ent->groundentity)
			{
				MOD = MOD_KICK;
				damage = 80;
				kick = 500;
					tr.ent->velocity[2] += 200;
					T_Damage (tr.ent, ent, ent, forward, tr.endpos, tr.plane.normal, damage_uppercut->value, kick, 1, MOD_UPPERCUT);
					ent->matrixflip = KUNGFU;
					ent->KungFuAnim = UPPERCUT;
					if(level.framenum < ent->LastHit + 15)
					{
					ent->CumulativeDamage += damage_uppercut->value;
					ent->HitStreak++;
					ent->LastHit = level.framenum;
					}
					else
					{
					ent->HitStreak = 1;
					ent->CumulativeDamage = damage_uppercut->value;
					ent->LastHit = level.framenum;
					}
					VectorClear(ent->velocity);
					swoosh (ent);//swoosh noise
					
					if (ent->solid != SOLID_NOT )
						if(IsBlocking(tr.ent))
						gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/phitf2.wav"), 1, ATTN_NORM, 0);
						else
						gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/phitf1.wav"), 1, ATTN_NORM, 0);
					
					PlayerNoise(ent, ent->s.origin, PNOISE_SELF);
					ent->client->next_kungfu = level.framenum + reload_uppercut->value;
					ent->client->kick_angles[0] = -40;
			}
			
			
	}

	
 VectorCopy(ent->client->v_angle, ent->oldvangles);//NB i am getting pissed off by this thing - WATCH IT.       
}

void MatrixCharge(edict_t *ent) // if you go fast and run into an opponent, you push them back (no damage, just a shoulder charge)
//this was a good idea in theory, but it means that people are pushed back before you can hit them.
{
	
	/*vec3_t	forward, right;
	vec3_t  offset, start, speed;
    trace_t tr;
    vec3_t end;

	AngleVectors (ent->client->v_angle, forward, right, NULL);
    VectorScale (forward, 0, ent->client->kick_origin);
    VectorSet(offset, 0, 0,  ent->viewheight-4);
    P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
    
	VectorCopy (ent->velocity, forward);
	VectorNormalize(forward);
    VectorMA( start, 80, forward, end );
    
    tr = gi.trace (ent->s.origin, NULL, NULL, end, ent, MASK_MONSTERSOLID);
	//trace in direction of speed.

	if(tr.fraction < 1.0 && tr.ent->takedamage && !tr.ent->deadflag && !ent->deadflag 
		&& fabs(VectorLength(ent->velocity)) > 210 
		&& fabs(VectorLength(tr.ent->velocity)) < 200)   
	{
		VectorCopy(ent->velocity, speed);//temp speed
		VectorCopy(tr.ent->velocity, ent->velocity);//player takes on enemy's speed
		VectorMA(speed, 50, tr.ent->velocity, tr.ent->velocity);//player's original speed added onto enemie's
	
	}
*/

}
void swoosh (edict_t *ent)
{
	float	sound;
	sound = random() * 3;
			if (sound <= 1)
			{
				gi.sound(ent, CHAN_WEAPON, gi.soundindex("misc/throw.wav"), 1, ATTN_NORM, 0);
				PlayerNoise(ent, ent->s.origin, PNOISE_SELF);
			}
			else if (sound <= 2)
			{
				gi.sound(ent, CHAN_WEAPON, gi.soundindex("misc/throw.wav"), 1, ATTN_NORM, 0);
				PlayerNoise(ent, ent->s.origin, PNOISE_SELF);
			}
			else if(sound <= 3)
			{
				gi.sound(ent, CHAN_WEAPON, gi.soundindex("misc/swish.wav"), 1, ATTN_NORM, 0);
				PlayerNoise(ent, ent->s.origin, PNOISE_SELF);
			}
}

void MatrixComboTally (edict_t *ent)
{
	char	string[1024];


	if(ent->HitStreak > 1 && ent->LastHit == level.framenum - 15)
	{
	

	// send the layout
	/*Com_sprintf (string, sizeof(string),
		"xv 160 yv 180 string2 \"%i Hit Combo\" ", 			//combo
		ent->HitStreak);

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
	gi.unicast (ent, true);*/
	if(combomessage->value)
	{
	gi.cprintf (ent, PRINT_CHAT, "%i Hit Combo: %i Damage Done\n", ent->HitStreak, (ent->CumulativeDamage * (2 + ent->damagelevel))/2);
	if(ent->CumulativeDamage >= 100)
	gi.sound(ent, CHAN_VOICE, gi.soundindex("frags/combo.wav"), 1, ATTN_NORM, 0);
	}
	if(ent->LastHit == level.framenum - 15)
	ent->HitStreak  = 0;
	ent->CumulativeDamage = 0;
	}

	if(ent->HitStreak && ent->LastHit  < level.framenum - 15)
	{
		ent->HitStreak = 0;
		ent->CumulativeDamage = 0;
	}

}

qboolean IsBlocking (edict_t *ent)
{
	if(!ent->client)
		return false;

	if(strcmp(ent->client->pers.weapon->classname, "weapon_fists") == 0  //if you're using fists or knoves you can block
	|| strcmp(ent->client->pers.weapon->classname, "weapon_knives") == 0)
		if(ent->forwardbuttons <= 0 && ent->strafebuttons == 0 && ent->client->next_kungfu <= level.framenum && ent->client->weaponstate != WEAPON_FIRING)
			return true;
	return false;


}