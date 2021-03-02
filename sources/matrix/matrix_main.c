#include "g_local.h"
// Globals here
cvar_t	*sv_maxlevel;
cvar_t	*vid_ref;
cvar_t	*zk_logonly;
cvar_t	*teamplay;
cvar_t	*tankmode;
cvar_t	*weaponrespawntime;
cvar_t	*weaponban;
cvar_t	*laseroff;
cvar_t	*streakoff;
cvar_t	*shellsoff;
cvar_t	*redteamskin;
cvar_t	*redteamname;
cvar_t	*blueteamskin;
cvar_t	*blueteamname;
cvar_t	*matchtimelimit;
cvar_t	*teamfraglimit;
cvar_t	*matchmode;
cvar_t	*choosestuff;
cvar_t	*hop;
cvar_t	*action;
cvar_t	*faglimit; //max number of neos/morpheuses/trinities allowed on a server.


/*
=================
Weapon C Vars
=================
*/

//damage
	cvar_t	*damage_deserts;
	cvar_t	*damage_mk23;
	cvar_t	*damage_mp5;
	cvar_t	*damage_m4;
	cvar_t	*damage_pumps;
	cvar_t	*damage_smc;
	cvar_t	*damage_sniper;
	cvar_t	*damage_knife;
	cvar_t	*damage_fist;

//damage radius/radius damage (where applicable)
	cvar_t	*damageradius_rack; //how far the damage spreads out
	cvar_t	*damageradius_grenade;
	cvar_t	*radiusdamage_rack; //how much damage is done with distance.
	cvar_t	*radiusdamage_grenade;

//ammo use
	cvar_t	*ammo_deserts;
	cvar_t	*ammo_mk23;
	cvar_t	*ammo_mp5;
	cvar_t	*ammo_m4;
	cvar_t	*ammo_pumps;
	cvar_t	*ammo_smc;
	cvar_t	*ammo_sniper;
	cvar_t	*ammo_knife;
	cvar_t	*ammo_grenade;
	cvar_t	*ammo_rack;

//integrated kungfu damage
	cvar_t	*damage_jab;
	cvar_t	*damage_hook;
	cvar_t	*damage_uppercut;
	cvar_t	*damage_hoverkick;
	cvar_t	*damage_spinkick;

// integrated kungfu "reload" times.
	cvar_t	*reload_jab;
	cvar_t	*reload_hook;
	cvar_t	*reload_uppercut;
	cvar_t	*reload_hoverkick;
	cvar_t	*reload_spinkick;
//	cvar_t	*leper;
	cvar_t	*possesban;
	cvar_t  *combomessage;
	cvar_t  *killstreakmessage;
	cvar_t  *streakmessage2;
	cvar_t  *streakmessage3;
	cvar_t  *streakmessage4;
	cvar_t  *streakmessage5;
	cvar_t  *streakmessage6;
matrix_locals_t	matrix;
	// yep
// End globals

void MatrixClientFrame (edict_t *ent)
{
	if(strcmp(ent->client->pers.weapon->classname, "weapon_knives") != 0 && 
		strcmp(ent->client->pers.weapon->classname, "weapon_fists") != 0)
	{
		if(ent->client->speed_framenum > level.framenum)
		{
			ent->client->speed_framenum = level.framenum;
			gi.cprintf (ent, PRINT_HIGH, "Switched from fists to guns. Speed boost comprimised\n");
		}
		if(ent->bullet_framenum > level.framenum)
		{
			ent->bullet_framenum = level.framenum;
			gi.cprintf (ent, PRINT_HIGH, "Switched to guns. Bullet stopping forsaken\n");
		}
	}

	//matrix. every fucking one wants to be neo. this gets kinda confusing
//	if (faglimit->value)
//	{
//		if (strcmp(ent->client->pers.netname, "neo") == 0 || strcmp(ent->client->pers.netname, "morpheus") == 0 || strcmp(ent->client->pers.netname, "trinity") == 0)
//		{
//			gi.centerprintf (ent, "Please Don't Use Names From the Film.");
//			gi.centerprintf (ent, "Show some originality choo FAG.");
//			gi.centerprintf (ent, "type \"name <name>\" to change name.");
//			stuffcmd (ent, "disconnect\n");
//		}
//	}

	if (!ent->client->akimbo)
	{
		ent->client->next_hand = HAND_DEFAULT;
		ent->client->current_hand = HAND_DEFAULT;
	}
	if (VectorLength(ent->velocity))
		ent->last_move_time = level.time;
	
	if (ent->vertbuttons > 0)
		ent->jumping = true;
	if (ent->groundentity)
		ent->jumping = false;

	
	heavybreathing(ent);//sound to indicate stamina/mana
	MatrixCheckSpeed(ent);//was used to slow people down but stopped them using cl_forwardspeeed
	MatrixStamina(ent);//stamina think
	MatrixStopBullets(ent);

	
	dodgebullets(ent);
	
	MatrixSwapThink (ent);
	MatrixOlympics (ent);//is the player doing strafe jumps n shit? if so, reward him with coolio speed streaks.
	MatrixKungfu (ent);
	MatrixScreenTilt (ent);
	MatrixComboTally (ent);
	
	if(ent->client->use)
	        ent->client->ps.gunindex = 0;        
	if(ent->client->speed_framenum == level.framenum + 5)
		gi.sound (ent, CHAN_VOICE, gi.soundindex ("thingoff.wav"), 1, ATTN_NORM, 0);
	if(ent->client->ir_framenum == level.framenum + 5)
		gi.sound (ent, CHAN_VOICE, gi.soundindex ("ir_off.wav"), 1, ATTN_NORM, 0);
	if(ent->client->cloak_framenum == level.framenum + 5)
		gi.sound (ent, CHAN_VOICE, gi.soundindex ("decloak.wav"), 1, ATTN_NORM, 0);

	ent->OLDforwardbuttons = ent->forwardbuttons;
	ent->OLDstrafebuttons = ent->strafebuttons;
	ent->OLDvertbuttons = ent->vertbuttons;

}
void MatrixDoAtDeath (edict_t *ent)
{
}

void MatrixClientInit (edict_t *ent)//done
{

	//FetchClientEntData (ent);
	ent->screentilt = true;
	ent->client->goggles = 0;
	ent->client->ps.rdflags &= ~RDF_IRGOGGLES;
	ent->last_jump_time = 0;
	ent->last_use_time = 0;
	ent->bullet_framenum = 0;
	ent->dodge_framenum= 0;
	ent->pinned_nextthink = 0;
	ent->stamina = 100;
	ent->healthlevel = 0;
	ent->staminalevel = 0;
	ent->damagelevel = 0;
	ent->blocking = false;
	ent->max_stamina = 200;
	ent->breathdelay = 0;
	ent->matrixbuttons = 0;
	ent->client->next_kungfu = level.framenum;
	ent->oldgroundentity = NULL;
	ent->LastKill = level.framenum - 30;
	ent->KillStreak = 0;
	ent->LastHit = level.framenum - 30;
	ent->HitStreak = 0;
	ent->CumulativeDamage = 0;


}
void MatrixBeginDM (edict_t *ent)
{
	Matrix_MOTD(ent);
	if ((teamplay->value)||
		(matchmode->value)||
		(tankmode->value))
		//Cmd_JoinMenu_f(ent);
		
	stuffcmd(ent, "alias +button0 onbutton0\nalias -button0 offbutton0\nalias +button1 onbutton1\nalias -button1 offbutton1\nalias +boot booton\nalias -boot bootoff\nalias +attack2 booton\nalias -attack2 bootoff");
//	else if (!ent->client->resp.choosen && choosestuff->value)
//		Cmd_ChooseMenu_f(ent);
}
void MatrixFrame ()
{
	if(matrix.lightsout_framenum == level.framenum)
	{
		matrix.lightsout = false;
		gi.configstring(CS_LIGHTS+0, "m");//lights back to normal
	}

	MatrixMatchThink();
	MatrixTankThink();
}

void MatrixInit ()
{	
// MATRIX
	matchtimelimit = gi.cvar ("matchtimelimit", "0", CVAR_SERVERINFO|CVAR_ARCHIVE);
	teamfraglimit = gi.cvar ("teamfraglimit", "0", CVAR_SERVERINFO|CVAR_ARCHIVE);
	matchmode = gi.cvar ("matchmode", "0", CVAR_SERVERINFO|CVAR_ARCHIVE);
	teamplay = gi.cvar("teamplay", "0", CVAR_SERVERINFO|CVAR_ARCHIVE);
	tankmode = gi.cvar("tankmode", "0", CVAR_SERVERINFO|CVAR_ARCHIVE);

	weaponban = gi.cvar ("weaponban", "0", CVAR_SERVERINFO|CVAR_ARCHIVE);
	weaponrespawntime = gi.cvar ("weaponrespawntime", "15", CVAR_SERVERINFO|CVAR_ARCHIVE);

	sv_maxlevel = gi.cvar ("sv_maxlevel", "4", CVAR_SERVERINFO|CVAR_ARCHIVE);
	zk_logonly = gi.cvar("zk_logonly", "0", CVAR_SERVERINFO|CVAR_ARCHIVE);
	laseroff = gi.cvar("laseroff", "0", CVAR_SERVERINFO|CVAR_ARCHIVE);
	streakoff = gi.cvar("streakoff", "0", CVAR_SERVERINFO|CVAR_ARCHIVE);
	shellsoff = gi.cvar("shellsoff", "0", CVAR_SERVERINFO|CVAR_ARCHIVE);

	hop = gi.cvar("hop", "250", CVAR_SERVERINFO|CVAR_ARCHIVE);
	action = gi.cvar("action", "0", CVAR_SERVERINFO|CVAR_ARCHIVE);

	blueteamname = gi.cvar("blueteamname", "Hackers", 0);
	redteamname = gi.cvar("redteamname", "Agents", 0);
	redteamskin = gi.cvar("redteamskin", "ctf_r", 0);
	blueteamskin = gi.cvar("blueteamskin", "ctf_b", 0);

	faglimit  = gi.cvar("damage_deserts", "40", CVAR_SERVERINFO|CVAR_ARCHIVE); //max number of neos/morpheuses/trinities allowed on a server.

/*
=================
Weapon C Vars
=================
*/

//damage
	damage_deserts = gi.cvar("damage_deserts", "40", CVAR_SERVERINFO|CVAR_ARCHIVE);
	damage_mk23 = gi.cvar("damage_mk23", "30", CVAR_SERVERINFO|CVAR_ARCHIVE);
	damage_mp5 = gi.cvar("damage_mp5", "10", CVAR_SERVERINFO|CVAR_ARCHIVE);
	damage_m4 = gi.cvar("damage_m4", "15", CVAR_SERVERINFO|CVAR_ARCHIVE);
	damage_pumps = gi.cvar("damage_pumps", "6", CVAR_SERVERINFO|CVAR_ARCHIVE);
	damage_smc = gi.cvar("damage_smc", "4", CVAR_SERVERINFO|CVAR_ARCHIVE);
	damage_sniper = gi.cvar("damage_sniper", "100", CVAR_SERVERINFO|CVAR_ARCHIVE);
	damage_knife = gi.cvar("damage_knife", "50", CVAR_SERVERINFO|CVAR_ARCHIVE);
	damage_fist = gi.cvar("damage_fist", "80", CVAR_SERVERINFO|CVAR_ARCHIVE);

//damage radius/radius damage (where applicable)
	damageradius_rack = gi.cvar("damageradius_rack", "120", CVAR_SERVERINFO|CVAR_ARCHIVE); //how far the damage spreads out
	damageradius_grenade = gi.cvar("damageradius_grenade", "200", CVAR_SERVERINFO|CVAR_ARCHIVE);
	radiusdamage_rack = gi.cvar("radiusdamage_rack", "120", CVAR_SERVERINFO|CVAR_ARCHIVE); //how much damage is done with distance.
	radiusdamage_grenade = gi.cvar("radiusdamage_grenade", "110", CVAR_SERVERINFO|CVAR_ARCHIVE);

//ammo use
	ammo_deserts = gi.cvar("ammo_deserts", "15", CVAR_SERVERINFO|CVAR_ARCHIVE);
	ammo_mk23 = gi.cvar("ammo_mk23", "10", CVAR_SERVERINFO|CVAR_ARCHIVE);
	ammo_mp5 = gi.cvar("ammo_mp5", "3", CVAR_SERVERINFO|CVAR_ARCHIVE);
	ammo_m4 = gi.cvar("ammo_m4", "4", CVAR_SERVERINFO|CVAR_ARCHIVE);
	ammo_pumps = gi.cvar("ammo_pumps", "20", CVAR_SERVERINFO|CVAR_ARCHIVE);
	ammo_smc = gi.cvar("ammo_smc", "1", CVAR_SERVERINFO|CVAR_ARCHIVE);
	ammo_sniper = gi.cvar("ammo_sniper", "40", CVAR_SERVERINFO|CVAR_ARCHIVE);
	ammo_knife = gi.cvar("ammo_knife", "20", CVAR_SERVERINFO|CVAR_ARCHIVE);
	ammo_grenade = gi.cvar("ammo_grenade", "30", CVAR_SERVERINFO|CVAR_ARCHIVE);
	ammo_rack = gi.cvar("ammo_rack", "50", CVAR_SERVERINFO|CVAR_ARCHIVE);

//integrated kungfu damage
	damage_jab = gi.cvar("damage_jab", "10", CVAR_SERVERINFO|CVAR_ARCHIVE);
	damage_hook = gi.cvar("damage_hook", "40", CVAR_SERVERINFO|CVAR_ARCHIVE);
	damage_uppercut = gi.cvar("damage_uppercut", "80", CVAR_SERVERINFO|CVAR_ARCHIVE);
	damage_hoverkick = gi.cvar("damage_hoverkick", "40", CVAR_SERVERINFO|CVAR_ARCHIVE);
	damage_spinkick = gi.cvar("damage_spinkick", "60", CVAR_SERVERINFO|CVAR_ARCHIVE);

// integrated kungfu "reload" times.
	reload_jab = gi.cvar("reload_jab", "2", CVAR_SERVERINFO|CVAR_ARCHIVE);
	reload_hook = gi.cvar("reload_hook", "10", CVAR_SERVERINFO|CVAR_ARCHIVE);
	reload_uppercut = gi.cvar("reload_uppercut", "18", CVAR_SERVERINFO|CVAR_ARCHIVE);
	reload_hoverkick = gi.cvar("reload_hoverkick", "0", CVAR_SERVERINFO|CVAR_ARCHIVE);
	reload_spinkick = gi.cvar("reload_spinkick", "10", CVAR_SERVERINFO|CVAR_ARCHIVE);
//	leper =  gi.cvar("leper", "1", CVAR_SERVERINFO|CVAR_ARCHIVE);
	possesban =  gi.cvar("possesban", "0", CVAR_SERVERINFO|CVAR_ARCHIVE);
	combomessage =  gi.cvar("combomessage", "1", CVAR_SERVERINFO|CVAR_ARCHIVE);
	killstreakmessage =  gi.cvar("killstreakmessage", "1", CVAR_SERVERINFO|CVAR_ARCHIVE);
	
	streakmessage2 = gi.cvar("streakmessage2", "Kill Streak", CVAR_SERVERINFO|CVAR_ARCHIVE);
	streakmessage3 = gi.cvar("streakmessage3", "Oooooooohhh Yeeeaaaah", CVAR_SERVERINFO|CVAR_ARCHIVE);
	streakmessage4 = gi.cvar("streakmessage4", "Hmmmm mmmmm mmm!", CVAR_SERVERINFO|CVAR_ARCHIVE);
	streakmessage5 = gi.cvar("streakmessage5", "Musn't Grrumble!", CVAR_SERVERINFO|CVAR_ARCHIVE);
	streakmessage6 = gi.cvar("streakmessage6", "M-M-M-Monster Frag", CVAR_SERVERINFO|CVAR_ARCHIVE);

	matrix.started = false;
	matrix.team_blue_score = 0;
	matrix.team_red_score = 0;
	matrix.start_tank_time = 0;
	matrix.lightsout = false;
	matrix.lightsout_framenum = 0;
	matrix.count_started = false;

	// MATRIX
}

int MatrixDamage(edict_t *targ, edict_t *attacker, int damage)
{
	if (attacker->damagelevel)
		damage *= (attacker->damagelevel / 2)+1;

	if ((targ->blocking) && (!targ->deadflag) && targ->stamina)
	{
		damage /= 2;
		targ->s.frame = 116;
		targ->client->anim_end = 122;
		if (damage > targ->stamina)
			targ->stamina = 0;
		else
			targ->stamina -= damage;// People end up suiciding if damage is high enough eg quad rails/rockets
	}
	return damage;
}

void MatrixSetStats (edict_t *ent)
{	
	vec3_t  forward, start, kick, blah;
	trace_t tr;
	qboolean	timer1taken = false;
	qboolean	timer2taken = false;
	//painberry
	// stamina
	//
	if(ent->stamina > 0)
	ent->client->ps.stats[STAT_STAMINA_ICON] = gi.imageindex ("st1");
	if(ent->stamina > 10)
	ent->client->ps.stats[STAT_STAMINA_ICON] = gi.imageindex ("st2");
	if(ent->stamina > 20)
	ent->client->ps.stats[STAT_STAMINA_ICON] = gi.imageindex ("st3");
	if(ent->stamina > 30)
	ent->client->ps.stats[STAT_STAMINA_ICON] = gi.imageindex ("st4");
	if(ent->stamina > 40)
	ent->client->ps.stats[STAT_STAMINA_ICON] = gi.imageindex ("st5");
	if(ent->stamina > 50)
	ent->client->ps.stats[STAT_STAMINA_ICON] = gi.imageindex ("st6");
	if(ent->stamina > 60)
	ent->client->ps.stats[STAT_STAMINA_ICON] = gi.imageindex ("st7");
	if(ent->stamina > 70)
	ent->client->ps.stats[STAT_STAMINA_ICON] = gi.imageindex ("st8");
	if(ent->stamina > 80)
	ent->client->ps.stats[STAT_STAMINA_ICON] = gi.imageindex ("st9");
	if(ent->stamina > 90)
	ent->client->ps.stats[STAT_STAMINA_ICON] = gi.imageindex ("st10");
	if(ent->stamina > 100)
	ent->client->ps.stats[STAT_STAMINA_ICON] = gi.imageindex ("st11");
	if(ent->stamina > 110)
	ent->client->ps.stats[STAT_STAMINA_ICON] = gi.imageindex ("st12");
	if(ent->stamina > 120)
	ent->client->ps.stats[STAT_STAMINA_ICON] = gi.imageindex ("st13");
	if(ent->stamina > 130)
	ent->client->ps.stats[STAT_STAMINA_ICON] = gi.imageindex ("st14");
	if(ent->stamina > 140)
	ent->client->ps.stats[STAT_STAMINA_ICON] = gi.imageindex ("st15");
	if(ent->stamina > 150)
	ent->client->ps.stats[STAT_STAMINA_ICON] = gi.imageindex ("st16");
	if(ent->stamina > 160)
	ent->client->ps.stats[STAT_STAMINA_ICON] = gi.imageindex ("st17");
	if(ent->stamina > 170)
	ent->client->ps.stats[STAT_STAMINA_ICON] = gi.imageindex ("st18");
	if(ent->stamina > 180)
	ent->client->ps.stats[STAT_STAMINA_ICON] = gi.imageindex ("st19");
	if(ent->stamina > 190)
	ent->client->ps.stats[STAT_STAMINA_ICON] = gi.imageindex ("st20");
	if(ent->stamina >= 200)
	ent->client->ps.stats[STAT_STAMINA_ICON] = gi.imageindex ("p_stam");

	ent->client->ps.stats[STAT_STAMINA] = ent->stamina;
/*	if (ent->client->pers.weapon->ammo)
	{
		ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex(FindItem(ent->client->pers.weapon->ammo)->icon);
		ent->client->ps.stats[STAT_AMMO] = ent->left_in_clip;
	}
	else
	{
		ent->client->ps.stats[STAT_AMMO_ICON] = 0;
		ent->client->ps.stats[STAT_AMMO] = 0;
	}*/
	
	ent->client->ps.stats[STAT_SNIPER_OK] = 1;
	
	if(ent->client->ps.gunindex == gi.modelindex(ent->client->pers.weapon->akimbo_model))
	{
	if(ent->client->next_hand == HAND_DEFAULT)
	ent->client->ps.stats[STAT_SNIPERCROSS] = gi.imageindex ("schr");
	else if(ent->client->next_hand == HAND_OTHER)
	ent->client->ps.stats[STAT_SNIPERCROSS] = gi.imageindex ("schl");
	}
	else
	ent->client->ps.stats[STAT_SNIPERCROSS] = gi.imageindex ("blank");
	
	
	if (strcmp(ent->client->pers.weapon->classname, "weapon_sniper") == 0 && ent->client->buttons & BUTTON_ATTACK)
	{
		MatrixSniperHud(ent);
	//	ent->client->ps.stats[STAT_SNIPERCROSS] = gi.imageindex ("schplain");
	}
	//else
	//	ent->client->ps.stats[STAT_SNIPERCROSS] = gi.imageindex ("blank");

	if (ent->client->resp.tank)
		ent->client->ps.stats[STAT_SPECTATOR] = 0;

	if(ent->client->ps.stats[STAT_TIMER_ICON])
		ent->client->ps.stats[STAT_TIMER_ICON] = 0;
	if(ent->client->ps.stats[STAT_TIMER])
		ent->client->ps.stats[STAT_TIMER] = 0;

	if(ent->client->ps.stats[STAT_TIMER2_ICON])
		ent->client->ps.stats[STAT_TIMER2_ICON] = 0;
	if(ent->client->ps.stats[STAT_TIMER2])
		ent->client->ps.stats[STAT_TIMER2] = 0;


	if (ent->someswaptime > level.time)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("posses");
		ent->client->ps.stats[STAT_TIMER] = (int)(ent->someswaptime - level.time);
		timer1taken = true;
	}
	if (ent->client->cloak_framenum > level.framenum)
	{
		if(!timer1taken)
		{
			ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("cloak");
			ent->client->ps.stats[STAT_TIMER] = (ent->client->cloak_framenum - level.framenum)/10;
			timer1taken = true;
		}
		else
		{
			ent->client->ps.stats[STAT_TIMER2_ICON] = gi.imageindex ("cloak");
			ent->client->ps.stats[STAT_TIMER2] = (ent->client->cloak_framenum - level.framenum)/10;
			timer2taken = true;
		}
	}
		if (ent->bullet_framenum > level.framenum && !(timer1taken && timer2taken))
	{
		if(!timer1taken)
		{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("bullstop");
		ent->client->ps.stats[STAT_TIMER] = (ent->bullet_framenum - level.framenum)/10;
		timer1taken = true;
		}
		else if(!timer2taken)
		{
		ent->client->ps.stats[STAT_TIMER2_ICON] = gi.imageindex ("bullstop");
		ent->client->ps.stats[STAT_TIMER2] = (ent->bullet_framenum - level.framenum)/10;
		timer2taken = true;
		}
	}
	if (ent->client->ir_framenum > level.framenum && !(timer1taken && timer2taken))
	{
		if(!timer1taken)
		{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("irvis");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->ir_framenum - level.framenum)/10;
		timer1taken = true;
		}
		else if(!timer2taken)
		{
		ent->client->ps.stats[STAT_TIMER2_ICON] = gi.imageindex ("irvis");
		ent->client->ps.stats[STAT_TIMER2] = (ent->client->ir_framenum - level.framenum)/10;
		timer2taken = true;
		}
		
	}
	if (ent->client->speed_framenum > level.framenum && !(timer1taken && timer2taken))
	{
		if(!timer1taken)
		{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("spedlevl");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->speed_framenum - level.framenum)/10;
		timer1taken = true;
		}
		else if(!timer2taken)
		{
		ent->client->ps.stats[STAT_TIMER2_ICON] = gi.imageindex ("spedlevl");
		ent->client->ps.stats[STAT_TIMER2] = (ent->client->speed_framenum - level.framenum)/10;
		timer2taken = true;
		}
		
	}
	if (matrix.lightsout_framenum > level.framenum && !(timer1taken && timer2taken))
	{
		
		if(!timer1taken)
		{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("irvis");
		ent->client->ps.stats[STAT_TIMER] = (matrix.lightsout_framenum - level.framenum)/10;
		timer1taken = true;
		}
		else if(!timer2taken)
		{
		ent->client->ps.stats[STAT_TIMER2_ICON] = gi.imageindex ("irvis");
		ent->client->ps.stats[STAT_TIMER2] = (matrix.lightsout_framenum - level.framenum)/10;
		timer2taken = true;
		}
		
	}
	
	
	ent->client->ps.stats[STAT_DAMAGELEVEL_ICON] = gi.imageindex ("damlevl");
	ent->client->ps.stats[STAT_DAMAGELEVEL] = ent->damagelevel;

	ent->client->ps.stats[STAT_STAMINALEVEL_ICON] = gi.imageindex ("stamlevl");
	ent->client->ps.stats[STAT_STAMINALEVEL] = ent->staminalevel;

	ent->client->ps.stats[STAT_HEALTHLEVEL_ICON] = gi.imageindex ("heallevl");
	ent->client->ps.stats[STAT_HEALTHLEVEL] = ent->healthlevel;

}

void MatrixBlend (edict_t *ent)
{
	int		remaining;
		// MATRIX
	if (ent->client->ir_framenum > level.framenum)
	{
		remaining = ent->client->ir_framenum - level.framenum;
		if(remaining > 30 || (remaining & 4))
		{
			ent->client->ps.rdflags |= RDF_IRGOGGLES;
			SV_AddBlend (1, 0, 0, 0.2, ent->client->ps.blend);
		}
		else
			ent->client->ps.rdflags &= ~RDF_IRGOGGLES;
	}
	else
	{
		ent->client->ps.rdflags &= ~RDF_IRGOGGLES;
	}
	// MATRIX

}


char *matrix_statusbar =
"yb	-24 "

// health
"xl	0 "
"hnum "
"xl	50 "
"pic 0 "


// ammo
"if 2 "
"	xv	100 "
"	anum "
"	xv	150 "
"	pic 2 "
"endif "


// stamina
//"if 18 "
"	yb	-50	"
"	xl	0 "
"	num	3	18	"
"	xl	50 "
"	pic 19 "
"yb	-24 "
//"endif "

// armor
"if 4 "
"	xv	200 "
"	rnum "
"	xv	250 "
"	pic 4 "
"endif "

// selected item
"if 6 "
"	xv	296 "
"	pic 6 "
"endif "

"yb	-50 "

// picked up item
"if 7 "
"yb	-102 "
"	xl	0 "
"	pic 7 "
"	xl	26 "
"	yb	-94 "
"	stat_string 8 "
"	yb	-50 "
"endif "

// timer
"if 9 "
"	xr	-58 "
"	num	2	10 "
"	xr	-24 "
"	pic	9 "
"endif "

// timer2
"if 22 "
"	yb	-74 "
"	xr	-58 "
"	num	2	22 "
"	xr	-24 "
"	pic	21 "
"	yb	-50 "
"endif "

//  help / weapon icon 
"if 11 "
"	xv	148 "
"	pic	11 "
"endif "

 //sniper
"if 31 "
  "	yv	20	"
  "	xv	60 "
  "	pic 20 "
  "	xl	50 "
  "	xv	100 "
  "yb	-24 "
"endif "

//  frags
"xr	-50 "
"yt 2 "
"num 3 14 "

//LEVELS

// spectator
"if 17 "
  "xv 0 "
  "yb -58 "
  "string2 \"SPECTATOR MODE\" "
"endif "

// chase camera
"if 16 "
  "xv 0 "
  "yb -68 "
  "string \"Chasing\" "
  "xv 64 "
  "stat_string 16 "
"endif "
;

 
