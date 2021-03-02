#include "g_local.h"
#include "m_player.h"



void Client_EndFeign (edict_t *ent)
{
	gclient_t	*client=NULL;
		
	//Can't get another key for 10 seconds
	ent->client->next_key_pickup = level.time + pickuptime->value;

	
	//Can't end feign if you're dead
	if(ent->deadflag & DEAD_DEAD || !ent->inuse)
		return;

	//Movetype and animation
	ent->client->anim_priority = ANIM_BASIC;
	ent->client->ps.pmove.pm_type = PM_NORMAL;
	
	ent->client->is_feigning=false;
	
	client = ent->client;

	ent->movetype = MOVETYPE_WALK;
	ent->viewheight = 22;
	ent->flags &= ~FL_NO_KNOCKBACK;

	client->ps.pmove.pm_flags &= ~PMF_DUCKED;
	client->ps.fov = 90;
	client->ps.gunindex = gi.modelindex(client->pers.weapon->view_model);

	// clear entity state values
	ent->s.effects = 0;
	ent->s.modelindex2 = 255;		// custom gun model
	ent->s.frame = 0;
	ent->client->anim_end = 0;
	
	VectorCopy (ent->s.angles, client->ps.viewangles);
	VectorCopy (ent->s.angles, client->v_angle);

	gi.sound(ent, CHAN_BODY, gi.soundindex("*jump1.wav"), 1, ATTN_NORM, 0);

	gi.linkentity (ent);

}

void Client_BeginFeign (edict_t *self)
{
	gitem_t	*item = NULL;
	static int i;
	
	//Can't begin feign if you're dead
	if(self->deadflag & DEAD_DEAD || !self->inuse)
		return;

	//K2:Toss the key
	if (self->client->key)
	{
		K2_RemoveKeyFromInventory(self);
		K2_SpawnKey(self,self->client->key,2);
		SelectPrevItem(self, IT_POWERUP);
	}
		
	//K2: Hook - Release hook if needed
	if (self->client->hook)
		Release_Grapple(self->client->hook);
	
	VectorClear (self->avelocity);

	self->movetype = MOVETYPE_TOSS;

	self->s.modelindex2 = 0;	// remove linked weapon model

	self->s.angles[0] = 0;
	self->s.angles[2] = 0;

	self->s.sound = 0;
	self->client->weapon_sound = 0;

	self->maxs[2] = -8;

	self->client->ps.pmove.pm_flags |= PMF_DUCKED;
	
	//self->movetype = MOVETYPE_NONE;
	
	self->client->ps.pmove.pm_type = PM_DEAD;
			
	//Drop weapon
	item = self->client->pers.weapon;
	if (strcmp(item->pickup_name, "Blaster"))
	{
		//Change to blaster, then drop
		self->client->newweapon = FindItem("Blaster");
		ChangeWeapon (self);
		item->drop (self, item);
	}
	
		
	// start a feigned death animation
	i = (i+1)%3;
	self->client->anim_priority = ANIM_DEATH;
	if (self->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		self->s.frame = FRAME_crdeath1-1;
		self->client->anim_end = FRAME_crdeath4;
	}
	else switch (i)
	{
	case 0:
		self->s.frame = FRAME_death101-1;
		self->client->anim_end = FRAME_death105;
		break;
	case 1:
		self->s.frame = FRAME_death201-1;
		self->client->anim_end = FRAME_death205;
		break;
	case 2:
		self->s.frame = FRAME_death301-1;
		self->client->anim_end = FRAME_death307;
		break;
	}

	gi.sound (self, CHAN_VOICE, gi.soundindex(va("*death%i.wav", (rand()%4)+1)), 1, ATTN_NORM, 0);

	//Init Key Vars
	K2_ResetClientKeyVars(self);

	self->client->is_feigning = true;
	
	gi.linkentity (self);
}

