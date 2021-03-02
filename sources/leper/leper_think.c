#include "g_local.h"

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

void Leper_Think (edict_t *ent)
{
	qboolean movefast;
	int	l,r;

	if(ent->leftleg<0)
		ent->leftleg = 0;
	if(ent->rightleg<0)
		ent->rightleg = 0;
	if(ent->leftarm<0)
		ent->leftarm = 0;
	if(ent->rightarm<0)
		ent->rightarm = 0;
	
	


	if(!ent->leftarm || !ent->rightarm)
		Leper_NoArms(ent);

	if(ent->velocity[0] >50
	|| ent->velocity[0] < -50
	|| ent->velocity[1] >50
	|| ent->velocity[1] < -50)
		movefast = true;
	else
		movefast = false;

	if((!ent->leftleg || !ent->rightleg) 
		&& !(!ent->leftleg && !ent->rightleg)
		&& movefast 
		&& ent->groundentity)
	{
		ent->velocity[2] += hop->value;
		r = 1 + (rand()&1);
		l = (1 + random()*2)*25;
		gi.sound (ent, CHAN_VOICE, gi.soundindex(va("*pain%i_%i.wav", l, r)), 1, ATTN_NORM, 0);
	}

	if(!ent->leftleg)
	ThrowUpNow(ent, LOC_LEFTLEG);
	if(!ent->rightleg)
	ThrowUpNow(ent, LOC_RIGHTLEG);
	if(!ent->rightarm)
	ThrowUpNow(ent, LOC_RIGHTARM);
	if(!ent->leftarm)
	ThrowUpNow(ent, LOC_LEFTARM);
}

void Leper_Effects(edict_t *ent)
{
	int scorethingy;
	float xyspeed = sqrt(ent->velocity[0]*ent->velocity[0] + ent->velocity[1]*ent->velocity[1]);

	if (IsNeutral(ent))
	{
	if(ent->leftleg > 0 && ent->rightleg > 0)
		ent->s.modelindex4 = gi.modelindex ("players/cyborg/bothlegs.md2");
	else if(ent->leftleg > 0 && !ent->rightleg)
		ent->s.modelindex4 = gi.modelindex ("players/cyborg/limb_lleg.md2");
	else if(!ent->leftleg && ent->rightleg> 0)
		ent->s.modelindex4 = gi.modelindex ("players/cyborg/limb_rleg.md2");
	else
		ent->s.modelindex4 = 0;
	
	if(ent->leftarm> 0 && ent->rightarm > 0)
		ent->s.modelindex3 = gi.modelindex ("players/cyborg/botharms.md2");
	else if(ent->leftarm > 0 && !ent->rightarm)
		ent->s.modelindex3 = gi.modelindex ("players/cyborg/limb_larm.md2");
	else if(!ent->leftarm && ent->rightarm > 0)
		ent->s.modelindex3 = gi.modelindex ("players/cyborg/limb_rarm.md2");
	}
	else if (IsFemale(ent))
	{
	if(ent->leftleg > 0 && ent->rightleg > 0)
		ent->s.modelindex4 = gi.modelindex ("players/female/bothlegs.md2");
	else if(ent->leftleg > 0 && !ent->rightleg)
		ent->s.modelindex4 = gi.modelindex ("players/female/limb_lleg.md2");
	else if(!ent->leftleg && ent->rightleg> 0)
		ent->s.modelindex4 = gi.modelindex ("players/female/limb_rleg.md2");
	else
		ent->s.modelindex4 = 0;
	
	if(ent->leftarm> 0 && ent->rightarm > 0)
		ent->s.modelindex3 = gi.modelindex ("players/female/botharms.md2");
	else if(ent->leftarm > 0 && !ent->rightarm)
		ent->s.modelindex3 = gi.modelindex ("players/female/limb_larm.md2");
	else if(!ent->leftarm && ent->rightarm > 0)
		ent->s.modelindex3 = gi.modelindex ("players/female/limb_rarm.md2");
	}
	else
	{
	if(ent->leftleg > 0 && ent->rightleg > 0)
		ent->s.modelindex4 = gi.modelindex ("players/male/bothlegs.md2");
	else if(ent->leftleg > 0 && !ent->rightleg)
		ent->s.modelindex4 = gi.modelindex ("players/male/limb_lleg.md2");
	else if(!ent->leftleg && ent->rightleg> 0)
		ent->s.modelindex4 = gi.modelindex ("players/male/limb_rleg.md2");
	else
		ent->s.modelindex4 = 0;
	
	if(ent->leftarm> 0 && ent->rightarm > 0)
		ent->s.modelindex3 = gi.modelindex ("players/male/botharms.md2");
	else if(ent->leftarm > 0 && !ent->rightarm)
		ent->s.modelindex3 = gi.modelindex ("players/male/limb_larm.md2");
	else if(!ent->leftarm && ent->rightarm > 0)
		ent->s.modelindex3 = gi.modelindex ("players/male/limb_rarm.md2");
	}

	if(!ent->leftarm && !ent->rightarm)
	{
		ent->s.modelindex3 = 0;
		ent->s.modelindex2 = 0;
	}
	else if(!ent->s.modelindex2)
				ent->s.modelindex2 = 255;

	if(ent->golden)
	{
		ent->s.effects |= EF_BLASTER;
		ent->s.effects |= EF_COLOR_SHELL;
		ent->s.renderfx |= (RF_SHELL_RED|RF_SHELL_GREEN);
		scorethingy = round(level.time*100) % 500;
		if (!scorethingy)
			if(xyspeed)
			ent->client->resp.score ++;
			else
			gi.cprintf (ent, PRINT_HIGH, "You must keep moving to get points from the Golden Limb.\n");
		
	}


}

void Leper_NoLegs(edict_t *ent)
{
	
	if(!ent->leftleg && !ent->rightleg)
	{
		stuffcmd (ent, "cl_forwardspeed 20\n");
		stuffcmd (ent, "cl_sidespeed 20\n");
	}
	else
	{
		stuffcmd (ent, "cl_forwardspeed 200\n");
		stuffcmd (ent, "cl_sidespeed 200\n");
	}

}

void Leper_NoArms(edict_t *ent)
{
	
}

void Cmd_KillLeg_f(edict_t *ent)
{
	edict_t *drop;
	if(ent->leftleg)
	{
		ent->leftleg = 0;
		if(ent->golden)
		{
			drop = Drop_Item (ent, FindItemByClassname ("item_goldenleg"));
			ent->golden = false;
			owned = false;
		}
		else
		drop = Drop_Item (ent, FindItemByClassname ("item_leg"));
		Leper_NoLegs(ent);
		return;
	}
	if(ent->rightleg)
	{
		ent->rightleg = 0;
		if(ent->golden)
		{
			drop = Drop_Item (ent, FindItemByClassname ("item_goldenleg"));
			ent->golden = false;
			owned = false;
		}
		else
		drop = Drop_Item (ent, FindItemByClassname ("item_leg"));
		Leper_NoLegs(ent);
		
	}
	

}

void Cmd_KillArm_f(edict_t *ent)
{
	edict_t *drop;
	if(ent->leftarm)
	{
		ent->leftarm = 0;
		drop = Drop_Item (ent, FindItemByClassname ("item_arm"));
		if(!ent->leftarm && !ent->rightarm)
		NoAmmoWeaponChange (ent);
	
		return;
	}
	if(ent->rightarm)
	{
		ent->rightarm = 0;
		drop = Drop_Item (ent, FindItemByClassname ("item_arm"));
		if(!ent->leftarm && !ent->rightarm)
		NoAmmoWeaponChange (ent);
	}
	

}
int Leper_RandomShot (edict_t *ent, int offsetangle)
{

	
	if(ent->leftarm && ent->rightarm)
		return 0;
	else if(!ent->leftarm || !ent->rightarm)
		return ((rand () % offsetangle) - (offsetangle/2));
	else
		return ((rand () % (offsetangle*2)) - (offsetangle));
}

void ThrowUpNow(edict_t *self, int limb)
{
	// use some local vector variables to work with
	vec3_t	forward, right;
	vec3_t	mouth_pos, spew_vector;
	
	if(self->deadflag)
		return;
	
	if(random() > 0.15)
		return;

	// set the spew vector, based on the client's view angle
	

	// Make the spew originate from our limb
	if(limb == LOC_LEFTARM)
	{
	AngleVectors (self->client->v_angle, forward, right, NULL);
	forward[YAW]=self->client->v_angle[YAW] - 90;
	VectorCopy (self->s.origin, mouth_pos);
	if(!self->leftleg && !self->rightleg)
	mouth_pos[2] += 2;
	else
	mouth_pos[2] += 11;
	VectorScale (forward, 1, spew_vector);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BLOOD);
	gi.WritePosition (mouth_pos);
	gi.WriteDir (forward);
	gi.multicast (mouth_pos, MULTICAST_PVS);
	}
	if(limb == LOC_RIGHTARM)
	{
	AngleVectors (self->client->v_angle, forward, right, NULL);
	forward[YAW]=self->client->v_angle[YAW] + 90;
	VectorCopy (self->s.origin, mouth_pos);
	if(!self->leftleg && !self->rightleg)
	mouth_pos[2] += 2;
	else
	mouth_pos[2] += 11;
	VectorScale (forward, 1, spew_vector);
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BLOOD);
	gi.WritePosition (mouth_pos);
	gi.WriteDir (forward);
	gi.multicast (mouth_pos, MULTICAST_PVS);
	}
	if(limb == LOC_LEFTLEG)
	{
	AngleVectors (self->client->v_angle, forward, right, NULL);
	forward[YAW]=self->client->v_angle[YAW] - 90;
	forward[ROLL]=self->client->v_angle[ROLL] - 45;
	VectorCopy (self->s.origin, mouth_pos);
	mouth_pos[2] -= 11;
	VectorScale (forward, 1, spew_vector);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BLOOD);
	gi.WritePosition (mouth_pos);
	gi.WriteDir (forward);
	gi.multicast (mouth_pos, MULTICAST_PVS);
	}
	if(limb == LOC_RIGHTLEG)
	{
	AngleVectors (self->client->v_angle, forward, right, NULL);
	forward[YAW]=self->client->v_angle[YAW] + 90;
	forward[ROLL]=self->client->v_angle[ROLL] + 45;
	VectorCopy (self->s.origin, mouth_pos);
	mouth_pos[2] -= 11;
	VectorScale (forward, 1, spew_vector);
	
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BLOOD);
	gi.WritePosition (mouth_pos);
	gi.WriteDir (forward);
	gi.multicast (mouth_pos, MULTICAST_PVS);
	}

}

void kick_attack (edict_t *ent, vec3_t start, vec3_t dir, int damage, int kick, int MOD)
{
        
    vec3_t          forward, right;
    vec3_t          offset;
    
    trace_t tr;
    vec3_t end;
    int sound;

	if (ent->deadflag || ent->health < 0)
		return;
	
	
	AngleVectors (ent->client->v_angle, forward, right, NULL);
    
    VectorScale (forward, 0, ent->client->kick_origin);
    
    VectorSet(offset, 0, 0,  ent->viewheight-20);
    P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
    
    VectorMA( start, 80, forward, end );
    
    tr = gi.trace (ent->s.origin, NULL, NULL, end, ent, MASK_MONSTERSOLID);
   
   

    if (tr.fraction < 1.0)        
    {            
        if (tr.ent->takedamage)            
        {
			T_Damage (tr.ent, ent, ent, vec3_origin, tr.endpos, tr.plane.normal, damage, kick, DAMAGE_NO_KNOCKBACK, MOD_TEETH);
			
		}   
    }
        
}

