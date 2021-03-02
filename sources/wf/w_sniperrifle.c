#include "g_local.h"

void fire_bullet (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int mod);
int srkick = 8;
void lasersight_on (edict_t *ent);
void lasersight_off (edict_t *ent);
/*
========================
Sniper Rifle
========================

G.A.R 10/22/98  -Removed laser sight code from here. Put in separate file

*/

#define FIRE_FRAME_1	4
#define FIRE_FRAME_2	7

void weapon_snipe_fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
//	vec3_t		end;

	if (!ent->client) return;

//if (wfdebug) gi.dprintf("Snipe first start, frame = %d\n",ent->client->ps.gunframe);

	//Should laser sight be on?
	if ((ent->client->pers.laseron) && (!ent->lasersight) && (ent->client->ps.gunframe == FIRE_FRAME_1))
	{
		lasersight_on (ent);
	}
	
	if (!ent->PlayerSnipingZoom)
		ent->PlayerSnipingZoom =35;
	if (ent->client->ps.gunframe == FIRE_FRAME_2)
	{
		if (ent->client->pers.autozoom)
			ent->client->ps.fov = 90;
		ent->client->ps.gunframe++;
		return;
	}

	if (ent->SniperDamage == 0)
	{
		if (ent->client->pers.autozoom)
			ent->client->ps.fov = ent->PlayerSnipingZoom; 
		ent->SniperDamage = 20;
		//if (ent->client->ps.gunframe == FIRE_FRAME_2)
		ent->client->weapon_damage = ent->SniperDamage;

		if(!ent->client->pers.fastaim)
			ent->superslow=1;

		return;
	}
	if (ent->client->buttons & BUTTON_ATTACK)
	{
		if (ent->client->pers.autozoom)
			ent->client->ps.fov = ent->PlayerSnipingZoom;
		if (ent->SniperDamage == 1) ent->SniperDamage = 0;
//		ent->SniperDamage += 10;
		ent->SniperDamage += 5;
		if(!ent->client->pers.fastaim)
		{
			ent->SniperDamage += 8;//was 6
			//Change max damage now that there is location damage.
			if (ent->SniperDamage > wf_game.weapon_damage[WEAPON_SNIPERRIFLE])
				ent->SniperDamage= wf_game.weapon_damage[WEAPON_SNIPERRIFLE];
		}
		else
		{
			//Change max damage now that there is location damage.
			ent->SniperDamage += 20;
			if (ent->SniperDamage>110)
				ent->SniperDamage= 110;
		}


		if (level.framenum & 3)
		{
			//gi.dprintf("N ");
		}
		else
		{
			//gi.dprintf("Y ");
			ent->client->weapon_damage = ent->SniperDamage;
		}
		srkick +=2;
		if (srkick>60)
			srkick = 60;
		return;
	}
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorScale (forward, -3, ent->client->kick_origin);

	ent->client->kick_angles[0] = -7;//Gregg - I added some more kick
	VectorSet(offset, 0, 7,  ent->viewheight-8);

	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	if (is_quad)
	{
		ent->SniperDamage *= 4;
		if ((int)level.time & 3)
			ent->client->weapon_damage = ent->SniperDamage;
		srkick *= 4;
	}
	
//	fire_bullet (ent, start, forward, ent->SniperDamage, srkick, 0, 0, MOD_SNIPERRIFLE);
	fire_rail   (ent, start, forward, ent->SniperDamage, srkick, MOD_SNIPERRIFLE);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_SHOTGUN);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
	ent->SniperDamage =0;
	ent->client->weapon_damage = ent->SniperDamage;
	srkick = 8;
   ent->superslow=0;

//if (wfdebug) gi.dprintf("Snipe laser off\n");
   lasersight_off (ent);
}

void Weapon_Snipe (edict_t *ent)
{
//	static int	pause_frames[]	= {22, 28, 34, 0};
//	static int	fire_frames[]	= { 8, 9, 0};
//
//	Weapon_Generic (ent, 7, 18, 36, 39, pause_frames, fire_frames, weapon_snipe_fire);

/*
	static int	pause_frames[]	= {16, 22, 28, 0};
	static int	fire_frames[]	= {8, 9, 0};

	Weapon_Generic (ent, 7, 13, 30, 33, pause_frames, fire_frames, weapon_snipe_fire);
*/

	static int	pause_frames[]	= {16, 28, 35, 0};
	static int	fire_frames[]	= {FIRE_FRAME_1, FIRE_FRAME_2, 0};

	Weapon_Generic (ent, 3, 12, 50, 55, pause_frames, fire_frames, weapon_snipe_fire);


}

