//#include "g_local.h"
//void Leper_Effects(edict_t *ent);
//void Leper_NoLegs(edict_t *ent);
//void Leper_NoArms(edict_t *ent);

//void ThrowUpNow(edict_t *self, int limb);
//static void P_ProjectSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result)
//{
//	vec3_t	_distance;
//
//	VectorCopy (distance, _distance);
//	if (client->pers.hand == LEFT_HANDED)
//		_distance[1] *= -1;
//	else if (client->pers.hand == CENTER_HANDED)
//		_distance[1] = 0;
//	G_ProjectSource (point, _distance, forward, right, result);
//}

/*void Leper_Think (edict_t *ent)
{
	qboolean movefast;
	int	l,r;
	if(!leper->value)
		return;
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

	if(fabs(ent->velocity[0]) >50 || fabs(ent->velocity[1]) >50)
		movefast = true;
	else
		movefast = false;

	if((!ent->leftleg || !ent->rightleg) //if you only have 1 leg, not none, not 2.
		&& !(!ent->leftleg && !ent->rightleg)
		&& movefast 
		&& ent->groundentity && action->value)
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

	if(!leper->value)
	{
		ent->s.modelindex4 = 0;
		ent->s.modelindex3 = 0;
		return;
	}


	if(ent->deadflag)
	{
		ent->s.modelindex4 = 0;
		ent->s.modelindex3 = 0;
		ent->s.modelindex2 = 0;
		return;
	}
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

	if(ent->client->cloak_framenum > level.framenum )
	{
	ent->s.modelindex4 = 0;
	ent->s.modelindex3 = 0;
//	ent->s.modelindex2 = 0;
	}


}

void Leper_NoLegs(edict_t *ent)
{
	if(!action->value)
		return;
	
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
	int	damage = random() * 100;
	if(ent->leftleg)
	{
		ent->leftleg = 0;		
		ThrowGib (ent, "models/objects/gibs/leg/tris.md2", damage, GIB_ORGANIC);

		Leper_NoLegs(ent);
		return;
	}
	if(ent->rightleg)
	{
		ent->rightleg = 0;
	
		ThrowGib (ent, "models/objects/gibs/leg/tris.md2", damage, GIB_ORGANIC);
		Leper_NoLegs(ent);
		
	}
	

}

void Cmd_KillArm_f(edict_t *ent)
{
	
	int	damage = random() * 100;
	if(ent->leftarm)
	{
		ent->leftarm = 0;
		ThrowGib (ent, "models/objects/gibs/arm/tris.md2", damage, GIB_ORGANIC);
		//if(!ent->leftarm && !ent->rightarm)
	//	NoAmmoWeaponChange (ent);
	
		return;
	}
	if(ent->rightarm)
	{
		ent->rightarm = 0;
		ThrowGib (ent, "models/objects/gibs/arm/tris.md2", damage, GIB_ORGANIC);
		//if(!ent->leftarm && !ent->rightarm)
	//	NoAmmoWeaponChange (ent);
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

void Leper_TDamage (edict_t *targ, edict_t *attacker, vec3_t point, int damage, int mod)
{
	edict_t		*drop;
	float      	xyspeed = sqrt(targ->velocity[0]*targ->velocity[0] + targ->velocity[1]*targ->velocity[1]);
	int lepmod = mod;
	
	if (!xyspeed)
		if(lepmod == LOC_LEFTARM)
			lepmod = LOC_BACK;
		else if(lepmod == LOC_BACK)
			lepmod = LOC_RIGHTARM;
		else if(lepmod == LOC_RIGHTARM)
			lepmod = LOC_CHEST;
		else if(lepmod == LOC_CHEST)
			lepmod = LOC_LEFTARM;

//legs crunch if you hit the ground hard		
if (mod == MOD_FALLING)
	{
		if(targ->leftleg) 
		{
		targ->leftleg-=damage;
		if(targ->leftleg<0)
			{
			damage = targ->leftleg * -1;
			targ->leftleg = 0;
			
			if(IsFemale(targ))
			ThrowGib (targ, "players/female/limb_lleg.md2", damage, GIB_ORGANIC);
			else if(IsNeutral(targ))
			ThrowGib (targ, "players/cyborg/limb_lleg.md2", damage, GIB_ORGANIC);
			else
			ThrowGib (targ, "players/male/limb_lleg.md2", damage, GIB_ORGANIC);
			}
	
		}
		
		if(targ->rightleg) 
		{
			targ->rightleg-=damage;
			if(targ->rightleg<0)
			{
			targ->rightleg = 0;
			damage = targ->leftleg * -1;

			if(IsFemale(targ))
			ThrowGib (targ, "players/female/limb_rleg.md2", damage, GIB_ORGANIC);
			else if(IsNeutral(targ))
			ThrowGib (targ, "players/cyborg/limb_rleg.md2", damage, GIB_ORGANIC);
			else
			ThrowGib (targ, "players/male/limb_rleg.md2", damage, GIB_ORGANIC);

			}
		
		}
		Leper_NoLegs(targ);
	}
	

if (lepmod == LOC_LEFTLEG)		
{
	if(targ->leftleg > 0)
	{
	targ->leftleg -= damage;
	if(targ->leftleg <= 0) 
	{

	targ->leftleg = 0;

			if(IsFemale(targ))
			ThrowGib (targ, "players/female/limb_lleg.md2", damage, GIB_ORGANIC);
			else if(IsNeutral(targ))
			ThrowGib (targ, "players/cyborg/limb_lleg.md2", damage, GIB_ORGANIC);
			else
			ThrowGib (targ, "players/male/limb_lleg.md2", damage, GIB_ORGANIC);

	Leper_NoLegs(targ);
	}

	}
	else
	{
	lepmod = LOC_RIGHTLEG;
	}
	
}
if (lepmod == LOC_RIGHTLEG && targ->rightleg > 0)		
{
	if(targ->rightleg > 0)
	{
	targ->rightleg -= damage;
	if(targ->rightleg <= 0) 
	{
	targ->rightleg = 0;	


	if(IsFemale(targ))
			ThrowGib (targ, "players/female/limb_rleg.md2", damage, GIB_ORGANIC);
			else if(IsNeutral(targ))
			ThrowGib (targ, "players/cyborg/limb_rleg.md2", damage, GIB_ORGANIC);
			else
			ThrowGib (targ, "players/male/limb_rleg.md2", damage, GIB_ORGANIC);

	Leper_NoLegs(targ);
	}

	}
	else
	{
	lepmod = LOC_LEFTLEG;
	}
}
if (lepmod == LOC_LEFTLEG)		
{
	if(targ->leftleg > 0)
	{
	targ->leftleg -= damage;
	if(targ->leftleg <= 0) 
	{
	targ->leftleg = 0;
		
		if(IsFemale(targ))
			ThrowGib (targ, "players/female/limb_lleg.md2", damage, GIB_ORGANIC);
			else if(IsNeutral(targ))
			ThrowGib (targ, "players/cyborg/limb_lleg.md2", damage, GIB_ORGANIC);
			else
			ThrowGib (targ, "players/male/limb_lleg.md2", damage, GIB_ORGANIC);

	Leper_NoLegs(targ);
	}

	}
	
	
	
}	
if (lepmod == LOC_LEFTARM && targ->leftarm > 0)		
{
	targ->leftarm -= damage;
	if(targ->leftarm <= 0) 
	{
	targ->leftarm=0;
		
	if(IsFemale(targ))
			ThrowGib (targ, "players/female/limb_larm.md2", damage, GIB_ORGANIC);
			else if(IsNeutral(targ))
			ThrowGib (targ, "players/cyborg/limb_larm.md2", damage, GIB_ORGANIC);
			else
			ThrowGib (targ, "players/male/limb_larm.md2", damage, GIB_ORGANIC);
	

//	if(!targ->leftarm && !targ->rightarm)
//	NoAmmoWeaponChange (targ);
	}


}
if (lepmod == LOC_RIGHTARM && targ->rightarm > 0)		
{
	targ->rightarm -= damage;
	if(targ->rightarm <= 0) 
	{
	targ->rightarm=0;
	if(IsFemale(targ))
			ThrowGib (targ, "players/female/limb_rarm.md2", damage, GIB_ORGANIC);
			else if(IsNeutral(targ))
			ThrowGib (targ, "players/cyborg/limb_rarm.md2", damage, GIB_ORGANIC);
			else
			ThrowGib (targ, "players/male/limb_rarm.md2", damage, GIB_ORGANIC);
	

//	if(!targ->leftarm && !targ->rightarm)
//	NoAmmoWeaponChange (targ);
	}

}	



	//leper
}

void ThrowUpNow(edict_t *self, int limb)
{
	// use some local vector variables to work with
	vec3_t	forward, right;
	vec3_t	mouth_pos, spew_vector;
	
	if(self->deadflag)
		return;
	
	if(random() > 0.01)
		return;

	// set the spew vector, based on the client's view angle
	

	// Make the spew originate from our limb
	if(limb == LOC_LEFTARM)
	{
	VectorCopy (self->client->v_angle, forward);
	forward[YAW]=self->client->v_angle[YAW] - 90;
		AngleVectors (forward, forward, right, NULL);
	
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
	VectorCopy (self->client->v_angle, forward);
	forward[YAW]=self->client->v_angle[YAW] + 90;
		AngleVectors (forward, forward, right, NULL);
	
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
	VectorCopy (self->client->v_angle, forward);
	forward[YAW]=self->client->v_angle[YAW] - 90;
	forward[ROLL]=self->client->v_angle[ROLL] - 45;
	AngleVectors (forward, forward, right, NULL);
	
	
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
	VectorCopy (self->client->v_angle, forward);
	forward[YAW]=self->client->v_angle[YAW] + 90;
	forward[ROLL]=self->client->v_angle[ROLL] + 45;
		AngleVectors (forward, forward, right, NULL);
	
	VectorCopy (self->s.origin, mouth_pos);
	mouth_pos[2] -= 11;
	VectorScale (forward, 1, spew_vector);
	
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BLOOD);
	gi.WritePosition (mouth_pos);
	gi.WriteDir (forward);
	gi.multicast (mouth_pos, MULTICAST_PVS);
	}

}*/