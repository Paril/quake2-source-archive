#include "g_local.h"

#define Posses 0
#define StopBullets 1
#define IR 2
#define Cloak 3
#define Flight 4
#define Speed 5
#define EMP 6
#define Health 7
#define Damage 8
#define Mana 9

qboolean SpellFullahhh(edict_t *ent);
void Cmd_AutoBuy_f (edict_t *ent)
{
	edict_t *cl_ent;
	int Choose[9];
	int FinalChoice = 10;
	int i;
	qboolean ownerizing = false;

	if(ent->deadflag)
		return;

	for (i = 0; i<=9;i++)
		Choose[i]=0;
	
	for (i=0 ; i<game.maxclients ; i++) //checks if player is not top of score sheet.
	{
	cl_ent = g_edicts + 1 + i;	
	if(cl_ent->client->resp.score > ent->client->resp.score)
		ownerizing = true;

	if(cl_ent->healthlevel || cl_ent->damagelevel)
		{
			Choose[Health] += random()*10;
			Choose[Damage] += random()*20;
		}
	
	}
//costs
	if (ent->stamina < 250)
		Choose[Posses] -= 100;

	if (ent->stamina  < 200)
		Choose[EMP] -= 100;

	if (ent->stamina  < 150)
	{
		Choose[Damage] -=100;
		Choose[Health] -=100;
		Choose[Mana] -=100;
	}
	if (ent->stamina  < 100)
	{
		Choose[Speed] -=100;
		Choose[Cloak] -=100;
	}

	if (ent->stamina  < 75)
	{
		Choose[StopBullets] -=100;
		
		Choose[IR] -=100;
		
		gi.cprintf (ent, PRINT_HIGH, "You dont have enough energy stored for ANYTHING!\n");
		return;
	}
//already got?
	if(ent->damagelevel + ent->healthlevel + ent->staminalevel >= sv_maxlevel->value)
	{
		Choose[Damage] -=100;
		Choose[Health] -=100;
		Choose[Mana] -=100;
	}
	
	if(ent->client->speed_framenum > level.framenum - 100)
	{
		Choose[Speed] -= 100;
		Choose[Cloak]+=random()*4;
	}
	if(ent->client->ir_framenum > level.framenum -100)
	{
		Choose[IR]-=100;
		Choose[Cloak]+=random()*10;
		Choose[EMP]+=random()*14;
	}
	if (ent->client->cloak_framenum > level.framenum - 100)
	{
		Choose[Cloak]-=100;
		Choose[EMP]+=random()*14;
		Choose[Damage] += random()*10;
	}
	if (ent->bullet_framenum > level.framenum - 100)
	{
		Choose[StopBullets] -= 100;
		Choose[Cloak]+=random()*10;
		Choose[Posses]+=random()*10;
		Choose[Damage] += random()*10;
	}
	if(possesban->value)
		Choose[Posses]-= 100;
	if(SpellFullahhh(ent))
	{
		Choose[StopBullets] -= 100;
		Choose[Cloak]-= 100;
		Choose[Posses]-= 100;
		Choose[IR] -= 100;
		Choose[Speed] -= 100;
	}

	if(matrix.lightsout)
		Choose[EMP] -=100;

//situations.
	//if you are using anything but knives and fists, you can't use speed or bullet stopping (too powerful)
	
	if (ent->light_level <= 40 & ent->health > 90) //if in the dark and alone
	{
		Choose[Posses]+=random()*8;
		Choose[Cloak]+=random()*8;
		Choose[IR]+=random()*8;
	}
	if (ent->light_level <=40  && ent->health > 90 && ent->staminalevel>1) //if in the dark and alone and someone is owning
	{
		Choose[Posses]+=random()*8;
	}
	if (ent->light_level <= 30 && !matrix.lightsout) //if in the dark and with others
	{
		Choose[Cloak]+=random()*8;
		Choose[IR]+=3 + random()*10;
	}

	if(ent->light_level>30 && !matrix.lightsout) //if other players are in the area and it's light
	{
		Choose[Speed]+=random()*8;
		Choose[StopBullets]+=random()*8;
		Choose[Cloak]+=random()*4;
		
		Choose[IR]-=random()*10;
	}
	if(matrix.lightsout)
	{	
		Choose[IR]+=random()*20;
		Choose[Cloak]+=random()*12;
	}
	if (ent->health<70)
	{
		Choose[StopBullets]+=random()*12;
		Choose[Speed]+=random()*6;
		
	}
	
	if (strcmp(ent->client->pers.weapon->classname, "weapon_sniper") == 0)
	{
		Choose[Cloak]+=random()*12;
		Choose[IR]+=random()*8;
		Choose[EMP]+=random()*8;
		Choose[Speed]-=random()*6;
	}
	if(ent->health < ent->max_health/1.5)
		Choose[Health] += random()*20;
	if(ent->health < 30)
		Choose[Health] += 5 + random()*20;//fuckn do it!

	if(ownerizing)
	{
		Choose[Mana] += random()*20;
		Choose[Damage] += random()*10;
	}
	if(strcmp(ent->client->pers.weapon->classname, "weapon_knives") == 0 || 
		strcmp(ent->client->pers.weapon->classname, "weapon_fists") == 0)
	{
		Choose[Speed]+= random()*20;
		Choose[StopBullets]+= random()*25;
	}
	else
	{
		Choose[Speed]-= 100;
		Choose[StopBullets]-= 100;
	}

	//see which is highest
	FinalChoice = 0;
	for (i=0;i<10;i++)
	{
		if(Choose[i]==FinalChoice)
			Choose[i]+=crandom()*2;
		if(Choose[i]>FinalChoice)
			FinalChoice = Choose[i];
	}
	if(FinalChoice <=0)
	{
		gi.cprintf (ent, PRINT_HIGH, "No spell available. Please try again later.\n");
		return;
	}
	//gi.cprintf (ent, PRINT_HIGH, "Posses: %i\nStop Bullets: %i\nIR: %i\nCloak: %i\nFlight: %i\nSpeed: %i\nEMP: %i\n", Choose[Posses],Choose[StopBullets], Choose[IR], Choose[Cloak], Choose[Flight], Choose[Speed], Choose[EMP]);
	//gi.cprintf (ent, PRINT_HIGH, "LightLevel: %i\n", ent->light_level);
	
	//buy related spell.
	
	if(FinalChoice == Choose[Posses])
		MatrixStartSwap(ent);
	if(FinalChoice == Choose[StopBullets])
		Cmd_StopBullets_f(ent);
	if(FinalChoice == Choose[IR])
		Cmd_Infrared_f (ent);		
	if(FinalChoice == Choose[Cloak])
		Cmd_Cloak_f (ent);
	if(FinalChoice == Choose[Speed])
		Cmd_BuySpeed_f (ent);
	if(FinalChoice == Choose[EMP])
		Cmd_Lights_f (ent);
}

qboolean SpellFull(edict_t *ent)
{
	int		tally = 0;

	
	if(ent->client->speed_framenum > level.framenum - 100)
	tally++;
	if(ent->client->ir_framenum > level.framenum -100)
	tally++;
	if (ent->client->cloak_framenum > level.framenum - 100)
	tally++;
	if (ent->bullet_framenum > level.framenum - 100)
	tally++;
	if (matrix.lightsout_framenum > level.framenum - 100 && ent == matrix.triggerent)
	tally++;

	if(tally>1)
		return true;
	else 
		return false;
}

qboolean SpellFullahhh(edict_t *ent)
{
	int		tally = 0;

	
	if(ent->client->speed_framenum > level.framenum - 100)
	tally++;
	if(ent->client->ir_framenum > level.framenum -100)
	tally++;
	if (ent->client->cloak_framenum > level.framenum - 100)
	tally++;
	if (ent->bullet_framenum > level.framenum - 100)
	tally++;
	if (matrix.lightsout_framenum > level.framenum - 100)
	tally++;

	if(tally>2)
		return true;
	else 
		return false;
}