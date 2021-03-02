#include    "g_local.h"
#include    "m_player.h"
#include    "g_brrr.h"

#define DEBRIS1_MODEL "models/objects/debris1/tris.md2"
#define DEBRIS2_MODEL "models/objects/debris2/tris.md2"
#define DEBRIS3_MODEL "models/objects/debris3/tris.md2"

#define CONVERT_SHELLS      1
#define CONVERT_BULLETS     2
#define CONVERT_GRENADES    3
#define CONVERT_ROCKETS     4
#define CONVERT_CELLS       5
#define CONVERT_SLUGS       6

#define RATE_SHELLS         1
#define RATE_BULLETS        3
#define RATE_GRENADES       8
#define RATE_ROCKETS        13
#define RATE_CELLS          4
#define RATE_SLUGS          16

#define USER_SHELLS         ent->client->pers.inventory[ITEM_INDEX(FindItem("Shells"))]
#define USER_BULLETS        ent->client->pers.inventory[ITEM_INDEX(FindItem("Bullets"))]
#define USER_GRENADES       ent->client->pers.inventory[ITEM_INDEX(FindItem("Grenades"))]
#define USER_ROCKETS        ent->client->pers.inventory[ITEM_INDEX(FindItem("Rockets"))]
#define USER_CELLS          ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))]
#define USER_SLUGS          ent->client->pers.inventory[ITEM_INDEX(FindItem("Slugs"))]

// Prototypes
edict_t     *TossClientWeapon(edict_t *self);
qboolean    IsFemale(edict_t *ent);
qboolean    Jet_Active(edict_t *ent);
int         PlayerSort (void const *a, void const *b);

/* *** DM PAUSE *** */
qboolean    dmpaused;
edict_t     *dmpauser;
/* *** */

/* *** SNIPER ZOOM *** */
cvar_t      *m_pitch=NULL;
cvar_t      *m_yaw=NULL;
float       m_pitch_default;
float       m_yaw_default;
/* *** */

/* *** ADDED ITEMS *** */
newitem_t   *newitem_head;
qboolean    erasing;
cvar_t      *itemcompat;
/* *** */

/* *** X-RADIO *** */
cvar_t      *radio_power;
/* *** */

/* *** ALLOW FLAGS *** */
cvar_t      *ann_allow;
cvar_t      *ann_allow2;
/* *** */

/* *** RESPAWN PROTECTION *** */
cvar_t      *respawn_protect;
/* *** */

/* *** MAP ROTATION *** */
maplist_t   maplist;
cvar_t      *rotate;
cvar_t      *rotate_filename;
/* *** */

/* *** LIMITS *** */
cvar_t		*laser_limit;
cvar_t		*prox_limit;
cvar_t		*pipe_limit;
/* *** */

/* Funky Functions */
qboolean fileexists(char *filename)
{
    FILE *fp;
    
    fp=fopen(filename,"rb");
    if (fp)
        fclose(fp);
    
    return (fp!=NULL);
}

void trimstring(char *buffer)
{
    int i;

    if (buffer[0]=='\0')
        return;

    while (buffer[0]==' ')
    {
        buffer=buffer+1;
    }

    i=strlen(buffer)-1;

    while (buffer[i]==' ')
    {
        buffer[i--]='\0';
    }

    return;
}

/* *** HOMING MISSILES *** */
void homing_think (edict_t *ent)
{
    home_ent(ent,5);
}

void home_ent (edict_t *ent, int count)
{
    edict_t *target = NULL;
    edict_t *blip = NULL;
    vec3_t  targetdir, blipdir;
    vec_t   speed;
    int     n;

    while ((blip = findradius(blip, ent->s.origin, 1000)) != NULL)
    {
        if (!(blip->svflags & SVF_MONSTER) && !blip->client)
            continue;
        if (blip == ent->owner)
            continue;
        if (!blip->takedamage)
            continue;
        if (blip->health <= 0)
            continue;
        /* *** CTF COMPATIBILITY *** */
          if (blip->client && (int) ctf->value && ent->owner->client->resp.ctf_team==blip->client->resp.ctf_team)
            continue;
        /* *** */
        if (blip->client && OnSameTeam(ent->owner,blip))
            continue;
        if (!visible(ent, blip))
            continue;
//      if (!infront(ent, blip))    /* *** TOM: No, it works better without
//          continue;                  *** */

        for (n=0;n<count;n++)
            VectorSubtract(blip->s.origin, ent->s.origin, blipdir);

        blipdir[2] += 16;
        if ((target == NULL) || (VectorLength(blipdir) < VectorLength(targetdir)))
        {
            target = blip;
            VectorCopy(blipdir, targetdir);
        }
    }

    if (target != NULL)
    {
        // target acquired, nudge our direction toward it
        VectorNormalize(targetdir);
        VectorScale(targetdir, 0.2, targetdir);
        VectorAdd(targetdir, ent->movedir, targetdir);
        VectorNormalize(targetdir);
        VectorCopy(targetdir, ent->movedir);
        vectoangles(targetdir, ent->s.angles);
        speed = VectorLength(ent->velocity);
        VectorScale(targetdir, speed, ent->velocity);
    }

    ent->nextthink = level.time + 0.1;

    if (Q_stricmp(ent->classname,"drbolt")==0)
    {
        ent->tomvar++;
        if (ent->tomvar==DISRUPTOR_STAYALIVE*10)
            G_FreeEdict(ent);
    }
}

/* *** KAMIKAZE MODE *** */
/*
Function: Start_Kamikaze_Mode

  Places the edict passed to it into Kamikaze Mode
  (probably best to pass a player to it)

Warns everyone that so and so is a kamikaze...
*/
void Start_Kamikaze_Mode(edict_t *the_doomed_one){
    /* see if we are already in  kamikaze mode*/
    if (the_doomed_one->client->kamikaze_mode & 1)  {
        gi.cprintf(the_doomed_one, PRINT_MEDIUM, "Too late to deactivate now!\n");
        return;
    }

    if (!((unsigned long)ann_allow2->value&ANN_ALLOW2_KAMIKAZE))
    {
        gi.cprintf(the_doomed_one, PRINT_HIGH, "Kamikaze mode is disallowed on this server!\n");
        return;
    }

    /* dont run if in god mode  */
    if (the_doomed_one->flags & FL_GODMODE){
        gi.cprintf(the_doomed_one, PRINT_MEDIUM, "Can't kamikaze in God mode, dumbass!\n");
        return;
    }

    if (the_doomed_one->health<KAMIKAZE_HEALTH)
    {
        gi.cprintf(the_doomed_one, PRINT_MEDIUM, "You must have at least %d%% health!\n",KAMIKAZE_HEALTH);
        return;
    }

    if (the_doomed_one->client->pers.inventory[ITEM_INDEX(FindItem("Rockets"))]<KAMIKAZE_ROCKETS)
    {
        gi.cprintf(the_doomed_one, PRINT_MEDIUM, "You must have at least %d rockets!\n",KAMIKAZE_ROCKETS);
        return;
    }

    the_doomed_one->client->pers.inventory[ITEM_INDEX(FindItem("Rockets"))]-=KAMIKAZE_ROCKETS;

    /* not in kamikaze mode yet */
    the_doomed_one->client->kamikaze_mode = 1;

    /*  Give us only so long */
    the_doomed_one->client->kamikaze_timeleft = (KAMIKAZE_BLOW_TIME-1)*10;
    the_doomed_one->client->kamikaze_framenum = level.framenum + the_doomed_one->client->kamikaze_timeleft;

    /* Warn the World */
    gi.bprintf (PRINT_MEDIUM,"%s is a Kamikaze - BANZAI!\n", the_doomed_one->client->pers.netname);
    gi.sound( the_doomed_one, CHAN_WEAPON, gi.soundindex("makron/rail_up.wav"), 1, ATTN_NONE, 0 );

    return;
}

/*
function: Kamikaze_Active
    Are we in Kamikaze Mode?
    a helper function to see if we are running in Kamikaze Mode
*/
qboolean Kamikaze_Active(edict_t *the_doomed_one){
    return (the_doomed_one->client->kamikaze_mode);
}

/*
Function: Kamikaze_Cancel
  Canceled for Some Reason
  Call if Player is killed before time is up
*/
void Kamikaze_Cancel(edict_t *the_spared_one){
    /* not in kamikaze mode yet */
    the_spared_one->client->kamikaze_mode = 0;
    /* Give us only so long */
    the_spared_one->client->kamikaze_timeleft = 0;
    the_spared_one->client->kamikaze_framenum = 0;

    return;
}

void Kamikaze_Explode(edict_t *the_doomed_one){
    /* A whole Lotta Damage */
    T_RadiusDamage (the_doomed_one, the_doomed_one, KAMIKAZE_DAMAGE, NULL, KAMIKAZE_DAMAGE_RADIUS, MOD_KAMIKAZE);

    /* BANG! and show the clients */
    gi.WriteByte (svc_temp_entity);
    gi.WriteByte (TE_EXPLOSION1);
    gi.WritePosition(the_doomed_one -> s.origin);
    gi.multicast (the_doomed_one->s.origin, MULTICAST_PVS);
}

/* *** FAKE DEATH *** */
void player_fake_die(edict_t *self)
{
    static int  i;
    edict_t     *temp;
    int         count;

	/* *** GUIDED MISSILES *** */
	if (self->client->rocketview)
		return;
	/* *** */

	/* *** FREEZETHROWER *** */
	if (self->client->frozen)
		return;
	/* *** */

    if (!self->client->flg_faking)      // If not faking death
    {
        // Don't even go there!
        if (self->deadflag!=DEAD_NO)
            return;

        /* *** TORTURE CHAMBER *** */
        if (self->client->inchamber)
            return;
        /* *** */


        if (!((unsigned long)ann_allow2->value&ANN_ALLOW2_FAKEDEATH))
        {
            gi.cprintf(self, PRINT_HIGH, "Faking death is disallowed on this server!\n");
            return;
        }

        // Fake death
        VectorClear (self->avelocity);

        self->takedamage = DAMAGE_YES;
        self->movetype = MOVETYPE_TOSS;

        self->s.modelindex2 = 0;    // remove linked weapon model
        self->s.modelindex3 = 0;    // remove linked ctf flag

        self->s.angles[0] = 0;
        self->s.angles[2] = 0;

        self->s.sound = 0;
        self->client->weapon_sound = 0;

        self->maxs[2] = -8;

        count=self->client->pers.inventory[self->client->ammo_index];
        temp=TossClientWeapon(self);

        if (temp && self->client->ammo_index)
        {
            temp->count = count;
            self->client->pers.inventory[self->client->ammo_index]=0;
            temp->tomvar=1;
        }

		self->client->newweapon = NULL;
		ChangeWeapon (self);

        i = (i+1)%3;
        // start a death animation
        self->client->anim_priority = ANIM_DEATH;
        if (self->client->ps.pmove.pm_flags & PMF_DUCKED)
        {
            self->s.frame = FRAME_crdeath1-1;
            self->client->anim_end = FRAME_crdeath5;
        }
        else
        {
            switch (i)
            {
                case 0:
                    self->s.frame = FRAME_death101-1;
                    self->client->anim_end = FRAME_death106;
                    break;
                case 1:
                    self->s.frame = FRAME_death201-1;
                    self->client->anim_end = FRAME_death206;
                    break;
                case 2:
                    self->s.frame = FRAME_death301-1;
                    self->client->anim_end = FRAME_death308;
                    break;
            }
            gi.sound (self, CHAN_VOICE, gi.soundindex(va("*death%i.wav", (rand()%4)+1)), 1, ATTN_NORM, 0);
        }

        self->deadflag = DEAD_DEAD;

		self->client->pers.inventory[ITEM_INDEX(self->client->pers.lastweapon)]=0;
		self->client->pers.inventory[ITEM_INDEX(FindItem("Light Saber"))]=1;	// Make sure they still have a light saber =]
		self->client->pers.inventory[ITEM_INDEX(FindItem("Grapple"))]=1;	// Make sure they still have a grapple =]
		CTFPlayerResetGrapple(self);
        CTFDeadDropFlag(self);
        CTFDeadDropTech(self);

        self->client->flg_faking=1;
    }
    else
    {
        // Stand up again
        self->s.frame=FRAME_stand01;
        self->client->anim_end=FRAME_stand40;

        // Reset flags
        self->deadflag=DEAD_NO;
        self->client->flg_faking=0;

		self->client->newweapon=FindItem("Blaster");
   		ChangeWeapon (self);
    }
}

/* *** ZOOM IN *** */
void ZoomIn(edict_t *ent)
{
    if (ent->client->ps.fov>=30)
        ent->client->ps.fov-=10;
}

/* *** ZOOM OUT *** */
void ZoomOut(edict_t *ent)
{
    if (ent->client->ps.fov<90)
    {
        ent->client->ps.fov+=10;
        if (ent->client->ps.fov>90)
            ent->client->ps.fov=90;
    }
}

/* *** CLUSTER GRENADES *** */
void Cluster_Explode (edict_t *ent)
{
    vec3_t  origin;
    int     mod;
    vec3_t  grenade1,grenade2,grenade3,grenade4;

    if (ent->owner->client)
        PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

    //FIXME: if we are onground then raise our Z just a bit since we are a point?
    if (ent->enemy)
    {
        float   points;
        vec3_t  v;
        vec3_t  dir;

        VectorAdd (ent->enemy->mins, ent->enemy->maxs, v);
        VectorMA (ent->enemy->s.origin, 0.5, v, v);
        VectorSubtract (ent->s.origin, v, v);
        points = ent->dmg - 0.5 * VectorLength (v);
        VectorSubtract (ent->enemy->s.origin, ent->s.origin, dir);

        mod = MOD_CLUSTER;

        T_Damage (ent->enemy, ent, ent->owner, dir, ent->s.origin, vec3_origin, (int)points, (int)points, DAMAGE_RADIUS, mod);
    }

    mod = MOD_CLUSTER;

    T_RadiusDamage(ent, ent->owner, ent->dmg, ent->enemy, ent->dmg_radius, mod);

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
    gi.multicast (ent->s.origin, MULTICAST_PHS);

    // Tom: Give grenades up/out vectors
    VectorSet(grenade1,20,20,40);
    VectorSet(grenade2,20,-20,40);
    VectorSet(grenade3,-20,20,40);
    VectorSet(grenade4,-20,-20,40);

    // Tom: damage = 40  +/- 20
    //      speed  = 10  +/- 3
    //      timer  = 1   +/- 0.3
    //      radius = 120 +/- 30
    fire_grenade2(ent,origin,grenade1,40+(((float)rand()/(float)RAND_MAX)*40)-20,10+(((float)rand()/(float)RAND_MAX)*6)-3,1.0+(((float)rand()/(float)RAND_MAX)*0.6)-0.3,120+(((float)rand()/(float)RAND_MAX)*60)-30,2);
    fire_grenade2(ent,origin,grenade2,40+(((float)rand()/(float)RAND_MAX)*40)-20,10+(((float)rand()/(float)RAND_MAX)*6)-3,1.0+(((float)rand()/(float)RAND_MAX)*0.6)-0.3,120+(((float)rand()/(float)RAND_MAX)*60)-30,2);
    fire_grenade2(ent,origin,grenade3,40+(((float)rand()/(float)RAND_MAX)*40)-20,10+(((float)rand()/(float)RAND_MAX)*6)-3,1.0+(((float)rand()/(float)RAND_MAX)*0.6)-0.3,120+(((float)rand()/(float)RAND_MAX)*60)-30,2);
    fire_grenade2(ent,origin,grenade4,40+(((float)rand()/(float)RAND_MAX)*40)-20,10+(((float)rand()/(float)RAND_MAX)*6)-3,1.0+(((float)rand()/(float)RAND_MAX)*0.6)-0.3,120+(((float)rand()/(float)RAND_MAX)*60)-30,2);

    G_FreeEdict (ent);
}

/* *** BLEEDING *** */
void BleedNow(edict_t *self)
{
    // use some local vector variables to work with
    vec3_t  up;
    vec3_t  mouth_pos, spew_vector;

    // set the spew vector, based on the client's view angle
    AngleVectors (self->client->v_angle, NULL, NULL, up);

    // Make the blood originate from our waist
    VectorScale (up, (((float)rand()/(float)RAND_MAX)*48)-12, mouth_pos);
    VectorAdd (mouth_pos, self->s.origin, mouth_pos);
    mouth_pos[2] += ((float)rand()/(float)RAND_MAX);

    // Make the blood go everywhere, in an upward direction!
    VectorScale (up, 0, spew_vector);

    // BLOOD!
    gi.WriteByte (svc_temp_entity);
    gi.WriteByte (TE_BLOOD);
    gi.WritePosition (mouth_pos);
    gi.WriteDir (spew_vector);
    gi.multicast (mouth_pos, MULTICAST_PVS);
}

/* *** X-RADIO *** */
void X_Radio_f(edict_t *self, char channel, char *msg, char *person)
{
    edict_t *player;
    int     i;
    char    cmd[80], temp[10], *pos;
    cvar_t  *temp1;

     if((pos = strstr(msg,";"))!=0)              // Check if msg contains ';'
        pos[0]='\0';                            // If so, terminate string there

    if (strlen(msg)==0)
    {
        gi.cprintf(self,PRINT_HIGH,"Error - no sample specified\n");
        return;
    }

    temp1=gi.cvar("gamedir","",0);
    if (strlen(temp1->string)>0)
        sprintf(cmd,"./%s/sound/radio/%s.wav",temp1->string,msg);
    else
        sprintf(cmd,"./sound/radio/%s.wav",msg);
    if (!fileexists(cmd))   // Check file exists
    {
        gi.cprintf(self,PRINT_HIGH,"Error - cannot find specified sample [%s]\n",strupr(msg));
        return;
    }

    if (channel == 1)
        strcpy(temp,"Global");
    else if (channel == 2)
        strcpy(temp,"Team");
    else if (channel == 3)
        strcpy(temp,"Private");
    sprintf(cmd, "echo %s message from %.16s - [%.20s]...;play radio/%s\n",temp,self->client->pers.netname,strupr(msg),msg);   // Build command to send
    if (channel == 1)       // To all players
    {
        for_each_player(player, i)
        {
            if (player->client->resp.radio_power==1)
                x_stuffcmd(player, cmd);
        }
    }
    else if (channel == 2)  // To team members only
    {
        if (ctf->value)
        {
            for_each_player(player, i)
            {
                if (player->client->resp.ctf_team == self->client->resp.ctf_team)
                    if (player->client->resp.radio_power>0)
                        x_stuffcmd(player, cmd);
            }
        }
        else
        {
            for_each_player(player, i)
            {
                if (OnSameTeam(player,self))
                    if (player->client->resp.radio_power>0)
                        x_stuffcmd(player, cmd);
            }
        }
    }
    else if (channel == 3)  // To individual player
    {
        if (strlen(person)==0)
        {
            gi.cprintf(self,PRINT_HIGH,"Error - no player specified\n");
            return;
        }
        player=ent_by_name(person);
        if (!player)
        {
            gi.cprintf(self,PRINT_HIGH,"\n%s not found on this server.\nRemember, player names are case-sensitive.\n",person);
        }
        else if (player->client->resp.radio_power==1)
        {
            x_stuffcmd(player, cmd);
        }
    }
}

/* *** REXEC *** */
void RExec(edict_t *self, char type, char *rcmd, char *person)
{
    edict_t *player,*player1;
    int     i,j,k;
    char    *temp;
	int     index[256];
    int     count;
    int     personid;

    if (strlen(rcmd)==0)
    {
        gi.cprintf(self,PRINT_HIGH,"Error - no command specified\n");
        return;
    }

    count=0;
	for (i = 0 ; i < maxclients->value ; i++)
    {
		if (game.clients[i].pers.connected)
		{
			index[count] = i;
			count++;
		}
    }

    // Change #s to ;s in the remote command string
     while ((temp=strstr(rcmd,"#"))!=0)
        temp[0]=';';

    if (type == 1)          // To all players
    {
        for_each_player(player, i)
        {
            if (player!=self)
                x_stuffcmd(player, rcmd);
        }

        x_stuffcmd(self, rcmd);   // Do self last so "rexecall quit" works, for example
    }
	else if (type == 2)     // To everyone except this client
    {
        for_each_player(player, i)
        {
            if (player!=self)
                x_stuffcmd(player, rcmd);
        }
    }
    else if (type == 3)     // To individual player
    {
        if (strlen(person)==0)
        {
            gi.cprintf(self,PRINT_HIGH,"Error - no player specified\n");
            return;
        }

        personid=atoi(person);

        k=0;
        for_each_player(player, i)
        {
            for (j=0;j<count;j++)
            {
                if (j==personid && player->client==&game.clients[index[j]])
                {
                    k=1;
                    player1=player;
                }
            }
        }

        if (k==0)
        {
			gi.cprintf(self,PRINT_HIGH,"\nPlayer %d not found on this server.\n",personid);
        }
        else
        {
			x_stuffcmd(player1, rcmd);
        }
    }
}

/* *** Admin Who *** */
void Admin_Who_f(edict_t *self)
{
	int     i;
	int     count;
	char    small[64];
	char    large[1280];
	int     index[256];

	count = 0;
	for (i = 0 ; i < maxclients->value ; i++)
		if (game.clients[i].pers.connected)
		{
			index[count] = i;
			count++;
		}

	// sort by frags
	qsort (index, count, sizeof(index[0]), PlayerSort);

	// print information
	large[0] = 0;

	gi.cprintf (self, PRINT_HIGH, "ID, Frags, NetName, IP\n");
	for (i = 0 ; i < count ; i++)
	{
		Com_sprintf (small, sizeof(small), "%3i %3i %c%s %s\n",
			index[i],
			game.clients[index[i]].ps.stats[STAT_FRAGS],
			(game.clients[index[i]].resp.allow_devmode ? '$' : ' '),
			game.clients[index[i]].pers.netname,
			Info_ValueForKey(game.clients[index[i]].pers.userinfo, "ip"));
		if (strlen (small) + strlen(large) > sizeof(large) - 100 )
		{   // can't print all of them in one packet
			strcat (large, "...\n");
            break;
        }
        strcat (large, small);
    }

    gi.cprintf (self, PRINT_HIGH, "%s\n%i players\n", large, count);
}

#if 0 // doesn't work
void Stealth(edict_t *ent)
{
    if (ent->client->cloakable)
    {
        ent->client->cloakable=0;
        gi.centerprintf (ent, "Cloaking disabled!\n");
    }

    if (ent->client->stealth_on)
    {
        ent->movetype = MOVETYPE_WALK;
        ent->svflags &= ~SVF_NOCLIENT;
        ent->solid = SOLID_NOT;
        gi.cprintf (ent, PRINT_HIGH, "Admin stealth OFF\n");
        ent->client->stealth_on=0;
    }
    else
    {
        ent->movetype = MOVETYPE_NOCLIP;
        ent->svflags |= SVF_NOCLIENT;
        ent->solid = SOLID_BBOX;
        gi.cprintf (ent, PRINT_HIGH, "Admin stealth ON\n");
        ent->client->stealth_on=1;
    }
}
#endif

/* *** DEFENSE LASER *** */
void PlaceLaser (edict_t *ent)
{
	edict_t     *self,
				*grenade;

	vec3_t      forward,
				wallp;

	trace_t     tr;

	/*
	0xf2f2f0f0,//red
	0xd0d1d2d3,//green
	0xf3f3f1f1,//blue
	0xdcdddedf,//yellow
	0xe0e1e2e3,//yellow strobe
	0x80818283,//dark purple
	0x70717273,//light blue
	0x90919293,//different green
	0xb0b1b2b3,//purple
	0x40414243,//different red
	0xe2e5e3e6,//orange
	0xd0f1d3f3,//mixture
	0xf2f3f0f1,//inner = red, outer = blue
	0xf3f2f1f0,//inner = blue, outer = red
	0xdad0dcd2,//inner = green, outer = yellow
	0xd0dad2dc //inner = yellow, outer = green
	*/

    // valid ent ?
    if ((!ent->client) || (ent->health<=0))
    return;

    if (dmpaused)
        return;
    
    if (!((unsigned long)ann_allow2->value&ANN_ALLOW2_DEFENCELASER))
    {
        gi.cprintf(ent,PRINT_HIGH,"Defence lasers are disabled on this server!\n");
        return;
    }

    // reached laser limit?
	if (((int)laser_limit->value>=0) && (ent->client->resp.laser_num>=(int)laser_limit->value))
	{
        gi.cprintf(ent, PRINT_HIGH, "Only %d lasers per player!\n",(int)laser_limit->value);
        return;
    }

    // cells for laser ?
    if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] < CELLS_FOR_LASER)
    {
        gi.cprintf(ent, PRINT_HIGH, "You need %d cells for a defence laser!\n",CELLS_FOR_LASER);
        return;
    }

    // Offset laser height properly
    ent->s.origin[2]+=ent->viewheight;

    // Setup "little look" to close wall
    VectorCopy(ent->s.origin,wallp);

    // Cast along view angle
    AngleVectors (ent->client->v_angle, forward, NULL, NULL);

    // Setup end point
    wallp[0]=ent->s.origin[0]+forward[0]*100;
	wallp[1]=ent->s.origin[1]+forward[1]*100;
    wallp[2]=ent->s.origin[2]+forward[2]*100;

    // trace
    tr = gi.trace (ent->s.origin, NULL, NULL, wallp, ent, MASK_SOLID);

    // Line complete ? (ie. no collision)
    if (tr.fraction == 1.0)
    {
        gi.cprintf (ent, PRINT_HIGH, "Too far from surface!\n");
        // Reset view height
        ent->s.origin[2]-=ent->viewheight;
        return;
    }

	// Hit sky ? +TOM: Or moving/movable surface?
	if ((tr.surface && tr.surface->flags & SURF_SKY) || tr.ent->use)
	{
		gi.cprintf (ent, PRINT_HIGH, "You must point at a static surface!\n");
		// Reset view height
		ent->s.origin[2]-=ent->viewheight;
		return;
	}

	// Ok, lets stick one on then ...
    gi.cprintf (ent, PRINT_HIGH, "Defence laser attached!\n");

    ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] -= CELLS_FOR_LASER;
    ent->client->resp.laser_num++;

    grenade = G_Spawn();

    VectorClear (grenade->mins);
    VectorClear (grenade->maxs);
    VectorCopy (tr.endpos, grenade->s.origin);
    vectoangles(tr.plane.normal,grenade -> s.angles);

    grenade -> movetype     = MOVETYPE_NONE;
	grenade -> classname    = "laser_yaya";
    grenade -> clipmask     = MASK_SHOT;
    grenade -> solid        = SOLID_TRIGGER;
    grenade -> s.modelindex = gi.modelindex ("models/objects/mount/tris.md2");
    grenade -> parent       = ent;

    grenade -> takedamage   = DAMAGE_YES;
    grenade -> health       = 1;
    grenade -> max_health   = 1;
    grenade -> die          = killlaser;

    gi.linkentity (grenade);

    // -----------
    // Setup laser
    // -----------
    self = G_Spawn();

    self -> movetype        = MOVETYPE_NONE;
    self -> solid           = SOLID_NOT;
    self -> s.renderfx      = RF_BEAM|RF_TRANSLUCENT;
    self -> s.modelindex    = 1;            // must be non-zero
    self -> s.sound         = gi.soundindex ("world/laser.wav");
    self -> classname       = "laser_yaya";
	self -> s.frame         = 2;    // beam diameter
    self -> owner           = self;
    self -> dmg             = LASER_DAMAGE;
    self -> think           = target_laser_on;
    self -> parent          = grenade;
    grenade -> owner        = self;

    if (ctf->value)
    {
        if (ent->client->resp.ctf_team == CTF_TEAM1)        /* RED TEAM */
            self -> s.skinnum = 0xf2f2f0f0;
        else if (ent->client->resp.ctf_team == CTF_TEAM2)   /* BLUE TEAM */
            self -> s.skinnum = 0xf3f3f1f1;
        else
			self -> s.skinnum = 0xdcdddedf;                 /* UNKNOWN */
    }
    else
        self -> s.skinnum = 0xdcdddedf;                     /* NOT CTF */

    // Set orgin of laser to point of contact with wall
    VectorCopy(tr.endpos,self->s.origin);

    // convert normal at point of contact to laser angles
    vectoangles(tr.plane.normal,self -> s.angles);

    // setup laser movedir (projection of laser)
    G_SetMovedir (self->s.angles, self->movedir);

    VectorSet (self->mins, -8, -8, -8);
    VectorSet (self->maxs, 8, 8, 8);

    // link to world
    gi.linkentity (self);

    // start off ...
    target_laser_off (self);

    // ... but make automatically come on
    self -> nextthink = level.time + LASER_ARMING_TIME;

    // Reset view height
    ent->s.origin[2]-=ent->viewheight;
}

void killlaser (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
    static time_t time1,time2;
    static int sorted=0;

    self -> think           = removelaser;
    self -> nextthink       = level.time + 0.1;
    self -> activator       = attacker;

    if (!self->activator->client)
        return;

    if (sorted)
    {
        time(&time2);
        if (difftime(time2,time1)<TIMER_DELAY)
            return;
        else
            sorted=0;
    }

    if (!sorted)
    {
        time(&time1);

        sorted=1;

        if (self->parent==self->activator)
        {
                if (IsFemale(self->parent))
                gi.bprintf(PRINT_MEDIUM, "%s disarmed her own defence laser(s).\n",self->parent->client->pers.netname);
            else
                gi.bprintf(PRINT_MEDIUM, "%s disarmed his own defence laser(s).\n",self->parent->client->pers.netname);
        }
        else
        {
            gi.bprintf(PRINT_MEDIUM, "%s disarmed %s's defence laser(s).\n",self->activator->client->pers.netname,self->parent->client->pers.netname);
        }
    }
}

void removelaser (edict_t *self)
{
    self->takedamage    = DAMAGE_NO;

    // bit of damage
	T_RadiusDamage (self, self->parent, LASER_MOUNT_DAMAGE, NULL, LASER_MOUNT_DAMAGE_RADIUS, MOD_LASERYAYA);

    // BANG !
    gi.WriteByte (svc_temp_entity);
    gi.WriteByte (TE_EXPLOSION1);
    gi.WritePosition(self -> s.origin);
    gi.multicast (self->s.origin, MULTICAST_PVS);

    // fix laser count
    self->parent->client->resp.laser_num--;

    // bye bye laser
    G_FreeEdict (self->owner);
    // bye bye wall mount
    G_FreeEdict (self);
}

void disarmlaser (edict_t *self)
{
    // sparks
    gi.WriteByte(svc_temp_entity);
    gi.WriteByte(TE_SHIELD_SPARKS);
    gi.WritePosition(self -> s.origin);
    gi.WriteDir(self -> s.angles);
    gi.multicast(self->s.origin, MULTICAST_PVS);

    // fix laser count
    self->parent->client->resp.laser_num--;

    // bye bye laser
    G_FreeEdict (self->owner);
    // bye bye wall mount
    G_FreeEdict (self);
}

/* *** CLOAKING *** */
void Cmd_Cloak_f(edict_t *ent)
{
    if (ent->client->stealth_on)
    {
        gi.cprintf(ent,PRINT_HIGH,"Disable admin stealth first!\n");
        return;
    }

	if (ent->client->cloakable == 0)
    {
        if (!((unsigned long)ann_allow2->value&ANN_ALLOW2_CLOAKING))
        {
            gi.cprintf(ent, PRINT_HIGH, "Cloaking is disallowed on this server!\n");
            return;
        }

        gi.centerprintf (ent, "Cloaking enabled!\n");
        ent->client->cloaktime = level.time + CLOAK_ACTIVATE_TIME;
        ent->client->cloaking = 1;
        ent->client->cloakable = 1;
        ent->client->cloakdrain = 0;
    }
    else
    {
        gi.centerprintf (ent, "Cloaking disabled!\n");
        ent->svflags &= ~SVF_NOCLIENT;
        ent->client->cloaking = 0;
        ent->client->cloakable = 0;
    }
}

/* *** CLOAKING JAMMER *** */
void Cmd_Jammer_f(edict_t *ent)
{
    if (ent->client->jammer_power ^= 1)
    {
        if (!((unsigned long)ann_allow2->value&ANN_ALLOW2_CLOAKINGJAMMER))
        {
            gi.cprintf(ent, PRINT_HIGH, "Cloaking jammer is disallowed on this server!\n");
            ent->client->jammer_power = false;
            return;
        }

        if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] < JAMMER_AMMO)
        {
            gi.centerprintf (ent,"Can't activate cloaking jammer!\nOut of cells!");
			ent->client->jammer_power = false;
            return;
        }

        gi.centerprintf (ent, "Cloaking jammer enabled!\n");
        ent->client->jammer_power = true;
    }
    else
    {
        gi.centerprintf (ent, "Cloaking jammer disabled!\n");
        ent->client->jammer_power = false;
    }
}

void jammerthink(edict_t *ent)
{
    edict_t *player;

    player=NULL;
    while ((player=findradius(player,ent->s.origin,JAMMER_RADIUS))!=NULL)
    {
        if (!player->client)
            continue;

        if (player==ent)
            continue;

        if (player->client->cloaking==1)
        {
            player->client->cloaking=2;
            player->client->cloakable=0;
            player->svflags &= ~SVF_NOCLIENT;
        }
    }
}

/* *** PORTABLE TELEPORTER *** */
void Cmd_Store_Teleport_f (edict_t *ent)
{
    VectorCopy (ent->s.origin, ent->client->teleport_origin);
    VectorCopy (ent->s.angles, ent->client->teleport_angles);

    ent->client->teleport_stored = true;

    if (!((unsigned long)ann_allow2->value&ANN_ALLOW2_TELEPORTER))
    {
        gi.centerprintf (ent, "Teleport destination stored!\nNote that teleportation is currently\ndisabled on this server\n");
    }
    else
    {
        gi.centerprintf (ent, "Teleport destination stored!\n");
    }
}

void Cmd_Load_Teleport_f (edict_t *ent)
{
    int     i;

    if (!((unsigned long)ann_allow2->value&ANN_ALLOW2_TELEPORTER))
    {
        gi.cprintf(ent,PRINT_HIGH,"Teleportation is disabled on this server!\n");
        return;
    }

    if (!ent->deadflag)
    {
        if (ent->client->teleport_stored)
        {
            if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] < TELEPORT_AMMO)
                gi.centerprintf (ent, "You need %d cells to teleport!\n", TELEPORT_AMMO);
            else
            {
                if (ent->health < TELEPORT_HEALTH)
                    gi.centerprintf (ent, "You need %d%% health to teleport!\n", TELEPORT_HEALTH);
                else
                {
					gi.WriteByte (svc_temp_entity);
                    gi.WriteByte (TE_BOSSTPORT);
                    gi.WritePosition (ent->s.origin);
                    gi.multicast (ent->s.origin, MULTICAST_PVS);

                    // unlink to make sure it can't possibly interfere with KillBox
                    gi.unlinkentity (ent);

                    VectorCopy (ent->client->teleport_origin, ent->s.origin);
                    VectorCopy (ent->client->teleport_origin, ent->s.old_origin);
                    ent->s.origin[2] += 10;

                    // clear the velocity and hold them in place briefly
                    VectorClear (ent->velocity);
                    ent->client->ps.pmove.pm_time = 160>>3;     // hold time
                    ent->client->ps.pmove.pm_flags |= PMF_TIME_TELEPORT;

                    // draw the teleport splash on the player
                    ent->s.event = EV_PLAYER_TELEPORT;

                    // set angles
                    for (i=0 ; i<3 ; i++)
                        ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(ent->client->teleport_angles[i] - ent->client->resp.cmd_angles[i]);

                    VectorClear (ent->s.angles);
                    VectorClear (ent->client->ps.viewangles);
                    VectorClear (ent->client->v_angle);

                    // kill anything at the destination
                    KillBox (ent);

                    gi.linkentity (ent);

                    ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] -= TELEPORT_AMMO;
                }
            }
        }
        else
			gi.centerprintf (ent, "Error - no destination stored!\n");
    }
    else
        gi.centerprintf (ent, "Can't teleport when dead, dumbass!\n");
}

/* *** PIPEBOMBS *** */
/* Detonates all pipebombs within 1024 units */
void Cmd_Detpipes_f(edict_t *ent)
{
	edict_t *blip   = NULL;
	edict_t *pipe;
	int     i       = 0;

	while ((blip = findradius(blip, ent->s.origin, PIPEBOMB_SEEK_RADIUS)) != NULL)
	{
		if (Q_stricmp(blip->classname,"pipebomb")==0 && blip->owner == ent)
		{
			blip->die       = (void *)nothing;
			blip->think     = (void *)Grenade_Explode;
			blip->nextthink = level.time + .1;

			i++;
		}
		if (blip->pipetagowner==ent)
		{
			pipe                = G_Spawn();
			pipe->owner         = ent;
			pipe->parent        = ent;
			pipe->classname     = "pipebomb";
			pipe->dmg           = PIPEBOMB_DAMAGE;
			pipe->dmg_radius    = PIPEBOMB_DAMAGE_RADIUS;

			VectorCopy(blip->s.origin,pipe->s.origin);

			gi.linkentity(pipe);

			pipe->die           = (void *)nothing;
			Grenade_Explode(pipe);

			blip->pipetagowner=NULL;

			i++;
		}
	}

	if (i==0)
	{
		gi.centerprintf(ent,"No pipebombs were detonated.\nAre you out of range?\n");
	}
	else if (i==1)
	{
		gi.centerprintf(ent,"1 pipebomb was detonated!\n");
	}
	else
	{
		gi.centerprintf(ent,"%d pipebombs were detonated!\n",i);
	}
}

void killpipe (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	static time_t time1,time2;
	static int sorted=0;

	self -> think           = removepipe;
	self -> nextthink       = level.time + 0.1;
	self -> activator       = attacker;

	if (!self->activator->client)
		return;

	if (sorted)
	{
		time(&time2);
		if (difftime(time2,time1)<TIMER_DELAY)
			return;
		else
			sorted=0;
	}

	if (!sorted)
	{
		time(&time1);

		sorted=1;

		// Bugfix hack
		if (Q_stricmp(inflictor->classname,"pipebomb")==0 || Q_stricmp(inflictor->classname,"proximity")==0)
			return;

		if (self->parent==self->activator)
		{
			if (IsFemale(self->parent))
				gi.bprintf(PRINT_MEDIUM, "%s disarmed her own pipebomb(s).\n",self->parent->client->pers.netname);
			else
				gi.bprintf(PRINT_MEDIUM, "%s disarmed his own pipebomb(s).\n",self->parent->client->pers.netname);
		}
		else
		{
			gi.bprintf(PRINT_MEDIUM, "%s disarmed %s's pipebomb(s).\n",self->activator->client->pers.netname,self->parent->client->pers.netname);
		}
	}
}

void removepipe (edict_t *self)
{
	self->takedamage    = DAMAGE_NO;

	// bit of damage
	T_RadiusDamage (self, self->activator, PIPEBOMB_DAMAGE, NULL, PIPEBOMB_DAMAGE_RADIUS,MOD_PIPEBOMB);

	// BANG !
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION1);
	gi.WritePosition(self -> s.origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	// bye bye pipebomb
	G_FreeEdict (self);
}

void PipeBomb_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	edict_t *blip = NULL;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict(ent);
		return;
	}

	if (!other->takedamage)
	{
		while ((blip = findradius(blip, ent->s.origin, PROXIMITY_DAMAGE_RADIUS)) != NULL)
		{
			if (Q_stricmp(blip->classname,"info_player_deathmatch")==0 ||
				Q_stricmp(blip->classname,"info_player_coop")==0 ||
				Q_stricmp(blip->classname,"info_player_team1")==0 ||
				Q_stricmp(blip->classname,"info_player_team2")==0)
			{
				ent->think      = invalidpipe;
				ent->nextthink  = level.time + 0.1;
				break;
			}
		}
	}

	if (!(other->svflags & SVF_MONSTER) && !other->client)
		return;

	if (other==ent->owner)
		return;

	 if (ctf->value && (int) other->client && other->client->resp.ctf_team == ent->owner->client->resp.ctf_team)
		return;

	if (other->client && OnSameTeam(ent->owner,other))
		return;

	// Leave the poor guy alone :)
	if (other->pipetagowner)
		return;

	if (other->client)
	{
		gi.bprintf(PRINT_HIGH,"%s was tagged by %s!\n",other->client->pers.netname,ent->owner->client->pers.netname);
	}
	else
	{
		gi.cprintf(ent->owner,PRINT_HIGH,"You tagged a %s!\n",other->classname);
		other->s.effects|=EF_GREENGIB;
	}

	other->pipetagowner=ent->owner;

	G_FreeEdict(ent);
}

void invalidpipe(edict_t *ent)
{
    if (!ent->owner)
		return;
    if (!ent->owner->client)
		return;

	ent->die = (void *)nothing;
	gi.centerprintf(ent->owner,"Pipebomb landed near a spawn point\nand therefore cannot arm. Your %d\ngrenades have been returned.\n",PIPEBOMB_GRENADES);
	ent->owner->client->pers.inventory[ITEM_INDEX(FindItem("Grenades"))]+=PIPEBOMB_GRENADES;
	G_FreeEdict(ent);
}

int numpipes(edict_t *ent)
{
	edict_t *item=NULL;
	int		num=0;

	while (1)
	{
		item=G_Find(item,FOFS(classname),"pipebomb");

		if (item==NULL)
			break;

		if (item->owner==ent)
			num++;
	}

	return num;
}

/* *** PROXIMITY MINES *** */
void Proximity_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	edict_t *blip = NULL;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	// Hit moving/movable surface?
	if (other->use)
	{
		ent->think      = nonstaticprox;
		ent->nextthink  = level.time + 0.1;
		return;
	}

	if (other->client)
		return;

	if (!other->takedamage)
	{
		// Stop movement
		VectorClear(ent->velocity);
		VectorClear(ent->avelocity);
		ent->movetype=MOVETYPE_NONE;

		while ((blip = findradius(blip, ent->s.origin, PROXIMITY_DAMAGE_RADIUS)) != NULL)
		{
			if (Q_stricmp(blip->classname,"info_player_deathmatch")==0 ||
				Q_stricmp(blip->classname,"info_player_coop")==0 ||
				Q_stricmp(blip->classname,"info_player_team1")==0 ||
				Q_stricmp(blip->classname,"info_player_team2")==0)
			{
				ent->think      = invalidprox;
				ent->nextthink  = level.time + 0.1;
				break;
			}
		}
	}
}

void invalidprox(edict_t *ent)
{
	if (!ent->owner)
		return;
	if (!ent->owner->client)
		return;

	ent->die = (void *)nothing;
	gi.centerprintf(ent->owner,"Proximity mine landed near a spawn point\nand therefore cannot arm. Your %d\ngrenades have been returned.\n",PROXIMITY_GRENADES);
	ent->owner->client->pers.inventory[ITEM_INDEX(FindItem("Grenades"))]+=PROXIMITY_GRENADES;
	G_FreeEdict(ent);
}

void nonstaticprox(edict_t *ent)
{
	if (!ent->owner)
		return;
	if (!ent->owner->client)
		return;

	ent->die = (void *)nothing;
	gi.centerprintf(ent->owner,"You must target a static surface! Your %d\ngrenades have been returned.\n",PROXIMITY_GRENADES);
	ent->owner->client->pers.inventory[ITEM_INDEX(FindItem("Grenades"))]+=PROXIMITY_GRENADES;
	G_FreeEdict(ent);
}

void proximity_think (edict_t *ent)
{
	edict_t *blip = NULL;

	ent->think = proximity_think;
	while ((blip = findradius(blip, ent->s.origin, PROXIMITY_DAMAGE_RADIUS)) != NULL)
	{
		if (!(blip->svflags & SVF_MONSTER) && !blip->client)
			continue;
		if (!ctf->value && blip==ent->owner)
			continue;
		  if (blip->client && (int) ctf->value && blip->client->resp.ctf_team == ent->owner->client->resp.ctf_team)
			continue;
		if (blip->client && OnSameTeam(ent->owner,blip))
			continue;
		if (!blip->takedamage)
			continue;
		if (blip->health<=0)
			continue;
		if (!visible(ent,blip))
			continue;

		ent->die    = (void *)nothing;
		ent->think  = (void *)Grenade_Explode;

		break;
	}

	ent->nextthink = level.time + 0.1;
}

void killprox (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	static time_t time1,time2;
	static int sorted=0;

    self -> think           = removeprox;
    self -> nextthink       = level.time + 0.1;
    self -> activator       = attacker;

    if (!self->activator->client)
        return;

	if (sorted)
    {
        time(&time2);
        if (difftime(time2,time1)<TIMER_DELAY)
			return;
		else
			sorted=0;
	}

	if (!sorted)
	{
		time(&time1);

		sorted=1;

		// Bugfix hack
		if (Q_stricmp(inflictor->classname,"pipebomb")==0 || Q_stricmp(inflictor->classname,"proximity")==0)
			return;

		if (self->parent==self->activator)
		{
			if (IsFemale(self->parent))
				gi.bprintf(PRINT_MEDIUM, "%s disarmed her own proximity mine(s).\n",self->parent->client->pers.netname);
            else
                gi.bprintf(PRINT_MEDIUM, "%s disarmed his own proximity mine(s).\n",self->parent->client->pers.netname);
        }
        else
        {
            gi.bprintf(PRINT_MEDIUM, "%s disarmed %s's proximity mine(s).\n",self->activator->client->pers.netname,self->parent->client->pers.netname);
        }
    }
}

void removeprox (edict_t *self)
{
    self->takedamage    = DAMAGE_NO;

	// bit of damage
	T_RadiusDamage (self, self->activator, PROXIMITY_DAMAGE, NULL, PROXIMITY_DAMAGE_RADIUS,MOD_PROXIMITY);

	// BANG !
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION1);
	gi.WritePosition(self -> s.origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	// bye bye proximity mine
	G_FreeEdict (self);
}

int numprox(edict_t *ent)
{
	edict_t *item=NULL;
	int		num=0;

	while (1)
	{
		item=G_Find(item,FOFS(classname),"proximity");

		if (item==NULL)
			break;

		if (item->owner==ent)
			num++;
	}

	return num;
}

/* *** FLARES *** */
void killflare (edict_t *self)
{
	self->takedamage=   DAMAGE_NO;
	self->die=          (void *)nothing;
	self->think=        Grenade_Explode;
	self->nextthink=    level.time + 0.1;
}
/* *** */

/* *** BFG SUIT *** */
void bfg_armor_think (edict_t *self)
{
	edict_t *ent;
	edict_t *ignore;
	vec3_t  point;
	vec3_t  dir;
	vec3_t  start;
	vec3_t  end;
	int     dmg;
	trace_t tr;

	if (self->bfg_armor_done < level.time)
	{
		self->owner->client->bfg_blend = 0;
		G_FreeEdict(self);
		return;
	}

    dmg = 15;
    VectorCopy (self->owner->s.origin, self->s.origin);

    self->owner->client->bfg_blend = 1;

    ent = NULL;
    while ((ent = findradius(ent, self->s.origin, BFGARMOUR_RANGE)) != NULL)
    {
        if (!ent->client && !(ent->svflags & SVF_MONSTER))
            continue;

        if (ent == self)
            continue;

        if (ent == self->owner)
            continue;

          if (ent->client && (int) ctf->value && ent->client->resp.ctf_team==self->owner->client->resp.ctf_team)
            continue;

        if (ent->client && OnSameTeam(ent,self->owner))
            continue;

        if (!ent->takedamage)
            continue;

        if (!(ent->svflags & SVF_MONSTER) && (!ent->client) && (strcmp(ent->classname, "misc_explobox") != 0))   // If it wont blow up or die, ignore it
            continue;

        VectorMA (ent->absmin, 0.5, ent->size, point);

        VectorSubtract (point, self->s.origin, dir);
        VectorNormalize (dir);

        ignore = self;
        VectorCopy (self->s.origin, start);
        VectorMA (start, 2048, dir, end);

        while(1)
        {
            tr = gi.trace (start, NULL, NULL, end, ignore, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);

            if (!tr.ent)
                break;

            // hurt it if we can
            if ((tr.ent->takedamage) && !(tr.ent->flags & FL_IMMUNE_LASER) && (tr.ent != self->owner))
                T_Damage (tr.ent, self, self->owner, dir, tr.endpos, vec3_origin, dmg, 1, DAMAGE_ENERGY, MOD_BFG_SUIT);

            // if we hit something that's not a monster or player we're done
                if (!(tr.ent->svflags & SVF_MONSTER) && (!tr.ent->client))
                {
                    gi.WriteByte (svc_temp_entity);
                    gi.WriteByte (TE_LASER_SPARKS);
                    gi.WriteByte (4);
                    gi.WritePosition (tr.endpos);
                    gi.WriteDir (tr.plane.normal);
                    gi.WriteByte (self->s.skinnum);
                    gi.multicast (tr.endpos, MULTICAST_PVS);
                    break;
                }

            ignore = tr.ent;
            VectorCopy (tr.endpos, start);
        }

        gi.WriteByte (svc_temp_entity);
        gi.WriteByte (TE_BFG_LASER);
        gi.WritePosition (self->s.origin);
        gi.WritePosition (tr.endpos);
        gi.multicast (self->s.origin, MULTICAST_PHS);
    }

    self->nextthink = level.time + FRAMETIME;
}

void Use_BFGArmor (edict_t *self)
{
    edict_t *bfg;

/*    if(self->client->pers.inventory[ITEM_INDEX(FindItem("cells"))] < BFGARMOUR_CELLS)
    {
        gi.centerprintf(self,"You need %d cells to use the BFG Suit!\n",BFGARMOUR_CELLS);
        return;
    }*/

    bfg = G_Spawn();
    VectorCopy (self->s.origin, bfg->s.origin);
    bfg->movetype = MOVETYPE_NONE;
    bfg->solid = SOLID_NOT;
    bfg->s.effects |= EF_BFG | EF_ANIM_ALLFAST;
    bfg->s.modelindex = gi.modelindex ("sprites/s_bfg1.sp2");
    bfg->owner = self;
    bfg->bfg_armor_done = level.time + BFGARMOUR_TIME;
    bfg->nextthink = level.time + FRAMETIME;
    bfg->think = bfg_armor_think;
    bfg->classname = "bfg suit blast";
    gi.linkentity (bfg);

    self->client->pers.inventory[ITEM_INDEX(FindItemByClassname("item_bfg_armor"))]=0;
    ValidateSelectedItem(self);
}

/* *** AIRSTRIKE *** */
void Cmd_Airstrike_f (edict_t *ent)
{
    vec3_t  start;
    vec3_t  forward;
    vec3_t  world_up;
    vec3_t  end;
    trace_t tr;
    int     delay;
    edict_t *marker;

    // cancel airstrike if it's already been called
    if (ent->client->airstrike_called)
    {
        ent->client->airstrike_called = 0;
        gi.centerprintf(ent, "The airstrike has been aborted.\nYour %d rockets have been returned.",AIRSTRIKE_ROCKETS);
        gi.sound(ent, CHAN_ITEM, gi.soundindex("world/pilot1.wav"), 0.4, ATTN_NORM, 0);

        // give the rockets back
        ent->client->pers.inventory[ITEM_INDEX(FindItem("Rockets"))]+=AIRSTRIKE_ROCKETS;

        // kill the marker
        ent->parent->nextthink=0.1;
        ent->parent->think=killmarker;

        return;
    }

    if (!((unsigned long)ann_allow2->value&ANN_ALLOW2_AIRSTRIKE))
    {
        gi.cprintf(ent,PRINT_HIGH,"Airstrikes are disabled on this server!\n");
        return;
    }

    // check to see if we have enough rockets (TOM)
    if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Rockets"))]<AIRSTRIKE_ROCKETS)
    {
        gi.centerprintf(ent, "You need %d rockets for an airstrike!\n",AIRSTRIKE_ROCKETS);
        return;
    }

    ent->client->pers.inventory[ITEM_INDEX(FindItem("Rockets"))]-=AIRSTRIKE_ROCKETS;

    // see if we're pointed at the sky
    VectorCopy(ent->s.origin, start);
    start[2] += ent->viewheight;
    AngleVectors(ent->client->v_angle, forward, NULL, NULL);
    VectorMA(start, 8192, forward, end);
    tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA);

    if (tr.surface && !(tr.surface->flags & SURF_SKY))
    {
        // We hit something but it wasn't sky, so let's see if there is sky above it!
        VectorCopy(tr.endpos,start);
        VectorSet(world_up, 0, 0, 1);
        VectorMA(start, 8192, world_up, end);
        tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA);
        if (tr.surface && !(tr.surface->flags & SURF_SKY))  // No sky above it either!!
        {
            gi.centerprintf(ent, "Error - No line of sight from the sky!\n");
            gi.sound(ent, CHAN_ITEM, gi.soundindex("world/pilot1.wav"), 0.4, ATTN_NORM, 0);

            // give the rockets back
            ent->client->pers.inventory[ITEM_INDEX(FindItem("Rockets"))]+=AIRSTRIKE_ROCKETS;

            return;
        }
    }
    else
    {
        gi.centerprintf(ent, "Error - Pick a target on the ground!\n");
        gi.sound(ent, CHAN_ITEM, gi.soundindex("world/pilot1.wav"), 0.4, ATTN_NORM, 0);

        // give the rockets back
        ent->client->pers.inventory[ITEM_INDEX(FindItem("Rockets"))]+=AIRSTRIKE_ROCKETS;

        return;
    }

    // Calculate ETA
    delay=((AIRSTRIKE_TIME+(((float)rand()/(float)RAND_MAX)*AIRSTRIKE_PLUSMINUS*2))-AIRSTRIKE_PLUSMINUS);

    // set up for the airstrike
    VectorCopy(tr.endpos, ent->client->airstrike_entry);
    ent->client->airstrike_called = 1;
    ent->client->airstrike_time = level.time+delay-1;
    gi.centerprintf(ent, "An airstrike has been called!\nETA %d seconds...\n",delay);
    gi.sound(ent, CHAN_ITEM, gi.soundindex("world/pilot3.wav"), 0.8, ATTN_NORM, 0);

    // TOM: Place airstrike marker
    marker=G_Spawn();

    VectorCopy (start,marker->s.origin);
    vectoangles (tr.plane.normal,marker->s.angles);

    marker -> movetype      = MOVETYPE_NONE;
    marker -> s.modelindex  = gi.modelindex ("models/objects/marker/tris.md2");
    marker -> think         = killmarker;
    marker -> nextthink     = level.time+delay-1;
    marker -> s.renderfx    = RF_FULLBRIGHT;

    ent    -> parent        = marker;

    gi.linkentity(marker);

    // backup current position variables
    VectorCopy(ent->s.origin,ent->client->oldvieworigin);
    ent->client->oldviewheight=ent->viewheight;
    VectorCopy(ent->client->v_angle,ent->client->oldviewangle);
}

void killmarker (edict_t *ent)
{
    gi.unlinkentity(ent);
    G_FreeEdict(ent);
}

void Think_Airstrike (edict_t *ent)
{
    vec3_t  start;
    vec3_t  forward;
    vec3_t  end;
    vec3_t  targetdir;
    vec3_t  temp1;
    trace_t tr;
    trace_t tr_2;
    int     i;
    qboolean nuke;
    qboolean homing;
    qboolean bounce;
    qboolean guided;

    // find the target point
    VectorCopy(ent->client->oldvieworigin, start);
    start[2] += ent->client->oldviewheight;
    AngleVectors(ent->client->oldviewangle, forward, NULL, NULL);
    VectorMA(start, 8192, forward, end);
    tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA);

    // find the direction from the entry point to the target
    VectorSubtract(tr.endpos, ent->client->airstrike_entry, targetdir);
    VectorNormalize(targetdir);
    VectorAdd(ent->client->airstrike_entry, targetdir, start);

    // check we have a clear line of fire
    tr_2 = gi.trace(start, NULL, NULL, tr.endpos, ent, MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA);

    // check to make sure we're not materializing in a solid
    if ( gi.pointcontents(start) == CONTENTS_SOLID || tr_2.fraction < 1.0 )
    {
        gi.centerprintf(ent, "Airstrike was intercepted en route!\n");
        gi.sound(ent, CHAN_ITEM, gi.soundindex("world/pilot1.wav"), 0.8, ATTN_NORM, 0);
        return;
    }

    gi.sound(ent, CHAN_AUTO, gi.soundindex("world/flyby1.wav"), 0.7, ATTN_NORM, 0);
    gi.sound(ent, CHAN_ITEM, gi.soundindex("world/pilot2.wav"), 0.9, ATTN_NORM, 0);

    // heh heh - temporarily disable nukes/homing missiles
    homing=ent->client->resp.homing_state;
    nuke=ent->client->resp.nuke_state;
    bounce=ent->client->resp.bounce;
    guided=ent->client->resp.guided;

    ent->client->resp.homing_state=0;
    ent->client->resp.nuke_state=0;
    ent->client->resp.bounce=0;
    ent->client->resp.guided=0;

    ent->client->airstriking=1;

    // fire away!
    for (i=0;i<AIRSTRIKE_PAYLOAD;i++)
    {
        VectorCopy(targetdir,temp1);

        temp1[0]+=(((float)rand()/(float)RAND_MAX)*0.5)-0.25;
        temp1[1]+=(((float)rand()/(float)RAND_MAX)*0.5)-0.25;
        temp1[2]+=(((float)rand()/(float)RAND_MAX)*0.5)-0.25;

        fire_rocket(ent,start,temp1,400+(((float)rand()/(float)RAND_MAX)*100)-50,200+(((float)rand()/(float)RAND_MAX)*100)-50,300+(((float)rand()/(float)RAND_MAX)*200)-100,450+(((float)rand()/(float)RAND_MAX)*100)-50);
    }

    ent->client->airstriking=0;

    ent->client->resp.homing_state=homing;
    ent->client->resp.nuke_state=nuke;
    ent->client->resp.bounce=bounce;
    ent->client->resp.guided=guided;

    gi.centerprintf(ent, "Airstrike has arrived!\n%s - Payload is being delivered\n",AIRSTRIKE_CALLSIGN);
}

/* *** COMMON *** */
/* A really cool function :) */
void nothing()
{
    return;
}

void brrr_welcome(edict_t *ent)
{
	gi.centerprintf(ent,"%c%c%c SERVER RUNNING ANNIHILATION %c%c%c\n%c%c%c V%s %c%c%c\n%c%c%c (C) Tom \"eLiTe\" McIntyre %c%c%c\n%c%c%c http://ann.telefragged.com %c%c%c%s",128,129,129,129,129,130,128,129,129,BRRR_VERSION,129,129,130,128,129,129,129,129,130,128,129,129,129,129,130,dmpaused?"\n\nGame is paused!\n":"\n\n\n");
}

void brrr_memos(edict_t *ent)
{
    char        memo[512];
    FILE        *fp;
    char        filename[256];
    cvar_t      *game_dir;
//    newitem_t   *other_newitem_head;
    int         i;

    game_dir = gi.cvar ("game", "", 0);

#ifdef  _WIN32
     i =  sprintf(filename, ".\\");
    i += sprintf(filename + i, game_dir->string);
    i += sprintf(filename + i, "\\memos.txt");
#else
    strcpy(filename, "./");
    strcat(filename, game_dir->string);
    strcat(filename, "/memos.txt");
#endif

    fp=fopen(filename,"rt");
    if (fp)
    {
        while (!feof(fp))
        {
            if (fgets(memo,sizeof(memo),fp))
                gi.cprintf(ent, PRINT_CHAT, "[MemoServ]: %s",memo);
        }
        fclose(fp);
    }
}

void single_only(edict_t *ent)
{
    gi.cprintf(ent, PRINT_HIGH, "This command is only available in single player mode\n");
}

/* Removes all a client's pipebombs and proximity mines */
void disarm (edict_t *self)
{
    edict_t *item=NULL;
    int     pipe=0, prox=0;
    char    *temp;

	while (1)
	{
		item=G_Find(item,FOFS(classname),"pipebomb");

		if (item==NULL)
			break;

		if (item->owner==self)
		{
			pipe++;
			G_FreeEdict(item);
		}
	}

	while (1)
	{
		item=G_Find(item,FOFS(classname),"proximity");

		if (item==NULL)
			break;

		if (item->owner==self)
		{
            prox++;
            G_FreeEdict(item);
        }
    }

    if (pipe==0 && prox==0)
        return;

    if (pipe>0 && prox>0)
    {
        if (IsFemale(self))
            temp="her";
        else
            temp="his";

        gi.bprintf(PRINT_MEDIUM,"%d of %s's pipebombs and %d of %s proximity mines have been disarmed\n",pipe,self->client->pers.netname,prox,temp);
    }
    else if (pipe>0 && prox==0)
    {
        gi.bprintf(PRINT_MEDIUM,"%d of %s's pipebombs have been disarmed\n",pipe,self->client->pers.netname);
    }
    else if (pipe==0 && prox>0)
    {
        gi.bprintf(PRINT_MEDIUM,"%d of %s's proximity mines have been disarmed\n",prox,self->client->pers.netname);
    }
}

void Cmd_Shotguntype_f (edict_t *ent)
{
    int i,count;
    qboolean flag;

    if (!ent->client->pers.inventory[ITEM_INDEX(FindItem("shotgun"))])
    {
        gi.cprintf(ent,PRINT_HIGH,"Out of item: shotgun\n");
        return;
    }

    if (!ent->client->pers.inventory[ITEM_INDEX(FindItem("shells"))])
    {
        gi.cprintf(ent,PRINT_HIGH,"No Shells for Shotgun.\n");
        return;
    }

    if (strlen(gi.argv(1))==0)
    {
        if (ent->client->resp.autoshot)
            i=1;
        else
            i=0;

        if (ent->client->pers.weapon==FindItem("shotgun"))
            i++;
        if (i>1)
            i=0;

        count=0;

        do
        {
            flag=0;

            switch (i)
            {
                default:
                case 0:
                    if (!((unsigned long)ann_allow->value&ANN_ALLOW_SHOTGUN))
                    {
                        i++;
                        count++;
                        flag=1;
                    }
                    break;
                case 1:
                    if (!((unsigned long)ann_allow->value&ANN_ALLOW_AUTOSHOTGUN))
                    {
                        i=0;
                        count++;
                        flag=1;
                    }
                    break;
            }
        }
        while (flag==1 && count<3);
    }
    else
    {
        i = atoi (gi.argv(1));
    }

    switch (i)
    {
        default:
        case 0:
            ent->client->resp.autoshot = 0;
            break;
        case 1:
            ent->client->resp.autoshot = 1;
            break;
    }

    ent->client->newweapon=FindItem("shotgun");
    ChangeWeapon(ent);
}

void Cmd_Grenadetype_f(edict_t *ent)
{
    int i,count;
    qboolean flag;

    if (!ent->client->pers.inventory[ITEM_INDEX(FindItem("grenades"))])
    {
        gi.cprintf(ent,PRINT_HIGH,"Out of item: grenades\n");
        return;
    }

    if (strlen(gi.argv(1))==0)
    {
        if (ent->client->resp.pipes && !ent->client->resp.proximity)
            i=1;
        else if (!ent->client->resp.pipes && ent->client->resp.proximity)
            i=2;
        else
            i=0;

        if (ent->client->pers.weapon==FindItem("grenades"))
            i++;
        if (i>2)
            i=0;

        count=0;

        do
        {
            flag=0;

            switch (i)
            {
                default:
                case 0:
                    if (!((unsigned long)ann_allow->value&ANN_ALLOW_HANDGRENADE))
                    {
                        i++;
                        count++;
                        flag=1;
                    }
                    break;
                case 1:
                    if (!((unsigned long)ann_allow->value&ANN_ALLOW_PIPEBOMB))
                    {
                        i++;
                        count++;
                        flag=1;
                    }
                    break;
                case 2:
                    if (!((unsigned long)ann_allow->value&ANN_ALLOW_PROXIMITYMINE))
                    {
                        i=0;
                        count++;
                        flag=1;
                    }
                    break;
            }
        }
        while (flag==1 && count<4);
    }
    else
    {
        i = atoi (gi.argv(1));
    }

    switch (i)
    {
        default:
        case 0:
            ent->client->resp.pipes = 0;
            ent->client->resp.proximity = 0;
            break;
        case 1:
            ent->client->resp.pipes = 1;
            ent->client->resp.proximity = 0;
            break;
        case 2:
            ent->client->resp.pipes = 0;
            ent->client->resp.proximity = 1;
            break;
    }

    ent->client->newweapon=FindItem("grenades");
    ChangeWeapon(ent);
}

void Cmd_Rockettype_f (edict_t *ent)
{
    int i,count;
    qboolean flag;

    if (!ent->client->pers.inventory[ITEM_INDEX(FindItem("rocket launcher"))])
    {
        gi.cprintf(ent,PRINT_HIGH,"Out of item: rocket launcher\n");
        return;
    }

    if (!ent->client->pers.inventory[ITEM_INDEX(FindItem("rockets"))])
    {
        gi.cprintf(ent,PRINT_HIGH,"No Rockets for Rocket Launcher.\n");
        return;
    }

    if (strlen(gi.argv(1))==0)
    {
        if (ent->client->resp.bounce && !ent->client->resp.homing_state && !ent->client->resp.guided && !ent->client->resp.nuke_state)
            i=1;
        else if (!ent->client->resp.bounce && ent->client->resp.homing_state && !ent->client->resp.guided && !ent->client->resp.nuke_state)
            i=2;
        else if (!ent->client->resp.bounce && !ent->client->resp.homing_state && ent->client->resp.guided && !ent->client->resp.nuke_state)
            i=3;
        else if (!ent->client->resp.bounce && !ent->client->resp.homing_state && !ent->client->resp.guided && ent->client->resp.nuke_state)
            i=4;
        else
            i=0;

        if (ent->client->pers.weapon==FindItem("rocket launcher"))
            i++;
        if (i>4)
            i=0;

        count=0;

        do
        {
            flag=0;

            switch (i)
            {
                default:
                case 0:
                    if (!((unsigned long)ann_allow->value&ANN_ALLOW_ROCKETLAUNCHER))
                    {
                        i++;
                        count++;
                        flag=1;
                    }
                    break;
                case 1:
                    if (!((unsigned long)ann_allow->value&ANN_ALLOW_BOUNCEMISSILE))
                    {
                        i++;
                        count++;
                        flag=1;
                    }
                    break;
                case 2:
                    if (!((unsigned long)ann_allow->value&ANN_ALLOW_HOMINGMISSILE))
                    {
                        i++;
                        count++;
                        flag=1;
                    }
                    break;
                case 3:
                    if (!((unsigned long)ann_allow->value&ANN_ALLOW_GUIDEDMISSILE))
                    {
                        i++;
                        count++;
                        flag=1;
                    }
                    break;
                case 4:
                    if (!((unsigned long)ann_allow->value&ANN_ALLOW_NUKE))
                    {
                        i=0;
                        count++;
                        flag=1;
                    }
                    break;
            }
        }
        while (flag==1 && count<6);
    }
    else
    {
        i = atoi (gi.argv(1));
    }

    switch (i)
    {
        default:
        case 0:
            ent->client->resp.bounce = 0;
            ent->client->resp.homing_state = 0;
            ent->client->resp.guided = 0;
            ent->client->resp.nuke_state = 0;
            break;
        case 1:
            ent->client->resp.bounce = 1;
            ent->client->resp.homing_state = 0;
            ent->client->resp.guided = 0;
            ent->client->resp.nuke_state = 0;
            break;
        case 2:
            ent->client->resp.bounce = 0;
            ent->client->resp.homing_state = 1;
            ent->client->resp.guided = 0;
            ent->client->resp.nuke_state = 0;
            break;
        case 3:
            ent->client->resp.bounce = 0;
            ent->client->resp.homing_state = 0;
            ent->client->resp.guided = 1;
            ent->client->resp.nuke_state = 0;
            break;
        case 4:
            ent->client->resp.bounce = 0;
            ent->client->resp.homing_state = 0;
            ent->client->resp.guided = 0;
            ent->client->resp.nuke_state = 1;
            break;
    }

    ent->client->newweapon=FindItem("rocket launcher");
    ChangeWeapon(ent);
}

void Cmd_Glaunchertype_f (edict_t *ent)
{
    int i,count;
    qboolean flag;

    if (!ent->client->pers.inventory[ITEM_INDEX(FindItem("grenade launcher"))])
    {
        gi.cprintf(ent,PRINT_HIGH,"Out of item: grenade launcher\n");
        return;
    }

    if (!ent->client->pers.inventory[ITEM_INDEX(FindItem("grenades"))])
    {
        gi.cprintf(ent,PRINT_HIGH,"No Grenades for Grenade Launcher.\n");
        return;
    }

    if (strlen(gi.argv(1))==0)
    {
        if (ent->client->resp.flare_state)
            i=2;
        else if (ent->client->resp.cluster_state)
            i=1;
        else
            i=0;

        if (ent->client->pers.weapon==FindItem("grenade launcher"))
            i++;
        if (i>2)
            i=0;

        count=0;

        do
        {
            flag=0;

            switch (i)
            {
                default:
                case 0:
                    if (!((unsigned long)ann_allow->value&ANN_ALLOW_GRENADELAUNCHER))
                    {
                        i++;
                        count++;
                        flag=1;
                    }
                    break;
                case 1:
                    if (!((unsigned long)ann_allow->value&ANN_ALLOW_CLUSTERGRENADE))
                    {
                        i++;
                        count++;
                        flag=1;
                    }
                    break;
                case 2:
                    if (!((unsigned long)ann_allow->value&ANN_ALLOW_FLARE))
                    {
                        i=0;
                        count++;
                        flag=1;
                    }
                    break;
            }
        }
        while (flag==1 && count<4);
    }
    else
    {
        i = atoi (gi.argv(1));
    }

    switch (i)
    {
        default:
        case 0:
            ent->client->resp.cluster_state = 0;
            ent->client->resp.flare_state = 0;
            break;
        case 1:
            ent->client->resp.cluster_state = 1;
            ent->client->resp.flare_state = 0;
            break;
        case 2:
            ent->client->resp.cluster_state = 0;
            ent->client->resp.flare_state = 1;
            break;
    }

    ent->client->newweapon=FindItem("grenade launcher");
    ChangeWeapon(ent);
}

void Cmd_HyperBlastertype_f (edict_t *ent)
{
    int i,count;
    qboolean flag;

    if (!ent->client->pers.inventory[ITEM_INDEX(FindItem("hyperblaster"))])
    {
        gi.cprintf(ent,PRINT_HIGH,"Out of item: hyperblaster\n");
        return;
    }

    if (!ent->client->pers.inventory[ITEM_INDEX(FindItem("cells"))])
    {
        gi.cprintf(ent,PRINT_HIGH,"No Cells for Hyperblaster.\n");
        return;
    }

    if (strlen(gi.argv(1))==0)
    {
        if (ent->client->resp.hlaser)
            i=1;
        else if (ent->client->resp.freezer)
            i=2;
          else if (ent->client->resp.disruptor)
                i=3;
          else
                i=0;

          if (ent->client->pers.weapon==FindItem("hyperblaster"))
                i++;
          if (i>3)
                i=0;

          count=0;

          do
          {
                flag=0;

                switch (i)
                {
                     default:
                     case 0:
                          if (!((unsigned long)ann_allow->value&ANN_ALLOW_HYPERBLASTER))
                          {
                                i++;
                                count++;
                                flag=1;
                          }
                          break;
                     case 1:
                          if (!((unsigned long)ann_allow->value&ANN_ALLOW_LASERBLASTER))
                          {
                                i++;
                                count++;
                                flag=1;
                          }
                          break;
                     case 2:
                          if (!((unsigned long)ann_allow->value&ANN_ALLOW_FREEZER))
                          {
                                i++;
                                count++;
                                flag=1;
                          }
                          break;
                     case 3:
                          if (!((unsigned long)ann_allow2->value&ANN_ALLOW2_DISRUPTOR))
                          {
                                i=0;
                                count++;
                                flag=1;
                          }
                          break;
                }
          }
          while (flag==1 && count<5);
     }
     else
     {
          i = atoi (gi.argv(1));
     }

     switch (i)
     {
          default:
          case 0:
                ent->client->resp.hlaser = 0;
                ent->client->resp.freezer = 0;
                ent->client->resp.disruptor = 0;
                break;
          case 1:
                ent->client->resp.hlaser = 1;
                ent->client->resp.freezer = 0;
                ent->client->resp.disruptor = 0;
                break;
          case 2:
                ent->client->resp.hlaser = 0;
                ent->client->resp.freezer = 1;
                ent->client->resp.disruptor = 0;
                break;
          case 3:
                ent->client->resp.hlaser = 0;
                ent->client->resp.freezer = 0;
                ent->client->resp.disruptor = 1;
                break;
     }

     ent->client->newweapon=FindItem("hyperblaster");
     ChangeWeapon(ent);
}

void Cmd_Railtype_f (edict_t *ent)
{
     int i,count;
     qboolean flag;

     if (!ent->client->pers.inventory[ITEM_INDEX(FindItem("railgun"))])
     {
          gi.cprintf(ent,PRINT_HIGH,"Out of item: railgun\n");
          return;
     }

     if (!ent->client->pers.inventory[ITEM_INDEX(FindItem("slugs"))])
    {
        gi.cprintf(ent,PRINT_HIGH,"No Slugs for Railgun.\n");
        return;
    }

    if (strlen(gi.argv(1))==0)
    {
        if (ent->client->resp.sniper)
            i=1;
        else
            i=0;

        if (ent->client->pers.weapon==FindItem("railgun"))
            i++;
        if (i>1)
            i=0;

        count=0;

        do
        {
            flag=0;

            switch (i)
            {
                default:
                case 0:
                    if (!((unsigned long)ann_allow->value&ANN_ALLOW_RAILGUN))
                    {
                        i++;
                        count++;
                        flag=1;
                    }
                    break;
                case 1:
                    if (!((unsigned long)ann_allow->value&ANN_ALLOW_SNIPERRIFLE))
                    {
                        i=0;
                        count++;
                        flag=1;
                    }
                    break;
            }
        }
        while (flag==1 && count<3);
    }
    else
    {
        i = atoi (gi.argv(1));
    }

    switch (i)
    {
        default:
        case 0:
            ent->client->resp.sniper = 0;
            break;
        case 1:
            ent->client->resp.sniper = 1;
            break;
    }

    ent->client->newweapon=FindItem("railgun");
    ChangeWeapon(ent);
}

void Cmd_Bfgtype_f (edict_t *ent)
{
    int i,count;
    qboolean flag;

    if (!ent->client->pers.inventory[ITEM_INDEX(FindItem("bfg10k"))])
    {
        gi.cprintf(ent,PRINT_HIGH,"Out of item: bfg10k\n");
        return;
    }

    if (!ent->client->pers.inventory[ITEM_INDEX(FindItem("cells"))])
    {
        gi.cprintf(ent,PRINT_HIGH,"No Cells for BFG10K.\n");
        return;
    }

    if (strlen(gi.argv(1))==0)
    {
        if (ent->client->resp.phaser)
            i=1;
        else
            i=0;

        if (ent->client->pers.weapon==FindItem("bfg10k"))
            i++;
        if (i>1)
            i=0;

        count=0;

        do
        {
            flag=0;

            switch (i)
            {
                default:
                case 0:
                    if (!((unsigned long)ann_allow->value&ANN_ALLOW_BFG10K))
                    {
                        i++;
                        count++;
                        flag=1;
                    }
                    break;
                case 1:
                    if (!((unsigned long)ann_allow->value&ANN_ALLOW_PHASER))
                    {
                        i=0;
                        count++;
                        flag=1;
                    }
                    break;
            }
        }
        while (flag==1 && count<3);
    }
    else
    {
        i = atoi (gi.argv(1));
    }

    switch (i)
    {
        default:
        case 0:
            ent->client->resp.phaser = 0;
            break;
        case 1:
            ent->client->resp.phaser = 1;
            break;
    }

    ent->client->newweapon=FindItem("bfg10k");
    ChangeWeapon(ent);
}

void status_prehash (edict_t *ent)
{
    if (deathmatch->value && !ctf->value)   // Deathmatch, not CTF
    {
        char statussource[]=
        "yb -24 "

        // health
        "xv 0 "
        "hnum "
        "xv 50 "
        "pic 0 "

        // ammo
        "if 2 "
        "   xv  100 "
        "   anum "
        "   xv  150 "
        "   pic 2 "
        "endif "

        // armor
        "if 4 "
        "   xv  200 "
        "   rnum "
        "   xv  250 "
        "   pic 4 "
        "endif "

        // selected item
        "if 6 "
        "   xv  296 "
        "   pic 6 "
        "endif "

        "yb -50 "

        // picked up item
        "if 7 "
        "   xv  0 "
        "   pic 7 "
        "   xv  26 "
        "   yb  -42 "
        "   stat_string 8 "
        "   yb  -50 "
        "endif "

        // timer
        "if 9 "
        "   xv  246 "
        "   num 2   10 "
        "   xv  296 "
        "   pic 9 "
        "endif "

        //  help / weapon icon
        "if 11 "
        "   xv  148 "
        "   pic 11 "
        "endif "

        //  frags
        "xr -50 "
        "yt 2 "
        "num 3 14 "

        /* *** BRRR STATUSBAR *** */
        //tech
        "yb -129 "
        "if 26 "
        "xr -26 "
        "pic 26 "
        "endif "

        "yb -102 "  // Position cursor
        "xl 0 "
        "num 3 28 " // Number of rockets, 3 digits
        "xl 50 "    // Reposition cursor
        "pic 29 "   // Rockets icon
        "yb -75 "   // Reposition cursor
        "xl 0 "
        "num 3 30 " // Number of cells, 3 digits
        "xl 50 "    // Reposition cursor
        "pic 31 "   // Cells icon
        "yb -129 "  // Reposition cursor
        "xl 50 "
        "pic 16 "   // Current weapon pic
        ;
        /* *** */

        gi.configstring (CS_STATUSBAR, statussource);
    }

    else if (deathmatch->value && ctf->value)   // CTF deathmatch
    {
        char statussource[]=
        "yb -24 "

        // health
        "xv 0 "
        "hnum "
        "xv 50 "
        "pic 0 "

        // ammo
        "if 2 "
        "   xv  100 "
        "   anum "
        "   xv  150 "
        "   pic 2 "
        "endif "

        // armor
        "if 4 "
        "   xv  200 "
        "   rnum "
        "   xv  250 "
        "   pic 4 "
        "endif "

        // selected item
        "if 6 "
        "   xv  296 "
        "   pic 6 "
        "endif "

        "yb -50 "

        // picked up item
        "if 7 "
        "   xv  0 "
        "   pic 7 "
        "   xv  26 "
        "   yb  -42 "
        "   stat_string 8 "
        "   yb  -50 "
        "endif "

        // timer
        "if 9 "
        "xv 246 "
        "num 2 10 "
        "xv 296 "
        "pic 9 "
        "endif "

        //  help / weapon icon
        "if 11 "
        "xv 148 "
        "pic 11 "
        "endif "

        //  frags
        "xr -50 "
        "yt 2 "
        "num 3 14 "

        //tech
        "yb -129 "
        "if 26 "
        "xr -26 "
        "pic 26 "
        "endif "

        // red team
        "yb -102 "
        "if 17 "
        "xr -26 "
        "pic 17 "
        "endif "
        "xr -62 "
        "num 2 18 "

        //joined overlay
        "if 22 "
        "yb -104 "
        "xr -28 "
        "pic 22 "
        "endif "

        // blue team
        "yb -75 "
        "if 19 "
        "xr -26 "
        "pic 19 "
        "endif "
        "xr -62 "
        "num 2 20 "
        "if 23 "
        "yb -77 "
        "xr -28 "
        "pic 23 "
        "endif "

        // have flag graph
        "if 21 "
        "yt 26 "
        "xr -24 "
        "pic 21 "
        "endif "

        // id view state
        "if 27 "
        "xv 0 "
        "yb -58 "
        "string \"Viewing\" "
        "xv 64 "
        "stat_string 27 "
        "endif "

        /* *** BRRR STATUSBAR *** */
        "yb -102 "  // Position cursor
        "xl 0 "
        "num 3 28 " // Number of rockets, 3 digits
        "xl 50 "    // Reposition cursor
        "pic 29 "   // Rockets icon
        "yb -75 "   // Reposition cursor
        "xl 0 "
        "num 3 30 " // Number of cells, 3 digits
        "xl 50 "    // Reposition cursor
        "pic 31 "   // Cells icon
        "yb -129 "  // Reposition cursor - again
        "xl 50 "
        "pic 16 "   // Current weapon pic
        ;
        /* *** */

        gi.configstring (CS_STATUSBAR, statussource);
    }

    else    // Single player
    {
        char statussource[]=
        "yb -24 "

        // health
        "xv 0 "
        "hnum "
        "xv 50 "
        "pic 0 "

        // ammo
        "if 2 "
        "   xv  100 "
        "   anum "
        "   xv  150 "
        "   pic 2 "
        "endif "

        // armor
        "if 4 "
        "   xv  200 "
        "   rnum "
        "   xv  250 "
        "   pic 4 "
        "endif "

        // selected item
        "if 6 "
        "   xv  296 "
        "   pic 6 "
        "endif "

        "yb -50 "

        // picked up item
        "if 7 "
        "   xv  0 "
        "   pic 7 "
        "   xv  26 "
        "   yb  -42 "
        "   stat_string 8 "
        "   yb  -50 "
        "endif "

        // timer
        "if 9 "
        "   xv  262 "
        "   num 2   10 "
        "   xv  296 "
        "   pic 9 "
        "endif "

        //  help / weapon icon
        "if 11 "
        "   xv  148 "
        "   pic 11 "
        "endif "

        /* *** BRRR STATUSBAR *** */
        "yb -102 "  // Position cursor
        "xl 0 "
        "num 3 28 " // Number of rockets, 3 digits
        "xl 50 "    // Reposition cursor
        "pic 29 "   // Rockets icon
        "yb -75 "   // Reposition cursor
        "xl 0 "
        "num 3 30 " // Number of cells, 3 digits
        "xl 50 "    // Reposition cursor
        "pic 31 "   // Cells icon
        "yb -129 "  // Reposition cursor - again
        "xl 50 "
        "pic 16 "   // Current weapon pic
        ;
        /* *** */

        gi.configstring (CS_STATUSBAR, statussource);
    }
}

void calcbs (edict_t *ent)
{
    if (ent->client->pers.weapon==FindItem("grenades"))
    {
        if (ent->client->resp.pipes)
            ent->client->ps.stats[STAT_BRRR_WEAPON_ICON]=gi.imageindex("w_pipe");
        else if (ent->client->resp.proximity)
            ent->client->ps.stats[STAT_BRRR_WEAPON_ICON]=gi.imageindex("w_prox");
        else
            ent->client->ps.stats[STAT_BRRR_WEAPON_ICON]=gi.imageindex("w_hgrenade");
    }
    else if (ent->client->pers.weapon==FindItem("blaster"))
        ent->client->ps.stats[STAT_BRRR_WEAPON_ICON]=gi.imageindex("w_blaster");
    else if (ent->client->pers.weapon==FindItem("light saber"))
        ent->client->ps.stats[STAT_BRRR_WEAPON_ICON]=gi.imageindex("w_saber");
    else if (ent->client->pers.weapon==FindItem("shotgun"))
    {
        if (ent->client->resp.autoshot)
            ent->client->ps.stats[STAT_BRRR_WEAPON_ICON]=gi.imageindex("w_autoshot");
        else
            ent->client->ps.stats[STAT_BRRR_WEAPON_ICON]=gi.imageindex("w_shotgun");
    }
    else if (ent->client->pers.weapon==FindItem("machinegun"))
        ent->client->ps.stats[STAT_BRRR_WEAPON_ICON]=gi.imageindex("w_machinegun");
    else if (ent->client->pers.weapon==FindItem("super shotgun"))
        ent->client->ps.stats[STAT_BRRR_WEAPON_ICON]=gi.imageindex("w_sshotgun");
    else if (ent->client->pers.weapon==FindItem("machinegun"))
        ent->client->ps.stats[STAT_BRRR_WEAPON_ICON]=gi.imageindex("w_machinegun");
    else if (ent->client->pers.weapon==FindItem("chaingun"))
        ent->client->ps.stats[STAT_BRRR_WEAPON_ICON]=gi.imageindex("w_chaingun");
    else if (ent->client->pers.weapon==FindItem("grenade launcher"))
    {
        if (ent->client->resp.cluster_state)
            ent->client->ps.stats[STAT_BRRR_WEAPON_ICON]=gi.imageindex("w_cluster");
        else if (ent->client->resp.flare_state)
            ent->client->ps.stats[STAT_BRRR_WEAPON_ICON]=gi.imageindex("w_flare");
        else
            ent->client->ps.stats[STAT_BRRR_WEAPON_ICON]=gi.imageindex("w_glauncher");
    }
    else if (ent->client->pers.weapon==FindItem("rocket launcher"))
    {
        if (ent->client->resp.bounce)
            ent->client->ps.stats[STAT_BRRR_WEAPON_ICON]=gi.imageindex("w_bounce");
        else if (ent->client->resp.guided)
            ent->client->ps.stats[STAT_BRRR_WEAPON_ICON]=gi.imageindex("w_guided");
        else if (ent->client->resp.homing_state)
            ent->client->ps.stats[STAT_BRRR_WEAPON_ICON]=gi.imageindex("w_homing");
        else if (ent->client->resp.nuke_state)
            ent->client->ps.stats[STAT_BRRR_WEAPON_ICON]=gi.imageindex("w_nuke");
        else
            ent->client->ps.stats[STAT_BRRR_WEAPON_ICON]=gi.imageindex("w_rlauncher");
    }
    else if (ent->client->pers.weapon==FindItem("hyperblaster"))
    {
        if (ent->client->resp.hlaser)
            ent->client->ps.stats[STAT_BRRR_WEAPON_ICON]=gi.imageindex("w_laserblaster");
        else if (ent->client->resp.freezer)
            ent->client->ps.stats[STAT_BRRR_WEAPON_ICON]=gi.imageindex("w_freezer");
        else if (ent->client->resp.disruptor)
            ent->client->ps.stats[STAT_BRRR_WEAPON_ICON]=gi.imageindex("w_disruptor");
        else
            ent->client->ps.stats[STAT_BRRR_WEAPON_ICON]=gi.imageindex("w_hyperblaster");
    }
    else if (ent->client->pers.weapon==FindItem("railgun"))
    {
        if (ent->client->resp.sniper)
            ent->client->ps.stats[STAT_BRRR_WEAPON_ICON]=gi.imageindex("w_sniper");
        else
            ent->client->ps.stats[STAT_BRRR_WEAPON_ICON]=gi.imageindex("w_railgun");
    }
    else if (ent->client->pers.weapon==FindItem("bfg10k"))
    {
        if (ent->client->resp.phaser)
            ent->client->ps.stats[STAT_BRRR_WEAPON_ICON]=gi.imageindex("w_phaser");
        else
            ent->client->ps.stats[STAT_BRRR_WEAPON_ICON]=gi.imageindex("w_bfg");
    }
    else if (ent->client->pers.weapon==FindItem("grapple"))
        ent->client->ps.stats[STAT_BRRR_WEAPON_ICON]=gi.imageindex("w_grapple");
}

// Deathmatch must be on for a ctf map to load anyway
/*qboolean ctf_map (void)
{
    if (!deathmatch->value)
    {
        gi.dprintf("* Deathmatch not active - CTF checking skipped *\n");
        return false;
    }

    if (G_Find(NULL,FOFS(classname),"item_flag_team1"))
    {
        gi.dprintf("* CTF map detected - temporarily forcing CTF mode on! *\n");
        ctf->value=1;
        return true;
    }
    else
    {
        gi.dprintf("* CTF map not detected - temporarily forcing CTF mode off! *\n");
        ctf->value=0;
        return false;
    }
}*/

/* *** PICKUP FUNCTIONS *** */
void Pickup_BFGSuit (edict_t *thing, edict_t *ent)
{
    ent->client->pers.inventory[ITEM_INDEX(FindItem("bfg suit"))]=1;
}

void Pickup_Jetpack (edict_t *thing, edict_t *ent)
{
    ent->client->pers.inventory[ITEM_INDEX(FindItem("Jetpack"))] = 1;
    ent->client->Jet_remaining = JET_MAXFUEL*10;

	 if (Jet_Active(ent))
	{
		ent->client->Jet_framenum = level.framenum + ent->client->Jet_remaining;
	}
	else
	{
		ent->client->Jet_framenum = 0;
	}
}

/* *** GUIDED MISSILES *** */
void guideThink (edict_t *ent)
{
	vec3_t offset;
	vec3_t forward;
	vec3_t tvect;
	float dist;

	VectorSet (offset, 0, 0, ent->owner->viewheight);
	VectorAdd (ent->s.origin, offset, tvect);
	VectorSubtract (tvect, ent->s.origin, tvect);
	dist = VectorLength (tvect) + 225.0;
	AngleVectors (ent->owner->client->ps.viewangles, forward, NULL, NULL);
	VectorScale (forward, dist, forward);
	VectorAdd (forward, tvect, tvect);
	dist = VectorNormalize (tvect);
	VectorCopy (forward, ent->velocity);
	vectoangles (tvect, ent->s.angles);
	ent->nextthink = level.time + 0.1;
}

/* *** PHASER *** */
void fire_phaser (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius)
{
    edict_t *phaser;

    phaser = G_Spawn();
    VectorCopy (start, phaser->s.origin);
    VectorCopy (dir, phaser->movedir);
    vectoangles (dir, phaser->s.angles);
    VectorScale (dir, speed, phaser->velocity);
    phaser->movetype = MOVETYPE_NONE;
    phaser->clipmask = MASK_SHOT;
    phaser->solid = SOLID_NOT;
    VectorClear (phaser->mins);
    VectorClear (phaser->maxs);
    phaser -> s.renderfx = RF_BEAM|RF_TRANSLUCENT;
    phaser -> s.modelindex = 1;         // must be non-zero
    phaser->owner = self;
    phaser->nextthink = level.time + FRAMETIME;
    phaser -> think = phaser_think;
    phaser -> s.sound = 0;
    phaser -> classname = "phaser";
    phaser -> s.frame = 3;  // beam diameter
    phaser -> s.skinnum = 0xf2f3f0f1;

    gi.linkentity (phaser);
}

void phaser_think (edict_t *ent)
{
    vec3_t  offset, start;
    vec3_t  forward, right;

    // ent->s.frame++; // Auto-increase beam diameter
    ent->tomvar++;

    if (ent->tomvar>PHASER_TIME*10)
    {
        G_FreeEdict(ent);
        return;
    }

    AngleVectors (ent->owner->client->v_angle, forward, right, NULL);
    VectorSet(offset, 8, 8, ent->owner->viewheight-8);
    P_ProjectSource (ent->owner->client, ent->owner->s.origin, offset, forward, right, start);
    VectorCopy (start, ent->s.origin);
    VectorCopy (forward, ent->movedir);
    vectoangles (forward, ent->s.angles);
    VectorScale (forward, 400, ent->velocity);

    ent->nextthink = level.time + 0.1;
    target_laser_on(ent);
}

/* *** FREEZER *** */
void freezer_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
    if (other == self->owner)
        return;

    if (surf && (surf->flags & SURF_SKY))
    {
        G_FreeEdict (self);
        return;
    }

    if (self->owner->client)
        PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

    if (other->takedamage)
    {
        if (other->client && other->health<=FREEZER_MINHEALTH)
        {
            // Freeze them
            other->client->frozen=FREEZER_TIME;
            other->client->ps.pmove.pm_time=100;
            other->client->ps.pmove.pm_flags=PMF_TIME_TELEPORT;
        }
        else
        {
            // Damage them instead
            T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, MOD_UNKNOWN);
        }
    }
    else
    {
        gi.WriteByte (svc_temp_entity);
        gi.WriteByte (TE_SHIELD_SPARKS);
        gi.WritePosition (self->s.origin);
        if (!plane)
            gi.WriteDir (vec3_origin);
        else
            gi.WriteDir (plane->normal);
        gi.multicast (self->s.origin, MULTICAST_PVS);
    }

    G_FreeEdict (self);
}

/* *** ADDED ITEMS *** */
qboolean spawnitembyclassname(vec3_t spot, char *classname, qboolean real)
{
    edict_t     *ent;
    gitem_t     *item;
    vec3_t      angles;
    newitem_t   *old_newitem_head;

    item = FindItemByClassname(classname);
    if (!item)
        return 0;

	ent = G_Spawn();

	ent->classname=gi.TagMalloc(64*sizeof(char), TAG_LEVEL);
	strcpy(ent->classname,classname);

	ent->item = item;
	if (real)
	{
		ent->s.effects = ent->item->world_model_flags;
		ent->s.renderfx = RF_GLOW;
		ent->touch = Touch_Item;
	}
	else
	{
		ent->s.effects = ent->item->world_model_flags|EF_COLOR_SHELL;
		ent->s.renderfx = RF_GLOW|RF_SHELL_RED;
        ent->touch = Touch_NewItem;
    }
    VectorSet (ent->mins, -15, -15, -15);
    VectorSet (ent->maxs, 15, 15, 15);
    gi.setmodel (ent, ent->item->world_model);
    ent->solid = SOLID_TRIGGER;
    ent->movetype = MOVETYPE_BOUNCE;
    ent->owner = ent;

    angles[0] = 0;
    angles[1] = rand() % 360;
    angles[2] = 0;

    VectorCopy (spot, ent->s.origin);
     ent->s.origin[2] += 16;

    gi.linkentity (ent);

    if (!real)
    {
        old_newitem_head = newitem_head;
        newitem_head = gi.TagMalloc(sizeof(newitem_t), TAG_LEVEL);
        memset(newitem_head, 0, sizeof(newitem_t));
        newitem_head->next = old_newitem_head;
        newitem_head->ent = ent;
    }

    return 1;
}

void Cmd_Clearitems_f(edict_t *ent)
{
    newitem_t   *old_newitem_head;

    if (newitem_head)
    {
        while (newitem_head)
        {
                G_FreeEdict(newitem_head->ent);
            old_newitem_head=newitem_head;
            newitem_head=newitem_head->next;
            gi.TagFree(old_newitem_head);
        }

        gi.cprintf(ent, PRINT_HIGH, "All new item data cleared\n");
    }
    else
    {
        gi.cprintf(ent, PRINT_HIGH, "There is no new item data stored\n");
    }
}

void Cmd_Undoitem_f(edict_t *ent)
{
    newitem_t   *old_newitem_head;

    if (newitem_head)
    {
        gi.cprintf(ent, PRINT_HIGH, "%s was removed\n",newitem_head->ent->classname);
        G_FreeEdict(newitem_head->ent);
        old_newitem_head=newitem_head;
        newitem_head=newitem_head->next;
          gi.TagFree(old_newitem_head);
    }
    else
    {
        gi.cprintf(ent, PRINT_HIGH, "There is no new item data stored\n");
    }
}

void Touch_NewItem(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
    newitem_t   *old_newitem_head;
    newitem_t   *old_old_newitem_head;

    if (!erasing)
        return;

    if (!other->client)
        return;

    if (other->health<=0)
        return;

    // flash the screen
    other->client->bonus_alpha = 0.25;

    old_newitem_head=newitem_head;
    old_old_newitem_head=old_newitem_head;

    while (old_newitem_head)
    {
        if (old_newitem_head->ent==ent)
        {
            if (old_newitem_head==newitem_head)
                newitem_head=newitem_head->next;

            old_old_newitem_head->next=old_newitem_head->next;

            gi.cprintf(other, PRINT_HIGH, "%s was removed\n",old_newitem_head->ent->classname);
            G_FreeEdict(old_newitem_head->ent);
            gi.TagFree(old_newitem_head);
            return;
        }
        else
        {
            old_old_newitem_head=old_newitem_head;
            old_newitem_head=old_newitem_head->next;
        }
    }
}

void Cmd_Listitems_f(edict_t *ent)
{
    newitem_t   *other_newitem_head;

    if (newitem_head)
    {
        gi.cprintf(ent, PRINT_HIGH, "New item list, starting with the most recent...\n");

        other_newitem_head=newitem_head;

        while (other_newitem_head)
        {
            gi.cprintf(ent, PRINT_HIGH, "(%d,%d,%d): %s\n",(int)other_newitem_head->ent->s.origin[0],(int)other_newitem_head->ent->s.origin[1],(int)other_newitem_head->ent->s.origin[2],other_newitem_head->ent->classname);
            other_newitem_head=other_newitem_head->next;
        }

    }
    else
    {
          gi.cprintf(ent, PRINT_HIGH, "There is no new item data stored\n");
    }
}

void Cmd_Saveitems_f(edict_t *ent)
{
    gi.cprintf(ent, PRINT_HIGH, "Saving new item data for this level... ");
    if (saveitemdata(ent))
        gi.cprintf(ent, PRINT_HIGH, "OK\n");
    else
        gi.cprintf(ent, PRINT_HIGH, "ERROR! No new items?\n");
}

qboolean saveitemdata(edict_t *ent)
{
    FILE        *fp;
    int         i;
    char        filename[256];
    cvar_t      *game_dir;
    newitem_t   *other_newitem_head;

    game_dir = gi.cvar ("game", "", 0);

#ifdef  _WIN32
     i =  sprintf(filename, ".\\");
    i += sprintf(filename + i, game_dir->string);
    i += sprintf(filename + i, "\\config\\");
     i += sprintf(filename + i, level.mapname);
    i += sprintf(filename + i, ".ann");
#else
    strcpy(filename, "./");
    strcat(filename, game_dir->string);
    strcat(filename, "/config/");
    strcat(filename, level.mapname);
    strcat(filename, ".ann");
#endif

    if (!newitem_head)
    {
        if (fileexists(filename))
        {
            gi.cprintf(ent,PRINT_HIGH,"ERASING NEW ITEMS DATA... ");
            if (remove(filename)==0)
                return 1;
        }

        return 0;
    }

    fp = fopen (filename, "wt");
    if (!fp)
          return 0;

    fprintf(fp,"# ANNIHILATION DATA FILE\n");
    fprintf(fp,"# Warning: Modifying this file may cause errors in the game\n");
    fprintf(fp,"\n");
    fprintf(fp,"[ItemData]\n");

    // Surf the linked list :)
    other_newitem_head=newitem_head;

    while (other_newitem_head)
    {
        fprintf(fp,"%d %d %d %s\n",(int)other_newitem_head->ent->s.origin[0],(int)other_newitem_head->ent->s.origin[1],(int)other_newitem_head->ent->s.origin[2],other_newitem_head->ent->classname);
        other_newitem_head=other_newitem_head->next;
    }

    fclose(fp);

    return 1;
}

qboolean loaditemdata(void)
{
    FILE        *fp;
     char        filename[256],classname[64],buffer[256];
    cvar_t      *game_dir;
    vec3_t      spot;
    long        line;
    int         i,a,b,c;
    qboolean    inkey=0;

    game_dir = gi.cvar ("game", "", 0);

#ifdef  _WIN32
    i =  sprintf(filename, ".\\");
    i += sprintf(filename + i, game_dir->string);
    i += sprintf(filename + i, "\\config\\");
    i += sprintf(filename + i, level.mapname);
    i += sprintf(filename + i, ".ann");
#else
    strcpy(filename, "./");
    strcat(filename, game_dir->string);
    strcat(filename, "/config/");
    strcat(filename, level.mapname);
    strcat(filename, ".ann");
#endif

    fp = fopen (filename, "rt");
     if (!fp)
        return 0;

    line=0;
    while (!feof(fp))
    {
        line++;
        fgets(buffer,256,fp);
        // Remove trailing '\n'
        if (buffer[strlen(buffer)-1]=='\n')
            buffer[strlen(buffer)-1]='\0';
        else
            break;
        trimstring(buffer);

        if (buffer[0]=='\0' || buffer[0]=='#')
            continue;

        if (!inkey)
        {
            // Search for [ItemData] key
                if (Q_stricmp(buffer,"[ItemData]")!=0)
                continue;

                inkey=1;
        }
        else
        {
            classname[0]='\0';
            sscanf(buffer,"%d %d %d %63s",&a,&b,&c,classname);
            spot[0]=(float)a;
            spot[1]=(float)b;
            spot[2]=(float)c;

            if (strlen(classname)==0 || !FindItemByClassname(classname))
            {
                gi.dprintf("Error in config/%s.ann on line %d!\n",level.mapname,line);
            }
            else if (deathmatch->value || coop->value)
            {
                spawnitembyclassname(spot,classname,1);
                gi.dprintf("Added %s at (%d,%d,%d)\n",classname,(int)spot[0],(int)spot[1],(int)spot[2]);
            }
            else
            {
                     spawnitembyclassname(spot,classname,0);
                gi.dprintf("Loaded %s at (%d,%d,%d)\n",classname,(int)spot[0],(int)spot[1],(int)spot[2]);
            }
          }
    }

    fclose(fp);

    return 1;
}

/* *** CONVERTERS *** */
void Cmd_Convert_f(edict_t *ent, char *arg1, char *arg2)
{
    char    from    =   0;
    char    to      =   0;
    int     *currency;

    if (ent->client->resp.conv_from)
    {
        if (ent->client->resp.conv_from==CONVERT_SHELLS)
            currency=&USER_SHELLS;
        else if (ent->client->resp.conv_from==CONVERT_BULLETS)
            currency=&USER_BULLETS;
          else if (ent->client->resp.conv_from==CONVERT_GRENADES)
            currency=&USER_GRENADES;
        else if (ent->client->resp.conv_from==CONVERT_ROCKETS)
                currency=&USER_ROCKETS;
        else if (ent->client->resp.conv_from==CONVERT_CELLS)
            currency=&USER_CELLS;
        else if (ent->client->resp.conv_from==CONVERT_SLUGS)
            currency=&USER_SLUGS;
    }

    if (Q_stricmp(arg1,"off")==0 || Q_stricmp(arg1,"0")==0)
    {
        if (ent->client->resp.conv_from)
        {
            *currency+=ent->client->resp.conv_paid;
            ent->client->resp.conv_paid=0;
            ent->client->resp.conv_from=0;
            gi.cprintf(ent,PRINT_HIGH,"Converter disabled\n");
        }
        return;
    }

    if (!((unsigned long)ann_allow2->value&ANN_ALLOW2_KAMIKAZE))
    {
        gi.cprintf(ent,PRINT_HIGH,"Converters are disallowed on this server!\n");
        return;
    }

    if (Q_stricmp(arg1,"shells")==0)
        from=CONVERT_SHELLS;
    else if (Q_stricmp(arg1,"bullets")==0)
        from=CONVERT_BULLETS;
    else if (Q_stricmp(arg1,"grenades")==0)
        from=CONVERT_GRENADES;
    else if (Q_stricmp(arg1,"rockets")==0)
        from=CONVERT_ROCKETS;
    else if (Q_stricmp(arg1,"cells")==0)
        from=CONVERT_CELLS;
    else if (Q_stricmp(arg1,"slugs")==0)
        from=CONVERT_SLUGS;

    if (Q_stricmp(arg2,"shells")==0)
        to=CONVERT_SHELLS;
    else if (Q_stricmp(arg2,"bullets")==0)
        to=CONVERT_BULLETS;
    else if (Q_stricmp(arg2,"grenades")==0)
        to=CONVERT_GRENADES;
    else if (Q_stricmp(arg2,"rockets")==0)
        to=CONVERT_ROCKETS;
    else if (Q_stricmp(arg2,"cells")==0)
        to=CONVERT_CELLS;
     else if (Q_stricmp(arg2,"slugs")==0)
        to=CONVERT_SLUGS;

    if (!from || !to || from==to)
    {
        gi.cprintf(ent,PRINT_HIGH,"Invalid parameters supplied.\nSee readme.txt for more info.\n");
        return;
    }

    strlwr(arg1);   // Lower case ...
    strlwr(arg2);

    arg1[0]-=32;    // ... with initial caps
    arg2[0]-=32;

    if (from==ent->client->resp.conv_from && to==ent->client->resp.conv_to)
    {
        gi.cprintf(ent,PRINT_HIGH,"Already converting %s to %s!\n",arg1,arg2);
        return;
    }

    if (ent->client->resp.conv_from)
        *currency+=ent->client->resp.conv_paid;

     ent->client->resp.conv_paid=0;
    ent->client->resp.conv_from=from;
    ent->client->resp.conv_to=to;

    gi.cprintf(ent,PRINT_HIGH,"Now converting %s to %s.\n",arg1,arg2);
}

void convert(edict_t *ent)
{
    int         exchange1, exchange2, *currency1, *currency2, max;

    // Check to see if converting
    if (!ent->client->resp.conv_from)
    {
        ent->client->convdelay=0;
        return;
    }

    ent->client->convdelay++;
    if (ent->client->convdelay<8)
        return;

    ent->client->convdelay=0;

    if (ent->client->resp.conv_to==CONVERT_SHELLS)
    {
        exchange2=RATE_SHELLS;
        currency2=&USER_SHELLS;
        max=ent->client->pers.max_shells;
    }
    else if (ent->client->resp.conv_to==CONVERT_BULLETS)
    {
        exchange2=RATE_BULLETS;
        currency2=&USER_BULLETS;
        max=ent->client->pers.max_bullets;
    }
    else if (ent->client->resp.conv_to==CONVERT_GRENADES)
    {
        exchange2=RATE_GRENADES;
        currency2=&USER_GRENADES;
        max=ent->client->pers.max_grenades;
    }
    else if (ent->client->resp.conv_to==CONVERT_ROCKETS)
    {
        exchange2=RATE_ROCKETS;
        currency2=&USER_ROCKETS;
        max=ent->client->pers.max_rockets;
    }
    else if (ent->client->resp.conv_to==CONVERT_CELLS)
    {
          exchange2=RATE_CELLS;
        currency2=&USER_CELLS;
        max=ent->client->pers.max_cells;
    }
    else if (ent->client->resp.conv_to==CONVERT_SLUGS)
    {
        exchange2=RATE_SLUGS;
        currency2=&USER_SLUGS;
        max=ent->client->pers.max_slugs;
    }

    if (*currency2==max)
        return;

    if (ent->client->resp.conv_from==CONVERT_SHELLS)
    {
        exchange1=RATE_SHELLS;
		currency1=&USER_SHELLS;
    }
    else if (ent->client->resp.conv_from==CONVERT_BULLETS)
    {
        exchange1=RATE_BULLETS;
        currency1=&USER_BULLETS;
    }
     else if (ent->client->resp.conv_from==CONVERT_GRENADES)
    {
        exchange1=RATE_GRENADES;
        currency1=&USER_GRENADES;
    }
    else if (ent->client->resp.conv_from==CONVERT_ROCKETS)
    {
        exchange1=RATE_ROCKETS;
        currency1=&USER_ROCKETS;
    }
    else if (ent->client->resp.conv_from==CONVERT_CELLS)
    {
        exchange1=RATE_CELLS;
        currency1=&USER_CELLS;
    }
    else if (ent->client->resp.conv_from==CONVERT_SLUGS)
    {
        exchange1=RATE_SLUGS;
        currency1=&USER_SLUGS;
    }

    if ((*currency1)+ent->client->resp.conv_paid<exchange2)
        return;

     (*currency1)--;
    ent->client->resp.conv_paid++;

    if (ent->client->resp.conv_paid>=exchange2)
    {
        *currency2+=exchange1;
        if (*currency2>max)
            *currency2=max;
        else
            ent->client->resp.conv_paid=0;
    }
}

/* *** TIME - we're on your side *** */
char *tomtime(void)
{
    time_t t;
    struct tm   *lt;
    char        day[10];
    static char timestring[31];

    time(&t);
    lt=localtime(&t);
    switch (lt->tm_wday)
     {
        case 0:
            strcpy(day,"Sunday");
            break;
        case 1:
            strcpy(day,"Monday");
            break;
        case 2:
            strcpy(day,"Tuesday");
            break;
        case 3:
            strcpy(day,"Wednesday");
            break;
        case 4:
            strcpy(day,"Thursday");
            break;
        case 5:
            strcpy(day,"Friday");
            break;
        case 6:
            strcpy(day,"Saturday");
            break;
        default:
            strcpy(day,"");
                break;
    }
    sprintf(timestring,"%s %02d/%02d/%04d %02d:%02d:%02d",day,lt->tm_mday,lt->tm_mon+1,lt->tm_year+1900,lt->tm_hour,lt->tm_min,lt->tm_sec);

    return timestring;
}

/* *** (old) ANTI-ZBOT *** */
/*void foundzbot(edict_t *ent, byte impulse)
{
    if ((int)zbot_protect->value&ZBOT_PROTECT)
    {
        if ((int)zbot_protect->value&ZBOT_GLOBALMSG)
            gi.bprintf(PRINT_CHAT,"%s @ %s - possible ZBOT! (Impulse %d)\n",ent->client->pers.netname,Info_ValueForKey(ent->client->pers.userinfo, "ip"),impulse);
        else
            gi.dprintf("%s @ %s - possible ZBOT! (Impulse %d)\n",ent->client->pers.netname,Info_ValueForKey(ent->client->pers.userinfo, "ip"),impulse);

        if ((int)zbot_protect->value&ZBOT_KICK)
        {
            x_stuffcmd(ent,"clear;echo You were caught using a ZBOT!\n");
            gi.bprintf(PRINT_HIGH,"%s was kicked\n",ent->client->pers.netname);
            x_stuffcmd(ent,"echo You were kicked from the game;echo \"\";disconnect\n");
        }
    }
}*/

/* *** RADAR *** */
void Cmd_Radar_f(edict_t *ent)
{
    if (ent->client->showradar)     // switch rader off
        radar_off(ent);
    else                            // switch radar on
        radar_on(ent);
}

void Cmd_Radarzoomin_f(edict_t *ent)
{
    if (ent->client->showradar && ent->client->resp.radar_zoom>1024)
    {
        ent->client->resp.radar_zoom/=2;
        gi.cprintf(ent,PRINT_HIGH,"Radar zoomed in to %d units.\n",ent->client->resp.radar_zoom);
        radar_update(ent);
        gi.unicast(ent,true);
    }
}

void Cmd_Radarzoomout_f(edict_t *ent)
{
    if (ent->client->showradar && ent->client->resp.radar_zoom<8192)
    {
		ent->client->resp.radar_zoom*=2;
		gi.cprintf(ent,PRINT_HIGH,"Radar zoomed out to %d units.\n",ent->client->resp.radar_zoom);
		radar_update(ent);
		gi.unicast(ent,true);
	}
}

void radar_on(edict_t *ent)
{
	if (!((unsigned long)ann_allow2->value&ANN_ALLOW2_RADAR))
	{
		gi.cprintf(ent,PRINT_HIGH,"Radars are disallowed on this server!\n");
		return;
	}

	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))]<RADAR_CELLS)
	{
		gi.cprintf(ent,PRINT_HIGH,"No cells for radar!\n",RADAR_CELLS);
		return;
	}

	if (ent->client->menu)
		PMenu_Close(ent);

	ent->client->showscores=1;
	ent->client->showradar=1;

	if (ent->client->resp.radar_zoom==0)
		ent->client->resp.radar_zoom=RADAR_RANGE;

	gi.cprintf(ent,PRINT_HIGH,"Radar activated.\n");

	radar_update(ent);
	gi.unicast(ent,true);
}

void radar_off(edict_t *ent)
{
	ent->client->showradar=0;
	ent->client->showscores=0;

	gi.cprintf(ent,PRINT_HIGH,"Radar deactivated.\n");
}

void radar_update(edict_t *ent)
{
    char                string[4096];   // Will be truncated before sending to server
    char                string2[1400];
    edict_t             *dude=NULL;
    vec3_t              between,tempangles;
    int                 dist;
    float               angle,angle1;
    int                 x,y;
    char                *type;

    if (ent->client->radarframe>=RADAR_FRAMES-1)
    {
        if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))]>=RADAR_CELLS)
        {
            ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))]-=RADAR_CELLS;
        }
        else
        {
            gi.cprintf(ent,PRINT_HIGH,"Out of cells for radar.\n");
            radar_off(ent);
            return;
        }

        ent->client->radarframe=0;
    }
    else
    {
        ent->client->radarframe++;
    }

    type="xxxxxx";      // Premalloc in stack

#ifdef ANN_BORLAND_WIN32
#pragma warn -pia
#endif
    sprintf(string,"xv 80 yv 40 picn rpic xv 240 yv 200 strin%s \"%s\" ",(ent->client->radarcol=!ent->client->radarcol)?"g":"g2",itoa(ent->client->resp.radar_zoom,type,10));
#ifdef ANN_BORLAND_WIN32
#pragma warn .pia
#endif

    while ((dude=findradius(dude,ent->s.origin,ent->client->resp.radar_zoom))!=NULL)
    {
        if (!(dude->svflags & SVF_MONSTER) && !dude->client)
            continue;
        if (dude==ent)
            continue;
        if (dude->health<=0)
            continue;
        if (dude->client && ((int)ann_allow2->value & ANN_ALLOW2_RADARHIDECLOAKERS) && dude->client->cloakable==1 && (dude->svflags & SVF_NOCLIENT))
            continue;

        VectorSubtract(ent->s.origin,dude->s.origin,between);
        dist=(VectorLength(between)/ent->client->resp.radar_zoom)*80;

        vectoangles(between,tempangles);
        angle=anglemod(tempangles[YAW]-ent->s.angles[YAW]);

        if (angle>=0 && angle<90)           // Top-right quadrant
            angle1=90-angle;
        else if (angle>=90 && angle<180)    // Bottom-right quadrant
            angle1=angle-90;
        else if (angle>=180 && angle<270)   // Bottom-left quadrant
            angle1=270-angle;
        else if (angle>=270 && angle<360)   // Top-left quadrant
            angle1=angle-270;

        y=dist*degsin(angle1);
        x=sqrt((dist*dist)-(y*y));

        if (angle>=90 && angle<180)
        {
            y=-y;
        }
        else if (angle>=180 && angle<270)
        {
            y=-y;
            x=-x;
		}
        else if (angle>=270 && angle<360)
        {
            x=-x;
        }

        if (dude->svflags & SVF_MONSTER)
            type="ry";
        else if ((int) coop->value && dude->client)
            type="rg";
        else if ((int)deathmatch->value && dude->client)
        {
            if (ctf->value)
            {
                if (dude->client->resp.ctf_team==CTF_TEAM1)
                    type="rr";
                else if (dude->client->resp.ctf_team==CTF_TEAM2)
                    type="rb";
                else
                    type="rg";
            }
            else
            {
                if (OnSameTeam(ent,dude))
                    type="rg";
                else
                    type="ry";
            }
        }
        else
            type="ry";

        sprintf((string+strlen(string)),"xv %d yv %d picn %s ",158+x,118+y,type);
    }

    strncpy(string2,string,1399);
    gi.WriteByte(svc_layout);
    gi.WriteString(string2);
}

/* Use the degsin() macro
float degsin(float in)
{
    float rad;

    rad=in*PI/180;

    return sin(rad);
}*/

/* *** DISRUPTOR RIFLE *** */
void disruptor_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
     if (other == self->owner)
          return;

     if (surf && (surf->flags & SURF_SKY))
     {
          G_FreeEdict (self);
          return;
     }

     if (self->owner->client)
          PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

     if (other->takedamage)
     {
          gi.WriteByte(svc_temp_entity);
          gi.WriteByte(TE_WELDING_SPARKS);
          gi.WriteByte(15000);
          gi.WritePosition (self->s.origin);
          if (!plane)
                gi.WriteDir (vec3_origin);
          else
                gi.WriteDir (plane->normal);
          gi.WriteByte(0xb0b1b2b3);
          gi.multicast (self->s.origin, MULTICAST_PVS);

          T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, MOD_DISRUPTOR);

          G_FreeEdict (self);
     }
     else
     {
          gi.WriteByte(svc_temp_entity);
          gi.WriteByte(TE_WELDING_SPARKS);
          gi.WriteByte(200);
          gi.WritePosition (self->s.origin);
          if (!plane)
                gi.WriteDir (vec3_origin);
          else
                gi.WriteDir (plane->normal);
          gi.WriteByte(0xb0b1b2b3);
          gi.multicast (self->s.origin, MULTICAST_PVS);
     }
}

void disruptor_think(edict_t *ent)
{
    home_ent(ent,1);
}

/* *** NUKES *** */
void Nuke_RadiusDamage (edict_t *inflictor, edict_t *attacker, float damage, edict_t *ignore, float radius, int mod)
{
    float   points;
    edict_t *ent = NULL;
    vec3_t  v;
    vec3_t  dir;

    while ((ent = findradius(ent, inflictor->s.origin, radius)) != NULL)
    {
        if (ent == ignore)
            continue;
        if (!ent->takedamage)
			continue;

        VectorAdd (ent->mins, ent->maxs, v);
        VectorMA (ent->s.origin, 0.5, v, v);
        VectorSubtract (inflictor->s.origin, v, v);
        points = damage - 0.5 * VectorLength (v);
        if (ent == attacker)
            points = points * 0.5;
        if (points > 0)
        {
            if (CanDamage (ent, inflictor))
            {
                /* Added bit... */
                if (ent->client)
                    ent->client->nukeblind=NUKE_BLINDTIME;
                /**/
                VectorSubtract (ent->s.origin, inflictor->s.origin, dir);
                T_Damage (ent, inflictor, attacker, dir, inflictor->s.origin, vec3_origin, (int)points, (int)points, DAMAGE_RADIUS, mod);
            }
        }
    }
}

/* *** IR GOGGLES *** */
void Cmd_Goggles_f (edict_t *ent)
{
    if (ent->client->goggles) // we're on
    {
        ent->client->goggles = 0;
        ent->client->ps.rdflags &= ~RDF_IRGOGGLES;
        gi.cprintf(ent,PRINT_HIGH,"IR Goggles deactivated.\n");
    }
    else // we're off
    {
        if (!((unsigned long)ann_allow2->value&ANN_ALLOW2_IRGOGGLES))
        {
            gi.cprintf(ent,PRINT_HIGH,"IR Goggles are disallowed on this server!\n");
            return;
		}

        if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))]>=IR_CELLS)
        {
            ent->client->goggles = 1;
            ent->client->ps.rdflags |= RDF_IRGOGGLES;
            gi.cprintf(ent,PRINT_HIGH,"IR Goggles activated.\n");
        }
        else
        {
            gi.cprintf(ent,PRINT_HIGH,"No cells for IR Goggles.\n");
        }
    }
}

void draingoggles(edict_t *ent)
{
    if (ent->client->goggles)
    {
        if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] >= IR_CELLS)
        {
            ent->client->goggledrain ++;
            if (ent->client->goggledrain == IR_FRAMES)
            {
                ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] -= IR_CELLS;
                ent->client->goggledrain = 0;
            }
        }
        else
        {
            ent->client->ps.rdflags &= ~RDF_IRGOGGLES;
            ent->client->goggles = 0;
            gi.cprintf(ent,PRINT_HIGH,"IR Goggles deactivated - out of cells.\n");
        }
    }
}
/* *** */

/* *** MAP ROTATION *** */
void initrotate(void)
{
    FILE            *fp;
    char            filename[MAX_QPATH],buffer[256];
    cvar_t          *game_dir;
    int             line;
    unsigned int    i;

    game_dir = gi.cvar ("game", "", 0);

#ifdef  _WIN32
    i =  sprintf(filename, ".\\");
    i += sprintf(filename + i, game_dir->string);
    i += sprintf(filename + i, "\\");
    i += sprintf(filename + i, rotate_filename->string);
#else
    strcpy(filename, "./");
    strcat(filename, game_dir->string);
    strcat(filename, "/");
    strcat(filename, rotate_filename->string);
#endif

    fp=fopen(filename, "rt");
    if (!fp)
    {
        gi.cprintf(NULL,PRINT_HIGH,"Error loading %.20s (specified in rotate_filename) for map rotation data!\n",rotate_filename->string);
        maplist.maxindex=-1;
        return;
    }

	line=0;
	while (!feof(fp))
	{
		fgets(buffer,256,fp);
		// Remove trailing '\n'
		if (buffer[strlen(buffer)-1]=='\n')
			buffer[strlen(buffer)-1]='\0';
		else
			break;
		trimstring(buffer);

		if (buffer[0]=='\0' || buffer[0]=='#')
			continue;

		line++;
	}

	fseek(fp,0,SEEK_SET);

	maplist.mapname=gi.TagMalloc(MAX_QPATH*sizeof(char)*line,TAG_GAME);
	if (maplist.mapname==NULL)
	{
		gi.cprintf(NULL,PRINT_HIGH,"Out of memory processing maplist!\n");
		maplist.maxindex=-1;
		return;
	}

	maplist.mapused=gi.TagMalloc(sizeof(qboolean)*line,TAG_GAME);
	if (maplist.mapused==NULL)
	{
		gi.cprintf(NULL,PRINT_HIGH,"Out of memory processing maplist!\n");
		gi.TagFree(maplist.mapname);
		maplist.maxindex=-1;
		return;
	}

	maplist.maxindex=line;
	maplist.mapindex=-1;

	line=0;
	while (!feof(fp))
	{
		fgets(buffer,256,fp);
		// Remove trailing '\n'
		if (buffer[strlen(buffer)-1]=='\n')
			buffer[strlen(buffer)-1]='\0';
		else
			break;
		trimstring(buffer);

		if (buffer[0]=='\0' || buffer[0]=='#')
			continue;

		strncpy(&maplist.mapname[line*MAX_QPATH*sizeof(char)],buffer,MAX_QPATH);
		maplist.mapused[line]=0;

		line++;
	}

	gi.cprintf(NULL,PRINT_HIGH,"Added %d maps to rotation list.\n",line);

	fclose(fp);
}

void listmaps(void)
{
	int n;

	if (!maperr())
		return;

	gi.cprintf(NULL,PRINT_HIGH,"\nUsed - Map\n");

	for (n=0;n<maplist.maxindex;n++)
		gi.cprintf(NULL,PRINT_HIGH,"   %d   %s\n",maplist.mapused[n],&maplist.mapname[n*MAX_QPATH*sizeof(char)]);

	gi.cprintf(NULL,PRINT_HIGH,"\n%d maps in list\n\n",maplist.maxindex);
}

void firstmap(void)
{
	if (maperr())
		BeginIntermission(maprotate(true));
}

void nextmap(void)
{
	if (maperr())
		BeginIntermission(maprotate(false));
}

void reloadmaplist(void)
{
	if (!maperr())
		return;

	if (maplist.mapused!=NULL)
		gi.TagFree(maplist.mapused);

	if (maplist.mapname!=NULL)
		gi.TagFree(maplist.mapname);

	initrotate();
}

edict_t *maprotate(qboolean reset)
{
	edict_t *ent;
	int n,count;

	if (reset)
	{
		maplist.mapindex=0;
	}
	else if ((int)rotate->value&MR_SEQUENTIAL)
	{
		maplist.mapindex++;
		if (maplist.mapindex>=maplist.maxindex)
			maplist.mapindex=0;
	}
	else if ((int)rotate->value&MR_RANDOM)
	{
		for (n=0,count=0;n<=maplist.maxindex;n++)
			count+=maplist.mapused[n];

		if (count>=maplist.maxindex)
		{
			for (n=0;n<=maplist.maxindex;n++)
				maplist.mapused[n]=0;
		}

		do
		{
			n=(int)(random()*maplist.maxindex);
		}
		while (maplist.mapused[n]);

		maplist.mapindex=n;
	}

	maplist.mapused[maplist.mapindex]=1;

	ent=G_Spawn();
	ent->classname="target_changelevel";
	ent->map=&maplist.mapname[maplist.mapindex*MAX_QPATH*sizeof(char)];

	gi.cprintf(NULL,PRINT_HIGH,"Rotating to %s.map ...\n",&maplist.mapname[maplist.mapindex*MAX_QPATH*sizeof(char)]);

	return ent;
}

qboolean maperr(void)
{
	if (!deathmatch->value)
	{
		gi.cprintf(NULL,PRINT_HIGH,"Server must be running in deathmatch mode!\n");
		return false;
	}

	if (!rotate->value)
	{
		gi.cprintf(NULL,PRINT_HIGH,"Map rotation disabled!\n");
		return false;
	}

	return true;
}
/* *** */

void adminnotify(edict_t *ent, char *string1, char *string2)
{
    edict_t *player;
    int     i;
    char    buffer[1024];

    strcpy(buffer,"*");
    strcat(buffer,ent->client->pers.netname);
    strcat(buffer,"*: ");
    strcat(buffer,string1);
    strcat(buffer," ");
    strcat(buffer,string2);

    for_each_player(player, i)
    {
        if (player->client->resp.allow_devmode)
            gi.cprintf(player,PRINT_HIGH,"%s\n",buffer);
    }
}

