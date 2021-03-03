if (ent->anti)
{
	blip = NULL;
	while ((blip = findradius(blip, ent->s.origin, 500)) != NULL)
	{

		//The only non-client to track is sentry guns
		if ((strcmp(blip->classname, "rocket") != 0) && (strcmp(blip->classname, "proximity") != 0)&&(strcmp(blip->classname, "turret") != 0)&&(strcmp(blip->classname, "napalm grenade") != 0)&&(strcmp(blip->classname, "goodyear") != 0)&&(strcmp(blip->classname, "grenade") != 0))
			continue;
		if(ent == blip->owner)
			continue;
		tr = gi.trace (ent->s.origin, NULL, NULL, blip->s.origin, ent, MASK_SOLID);
		if (tr.fraction != 1.0)
			continue;
		ent->enemy = blip;
	}
	if(ent->enemy)
	{
	
		if (ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))]>0)
		{
			ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))]--;
			
			start[0] = ent->s.origin[0];
			start[1] = ent->s.origin[1];
			start[2] = ent->s.origin[2];

			// calc direction to where we targetd
			VectorMA (ent->enemy->s.origin, -0.05, ent->enemy->velocity, target);
			
			
			VectorSubtract (target, start, dir);
			VectorNormalize (dir);
			
			//fire bullet
			fire_bullet (ent, start, dir, 8, 0, 125, 125, MOD_SENTRY);

			// send muzzle flash
			gi.WriteByte (svc_muzzleflash);
			gi.WriteShort (ent - g_edicts);
			gi.WriteByte (MZ_SHOTGUN );
			gi.multicast (start, MULTICAST_PVS);
			gi.sound(ent, CHAN_VOICE, gi.soundindex("boss3\xfire.wav"), 1, ATTN_NORM, 0);
		}
		ent->enemy=NULL;
	}
}