
#include "g_local.h"
#include "m_player.h"



static	edict_t		*current_player;
static	gclient_t	*current_client;

static	vec3_t	forward, right, up;
float	xyspeed;

float	bobmove;
int		bobcycle;		// odd cycles are right foot going forward
float	bobfracsin;		// sin(bobfrac*M_PI)

void PMenuDoUpdate(edict_t *ent)
{
	char string[1400];
	int i;
	pmenu_t *p;
	int x;
	pmenuhnd_t *hnd;
	char *t;
	qboolean alt = false;

	if (!ent->client->menu) {
		gi.dprintf("warning:  ent has no menu\n");
		return;
	}

	hnd = ent->client->menu;

	strcpy(string, "xv 32 yv 8 picn inventory ");

	for (i = 0, p = hnd->entries; i < hnd->num; i++, p++) {
		if (!p->text || !*(p->text))
			continue; // blank line
		t = p->text;
		if (*t == '*') {
			alt = true;
			t++;
		}
		sprintf(string + strlen(string), "yv %d ", 32 + i * 8);
		if (p->align == PMENU_ALIGN_CENTER)
			x = 196/2 - strlen(t)*4 + 64;
		else if (p->align == PMENU_ALIGN_RIGHT)
			x = 64 + (196 - strlen(t)*8);
		else
			x = 64;

		sprintf(string + strlen(string), "xv %d ",
			x - ((hnd->cur == i) ? 8 : 0));

		if (hnd->cur == i)
			sprintf(string + strlen(string), "string2 \"\x0d%s\" ", t);
		else if (alt)
			sprintf(string + strlen(string), "string2 \"%s\" ", t);
		else
			sprintf(string + strlen(string), "string \"%s\" ", t);
		alt = false;
	}

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}
/*
===============
SV_CalcRoll

===============
*/
float SV_CalcRoll (vec3_t angles, vec3_t velocity)
{
	float	sign;
	float	side;
	float	value;
	
	side = DotProduct (velocity, right);
	sign = side < 0 ? -1 : 1;
	side = fabs(side);
	
	value = sv_rollangle->value;

	if (side < sv_rollspeed->value)
		side = side * value / sv_rollspeed->value;
	else
		side = value;
	
	return side*sign;
	
}


/*
===============
P_DamageFeedback

Handles color blends and view kicks
===============
*/
void ClientUserinfoChanged (edict_t *ent, char *userinfo);
void P_DamageFeedback (edict_t *player)
{
	gclient_t	*client;
	float	side;
	float	realcount, count, kick;
	vec3_t	v;
	int		r;
	static	vec3_t	power_color = {0.0, 1.0, 0.0};
	static	vec3_t	acolor = {1.0, 1.0, 1.0};
	static	vec3_t	bcolor = {1.0, 0.0, 0.0};

	client = player->client;

	if (player->health < 1)
	{
		char		userinfo[MAX_INFO_STRING]; // Max String Size

		memcpy (userinfo, client->pers.userinfo, sizeof(userinfo)); // Copy it
		ClientUserinfoChanged (player, userinfo); // Send new info
	}

	// flash the backgrounds behind the status numbers
	client->ps.stats[STAT_FLASHES] = 0;
	if (client->damage_blood)
		client->ps.stats[STAT_FLASHES] |= 1;
	if (client->damage_armor && !(player->flags & FL_GODMODE) && (client->invincible_framenum <= level.framenum))
		client->ps.stats[STAT_FLASHES] |= 2;

	// total points of damage shot at the player this frame
	count = (client->damage_blood + client->damage_armor + client->damage_parmor);
	if (count == 0)
		return;		// didn't take any damage

	// start a pain animation if still in the player model
	/*if (client->anim_priority < ANIM_PAIN && player->s.modelindex == 255)
	{
		static int		i;

		client->anim_priority = ANIM_PAIN;
		if (client->ps.pmove.pm_flags & PMF_DUCKED)
		{
			player->s.frame = FRAME_crpain1-1;
			client->anim_end = FRAME_crpain4;
		}
		else
		{
			i = (i+1)%3;
			switch (i)
			{
			case 0:
				player->s.frame = FRAME_pain101-1;
				client->anim_end = FRAME_pain104;
				break;
			case 1:
				player->s.frame = FRAME_pain201-1;
				client->anim_end = FRAME_pain204;
				break;
			case 2:
				player->s.frame = FRAME_pain301-1;
				client->anim_end = FRAME_pain304;
				break;
			}
		}
	}*/

	realcount = count;
	if (count < 10)
		count = 10;	// always make a visible effect
if (client->anim_priority < ANIM_PAIN && player->s.modelindex == 255)
	{
	// play an apropriate pain sound
	if ((level.time > player->pain_debounce_time) && !(player->flags & FL_GODMODE) && (client->invincible_framenum <= level.framenum))
	{
		if (player->health < 1)
			return;
		player->pain_debounce_time = level.time + 8.0;

		if (player->client->resp.class == 1)
		{
			player->s.frame = 80;
			player->client->anim_end = 96;
			if (player->s.frame == 82)
				gi.sound(player, 0, gi.soundindex ("world/land.wav"), 1, ATTN_NORM, 0);
			r = rndnum (1, 5);
			gi.sound (player, CHAN_VOICE, gi.soundindex(va("soldier/solpain%i.wav", r)), 1, ATTN_NORM, 0);
		}
		else if (player->client->resp.class == 36)
		{
			player->s.frame = 80;
			player->client->anim_end = 96;
			if (player->s.frame == 82)
				gi.sound(player, 0, gi.soundindex ("world/land.wav"), 1, ATTN_NORM, 0);
			r = rndnum (1, 5);
			gi.sound (player, CHAN_VOICE, gi.soundindex(va("soldier/solpain%i.wav", r)), 1, ATTN_NORM, 0);
		}
		else if (player->client->resp.class == 20)
		{
			player->s.frame = 80;
			player->client->anim_end = 96;
			if (player->s.frame == 82)
				gi.sound(player, 0, gi.soundindex ("world/land.wav"), 1, ATTN_NORM, 0);
			r = rndnum (1, 5);
			gi.sound (player, CHAN_VOICE, gi.soundindex(va("soldier/solpain%i.wav", r)), 1, ATTN_NORM, 0);
		}
		else if (player->client->resp.class == 2)
		{
			player->s.frame = 110;
			player->client->anim_end = 119;
			r = rndnum (1, 4);
			gi.sound (player, CHAN_VOICE, gi.soundindex(va("infantry/infpain%i.wav", r)), 1, ATTN_NORM, 0);
		}
		else if (player->client->resp.class == 3)
		{
			player->s.frame = 200;
			player->client->anim_end = 205;
			gi.sound (player, CHAN_VOICE, gi.soundindex("tank/tnkpain2.wav"), 1, ATTN_NORM, 0);
		}
		else if (player->client->resp.class == 33)
		{
			player->s.frame = 200;
			player->client->anim_end = 205;
			gi.sound (player, CHAN_VOICE, gi.soundindex("tank/tnkpain2.wav"), 1, ATTN_NORM, 0);
		}
		else if (player->client->resp.class == 4)
		{
			player->s.frame = 108;
			player->client->anim_end = 115;
			r = rndnum (1, 4);
			gi.sound (player, CHAN_VOICE, gi.soundindex(va("medic/medpain%i.wav", r)), 1, ATTN_NORM, 0);
		}
		else if (player->client->resp.class == 28)
		{
			player->s.frame = 108;
			player->client->anim_end = 115;
			r = rndnum (1, 3);
			gi.sound (player, CHAN_VOICE, gi.soundindex(va("medic/medpain%i.wav", r)), 1, ATTN_NORM, 0);
		}
		else if (player->client->resp.class == 5)
		{
			player->s.frame = 177;
			player->client->anim_end = 184;
			r = rndnum (1, 4);
			gi.sound (player, CHAN_VOICE, gi.soundindex(va("gunner/gunpain%i.wav", r)), 1, ATTN_NORM, 0);
		}
		else if (player->client->resp.class == 6)
		{
			player->s.frame = 55;
			player->client->anim_end = 60;
			gi.sound (player, CHAN_VOICE, gi.soundindex("gladiator/pain.wav"), 1, ATTN_NORM, 0);
		}
		else if (player->client->resp.class == 34)
		{
			player->s.frame = 55;
			player->client->anim_end = 60;
			gi.sound (player, CHAN_VOICE, gi.soundindex("gladiator/pain.wav"), 1, ATTN_NORM, 0);
		}
		else if (player->client->resp.class == 21)
		{
			player->s.frame = 55;
			player->client->anim_end = 60;
			gi.sound (player, CHAN_VOICE, gi.soundindex("gladiator/pain.wav"), 1, ATTN_NORM, 0);
		}
		else if (player->client->resp.class == 7)
		{
			player->s.frame = 171;
			player->client->anim_end = 175;
			r = rndnum (1, 5);
			gi.sound (player, CHAN_VOICE, gi.soundindex(va("bosstank/btkpain%i.wav", r)), 1, ATTN_NORM, 0);
		}
		else if (player->client->resp.class == 39)
		{
			player->s.frame = 171;
			player->client->anim_end = 175;
			r = rndnum (1, 5);
			gi.sound (player, CHAN_VOICE, gi.soundindex(va("bosstank/btkpain%i.wav", r)), 1, ATTN_NORM, 0);
		}
		else if (player->client->resp.class == 24)
		{
			player->s.frame = 171;
			player->client->anim_end = 175;
			r = rndnum (1, 5);
			gi.sound (player, CHAN_VOICE, gi.soundindex(va("bosstank/btkpain%i.wav", r)), 1, ATTN_NORM, 0);
		}
		else if (player->client->resp.class == 8)
		{
			player->s.frame = 134;
			player->client->anim_end = 142;
			r = rndnum (1, 4);
			gi.sound (player, CHAN_VOICE, gi.soundindex(va("flyer/flypain%i.wav", r)), 1, ATTN_NORM, 0);
		}
		else if (player->client->resp.class == 35)
		{
			player->s.frame = 134;
			player->client->anim_end = 142;
			r = rndnum (1, 4);
			gi.sound (player, CHAN_VOICE, gi.soundindex(va("flyer/flypain%i.wav", r)), 1, ATTN_NORM, 0);
		}
		else if (player->client->resp.class == 32)
		{
			player->s.frame = 140;
			player->client->anim_end = 145;
			r = rndnum (1, 4);
			gi.sound (player, CHAN_VOICE, gi.soundindex(va("flyer/flypain%i.wav", r)), 1, ATTN_NORM, 0);
		}
		else if (player->client->resp.class == 16)
		{
			player->s.frame = 117;
			player->client->anim_end = 123;
			r = rndnum (1, 3);
			gi.sound (player, CHAN_VOICE, gi.soundindex(va("floater/fltpain%i.wav", r)), 1, ATTN_NORM, 0);
		}
		else if (player->client->resp.class == 17)
		{
			player->s.frame = 94;
			player->client->anim_end = 98;
			r = rndnum (1, 3);
			gi.sound (player, CHAN_VOICE, gi.soundindex(va("flipper/flppain%i.wav", r)), 1, ATTN_NORM, 0);
		}
		else if (player->client->resp.class == 9)
		{
			player->s.frame = 199;
			player->client->anim_end = 222;
			gi.sound (player, CHAN_VOICE, gi.soundindex("berserk/berpain2.wav"), 1, ATTN_NORM, 0);
		}
		else if (player->client->resp.class == 10)
		{
			player->s.frame = 90;
			player->client->anim_end = 94;
			r = rndnum (1, 4);
			gi.sound (player, CHAN_VOICE, gi.soundindex(va("chick/chkpain%i.wav", r)), 1, ATTN_NORM, 0);
		}
		else if (player->client->resp.class == 23)
		{
			player->s.frame = 90;
			player->client->anim_end = 94;
			r = rndnum (1, 4);
			gi.sound (player, CHAN_VOICE, gi.soundindex(va("chick/chkpain%i.wav", r)), 1, ATTN_NORM, 0);
		}
		else if (player->client->resp.class == 11)
		{
			player->s.frame = 110;
			player->client->anim_end = 131;
			r = rndnum (1, 4);
			gi.sound (player, CHAN_VOICE, gi.soundindex(va("bosshovr/BHVPAIN%i.wav", r)), 1, ATTN_NORM, 0);
		}
		else if (player->client->resp.class == 29)
		{
			gi.sound (player, CHAN_VOICE, gi.soundindex("carrier/pain_md.wav"), 1, ATTN_NORM, 0);
		}
		else if (player->client->resp.class == 12)
		{
			player->s.frame = 117;
			player->client->anim_end = 112;
			r = rndnum (1, 3);
			gi.sound (player, CHAN_VOICE, gi.soundindex(va("brain/brnpain%i.wav", r)), 1, ATTN_NORM, 0);
		}
		else if (player->client->resp.class == 22)
		{
			player->s.frame = 117;
			player->client->anim_end = 112;
			r = rndnum (1, 3);
			gi.sound (player, CHAN_VOICE, gi.soundindex(va("brain/brnpain%i.wav", r)), 1, ATTN_NORM, 0);
		}
		else if (player->client->resp.class == 13)
		{
			player->s.frame = 34;
			player->client->anim_end = 38;
			r = rndnum (1, 4);
			gi.sound (player, CHAN_VOICE, gi.soundindex(va("mutant/mutpain%i.wav", r)), 1, ATTN_NORM, 0);
		}
		else if (player->client->resp.class == 14)
		{
/*			player->s.frame = 57;
			player->client->anim_end = 67;
			r = rndnum (1, 3);
			gi.sound (player, CHAN_VOICE, gi.soundindex(va("parasite/parpain%i.wav", r)), 1, ATTN_NORM, 0);
		*/	player->s.frame = 200;
			player->client->anim_end = 205;
			gi.sound (player, CHAN_VOICE, gi.soundindex("tank/tnkpain2.wav"), 1, ATTN_NORM, 0);

		}
		else if (player->client->resp.class == 41)
		{
/*			player->s.frame = 57;
			player->client->anim_end = 67;
			r = rndnum (1, 3);
			gi.sound (player, CHAN_VOICE, gi.soundindex(va("parasite/parpain%i.wav", r)), 1, ATTN_NORM, 0);
		*/	player->s.frame = 200;
			player->client->anim_end = 205;
			gi.sound (player, CHAN_VOICE, gi.soundindex("tank/tnkpain2.wav"), 1, ATTN_NORM, 0);

		}
		else if (player->client->resp.class == 40)
		{
			player->s.frame = 57;
			player->client->anim_end = 67;
			r = rndnum (1, 3);
			gi.sound (player, CHAN_VOICE, gi.soundindex(va("parasite/parpain%i.wav", r)), 1, ATTN_NORM, 0);
		}
		else if (player->client->resp.class == 18)
		{
			player->s.frame = 87;
			player->client->anim_end = 111;
			r = rndnum (1, 4);
			gi.sound (player, CHAN_VOICE, gi.soundindex(va("boss3/bs3pain%i.wav", r)), 1, ATTN_NORM, 0);
		}
		else if (player->client->resp.class == 19)
		{
			player->s.frame = 183;
			player->client->anim_end = 185;
			r = rndnum (1, 4);
			gi.sound (player, CHAN_VOICE, gi.soundindex(va("makron/pain%i.wav", r)), 1, ATTN_NORM, 0);
		}
		else if (player->client->resp.class == 30)
		{
			player->s.frame = 113;
			player->client->anim_end = 115;
			r = rndnum (1, 4);
			gi.sound (player, CHAN_VOICE, gi.soundindex(va("widow/bw1pain%i.wav", r)), 1, ATTN_NORM, 0);
		}
		else if (player->client->resp.class == 31)
		{
			player->s.frame = 55;
			player->client->anim_end = 59;
			r = rndnum (1, 4);
			gi.sound (player, CHAN_VOICE, gi.soundindex(va("widow/bw2pain%i.wav", r)), 1, ATTN_NORM, 0);
		}
		else if (player->client->resp.class == 15)
		{
			player->s.frame = 153;
			player->client->anim_end = 161;
			r = rndnum (1, 4);
			gi.sound (player, CHAN_VOICE, gi.soundindex(va("hover/hovpain%i.wav", r)), 1, ATTN_NORM, 0);
		}
		else if (player->client->resp.class == 26)
		{
			player->s.frame = 153;
			player->client->anim_end = 161;
			r = rndnum (1, 3);
			gi.sound (player, CHAN_VOICE, gi.soundindex(va("daedalus/daedpain%i.wav", r)), 1, ATTN_NORM, 0);
		}
		else if (player->client->resp.class == 25)
		{
			player->s.frame = 90;
			player->client->anim_end = 100;
			r = rndnum (1, 3);
			gi.sound (player, CHAN_VOICE, gi.soundindex(va("gek/gk_pain%i.wav", r)), 1, ATTN_NORM, 0);
		}
		else if (player->client->resp.class == 27)
		{
			player->s.frame = 66;
			player->client->anim_end = 69;
			r = rndnum (1, 2);
			gi.sound (player, CHAN_VOICE, gi.soundindex("stalker/pain.wav"), 1, ATTN_NORM, 0);
		}
		else if (player->client->resp.class == 37)
		{
			int ghg = rndnum (0, 2);
			switch (ghg)
			{
			case 0:
				player->s.frame = FRAME_pain101-1;
				client->anim_end = FRAME_pain104;
				break;
			case 1:
				player->s.frame = FRAME_pain201-1;
				client->anim_end = FRAME_pain204;
				break;
			case 2:
				player->s.frame = FRAME_pain301-1;
				client->anim_end = FRAME_pain304;
				break;
			}
		}
		else if (player->client->resp.class == 38)
		{
			int ghg2 = rndnum (0, 2);
			switch (ghg2)
			{
			case 0:
				player->s.frame = FRAME_pain101-1;
				client->anim_end = FRAME_pain104;
				break;
			case 1:
				player->s.frame = FRAME_pain201-1;
				client->anim_end = FRAME_pain204;
				break;
			case 2:
				player->s.frame = FRAME_pain301-1;
				client->anim_end = FRAME_pain304;
				break;
			}
			
			r = rndnum (1, 2);
			gi.sound (player, CHAN_VOICE, gi.soundindex(va("../players/female/pain100_%i.wav", r)), 1, ATTN_NORM, 0);
		}
	}
	}
	// the total alpha of the blend is always proportional to count
	if (client->damage_alpha < 0)
		client->damage_alpha = 0;
	client->damage_alpha += count*0.01;
	if (client->damage_alpha < 0.2)
		client->damage_alpha = 0.2;
	if (client->damage_alpha > 0.6)
		client->damage_alpha = 0.6;		// don't go too saturated

	// the color of the blend will vary based on how much was absorbed
	// by different armors
	VectorClear (v);
	if (client->damage_parmor)
		VectorMA (v, (float)client->damage_parmor/realcount, power_color, v);
	if (client->damage_armor)
		VectorMA (v, (float)client->damage_armor/realcount,  acolor, v);
	if (client->damage_blood)
		VectorMA (v, (float)client->damage_blood/realcount,  bcolor, v);
	VectorCopy (v, client->damage_blend);


	//
	// calculate view angle kicks
	//
	kick = abs(client->damage_knockback);
	if (kick && player->health > 0)	// kick of 0 means no view adjust at all
	{
		kick = kick * 100 / player->health;

		if (kick < count*0.5)
			kick = count*0.5;
		if (kick > 50)
			kick = 50;

		VectorSubtract (client->damage_from, player->s.origin, v);
		VectorNormalize (v);
		
		side = DotProduct (v, right);
		client->v_dmg_roll = kick*side*0.3;
		
		side = -DotProduct (v, forward);
		client->v_dmg_pitch = kick*side*0.3;

		client->v_dmg_time = level.time + DAMAGE_TIME;
	}

	//
	// clear totals
	//
	client->damage_blood = 0;
	client->damage_armor = 0;
	client->damage_parmor = 0;
	client->damage_knockback = 0;

	// Paril
	// Lastly, change into pain skin if needed
	// Might need a better way...
	if (player->health < (player->max_health / 2))
	{
		char		userinfo[MAX_INFO_STRING]; // Max String Size

		if (player->client->pers.skin_pain)
			return;

		player->client->pers.skin_pain = 1;
		memcpy (userinfo, client->pers.userinfo, sizeof(userinfo)); // Copy it
		ClientUserinfoChanged (player, userinfo); // Send new info
	}
}




/*
===============
SV_CalcViewOffset

Auto pitching on slopes?

  fall from 128: 400 = 160000
  fall from 256: 580 = 336400
  fall from 384: 720 = 518400
  fall from 512: 800 = 640000
  fall from 640: 960 = 

  damage = deltavelocity*deltavelocity  * 0.0001

===============
*/
void SV_CalcViewOffset (edict_t *ent)
{
	float		*angles;
	float		bob;
	float		ratio;
	float		delta;
	vec3_t		v;


//===================================

	// base angles
	angles = ent->client->ps.kick_angles;

	// if dead, fix the angle and don't add any kick
	if (ent->deadflag)
	{
		VectorClear (angles);

		ent->client->ps.viewangles[ROLL] = 40;
		ent->client->ps.viewangles[PITCH] = -15;
		ent->client->ps.viewangles[YAW] = ent->client->killer_yaw;
	}
	else
	{
		// add angles based on weapon kick

		VectorCopy (ent->client->kick_angles, angles);

		// add angles based on damage kick

		ratio = (ent->client->v_dmg_time - level.time) / DAMAGE_TIME;
		if (ratio < 0)
		{
			ratio = 0;
			ent->client->v_dmg_pitch = 0;
			ent->client->v_dmg_roll = 0;
		}
		angles[PITCH] += ratio * ent->client->v_dmg_pitch;
		angles[ROLL] += ratio * ent->client->v_dmg_roll;

		// add pitch based on fall kick

		ratio = (ent->client->fall_time - level.time) / FALL_TIME;
		if (ratio < 0)
			ratio = 0;
		angles[PITCH] += ratio * ent->client->fall_value;

		// add angles based on velocity

		delta = DotProduct (ent->velocity, forward);
		angles[PITCH] += delta*run_pitch->value;
		
		delta = DotProduct (ent->velocity, right);
		angles[ROLL] += delta*run_roll->value;

		// add angles based on bob

		delta = bobfracsin * bob_pitch->value * xyspeed;
//		if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
//			delta *= 6;		// crouching
		angles[PITCH] += delta;
		delta = bobfracsin * bob_roll->value * xyspeed;
//		if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
//			delta *= 6;		// crouching
		if (bobcycle & 1)
			delta = -delta;
		angles[ROLL] += delta;
	}

//===================================

	// base origin

	VectorClear (v);

	// add view height

	v[2] += ent->viewheight;

	// add fall height

	ratio = (ent->client->fall_time - level.time) / FALL_TIME;
	if (ratio < 0)
		ratio = 0;
	v[2] -= ratio * ent->client->fall_value * 0.4;

	// add bob height

	bob = bobfracsin * xyspeed * bob_up->value;
	if (bob > 6)
		bob = 6;
	//gi.DebugGraph (bob *2, 255);
	v[2] += bob;

	// add kick offset

	VectorAdd (v, ent->client->kick_origin, v);

	// absolutely bound offsets
	// so the view can never be outside the player box

        //Wyrm: chasecam "change view" function
        if (!ent->client->chasetoggle)
        {                                              
                if (v[0] < -14)
                        v[0] = -14;
                else if (v[0] > 14)
                        v[0] = 14;
                if (v[1] < -14)
                        v[1] = -14;
                else if (v[1] > 14)
                        v[1] = 14;
                if (v[2] < -22)
                        v[2] = -22;
                else if (v[2] > 30)
                        v[2] = 30;
        }
        else
        {
                VectorSet(v, 0, 0, 0);
               // else
                if (ent->client->chasecam != NULL)
                {
//                        VectorScale(ent->client->chasecam->s.origin, 8, ent->client->ps.pmove.origin);
                        ent->client->ps.pmove.origin[0] = ent->client->chasecam->s.origin[0]*8;
                        ent->client->ps.pmove.origin[1] = ent->client->chasecam->s.origin[1]*8;
                        ent->client->ps.pmove.origin[2] = ent->client->chasecam->s.origin[2]*8;
                        VectorCopy(ent->client->chasecam->s.angles, ent->client->ps.viewangles);
                }

                 
        }

	VectorCopy (v, ent->client->ps.viewoffset);
}

/*
==============
SV_CalcGunOffset
==============
*/
void SV_CalcGunOffset (edict_t *ent)
{
	int		i;
	float	delta;

	// gun angles from bobbing
	ent->client->ps.gunangles[ROLL] = xyspeed * bobfracsin * 0.005;
	ent->client->ps.gunangles[YAW] = xyspeed * bobfracsin * 0.01;
	if (bobcycle & 1)
	{
		ent->client->ps.gunangles[ROLL] = -ent->client->ps.gunangles[ROLL];
		ent->client->ps.gunangles[YAW] = -ent->client->ps.gunangles[YAW];
	}

	ent->client->ps.gunangles[PITCH] = xyspeed * bobfracsin * 0.005;

	// gun angles from delta movement
	for (i=0 ; i<3 ; i++)
	{
		delta = ent->client->oldviewangles[i] - ent->client->ps.viewangles[i];
		if (delta > 180)
			delta -= 360;
		if (delta < -180)
			delta += 360;
		if (delta > 45)
			delta = 45;
		if (delta < -45)
			delta = -45;
		if (i == YAW)
			ent->client->ps.gunangles[ROLL] += 0.1*delta;
		ent->client->ps.gunangles[i] += 0.2 * delta;
	}

	// gun height
	VectorClear (ent->client->ps.gunoffset);
//	ent->ps->gunorigin[2] += bob;

	// gun_x / gun_y / gun_z are development tools
	for (i=0 ; i<3 ; i++)
	{
		ent->client->ps.gunoffset[i] += forward[i]*(gun_y->value);
		ent->client->ps.gunoffset[i] += right[i]*gun_x->value;
		ent->client->ps.gunoffset[i] += up[i]* (-gun_z->value);
	}
}


/*
=============
SV_AddBlend
=============
*/
void SV_AddBlend (float r, float g, float b, float a, float *v_blend)
{
	float	a2, a3;

	if (a <= 0)
		return;
	a2 = v_blend[3] + (1-v_blend[3])*a;	// new total alpha
	a3 = v_blend[3]/a2;		// fraction of color from old

	v_blend[0] = v_blend[0]*a3 + r*(1-a3);
	v_blend[1] = v_blend[1]*a3 + g*(1-a3);
	v_blend[2] = v_blend[2]*a3 + b*(1-a3);
	v_blend[3] = a2;
}


/*
=============
SV_CalcBlend
=============
*/
void SV_CalcBlend (edict_t *ent)
{
	int		contents;
	vec3_t	vieworg;
	int		remaining;

	ent->client->ps.blend[0] = ent->client->ps.blend[1] = 
		ent->client->ps.blend[2] = ent->client->ps.blend[3] = 0;

	// add for contents
	VectorAdd (ent->s.origin, ent->client->ps.viewoffset, vieworg);
	contents = gi.pointcontents (vieworg);
	if (contents & (CONTENTS_LAVA|CONTENTS_SLIME|CONTENTS_WATER) )
	{
		if (ent->client->resp.class != 17)
			ent->client->ps.rdflags |= RDF_UNDERWATER;
	}
	else
		ent->client->ps.rdflags &= ~RDF_UNDERWATER;

	if (contents & (CONTENTS_SOLID|CONTENTS_LAVA))
		SV_AddBlend (1.0, 0.3, 0.0, 0.6, ent->client->ps.blend);
	else if (contents & CONTENTS_SLIME)
		SV_AddBlend (0.0, 0.1, 0.05, 0.6, ent->client->ps.blend);
	else if (contents & CONTENTS_WATER)
	{
		if (ent->client->resp.class != 17)
			SV_AddBlend (0.5, 0.3, 0.2, 0.4, ent->client->ps.blend);
	}

        // JDB: lowlight vision effect 4/4/98
        if (ent->client->pers.abilities.mutant_nightvision)
                SV_AddBlend (0.0, 0.7, 0.0, 0.5, ent->client->ps.blend);


	// add for powerups
	if (ent->client->quad_framenum > level.framenum)
	{
		remaining = ent->client->quad_framenum - level.framenum;
		if (remaining == 30)	// beginning to fade
			gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage2.wav"), 1, ATTN_NORM, 0);
		if (remaining > 30 || (remaining & 4) )
			SV_AddBlend (0, 0, 1, 0.08, ent->client->ps.blend);
	}
	/* Wonderslug   make screen go colored  */
	else if (ent->client->kamikaze_framenum > level.framenum){
		remaining = ent->client->kamikaze_framenum - level.framenum;
		ent->client->kamikaze_timeleft = remaining;
		if (remaining == 30)
			gi.sound(ent, CHAN_ITEM, gi.soundindex("items/protect2.wav"), 1, ATTN_NORM, 0);
		if (remaining > 30 || (remaining & 4))
			SV_AddBlend (0, 0, 1, 0.08, ent->client->ps.blend);
	}
	/* Wonderslug End */
	else if (ent->client->invincible_framenum > level.framenum)
	{
		remaining = ent->client->invincible_framenum - level.framenum;
		if (remaining == 30)	// beginning to fade
			gi.sound(ent, CHAN_ITEM, gi.soundindex("items/protect2.wav"), 1, ATTN_NORM, 0);
		if (remaining > 30 || (remaining & 4) )
			SV_AddBlend (1, 1, 0, 0.08, ent->client->ps.blend);
	}
	else if (ent->client->enviro_framenum > level.framenum)
	{
		remaining = ent->client->enviro_framenum - level.framenum;
		if (remaining == 30)	// beginning to fade
			gi.sound(ent, CHAN_ITEM, gi.soundindex("items/airout.wav"), 1, ATTN_NORM, 0);
		if (remaining > 30 || (remaining & 4) )
			SV_AddBlend (0, 1, 0, 0.08, ent->client->ps.blend);
	}
	else if (ent->client->breather_framenum > level.framenum)
	{
		remaining = ent->client->breather_framenum - level.framenum;
		if (remaining == 30)	// beginning to fade
			gi.sound(ent, CHAN_ITEM, gi.soundindex("items/airout.wav"), 1, ATTN_NORM, 0);
		if (remaining > 30 || (remaining & 4) )
			SV_AddBlend (0.4, 1, 0.4, 0.04, ent->client->ps.blend);
	}

	// add for damage
	if (ent->client->damage_alpha > 0)
		SV_AddBlend (ent->client->damage_blend[0],ent->client->damage_blend[1]
		,ent->client->damage_blend[2], ent->client->damage_alpha, ent->client->ps.blend);

	if (ent->client->bonus_alpha > 0)
		SV_AddBlend (0.85, 0.7, 0.3, ent->client->bonus_alpha, ent->client->ps.blend);

	// drop the damage value
	ent->client->damage_alpha -= 0.06;
	if (ent->client->damage_alpha < 0)
		ent->client->damage_alpha = 0;

	// drop the bonus value
	ent->client->bonus_alpha -= 0.1;
	if (ent->client->bonus_alpha < 0)
		ent->client->bonus_alpha = 0;
}


/*
=================
P_FallingDamage
=================

Monsters have no falling damage!!!
Edit: A cvar now!
*/
void P_FallingDamage (edict_t *ent)
{
	float	delta;
	int		damage;
	vec3_t	dir;

	if (ent->s.modelindex != 255)
		return;		// not in the player model

	if (ent->movetype == MOVETYPE_NOCLIP)
		return;

	if ((ent->client->oldvelocity[2] < 0) && (ent->velocity[2] > ent->client->oldvelocity[2]) && (!ent->groundentity))
	{
		delta = ent->client->oldvelocity[2];
	}
	else
	{
		if (!ent->groundentity)
			return;
		delta = ent->velocity[2] - ent->client->oldvelocity[2];
	}
	delta = delta*delta * 0.0001;

//ZOID
	// never take damage if just release grapple or on grapple
	if (level.time - ent->client->ctf_grapplereleasetime <= FRAMETIME * 2 ||
		(ent->client->ctf_grapple && 
		ent->client->ctf_grapplestate > CTF_GRAPPLE_STATE_FLY))
		return;
//ZOID

	// never take falling damage if completely underwater
	if (ent->waterlevel == 3)
		return;
	if (ent->waterlevel == 2)
		delta *= 0.25;
	if (ent->waterlevel == 1)
		delta *= 0.5;

	if (delta < 1)
		return;

	if (delta < 15)
	{
		//ent->s.event = EV_FOOTSTEP;
		//return;
	}

	ent->client->fall_value = delta*0.5;
	if (ent->client->fall_value > 40)
		ent->client->fall_value = 40;
	ent->client->fall_time = level.time + FALL_TIME;

	if (delta > 15)
	{
		if (ent->health > 0)
		{
			if (delta >= 55)
				damage = 33;

			else
				damage = 0;
		}
		ent->pain_debounce_time = level.time;	// no normal pain sound
		if (fallingdamage->value)
			T_Damage (ent, world, world, dir, ent->s.origin, vec3_origin, damage, 0, 0, MOD_FALLING);
		gi.sound(ent, 0, gi.soundindex ("world/land.wav"), 1, ATTN_NORM, 0);
		if (fallingdamage->value)
			VectorSet (dir, 0, 0, 1);
	}
	else
	{
		//gi.sound(ent, 0, gi.soundindex ("world/land.wav"), 1, ATTN_NORM, 0);
		//ent->s.event = EV_FALLSHORT;
		return;
	}
}



/*
=============
P_WorldEffects
=============
*/
void P_WorldEffects (void)
{
	qboolean	breather;
	qboolean	envirosuit;
	int			waterlevel, old_waterlevel;

	if (current_player->movetype == MOVETYPE_NOCLIP)
	{
		current_player->air_finished = level.time + 12;	// don't need air
		return;
	}

	//Give air time if frozen 4/99 Acrid
	if (current_player->frozen)
		current_player->air_finished = level.time + 6;

	waterlevel = current_player->waterlevel;
	old_waterlevel = current_client->old_waterlevel;
	current_client->old_waterlevel = waterlevel;

	breather = current_client->breather_framenum > level.framenum;
	envirosuit = current_client->enviro_framenum > level.framenum;

		//
		// Class Specific -
		// Check for.. out of.. water?
		// Barracuda shark cannot stay on land for long!
		//
		if (current_player->client->resp.class == 17)
		{
			if (current_player->health >= 1)
			{
				if (current_player->waterlevel <= 1)
				{
//					if (ent->groundentity)
//					{
						if (current_player->client->resp.nextdrown < level.time)
						{
							T_Damage (current_player, world, world, vec3_origin, current_player->s.origin, vec3_origin, 5, 0, DAMAGE_NO_ARMOR, MOD_WATER);
							current_player->client->resp.nextdrown = level.time + 2;
						}
//					}
				}
			}
		}

	//
	// if just entered a water volume, play a sound
	//
	if (!old_waterlevel && waterlevel)
	{
		PlayerNoise(current_player, current_player->s.origin, PNOISE_SELF);
		if (current_player->watertype & CONTENTS_LAVA)
			gi.sound (current_player, CHAN_BODY, gi.soundindex("player/lava_in.wav"), 1, ATTN_NORM, 0);
		else if (current_player->watertype & CONTENTS_SLIME)
			gi.sound (current_player, CHAN_BODY, gi.soundindex("player/watr_in.wav"), 1, ATTN_NORM, 0);
		else if (current_player->watertype & CONTENTS_WATER)
			gi.sound (current_player, CHAN_BODY, gi.soundindex("player/watr_in.wav"), 1, ATTN_NORM, 0);
		current_player->flags |= FL_INWATER;

		// clear damage_debounce, so the pain sound will play immediately
		current_player->damage_debounce_time = level.time - 1;
	}

	//
	// if just completely exited a water volume, play a sound
	//
	if (old_waterlevel && ! waterlevel)
	{
		PlayerNoise(current_player, current_player->s.origin, PNOISE_SELF);
		gi.sound (current_player, CHAN_BODY, gi.soundindex("player/watr_out.wav"), 1, ATTN_NORM, 0);
		current_player->flags &= ~FL_INWATER;
	}

	//
	// check for head just going under water
	//
	if (old_waterlevel != 3 && waterlevel == 3)
	{
		gi.sound (current_player, CHAN_BODY, gi.soundindex("player/watr_un.wav"), 1, ATTN_NORM, 0);
	}

	//
	// check for head just coming out of water
	//
	if (old_waterlevel == 3 && waterlevel != 3)
	{
		if (current_player->air_finished < level.time)
		{	// gasp for air
			//gi.sound (current_player, CHAN_VOICE, gi.soundindex("player/gasp1.wav"), 1, ATTN_NORM, 0);
			PlayerNoise(current_player, current_player->s.origin, PNOISE_SELF);
		}
		else  if (current_player->air_finished < level.time + 11)
		{	// just break surface
			//gi.sound (current_player, CHAN_VOICE, gi.soundindex("player/gasp2.wav"), 1, ATTN_NORM, 0);
		}
	}

	//
	// check for drowning
	//
	if (current_player->client->resp.class != 17)
	{
	if (waterlevel == 3)
	{
		// breather or envirosuit give air
		if (breather || envirosuit)
		{
			current_player->air_finished = level.time + 10;

			if (((int)(current_client->breather_framenum - level.framenum) % 25) == 0)
			{
				//if (!current_client->breather_sound)
					//gi.sound (current_player, CHAN_AUTO, gi.soundindex("player/u_breath1.wav"), 1, ATTN_NORM, 0);
				//else
					//gi.sound (current_player, CHAN_AUTO, gi.soundindex("player/u_breath2.wav"), 1, ATTN_NORM, 0);
				current_client->breather_sound ^= 1;
				PlayerNoise(current_player, current_player->s.origin, PNOISE_SELF);
				//FIXME: release a bubble?
			}
		}

		// if out of air, start drowning
		if (current_player->air_finished < level.time)
		{	// drown!
			if (current_player->client->next_drown_time < level.time 
				&& current_player->health > 0)
			{
				current_player->client->next_drown_time = level.time + 1;

				// take more damage the longer underwater
				current_player->dmg += 2;
				if (current_player->dmg > 15)
					current_player->dmg = 15;

				// play a gurp sound instead of a normal pain sound
				/*if (current_player->health <= current_player->dmg)
					gi.sound (current_player, CHAN_VOICE, gi.soundindex("player/drown1.wav"), 1, ATTN_NORM, 0);
				else if (rand()&1)
					gi.sound (current_player, CHAN_VOICE, gi.soundindex("*gurp1.wav"), 1, ATTN_NORM, 0);
				else
					gi.sound (current_player, CHAN_VOICE, gi.soundindex("*gurp2.wav"), 1, ATTN_NORM, 0);*/

				current_player->pain_debounce_time = level.time;

				T_Damage (current_player, world, world, vec3_origin, current_player->s.origin, vec3_origin, current_player->dmg, 0, DAMAGE_NO_ARMOR, MOD_WATER);
			}
		}
	}
	else
	{
		current_player->air_finished = level.time + 12;
		current_player->dmg = 2;
	}
	}

	//
	// check for sizzle damage
	//
	if (waterlevel && (current_player->watertype&(CONTENTS_LAVA|CONTENTS_SLIME)) )
	{
		if (current_player->watertype & CONTENTS_LAVA)
		{
			if (current_player->health > 0
				&& current_player->pain_debounce_time <= level.time
				&& current_client->invincible_framenum < level.framenum)
			{
				/*if (rand()&1)
					gi.sound (current_player, CHAN_VOICE, gi.soundindex("player/burn1.wav"), 1, ATTN_NORM, 0);
				else
					gi.sound (current_player, CHAN_VOICE, gi.soundindex("player/burn2.wav"), 1, ATTN_NORM, 0);
				*/current_player->pain_debounce_time = level.time + 1;
			}

			if (envirosuit)	// take 1/3 damage with envirosuit
				T_Damage (current_player, world, world, vec3_origin, current_player->s.origin, vec3_origin, 1*waterlevel, 0, 0, MOD_LAVA);
			else
				T_Damage (current_player, world, world, vec3_origin, current_player->s.origin, vec3_origin, 3*waterlevel, 0, 0, MOD_LAVA);
		}

		if (current_player->watertype & CONTENTS_SLIME)
		{
			if (!envirosuit)
			{	// no damage from slime with envirosuit
				T_Damage (current_player, world, world, vec3_origin, current_player->s.origin, vec3_origin, 1*waterlevel, 0, 0, MOD_SLIME);
			}
		}
	}
}


/*
===============
G_SetClientEffects
===============
*/
void G_SetClientEffects (edict_t *ent)
{
	int		pa_type;
	int		remaining;

    if (ent->health > 0)
	{
		ent->s.effects = 0;
		ent->s.renderfx = 0;
	}//acrid3 to stop skipped freeze effect on dead bodies


	ent->s.effects = 0;
	ent->s.renderfx = 0;

	if (ent->health <= 0 || level.intermissiontime)
		return;

   if (!(ent->s.renderfx & RF_IR_VISIBLE))
       ent->s.renderfx |= RF_IR_VISIBLE; 

	if (ent->powerarmor_time > level.time)
	{
		pa_type = PowerArmorType (ent);
		if (pa_type == POWER_ARMOR_SCREEN)
		{
			ent->s.effects |= EF_POWERSCREEN;
		}
		else if (pa_type == POWER_ARMOR_SHIELD)
		{
			ent->s.effects |= EF_COLOR_SHELL;
			ent->s.renderfx |= RF_SHELL_GREEN;
		}
	}

//ZOID
	CTFEffects(ent);
//ZOID

	if (ent->client->quad_framenum > level.framenum)
	{
		remaining = ent->client->quad_framenum - level.framenum;
		if (remaining > 30 || (remaining & 4) )
			ent->s.effects |= EF_QUAD;
	}

	if (ent->client->invincible_framenum > level.framenum)
	{
		remaining = ent->client->invincible_framenum - level.framenum;
		if (remaining > 30 || (remaining & 4) )
			ent->s.effects |= EF_PENT;
	}

	// show cheaters!!! and Kamikazes!!!
	if ((ent->flags & FL_GODMODE) || (ent->client->kamikaze_mode & 1))
	{
		ent->s.effects |= EF_HALF_DAMAGE;
	}
	if (ent->client->pers.abilities.berserk_shield)
	{
		ent->s.effects |= EF_COLOR_SHELL;
		ent->s.renderfx |= RF_SHELL_GREEN;
	}
	//acrid 3/99
	if (ent->frozen)//white shell
	{
		ent->client->ps.rdflags |= RDF_UNDERWATER;
		ent->s.effects |= EF_COLOR_SHELL;
		ent->s.renderfx |= (RF_SHELL_RED|RF_SHELL_GREEN|RF_SHELL_BLUE);
		SV_AddBlend (0.75, 0.75, 0.75, 0.6, ent->client->ps.blend);
	}
}


/*
===============
G_SetClientEvent
===============
*/
void G_SetClientEvent (edict_t *ent)
{
	int r;

	if (ent->s.event)
		return;

	if (ent->health > 1)
	{
	if (ent->client->resp.class == 1)
	{
		if ( ent->groundentity )
		{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				ent->client->resp.nextidlesound = level.time +10.5;
				return;
			}
			if (ent->client->resp.nextidlesound < level.time)
			{
				gi.sound(ent, CHAN_AUTO, gi.soundindex ("soldier/solidle1.wav"), 1, ATTN_NORM, 0);
				ent->client->resp.nextidlesound = level.time +10.5;
			}
		}
	}
	if (ent->client->resp.class == 36)
	{
		if ( ent->groundentity )
		{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				ent->client->resp.nextidlesound = level.time +10.5;
				return;
			}
			if (ent->client->resp.nextidlesound < level.time)
			{
				gi.sound(ent, CHAN_AUTO, gi.soundindex ("soldier/solidle1.wav"), 1, ATTN_NORM, 0);
				ent->client->resp.nextidlesound = level.time +10.5;
			}
		}
	}
	if (ent->client->resp.class == 20)
	{
		if ( ent->groundentity )
		{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				ent->client->resp.nextidlesound = level.time +10.5;
				return;
			}
			if (ent->client->resp.nextidlesound < level.time)
			{
				gi.sound(ent, CHAN_AUTO, gi.soundindex ("soldier/solidle1.wav"), 1, ATTN_NORM, 0);
				ent->client->resp.nextidlesound = level.time +10.5;
			}
		}
	}
	else if (ent->client->resp.class == 2)
	{
		r = rndnum (0, 4);

			if (ent->s.frame == 70)
			{
				//gi.bprintf (PRINT_HIGH, "%i", r);
				if (r == 2)
				{
					ent->s.frame = 1;
					ent->client->anim_end = 48;
				}
				else
				{
					ent->s.frame = 49;
					ent->client->anim_end = 71;
				}
			}
			//if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			//{
				//ent->client->resp.nextidlesound = level.time +10.5;
				//return;
			//}
			//if (ent->client->resp.nextidlesound < level.time)
			//{
			if (ent->s.frame == 1)
				gi.sound(ent, CHAN_AUTO, gi.soundindex ("infantry/infidle1.wav"), 1, ATTN_NORM, 0);
				//ent->client->resp.nextidlesound = level.time +10.5;
			//}
		//}
	}
	else if (ent->client->resp.class == 14)
	{
		if ( ent->groundentity )
		{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				ent->client->resp.nextidlesound = level.time +10.5;
				return;
			}
			if (ent->client->resp.nextidlesound < level.time)
			{
				gi.sound(ent, CHAN_AUTO, gi.soundindex ("tank/tnkidle1.wav"), 1, ATTN_NORM, 0);
				ent->client->resp.nextidlesound = level.time +10.5;
			}
		}
	}
	else if (ent->client->resp.class == 41)
	{
		if ( ent->groundentity )
		{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				ent->client->resp.nextidlesound = level.time +10.5;
				return;
			}
			if (ent->client->resp.nextidlesound < level.time)
			{
				gi.sound(ent, CHAN_AUTO, gi.soundindex ("tank/tnkidle1.wav"), 1, ATTN_NORM, 0);
				ent->client->resp.nextidlesound = level.time +10.5;
			}
		}
	}
	else if (ent->client->resp.class == 3)
	{
		if ( ent->groundentity )
		{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				ent->client->resp.nextidlesound = level.time +10.5;
				return;
			}
			if (ent->client->resp.nextidlesound < level.time)
			{
				gi.sound(ent, CHAN_AUTO, gi.soundindex ("tank/tnkidle1.wav"), 1, ATTN_NORM, 0);
				ent->client->resp.nextidlesound = level.time +10.5;
			}
		}
	}
	else if (ent->client->resp.class == 33)
	{
		if ( ent->groundentity )
		{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				ent->client->resp.nextidlesound = level.time +10.5;
				return;
			}
			if (ent->client->resp.nextidlesound < level.time)
			{
				gi.sound(ent, CHAN_AUTO, gi.soundindex ("tank/tnkidle1.wav"), 1, ATTN_NORM, 0);
				ent->client->resp.nextidlesound = level.time +10.5;
			}
		}
	}
	else if (ent->client->resp.class == 4)
	{
		if ( ent->groundentity )
		{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				ent->client->resp.nextidlesound = level.time +10.5;
				return;
			}
			if (ent->client->resp.nextidlesound < level.time)
			{
				gi.sound(ent, CHAN_AUTO, gi.soundindex ("medic/idle.wav"), 1, ATTN_NORM, 0);
				ent->client->resp.nextidlesound = level.time +10.5;
			}
		}
	}
	else if (ent->client->resp.class == 28)
	{
		if ( ent->groundentity )
		{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				ent->client->resp.nextidlesound = level.time +10.5;
				return;
			}
			if (ent->client->resp.nextidlesound < level.time)
			{
				gi.sound(ent, CHAN_AUTO, gi.soundindex ("medic_commander/medidle.wav"), 1, ATTN_NORM, 0);
				ent->client->resp.nextidlesound = level.time +10.5;
			}
		}
	}
	else if (ent->client->resp.class == 5)
	{
		if ( ent->groundentity )
		{
			//if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			//{
				//ent->client->resp.nextidlesound = level.time +10.5;
				//return;
			//}
			//if (ent->client->resp.nextidlesound < level.time)
			//{
			if (ent->s.frame == 29)
			{
				if (random() > 0.10)
				{
					ent->s.frame = 30;
					ent->client->anim_end = 69;
				}
				else
				{
					ent->s.frame = 0;
					ent->client->anim_end = 69;
				}
			}
			if (ent->s.frame == 34)
				gi.sound(ent, CHAN_AUTO, gi.soundindex ("gunner/gunidle1.wav"), 1, ATTN_NORM, 0);
				//ent->client->resp.nextidlesound = level.time +10.5;
			//}
		}
	}
	else if (ent->client->resp.class == 6)
	{
		if ( ent->groundentity )
		{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				ent->client->resp.nextidlesound = level.time +10.5;
				return;
			}
			if (ent->client->resp.nextidlesound < level.time)
			{
				gi.sound(ent, CHAN_AUTO, gi.soundindex ("gladiator/gldidle1.wav"), 1, ATTN_NORM, 0);
				ent->client->resp.nextidlesound = level.time +10.5;
			}
		}
	}
	else if (ent->client->resp.class == 34)
	{
		if ( ent->groundentity )
		{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				ent->client->resp.nextidlesound = level.time +10.5;
				return;
			}
			if (ent->client->resp.nextidlesound < level.time)
			{
				gi.sound(ent, CHAN_AUTO, gi.soundindex ("gladiator/gldidle1.wav"), 1, ATTN_NORM, 0);
				ent->client->resp.nextidlesound = level.time +10.5;
			}
		}
	}
	else if (ent->client->resp.class == 21)
	{
		if ( ent->groundentity )
		{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				ent->client->resp.nextidlesound = level.time +10.5;
				return;
			}
			if (ent->client->resp.nextidlesound < level.time)
			{
				gi.sound(ent, CHAN_AUTO, gi.soundindex ("gladiator/gldidle1.wav"), 1, ATTN_NORM, 0);
				ent->client->resp.nextidlesound = level.time +10.5;
			}
		}
	}
	else if (ent->client->resp.class == 7)
	{
		if ( ent->groundentity )
		{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				ent->client->resp.nextidlesound = level.time +10.5;
				return;
			}
			if (ent->client->resp.nextidlesound < level.time)
			{
				gi.sound(ent, CHAN_AUTO, gi.soundindex ("bosstank/btkunqv1.wav"), 1, ATTN_NORM, 0);
				ent->client->resp.nextidlesound = level.time +10.5;
			}
		}
	}
	else if (ent->client->resp.class == 39)
	{
		if ( ent->groundentity )
		{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				ent->client->resp.nextidlesound = level.time +10.5;
				return;
			}
			if (ent->client->resp.nextidlesound < level.time)
			{
				gi.sound(ent, CHAN_AUTO, gi.soundindex ("bosstank/btkunqv1.wav"), 1, ATTN_NORM, 0);
				ent->client->resp.nextidlesound = level.time +10.5;
			}
		}
	}	else if (ent->client->resp.class == 24)
	{
		if ( ent->groundentity )
		{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				ent->client->resp.nextidlesound = level.time +10.5;
				return;
			}
			if (ent->client->resp.nextidlesound < level.time)
			{
				gi.sound(ent, CHAN_AUTO, gi.soundindex ("bosstank/btkunqv1.wav"), 1, ATTN_NORM, 0);
				ent->client->resp.nextidlesound = level.time +10.5;
			}
		}
	}
	else if (ent->client->resp.class == 8)
	{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				ent->client->resp.nextidlesound = level.time +10.5;
				return;
			}
		if (ent->client->resp.nextidlesound < level.time)
		{
			gi.sound(ent, CHAN_AUTO, gi.soundindex ("flyer/flysrch1.wav"), 1, ATTN_NORM, 0);
			ent->client->resp.nextidlesound = level.time +10.5;
		}
	}
	else if (ent->client->resp.class == 35)
	{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				ent->client->resp.nextidlesound = level.time +10.5;
				return;
			}
		if (ent->client->resp.nextidlesound < level.time)
		{
			gi.sound(ent, CHAN_AUTO, gi.soundindex ("flyer/flysrch1.wav"), 1, ATTN_NORM, 0);
			ent->client->resp.nextidlesound = level.time +10.5;
		}
	}
	else if (ent->client->resp.class == 32)
	{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				ent->client->resp.nextidlesound = level.time +10.5;
				return;
			}
		if (ent->client->resp.nextidlesound < level.time)
		{
			gi.sound(ent, CHAN_AUTO, gi.soundindex ("flyer/flysrch1.wav"), 1, ATTN_NORM, 0);
			ent->client->resp.nextidlesound = level.time +10.5;
		}
	}
	else if (ent->client->resp.class == 17)
	{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				ent->client->resp.nextidlesound = level.time +10.5;
				return;
			}
		if (ent->client->resp.nextidlesound < level.time)
		{
			gi.sound(ent, CHAN_AUTO, gi.soundindex ("flipper/Flpidle1.wav"), 1, ATTN_NORM, 0);
			ent->client->resp.nextidlesound = level.time +10.5;
		}
	}
	else if (ent->client->resp.class == 16)
	{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				ent->client->resp.nextidlesound = level.time +10.5;
				return;
			}
		if (ent->client->resp.nextidlesound < level.time)
		{
			gi.sound(ent, CHAN_AUTO, gi.soundindex ("floater/fltidle1.wav"), 1, ATTN_NORM, 0);
			ent->client->resp.nextidlesound = level.time +10.5;
		}
	}
	else if (ent->client->resp.class == 9)
	{
			//if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			//{
				//ent->client->resp.nextidlesound = level.time +10.5;
				//return;
			//}
		//if (ent->client->resp.nextidlesound < level.time)
		//{
		r = rndnum (0, 8);
			if (ent->s.frame == 4)
			{
				//gi.bprintf (PRINT_HIGH, "%i", r);
				if (r == 4)
				{
					ent->s.frame = 5;
					ent->client->anim_end = 24;
				}
				else
				{
					ent->s.frame = 0;
					ent->client->anim_end = 4;
				}
			}
			if (ent->s.frame == 10)
				gi.sound(ent, CHAN_AUTO, gi.soundindex ("berserk/beridle1.wav"), 1, ATTN_NORM, 0);
			//ent->client->resp.nextidlesound = level.time +10.5;
		//}
	}
	else if (ent->client->resp.class == 10)
	{
		//	if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
		//	{
		//		ent->client->resp.nextidlesound = level.time +10.5;
		//		return;
		//	}
	//	if (ent->client->resp.nextidlesound < level.time)
	//	{
		if (ent->s.frame == 154)
			gi.sound(ent, CHAN_AUTO, gi.soundindex ("chick/chkidle1.wav"), 1, ATTN_NORM, 0);
	//		ent->client->resp.nextidlesound = level.time +10.5;
	//	}
	}
	else if (ent->client->resp.class == 23)
	{
		//	if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
		//	{
		//		ent->client->resp.nextidlesound = level.time +10.5;
		//		return;
		//	}
	//	if (ent->client->resp.nextidlesound < level.time)
	//	{
		if (ent->s.frame == 154)
			gi.sound(ent, CHAN_AUTO, gi.soundindex ("chick/chkidle1.wav"), 1, ATTN_NORM, 0);
	//		ent->client->resp.nextidlesound = level.time +10.5;
	//	}
	}
	else if (ent->client->resp.class == 12)
	{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				ent->client->resp.nextidlesound = level.time +10.5;
				return;
			}
		if (ent->client->resp.nextidlesound < level.time)
		{
			gi.sound(ent, CHAN_AUTO, gi.soundindex ("brain/brnlens1.wav"), 1, ATTN_NORM, 0);
			ent->client->resp.nextidlesound = level.time +10.5;
		}
	}
	else if (ent->client->resp.class == 22)
	{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				ent->client->resp.nextidlesound = level.time +10.5;
				return;
			}
		if (ent->client->resp.nextidlesound < level.time)
		{
			gi.sound(ent, CHAN_AUTO, gi.soundindex ("brain/brnlens1.wav"), 1, ATTN_NORM, 0);
			ent->client->resp.nextidlesound = level.time +10.5;
		}
	}
	else if (ent->client->resp.class == 13)
	{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				ent->client->resp.nextidlesound = level.time +10.5;
				return;
			}
		if (ent->client->resp.nextidlesound < level.time)
		{
			gi.sound(ent, CHAN_AUTO, gi.soundindex ("mutant/mutidle1.wav"), 1, ATTN_NORM, 0);
			ent->client->resp.nextidlesound = level.time +10.5;
		}
	}
	else if (ent->client->resp.class == 18)
	{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				ent->client->resp.nextidlesound = level.time +10.5;
				return;
			}
		if (ent->client->resp.nextidlesound < level.time)
		{ 
		// Paril - left 1 right 2
		//if (ent->s.frame == 112)
			gi.sound(ent, CHAN_AUTO, gi.soundindex ("boss3/bs3idle1.wav"), 1, ATTN_NORM, 0);
		//if (ent->s.frame == 146 || ent->s.frame == 159)
		//	gi.sound(ent, CHAN_AUTO, gi.soundindex ("boss3/step2.wav"), 1, ATTN_NORM, 0);
		//if (ent->s.frame == 149 || ent->s.frame == 162)
		//	gi.sound(ent, CHAN_AUTO, gi.soundindex ("boss3/step1.wav"), 1, ATTN_NORM, 0);
			ent->client->resp.nextidlesound = level.time +10.5;
		}
	}
	else if (ent->client->resp.class == 19)
	{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				ent->client->resp.nextidlesound = level.time +10.5;
				return;
			}
		if (ent->client->resp.nextidlesound < level.time)
		{
			gi.sound(ent, CHAN_AUTO, gi.soundindex ("makron/laf3.wav"), 1, ATTN_NORM, 0);
			ent->client->resp.nextidlesound = level.time +10.5;
		}
	}
	else if (ent->client->resp.class == 30)
	{
		r = rndnum (0, 60);
		//	if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
		//	{
		//		ent->client->resp.nextidlesound = level.time +10.5;
		//		return;
		//	}
		//if (ent->client->resp.nextidlesound < level.time)
		//{
			if (ent->s.frame == 0)
			{
				//gi.bprintf (PRINT_HIGH, "%i", r);
				if (r == 5)
				{
					ent->s.frame = 1;
					ent->client->anim_end = 10;
				}
				else
				{
					ent->s.frame = 0;
					ent->client->anim_end = 0;
				}
			}
		if (ent->s.frame == 1)
			gi.sound(ent, CHAN_AUTO, gi.soundindex ("widow/laugh.wav"), 1, ATTN_NORM, 0);
		//	ent->client->resp.nextidlesound = level.time +10.5;
		//}
	}
	else if (ent->client->resp.class == 31)
	{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				ent->client->resp.nextidlesound = level.time +10.5;
				return;
			}
		if (ent->client->resp.nextidlesound < level.time)
		{
			gi.sound(ent, CHAN_AUTO, gi.soundindex ("zortemp/ack.wav"), 1, ATTN_NORM, 0);
			ent->client->resp.nextidlesound = level.time +10.5;
		}
	}
	else if (ent->client->resp.class == 15)
	{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				ent->client->resp.nextidlesound = level.time +10.5;
				return;
			}
		if (ent->client->resp.nextidlesound < level.time)
		{
			gi.sound(ent, CHAN_AUTO, gi.soundindex ("hover/hovsrch1.wav"), 1, ATTN_NORM, 0);
			ent->client->resp.nextidlesound = level.time +10.5;
		}
	}
	else if (ent->client->resp.class == 26)
	{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				ent->client->resp.nextidlesound = level.time +10.5;
				return;
			}
		if (ent->client->resp.nextidlesound < level.time)
		{
			gi.sound(ent, CHAN_AUTO, gi.soundindex ("daedalus/daedsrch2.wav"), 1, ATTN_NORM, 0);
			ent->client->resp.nextidlesound = level.time +10.5;
		}
	}
	else if (ent->client->resp.class == 25)
	{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				ent->client->resp.nextidlesound = level.time +10.5;
				return;
			}
		if (ent->client->resp.nextidlesound < level.time)
		{
			gi.sound(ent, CHAN_AUTO, gi.soundindex ("gek/gk_idle1.wav"), 1, ATTN_NORM, 0);
			ent->client->resp.nextidlesound = level.time +10.5;
		}
	}
	else if (ent->client->resp.class == 27)
	{
			if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			{
				ent->client->resp.nextidlesound = level.time +10.5;
				return;
			}
		if (ent->client->resp.nextidlesound < level.time)
		{
			gi.sound(ent, CHAN_AUTO, gi.soundindex ("stalker/idle.wav"), 1, ATTN_NORM, 0);
			ent->client->resp.nextidlesound = level.time +10.5;
		}
	}
	}
	if (ent->client->resp.class == 3)
	{
		//if ( ent->groundentity && xyspeed > 10)
		//{
			//if (ent->client->resp.nextfootstep < level.time)
			//{
				//gi.sound(ent, 0, gi.soundindex ("tank/step.wav"), 1, ATTN_NORM, 0);
				//ent->client->resp.nextfootstep = level.time +0.8;
			//}
			if (ent->s.frame == 37 || ent->s.frame == 45)
				gi.sound(ent, 0, gi.soundindex ("tank/step.wav"), 1, ATTN_NORM, 0);
		//}
	}
	if (ent->client->resp.class == 33)
	{
		//if ( ent->groundentity && xyspeed > 10)
		//{
			//if (ent->client->resp.nextfootstep < level.time)
			//{
				//gi.sound(ent, 0, gi.soundindex ("tank/step.wav"), 1, ATTN_NORM, 0);
				//ent->client->resp.nextfootstep = level.time +0.8;
			//}
			if (ent->s.frame == 37 || ent->s.frame == 45)
				gi.sound(ent, 0, gi.soundindex ("tank/step.wav"), 1, ATTN_NORM, 0);
		//}
	}
	if (ent->client->resp.class == 14)
	{
		//if ( ent->groundentity && xyspeed > 10)
		//{
			//if (ent->client->resp.nextfootstep < level.time)
			//{
				//gi.sound(ent, 0, gi.soundindex ("tank/step.wav"), 1, ATTN_NORM, 0);
				//ent->client->resp.nextfootstep = level.time +0.8;
			//}
			if (ent->s.frame == 37 || ent->s.frame == 45)
				gi.sound(ent, 0, gi.soundindex ("tank/step.wav"), 1, ATTN_NORM, 0);
		//}
	}
	if (ent->client->resp.class == 41)
	{
		//if ( ent->groundentity && xyspeed > 10)
		//{
			//if (ent->client->resp.nextfootstep < level.time)
			//{
				//gi.sound(ent, 0, gi.soundindex ("tank/step.wav"), 1, ATTN_NORM, 0);
				//ent->client->resp.nextfootstep = level.time +0.8;
			//}
			if (ent->s.frame == 37 || ent->s.frame == 45)
				gi.sound(ent, 0, gi.soundindex ("tank/step.wav"), 1, ATTN_NORM, 0);
		//}
	}
	if (ent->client->resp.class == 13)
	{
		//if ( ent->groundentity && xyspeed > 10)
		//{
			//if (ent->client->resp.nextfootstep < level.time)
		//	{
				r = rndnum (1, 5);
				if (ent->s.frame == 57 || ent->s.frame == 59)
					gi.sound(ent, 0, gi.soundindex (va("mutant/step%i.wav", r)), 1, ATTN_NORM, 0);
			//	ent->client->resp.nextfootstep = level.time +0.2;
		//	}
		//}
	}
	if (ent->client->resp.class == 25)
	{
		//if ( ent->groundentity && xyspeed > 10)
		//{
			//if (ent->client->resp.nextfootstep < level.time)
			//{
				r = rndnum (1, 5);
				if (ent->s.frame == 42 || ent->s.frame == 44)
					gi.sound(ent, 0, gi.soundindex (va("gek/gk_step%i.wav", r)), 1, ATTN_NORM, 0);
				//ent->client->resp.nextfootstep = level.time +0.2;
			//}
		//}
	}
	if (ent->client->resp.class == 18)
	{
		//if ( ent->groundentity && xyspeed > 10)
		//{
			//if (ent->client->resp.nextfootstep < level.time)
			//{
				r = rndnum (1, 4);

				if (ent->s.frame == 167 || ent->s.frame == 174 || ent->s.frame == 181 || ent->s.frame == 187)
					gi.sound(ent, 0, gi.soundindex (va("boss3/step%i.wav", r)), 1, ATTN_NORM, 0);
			//	ent->client->resp.nextfootstep = level.time +0.4;
			//}
		//}
	}
	if (ent->client->resp.class == 19)
	{
		//if ( ent->groundentity && xyspeed > 10)
		//{
			//if (ent->client->resp.nextfootstep < level.time)
			//{
				r = rndnum (1, 3);

				if (ent->s.frame == 275 || ent->s.frame == 280 || ent->s.frame == 285 || ent->s.frame == 288)
					gi.sound(ent, 0, gi.soundindex (va("makron/step%i.wav", r)), 1, ATTN_NORM, 0);
			//	ent->client->resp.nextfootstep = level.time +0.3;
			//}
		//}
	}
	if (ent->client->resp.class == 30)
	{
		//if ( ent->groundentity && xyspeed > 10)
		//{
			//if (ent->client->resp.nextfootstep < level.time)
			//{
				r = rndnum (1, 4);
				
				if (ent->s.frame == 17 || ent->s.frame == 22)
					gi.sound(ent, 0, gi.soundindex (va("widow/bwstep%i.wav", r)), 1, ATTN_NORM, 0);
				//ent->client->resp.nextfootstep = level.time +0.3;
			//}
		//}
	}
	if (ent->client->resp.class == 6)
	{
		//if ( ent->groundentity && xyspeed > 10)
		//{
			//if (ent->client->resp.nextfootstep < level.time)
			//{
				if (ent->s.frame == 10 || ent->s.frame == 17 || ent->s.frame == 24 || ent->s.frame == 27)
					gi.sound(ent, 0, gi.soundindex ("gladiator/gldsrch1.wav"), 1, ATTN_NORM, 0);
				//ent->client->resp.nextfootstep = level.time +0.3;
			//}
		//}
	}
	if (ent->client->resp.class == 34)
	{
		//if ( ent->groundentity && xyspeed > 10)
		//{
			//if (ent->client->resp.nextfootstep < level.time)
			//{
				if (ent->s.frame == 10 || ent->s.frame == 17 || ent->s.frame == 24 || ent->s.frame == 27)
					gi.sound(ent, 0, gi.soundindex ("gladiator/gldsrch1.wav"), 1, ATTN_NORM, 0);
				//ent->client->resp.nextfootstep = level.time +0.3;
			//}
		//}
	}	if (ent->client->resp.class == 21)
	{
		//if ( ent->groundentity && xyspeed > 10)
		//{
			//if (ent->client->resp.nextfootstep < level.time)
			//{
				if (ent->s.frame == 9 || ent->s.frame == 18 || ent->s.frame == 24 || ent->s.frame == 27)
					gi.sound(ent, 0, gi.soundindex ("gladiator/gldsrch1.wav"), 1, ATTN_NORM, 0);
				//ent->client->resp.nextfootstep = level.time +0.3;
			//}
		//}
	}
	else
	{
		if ( ent->groundentity && xyspeed > 225)
		{
			if (ent->client->resp.nextfootstep < level.time)
			{
				//ent->s.event = EV_FOOTSTEP;
				ent->client->resp.nextfootstep = level.time +0.5;
			}
		}
	}
}

/*
===============
G_SetClientSound
===============
*/
void G_SetClientSound (edict_t *ent)
{
	char	*weap;

	if (ent->client->pers.game_helpchanged != game.helpchanged)
	{
		ent->client->pers.game_helpchanged = game.helpchanged;
		ent->client->pers.helpchanged = 1;
	}

	// help beep (no more than three times)
	if (ent->client->pers.helpchanged && ent->client->pers.helpchanged <= 3 && !(level.framenum&63) )
	{
		ent->client->pers.helpchanged++;
		gi.sound (ent, CHAN_VOICE, gi.soundindex ("misc/pc_up.wav"), 1, ATTN_STATIC, 0);
	}


	if (ent->client->pers.weapon)
		weap = ent->client->pers.weapon->classname;
	else
		weap = "";

	if (ent->waterlevel && (ent->watertype&(CONTENTS_LAVA|CONTENTS_SLIME)) )
		ent->s.sound = snd_fry;
	else if (strcmp(weap, "weapon_trailgun") == 0)
		ent->s.sound = gi.soundindex("weapons/rg_hum.wav");
	//else if (strcmp(weap, "weapon_bfg") == 0)
		//ent->s.sound = gi.soundindex("weapons/bfg_hum.wav");
	else if (ent->client->weapon_sound)
		ent->s.sound = ent->client->weapon_sound;
	else
		ent->s.sound = 0;
}

/*
===============
G_SetClientFrame
===============
*/
/*
// client_t->anim_priority
#define	ANIM_BASIC		0		// stand / run
#define	ANIM_WAVE		1
#define	ANIM_JUMP		2
#define	ANIM_PAIN		3
#define	ANIM_ATTACK		4
#define	ANIM_DEATH		5
#define	ANIM_REVERSE	6
*/
void G_SetClientFrame (edict_t *ent)
{
	gclient_t	*client;
	qboolean	duck, run, walk; // New, by Paril
	int walkspeed;

	walkspeed = ent->client->resp.walking;

	if( ent->frozen ) //acrid3 might not be needed
		return;

	if (ent->s.modelindex != 255)
		return;		// not in the player model

	client = ent->client;

	if (client->ps.pmove.pm_flags & PMF_DUCKED)
		duck = true;
	else
		duck = false;
	if (xyspeed)
		run = true;
	else
		run = false;
	if (walkspeed)
	{
		run = false;
		walk = true;
	}
	else
		walk = false;
	// check for stand/duck and stop/go transitions
	if (duck != client->anim_duck && client->anim_priority < ANIM_DEATH)
		goto newanim;
	if (run != client->anim_run && client->anim_priority == ANIM_BASIC)
		goto newanim;
	if (walk != client->anim_walk && client->anim_priority == ANIM_BASIC)
		goto newanim;

	if (ent->client->resp.class == 13)
	{
		if (!ent->groundentity && client->anim_priority <= ANIM_WAVE)
			goto newanim;
	}

	if(client->anim_priority == ANIM_REVERSE)
	{
		if(ent->s.frame > client->anim_end)
		{
			ent->s.frame--;
		if (ent->client->pers.abilities.soldier_haste)
			ent->s.frame--;
			return;
		}
	}
	else if (ent->s.frame < client->anim_end)
	{	// continue an animation
		ent->s.frame++;
		if (ent->client->pers.abilities.soldier_haste)
			ent->s.frame++;
		return;
	}

	if (client->anim_priority == ANIM_DEATH)
		return;		// stay there
	if (client->anim_priority == ANIM_JUMP)
	{
		if (!ent->client->resp.class == 13 || !ent->client->resp.class == 27)
		{
			goto runframe;
		}
		if (!ent->groundentity)
			goto runframe;/*return;*/		// stay there
		ent->client->anim_priority = ANIM_WAVE;
		if (ent->client->resp.class == 13)
		{
			ent->s.frame = 0;
			ent->client->anim_end = 0;
			return;
		}
		else if (ent->client->resp.class == 27)
		{
			ent->s.frame = 42;
			ent->client->anim_end = 42;
			return;
		}
		else
		{
			ent->s.frame = 0;
			ent->client->anim_end = 0;
			return;
		}
	}

newanim:
	// return to either a running or standing frame
	client->anim_priority = ANIM_BASIC;
	client->anim_duck = duck;
	client->anim_run = run;
	client->anim_walk = walk;

	// Paril, playing dead.
	if (ent->client->playing_dead)
		return;

	if (!ent->groundentity)
	{
		if (!ent->client->resp.class == 13 || !ent->client->resp.class == 27)
			goto runframe;
		if (ent->client->resp.thrusting)
			goto runframe;
		client->anim_priority = ANIM_JUMP;
		if (ent->s.frame != 0)
			ent->s.frame = 0;
		client->anim_end = 0;
		if (ent->client->resp.class == 13)
		{
			if (ent->s.frame != 3)
				ent->s.frame = 2;
			client->anim_end = 3;
		}
		if (ent->client->resp.class == 27)
		{
			if (ent->s.frame != 46)
				ent->s.frame = 45;
			client->anim_end = 46;
		}
	}
	else if (run)
	{	// running
		/*if (duck)
		{
			if (ent->client->resp.class == 1)
			{
				ent->s.frame = 98;
				ent->client->anim_end = 104;
			}
			if (ent->client->resp.class == 36)
			{
				ent->s.frame = 98;
				ent->client->anim_end = 104;
			}
			if (ent->client->resp.class == 20)
			{
				ent->s.frame = 97;
				ent->client->anim_end = 108;
			}
			if (ent->client->resp.class == 2)
			{
				ent->s.frame = 92;
				ent->client->anim_end = 99;
			}
			if (ent->client->resp.class == 3)
			{
				ent->s.frame = 34;
				ent->client->anim_end = 49;
			}
			if (ent->client->resp.class == 33)
			{
				ent->s.frame = 34;
				ent->client->anim_end = 49;
			}
			if (ent->client->resp.class == 4)
			{
				ent->s.frame = 102;
				ent->client->anim_end = 107;
			}
			if (ent->client->resp.class == 28)
			{
				ent->s.frame = 102;
				ent->client->anim_end = 107;
			}
			if (ent->client->resp.class == 5)
			{
				ent->s.frame = 94;
				ent->client->anim_end = 101;
			}
			if (ent->client->resp.class == 6)
			{
				ent->s.frame = 23;
				ent->client->anim_end = 28;
			}
			if (ent->client->resp.class == 34)
			{
				ent->s.frame = 23;
				ent->client->anim_end = 28;
			}
			if (ent->client->resp.class == 21)
			{
				ent->s.frame = 23;
				ent->client->anim_end = 28;
			}
			if (ent->client->resp.class == 7)
			{
				ent->s.frame = 128;
				ent->client->anim_end = 145;
			}
			if (ent->client->resp.class == 24)
			{
				ent->s.frame = 128;
				ent->client->anim_end = 145;
			}
			if (ent->client->resp.class == 8)
			{
				ent->s.frame = 13;
				ent->client->anim_end = 57;
			}
			if (ent->client->resp.class == 35)
			{
				ent->s.frame = 13;
				ent->client->anim_end = 57;
			}
			if (ent->client->resp.class == 32)
			{
				ent->s.frame = 121;
				ent->client->anim_end = 139;
			}
			if (ent->client->resp.class == 9)
			{
				ent->s.frame = 36;
				ent->client->anim_end = 41;
			}
			if (ent->client->resp.class == 10)
			{
				ent->s.frame = 187;
				ent->client->anim_end = 198;
			}
			if (ent->client->resp.class == 23)
			{
				ent->s.frame = 187;
				ent->client->anim_end = 198;
			}
			if (ent->client->resp.class == 11)
			{
				ent->s.frame = 0;
				ent->client->anim_end = 49;
			}
			if (ent->client->resp.class == 29)
			{
				ent->s.frame = 0;
				ent->client->anim_end = 12;
			}
			if (ent->client->resp.class == 12)
			{
				ent->s.frame = 0;
				ent->client->anim_end = 12;
			}
			if (ent->client->resp.class == 22)
			{
				ent->s.frame = 0;
				ent->client->anim_end = 12;
			}
			if (ent->client->resp.class == 13)
			{
				ent->s.frame = 56;
				ent->client->anim_end = 61;
			}
			if (ent->client->resp.class == 14)
			{
				ent->s.frame = 34;
				ent->client->anim_end = 49;
			}
			if (ent->client->resp.class == 15)
			{
				ent->s.frame = 30;
				ent->client->anim_end = 64;
			}
			if (ent->client->resp.class == 26)
			{
				ent->s.frame = 30;
				ent->client->anim_end = 64;
			}
			if (ent->client->resp.class == 16)
			{
				ent->s.frame = 144;
				ent->client->anim_end = 247;
			}
			if (ent->client->resp.class == 17)
			{
				ent->s.frame = 41;
				ent->client->anim_end = 64;
			}
			if (ent->client->resp.class == 18)
			{
				ent->s.frame = 163;
				ent->client->anim_end = 175;
			}
			if (ent->client->resp.class == 19)
			{
				ent->s.frame = 274;
				ent->client->anim_end = 283;
			}
			if (ent->client->resp.class == 30)
			{
				ent->s.frame = 11;
				ent->client->anim_end = 23;
			}
			if (ent->client->resp.class == 31)
			{
				ent->s.frame = 1;
				ent->client->anim_end = 9;
			}
			if (ent->client->resp.class == 25)
			{
				ent->s.frame = 39;
				ent->client->anim_end = 44;
			}
			if (ent->client->resp.class == 27)
			{
				ent->s.frame = 49;
				ent->client->anim_end = 52;
			}
		}
		else
		{*/
runframe:
			if (ent->client->resp.class == 1)
			{
				ent->s.frame = 98;
				ent->client->anim_end = 104;
			}
			if (ent->client->resp.class == 36)
			{
				ent->s.frame = 98;
				ent->client->anim_end = 104;
			}
			if (ent->client->resp.class == 20)
			{
				ent->s.frame = 97;
				ent->client->anim_end = 108;
			}
			if (ent->client->resp.class == 2)
			{
				ent->s.frame = 92;
				ent->client->anim_end = 99;
			}
			if (ent->client->resp.class == 3)
			{
				ent->s.frame = 34;
				ent->client->anim_end = 49;
			}
			if (ent->client->resp.class == 33)
			{
				ent->s.frame = 34;
				ent->client->anim_end = 49;
			}
			if (ent->client->resp.class == 4)
			{
				ent->s.frame = 102;
				ent->client->anim_end = 107;
			}
			if (ent->client->resp.class == 28)
			{
				ent->s.frame = 102;
				ent->client->anim_end = 107;
			}
			if (ent->client->resp.class == 5)
			{
				ent->s.frame = 94;
				ent->client->anim_end = 101;
			}
			if (ent->client->resp.class == 6)
			{
				ent->s.frame = 23;
				ent->client->anim_end = 28;
			}
			if (ent->client->resp.class == 34)
			{
				ent->s.frame = 23;
				ent->client->anim_end = 28;
			}
			if (ent->client->resp.class == 21)
			{
				ent->s.frame = 23;
				ent->client->anim_end = 28;
			}
			if (ent->client->resp.class == 7)
			{
				ent->s.frame = 128;
				ent->client->anim_end = 145;
			}
			if (ent->client->resp.class == 39)
			{
				ent->s.frame = 128;
				ent->client->anim_end = 145;
			}
			if (ent->client->resp.class == 24)
			{
				ent->s.frame = 128;
				ent->client->anim_end = 145;
			}
			if (ent->client->resp.class == 8)
			{
				ent->s.frame = 13;
				ent->client->anim_end = 57;
			}
			if (ent->client->resp.class == 35)
			{
				ent->s.frame = 13;
				ent->client->anim_end = 57;
			}
			if (ent->client->resp.class == 32)
			{
				ent->s.frame = 121;
				ent->client->anim_end = 139;
			}
			if (ent->client->resp.class == 9)
			{
				ent->s.frame = 36;
				ent->client->anim_end = 41;
			}
			if (ent->client->resp.class == 10)
			{
				ent->s.frame = 187;
				ent->client->anim_end = 198;
			}
			if (ent->client->resp.class == 23)
			{
				ent->s.frame = 187;
				ent->client->anim_end = 198;
			}
			if (ent->client->resp.class == 11)
			{
				ent->s.frame = 0;
				ent->client->anim_end = 49;
			}
			if (ent->client->resp.class == 12)
			{
				ent->s.frame = 0;
				ent->client->anim_end = 12;
			}
			if (ent->client->resp.class == 22)
			{
				ent->s.frame = 0;
				ent->client->anim_end = 12;
			}
			if (ent->client->resp.class == 13)
			{
				ent->s.frame = 56;
				ent->client->anim_end = 61;
			}
			if (ent->client->resp.class == 14)
			{
				ent->s.frame = 34;
				ent->client->anim_end = 49;
			}
			if (ent->client->resp.class == 41)
			{
				ent->s.frame = 34;
				ent->client->anim_end = 49;
			}
			if (ent->client->resp.class == 40)
			{
				ent->s.frame = 68;
				ent->client->anim_end = 77;
			}
			if (ent->client->resp.class == 15)
			{
				ent->s.frame = 30;
				ent->client->anim_end = 64;
			}
			if (ent->client->resp.class == 26)
			{
				ent->s.frame = 30;
				ent->client->anim_end = 64;
			}
			if (ent->client->resp.class == 16)
			{
				ent->s.frame = 144;
				ent->client->anim_end = 247;
			}
			if (ent->client->resp.class == 17)
			{
				ent->s.frame = 41;
				ent->client->anim_end = 64;
			}
			if (ent->client->resp.class == 18)
			{
				ent->s.frame = 163;
				ent->client->anim_end = 175;
			}
			if (ent->client->resp.class == 19)
			{
				ent->s.frame = 274;
				ent->client->anim_end = 283;
			}
			if (ent->client->resp.class == 30)
			{
				ent->s.frame = 11;
				ent->client->anim_end = 23;
			}
			if (ent->client->resp.class == 31)
			{
				ent->s.frame = 1;
				ent->client->anim_end = 9;
			}
			if (ent->client->resp.class == 25)
			{
				ent->s.frame = 39;
				ent->client->anim_end = 44;
			}
			if (ent->client->resp.class == 27)
			{
				ent->s.frame = 49;
				ent->client->anim_end = 52;
			}
			if (ent->client->resp.class == 29)
			{
				ent->s.frame = 0;
				ent->client->anim_end = 12;
			}
			if (ent->client->resp.class == 37)
			{
				ent->s.frame = 40;
				ent->client->anim_end = 45;
			}
			if (ent->client->resp.class == 38)
			{
				ent->s.frame = 40;
				ent->client->anim_end = 45;
			}
		//}
	}
	else
	{	// standing
		/*if (duck)
		{
			if (ent->client->resp.class == 1)
			{
				ent->s.frame = 146;
				ent->client->anim_end = 175;
			}
			if (ent->client->resp.class == 36)
			{
				ent->s.frame = 146;
				ent->client->anim_end = 175;
			}
			if (ent->client->resp.class == 20)
			{
				ent->s.frame = 146;
				ent->client->anim_end = 175;
			}
			if (ent->client->resp.class == 2)
			{
				ent->s.frame = 49;
				ent->client->anim_end = 71;
			}
			if (ent->client->resp.class == 3)
			{
				ent->s.frame = 0;
				ent->client->anim_end = 29;
			}
			if (ent->client->resp.class == 33)
			{
				ent->s.frame = 0;
				ent->client->anim_end = 29;
			}
			if (ent->client->resp.class == 4)
			{
				ent->s.frame = 12;
				ent->client->anim_end = 101;
			}
			if (ent->client->resp.class == 28)
			{
				ent->s.frame = 12;
				ent->client->anim_end = 101;
			}
			if (ent->client->resp.class == 5)
			{
				ent->s.frame = 0;
				ent->client->anim_end = 29;
			}
			if (ent->client->resp.class == 6)
			{
				ent->s.frame = 0;
				ent->client->anim_end = 6;
			}
			if (ent->client->resp.class == 34)
			{
				ent->s.frame = 0;
				ent->client->anim_end = 6;
			}
			if (ent->client->resp.class == 21)
			{
				ent->s.frame = 0;
				ent->client->anim_end = 6;
			}
			if (ent->client->resp.class == 7)
			{
				ent->s.frame = 194;
				ent->client->anim_end = 253;
			}
			if (ent->client->resp.class == 24)
			{
				ent->s.frame = 194;
				ent->client->anim_end = 253;
			}
			if (ent->client->resp.class == 8)
			{
				ent->s.frame = 13;
				ent->client->anim_end = 57;
			}
			if (ent->client->resp.class == 35)
			{
				ent->s.frame = 13;
				ent->client->anim_end = 57;
			}
			if (ent->client->resp.class == 32)
			{
				ent->s.frame = 121;
				ent->client->anim_end = 139;
			}
			if (ent->client->resp.class == 9)
			{
				ent->s.frame = 0;
				ent->client->anim_end = 4;
			}
			if (ent->client->resp.class == 10)
			{
				ent->s.frame = 121;
				ent->client->anim_end = 180;
			}
			if (ent->client->resp.class == 23)
			{
				ent->s.frame = 121;
				ent->client->anim_end = 180;
			}
			if (ent->client->resp.class == 11)
			{
				ent->s.frame = 0;
				ent->client->anim_end = 49;
			}
			if (ent->client->resp.class == 12)
			{
				ent->s.frame = 162;
				ent->client->anim_end = 221;
			}
			if (ent->client->resp.class == 22)
			{
				ent->s.frame = 162;
				ent->client->anim_end = 221;
			}
			if (ent->client->resp.class == 13)
			{
				ent->s.frame = 62;
				ent->client->anim_end = 125;
			}
			if (ent->client->resp.class == 14)
			{
				ent->s.frame = 0;
				ent->client->anim_end = 29;
			}
			if (ent->client->resp.class == 15)
			{
				ent->s.frame = 0;
				ent->client->anim_end = 29;
			}
			if (ent->client->resp.class == 26)
			{
				ent->s.frame = 0;
				ent->client->anim_end = 29;
			}
			if (ent->client->resp.class == 16)
			{
				ent->s.frame = 144;
				ent->client->anim_end = 247;
			}
			if (ent->client->resp.class == 17)
			{
				ent->s.frame = 65;
				ent->client->anim_end = 93;
			}
			if (ent->client->resp.class == 18)
			{
				ent->s.frame = 112;
				ent->client->anim_end = 162;
			}
			if (ent->client->resp.class == 19)
			{
				ent->s.frame = 274;
				ent->client->anim_end = 283;
			}
			if (ent->client->resp.class == 30)
			{
				ent->s.frame = 0;
				ent->client->anim_end = 0;
			}
			if (ent->client->resp.class == 31)
			{
				ent->s.frame = 0;
				ent->client->anim_end = 0;
			}
			if (ent->client->resp.class == 25)
			{
				ent->s.frame = 0;
				ent->client->anim_end = 38;
			}
			if (ent->client->resp.class == 27)
			{
				ent->s.frame = 0;
				ent->client->anim_end = 33;
			}
			if (ent->client->resp.class == 29)
			{
				ent->s.frame = 0;
				ent->client->anim_end = 12;
			}
		}
		else
		{*/
			if (ent->client->resp.class == 1)
			{
				ent->s.frame = 146;
				ent->client->anim_end = 175;
			}
			if (ent->client->resp.class == 36)
			{
				ent->s.frame = 146;
				ent->client->anim_end = 175;
			}
			if (ent->client->resp.class == 20)
			{
				ent->s.frame = 146;
				ent->client->anim_end = 175;
			}
			if (ent->client->resp.class == 2)
			{
				ent->s.frame = 49;
				ent->client->anim_end = 71;
			}
			if (ent->client->resp.class == 3)
			{
				ent->s.frame = 0;
				ent->client->anim_end = 29;
			}
			if (ent->client->resp.class == 33)
			{
				ent->s.frame = 0;
				ent->client->anim_end = 29;
			}
			if (ent->client->resp.class == 4)
			{
				ent->s.frame = 12;
				ent->client->anim_end = 101;
			}
			if (ent->client->resp.class == 28)
			{
				ent->s.frame = 12;
				ent->client->anim_end = 101;
			}
			if (ent->client->resp.class == 5)
			{
				ent->s.frame = 0;
				ent->client->anim_end = 29;
			}
			if (ent->client->resp.class == 6)
			{
				ent->s.frame = 0;
				ent->client->anim_end = 6;
			}
			if (ent->client->resp.class == 34)
			{
				ent->s.frame = 0;
				ent->client->anim_end = 6;
			}
			if (ent->client->resp.class == 21)
			{
				ent->s.frame = 0;
				ent->client->anim_end = 6;
			}
			if (ent->client->resp.class == 7)
			{
				ent->s.frame = 194;
				ent->client->anim_end = 253;
			}
			if (ent->client->resp.class == 39)
			{
				ent->s.frame = 194;
				ent->client->anim_end = 253;
			}
			if (ent->client->resp.class == 24)
			{
				ent->s.frame = 194;
				ent->client->anim_end = 253;
			}
			if (ent->client->resp.class == 8)
			{
				ent->s.frame = 13;
				ent->client->anim_end = 57;
			}
			if (ent->client->resp.class == 35)
			{
				ent->s.frame = 13;
				ent->client->anim_end = 57;
			}
			if (ent->client->resp.class == 32)
			{
				ent->s.frame = 121;
				ent->client->anim_end = 139;
			}
			if (ent->client->resp.class == 9)
			{
				ent->s.frame = 0;
				ent->client->anim_end = 4;
			}
			if (ent->client->resp.class == 10)
			{
				ent->s.frame = 121;
				ent->client->anim_end = 180;
			}
			if (ent->client->resp.class == 23)
			{
				ent->s.frame = 121;
				ent->client->anim_end = 180;
			}
			if (ent->client->resp.class == 11)
			{
				ent->s.frame = 0;
				ent->client->anim_end = 49;
			}
			if (ent->client->resp.class == 12)
			{
				ent->s.frame = 162;
				ent->client->anim_end = 221;
			}
			if (ent->client->resp.class == 22)
			{
				ent->s.frame = 162;
				ent->client->anim_end = 221;
			}
			if (ent->client->resp.class == 13)
			{
				ent->s.frame = 62;
				ent->client->anim_end = 125;
			}
			if (ent->client->resp.class == 14)
			{
				ent->s.frame = 0;
				ent->client->anim_end = 29;
			}
			if (ent->client->resp.class == 41)
			{
				ent->s.frame = 0;
				ent->client->anim_end = 29;
			}
			if (ent->client->resp.class == 40)
			{
				ent->s.frame = 83;
				ent->client->anim_end = 99;
			}
			if (ent->client->resp.class == 15)
			{
				ent->s.frame = 0;
				ent->client->anim_end = 29;
			}
			if (ent->client->resp.class == 26)
			{
				ent->s.frame = 0;
				ent->client->anim_end = 29;
			}
			if (ent->client->resp.class == 16)
			{
				ent->s.frame = 144;
				ent->client->anim_end = 247;
			}
			if (ent->client->resp.class == 17)
			{
				ent->s.frame = 65;
				ent->client->anim_end = 93;
			}
			if (ent->client->resp.class == 18)
			{
				ent->s.frame = 112;
				ent->client->anim_end = 162;
			}
			if (ent->client->resp.class == 19)
			{
				ent->s.frame = 213;
				ent->client->anim_end = 272;
			}
			if (ent->client->resp.class == 30)
			{
				ent->s.frame = 0;
				ent->client->anim_end = 0;
			}
			if (ent->client->resp.class == 31)
			{
				ent->s.frame = 0;
				ent->client->anim_end = 0;
			}
			if (ent->client->resp.class == 25)
			{
				ent->s.frame = 0;
				ent->client->anim_end = 38;
			}
			if (ent->client->resp.class == 27)
			{
				ent->s.frame = 0;
				ent->client->anim_end = 33;
			}
			if (ent->client->resp.class == 29)
			{
				ent->s.frame = 0;
				ent->client->anim_end = 12;
			}
			if (ent->client->resp.class == 37)
			{
				ent->s.frame = 0;
				ent->client->anim_end = 39;
			}
			if (ent->client->resp.class == 38)
			{
				ent->s.frame = 0;
				ent->client->anim_end = 39;
			}
	if (walk)
		{
			if (ent->client->resp.class == 1)
			{
				ent->s.frame = 250;
				ent->client->anim_end = 261;
			}
			if (ent->client->resp.class == 36)
			{
				ent->s.frame = 250;
				ent->client->anim_end = 261;
			}
			if (ent->client->resp.class == 20)
			{
				ent->s.frame = 250;
				ent->client->anim_end = 261;
			}
			if (ent->client->resp.class == 2)
			{
				ent->s.frame = 74;
				ent->client->anim_end = 85;
			}
			if (ent->client->resp.class == 3)
			{
				ent->s.frame = 34;
				ent->client->anim_end = 49;
			}
			if (ent->client->resp.class == 33)
			{
				ent->s.frame = 34;
				ent->client->anim_end = 49;
			}
			if (ent->client->resp.class == 4)
			{
				ent->s.frame = 0;
				ent->client->anim_end = 11;
			}
			if (ent->client->resp.class == 28)
			{
				ent->s.frame = 0;
				ent->client->anim_end = 11;
			}
			if (ent->client->resp.class == 5)
			{
				ent->s.frame = 76;
				ent->client->anim_end = 88;
			}
			if (ent->client->resp.class == 6)
			{
				ent->s.frame = 7;
				ent->client->anim_end = 22;
			}
			if (ent->client->resp.class == 34)
			{
				ent->s.frame = 7;
				ent->client->anim_end = 22;
			}
			if (ent->client->resp.class == 21)
			{
				ent->s.frame = 7;
				ent->client->anim_end = 22;
			}
			if (ent->client->resp.class == 7)
			{
				ent->s.frame = 128;
				ent->client->anim_end = 145;
			}
			if (ent->client->resp.class == 39)
			{
				ent->s.frame = 128;
				ent->client->anim_end = 145;
			}
			if (ent->client->resp.class == 24)
			{
				ent->s.frame = 128;
				ent->client->anim_end = 145;
			}
			if (ent->client->resp.class == 8)
			{
				ent->s.frame = 13;
				ent->client->anim_end = 57;
			}
			if (ent->client->resp.class == 35)
			{
				ent->s.frame = 13;
				ent->client->anim_end = 57;
			}
			if (ent->client->resp.class == 32)
			{
				ent->s.frame = 121;
				ent->client->anim_end = 139;
			}
			if (ent->client->resp.class == 9)
			{
				ent->s.frame = 25;
				ent->client->anim_end = 35;
			}
			if (ent->client->resp.class == 10)
			{
				ent->s.frame = 187;
				ent->client->anim_end = 198;
			}
			if (ent->client->resp.class == 23)
			{
				ent->s.frame = 187;
				ent->client->anim_end = 198;
			}
			if (ent->client->resp.class == 11)
			{
				ent->s.frame = 0;
				ent->client->anim_end = 49;
			}
			if (ent->client->resp.class == 12)
			{
				ent->s.frame = 0;
				ent->client->anim_end = 12;
			}
			if (ent->client->resp.class == 22)
			{
				ent->s.frame = 0;
				ent->client->anim_end = 12;
			}
			if (ent->client->resp.class == 13)
			{
				ent->s.frame = 130;
				ent->client->anim_end = 141;
			}
			if (ent->client->resp.class == 14)
			{
				ent->s.frame = 34;
				ent->client->anim_end = 49;
			}
			if (ent->client->resp.class == 41)
			{
				ent->s.frame = 34;
				ent->client->anim_end = 49;
			}
			if (ent->client->resp.class == 40)
			{
				ent->s.frame = 68;
				ent->client->anim_end = 77;
			}
			if (ent->client->resp.class == 15)
			{
				ent->s.frame = 30;
				ent->client->anim_end = 64;
			}
			if (ent->client->resp.class == 26)
			{
				ent->s.frame = 30;
				ent->client->anim_end = 64;
			}
			if (ent->client->resp.class == 16)
			{
				ent->s.frame = 144;
				ent->client->anim_end = 247;
			}
			if (ent->client->resp.class == 17)
			{
				ent->s.frame = 41;
				ent->client->anim_end = 64;
			}
			if (ent->client->resp.class == 18)
			{
				ent->s.frame = 163;
				ent->client->anim_end = 175;
			}
			if (ent->client->resp.class == 19)
			{
				ent->s.frame = 274;
				ent->client->anim_end = 283;
			}
			if (ent->client->resp.class == 30)
			{
				ent->s.frame = 11;
				ent->client->anim_end = 23;
			}
			if (ent->client->resp.class == 31)
			{
				ent->s.frame = 1;
				ent->client->anim_end = 9;
			}
			if (ent->client->resp.class == 25)
			{
				ent->s.frame = 39;
				ent->client->anim_end = 44;
			}
			if (ent->client->resp.class == 27)
			{
				ent->s.frame = 49;
				ent->client->anim_end = 52;
			}
			if (ent->client->resp.class == 29)
			{
				ent->s.frame = 0;
				ent->client->anim_end = 12;
			}
			if (ent->client->resp.class == 37)
			{
				ent->s.frame = 40;
				ent->client->anim_end = 45;
			}
			if (ent->client->resp.class == 38)
			{
				ent->s.frame = 40;
				ent->client->anim_end = 45;
			}
		//}
		}
	}
}

void hre (edict_t *whoo)
{
}
/*
=================
ClientEndServerFrame

Called for each player at the end of the server frame
and right after spawning
=================
*/
void ClientEndServerFrame (edict_t *ent)
{
	float	bobtime;
	int		i;

	current_player = ent;
	current_client = ent->client;

	//
	// If the origin or velocity have changed since ClientThink(),
	// update the pmove values.  This will happen when the client
	// is pushed by a bmodel or kicked by an explosion.
	// 
	// If it wasn't updated here, the view position would lag a frame
	// behind the body position when pushed -- "sinking into plats"
	//
	for (i=0 ; i<3 ; i++)
	{
		current_client->ps.pmove.origin[i] = ent->s.origin[i]*8.0;
		current_client->ps.pmove.velocity[i] = ent->velocity[i]*8.0;
	}

	//
	// If the end of unit layout is displayed, don't give
	// the player any normal movement attributes
	//
	if (level.intermissiontime)
	{
		// FIXME: add view drifting here?
		current_client->ps.blend[3] = 0;
		current_client->ps.fov = 90;
		G_SetStats (ent);
		return;
	}

	AngleVectors (ent->client->v_angle, forward, right, up);

	// burn from lava, etc
	P_WorldEffects ();

	//
	// set model angles from view angles so other things in
	// the world can tell which direction you are looking
	//
	if (ent->client->v_angle[PITCH] > 180)
		ent->s.angles[PITCH] = (-360 + ent->client->v_angle[PITCH])/3;
	else
		ent->s.angles[PITCH] = ent->client->v_angle[PITCH]/3;
	ent->s.angles[YAW] = ent->client->v_angle[YAW];
	ent->s.angles[ROLL] = 0;
	ent->s.angles[ROLL] = SV_CalcRoll (ent->s.angles, ent->velocity)*4;

	//
	// calculate speed and cycle to be used for
	// all cyclic walking effects
	//
	xyspeed = sqrt(ent->velocity[0]*ent->velocity[0] + ent->velocity[1]*ent->velocity[1]);

	if (xyspeed < 5)
	{
		bobmove = 0;
		current_client->bobtime = 0;	// start at beginning of cycle again
	}
	else if (ent->groundentity)
	{	// so bobbing only cycles when on ground
		if (xyspeed > 210)
			bobmove = 0.25;
		else if (xyspeed > 100)
			bobmove = 0.125;
		else
			bobmove = 0.0625;
	}
	
	bobtime = (current_client->bobtime += bobmove);

//	if (current_client->ps.pmove.pm_flags & PMF_DUCKED)
//		bobtime *= 4;

	bobcycle = (int)bobtime;
	bobfracsin = fabs(sin(bobtime*M_PI));

	// detect hitting the floor
	P_FallingDamage (ent);

	// apply all the damage taken this frame
	P_DamageFeedback (ent);

	// determine the view offsets
	SV_CalcViewOffset (ent);

	// determine the gun offsets
	SV_CalcGunOffset (ent);

	// determine the full screen color blend
	// must be after viewoffset, so eye contents can be
	// accurately determined
	// FIXME: with client prediction, the contents
	// should be determined by the client
	SV_CalcBlend (ent);

	// chase cam stuff
	if (ent->client->resp.spectator)
		G_SetSpectatorStats(ent);
	else
	{
	//ZOID
	if (!ent->client->chase_target)
	//ZOID
		G_SetStats (ent);
	}

//ZOID
//update chasecam follower stats
	for (i = 1; i <= maxclients->value; i++) {
		edict_t *e = g_edicts + i;
		if (!e->inuse || e->client->chase_target != ent)
			continue;
		memcpy(e->client->ps.stats, 
			ent->client->ps.stats, 
			sizeof(ent->client->ps.stats));
		e->client->ps.stats[STAT_LAYOUTS] = 1;
		break;
	}
//ZOID

	G_CheckChaseStats(ent);

	G_SetClientEvent (ent);

	G_SetClientEffects (ent);

	G_SetClientSound (ent);

	G_SetClientFrame (ent);

	VectorCopy (ent->velocity, ent->client->oldvelocity);
	VectorCopy (ent->client->ps.viewangles, ent->client->oldviewangles);

	// clear weapon kicks
	VectorClear (ent->client->kick_origin);
	VectorClear (ent->client->kick_angles);

	// if the scoreboard is up, update it
	if (ent->client->showscores && !(level.framenum & 31) )
	{
//ZOID
		if (ent->client->menu) {
			PMenuDoUpdate(ent);
			ent->client->menudirty = false;
			ent->client->menutime = level.time;
		} else
//ZOID
		DeathmatchScoreboardMessage (ent, ent->enemy);
		gi.unicast (ent, false);
	}


//Wyrm: chasecam
        if (ent->client->chasetoggle)
                CheckChasecam_Viewent(ent);

		// Update this player's current_party every frame.
		// FIXME: Too computer-eating?
		if (ent->client->resp.in_party)
		{
			gclient_t *client = InParty(ent);
			ent->client->resp.current_party = client->resp.party;
		}


	// Alright, let's stick powerpoint stuff here
	if (!ent->client->pers.abilities.soldier_haste || !ent->client->pers.abilities.tank_dualrockets || !ent->client->pers.abilities.gunner_forcegrenades || !ent->client->pers.abilities.gladiator_wallpierce || !ent->client->pers.abilities.floater_fragmulti || !ent->client->pers.abilities.berserk_shield || !ent->client->pers.abilities.chick_triplespread || ent->client->pers.abilities.icarus_explobolts || !ent->client->pers.abilities.soldierb_speed || !ent->client->pers.abilities.bbrain_tentacles || !ent->client->pers.abilities.dchick_plasma || ent->client->pers.abilities.daed_ir || !ent->client->pers.abilities.stalker_homing || !ent->client->pers.abilities.flyer_kaboom)
	{
		if (ent->client->pers.abilities.powerpoints > 499)
			ent->client->pers.abilities.powerpoints = 499;
		ent->client->pers.abilities.powerpoints+= 1;
	} // Add powerpoints when not taking away.

	if (ent->client->pers.abilities.soldier_haste || ent->client->pers.abilities.tank_dualrockets || ent->client->pers.abilities.gunner_forcegrenades || ent->client->pers.abilities.gladiator_wallpierce || ent->client->pers.abilities.floater_fragmulti || ent->client->pers.abilities.berserk_shield || ent->client->pers.abilities.chick_triplespread || ent->client->pers.abilities.icarus_explobolts || ent->client->pers.abilities.soldierb_speed || ent->client->pers.abilities.bbrain_tentacles || ent->client->pers.abilities.dchick_plasma || ent->client->pers.abilities.daed_ir || ent->client->pers.abilities.stalker_homing || ent->client->pers.abilities.flyer_kaboom)
	{
		if (ent->client->pers.abilities.powerpoints < 1)
		{
			ent->client->pers.abilities.soldier_haste = 0;
			ent->client->pers.abilities.tank_dualrockets = 0;
			ent->client->pers.abilities.gunner_forcegrenades = 0;
			ent->client->pers.abilities.gladiator_wallpierce = 0;
			ent->client->pers.abilities.floater_fragmulti = 0;
			ent->client->pers.abilities.berserk_shield = 0;
			ent->client->pers.abilities.chick_triplespread = 0;
			ent->client->pers.abilities.icarus_explobolts = 0;
			ent->client->pers.abilities.soldierb_speed = 0;
			ent->client->pers.abilities.bbrain_tentacles = 0;
			ent->client->pers.abilities.dchick_plasma = 0;
			ent->client->pers.abilities.daed_ir = 0;
			ent->client->pers.abilities.stalker_homing = 0;
			ent->client->pers.abilities.flyer_kaboom = 0;
			ent->client->pers.abilities.powerpoints = 0;
			ent->client->ps.rdflags &= ~RDF_IRGOGGLES;
			ent->svflags = 0;
			gi.cvar_forceset("gl_saturatelighting","0"); 
			gi.cvar_forceset("r_fullbright","0"); 
			ent->s.effects |= 0;
			ent->s.renderfx |= RF_IR_VISIBLE;
			ent->s.renderfx |= RF_FRAMELERP;
			safe_cprintf (ent, PRINT_HIGH, "Ran out of points!\n");
			return;
		}
		ent->client->pers.abilities.powerpoints -= 2;
	}
}

