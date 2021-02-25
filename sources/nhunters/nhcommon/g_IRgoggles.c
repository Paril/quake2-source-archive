
#include "g_local.h"
#include "g_IRgoggles.h"

// Declare Use_Quad and Use_Invulnerability
void Use_Quad(edict_t *ent, gitem_t *item) ;
void Use_Invulnerability(edict_t *ent, gitem_t *item) ;

void	Use_IRgoggles (edict_t *ent, gitem_t *item)
{
        // This is an inventory item. We want to use the quad.
        IR_type_dropped = item ;
	ent->client->pers.inventory[ITEM_INDEX(item)]--;

        ValidateSelectedItem (ent);

	// For marines, set fov to cvar.
	if (!ent->isPredator)
		ent->client->ps.fov = getIRMarineFOV() ;

        // Make the time a config variable.
	if (ent->client->IRgoggles_framenum > level.framenum)
	  ent->client->IRgoggles_framenum += (getIREffectTime() * 10) ;
	else
	  // 30 seconds of energy
	  ent->client->IRgoggles_framenum =
	    level.framenum + (getIREffectTime() * 10) ;
	
}

void enable_IRgoggles() 
{

	// Find quad and invulnerability and change them
	gitem_t *it = FindItemByClassname("item_quad") ;
	it->use = Use_IRgoggles ;
	it->pickup_name = "IR goggles" ;

	it = FindItemByClassname("item_invulnerability") ;
	it->use = Use_IRgoggles ;
	it->pickup_name = "IR goggles" ;

}


void enable_Quad() 
{

	// Find quad and invulnerability and reset them back to normal.
	gitem_t *it = FindItemByClassname("item_quad") ;
	it->use = Use_Quad ;
	it->pickup_name = "Quad Damage" ;

	it = FindItemByClassname("item_invulnerability") ;
	it->use = Use_Invulnerability ;
	it->pickup_name = "Invulnerability" ;

}

void G_SetIREffects(edict_t *ent) {

  edict_t *ent2 ;
  int j ;

  // NH changes: IR goggles.
  // Is this player's IR goggles active?
  // Also set for players in observe mode.
  if ((ent->client->IRgoggles_framenum > level.framenum) ||
      (ent->isObserving && !ent->client->chase_target))
    {

      // If marine and not observing, set fov
//      if ((!ent->isPredator) && (!ent->isObserving))	// not needed while chasing
//	ent->client->ps.fov = getIRMarineFOV() ;	// not needed while chasing

      ent2 = &g_edicts[1];	// Start with first client

      // FIXME! Use a FindRadius instead?
      // Loop through all clients
      for (j = 1; j < maxclients->value; j++, ent2++) {

	if (!ent2->inuse)	// Skip all thats not active
	  continue;

	if (ent2 == ent)	// Skip self
	  continue;

	if (ent2->isObserving)	// Skip people observing
	  continue;

	if (ent2->deadflag)     // Skip dead ones.
	  continue ;
	
	// Set the effects and renderfx to a red shell.
	// Send a flash at ent2-player's origin,
	// only to the ent-player.
	gi.WriteByte (svc_temp_entity);
	
	// This is the only temp_entity that give a
	// somewhat proper flash of light.
	gi.WriteByte (TE_WELDING_SPARKS);	
	
	
	// Number of pixels. We need very very few!
	gi.WriteByte (1);			
	
	// Origin of light
	gi.WritePosition (ent2->s.origin);	
	
	// Not relevant, but didn't want to create a
	// variable just to contain zeros.
	gi.WriteDir (ent2->movedir);		
	
	// Color-Index for pixels
	gi.WriteByte (0);			

	// Don't make it reliable, it isn't so important
	gi.unicast(ent, false);			
      }
   }
  else {

    // Reset marine fov.
    if (!ent->isPredator) {
      if (atoi(Info_ValueForKey(
				ent->client->pers.userinfo, "fov")) >=90)
	ent->client->ps.fov = atoi(Info_ValueForKey(
			        ent->client->pers.userinfo, "fov"));
      else
	ent->client->ps.fov = 90 ;
     }
      
    }


  // Duplicated by Alex for setting IR when chasing a person who has IR..
  if ((ent->client->chase_target))
    {
    if (ent->client->chase_target->client->IRgoggles_framenum > level.framenum)
    {
      // If marine and not observing, set fov
      if ((!ent->isPredator) && (!ent->isObserving))
	ent->client->ps.fov = getIRMarineFOV() ;

      ent2 = &g_edicts[1];	// Start with first client

      // FIXME! Use a FindRadius instead?
      // Loop through all clients
      for (j = 1; j < maxclients->value; j++, ent2++) {

	if (!ent2->inuse)	// Skip all thats not active
	  continue;

	if (ent2 == ent)	// Skip self
	  continue;

	if (ent2->isObserving)	// Skip people observing
	  continue;

	if (ent2->deadflag)     // Skip dead ones.
	  continue ;

	if (ent2 == ent->client->chase_target) // Skip the person you are chasing
	  continue ;
	
	// Set the effects and renderfx to a red shell.
	// Send a flash at ent2-player's origin,
	// only to the ent-player.
	gi.WriteByte (svc_temp_entity);
	
	// This is the only temp_entity that give a
	// somewhat proper flash of light.
	gi.WriteByte (TE_WELDING_SPARKS);	
	
	
	// Number of pixels. We need very very few!
	gi.WriteByte (1);			
	
	// Origin of light
	gi.WritePosition (ent2->s.origin);	
	
	// Not relevant, but didn't want to create a
	// variable just to contain zeros.
	gi.WriteDir (ent2->movedir);		
	
	// Color-Index for pixels
	gi.WriteByte (0);			

	// Don't make it reliable, it isn't so important
	gi.unicast(ent, false);			

  }
 }
}
}
void deadDropIRgoggles(edict_t *self) 
{

  // Are we in the middle of using the IRgoggles?
  if (self->client->IRgoggles_framenum <= level.framenum) 
    return ;

  (void) Drop_Item(self, IR_type_dropped) ;

  // Temporary for test.
  self->client->IRgoggles_framenum = 0 ;

}

void validateIRMarineFOV() {

  // IR_marine_fov must be between 45 and 120
  if ((IR_marine_fov->value < 45) ||
      (IR_marine_fov->value > 120))
    gi.cvar_set("IR_marine_fov", IR_MARINE_FOV_DEFAULT );
}

int getIRMarineFOV() {

  // Check if the cvar has changed.
  if (IR_marine_fov->modified)
    validateIRMarineFOV() ;

  return IR_marine_fov->value ;
}

void validateIREffectTime() {

  // IR_effect_time must be between 0 and 120
  if ((IR_effect_time->value < 0) ||
      (IR_effect_time->value > 120))
    gi.cvar_set("IR_effect_time", IR_EFFECT_TIME_DEFAULT );
}

int getIREffectTime() {

  // Check if the cvar has changed.
  if (IR_effect_time->modified)
    validateIREffectTime() ;

  return IR_effect_time->value ;
}


