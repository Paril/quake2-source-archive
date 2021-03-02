#include "g_local.h"

#define newSupplyDepot self->supply
void SP_SupplyDepot(edict_t *self);
void supplydepot_explode (edict_t *self);
/*
=================
Supply Depot
=================
*/

void SupplyThink (edict_t *self)
{
	edict_t *other;
	int x;
	int contents;
	other = NULL;
	x = 0;

	contents = (int)gi.pointcontents(self->s.origin);

	if (contents & CONTENTS_SOLID)
	{
		safe_cprintf(self->owner, PRINT_HIGH, "Your supply depot was in a bad map position, so it was removed.\n");//5/99
		supplydepot_explode (self);
		return;
	}


	while ((other = findradius(other, self->s.origin, 16)) != NULL)
	{
		if (other->client)
		{
			x = 0;
//			other->DrunkTime=0;
//			other->disease= 0;
//			other->lame = 0;

    //destroy depot for bots if enemy is using it
	if ((self->wf_team != other->wf_team) && (self->owner->bot_client))
	{	supplydepot_explode (self);
		return;
    }
			if ((self->wf_team != other->wf_team) && (self->owner))
				safe_cprintf(self->owner, PRINT_HIGH, "Enemies are using your depot!\n");


			if (other->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))]<other->client->pers.max_bullets)
			{
				x =1;
				other->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))]+= 1;
			}
			if (other->client->pers.inventory[ITEM_INDEX(FindItem("shells"))]<other->client->pers.max_shells)
			{
				x=1;
				other->client->pers.inventory[ITEM_INDEX(FindItem("shells"))]+= 1;
			}
			if (other->client->pers.inventory[ITEM_INDEX(FindItem("grenades"))]<other->client->pers.max_grenades)
			{
				x=1;
				other->client->pers.inventory[ITEM_INDEX(FindItem("grenades"))]+= 1;
			}
			if (other->client->pers.inventory[ITEM_INDEX(FindItem("rockets"))]<other->client->pers.max_rockets)
			{
				x=1;
				other->client->pers.inventory[ITEM_INDEX(FindItem("rockets"))]+= 1;
			}
			if (other->client->pers.inventory[ITEM_INDEX(FindItem("slugs"))]<other->client->pers.max_slugs)
			{
				x=1;
				other->client->pers.inventory[ITEM_INDEX(FindItem("slugs"))]+= 1;
			}
			if (other->client->pers.inventory[ITEM_INDEX(FindItem("cells"))]<other->client->pers.max_cells)
			{
				x=1;
				other->client->pers.inventory[ITEM_INDEX(FindItem("cells"))]+= 1;
			}
		}
	}

	if (x ==1)
		gi.sound(self, CHAN_ITEM, gi.soundindex("misc/w_pkup.wav"), 1, ATTN_NORM, 0);

	self->nextthink = level.time + 0.2;
}

// Creator destroyed it
void supplydepot_explode (edict_t *self)
{
	edict_t *blip = NULL;
	int i;
//42 bot supply clear itemnode owner
if (self->owner->bot_client)
	for (i=1, blip=g_edicts+i ; i < globals.num_edicts ; i++,blip++)
	{
 	  if (blip->owner == self->owner)
	  {
		 if (!strcmp(blip->classname, "item_depotspot") )
		{
			blip->owner = NULL;
		}
	  }
	}
//42 end clear item owner

	gi.WriteByte (svc_temp_entity);
	if (self->waterlevel)
		gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	else
		gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);
	make_debris(self);

	self->takedamage = DAMAGE_NO;

	if (self->owner) safe_cprintf(self->owner, PRINT_HIGH, "Supply Depot Off.\n");

	if ((self->owner) && (self->owner->supply))
		self->owner->supply = NULL;

	T_RadiusDamage(self, self->owner, self->dmg, NULL, self->dmg_radius, MOD_DEPOT);
	G_FreeEdict(self);

}

// some other player blew it up
void supplydepot_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	edict_t *blip = NULL;
	int i;

//42 bot supply clear itemnode owner
if (self->owner->bot_client)
	for (i=1, blip=g_edicts+i ; i < globals.num_edicts ; i++,blip++)
	{
 	  if (blip->owner == self->owner)
	  {
		 if (!strcmp(blip->classname, "item_depotspot") )
		{
			blip->owner = NULL;
		}
	  }
	}
//42 end clear item owner

	gi.WriteByte (svc_temp_entity);
	if (self->waterlevel)
		gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	else
		gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);
	make_debris(self);

	self->takedamage = DAMAGE_NO;

	if (self->owner)
	{
		safe_cprintf(self->owner, PRINT_HIGH, "Supply Depot Destroyed.\n");
		if (self->owner->supply)
			self->owner->supply = NULL;
	}

//	T_RadiusDamage(self, self->owner, self->dmg, NULL, self->dmg_radius, MOD_DEPOT);
	T_RadiusDamage(self, attacker, self->dmg, NULL, self->dmg_radius, MOD_DEPOT_EXPLODE);
	G_FreeEdict(self);

}

void SP_SupplyDepot(edict_t *self)
{
	if ( newSupplyDepot )
	{
		supplydepot_explode(newSupplyDepot);
		return;
	}

	// cells for laser ?
if (!self->bot_client)
{
	if (self->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] < 10)
	{
 		safe_cprintf(self, PRINT_HIGH, "Need 10 cells for a Supply Depot.\n");
		return;
	}
	if (self->client->pers.inventory[ITEM_INDEX(FindItem("Bullets"))] < 10)
	{
 		safe_cprintf(self, PRINT_HIGH, "Need 10 bullets for a Supply Depot.\n");
		return;
	}
	if (self->client->pers.inventory[ITEM_INDEX(FindItem("Shells"))] < 10)
	{
 		safe_cprintf(self, PRINT_HIGH, "Need 10 shells for a Supply Depot.\n");
		return;
	}
	if (self->client->pers.inventory[ITEM_INDEX(FindItem("Rockets"))] < 10)
	{
 		safe_cprintf(self, PRINT_HIGH, "Need 10 rockets for a Supply Depot.\n");
		return;
	}
	if (self->client->pers.inventory[ITEM_INDEX(FindItem("Grenades"))] < 10)
	{
 		safe_cprintf(self, PRINT_HIGH, "Need 10 grenades for a Supply Depot.\n");
gi.dprintf("you only have %d grenades.\n",self->client->pers.inventory[ITEM_INDEX(FindItem("Grenades"))] );
		return;
	}
	if (self->client->pers.inventory[ITEM_INDEX(FindItem("Slugs"))] < 10)
	{
 		safe_cprintf(self, PRINT_HIGH, "Need 10 slugs for a Supply Depot.\n");
		return;
	}
	safe_cprintf(self, PRINT_HIGH, "Supply Depot on.\n");
}
	newSupplyDepot = G_Spawn ();
	VectorCopy(self->s.origin,newSupplyDepot->s.origin);
	newSupplyDepot->s.origin[2] += 40;
	newSupplyDepot->classname="depot";
	newSupplyDepot->takedamage=DAMAGE_AIM;
	newSupplyDepot->movetype= MOVETYPE_TOSS;
	newSupplyDepot->mass = 200;
	newSupplyDepot->solid = SOLID_BBOX;
	//newSupplyDepot->clipmask=MASK_ALL;
	newSupplyDepot->deadflag =DEAD_NO;
	newSupplyDepot->clipmask = MASK_SHOT;
	newSupplyDepot->model = self->model;
	newSupplyDepot->s.modelindex = gi.modelindex ("models/objects/dmspot/tris.md2");
	newSupplyDepot->s.skinnum = 1;
	newSupplyDepot->solid = SOLID_BBOX;
	newSupplyDepot->noteamdamage = true;	//Don't let teammates damage it
	newSupplyDepot->wf_team = self->wf_team;

	VectorSet (newSupplyDepot->mins, -32, -32, -24);
	VectorSet (newSupplyDepot->maxs, 32, 32, -16);
	newSupplyDepot->s.frame =0;
	newSupplyDepot->waterlevel = 0;
	newSupplyDepot->watertype=0;
	newSupplyDepot->health= 100;
	newSupplyDepot->max_health =100;
	newSupplyDepot->gib_health = -80;
	newSupplyDepot->die = supplydepot_die;
	newSupplyDepot->owner = self;
	newSupplyDepot->dmg = 150;
	newSupplyDepot->dmg_radius = 160;
//	newSupplyDepot->touch = SupplyTouch;
	newSupplyDepot->think =SupplyThink;
	newSupplyDepot->nextthink = level.time +1;
	VectorClear (newSupplyDepot->velocity);
	gi.linkentity (newSupplyDepot);
if (!self->bot_client)
{
	self->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))]-= 10;

	self->client->pers.inventory[ITEM_INDEX(FindItem("shells"))]-= 10;

	self->client->pers.inventory[ITEM_INDEX(FindItem("grenades"))]-= 10;

	self->client->pers.inventory[ITEM_INDEX(FindItem("rockets"))]-= 10;

	self->client->pers.inventory[ITEM_INDEX(FindItem("slugs"))]-= 10;

	self->client->pers.inventory[ITEM_INDEX(FindItem("cells"))]-= 10;

	if (self->client) gi.centerprintf (self,"Supply Depot set!\nNote: 10 of each ammo has been\ndrained into the Supply Depot to be\nreplicated\n");
}
}