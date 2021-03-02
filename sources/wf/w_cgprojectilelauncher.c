#include "g_local.h"
void Weapon_Generic (edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent));
/*
=====================================================
Computer Guided Projectile Launcher http://haydon.niehs.nih.gov/maplist2.htm
=====================================================
*/

void FireComputerGuidedProjectile(edict_t *ent,int damage,int kick)
{
	vec3_t forward, right, target, dir,start;
	vec3_t check;
	trace_t tr;
	edict_t *blip, *targ;
	if(ent->light_level<1)
		return;
	ent->light_level--;
	AngleVectors (ent->client->v_angle, forward, right, NULL);
		

	check[0]=ent->s.origin[0] + forward[0]*750;
	check[1]=ent->s.origin[1] + forward[1]*750;
	check[2]=ent->s.origin[2] + forward[2]*750;
	targ=NULL;
	blip = NULL;
	while (blip = findradius (blip, check, 375))
	{
		
		if (!(blip->svflags & SVF_MONSTER) && !blip->client)
			continue;
		if (blip->health <= 0)
			continue;

		tr = gi.trace (ent->s.origin, NULL, NULL, blip->s.origin, ent, MASK_SOLID);
		if (tr.fraction != 1.0)
			continue;
		targ = blip;
	}
	if(!targ)
	{
		check[0]=ent->s.origin[0] + forward[0]*500;
		check[1]=ent->s.origin[1] + forward[1]*500;
		check[2]=ent->s.origin[2] + forward[2]*500;
		while (blip = findradius (blip, check, 175))
		{	
		
			if (!(blip->svflags & SVF_MONSTER) && !blip->client)
				continue;
			if (blip->health <= 0)
				continue;


			tr = gi.trace (ent->s.origin, NULL, NULL, blip->s.origin, ent, MASK_SOLID);
			if (tr.fraction != 1.0)
				continue;
			targ = blip;
		}
	}
	if(!targ)
	{
		check[0]=ent->s.origin[0] + forward[0]*900;
		check[1]=ent->s.origin[1] + forward[1]*900;
		check[2]=ent->s.origin[2] + forward[2]*900;
		while (blip = findradius (blip, check, 250))
		{	
		
			if (!(blip->svflags & SVF_MONSTER) && !blip->client)
				continue;
			if (blip->health <= 0)
				continue;

			tr = gi.trace (ent->s.origin, NULL, NULL, blip->s.origin, ent, MASK_SOLID);
			if (tr.fraction != 1.0)
				continue;
			targ = blip;
		}
	}
	if(!targ)
	{
		check[0]=ent->s.origin[0] + forward[0]*1050;
		check[1]=ent->s.origin[1] + forward[1]*1050;
		check[2]=ent->s.origin[2] + forward[2]*1050;
		while (blip = findradius (blip, check, 175))
		{	
		
			if (!(blip->svflags & SVF_MONSTER) && !blip->client)
				continue;
			if (blip->health <= 0)
				continue;

			tr = gi.trace (ent->s.origin, NULL, NULL, blip->s.origin, ent, MASK_SOLID);
			if (tr.fraction != 1.0)
				continue;
			targ = blip;
		}
	}
	if(!targ)
	{
		check[0]=ent->s.origin[0] + forward[0]*1125;
		check[1]=ent->s.origin[1] + forward[1]*1125;
		check[2]=ent->s.origin[2] + forward[2]*1125;
		while (blip = findradius (blip, check, 75))
		{	
		
			if (!(blip->svflags & SVF_MONSTER) && !blip->client)
				continue;
			if (blip->health <= 0)
				continue;

			tr = gi.trace (ent->s.origin, NULL, NULL, blip->s.origin, ent, MASK_SOLID);
			if (tr.fraction != 1.0)
				continue;
			targ = blip;
		}
	}
	if(targ)
	{	
		// calc direction to where we targted
		VectorMA (targ->s.origin, -0.2, targ->velocity, target);
		
		//Adjust for height
		target[2] -= targ->viewheight/1.5;
		
		VectorSubtract (target, ent->s.origin, dir);
		VectorNormalize (dir);
		safe_cprintf (ent, PRINT_HIGH, "Target Acquired by Computer!\n");

		VectorCopy(dir,forward);
		
	}	
	
	start[0] = ent->s.origin[0]+forward[0]*3;
	start[1] = ent->s.origin[1]+forward[1]*3;
	
	start[2] = ent->s.origin[2]+forward[2]*3+ent->viewheight;
	//gi.error("JOhn");
	//fire bullet
	
	fire_bullet (ent, start, forward, damage, kick, 75, 75,0);
		
	// send muzzle flash
/*	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_SHOTGUN);
	gi.multicast (ent->s.origin, MULTICAST_PVS);*/
}

void weapon_ComputerGuidedProjectileLauncher_fire (edict_t *ent)
{
	vec3_t		start;
	int damage = 8;
	int kick = 80;
	if ((ent->client->buttons & BUTTON_ATTACK) && (ent->client->ps.gunframe == 11))
	{
		ent->client->ps.gunframe=8;
		return;
	}
	/*
	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}*/

	
	FireComputerGuidedProjectile(ent, damage, kick);


	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

}

void Weapon_ComputerGuidedProjectileLauncher (edict_t *ent)
{
	static int	pause_frames[]	= {22, 28, 34, 0};
	static int	fire_frames[]	= { 9,11, 0};

	Weapon_Generic (ent, 7, 18, 36, 39, pause_frames, fire_frames, weapon_ComputerGuidedProjectileLauncher_fire);
}