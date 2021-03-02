#include "g_local.h"
/*
==================
Painberry's Commands

==================
*/
//PainBerry
void Cmd_Leg_f (edict_t *ent)
{
	Decide_attack (ent);
}

void Cmd_Jump_f (edict_t *ent)
{
	if (ent->groundentity)
	{
		if (ent->waterlevel == 0)
			ent->velocity[2] += 375;
		if (ent->waterlevel == 1)
			ent->velocity[2] += 200;
		if (ent->waterlevel == 2)
			ent->velocity[2] += 150;
		if (ent->waterlevel == 3)
			ent->velocity[2] += 100;
	}
}

void Cmd_BuySpeed_f (edict_t *ent)		//this is now a spell. i couldn't see much point of it being a level.
{
	
	if(strcmp(ent->client->pers.weapon->classname, "weapon_knives") == 0 || 
		strcmp(ent->client->pers.weapon->classname, "weapon_fists") == 0)
	{
	if (ent->client->speed_framenum > level.framenum - 100)
	{
		gi.cprintf (ent, PRINT_HIGH, "You can't buy until recharge is complete:%f seconds\n", (ent->client->speed_framenum - level.framenum + 100)/10);
		return;
	}
	if (SpellFull(ent))
	{
		gi.cprintf (ent, PRINT_HIGH, "You can only have 2 spells at a time\n");
		return;
	}
	if ( ent->stamina < 100)
	{
		gi.cprintf (ent, PRINT_HIGH, "You dont have enough energy stored\n");
		return;
	}
	
	ent->stamina -= 100;
	gi.bprintf (PRINT_HIGH, "%s goes into REDICULOUS SPEEEED (TM)\n", ent->client->pers.netname);
	if (ent->client->speed_framenum > level.framenum)
		ent->client->speed_framenum += 150;
	else
		ent->client->speed_framenum = level.framenum + 150;
	
	if(ent->client->speed_framenum - level.framenum > 150)
		ent->client->speed_framenum = level.framenum + 150;	
	
	gi.sound (ent, CHAN_VOICE, gi.soundindex ("thingon.wav"), 1, ATTN_NORM, 0);
	//gi.sound (ent, CHAN_VOICE, gi.soundindex ("thingoff.wav"), 1, ATTN_NORM, 0);

	MatrixSpeed(ent);
	}
	else
		gi.cprintf (ent, PRINT_HIGH, "You can't buy speed while using guns.\n", (ent->client->speed_framenum - level.framenum + 100)/10);

}

void Cmd_Infrared_f (edict_t *ent) // PSY: IR Goggles
{
	if (ent->client->ir_framenum > level.framenum - 100)
	{
		gi.cprintf (ent, PRINT_HIGH, "You can't buy until recharge is complete:%f seconds\n", (ent->client->speed_framenum - level.framenum + 100)/10);
		return;
	}
	if (ent->stamina < 75)
	{
		gi.cprintf (ent, PRINT_HIGH, "You dont have enough energy stored\n");
		return;
	}
	if (SpellFull(ent))
	{
		gi.cprintf (ent, PRINT_HIGH, "You can only have 2 spells at a time\n");
		return;
	}
	if (ent->client->ir_framenum > level.framenum)
		ent->client->ir_framenum += 600;
	else
		ent->client->ir_framenum = level.framenum + 600;


	ent->stamina -= 75;

	gi.bprintf (PRINT_HIGH, "%s has nightvision\n", ent->client->pers.netname);
	gi.sound(ent, CHAN_ITEM, gi.soundindex("ir_on.wav"), 1, ATTN_NORM, 0);
		ent->client->goggles = 1;
		ent->client->ps.rdflags |= RDF_IRGOGGLES;	


}  

void Cmd_Cloak_f (edict_t *ent) 
{
	
	if (ent->client->cloak_framenum > level.framenum - 100)
	{
		gi.cprintf (ent, PRINT_HIGH, "You can't buy until recharge is complete:%f seconds\n", (ent->client->speed_framenum - level.framenum + 100)/10);
		return;
	}
	if (SpellFull(ent))
	{
		gi.cprintf (ent, PRINT_HIGH, "You can only have 2 spells at a time\n");
		return;
	}
	if (ent->stamina < 100)
	{
		gi.cprintf (ent, PRINT_HIGH, "You dont have enough energy stored\n");
		return;
	}
	
	if (ent->client->cloak_framenum > level.framenum + 100)
		ent->client->cloak_framenum += 600;
	else
		ent->client->cloak_framenum = level.framenum + 600;

	ent->stamina -= 100;
	
	gi.bprintf (PRINT_HIGH, "%s's body becomes translucent.\n", ent->client->pers.netname);
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BOSSTPORT);
	gi.WritePosition (ent->s.origin);
	gi.multicast (ent->s.origin, MULTICAST_ALL);
	ent->s.modelindex2 = 0;
	gi.sound(ent, CHAN_ITEM, gi.soundindex("cloak.wav"), 1, ATTN_NORM, 0);
}  

/*
===================
Fear's Commands
===================
*//*
void Cmd_JoinTeam_f (edict_t *ent)
{
	char *s = gi.argv(1);
	
	if (!teamplay->value)
	{
		JoinTeam(ent, TEAM_FREE);
		return;
	}
	if (Q_stricmp(s, redteamname->string)==0)
		JoinTeam(ent, TEAM_RED);
	else if (Q_stricmp(s, blueteamname->string)==0)
		JoinTeam(ent, TEAM_BLUE);
	else if ((Q_stricmp(s, "none")==0) || (Q_stricmp(s, "leave")==0))
	{
		ent->client->resp.spectator = true;
		ent->client->resp.team = 0;
		spectator_respawn(ent);
	}
	else
		gi.cprintf(ent,PRINT_HIGH, "You are on the %s team\n", TeamName(ent->client->resp.team));

}
void Cmd_Ready_f (edict_t *ent)
{
	if (ent->client->resp.ready)
		ent->client->resp.ready = false;
	else
		ent->client->resp.ready = true;
	gi.bprintf (PRINT_HIGH, "%s is%sready\n", ent->client->pers.netname, ent->client->resp.ready ? " ":" not ");
	MatrixWhosReady();
}
*/

void Cmd_StopBullets_f (edict_t *ent)
{

	if(strcmp(ent->client->pers.weapon->classname, "weapon_knives") == 0 || 
		strcmp(ent->client->pers.weapon->classname, "weapon_fists") == 0)
	{
		
	

	if (ent->bullet_framenum > level.framenum - 100)
	{
		gi.cprintf (ent, PRINT_HIGH, "You can't buy until recharge is complete:%f seconds\n", (ent->client->speed_framenum - level.framenum + 100)/10);
		return;
	}
	if (SpellFull(ent))
	{
		gi.cprintf (ent, PRINT_HIGH, "You can only have 2 spells at a time\n");
		return;
	}
	if (ent->stamina < 75)
	{
		gi.cprintf (ent, PRINT_HIGH, "You dont have enough energy stored\n");
		return;
	}
	ent->stamina -= 75;
	
	if (ent->bullet_framenum > level.framenum + 100)
		ent->bullet_framenum += 300;
	else
		ent->bullet_framenum = level.framenum + 300;


	gi.bprintf (PRINT_HIGH, "%s defends himself from projectiles\n", ent->client->pers.netname);
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_NUKEBLAST);
	gi.WritePosition (ent->s.origin);
	gi.multicast (ent->s.origin, MULTICAST_ALL);
	gi.sound (ent, CHAN_VOICE, gi.soundindex ("laugh3.wav"), 1, ATTN_NORM, 0);

	}
	else
		gi.cprintf (ent, PRINT_HIGH, "You can't buy bullet stopping while using guns.\n", (ent->client->speed_framenum - level.framenum + 100)/10);

}  


void Cmd_Lights_f (edict_t *ent)
{
	
	if (matrix.lightsout_framenum > level.framenum)
	{
		gi.cprintf (ent, PRINT_HIGH, "You can't buy until the effect has worn off\n");
		return;
	}
	if (SpellFull(ent))
	{
		gi.cprintf (ent, PRINT_HIGH, "You can only have 2 spells at a time\n");
		return;
	}
	if (ent->stamina < 200)
	{
		gi.cprintf (ent, PRINT_HIGH, "You dont have enough energy stored\n");
		return;
	}
	ent->stamina -= 200;
	
	if (matrix.lightsout_framenum > level.framenum)
		matrix.lightsout_framenum += 300;
	else
		matrix.lightsout_framenum = level.framenum + 300;
	
	if(matrix.lightsout_framenum - level.framenum > 300)
		matrix.lightsout_framenum = level.framenum + 300;
	matrix.triggerent = ent;
	
	matrix.lightsout = true;
	gi.bprintf (PRINT_HIGH, "%s explodes an emp device!!\n", ent->client->pers.netname);
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_NUKEBLAST);
	gi.WritePosition (ent->s.origin);
	gi.multicast (ent->s.origin, MULTICAST_ALL);

	
	
	// 0 normal
	gi.configstring(CS_LIGHTS+0, "bbbbaaaaaaaaaabbbbccdccbbbbbaaaaaaaaaaabcbbbaaaaamaaazoie");
	
}  


void Cmd_ScreenTilt_f (edict_t *ent)
{
	if(ent->screentilt)
	{
		gi.cprintf (ent, PRINT_HIGH, "Screen tilting off\n");
		ent->screentilt = false;
	}
	else
	{
		ent->screentilt = true;
		gi.cprintf (ent, PRINT_HIGH, "Screen tilting on\n");
	}
}

void cmd_dodgebullets_f (edict_t *ent)
{
	

	if( strcmp(ent->client->pers.weapon->classname, "weapon_knives") == 0 || 
		strcmp(ent->client->pers.weapon->classname, "weapon_fists") == 0)
	{

//  if (ent->dodge_framenum > level.framenum - 100)
//	{
//		gi.cprintf (ent, PRINT_HIGH, "Patience is a virtue...\n", (ent->client->speed_framenum - level.framenum + 100)/10);
//		return;
//	}
	

	if (SpellFull(ent))
	{
		gi.cprintf (ent, PRINT_HIGH, "You can only have 2 spells at a time\n");
		return;
	}
	
	if(ent->dodgethis)
	{

	if (ent->stamina < 1)///this is a test, it was originally 75...
	{
		gi.cprintf (ent, PRINT_HIGH, "You dont have enough energy stored\n");
		return;
	}

	//ent->stamina -= 1; ///this is a test, it was originally 75...
	
	if (ent->dodge_framenum > level.framenum + 100)
		ent->dodge_framenum += 300;
	else
		ent->dodge_framenum = level.framenum + 300;
	// gi.cprintf (ent, PRINT_HIGH, "hi i'm working!");
	//gi.bprintf (PRINT_HIGH, "%s defends himself from projectiles\n", ent->client->pers.netname);
//	gi.WriteByte (svc_temp_entity);
	//gi.WriteByte (TE_NUKEBLAST);
//	gi.WritePosition (ent->s.origin);
//	gi.multicast (ent->s.origin, MULTICAST_ALL);
//	gi.sound (ent, CHAN_VOICE, gi.soundindex ("laugh3.wav"), 1, ATTN_NORM, 0);
	}
	}
	else
		gi.cprintf (ent, PRINT_HIGH, "You can't dodge bullets while using guns.\n", (ent->client->speed_framenum - level.framenum + 100)/10);
	
}