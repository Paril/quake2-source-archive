#include "bot.h"
#include "m_player.h"

void NoAmmoWeaponChange (edict_t *ent);
void CTFJoinTeam(edict_t *ent, int desired_team);
qboolean botTouchingLadder(edict_t *self);

vec3_t NOTHING_VEC	= {0, 0, 0};

char *ClientTeam (edict_t *ent);

int number_bot;
int current_number_bot;					//current of bots in the game
int cam_way0, cam_way1;					//deathcam x and y
int bot_point_stuff;					//loop number for bot_point
int TP_score_types[3], TP_start_bot;	//Teamplay (scoreboard and move)
int TP_player_team;						//Teamplay

int bot_total_msg_killer;						//Amount of messages for killer
int bot_total_msg_killed;						//Amount of messages for killed

int bot_name_total;
int def_team1, def_team2;				//ctf (amout of bot defening base)

int bot_loaded[20];

bot_message_string bot_msg_killer[60];
bot_message_string bot_msg_killed[100];
bot_message_string bot_names[300];

int TP_teamwithlowest_players(void)
{
	int male, female, cyborg, i;
	gclient_t	*cl;
	edict_t 	*cl_ent;
	
	male=0;
	female=0;
	cyborg=0;
	
	for (i=0 ; i<maxclients->value ; i++)
	{
		
		cl = game.clients + i;
		cl_ent = g_edicts + 1 + i;
		
		if (!g_edicts[i+1].inuse)
			continue;
		
		//add up the players
		if (Q_stricmp(ClientTeam(cl_ent),"male") == 0)
			male++;
		
		else if (Q_stricmp(ClientTeam(cl_ent),"female") == 0)
			female++;
		
		else		
			cyborg++;
		
	}
	
	
	//work out the team with the lowest players
	if ((male < female) && 
		((teams->value == 2) || (male < cyborg)) )
		return 0;
	if ((female < male) && 
		((teams->value == 2) || (female < cyborg)) )
		return 1;
	if ((teams->value == 3) &&
		(cyborg < male) && (cyborg < male)) 
		return 2;
	
	//all equal
	return 0;
}

void Bot_Create(void)
{
	int 	 i, r, line_nu;
	char	 userinfo[MAX_INFO_STRING], *skin, *name;
	edict_t *bot;	
	
	// Search for a free client slot
	for (i = maxclients->value; i > 0; i--)
	{
		bot = g_edicts + i + 1;
		if (!bot->inuse)
			break;
		// end if
	}
	// next
	if (bot->inuse)
		bot = NULL;
	// end if
	
	if (bot != NULL)
	{
		// Initialize userinfo
		memset(userinfo, 0, MAX_INFO_STRING);
		
		i=0;
		r = 0;
		
repick:
		i=0;
		r = random() * ((bot_name_total /2)-1);
		
		line_nu = r * 2;
		
		for (i=0;i<current_number_bot;i++)
			if (bot_loaded[i] == line_nu) goto repick;
			
			bot_loaded[current_number_bot]=line_nu;
			
			//Get a random skin for bot
			if ((int)(dmflags->value) & (DF_MODELTEAMS))
			{
				int bot_team;
				
				bot_team = TP_teamwithlowest_players();

				if (bot_team == -1) 
					return;
				
				if (bot_team == 0)
				{
					r = random()*6;
					
					if (r == 1)
					{
						skin = "male/grunt";
						name = "Grunt";
					}
					else if (r == 2)
					{
						skin = "male/cipher";
						name = "Cipher";
					}
					else if (r == 3)
					{
						skin = "male/flak";
						name = "Flak";
					}
					else if (r == 4)
					{
						skin = "male/pointman";
						name = "Pointman";
					}
					else if (r == 5)
					{
						skin = "male/major";
						name = "Major";
					}
					else 
					{
						skin = "male/sniper";
						name = "Sniper";
					}
				}
				if (bot_team == 1)
				{
					r = random()*7;
					
					if (r == 1)
					{
						skin = "female/voodoo";
						name = "Voodoo";
					}
					else if (r == 2)
					{
						skin = "female/stiletto";
						name = "Stiletto";
					}
					else if (r == 3)
					{
						skin = "female/athena";
						name = "Athena";
					}
					else if (r == 4)
					{
						skin = "female/jungle";
						name = "Jungle";
					}
					else if (r == 5)
					{
						skin = "female/jezebel";
						name = "Jezebel";
					}
					else if (r == 6)
					{
						skin = "female/brianna";
						name = "Brianna";
					}
					else
					{
						skin = "female/cobalt";
						name = "Cobalt";
					}
				}
				if (bot_team == 2)
				{
					r = random()*3;
					
					if (r == 1)
					{
						skin = "cyborg/ps9000";
						name = "Ps9000";
					}
					else if (r == 2)
					{
						skin = "cyborg/oni911";
						name = "Oni911";
					}
					else 
					{
						skin = "cyborg/tyr574";
						name = "Tyr574";
					}
				}
				
				
			} 
			else 
			{		
				skin = bot_names[line_nu+1];
				name = bot_names[line_nu];				
			} 
			
			current_number_bot++;
			
			//			name = bot_names[line_nu];				
			
			// Add bot's name/skin/hand to userinfo
			Info_SetValueForKey(userinfo, "name", name);
			Info_SetValueForKey(userinfo, "skin", skin);
			Info_SetValueForKey(userinfo, "hand", "2");   // Center handed
			
			ClientConnect(bot, userinfo);
			G_InitEdict(bot);
			InitClientResp(bot->client);
			ClientUserinfoChanged (bot, userinfo);
			
			// Put the bot into the game world
			Bot_Spawn(bot);
			
			// Send effects
			gi.WriteByte(svc_muzzleflash);
			gi.WriteShort(bot - g_edicts);
			gi.WriteByte(MZ_LOGIN);
			gi.multicast(bot->s.origin, MULTICAST_PVS);
			
			// Broadcast to let everyone know the bot is in!
			gi.bprintf(PRINT_CHAT, "%s", bot->client->pers.netname);
			gi.bprintf(PRINT_HIGH, " entered the game.\n");
			
			// Make sure all view stuff is valid
			ClientEndServerFrame(bot);		
			
			//make a light
			if (darkmatch->value)
				make_light(bot, bot->s.origin );
			
			if ((ctf->value) || (mbm->value))
			{
				if (TP_start_bot == 0)
				{
					CTFJoinTeam(bot, CTF_TEAM2);
					TP_start_bot = 1;
				}
				
				else if (TP_start_bot == 1)
				{
					CTFJoinTeam(bot, CTF_TEAM1);
					TP_start_bot = 0;
				}
			}
			
			
			
		}
		else
		{
			gi.dprintf("Cannot connect - server is full!\n");
		}
		// end if
	}
	// end of Bot_Create
	
	
	void Bot_Spawn(edict_t *ent)
	{
		vec3_t				 origin, angles;
		vec3_t				 mins = {-16, -16, -24};
		vec3_t				 maxs = {16, 16, 32};
		int 				 i, index;
		client_persistant_t  pers;	   // Note: wrong spelling for "persistent"!
		client_respawn_t	 resp;
		char userinfo[MAX_INFO_STRING];
		
		SelectSpawnPoint(ent, origin, angles);
		VectorCopy(origin, ent->s.origin);
		
		index = ent - g_edicts - 1;
		
		//if (deathmatch->value)	// Deathmatch wipes most client data every spawn
		
		
		resp = ent->client->resp;
		memcpy(userinfo, ent->client->pers.userinfo, MAX_INFO_STRING);
		InitClientPersistant(ent->client);
		ClientUserinfoChanged(ent, userinfo);				
		
		//	 else
		//	memset(&resp, 0, sizeof(client_respawn_t));
		// end if
		
		// Clear everything except the persistent data
		pers = ent->client->pers;
		memset(ent->client, 0, sizeof(gclient_t));
		ent->client->pers = pers;
		ent->client->resp = resp;
		
		// Copy some data from the client to the entity
		FetchClientEntData(ent);
		
		// Fill in entity values
		ent->groundentity	  = NULL;
		ent->client 		  = &game.clients[index];
		ent->takedamage 	  = DAMAGE_AIM;
		ent->movetype		  = MOVETYPE_WALK;
		ent->viewheight 	  = 22;
		ent->inuse			  = true;
		ent->classname		  = "bot";
		//ent->mass 			= 20;
		ent->solid			  = SOLID_BBOX;
		ent->deadflag		  = DEAD_NO;
		ent->air_finished	  = level.time + 12;
		ent->clipmask		  = MASK_PLAYERSOLID;
		ent->think			  = Bot_Think;
		ent->touch			  = NULL;
		ent->pain			  = Bot_Pain;
		ent->die			  = player_die;
		ent->waterlevel 	  = 0;
		ent->watertype		  = 0;
		ent->flags			 &= ~FL_NO_KNOCKBACK;
		ent->enemy			  = NULL;
		ent->movetarget 	  = NULL;
		ent->bot_stuff		  = NULL;
		ent->bot_fow		  = NULL;
		ent->head_shot		  = 0;
		ent->kills_in_time	  = 0;
		ent->killing_time	  = 0;		
		
		ent->svflags		 &= ~SVF_DEADMONSTER;
		ent->flags			 &= ~FL_NO_KNOCKBACK;		
		
		VectorCopy(mins, ent->mins);
		VectorCopy(maxs, ent->maxs);
		VectorClear(ent->velocity);
		
		// Clear player_state values
		memset(&ent->client->ps, 0, sizeof(player_state_t));
		
		// Set up orientation and delta angles
		for (i = 0; i < 3; i++)
		{
			ent->client->ps.pmove.origin[i] = origin[i] * 8;
			ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(angles[i] - ent->client->resp.cmd_angles[i]);
		}
		// next
		
		ent->client->ps.fov = 90;
		ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);
		
		// Clear entity state values
		ent->s.effects		  = 0;
		ent->s.skinnum		  = index;
		ent->s.modelindex	  = 255;	  // will use the skin specified model
		ent->s.modelindex2	  = 255;	  // custom gun model
		ent->s.modelindex3	  = 0;
		ent->s.frame		  = 0;
		
		ent->s.origin[2]++; 			  // make sure off ground
		
		ent->s.angles[PITCH]  = 0;
		ent->s.angles[YAW]	  = angles[YAW];
		ent->s.angles[ROLL]   = 0;
		
		VectorCopy(ent->s.angles, ent->client->ps.viewangles);
		VectorCopy(ent->s.angles, ent->client->v_angle);
		
		gi.unlinkentity(ent); // Must do this before KillBox
		KillBox(ent);		  // Telefrag!
		gi.linkentity(ent);   // Tada!	In the game world!
		
		// Force the current weapon up
		ent->client->newweapon = ent->client->pers.weapon;	
		
		ChangeWeapon(ent);
		
		ent->nextthink = level.time + FRAMETIME;
	}
	// end of ent_Spawn
	
	
	void Bot_Respawn(edict_t *ent)
	{
		CopyToBodyQue(ent);
		
		Bot_Spawn(ent);
		
		ent->s.event = EV_PLAYER_TELEPORT;
	}
	// end of Bot_Respawn
	
	
	void Bot_Think(edict_t *ent)
	{
		usercmd_t	cmd;
		vec3_t		angles = { 0, 0, 0 }, test1, test2, test3, test4, dist;
		float r;
		int r2, wipe_fow, high;
		qboolean bot_Frames;
		edict_t *newfow;
		
		
		newfow = NULL;
		
		VectorClear(angles);
		VectorClear(test1);
		VectorClear(test2);
		VectorClear(test3);
		VectorClear(test4);
		VectorClear(dist);
		
		VectorCopy(ent->client->v_angle, angles);
		VectorSet(ent->client->ps.pmove.delta_angles, 0, 0, 0);
		memset(&cmd, 0, sizeof(usercmd_t)); 
		wipe_fow=0;
		
		//copy the player wave, point, salute or taunt, if playing a coop game
		
		if ((coop->value) && (ent->bot_fow != NULL) && (ent->enemy == NULL) && (ent->bot_stuff == NULL))
			if ((strcmp(ent->bot_fow->classname, "player") == 0) && (visible(ent,ent->bot_fow)) 
				
				&& ((ent->bot_fow->s.frame == FRAME_taunt01) || 
				(ent->bot_fow->s.frame == FRAME_wave01-1) || 
				(ent->bot_fow->s.frame == FRAME_point01-1) || 
				(ent->bot_fow->s.frame == FRAME_flip01-1) || 
				(ent->bot_fow->s.frame == FRAME_salute01-1)))
			{
				ent->s.frame = ent->bot_fow->s.frame;
				ent->client->anim_end = ent->bot_fow->client->anim_end;
			}
			
			//Someone died, taunt or salute them, if monster then salute
			if ((ent->enemy != NULL) && (ent->health > 10) && (ent->bot_fow != NULL) && (ent->bot_stuff == NULL) && (ent->deadflag != DEAD_DEAD))
			{
				if ((ent->enemy->health <1) && (visible(ent,ent->enemy)) && (ent->health > 0))
				{
					if (ctf->value || deathmatch->value)
					{				
						Bot_Aim(ent, ent->enemy, angles);
						ent->count = 100;
						ent->client->killer_info = ent->enemy;
					}			
					else if (coop->value)
					{
						if (strcmp(ent->bot_fow->classname, "player") == 0)
						{
							Bot_Aim(ent, ent->bot_fow, angles);
							ent->count = 0;
						}
					}
					
					r = random();
					
					if ((r <= 0.25) && (ctf->value || deathmatch->value))
					{
						ent->s.frame = FRAME_taunt01-1;
						ent->client->anim_end = FRAME_taunt17;
					} 
					if ((r <= 0.5) && (ctf->value || deathmatch->value))
					{
						ent->s.frame = FRAME_flip01-1;
						ent->client->anim_end = FRAME_flip12;
					} 
					else if ((r <= 0.75) || (!ctf->value && !deathmatch->value)) 
					{
						ent->s.frame = FRAME_salute01;
						ent->client->anim_end = FRAME_salute11;
					}		
				}
			}
			
			//bot messages
			if ((ent->count == 115) && (ent->client->killer_info != NULL))
			{
				int i;
				char outmsg[1024];
				char *msg;
				char *p;
				
				i = (random()*bot_total_msg_killer)+1;
				
				if ((i <1) || (i>bot_total_msg_killer))
					i = (random()*bot_total_msg_killer)+1;
				
				msg=bot_msg_killer[i];
				
				outmsg[0] = 0;
				
				if (*msg == '\"') 
				{
					msg[strlen(msg) - 1] = 0;
					msg++;
				}
				
				for (p = outmsg; *msg && (p - outmsg) < sizeof(outmsg) - 1; msg++) 
				{
					if (*msg == '%') 
					{
						switch (*++msg) 
						{
							
							//other's name (who died)
						case 'n' :
							if ((ent->client->killer_info != NULL) &&
								(ent->client->killer_info->client != NULL))
							{
								strcpy(p, ent->client->killer_info->client->pers.netname);
								p += strlen(ent->client->killer_info->client->pers.netname);
							}
							break;
							//current weapon
						case 'w' :	
							if ((ent->client != NULL) &&
								(ent->client->pers.weapon != NULL))
							{
								strcpy(p, ent->client->pers.weapon->pickup_name);
								p += strlen(ent->client->pers.weapon->pickup_name);
							}
							break;
							//my name (attacker)
						case 's' :
							if (ent->client != NULL)
							{
								strcpy(p, ent->client->pers.netname);
								p += strlen(ent->client->pers.netname);
							}
							break;
						case 'm' :
							//current map name
							if (*level.level_name)
							{
								strcpy(p, level.level_name);
								p += strlen(level.level_name);	
							}
							break;
						default :
							*p++ = *msg;
							
						}
					} else
						*p++ = *msg;
				}
				*p = 0;
								
				gi.bprintf(PRINT_CHAT, "%s: %s\n",ent->client->pers.netname, outmsg);
				
				if (strcmp(ent->client->killer_info->classname, "player") == 0)
				{
					ent->count = 0; 
					ent->bot_message = 0;			
				} else					
					ent->bot_message = (i * 2);
				
			}
			
			if ((ent->count == 150) && (ent->client->killer_info != NULL))
			{
				char outmsg[1024];
				char *msg;
				char *p;
				
				if ((ent->bot_message > bot_total_msg_killed) || (ent->bot_message < 2))
					ent->bot_message=3;
				
				if (random() <= 0.5)				
					msg=bot_msg_killed[ (ent->bot_message)];
				else
					msg=bot_msg_killed[ (ent->bot_message-1)];
				
				outmsg[0] = 0;
				
				if (*msg == '\"') 
				{
					msg[strlen(msg) - 1] = 0;
					msg++;
				}
				
				for (p = outmsg; *msg && (p - outmsg) < sizeof(outmsg) - 1; msg++) 
				{
					if (*msg == '%') 
					{
						switch (*++msg) 
						{
							
							//other's name (who died)
						case 'n' :
							if ((ent->client->killer_info != NULL) &&
								(ent->client->killer_info->client != NULL))
							{
								strcpy(p, ent->client->killer_info->client->pers.netname);
								p += strlen(ent->client->killer_info->client->pers.netname);
							}
							break;
							//current weapon
						case 'w' :	
							if ((ent->client != NULL) &&
								(ent->client->pers.weapon != NULL))
							{
								strcpy(p, ent->client->pers.weapon->pickup_name);
								p += strlen(ent->client->pers.weapon->pickup_name);
							}
							break;
							//my name (attacker)
						case 's' :
							if ((ent != NULL) && (ent->client != NULL))
							{
								strcpy(p, ent->client->pers.netname);
								p += strlen(ent->client->pers.netname);
							}
							break;
						case 'm' :
							//current map name
							if (*level.level_name)
							{
								strcpy(p, level.level_name);
								p += strlen(level.level_name);						
							}
							break;
						default :
							*p++ = *msg;
							
						}
					} else
						*p++ = *msg;
				}
				*p = 0;
				
				gi.bprintf(PRINT_CHAT, "%s: %s\n", ent->client->killer_info->client->pers.netname, outmsg);
				
				ent->count = 0;
				ent->client->killer_info = NULL;			
			}				
			
			//keep counting..up
			if (ent->count >= 100)
				ent->count++;
			
			if ((ent->count > 200) || (ent->count < 0))
				ent->count = 0;
			
			//look for new enemy, follow thing or item			
			Bot_FindStuff(ent);
			//if playing ctf then check base status
			check_for_stay_at_base(ent);
			
			//part one of the thinking stage (if)
			if (ent->deadflag == DEAD_DEAD)
			{
				ent->client->buttons = 0;
				cmd.buttons = BUTTON_ATTACK;   // This will cause a respawn
			}
			else if ((ent->enemy != NULL) && (ent->enemy->health > 0) && (ent->count <10))
			{	
				if (!visible(ent,ent->enemy))
					ent->count++;		
				
				if (bot_skill->value == 2)
				{
					r = random();
					if (r < 0.15)		 
						cmd.upmove = 400;  // jump
					else if (r < 0.3)
						cmd.upmove = -400;	// duck!
				}
				
				Bot_Aim(ent, ent->enemy, angles);
				
				Bot_Attack(ent, &cmd, angles); // Attack enemy! 
				
				
			}
			
			//check the bot frames
			bot_Frames = bot_check_frame(ent);

            if ((ent->bot_fow != NULL) && (ent->health > 0))
			{       
                            
				if ((Q_stricmp(ent->bot_fow->classname, "bot_point") == 0))
				{
//					high = ent->bot_fow->s.origin[2] - ent->s.origin[2];
                                
					if (!visible(ent, ent->bot_fow)) 
						wipe_fow = 1;
					//if ((high > 100) && (ent->waterlevel == 0) && (ent->ladder==0)) // || (!bot_test_ground(ent))))
						//  wipe_fow = 1;

					if (VectorCompare (ent->bot_fow->s.origin, NOTHING_VEC))
						wipe_fow = 1;
				}
			}
                       
			if (wipe_fow == 1)
				ent->bot_fow = NULL;
						
			
			if ((ent->bot_stuff != NULL)
				&& (ent->health > 0)
				&& (visible(ent, ent->bot_stuff))
				&& (!bot_test_ground(ent))
				&& (!bot_Frames))
			{
				if ((ctf->value) && (ent->waiting_return_flag == 1)) ctf_wait_base(ent,angles);
				else
					bot_move(ent, ent->bot_stuff, angles);								
			}	
			
			else if ((ent->bot_fow != NULL) 
				&& ((ent->bot_fow->health > 0) || (strcmp(ent->bot_fow->classname, "player") != 0))
				&& (ent->health > 0)
				&& (!bot_test_ground(ent))
				&& (visible(ent, ent->bot_fow))
				&& (!bot_Frames))
			{				
				int distance;
				//vec3_t forward, temp, move;
				
				VectorSubtract ( ent->bot_fow->s.origin, ent->s.origin, dist);
				distance = VectorLength(dist);				
				
				//AngleVectors (angles, forward, NULL, NULL);
				//VectorScale (forward, 2, temp);
				//VectorAdd (temp, ent->s.origin, move);
				
				//	if ((ctf->value) && (distance <= 100) && ((ent->bot_fow->classname == "item_flag_team1") || (ent->bot_fow->classname == "item_flag_team2")))
				//		angles[1] +=5;
				//bot_move(ent, ent->bot_fow, angles);
				
				//			else if ((ctf->value) && (distance > 100) && ((ent->bot_fow->classname == "item_flag_team1") || (ent->bot_fow->classname == "item_flag_team2")))
				//					bot_move(ent, ent->bot_fow, angles);
				
				//else if ((ctf->value) && ((ent->bot_fow->classname =="player")||(ent->bot_fow->classname =="bot")))
				//					{
				//			Bot_Aim(ent, ent->bot_fow, angles);
				//						bot_move(ent, ent->bot_fow, angles);
				//					}
				//else 
				if ((strcmp(ent->bot_fow->classname, "player") == 0) && ((coop->value) ||(OnSameTeam(ent,ent->bot_fow))) )//!(deathmatch->value || ctf->value || mbm->value) ))
				{						
					if (distance > 120)
						bot_move(ent, ent->bot_fow, angles);

					if (distance < 60)
					{
						angles[1] = ent->bot_fow->s.angles[1];
						cmd.forwardmove = 400;
					}
				}

				//else if (Q_stricmp(ent->bot_fow->classname, "bot_point")==0) 
				else if (strcmp(ent->bot_fow->classname, "bot_point") == 0)
				{			
					
					int loop;
					int testtimes;
					
					testtimes=100;
					loop =0;	
					
					VectorSubtract ( ent->bot_fow->s.origin, ent->s.origin, dist);
					distance = VectorLength(dist);
					
		//			cmd.forwardmove = 400;
					
					if ((ent->air_finished > (level.time + 5)) )//&& (ent->ladder == 0))
						bot_move(ent, ent->bot_fow, angles);
					//distance <45
					if	((distance < 45))// && (ent->ladder == 0))//|| ((distance < 150) && (ent->ladder > 0)))
					{
						int p;
						char newtarget[128] = { 0 };
						
						p = atoi(ent->bot_fow->targetname);
						
retest:
						if (p == bot_point_stuff)
							ent->bot_fow_way = 1;
						else //if  (p == 2)
							ent->bot_fow_way = 0;
						
						if ((ent->bot_fow_way == 0) && (p < bot_point_stuff))
							p++;
						else if ((ent->bot_fow_way == 1) && (p > 2))
							p--;

						loop++;
						sprintf(newtarget, "%i_route", p);

						newfow = NULL;			
						
						if ((p <= bot_point_stuff) && (p >2))	
						{
							if (ent->bot_fow_way == 0)			
								newfow = G_Find (ent->bot_fow, FOFS(targetname), newtarget);
							else if (ent->bot_fow_way == 1)
								newfow = G_Find (NULL, FOFS(targetname), newtarget);
						}
						
						//test
						if (newfow != NULL)
						{
							high = newfow->s.origin[2] - ent->s.origin[2];
														
							if ((VectorCompare (newfow->s.origin, NOTHING_VEC))&& (loop < testtimes))
								goto retest;
							
							if (!visible(ent, newfow) && (loop < testtimes))
								goto retest;
							
//							if ( (ent->ladder==0)&& (high > 100) && (ent->waterlevel == 0) && (loop < testtimes))
//								goto retest;
							
							if (VectorCompare (newfow->s.origin, NOTHING_VEC))
								goto retest;
							
							if (loop < testtimes) 
								ent->bot_fow = newfow;
							else
								wipe_fow =1;
						} else if ((newfow == NULL) && (loop < testtimes))
							goto retest;
						else
							newfow = NULL;
						
						
					}
					
				}
				
					}
					
					else if ((!bot_Frames) && 
						(ent->bot_fow == NULL) && (ent->bot_stuff == NULL)
						&& (!bot_test_ground(ent)) 
//						(!botTouchingLadder(ent))
						)
						
					{							
						VectorCopy(ent->s.origin, test1);						
						
						test1[2]+=32;
						VectorCopy(test1, test2);
						VectorCopy(test1, test3);		
						VectorCopy(test1, test4);
						test1[0] += 32; 		 
						test2[1] += 32;
						test3[0] -= 32; 		 
						test4[1] -= 32;
						
						if ( (VectorCompare(ent->old_place, ent->s.origin)) || 
							((gi.pointcontents(test1) & CONTENTS_SOLID) == 1)|| 
							((gi.pointcontents(test2) & CONTENTS_SOLID) == 1)||
							((gi.pointcontents(test3) & CONTENTS_SOLID) == 1)||
							((gi.pointcontents(test4) & CONTENTS_SOLID) == 1) )
						{
							r2= (random() *60)+15;
							r = random();
							if (r <0.7)
								angles[1]+=r2;
							else if (r < 0.85)
								angles[1]-=r2;
							else
								cmd.upmove = -400;
							cmd.forwardmove = 400;			
						}
						
						VectorCopy(ent->s.origin, ent->old_place);
						cmd.forwardmove = 400;
					}
					
					//ctf stuff (defense)		
					/*			if ((ent->bot_fow != NULL) && (ctf->value))
					if ((!visible(ent, ent->bot_fow)) && ((Q_stricmp(ent->bot_fow->classname, "item_flag_team1") == 0) || (Q_stricmp(ent->bot_fow->classname, "item_flag_team2") == 0)))
					{
					if (ent->client->resp.ctf_team == CTF_TEAM1)
					def_team1--;
					else if (ent->client->resp.ctf_team == CTF_TEAM2)
					def_team2--;
					
					  ent->bot_fow = NULL;
					  }
					*/				
					//try going up for air
					if (ent->air_finished < (level.time + 5))
						cmd.upmove = 400;
					
					//air running out, try moving forward
					if (ent->air_finished < (level.time + 3))
						cmd.forwardmove = 400;
					
					//check to see if bot can follow a bot_point

					
					if (!bot_test_ground(ent))
					{

						
						if (VectorCompare (ent->s.origin, NOTHING_VEC))
							player_die (ent, ent, ent, 100000, vec3_origin);
//						if (gi.pointcontents(ent->s.origin) & CONTENTS_SOLID)
//							player_die (ent, ent, ent, 100000, vec3_origin);
						
						if	((ent->waterlevel != 0) &&
							((ent->watertype & CONTENTS_LAVA) == 8)||
							((ent->watertype & CONTENTS_SLIME) == 16))
						{
							cmd.upmove = 400;
							cmd.forwardmove = 400;
						}
						
						
					}						
					
					if (ent->groundentity != NULL) 
						if ((Q_stricmp(ent->groundentity->classname, "func_plat")==0) && (ent->groundentity->moveinfo.state == 1))
							cmd.forwardmove =20;
						
						//clear the up/down angles if not firing at a enemy
						if ( (ent->enemy == NULL) && (angles[0] != 0))
							angles[0] = 0;
						
						//if you are in the way duck
						if ((coop->value)&&(ent->monsterinfo.aiflags & AI_DUCKED))
						{
							cmd.upmove = -400;
							ent->count2=20;
							ent->monsterinfo.aiflags =0;
						}
						if (ent->count2 > 0)
						{
							if (coop->value)
								cmd.upmove = -400;
							
							ent->count2--;
						}
						
						// Approximate the time it takes for a client to render the current frame
						cmd.msec = 100;
						
						// View angle
						cmd.angles[PITCH] = ANGLE2SHORT(angles[PITCH]);
						cmd.angles[YAW]   = ANGLE2SHORT(angles[YAW]);
						cmd.angles[ROLL]  = ANGLE2SHORT(angles[ROLL]);
						
						// Tell the game engine to process the bot's command
						ClientThink(ent, &cmd);
						
						//	ent->client->ping = 15 + (random() * 10);
						ent->nextthink = level.time + 0.1;
						
}
// end of Bot_Think


void Bot_Aim(edict_t *ent, edict_t *target, vec3_t angles)
{
	vec3_t dir;
	
	VectorSubtract(target->s.origin, ent->s.origin, dir);
	vectoangles(dir, angles);	
}
// end of Bot_Aim


void Bot_Attack(edict_t *ent, usercmd_t *cmd, vec3_t angles)
{
	
	if ((ent->enemy->deadflag == DEAD_DEAD) || (ent->enemy->health < 1))
	{
		ent->enemy = NULL;	  // No need to mess with the dead		
	}
	else if (ent->enemy != NULL)
	{				
		if (ent->oldenemy != ent->enemy)
		{
			int i;
			
			NoAmmoWeaponChange (ent);
			//ChangeWeapon(ent);
			
			ent->client->pers.lastweapon = ent->client->pers.weapon;
			ent->client->pers.weapon = ent->client->newweapon;
			ent->client->newweapon = NULL;
			ent->client->machinegun_shots = 0;
			
			// set visible model
			if (ent->s.modelindex == 255) 
			{
				if (ent->client->pers.weapon)
					i = ((ent->client->pers.weapon->weapmodel & 0xff) << 8);
				else
					i = 0;
				ent->s.skinnum = (ent - g_edicts - 1) | i;
			}
			ent->client->weaponstate = WEAPON_ACTIVATING;
			ent->client->ps.gunframe = 0;
			ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);
		}
		
		ent->oldenemy = ent->enemy;
		
		//if player or bot, duck with them
		if (strcmp(ent->enemy->classname, "player") == 0 || strcmp(ent->enemy->classname, "bot") == 0)
			if ((ent->enemy->client->ps.pmove.pm_flags & PMF_DUCKED))
				cmd->upmove = -400;
			
			//if monster, then duck with them
			if (ent->enemy->svflags & SVF_MONSTER)
				if (ent->enemy->monsterinfo.aiflags & AI_DUCKED)
					cmd->upmove = -400;
				
				//return;
				if ((Q_stricmp(ent->client->pers.weapon->pickup_name, "grenades") == 0)||
					(Q_stricmp(ent->client->pers.weapon->pickup_name, "Grenade Launcher") == 0)) 
				{
					int distance;
					vec3_t dist;
					
					// grenade blow up on you if you don't let go!!
					if (random() < 0.1) 
						cmd->buttons = BUTTON_ATTACK;							
					
					VectorSubtract ( ent->s.origin,ent->enemy->s.origin, dist);
					distance = VectorLength(dist);	
					
					angles[PITCH] -= 15 * ((distance < 384) ? ((distance / 384) * 2) - 1: 1);
					
				}
				else if (bot_skill->value == 0)
				{
					if (random() < 0.4)
						cmd->buttons = BUTTON_ATTACK;  // Make an attack!
				}
				else
					cmd->buttons = BUTTON_ATTACK;  // Make an attack!
				
				
				//skill adjustment
				if ( (bot_skill->value < 2) && ( (deathmatch->value)||(ctf->value) ))
				{
					int i3;
					i3=(random() * ((2-bot_skill->value)*10));
					
					if (random() < 0.5)
						angles[1] += i3;
					else
						angles[1] -= i3;
				} else if ((bot_skill->value == 2) && ((Q_stricmp(ent->client->pers.weapon->pickup_name, "Rocket Launcher") == 0)) )
					//aim lower for to make the rocket hit the floor
					angles[0]+=5;
				else if (bot_skill->value == 2)
					//if in hard difrecult and have a head shot weapon, aim for head
					if (ent->client->pers.weapon != NULL)
						if ( (strcmp(ent->client->pers.weapon->pickup_name, "Shotgun") == 0) || (strcmp(ent->client->pers.weapon->pickup_name, "Railgun") == 0) || (strcmp(ent->client->pers.weapon->pickup_name, "Super Shotgun") == 0) )
							angles[2] = ent->enemy->s.origin[2] + 40 + ent->enemy->viewheight;
						//aim for head
						
	}
	// end if
}
// end of Bot_Attack


void Bot_Pain(edict_t *ent, edict_t *other, float kickback, int damage)
{
	// The attacker becomes the new enemy
	
	if ((other != NULL) && (ent->enemy != NULL))
		if ((ent != other) && (ent->health > 0) && (other->health > 0) &&
			((!coop->value) && ((strcmp(ent->enemy->classname, "player") == 0)||(strcmp(ent->enemy->classname, "bot") == 0))))
			ent->enemy	  = other;			 
			/*	if (((other != NULL) && (other != ent)) &&
			((!coop->value) && ((ent->enemy->classname == "player")||(ent->enemy->classname == "bot"))))
		ent->enemy = other;*/
		
		player_pain(ent, other, kickback, damage);	// Just use the normal pain function
}
// end of Bot_Pain


void Bot_FindStuff(edict_t *ent)
{
	int range = 500, armor_amount;
	edict_t *newstuff = NULL;
	gitem_t *item;
	int 	index, bullets, shells, cells, slugs, rockets, grenades, amount, distance;
	vec3_t	temp, dist; 	
	
	//int count = 0;
	
	if (ent->enemy != NULL)
		if ((ent->enemy->health < 1) || ((ent->count >=10)))
			ent->enemy	   = NULL;
		
		ent->bot_stuff = NULL;
		
		armor_amount = ent->client->pers.inventory[ArmorIndex (ent)];
		
		item = FindItem("Bullets");
		if (item)
		{
			index = ITEM_INDEX(item);
			bullets = ent->client->pers.inventory[index];
		}
		item = FindItem("Shells");
		if (item)
		{
			index = ITEM_INDEX(item);
			shells = ent->client->pers.inventory[index];		
		}
		item = FindItem("Cells");
		if (item)
		{
			index = ITEM_INDEX(item);
			cells = ent->client->pers.inventory[index];
		}
		
		item = FindItem("Slugs");
		if (item)
		{
			index = ITEM_INDEX(item);
			slugs = ent->client->pers.inventory[index];
		}
		item = FindItem("Rockets");
		if (item)
		{
			index = ITEM_INDEX(item);
			rockets = ent->client->pers.inventory[index];
		}
		item = FindItem("Grenades");
		if (item)
		{
			index = ITEM_INDEX(item);
			grenades = ent->client->pers.inventory[index];
		}
		
		
		if (ent->bot_fow != NULL)
			if ((strcmp(ent->bot_fow->classname, "player") == 0) && (ent->bot_fow->health < 1))
				ent->bot_fow = NULL;
			
			// Scan the perimeter for an item
			while ((newstuff = findradius(newstuff, ent->s.origin, range)) != NULL)
			{
				
				if ((newstuff != ent) && (newstuff != NULL))
				{		
					
					
					if (visible(ent, newstuff))
					{
						//attack!
						if	(((((Q_stricmp(newstuff->classname, "player") == 0) || 
							(Q_stricmp(newstuff->classname, "bot") == 0))
							&& (ctf->value || deathmatch->value)) || (newstuff->svflags & SVF_MONSTER))
							&& (newstuff->deadflag != DEAD_DEAD) 
							&& (newstuff->health > 0) &&
							(!OnSameTeam(ent,newstuff)))	
						{
							ent->enemy = newstuff;
							ent->count = 0; 						
						}
						
						if (ctf->value) 
						{
							//get the enemy flag
							if (((ent->client->resp.ctf_team == CTF_TEAM1) &&
								(Q_stricmp(newstuff->classname, "item_flag_team2") == 0) &&
								(newstuff->solid == 1)) ||
								((ent->client->resp.ctf_team == CTF_TEAM2) &&
								(Q_stricmp(newstuff->classname, "item_flag_team1") == 0) &&
								(newstuff->solid == 1)) ||
								
								//a droped flag, go collect it
								((Q_stricmp(newstuff->classname, "item_flag_team1") == 0) ||
								(Q_stricmp(newstuff->classname, "item_flag_team2") == 0)) &&
								(newstuff->spawnflags & DROPPED_ITEM))	
								
								ent->bot_stuff = newstuff;
						}
						
						//follow player about in dm
						if ((((deathmatch->value && !ctf->value) && (OnSameTeam(ent,newstuff))) || coop->value) && (Q_stricmp(newstuff->classname, "player") == 0) && (newstuff->health > 0))
						{
							if (ent->bot_fow != newstuff )
							{
								char *temp;
								float r;
								
								//salute the player
								ent->s.frame = FRAME_salute01;
								ent->client->anim_end = FRAME_salute11;
								
								//say a random message to the player
								if (darkmatch->value)
									temp = "Need an light? ";
								else
								{
									r = random() * 6;
									if (r == 1)
										temp = "hi";
									else if (r == 2)
										temp = "show me the way";
									else if (r == 3)
										temp = "lead the way";
									else if (r == 4)
										temp = "ready to kick some ass";
									else if (r == 5)
										temp = "it's good to see you";
									else 
										temp = "hello";
								}
								gi.cprintf(newstuff, PRINT_CHAT,"%s: ",ent->client->pers.netname);
								gi.cprintf(newstuff, PRINT_HIGH,"%s %s.\n", temp, newstuff->client->pers.netname);
							}
							
							ent->bot_fow = newstuff;
						}
						
						//calute highs and distance of the new object
						temp[2] = newstuff->s.origin[2] - ent->s.origin[2];
						VectorSubtract ( ent->s.origin,newstuff->s.origin, dist);
						distance = VectorLength(dist);
						
						//if (distance < 400)
						if ((Q_stricmp(newstuff->classname, "bot_point") == 0) && (ent->bot_fow == NULL) && (temp[2] < 50))
							ent->bot_fow = newstuff;
						
						if ((Q_stricmp(newstuff->classname, "misc_teleporter") == 0) && (distance < 300))
							ent->bot_stuff = newstuff;				
						
						if ((newstuff->solid == 1) && (temp[2] < 55))
						{
							
							if (((int)dmflags->value & DF_WEAPONS_STAY) || (coop->value))	
								amount = 0;
							else 
								amount = 50;
							
							//only find the above item if close to it
							if (((distance < 340) || (ent->waterlevel != 0)) && ((newstuff->item != NULL)))
							{
								if	(
									//some ammo, maybe?
									((newstuff->item->flags & IT_AMMO) &&
									((Q_stricmp(newstuff->classname, "ammo_shells") == 0) && (shells < 100)) ||
									((Q_stricmp(newstuff->classname, "ammo_bullets") == 0) && (bullets < 200)) ||
									((Q_stricmp(newstuff->classname, "ammo_cells") == 0) && (cells < 200)) ||
									((Q_stricmp(newstuff->classname, "ammo_slugs") == 0) && (slugs < 50)) ||
									((Q_stricmp(newstuff->classname, "ammo_rockets") == 0) && (rockets < 50)) ||
									((Q_stricmp(newstuff->classname, "ammo_grenades") == 0) && (grenades < 50)))||
									
									//new weapon
									((newstuff->item->flags & IT_WEAPON) && ((ent->client->pers.inventory[ITEM_INDEX(FindItem(newstuff->item->pickup_name))] <= amount ))) ||
									
									//or a powerup
									((newstuff->item->flags & IT_POWERUP) && ((ent->client->pers.inventory[ITEM_INDEX(FindItem(newstuff->item->pickup_name))] <= amount ))) ||
									
									//something else, maybe?
									((Q_stricmp(newstuff->classname, "item_adrenaline") == 0) && (ent->health < 100)) ||
									(Q_stricmp(newstuff->classname, "item_bandolier") == 0) ||
									(Q_stricmp(newstuff->classname, "item_pack") == 0) ||
									((Q_stricmp(newstuff->classname, "item_health") == 0) && (ent->health < 100)) ||
									((Q_stricmp(newstuff->classname, "item_health_small") == 0) && (ent->health < 200)) ||
									((Q_stricmp(newstuff->classname, "item_health_mega") == 0) && (ent->health < 200)) ||
									
									//hmmm, some nice, new armour
									((newstuff->item->flags & IT_ARMOR) &&
									((Q_stricmp(newstuff->classname, "item_armor_body") == 0) && (armor_amount < 100))	||
									((Q_stricmp(newstuff->classname, "item_armor_combat") == 0) && (armor_amount < 50)) ||
									((Q_stricmp(newstuff->classname, "item_armor_jacket") == 0) && (armor_amount < 25)) ||
									((Q_stricmp(newstuff->classname, "item_armor_shard") == 0) && (armor_amount < 200)) ||
									((Q_stricmp(newstuff->classname, "item_health_large") == 0) && (ent->health < 100))) || 
									
									//or even a tech powerup
									((newstuff->item->flags & IT_TECH) && !CTFWhat_Tech(ent))
									)
									
									ent->bot_stuff = newstuff;
								
							}
							//ctf
							
						}
						}
						
						// end if
				} 
				// end if
				
				//count++;
				// end if
				if ((ent->enemy != NULL) && ((ent->bot_stuff != NULL) || (ent->bot_fow != NULL)))
					break;
				
			}
			// end while
			
			//gi.dprintf("%s - %i\n",ent->client->pers.netname,count);
	}
	
	qboolean bot_test_ground (edict_t *ent)
	{
		//check to see if on a lift 	
		if (ent->groundentity != NULL) 
			if ((Q_stricmp(ent->groundentity->classname, "func_plat")==0) && (ent->groundentity->moveinfo.state > 0))
				return true;
			
			return false;
			
	}
	
	qboolean bot_check_frame (edict_t *ent)
	{
		//check to see if bot is saluting, fliping or taunt someone.
		if ( 
			(!(ent->s.frame > FRAME_salute01 && ent->s.frame < FRAME_salute11))
			&& (!(ent->s.frame > FRAME_flip01 && ent->s.frame < FRAME_flip12)) 
			&& (!(ent->s.frame > FRAME_taunt01 && ent->s.frame < FRAME_taunt17))
			)
			return false;
		
		return true;
		
	}
	
	
	// move the bot to its new location
	void bot_move (edict_t *ent, edict_t *goal, vec3_t angles)
	{
		float distance, travel;
		vec3_t temp;
		
		//		return;
		
		if (ent->deadflag == DEAD_DEAD)
			return;
		
		if (goal == NULL)
			return;
	
		VectorSubtract ( ent->s.origin, goal->s.origin, temp);
		distance = VectorLength(temp);			
		
		//	if ((distance < 500)) //&& (distance > 60))
		travel = distance / 20; //45
		//	else
		//		travel = 2;
		//	travel = 50;

/*		if (ent->bot_stuff != NULL)
		{
				char *temp1,temp2[40] = "test.txt";
	FILE *in;
							in	= fopen( temp2, "a");
	fprintf( in, "classname:%s   org:%f %f %f   stuff:%f %f %f  dist:%f  result:%f\n", 
		ent->bot_stuff->classname, ent->s.origin[0], ent->s.origin[1],ent->s.origin[2],
	ent->bot_stuff->s.origin[0], ent->bot_stuff->s.origin[1],ent->bot_stuff->s.origin[2],
		distance,travel);
	fclose(in);
		}
*/
		VectorClear(ent->velocity);

		//give the appear that bot is running
		if ((ent->s.frame == FRAME_stand01) || (ent->s.frame == FRAME_run6))
		{
			ent->s.frame = FRAME_run1;
			ent->client->anim_end = FRAME_run6;
		}
		
		if (!botTouchingLadder(ent))
			ent->ladder=0;
		
		//normal travel
	//	if (ent->ladder==0)
	//	{
		temp[0] = (((goal->s.origin[0] - ent->s.origin[0])/ travel) + ent->s.origin[0]);
		temp[1] = (((goal->s.origin[1] - ent->s.origin[1])/ travel) + ent->s.origin[1]);
		temp[2] = (((goal->s.origin[2] - ent->s.origin[2])/ travel) + ent->s.origin[2]);
		temp[2]++;
/*		}else if (ent->ladder == 1)
		{
			temp[0]=ent->s.origin[0];
			temp[1]=ent->s.origin[1];
			temp[2]=ent->s.origin[2]-40;
		}
		else if (ent->ladder == 2)
		{
			temp[0]=ent->s.origin[0];
			temp[1]=ent->s.origin[1];

			temp[2]=ent->s.origin[2]+40;
		}		*/
		if (!((gi.pointcontents(temp) & CONTENTS_SOLID)))
			VectorCopy(temp, ent->s.origin);
	//	else if (ent->ladder >0)
	//		ent->ladder=0;
	//	ent->enemy = goal;
	//SV_NewChaseDir (ent, goal, distance); 
	//ent->goalentity = goal;
	//M_MoveToGoal(ent,40);

		if (ent->enemy == NULL)
			Bot_Aim(ent, goal, angles);
		
	}
	
	
	qboolean botTouchingLadder(edict_t *self)
	{
		vec3_t org;
		
		VectorCopy(self->s.origin, org);
		
		org[2] += 48;
		
		if (gi.pointcontents(org) & CONTENTS_LADDER)
			return true;
		
		org[0] += 8; org[1] += 8;
		if (gi.pointcontents(org) & CONTENTS_LADDER)
			return true;
		
		org[0] += -16;
		if (gi.pointcontents(org) & CONTENTS_LADDER)
			return true;
		
		org[1] += -16;
		if (gi.pointcontents(org) & CONTENTS_LADDER)
			return true;
		
		org[0] += 16;
		if (gi.pointcontents(org) & CONTENTS_LADDER)
			return true;
		
		org[0] -= 8; org[1] += 8;
		org[2] -= 48;
		if (gi.pointcontents(org) & CONTENTS_LADDER)
			return true;
		
		
		return false;
	};
	
	//see if you can see your flag
	edict_t *your_flag_vis(edict_t *ent)
	{
		edict_t *flag;
		char *c;
		
		switch (ent->client->resp.ctf_team)
		{
		case CTF_TEAM1:
			c = "item_flag_team1";
			break;
		case CTF_TEAM2:
			c = "item_flag_team2";
			break;
		default:
			return NULL;
		}
		
		flag = NULL;
		while ((flag = G_Find (flag, FOFS(classname), c)) != NULL) 
		{
			//its not an dropped flag
			if (!(flag->spawnflags & DROPPED_ITEM))
				if (visible(ent, flag))
					return flag;
		}
		
		return NULL;
	}
	
	//see if you are the one with the flag and can see your own, so go
	//by flag and wait for the return of your flag
	void check_for_stay_at_base(edict_t *ent)
	{
		gitem_t *item;
		int 	index;
		
		if (ent->bot_stuff != NULL)
		{
			ent->waiting_return_flag = 0;
			return;
		}
		
		if (ctf->value)
		{
			char *c;
			
			//check to see you are carring the flag
			switch (ent->client->resp.ctf_team)
			{
			case CTF_TEAM1:
				c = "Blue Flag";
				break;
			case CTF_TEAM2:
				c = "Red Flag";
				break;
			default:
				{
					return;
					ent->waiting_return_flag = 0;
				}
			}
			item = NULL;
			item = FindItem(c);
			
			if (item)
			{
				edict_t *temp;
				
				temp=NULL;
				index = ITEM_INDEX(item);
				if (ent->client->pers.inventory[index])
					if ((temp = your_flag_vis(ent)) != NULL)
					{
						//found the flag, now go wait
						ent->bot_stuff = temp;
						ent->waiting_return_flag =1;
						return;
					}
			}
		}
		ent->waiting_return_flag = 0;
	}
	
	//move to flag or wait at base
	void ctf_wait_base(edict_t *ent, vec3_t angles)
	{
		int distance;
		vec3_t temp;
		
		VectorSubtract ( ent->s.origin, ent->bot_stuff->s.origin, temp);
		distance = VectorLength(temp);
		
		if ((distance > 120) ||
			((gi.pointcontents(ent->s.origin) & CONTENTS_SOLID) != 0) ||
			(ent->bot_stuff->solid == 1))
			
			bot_move(ent, ent->bot_stuff, angles);
		else
		{
			angles[1]+=2;
			if (angles[1]>360) angles[1]=0;
		}
	}