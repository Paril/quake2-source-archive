#include "g_local.h"

void Cluster_Explode (edict_t *ent)
{
	vec3_t		origin;

	//Sean added these 4 vectors

	vec3_t   grenade1;
	vec3_t   grenade2;
	vec3_t   grenade3;
	vec3_t   grenade4;
        vec3_t   grenade5;

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	//FIXME: if we are onground then raise our Z just a bit since we are a point?
   
        T_RadiusDamage(ent, ent->owner, ent->dmg, ent->enemy, ent->dmg_radius, MOD_HELD_GRENADE);

	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);
	gi.WriteByte (svc_temp_entity);
	if (ent->waterlevel)
	{
		if (ent->groundentity)
			gi.WriteByte (TE_GRENADE_EXPLOSION_WATER);
		else
			gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	}
	else
	{
		if (ent->groundentity)
			gi.WriteByte (TE_GRENADE_EXPLOSION);
		else
                        gi.WriteByte (TE_ROCKET_EXPLOSION);
	}
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	// SumFuka did this bit : give grenades up/outwards velocities
        VectorSet(grenade1,20,20,40);
        VectorSet(grenade2,20,-20,40);
        VectorSet(grenade3,-20,20,40);
        VectorSet(grenade4,-20, -20,40);
        VectorSet(grenade5,0,0,40);

        fire_grenade2(ent->owner, origin, grenade1, 120, 10, 1.8, 120, 140);
        fire_grenade2(ent->owner, origin, grenade2, 120, 10, 1.8, 120, 140);
        fire_grenade2(ent->owner, origin, grenade3, 120, 10, 1.8, 120, 140);
        fire_grenade2(ent->owner, origin, grenade4, 120, 10, 1.8, 120, 140);
        fire_grenade2(ent->owner, origin, grenade5, 120, 10, 1.8, 120, 140);
	G_FreeEdict (ent);
}


void homing_think (edict_t *ent)
{
       edict_t *target = NULL;
       edict_t *blip = NULL;
       vec3_t  end;
       vec3_t  dir;
       vec3_t start;

       while ((blip = findradius(blip, ent->s.origin, 500)) != NULL)
       {
               if (!(blip->svflags & SVF_MONSTER) && !blip->client)
                       continue;
               if (blip == ent->owner)
                       continue;
               if (!blip->takedamage)
                       continue;
               if (blip->health <= 0)
                       continue;
               if (!visible(ent, blip))
                       continue;
               if (!infront(ent, blip))
                       continue;
        //VectorCopy (self->enemy->s.origin, end);
        //end[2] += self->enemy->viewheight;
        //VectorSubtract (end, start, dir);


        VectorCopy (blip->s.origin, end);
        VectorCopy (ent->s.origin, start);
        
        end[2] += blip->viewheight;
        VectorSubtract (end, start, dir);

   //fire_blaster (ent, ent->owner, dir, 20, 350, EF_BLASTER, true);
		monster_fire_blaster (ent, 
			start, // ent->owner, 
			dir, 
			2, 
			1000, 
			MZ2_MEDIC_BLASTER_1, 
			EF_BLASTER);
        }
        
       ent->nextthink = level.time + .6;
}


void Proxim_Think (edict_t *ent)
{
       edict_t *blip = NULL;
       int arpa;

       if (level.time > ent->delay)
       {
               Grenade_Explode(ent);
       }
 
       ent->think = Proxim_Think;
       while ((blip = findradius(blip, ent->s.origin, 100)) != NULL)
       {
               if (!(blip->svflags & SVF_MONSTER) && (!blip->client & !blip->player))
                       continue;
               if (blip == ent->owner)
                       continue;
               if (!blip->takedamage)
                       continue;
               if (blip->health <= 0)
                       continue;
               if (!visible(ent, blip))
                       continue;
arpa = 1 + (int)(random() * 5.0);
arpa = (int) arpa;
                    if (arpa == 5)
                    ent->think = Cluster_Explode;
                    else
                    //if (arpa == 4)
                    //ent->think = homing_think;
                    //else
                    {
                    ent->think = Grenade_Explode;
                    }
               break;
       }

       ent->nextthink = level.time + .2;
}


void SP_boobytrap (edict_t *owner, int type)
{
        
      edict_t *blip = NULL;
	vec3_t		forward,
				wallp;

	trace_t		tr;
	gitem_t		*item;
      edict_t *self;


	// valid ent ?
        if ((!owner->client) || (owner->health<=0))
	   return;

	// Setup "little look" to close wall
        VectorCopy(owner->s.origin,wallp);         

	// Cast along view angle
        AngleVectors (owner->client->v_angle, forward, NULL, NULL);

	// Setup end point
        wallp[0]=owner->s.origin[0]+forward[0]*150;
        wallp[1]=owner->s.origin[1]+forward[1]*150;
        wallp[2]=owner->s.origin[2]+forward[2]*150;  

	// trace
        tr = gi.trace (owner->s.origin, NULL, NULL, wallp, owner, MASK_SOLID);

	// Line complete ? (ie. no collision)
        if (tr.fraction != 1.0)
	{
                gi.cprintf (owner, PRINT_HIGH, "Too close to wall.\n");
		return;
	}


       while ((blip = findradius(blip, owner->s.origin, 150)) != NULL)
       {
               if (blip->client)
               continue;

               if (!strcmp(blip->classname, "trap")) {
               gi.cprintf (owner, PRINT_HIGH, "Another Trap Too Close\n");
               return;
       }
       }

       while ((blip = findradius(blip, owner->s.origin, 300)) != NULL)
       {
               if (blip->client) if (!blip->gameleader) {
               gi.cprintf (owner, PRINT_HIGH, "Players are nearby,cant place\n");
               return;
               }
               //continue;

       
       }

	// Hit sky ?
	if (tr.surface)
		if (tr.surface->flags & SURF_SKY)
			return;
    

      //it's ok to spawn it now

      self = G_Spawn();

        item = FindItem("Cells");
        owner->client->pers.selected_item = ITEM_INDEX(item);
        owner->client->pers.inventory[owner->client->pers.selected_item] = owner->client->pers.inventory[owner->client->pers.selected_item] - 10;

      self->owner = owner;    //for the decoy, this is a pointer to the owner

      AngleVectors(owner->client->v_angle, forward, NULL, NULL);
      VectorMA(owner->s.origin, 100, forward, self->s.origin);
      self->s.angles[PITCH] = owner->s.angles[PITCH];
      self->s.angles[YAW] = owner->s.angles[YAW];
      self->s.angles[ROLL] = owner->s.angles[ROLL];
	gi.WriteByte (svc_muzzleflash);
        gi.WriteShort (self-g_edicts);
	gi.WriteByte (MZ_LOGIN);
        gi.multicast (self->s.origin, MULTICAST_PVS);

        self->dmg = 120;
        self->dmg_radius = 110;
        self->clipmask = MASK_SHOT;
        self->solid = SOLID_BBOX;
        if ((type != 3) & (type != 2)) self->s.effects |= EF_ROTATE;
        self->movetype = MOVETYPE_TOSS;
        self->s.modelindex = gi.modelindex ("models/items/armor/body/tris.md2");
        if (type == 1) 
        self->s.modelindex = gi.modelindex ("models/items/quaddama/tris.md2");
        if (type == 2) 
        self->s.modelindex = gi.modelindex ("models/items/mega_h/tris.md2");
        if (type == 3) 
        self->s.modelindex = gi.modelindex ("models/objects/barrels/tris.md2");

        self->classname = "trap";
        self->think = Proxim_Think;
        self->nextthink = level.time + 1;
        self->delay = level.time + 180;
        if ((type != 3) & (type != 2)) VectorSet (self->mins, -16, -16, -24);
        else {
        if (type == 3) VectorSet (self->mins, -16, -16, 0);
        else
        VectorSet (self->mins, -16, -16, -16);
        }
        VectorSet (self->maxs, 16, 16, 32);

        self->mass = 40;
    
    	gi.linkentity (self);

}

void mine_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
        self->think = Grenade_Explode;
}


void Mine_Think (edict_t *ent)
{
       edict_t *blip = NULL;

       if (level.time > ent->delay)
       {
               Cluster_Explode(ent);
       }
 
       ent->think = Mine_Think;
       while ((blip = findradius(blip, ent->s.origin, 60)) != NULL)
       {
               if (!(blip->svflags & SVF_MONSTER) && (!blip->client & !blip->player))
                       continue;
               if (blip == ent->owner)
                       continue;
               if (!blip->takedamage)
                       continue;
               if (blip->health <= 0)
                       continue;
               if (!visible(ent, blip))
                       continue;
                    ent->think = Cluster_Explode;
               break;
       }
       if (ent->health <= 0) Cluster_Explode(ent);
       ent->nextthink = level.time + .2;
}



void SP_minetrap (edict_t *owner, int type)
{
        
      edict_t *blip = NULL;
	vec3_t		forward,
				wallp;

	trace_t		tr;
	gitem_t		*item;
      edict_t *self;


	// valid ent ?
        if ((!owner->client) || (owner->health<=0))
	   return;

	// Setup "little look" to close wall
        VectorCopy(owner->s.origin,wallp);         

	// Cast along view angle
        AngleVectors (owner->client->v_angle, forward, NULL, NULL);

	// Setup end point
        wallp[0]=owner->s.origin[0]+forward[0]*150;
        wallp[1]=owner->s.origin[1]+forward[1]*150;
        wallp[2]=owner->s.origin[2]+forward[2]*150;  

	// trace
        tr = gi.trace (owner->s.origin, NULL, NULL, wallp, owner, MASK_SOLID);

	// Line complete ? (ie. no collision)
        if (tr.fraction != 1.0)
	{
                gi.cprintf (owner, PRINT_HIGH, "Too close to wall.\n");
		return;
	}


       while ((blip = findradius(blip, owner->s.origin, 150)) != NULL)
       {
               if (blip->client)
               continue;

               if (!strcmp(blip->classname, "trap")) {
               gi.cprintf (owner, PRINT_HIGH, "Another Trap Too Close\n");
               return;
       }
       }

       while ((blip = findradius(blip, owner->s.origin, 300)) != NULL)
       {
               if (blip->client) if (!blip->gameleader) {
               gi.cprintf (owner, PRINT_HIGH, "Players are nearby,cant place\n");
               return;
               }
               //continue;

       
       }

	// Hit sky ?
	if (tr.surface)
		if (tr.surface->flags & SURF_SKY)
			return;
    

      //it's ok to spawn it now

      self = G_Spawn();

        item = FindItem("Cells");
      //  owner->client->pers.selected_item = ITEM_INDEX(item);
      //  index = ITEM_INDEX(it);
        owner->client->pers.inventory[ITEM_INDEX(item)] = owner->client->pers.inventory[ITEM_INDEX(item)] - 2;

      self->owner = owner;    //for the decoy, this is a pointer to the owner

      AngleVectors(owner->client->v_angle, forward, NULL, NULL);
      VectorMA(owner->s.origin, 100, forward, self->s.origin);
      self->s.angles[PITCH] = owner->s.angles[PITCH];
      self->s.angles[YAW] = owner->s.angles[YAW];
      self->s.angles[ROLL] = owner->s.angles[ROLL];
	gi.WriteByte (svc_muzzleflash);
        gi.WriteShort (self-g_edicts);
	gi.WriteByte (MZ_LOGIN);
        gi.multicast (self->s.origin, MULTICAST_PVS);

	self->solid = SOLID_BBOX;
        self->die = mine_die;
        self->health = 30;
        self->dmg = 120;
        self->dmg_radius = 110;
        self->clipmask = MASK_SHOT;
        self->solid = SOLID_BBOX;
        self->s.modelindex = gi.modelindex ("models/objects/mine/tris.md2");
        self->movetype = MOVETYPE_TOSS;
        self->classname = "trap";
        self->think = Mine_Think;
        self->nextthink = level.time + 5;
        self->delay = level.time + 300;
        VectorSet (self->mins, -16, -16, 0);
        VectorSet (self->maxs, 16, 16, 8);
	self->takedamage = DAMAGE_AIM;
        self->mass = 40;
    
    	gi.linkentity (self);

}




