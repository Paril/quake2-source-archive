#include "g_local.h"
#include "s_readconfig.h"
#include <time.h> 


// SCONFIG START
void MakeBoldString (char *text);
void SetCTFStats(edict_t *ent);
void HelpHud(edict_t *ent);
//void endLevelshowTop10(edict_t *ent);
// SCONFIG STOP


/*
======================================================================

INTERMISSION

======================================================================
*/

void MoveClientToIntermission (edict_t *ent)
{
	if (deathmatch->value || coop->value)
		ent->client->showscores = true;
	VectorCopy (level.intermission_origin, ent->s.origin);
	ent->client->ps.pmove.origin[0] = level.intermission_origin[0]*8;
	ent->client->ps.pmove.origin[1] = level.intermission_origin[1]*8;
	ent->client->ps.pmove.origin[2] = level.intermission_origin[2]*8;
	VectorCopy (level.intermission_angle, ent->client->ps.viewangles);
	ent->client->ps.pmove.pm_type = PM_FREEZE;
	ent->client->ps.gunindex = 0;
	ent->client->ps.blend[3] = 0;
	ent->client->ps.rdflags &= ~RDF_UNDERWATER;

	// clean up powerup info
	ent->client->quad_framenum = 0;
	ent->client->invincible_framenum = 0;
	ent->client->breather_framenum = 0;
	ent->client->enviro_framenum = 0;
	ent->client->grenade_blew_up = false;
	ent->client->grenade_time = 0;

	ent->viewheight = 0;
	ent->s.modelindex = 0;
	ent->s.modelindex2 = 0;
	ent->s.modelindex3 = 0;
	ent->s.modelindex = 0;
	ent->s.effects = 0;
	ent->s.sound = 0;
	ent->solid = SOLID_NOT;

	// add the layout

	if (deathmatch->value || coop->value)
	{
		DeathmatchScoreboardMessage (ent, NULL);
		gi.unicast (ent, true);
	}

	// DISPLAY HIGH SCORES...
//	if (deathmatch->value || coop->value)
//	{
//	endLevelshowTop10(ent);
//	gi.unicast(ent,true);
//	}



}

void BeginIntermission (edict_t *targ)
{
	int		i, n;
	edict_t	*ent, *client;

	if (level.intermissiontime)
		return;		// already activated

	
	game.autosaved = false;

	// respawn any dead clients
	for (i=0 ; i<maxclients->value ; i++)
	{
		client = g_edicts + 1 + i;
		if (!client->inuse)
			continue;
		if (client->health <= 0)
			respawn(client);
	}

	level.intermissiontime = level.time;
	level.changemap = targ->map;

	if (strstr(level.changemap, "*"))
	{
		if (coop->value)
		{
			for (i=0 ; i<maxclients->value ; i++)
			{
				client = g_edicts + 1 + i;
				if (!client->inuse)
					continue;
				// strip players of all keys between units
				for (n = 0; n < MAX_ITEMS; n++)
				{
					if (itemlist[n].flags & IT_KEY)
						client->client->pers.inventory[n] = 0;
				}
			}
		}
	}
	else
	{
		if (!deathmatch->value)
		{
			level.exitintermission = 1;		// go immediately to the next level
			return;
		}
	}

	level.exitintermission = 0;

	// find an intermission spot
	ent = G_Find (NULL, FOFS(classname), "info_player_intermission");
	if (!ent)
	{	// the map creator forgot to put in an intermission point...
		ent = G_Find (NULL, FOFS(classname), "info_player_start");
		if (!ent)
			ent = G_Find (NULL, FOFS(classname), "info_player_deathmatch");
	}
	else
	{	// chose one of four spots
		i = rand() & 3;
		while (i--)
		{
			ent = G_Find (ent, FOFS(classname), "info_player_intermission");
			if (!ent)	// wrap around the list
				ent = G_Find (ent, FOFS(classname), "info_player_intermission");
		}
	}

	VectorCopy (ent->s.origin, level.intermission_origin);
	VectorCopy (ent->s.angles, level.intermission_angle);

	// move all clients to the intermission point
	for (i=0 ; i<maxclients->value ; i++)
	{
		client = g_edicts + 1 + i;
		if (!client->inuse)
			continue;
		MoveClientToIntermission (client);
	}
}



void HighScoreMessage ()
{

// ***************************************************************
// HIGH SCORE VARS
// ***************************************************************
int playercount=0;//=2; //9; // In example below playercount is 9, [0]..[8]
int ii=0;
int outerloop;
int playerindex=0;
int z=0;

	// Variable declaration
	FILE *highscorefile;
	FILE *newhighscorefile; // for saving

	char dynamicName[80];
	char line[80];
	int ranknum=0;
	int exist=0;

	int linenum=0;
	char *eof;
	int x,i;

	int myLength;
	char PLAYER[80];
	int player=0,pl=0, p=0;
	
	char SCORE[15];
	int score=0, sc=0, s=0;

	char DATE[15];
	int date=0, da=0,d=0;
	char STORAGE[15];

	int loop=0;
	char temp[80];
	char space=0;
	int num=-1;
	
	// DATE HANDLING
	time_t t;
	char todayDay[3];
	char todayMonth[4];
	char todayYear[3];
	char formatDate[9];
	char todayDate[30];
	char saveDate[9]; // What will be saved back to file
	// DATE HANDLING

// ***************************************************************
// END HIGH SCORE VARS
// ***************************************************************
	
	gclient_t	*cl;
	edict_t		*cl_ent;
	int k,j=0;
	int		Cscore, total=0;
	int		sorted[MAX_CLIENTS];
	int		sortedscores[MAX_CLIENTS];

	for (i=0; i<80;i++)
		dynamicName[i]='\0';
	

	for (i=0; i<15; i++) {
		for (j=0; j<15; j++) {
			highscore[i].name[j]='\0';
			gamescore[i].name[j]='\0';
			highscore[i].filedate[j]='\0';
			highscore[i].date[j]='\0';
			gamescore[i].date[j]='\0';
			highscore[i].huddate[j]='\0';
		}
		highscore[i].score=0;
		highscore[i].updated=0;
		gamescore[i].score=0;
	}

	i=0;
	j=0;



	// sort the clients by score
	total = 0;
	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;
		Cscore = game.clients[i].resp.score;
		for (j=0 ; j<total ; j++)
		{
			if (Cscore > sortedscores[j])
				break;
		}
		for (k=total ; k>j ; k--)
		{
			sorted[k] = sorted[k-1];
			sortedscores[k] = sortedscores[k-1];
		}
		sorted[j] = i;
		sortedscores[j] = Cscore;
		total++;
	}

	
	// Limit to 15
	if (total > 15) {
		total = 15;
	}

	for (i=0 ; i<total ; i++)
	{
		cl = &game.clients[sorted[i]];
		cl_ent = g_edicts + 1 + sorted[i];
		
		strcpy(gamescore[i].name,cl->pers.netname);  // Copy off player name
		gamescore[i].score = cl->resp.score;		 // Copy off player score
		
		// Date will be appended later after sort	
		
		playercount++;
	}
	
// ***************************************************************
// HIGH SCORE
// ***************************************************************

// Open existing file, if it doesn't exist let's create it.
	strcpy(dynamicName,HIGHSCORE_DIR);
	strcat(dynamicName,level.mapname);
	strcat(dynamicName,".txt");
	highscorefile = fopen(dynamicName/*"c:/newscores.txt"*/,"r"); 
	//strcpy(dynamicName,"D:/");
	//strcat(dynamicName,level.mapname);
	//strcat(dynamicName,".txt\0");
    //highscorefile = fopen(dynamicName,"r"); 

		if (highscorefile == NULL) {
			gi.dprintf("Generating High Score file for %s\n", level.mapname);
			exist=0;
		}
		else
			exist=1; // Set flag to determine how to handle reading/saving high scores

		if (exist==1) {

		while ((eof = fgets(line, 80, highscorefile)) != NULL){

			for (i=0; i<80; i++)
				temp[i]='\0';
			linenum++;

			x=0;
			myLength=strlen(line);

			for (i=0; i<myLength; i++)
			{
				for (loop=0; loop<15; loop++) {
					STORAGE[loop]='\0';
				}
				
				temp[i]=line[i];

				if (strcmp(temp,"SCORE ")==0) {
					ranknum++;
					temp[0]='\0';
					num++;
				}

				if (strcmp(temp,"PLAYER=")==0) {
					for (p=i+1; p<myLength; p++) {
						PLAYER[player]=line[p];
						player++;
					}
					PLAYER[player-1]='\0'; // Cut off if b4 than 15 characters
					PLAYER[15]='\0'; // Cut off solely at 15 characters
					strcpy(highscore[num].name,PLAYER);

					//playerindex=strlen(PLAYER);
					for (pl=0; pl<80;pl++)
						PLAYER[pl]='\0';
					player=0;
					space=0;
					playerindex=0;
				}
			
				if (strcmp(temp,"SCORE=")==0) {
					for (s=i+1; s<myLength; s++) {
						SCORE[score]=line[s];
						score++;
					}
					SCORE[score-1]='\0';
					highscore[num].score = atoi(SCORE);
					for (sc=0; sc<15;sc++)
						SCORE[sc]='\0';
					score=0;
				}
			
				if (strcmp(temp,"DATE=")==0) {
					for (d=i+1; d<myLength; d++) {
						DATE[date]=line[d];
						date++;
					}
					DATE[date-1]='\0';
					// Format date to US style... mm/dd/yyyy rather than saved yy/mm/dd
					// Note be sure to flip back before saving back to file...
					STORAGE[0]=DATE[3];
					STORAGE[1]=DATE[4];
					STORAGE[2]='/';
					STORAGE[3]=DATE[6];
					STORAGE[4]=DATE[7];
					STORAGE[5]='/';
					STORAGE[6]=DATE[0];
					STORAGE[7]=DATE[1];
					STORAGE[8]='\0';
									
					strcpy(highscore[num].date,STORAGE);  // ORIGINAL
					strcpy(highscore[num].filedate, DATE); // SAVE THIS
					for (da=0; da<15;da++) {
						DATE[da]='\0';
						STORAGE[da]='\0';
					}
					date=0;	
				highscore[num].updated = 0;	
				}	
				//we have line
			} // end for
		} // end while

		if  (eof == NULL)
			fclose(highscorefile);

		} // if if exist == 1	
	
		/*if  (eof == NULL)
			fclose(highscorefile);*/

		/*
		// ON THE FLY DISPLAY OF HIGH SCORES, DIRECT FROM CURRENT FILE
			printf("Name, Frags, Date, New(0/1)\n");
			printf("-----------------\n");

			for (ii=0; ii<15; ii++)
				printf("%s --  %i  --  %s -- %i\n",
				highscore[ii].name,highscore[ii].score,highscore[ii].date,
				highscore[ii].updated);
		*/

	time ( &t );
	strcpy(todayDate, ctime ( &t ) );

	// Check month name, convert to numeric	
	todayMonth[0] = todayDate[4];
	todayMonth[1] = todayDate[5];
	todayMonth[2] = todayDate[6];
	todayMonth[3] = '\0';

	if (strcmp(todayMonth,"Jan") == 0 )
		strcpy(formatDate,"01/");
	else if (strcmp(todayMonth,"Feb") == 0 )
		strcpy(formatDate,"02/");
	else if (strcmp(todayMonth,"Mar") == 0 )
		strcpy(formatDate,"03/");
	else if (strcmp(todayMonth,"Apr") == 0 )
		strcpy(formatDate,"04/");
	else if (strcmp(todayMonth,"May") == 0 )
		strcpy(formatDate,"05/");
	else if (strcmp(todayMonth,"Jun") == 0 )
		strcpy(formatDate,"06/");
	else if (strcmp(todayMonth,"Jul") == 0 )
		strcpy(formatDate,"07/");
	else if (strcmp(todayMonth,"Aug") == 0 )
		strcpy(formatDate,"08/");
	else if (strcmp(todayMonth,"Sep") == 0 )
		strcpy(formatDate,"09/");
	else if (strcmp(todayMonth,"Oct") == 0 )
		strcpy(formatDate,"10/");
	else if (strcmp(todayMonth,"Nov") == 0 )
		strcpy(formatDate,"11/");
	else if (strcmp(todayMonth,"Dec") == 0 )
		strcpy(formatDate,"12/");
	

	todayDay[0]= todayDate[8];
	todayDay[1]= todayDate[9];
	todayDay[2]= '\0';

	strcat(formatDate,todayDay);
	strcat(formatDate,"/");

	todayYear[0]=todayDate[22];
	todayYear[1]=todayDate[23];
	todayYear[2]='\0';
	
	
	strcat(formatDate,todayYear);
	formatDate[8]='\0';

	// yy/mm/dd (FOR FILE OUTPUT)...
	saveDate[0]=todayYear[0];
	saveDate[1]=todayYear[1];
	saveDate[2]='/';
	saveDate[3]=formatDate[0];
	saveDate[4]=formatDate[1];
	saveDate[5]='/';
	saveDate[6]=todayDay[0];
	saveDate[7]=todayDay[1];
	saveDate[8]='\0';

if (exist==0) {
	for (z=0; z<15; z++)
	{
		highscore[z].score=1;
		strcpy(highscore[z].name,"<EMPTY>");
		strcpy(highscore[z].huddate,"01/01/98");
		strcpy(highscore[z].filedate,"98/01/01"); // Version 3.0 FIX
	}
}

for (outerloop=0; outerloop < playercount; outerloop++) {
	
	for (z=0; z<15; z++)
	{
		if (gamescore[playerindex].score > highscore[z].score) {
			// True value detected, gamescore greater than existing high score
			// Must shift all values down by one and insert in this position
			for (ii=14; ii>z; ii--) { 
				strcpy(highscore[ii].name,highscore[ii-1].name);
				highscore[ii].score = highscore[ii-1].score;
				strcpy(highscore[ii].date,highscore[ii-1].date);
				strcpy(highscore[ii].filedate, highscore[ii-1].filedate); // ******* ADD *********
				strcpy(highscore[ii].huddate,highscore[ii-1].huddate);
				highscore[ii].updated=0; // make sure this is set to zero for safety
			} // end for (i=14; i<outerloop; i--)
	
			// Next copy new updated gamescore into high score position
			strcpy(highscore[z].name,gamescore[playerindex].name);
			highscore[z].score=gamescore[playerindex].score;
			strcpy(highscore[z].huddate,formatDate); // sep. routine will fill  (today's date on hud)
			
			strcpy(highscore[z].filedate,saveDate); // sep. routine will fill (today's date in file)
			//strcpy(highscore[z].date,saveDate); // sep. routine will fill (today's date in file)
			highscore[z].updated=1; // make sure this is set to zero for safety
			playerindex++; // Increment player check against existing high scores
			break; 	// force to exit inner loop
		} // End if (gamescore[playerindex].score > highscore[outerloop].score)
	} // end z
} // End for (outerloop=0; outerloop < 15; outerloop++)

// DISPLAY INFO TO HUD, use com_sprintf later
/*			printf("\nPLAYER INDEX: %i\n", playerindex);
			printf("Name, Frags, Date, New(0/1)\n");
			printf("-----------------\n");

			for (ii=0; ii<15; ii++) {
			if (strlen(highscore[ii].huddate)==8)
				printf("%s --  %i  --  %s -- %i\n",
				highscore[ii].name,highscore[ii].score,highscore[ii].huddate,
				highscore[ii].updated);
			else
				printf("%s --  %i  --  %s -- %i\n",
				highscore[ii].name,highscore[ii].score,highscore[ii].date,
				highscore[ii].updated);
			}
*/
			


	// SAVE OUT TO FILE.. CHANGES WILL HAVE TOOK PLACE		
	newhighscorefile = fopen(dynamicName,"w+");  //substitute level.mapname here later
	
	if (newhighscorefile==NULL)
		gi.dprintf("**** ERROR WRITING TO HIGH SCORE DIRECTORY ***\n1) Check your 'HighScoreDir' value in your CONFIG.TXT file\n2) Check to make sure you have the directory specified!\n**** NO SCORES SAVED OUT TO FILE ****\n");
	else {

		for (ii=0; ii<15; ii++) {
		fprintf (newhighscorefile,"SCORE %i ----------------------\n",ii+1);

		if (strlen(highscore[ii].name) != 0) {
		fprintf (newhighscorefile,"PLAYER=%s\n",highscore[ii].name);
		fprintf (newhighscorefile,"SCORE=%i\n",highscore[ii].score);
		fprintf (newhighscorefile,"TEAM=1\n");
		fprintf (newhighscorefile,"RED=0\n");
		fprintf (newhighscorefile,"BLUE=0\n");
		fprintf (newhighscorefile,"DATE=%s\n", highscore[ii].filedate);
		}
		else // failsafe, code never gets executed...
		{
		fprintf (newhighscorefile,"PLAYER=<EMPTY>\n");
		fprintf (newhighscorefile,"SCORE=1\n");
		fprintf (newhighscorefile,"TEAM=1\n");
		fprintf (newhighscorefile,"RED=0\n");
		fprintf (newhighscorefile,"BLUE=0\n");
		fprintf (newhighscorefile,"DATE=98/01/01\n");

		} // end if
		} // end for
	
	fclose(newhighscorefile);
	
	} // end else

	exist=0;
	

// ***************************************************************
// END HIGH SCORE
// ***************************************************************
}


void endLevelshowTop10(edict_t *ent)
{
	gclient_t	*cl;
	edict_t		*cl_ent;

	int music=0;
	int maplengthname=0;
//	int myLength=0;
	int pos=0;

	int		sorted[MAX_CLIENTS];
	int		sortedscores[MAX_CLIENTS];
	char	entry[1024];
	char	string[1400];//1400
	int		stringlength=0;
	int x,y,j,i=0;
	int ii=0;
	int total=0;
	int k=0;
	int score=0;
	

	// NEW 3.1
	// Make sure the scoreboard has no bad data in it
	for (i=0; i<1400; i++)
		string[i]='\0';
	// NEW 3.1

	// sort the clients by score
	total = 0;
	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;
		score = game.clients[i].resp.score;
		for (j=0 ; j<total ; j++)
		{
			if (score > sortedscores[j])
				break;
		}
		for (k=total ; k>j ; k--)
		{
			sorted[k] = sorted[k-1];
			sortedscores[k] = sortedscores[k-1];
		}
		sorted[j] = i;
		sortedscores[j] = score;
		total++;
	}

		for (i=0 ; i<total ; i++)
		{
		cl = &game.clients[sorted[i]];
		cl_ent = g_edicts + 1 + sorted[i];
		}

//	string[0] = 0;   (Removed for NEW 3.1)

	// NEW 3.1
	stringlength = 0; //strlen(string);	
	// NEW 3.1

	maplengthname=strlen(level.level_name);
//	myLength=maplengthname;
	maplengthname=(maplengthname*8)+104;

	pos=(276-maplengthname)/2;

	Com_sprintf(entry, sizeof(entry), 
		"xv %i yv 1 string2 \"High Scores for %s\" "
		"xv 10 yv 16 string \"Rank\" "
		"xv 50 string \"Player\" "
		"xv 194 string \"Frags Date\" "
		"xv 10 yv 24 string \"---- ---------------   ----- --------\" ",pos,level.level_name);
		
		j = strlen(entry);//+myLength; // NEW 3.1 (+maplengthname)
		strcpy (string + stringlength, entry);
		stringlength += j;

		
	for (ii=0; ii<10; ii++) {
		
		x=32;
		y=32+8*ii;
		
		if (highscore[ii].updated==0) { 
	
		// TAKEN OUT NEW 3.1
//					if (strlen(highscore[ii].huddate)==8) {	
		// TAKEN OUT NEW 3.1
	
//				Com_sprintf(entry, sizeof(entry), 
//					"xv 18 yv %i string \"%2i  %s\" "
//					"xv 200 string \"%3i  %s\"",
//					y, ii+1, 
//					highscore[ii].name, 
//					highscore[ii].score, 
//					highscore[ii].huddate); 
//
//				j = strlen(entry); 
//		
//				if (stringlength + j > 1024) 
//					break; 
//				strcpy (string + stringlength, entry); 
//				stringlength += j; 
// TAKEN OUT NEW 3.1
//			}
//			else 
//			{

				Com_sprintf(entry, sizeof(entry), 
					"xv 18 yv %i string \"%2i  %s\" "
					"xv 200 string \"%3i  %s\"",
					y, ii+1, 
					highscore[ii].name, 
					highscore[ii].score, 
					highscore[ii].date); 
		
				j = strlen(entry); 
				if (stringlength + j > 1024) 
					break; 
				strcpy (string + stringlength, entry); 
				stringlength += j; 
//			} // end else
// TAKEN OUT NEW 3.1
		} // end if update == 0
		else 
		{
			if (strlen(highscore[ii].huddate)==8) {	
				music=1; // Play computer update later flag;

				Com_sprintf(entry, sizeof(entry), 
					"xv 18 yv %i string2 \"%2i  %s\" "
					"xv 200 string2 \"%3i  %s\"",
					y, ii+1, 
					highscore[ii].name, 
					highscore[ii].score, 
					highscore[ii].huddate); 

				j = strlen(entry); 
				if (stringlength + j > 1024) 
					break; 
				strcpy (string + stringlength, entry); 
				stringlength += j; 
			}
			else 
			{
				Com_sprintf(entry, sizeof(entry), 
					"xv 18 yv %i string2 \"%2i  %s\" "
					"xv	200 string2 \"%3i  %s\"",
					y, ii+1, 
					highscore[ii].name, 
					highscore[ii].score, 
					highscore[ii].date); 
		
				j = strlen(entry); 
				if (stringlength + j > 1024) 
					break; 
				strcpy (string + stringlength, entry); 
				stringlength += j; 
			} // end else
		
		} // end else

	} // end for (each iteration...)

		if (music==1) {
			gi.sound (ent, CHAN_VOICE, gi.soundindex ("misc/pc_up.wav"), 1, ATTN_STATIC, 0);
			music=0; // clean up to be safe
		}

		gi.WriteByte (svc_layout);
		gi.WriteString (string);

}

/*
void inGameshowTop10(edict_t *ent)
{

	gclient_t	*cl;
	edict_t		*cl_ent;

	int		sorted[MAX_CLIENTS];
	int		sortedscores[MAX_CLIENTS];
	char	entry[1024];
	char	string[1400];//1400
	int		stringlength=0;
	int x,y,j,i;
	int ii;
	int total;
	int k;
	int score;
	

	// sort the clients by score
	total = 0;
	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;
		score = game.clients[i].resp.score;
		for (j=0 ; j<total ; j++)
		{
			if (score > sortedscores[j])
				break;
		}
		for (k=total ; k>j ; k--)
		{
			sorted[k] = sorted[k-1];
			sortedscores[k] = sortedscores[k-1];
		}
		sorted[j] = i;
		sortedscores[j] = score;
		total++;
	}

		for (i=0 ; i<total ; i++)
		{
		cl = &game.clients[sorted[i]];
		cl_ent = g_edicts + 1 + sorted[i];
		}

	string[0] = 0;

	stringlength = strlen(string);	

	Com_sprintf(entry, sizeof(entry), 
		"xv 10 yv 16 string \"Rank\" "
		"xv 50 string \"Player\" "
		"xv 194 string \"Frags Date\" "
		"xv 10 yv 24 string \"---- ---------------   ----- --------\" ");
		
		j = strlen(entry);
		strcpy (string + stringlength, entry);
		stringlength += j;

		
	for (ii=0; ii<10; ii++) {
		x=32;
		y=32+8*ii;

	if (strlen(highscore[ii].huddate)==8) {	
		Com_sprintf(entry, sizeof(entry), 
			"xv 18 yv %i string \"%2i  %s\" "
			"xv 200 string \"%3i  %s\"",
			y, ii+1, 
			 highscore[ii].name, 
			 highscore[ii].score, 
			 highscore[ii].huddate); 

		j = strlen(entry); 
		if (stringlength + j > 1024) 
			break; 
		strcpy (string + stringlength, entry); 
		stringlength += j; 

	}
	else {

		Com_sprintf(entry, sizeof(entry), 
			"xv 18 yv %i string \"%2i  %s\" "
			"xv 200 string \"%3i  %s\"",
			y, ii+1, 
			 highscore[ii].name, 
			 highscore[ii].score, 
			 highscore[ii].date); 
		
		j = strlen(entry); 
		if (stringlength + j > 1024) 
			break; 
		strcpy (string + stringlength, entry); 
		stringlength += j; 
	} // end else
	} // end for

	gi.WriteByte (svc_layout);
	gi.WriteString (string);

}
*/


/*
==================
DeathmatchScoreboardMessage

==================
*/
void DeathmatchScoreboardMessage (edict_t *ent, edict_t *killer)
{
	char	entry[1024];
	char	string[1400];
	int		stringlength;
	int		i, j, k;
	int		sorted[MAX_CLIENTS];
	int		sortedscores[MAX_CLIENTS];
	int		score, total;
	int		picnum;
	int		x, y;
	gclient_t	*cl;
	edict_t		*cl_ent;
	char	*tag;


	// sort the clients by score
	total = 0;
	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;
		score = game.clients[i].resp.score;
		for (j=0 ; j<total ; j++)
		{
			if (score > sortedscores[j])
				break;
		}
		for (k=total ; k>j ; k--)
		{
			sorted[k] = sorted[k-1];
			sortedscores[k] = sortedscores[k-1];
		}
		sorted[j] = i;
		sortedscores[j] = score;
		total++;
	}

	// print level name and exit rules
	string[0] = 0;

	stringlength = strlen(string);

	// add the clients in sorted order
	if (total > 12)
		total = 12;

	for (i=0 ; i<total ; i++)
	{
		cl = &game.clients[sorted[i]];
		cl_ent = g_edicts + 1 + sorted[i];

		picnum = gi.imageindex ("i_fixme");
		x = (i>=6) ? 160 : 0;
		y = 32 + 32 * (i%6);

		// add a dogtag
		if (cl_ent == ent)
			tag = "tag1";
		else if (cl_ent == killer)
			tag = "tag2";
		else
			tag = NULL;
		if (tag)
		{
			Com_sprintf (entry, sizeof(entry),
				"xv %i yv %i picn %s ",x+32, y, tag);
			j = strlen(entry);
			if (stringlength + j > 1024)
				break;
			strcpy (string + stringlength, entry);
			stringlength += j;
		}

		// send the layout
		Com_sprintf (entry, sizeof(entry),
			"client %i %i %i %i %i %i ",
			x, y, sorted[i], cl->resp.score, cl->ping, (level.framenum - cl->resp.enterframe)/600);
		j = strlen(entry);
		if (stringlength + j > 1024)
			break;
		strcpy (string + stringlength, entry);
		stringlength += j;
	}

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
	
//	gi.sound(ent, CHAN_VOICE, gi.soundindex("world/xianbeats.wav"), 1, ATTN_IDLE, 0);


}

// SCONFIG START

// Style 2
/* 
================== 
DeathmatchScoreboardMessage *Improved!* 
================== 
*/ 
void DeathmatchScoreboardMessage2 (edict_t *ent, edict_t *killer)
{
	char	entry[1024];
	char	string[1400];
	int		stringlength;
	int		i, j, k;
	int		sorted[MAX_CLIENTS];
	int		sortedscores[MAX_CLIENTS];
	int		score, total;
	int		picnum;
	int		x, y;
	gclient_t	*cl;
	edict_t		*cl_ent;
	//char	*tag;

	// sort the clients by score
	total = 0;
	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;
		score = game.clients[i].resp.score;
		for (j=0 ; j<total ; j++)
		{
			if (score > sortedscores[j])
				break;
		}
		for (k=total ; k>j ; k--)
		{
			sorted[k] = sorted[k-1];
			sortedscores[k] = sortedscores[k-1];
		}
		sorted[j] = i;
		sortedscores[j] = score;
		total++;
	}

	// print level name and exit rules
	string[0] = 0;
	stringlength = strlen(string);

	// make a header for the data 
	Com_sprintf(entry, sizeof(entry), 
		"xv 24 yv 16 string2 \"Rank\" " 
		"xv 64 yv 16 string2 \"Player\" " 
		"xv 232 yv 16 string2 \"Frags\" " 
		"xv 280 yv 16 string2 \"Ping\" " 
		"xv 320 yv 16 string2 \"Time\" " 
		"xv 24 yv 24 string2 \"-----------------------------------------\" "); 
	j = strlen(entry); 

	if (stringlength + j < 1024) 
	{ 
		strcpy (string + stringlength, entry); 
		stringlength += j; 
	} 


	// add the clients in sorted order
	if (total > 25)
		total = 25;

	for (i=0 ; i<total ; i++)
	{
		cl = &game.clients[sorted[i]];
		cl_ent = g_edicts + 1 + sorted[i];

		picnum = gi.imageindex ("i_fixme");
		//x = (i>=6) ? 160 : 0;
		//y = 32 + 32 * (i%6);

		x=32;
		y=32+8*i;

//		if (cl_ent == ent) 
//			tag = "->"; 
//		else if (cl_ent == killer) 
//			tag = "-X>"; 
//		else 
//			tag = NULL; 
//		if (tag) 
//		{ 
			Com_sprintf (entry, sizeof(entry), 
				"xv 40 yv %i string \"%2i\" ", y, /*tag,*/ i+1); 
			j = strlen(entry); 
			if (stringlength + j > 1024) 
				break; 
			strcpy (string + stringlength, entry); 
			stringlength += j; 
//		} 

	
	Com_sprintf(entry, sizeof(entry), 
		"xv 64 yv %i string2 \"%s\" " 
		/*"xv 168 yv %i string \"  %3i  %3i  %3i\" ", */
		"xv 232 yv %i string \"  %3i  %3i  %3i\" ", 
		y, cl->pers.netname, 
		y, cl->resp.score, 
		cl->ping, 
		(level.framenum - cl->resp.enterframe)/600); 

		j = strlen(entry); 
		if (stringlength + j > 1024) 
			break; 
		strcpy (string + stringlength, entry); 
		stringlength += j; 
	}

	gi.WriteByte (svc_layout);
	gi.WriteString (string);

	// Version 2.3 Scoreboard 2
//	gi.sound(ent, CHAN_VOICE, gi.soundindex("world/xianbeats.wav"), 1, ATTN_IDLE, 0);
}
 


void DeathmatchScoreboardMessage3 (edict_t *ent, edict_t *killer)
{
	char	entry[1024];
	char	string[1400];
	int		len;
	int		i, j, k, n;
	int		sorted[2][MAX_CLIENTS];
	int		sortedscores[2][MAX_CLIENTS];
	int		score, total[2], totalscore[2];
	int		last[2];
	gclient_t	*cl;
	edict_t		*cl_ent;
	int team;
	int maxsize = 1000;
	int myLength=100;

	// ADDED!
	char	*tag,myHeader[10];
	int picnum,x,y,stringlength;
	strcpy(myHeader,"Rankings\0");
	// END ADDED!

	// sort the clients by team and score
	total[0] = total[1] = 0;
	last[0] = last[1] = 0;
	totalscore[0] = totalscore[1] = 0;
	
	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;
		if (game.clients[i].resp.ctf_team == CTF_TEAM1)
			team = 0;
		else
			continue; // unknown team?

		score = game.clients[i].resp.score;
		for (j=0 ; j<total[team] ; j++)
		{
			if (score > sortedscores[team][j])
				break;
		}
		for (k=total[team] ; k>j ; k--)
		{
			sorted[team][k] = sorted[team][k-1];
			sortedscores[team][k] = sortedscores[team][k-1];
		}
		sorted[team][j] = i;
		sortedscores[team][j] = score;
		totalscore[team] += score;
		total[team]++;
	}

	// print level name and exit rules
	// add the clients in sorted order
	*string = 0;
	len = 0;

	// ADDED!!!!!!!!!!!!

	string[0] = 0;
	stringlength = strlen(string);

	picnum = gi.imageindex ("i_fixme");
		x = (i>=6) ? 160 : 0;
		y = 32 + 32 * (i%6);

		tag = "tag2";
		
		Com_sprintf (entry, sizeof(entry),
				"xv %i yv %i picn %s "
			    "xv %i yv %i string \"%10s\" "		
				,x-58, 5, tag, x-55, 18, myHeader);

			strcat(string, entry);	
					
			j = strlen(entry);

			strcpy (string + stringlength, entry);
			stringlength += j;


	// END ADDED!!!!!!!!!!!!

	len = myLength; 
	
	for (i=0 ; i<32 ; i++) // Let's fill up array with 32
	{
		if (i >= total[0] /*&& i >= total[1]*/)
			break; // we're done

#if 0 //ndef NEW_SCORE
		// set up y
		sprintf(entry, "yv %d ", 42 + i * 8);
		if (maxsize - len > strlen(entry)) {
			strcat(string, entry);
			len = myLength; /*strlen(string);*/
		}
#else
		*entry = 0;
#endif

		// left side
		if (i < total[0]) {
			cl = &game.clients[sorted[0][i]];
			cl_ent = g_edicts + 1 + sorted[0][i];

#if 0 //ndef NEW_SCORE
			sprintf(entry+strlen(entry),
			"xv 0 %s \"%3d %3d %-12.12s\" ",
			(cl_ent == ent) ? "string2" : "string",
			cl->resp.score, 
			(cl->ping > 999) ? 999 : cl->ping, 
			cl->pers.netname);

#else
			// If we have < 12 clients, center their scores
			if (total[0] < 12) // (0..11 = 12 people)
			{
				sprintf(entry+strlen(entry),
					"ctf 80 %d %d %d %d ",
					42 + i * 8,
					sorted[0][i], /* Sorted */
					cl->resp.score,
					cl->ping > 999 ? 999 : cl->ping);
			}
			else { // Enter below if we have >= 10 clients
				if (i<12) {
				sprintf(entry+strlen(entry),
					"ctf 0 %d %d %d %d ",
					42 + i * 8,
					sorted[0][i], /* Sorted */
					cl->resp.score,
					cl->ping > 999 ? 999 : cl->ping);
				} else if (i>=12 && i<24) {
				sprintf(entry+strlen(entry),
					"ctf 160 %d %d %d %d ",
					42 + i * 8,
					sorted[0][i], 
					cl->resp.score,
					cl->ping > 999 ? 999 : cl->ping);
				}
			} // End else to determine if clients in game >= 15

#endif

			if (maxsize - len > strlen(entry)) {
				strcat(string, entry);
				len = myLength; /*strlen(string);*/
				last[0] = i;
			}
		}


	}

	// put in spectators if we have enough room
	//if (last[0] > last[1])
		j = last[0];
	//else
	j = (j + 2) * 8 + 42;

	k = n = 0;
	if (maxsize - len > 50) {
		for (i = 0; i < maxclients->value; i++) {
			cl_ent = g_edicts + 1 + i;
			cl = &game.clients[i];
			if (!cl_ent->inuse ||
				cl_ent->solid != SOLID_NOT ||
				cl_ent->client->resp.ctf_team != CTF_NOTEAM)
				continue;

			if (!k) {
				k = 1;
				sprintf(entry, "xv 0 yv %d string2 \"Spectators\" ", j);
				strcat(string, entry);
				len = myLength; /*strlen(string);*/
				j += 8;
			}

			sprintf(entry+strlen(entry),
				"ctf %d %d %d %d %d ",
				(n & 1) ? 160 : 0, // x
				j, // y
				i, // playernum
				cl->resp.score,
				cl->ping > 999 ? 999 : cl->ping);
			if (maxsize - len > strlen(entry)) {
				strcat(string, entry);
				len = strlen(string);
			}
			
			if (n & 1)
				j += 8;
			n++;
		}
	}

	if (total[0] - last[0] > 1) // couldn't fit everyone
		sprintf(string + strlen(string), "xv 8 yv %d string \"..and %d more\" ",
			42 + (last[0]+1)*8, total[0] - last[0] - 1);
	if (total[1] - last[1] > 1) // couldn't fit everyone
		sprintf(string + strlen(string), "xv 168 yv %d string \"..and %d more\" ",
			42 + (last[1]+1)*8, total[1] - last[1] - 1);

	gi.WriteByte (svc_layout);
	gi.WriteString (string);

//	gi.sound(ent, CHAN_VOICE, gi.soundindex("world/xianbeats.wav"), 1, ATTN_IDLE, 0);
}

// SCONFIG STOP



/*
==================
DeathmatchScoreboard

Draw instead of help message.
Note that it isn't that hard to overflow the 1400 byte message limit!
8 bytes in a single char, giving us 175 characters....
names for 15 clients * 15 chars each at 8 bytes each total 1800 bytes
15 peoples frags (assuming all 3 chars at 8 bytes each) total 360 bytes
rank (1-9) is 9 chars, 10,11,12,13,14,15 = 12 chars, 21 chars total 168 bytes
date 01/01/99 is 8 chars by 15 players total = 960
w/o header approx =3463 chars.. hmmmm
==================
*/
void DeathmatchScoreboard (edict_t *ent)
{
	if (scoreboard == 0) 
		DeathmatchScoreboardMessage (ent, ent->enemy);
	else if (scoreboard == 1)
		DeathmatchScoreboardMessage2 (ent, ent->enemy);
	else if (scoreboard == 2)
		DeathmatchScoreboardMessage3 (ent, ent->enemy);
	else
		DeathmatchScoreboardMessage (ent, ent->enemy);
	gi.unicast (ent, true);
}


/*
==================
Cmd_Score_f

Display the scoreboard
==================
*/
void Cmd_Score_f (edict_t *ent)
{
	ent->client->showinventory = false;
	ent->client->showhelp = false;
	
	// SCONFIG START
	// ZOID
	if (ent->client->menu)
		PMenu_Close(ent);
	// ZOID
	// SCONFIG STOP


	if (!deathmatch->value && !coop->value)
		return;

	if (ent->client->showscores)
	{
		ent->client->showscores = false;
		return;
	}

	ent->client->showscores = true;
	
	DeathmatchScoreboard (ent);
	
}


/*
==================
HelpComputer

Draw help computer.
==================
*/
void HelpComputer (edict_t *ent)
{
	char	string[1024];
	char	*sk;

	if (skill->value == 0)
		sk = "easy";
	else if (skill->value == 1)
		sk = "medium";
	else if (skill->value == 2)
		sk = "hard";
	else
		sk = "hard+";

	// send the layout
	Com_sprintf (string, sizeof(string),
		"xv 32 yv 8 picn help "			// background
		"xv 202 yv 12 string2 \"%s\" "		// skill
		"xv 0 yv 24 cstring2 \"%s\" "		// level name
		"xv 0 yv 54 cstring2 \"%s\" "		// help 1
		"xv 0 yv 110 cstring2 \"%s\" "		// help 2
		"xv 50 yv 164 string2 \" kills     goals    secrets\" "
		"xv 50 yv 172 string2 \"%3i/%3i     %i/%i       %i/%i\" ", 
		sk,
		level.level_name,
		game.helpmessage1,
		game.helpmessage2,
		level.killed_monsters, level.total_monsters, 
		level.found_goals, level.total_goals,
		level.found_secrets, level.total_secrets);

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
	gi.unicast (ent, true);
}


/*
==================
Cmd_Help_f

Display the current help message
==================
*/
void Cmd_Help_f (edict_t *ent)
{
	// this is for backwards compatability
	if (deathmatch->value)
	{
		Cmd_Score_f (ent);
		return;
	}

	ent->client->showinventory = false;
	ent->client->showscores = false;

	if (ent->client->showhelp && (ent->client->resp.game_helpchanged == game.helpchanged))
	{
		ent->client->showhelp = false;
		return;
	}

	ent->client->showhelp = true;
	ent->client->resp.helpchanged = 0;
	HelpComputer (ent);
}


//=======================================================================

/*
===============
G_SetStats
===============
*/
void G_SetStats (edict_t *ent)
{
	gitem_t		*item;
	int			index, cells;
// SCONFIG START
	int			myTime;
// SCONFIG STOP
	int			power_armor_type;
	
	// Init routine
	int my_score;
	short int place;
	int xz;
	edict_t		*cl_ent;	

	int total;
	int i;
	//
	// health
	//
	ent->client->ps.stats[STAT_HEALTH_ICON] = level.pic_health;
	ent->client->ps.stats[STAT_HEALTH] = ent->health;

	//
	// ammo
	//
	if (!ent->client->ammo_index /* || !ent->client->pers.inventory[ent->client->ammo_index] */)
	{
		ent->client->ps.stats[STAT_AMMO_ICON] = 0;
		ent->client->ps.stats[STAT_AMMO] = 0;
	}
	else
	{
		item = &itemlist[ent->client->ammo_index];
		ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex (item->icon);
		ent->client->ps.stats[STAT_AMMO] = ent->client->pers.inventory[ent->client->ammo_index];
	}
	
	//
	// armor
	//
	power_armor_type = PowerArmorType (ent);
	if (power_armor_type)
	{
		cells = ent->client->pers.inventory[ITEM_INDEX(FindItem ("cells"))];
		if (cells == 0)
		{	// ran out of cells for power armor
			ent->flags &= ~FL_POWER_ARMOR;
			gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/power2.wav"), 1, ATTN_NORM, 0);
			power_armor_type = 0;;
		}
	}

	index = ArmorIndex (ent);
	if (power_armor_type && (!index || (level.framenum & 8) ) )
	{	// flash between power armor and other armor icon
		ent->client->ps.stats[STAT_ARMOR_ICON] = gi.imageindex ("i_powershield");
		ent->client->ps.stats[STAT_ARMOR] = cells;
	}
	else if (index)
	{
		item = GetItemByIndex (index);
		ent->client->ps.stats[STAT_ARMOR_ICON] = gi.imageindex (item->icon);
		ent->client->ps.stats[STAT_ARMOR] = ent->client->pers.inventory[index];
	}
	else
	{
		ent->client->ps.stats[STAT_ARMOR_ICON] = 0;
		ent->client->ps.stats[STAT_ARMOR] = 0;
	}

	//
	// pickup message
	//
	if (level.time > ent->client->pickup_msg_time)
	{
		ent->client->ps.stats[STAT_PICKUP_ICON] = 0;
		ent->client->ps.stats[STAT_PICKUP_STRING] = 0;
	}

	//
	// timers
	//
	if (ent->client->quad_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_quad");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->quad_framenum - level.framenum)/10;
	}
	else if (ent->client->invincible_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_invulnerability");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->invincible_framenum - level.framenum)/10;
	}
	else if (ent->client->enviro_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_envirosuit");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->enviro_framenum - level.framenum)/10;
	}
	else if (ent->client->breather_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_rebreather");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->breather_framenum - level.framenum)/10;
	}
	else
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = 0;
		ent->client->ps.stats[STAT_TIMER] = 0;
	}

	//
	// selected item
	//
	if (ent->client->pers.selected_item == -1)
		ent->client->ps.stats[STAT_SELECTED_ICON] = 0;
	else
		ent->client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex (itemlist[ent->client->pers.selected_item].icon);

	ent->client->ps.stats[STAT_SELECTED_ITEM] = ent->client->pers.selected_item;

	//
	// layouts
	//
	ent->client->ps.stats[STAT_LAYOUTS] = 0;

	if (deathmatch->value)
	{
		if (ent->client->pers.health <= 0 || level.intermissiontime
			|| ent->client->showscores)
			ent->client->ps.stats[STAT_LAYOUTS] |= 1;
		if (ent->client->showinventory && ent->client->pers.health > 0)
			ent->client->ps.stats[STAT_LAYOUTS] |= 2;
	}
	else
	{
		if (ent->client->showscores || ent->client->showhelp)
			ent->client->ps.stats[STAT_LAYOUTS] |= 1;
		if (ent->client->showinventory && ent->client->pers.health > 0)
			ent->client->ps.stats[STAT_LAYOUTS] |= 2;
	}

	//
	// frags
	//
	ent->client->ps.stats[STAT_FRAGS] = ent->client->resp.score;

//	if (timehud == 0 && rankhud == 0 && playershud == 0)		// (000) - 0
//	else if (timehud == 0 && rankhud == 0 && playershud == 1)   // (001) - 1 X
//	else if (timehud == 0 && rankhud == 1 && playershud == 1)   // (011) - 2 X
//	else if (timehud == 1 && rankhud == 1 && playershud == 1)   // (111) - 3 X
//	else if (timehud == 1 && rankhud == 0 && playershud == 0)   // (100) - 4 X
//	else if (timehud == 1 && rankhud == 1 && playershud == 0)   // (110) - 5 X
//	else if (timehud == 1 && rankhud == 0 && playershud == 1)   // (101) - 6
//	else if (timehud == 0 && rankhud == 1 && playershud == 0)   // (010) - 7 X



	if (timehud==0)
		ent->client->ps.stats[STAT_TIMELEFT] = 0;
	else {
	// SCONFIG START
	// Note if you change the timelimit variable in the middle
	// of the game it subtracts what time has already been used.
	// ie. I've been on the board 4 minutes, setting time limit
	// from console will show 6 minites remaining, hence 10-4=6.
	if (timelimit->value && !level.intermissiontime)
	{
     	// Make sure when we hit 3 minutes it shows 3 until it hits 2 minutes
		myTime=((((timelimit->value*60)/*+1*/) - level.time) / 60) +1;
		if ( myTime > 1)
				ent->client->ps.stats[STAT_TIMELEFT] =   myTime;
		else if (myTime > 0) 
		      // version 2.3 (coundown timer)
			if (((timelimit->value*60) - level.time) == 11) {
		      gi.sound(ent, CHAN_NO_PHS_ADD, gi.soundindex("world/10_0.wav"), 1, ATTN_NONE, 0);	
			  ent->client->ps.stats[STAT_TIMELEFT] = ((timelimit->value*60) - level.time);
			}
			else // End 2.3 countdown timer
				ent->client->ps.stats[STAT_TIMELEFT] = ((timelimit->value*60) - level.time);
			
		else
			ent->client->ps.stats[STAT_TIMELEFT] = 0;
	}
	}

	if (rankhud==0)
		ent->client->ps.stats[STAT_RANK] = 0;
	else {
	// RANKING
	my_score = ent->client->resp.score;
	place=1;
	for (xz=0 ; xz<game.maxclients ; xz++)
	{
		cl_ent = g_edicts + 1 + xz;
		if (!cl_ent->inuse)
			continue;
		if (game.clients[xz].resp.score > my_score) 
			place++;	
	}
		ent->client->ps.stats[STAT_RANK] = place;
	} // end else disableranking

	if (playershud==0)
		ent->client->ps.stats[STAT_PLAYERS] = 0;
	else {
	// PLAYERS IN GAME
	total = 0;
	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;
		total++;
	}
		ent->client->ps.stats[STAT_PLAYERS] = total;
	} // end else disableplayers
	
	// Check Prevents cells appearing in float mode if user went into chase
	// with cloak active.
	if (ent->client->hudcloak_state && ent->movetype != MOVETYPE_NOCLIP) {
	cells = ent->client->pers.inventory[ITEM_INDEX(FindItem ("cells"))];
	if (cells !=0){
	ent->client->ps.stats[STAT_CLOAKCELLS] = 1;
	ent->client->ps.stats[STAT_NUMCLOAKCELLS] = cells;
	}
	else{
		ent->client->ps.stats[STAT_NUMCLOAKCELLS] = 0;
		ent->client->ps.stats[STAT_CLOAKCELLS] = 0;
		ent->client->hudcloak_state=false;
	}

	}
	else
	ent->client->ps.stats[STAT_CLOAKCELLS] = 0;


	//
	// help icon / current weapon if not shown
	//
	if (ent->client->resp.helpchanged && (level.framenum&8) )
		ent->client->ps.stats[STAT_HELPICON] = gi.imageindex ("i_help");
	else if ( (ent->client->pers.hand == CENTER_HANDED || ent->client->ps.fov > 91)
		&& ent->client->pers.weapon)
		ent->client->ps.stats[STAT_HELPICON] = gi.imageindex (ent->client->pers.weapon->icon);
	else
		ent->client->ps.stats[STAT_HELPICON] = 0;

	
	HelpHud(ent);
	// SCONFIG START
	SetCTFStats(ent);
	// SCONFIG STOP
}

