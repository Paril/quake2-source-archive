#include "g_local.h"
#include <stdlib.h>

#include "DeCamper.h"
//#include "ModUtils.h"

struct VotingRegistry CamperVotingRegistry;

// Linked list of Campers (nominated, immune, or decamped)
struct Camper *AllCampers;

static char *DeCamperPassword = NULL;


// Slightly adapted from Q_strncasecmp.
int StrBeginsWith (char *s1, char *s2)
	{
	int		c1, c2, max = 999999;
	
	do
	{
		c1 = *s1++;
		c2 = *s2++;
			if (!c1) return(1); // Reached end of search string
		if (!max--)
			return 1;		// strings are equal until end point
		
		if (c1 != c2)
		{
			if (c1 >= 'a' && c1 <= 'z')
				c1 -= ('a' - 'A');
			if (c2 >= 'a' && c2 <= 'z')
				c2 -= ('a' - 'A');
			if (c1 != c2)
				return 0;		// strings not equal
		}
	} while (c1);
	
	return 1;		// strings are equal
}

void PrintVotingRegistry ()
{
	struct CamperVoter *voter;

	voter = CamperVotingRegistry.voters;
	while (voter != NULL)
	{
//		BPRINT1("VOTE: %s is a voter\n", voter->edict->client->pers.netname);
		voter = voter->next;
	}
}

void SetPlayerShellColor (edict_t *ent, int color)
{
	if (color == DC_SHELL_COLOR_NONE)
	{
		ent->s.renderfx &= ~DC_SHELL_COLOR_WHITE;
		ent->s.effects &= ~EF_COLOR_SHELL;
	}
	else {
		// "erase" any other colors that were set
		ent->s.renderfx &= ~DC_SHELL_COLOR_WHITE;
		ent->s.renderfx |= color;
		ent->s.effects |= EF_COLOR_SHELL;
	}
}

edict_t *FindClientByNetname (char *name)
{
	int n;
	edict_t *player;

	// Code obtained from PlayersRangeFromSpot in p_client.c
	for (n = 1; n <= maxclients->value; n++)
	{
		player = &g_edicts[n];
		if (!player->inuse)
			continue;
		if (!player->client)
			continue;
		if (!player->client->pers.netname)
			continue;
		if (!Q_strcasecmp(name, player->client->pers.netname))
			return(player);
	}
	return(NULL);
}

edict_t *FindClientByMatchingNetname (char *name)
{
	int n, match_count;
	edict_t *player, *match_player;
	char *match;

	match_count = 0;
	// Code obtained from PlayersRangeFromSpot in p_client.c
	for (n = 1; n <= maxclients->value; n++)
	{
		player = &g_edicts[n];
		if (!player->inuse)
			continue;
		if (!player->client)
			continue;
		if (!player->client->pers.netname)
			continue;
		match = strstr(player->client->pers.netname, name);

		if (match != NULL) {
			match_player = player;
			match_count++;
		}
	}
	if (match_count == 1)
		return(match_player);
	else return(NULL);
}



struct Camper *FindCamper (edict_t *ent)
{
	struct Camper *cur;

	cur = AllCampers;
	while (cur != NULL)
	{
		if (cur->edict == ent)
			return(cur);
		cur = cur->next;
	}
	return(NULL);
}


struct CamperVoter *FindCamperVoter (edict_t *ent)
{
	struct CamperVoter *cur;

	cur = CamperVotingRegistry.voters;
	while (cur != NULL)
	{
		if (cur->edict == ent)
			return(cur);
		cur = cur->next;
	}
	return(NULL);
}

void AddCamperToList (struct Camper *camper)
{
	struct Camper *cur, *last;

	// Redo: just push it to the beginning of the list!  Duh...

	if (AllCampers) {
		last = AllCampers;
		while ((cur = last->next) != NULL)
			last = cur;
		last->next = camper;
		camper->prev = last;
	}
	else {
		AllCampers = camper;
	}
}


struct Camper *CreateCamper (edict_t *ent)
{
	struct Camper *camper;
	camper = malloc(sizeof(struct Camper));
	camper->name = ent->client->pers.netname;
	camper->status = DC_CAMPER_STATUS_NONE;
	camper->change_time = 0;
	camper->prev = NULL;
	camper->next = NULL;
	camper->edict = ent;
	camper->inventory = NULL;
	AddCamperToList(camper);
	return(camper);
}

struct CamperVoter *CreateCamperVoter (edict_t *client)
{
	struct CamperVoter *voter;
	voter = malloc(sizeof(struct CamperVoter));
	voter->vote = DC_VOTE_NONE;
	voter->edict = client;
	voter->next = NULL;
	voter->prev = NULL;
	return(voter);
}

void DeleteCamperVoter (edict_t *ent)
{
	struct CamperVoter *voter = FindCamperVoter(ent);
	if (! voter) return;
	if (voter->prev)
		(voter->prev)->next = voter->next;
	else 
		CamperVotingRegistry.voters = voter->next;
	if (voter->next)
		(voter->next)->prev = voter->prev;
	free(voter);
	CamperVotingRegistry.num_voters--;
}

int CamperVoteExpired ()
{
	if (CamperVotingRegistry.nominated_camper)
	{
		if (CamperVotingRegistry.expiration <= level.time)
		   return(1);
		else return(0);
	}
	else return(0);
}

int CamperVoteOccurring ()
{
	if (CamperVotingRegistry.nominated_camper)
		return(1);
	else return(0);
}

// FIXME: set shell color to "alternate" with other shell colors
// (use logical &4 or somesuch, like when powerups fade)
// See G_SetClientEffects in p_view.c
void RenderCamperEffects (edict_t *ent)
{
	struct Camper *camper = FindCamper(ent);
	int		pa_type;
	int		remaining;
	int has_effects = 0;

	if (!camper) return;

	// Adapted from G_SetClientEffects in p_view.c
	if (ent->powerarmor_time > level.time)
	{
		pa_type = PowerArmorType (ent);
		if (pa_type == POWER_ARMOR_SCREEN)
			has_effects = 1;
		else if (pa_type == POWER_ARMOR_SHIELD)
			has_effects = 1;
	}
	if (ent->client->quad_framenum > level.framenum)
	{
		remaining = ent->client->quad_framenum - level.framenum;
		if (remaining > 30 || (remaining & 4) )
			has_effects = 1;
	}
	if (ent->client->invincible_framenum > level.framenum)
	{
		remaining = ent->client->invincible_framenum - level.framenum;
		if (remaining > 30 || (remaining & 4) )
			has_effects = 1;
	}

	// show cheaters!!!
	if (ent->flags & FL_GODMODE) has_effects = 1;


	if (camper->status == DC_CAMPER_STATUS_DECAMPED) 
	{
		// If powerups generate shell effects, alternate them with the yellow
		if (has_effects)
		{
			if (level.framenum & 4) {
				ent->s.effects = 0;
				SetPlayerShellColor(camper->edict, DC_SHELL_COLOR_YELLOW);
			}
		}
		else SetPlayerShellColor(camper->edict, DC_SHELL_COLOR_YELLOW);
	}
	else if (camper->status == DC_CAMPER_STATUS_NOMINATED)
	{
		// Rapidly flash white!  Try a different color??
		if (level.framenum & 2) {
			ent->s.effects = 0;
			SetPlayerShellColor(camper->edict, DC_SHELL_COLOR_WHITE);
		}
	}
}

int HasBeenDeCamped (edict_t *ent)
{
	struct Camper *camper = FindCamper(ent);

	if (camper && (camper->status == DC_CAMPER_STATUS_DECAMPED))
		return(1);
	else return(0);
}

// This should also delete/change Camper objects

void FreeCamperVoterObjects ()
{
	struct CamperVoter *voter, *next;

	voter = CamperVotingRegistry.voters;
	while (voter != NULL)
	{
		next = voter->next;
		free(voter);
		voter = next;
	}
	CamperVotingRegistry.voters = NULL;
}


void PrepareForNextCamperVote ()
{
	// Camper status is now official and in AllCampers
	CamperVotingRegistry.nominated_camper = NULL;
	FreeCamperVoterObjects();
	CamperVotingRegistry.num_voters = 0;
	CamperVotingRegistry.yes_votes = 0;
	CamperVotingRegistry.no_votes = 0;
	// 10 frames per second * 60 secs per minute
	CamperVotingRegistry.next_nomination = level.time + 
										(CamperVotingRegistry.vote_disabled_minutes * 10 * 60);
	CamperVotingRegistry.expiration = 0;
}



// ONLY CALL THIS ON A LEVEL CHANGE! (like the name implies, duh)
void AdjustCamperLevelChangeInfo ()
{
	struct Camper *camper;
	camper = AllCampers;
	while (camper != NULL)
	{
		camper->change_time = camper->change_time - level.time;
		camper = camper->next;
	}
}

void DeleteCamperInventory (struct Camper *camper)
{
	struct CamperInventoryItem *item, *last;

	item = camper->inventory;
	while (item != NULL)
	{
		last = item->next;
		free(item);
		item = last;
	}
	camper->inventory = NULL;
}


// SteQve 12/15/98.  Sometimes want to delete a camper (e.g. on disconnect)
// which could cancel a vote; other times a cancel vote is an action that
// deletes a camper.  It's cyclical...
void CancelCamperVote (char *reason);


void DeleteCamper (struct Camper *camper)
{

	if (camper->prev)
		(camper->prev)->next = camper->next;
	else
		AllCampers = camper->next;
	if (camper->next)
		(camper->next)->prev = camper->prev;
	if (CamperVotingRegistry.nominated_camper == camper)
		CancelCamperVote("the nominated camper disconnected");
	DeleteCamperInventory(camper);
	free(camper);
}



void CancelCamperVote (char *reason)
{
	struct Camper *nominee;

	BPRINT1("DeCamper: Voting canceled because %s\n", reason);
	if (CamperVotingRegistry.nominated_camper != NULL)
	{
		// Clear the nominee so as to avoid cyclical calls (cancel vote
		// calls deletecamper which calls cancel vote which calls deletecamper...
		// etc. etc. etc.
		nominee = CamperVotingRegistry.nominated_camper;
		CamperVotingRegistry.nominated_camper = NULL;
		DeleteCamper(nominee);
	}
	PrepareForNextCamperVote();
	// hack: wipe out pre-set next_nomination.
	CamperVotingRegistry.next_nomination = 0;
}


// If a vote is occurring, cancel it.  For all campers, adjust their time left
// by subtracting the "previous" level's time.
void CamperVoteChangeLevels ()
{
	if (CamperVoteOccurring())
		CancelCamperVote("the level changed");
	CamperVotingRegistry.next_nomination = 0;
	AdjustCamperLevelChangeInfo();
}

// Generally, make the ent the nominee.  Frankly I don't really know how this
// gi.sound thing really works.
void DeCamperNotifySound (edict_t *ent)
{
	gi.sound(ent, CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex("misc/keytry.wav"),
			 1, ATTN_NONE, 0);
}

int CamperCanPickupInventoryItem (edict_t *ent, edict_t *item)
{
	struct CamperInventoryItem *itemobj;
	struct Camper *camper = FindCamper(ent);

	if (!camper) return(1);

	// Nominated/immune/etc. campers can pick up items
	if (camper->status != DC_CAMPER_STATUS_DECAMPED)
		return(1);

	itemobj = camper->inventory;
	while (itemobj != NULL)
	{
		if (itemobj->edict == item) return(0);
		itemobj = itemobj->next;
	}
	// Not in inventory... but it WILL be, VERY soon ;-)
	return(1);
}


void AddCamperInventoryItem (edict_t *ent, edict_t *item, int respawn_time)
{
	struct Camper *camper = FindCamper(ent);
	struct CamperInventoryItem *item_obj;

	if (! camper) return;

	if (camper->status != DC_CAMPER_STATUS_DECAMPED) return;

	// just do a "push" which is maybe a little more efficient anyway
	// assuming the camper tries to get a respawned item
	item_obj = malloc(sizeof(struct CamperInventoryItem));
	item_obj->edict = item;
	item_obj->expiration = level.time + respawn_time;
	item_obj->next = NULL;
	item_obj->prev = NULL;
	if (camper->inventory) {
		item_obj->next = camper->inventory;
		camper->inventory->prev = item_obj;
		camper->inventory = item_obj;
	}
	else
		camper->inventory = item_obj;
}


void RemoveExpiredCamperInventoryItems (struct Camper *camper)
{
	struct CamperInventoryItem *item, *last;
	int count = 0;

	// If I did this correctly in all the 10 other places I had to remember
	// to do, status *should* be DeCamped.  But why error check when you can
	// rely on blind faith?  The computer knows what I mean.

	if (camper->status != DC_CAMPER_STATUS_DECAMPED)
		return;

	item = camper->inventory;
	while (item != NULL)
	{
		if (item->expiration <= level.time)
		{
			if (item == camper->inventory)
				camper->inventory = item->next;
			if (item->next)
				item->next->prev = item->prev;
			if (item->prev)
				item->prev->next = item->next;
			last = item->next;
			free(item);
			item = last;
			count++;
		}
		else item = item->next;
	}
	//if (count)
	//	BPRINT2("DeCamper: Removed %d inventory items for %s\n",
	//			count, camper->name);
}


void FailedDeCampNominee (char *banner)
{
	struct Camper *nominee;

	nominee = CamperVotingRegistry.nominated_camper;
	DeCamperNotifySound(nominee->edict);
	BPRINT0(banner);
	if (CamperVotingRegistry.immune_time)
	{
		nominee->status = DC_CAMPER_STATUS_IMMUNE;
		nominee->change_time = level.time + CamperVotingRegistry.immune_time;
	}
	else {
		// Nominated camper is not immune!
		nominee->status = DC_CAMPER_STATUS_NONE;
		nominee->change_time = 0;
	}

}


void DeCampNominee (char *banner)
{
	struct Camper *nominee;

	nominee = CamperVotingRegistry.nominated_camper;
	nominee->status = DC_CAMPER_STATUS_DECAMPED;
	nominee->change_time = level.time + (CamperVotingRegistry.decamp_time_minutes * 60);
	if (CamperVotingRegistry.frag_reduce)
		nominee->edict->client->resp.score -= CamperVotingRegistry.frag_reduce;
	DeCamperNotifySound(nominee->edict);
	BPRINT0(banner);
}

void DeCamperEndLevel ()
{
	struct Camper *camper;
	// Do stuff at the end of the level
	CamperVoteChangeLevels();

	// Delete all camper inventories.
	camper = AllCampers;
	while (camper != NULL) {
		DeleteCamperInventory(camper);
		camper = camper->next;
	}
}


// Called once each frame.
void EvaluateDeCamperVote ()
{
	int perc;
	char banner[512];
	int decamp = 0, yes, no, num_voters;

	if (! CamperVotingRegistry.nominated_camper)
		return;

	yes = CamperVotingRegistry.yes_votes;
	no = CamperVotingRegistry.no_votes;
	num_voters = CamperVotingRegistry.num_voters;

	// Every 10 seconds, print a vote tally?
	if (level.framenum % 100 == 0) {
		//BPRINT2("Frame level is %d; time is %3.2f\n", level.framenum, level.time);
		BPRINT4("DeCamper Vote: %d YES, %d NO so far (%d voters)\n     %d seconds remaining\n", 
			yes, no, num_voters, (int)(CamperVotingRegistry.expiration - level.time));
		//BPRINT2("DeCamper: cur time is %3.2f, expiration is %3.1f\n", level.time, CamperVotingRegistry.expiration);
	}

	if (! CamperVoteExpired())
		return;

	if (num_voters <= 0)
	{
		DeCamperNotifySound(CamperVotingRegistry.nominated_camper->edict);
		CancelCamperVote("there were not enough voters");
		return;
	}

	// Hack: in the sprintfs, there is a %%%% to print a single percent sign.  This
	// prints a double % to the buffer, which is used as a format string
	// in the BPRINT - so this generates a single %.  Icky, huh?
	//
	// SteQve 12/15/98.  I must have been loopy when I did this formatting stuff. 
	// It should be easily changed, though - but by YOU, not ME!  Bwa ha ha ha!
	banner[0] = 0;
	if (CamperVotingRegistry.min_num_yes) {
		perc = (yes * 100) / num_voters;
		if (yes >= CamperVotingRegistry.min_num_yes) {
			decamp = 1;
			sprintf(banner, "%s has been DeCamped!  %d yes and %d no (%d%%%% of %d voters); a minimum %d was required\n",
					CamperVotingRegistry.nominated_camper->name,
					yes, no, perc, CamperVotingRegistry.num_voters,
					CamperVotingRegistry.min_num_yes);
		}
		else {
			decamp = 0;
			sprintf(banner, "%s could not be DeCamped!  %d yes and %d no (%d%%%% of %d voters); a minimum %d%%%% was required\n",
					CamperVotingRegistry.nominated_camper->name,
					yes, no, perc, CamperVotingRegistry.num_voters,
					CamperVotingRegistry.min_num_yes);
		}
	}
	if (!decamp)
	{
		if (CamperVotingRegistry.min_perc_yes)
		{
			perc = (yes * 100) / num_voters;
			if (perc >= CamperVotingRegistry.min_perc_yes) {
				decamp = 1;
				sprintf(banner, "%s has been DeCamped!  %d yes and %d no (%d%%%% of %d voters); a minimum %d%%%% was required\n",
					CamperVotingRegistry.nominated_camper->name,
					yes, no, perc, CamperVotingRegistry.num_voters,
					CamperVotingRegistry.min_perc_yes);
			}
			else {
				decamp = 0;
				sprintf(banner, "DeCamper: %s was not DeCamped!  %d yes and %d no (%d%%%% of %d voters); a minimum %d%%%% was required\n",
						CamperVotingRegistry.nominated_camper->name,
						yes, no, perc, CamperVotingRegistry.num_voters,
						CamperVotingRegistry.min_perc_yes);
			}
		}
		else {
		// No min % or min yes specified!?  *** set this on initialize, or disable DeCamper
			decamp = 0;
			sprintf(banner, "DeCamper: %s was not DeCamped!  The server operator did not specify minimum vote requirements\n");
		}
	}
	if (decamp)
	{
		DeCampNominee(banner);
		PrepareForNextCamperVote();
	}
	else 
	{
		FailedDeCampNominee(banner);
		PrepareForNextCamperVote();
	}

}

void EvaluateDeCamperState ()
{
	struct Camper *camper;

	EvaluateDeCamperVote();

	// Only clean up inventories every 5 seconds
	if (level.framenum % 50 == 0)
	{
		camper = AllCampers;
		while (camper != NULL) {
			RemoveExpiredCamperInventoryItems(camper);
			camper = camper->next;
		}
	}
}


int SetDeCamperOption (char *option, char *value)
{

	int val = atoi(value);

	if (val < 0) val = 0;

	if (!Q_stricmp(option, "pickup_powerup")) {
		CamperVotingRegistry.pickup_powerup = val;
//		CPRINT1(ent, "DeCamper: Pickup powerup set to %d\n", val);
	}
	else if (!Q_stricmp(option, "frag_reduce")) {
		CamperVotingRegistry.frag_reduce = val;
//		CPRINT1(ent, "DeCamper: Frag reduce set to %d\n", val);
	}
	else if (!Q_stricmp(option, "min_num_yes")) {
		CamperVotingRegistry.min_num_yes = val;
//		CPRINT1(ent, "DeCamper: Minimum number yes votes set to %d\n", val);
	}
	else if (!Q_stricmp(option, "min_perc_yes")) {
		CamperVotingRegistry.min_perc_yes = val;
//		CPRINT1(ent, "DeCamper: Minimum perc yes votes set to %d\n", CamperVotingRegistry.min_perc_yes);
	}
	else if (!Q_stricmp(option, "vote_seconds")) {
		CamperVotingRegistry.vote_seconds = val;
//		CPRINT1(ent, "DeCamper: Vote_seconds set to %d\n", val);
	}
	else if (!Q_stricmp(option, "vote_disabled_minutes")) {
		CamperVotingRegistry.vote_disabled_minutes = val;
//		CPRINT1(ent, "DeCamper: Vote disabled minutes set to %d\n", val);
	}
	else if (!Q_stricmp(option, "decamp_time_minutes")) {
		CamperVotingRegistry.decamp_time_minutes = val;
//		CPRINT1(ent, "DeCamper: decamp_time_minutes set to %d\n", val);
	}
	else if (!Q_stricmp(option, "immune_time")) {
		CamperVotingRegistry.immune_time = val;
//		CPRINT1(ent, "DeCamper: immune_time set to %d\n", val);
	}
	else if (!Q_stricmp(option, "replace_powerup")) {
		CamperVotingRegistry.replace_powerup = val;
//		CPRINT1(ent, "DeCamper: Replace powerup set to %d\n", val);
	}
	else if (!Q_stricmp(option, "replace_weapon")) {
		CamperVotingRegistry.replace_weapon = val;
//		CPRINT1(ent, "DeCamper: Replace weapon set to %d\n", val);
	}
	else if (!Q_stricmp(option, "replace_ammo")) {
		CamperVotingRegistry.replace_ammo = val;
//		CPRINT1(ent, "DeCamper: Replace ammo set to %d\n", val);
	}
	else if (!Q_stricmp(option, "replace_armor")) {
		CamperVotingRegistry.replace_armor = val;
//		CPRINT1(ent, "DeCamper: Replace armor set to %d\n", val);
	}
	else if (!Q_stricmp(option, "damage_perc")) {
		CamperVotingRegistry.damage_perc = val;
//		CPRINT1(ent, "DeCamper: damage_perc set to %d\n", val);
	}
	else {
		return(0);
//		CPRINT1(ent, "DeCamper: Could not understand command '%s'\n", name);
	}
	return(1);
}


int VoteDeCamp (edict_t *ent, int vote)
{
	struct CamperVoter *voterobj;

	if (! CamperVotingRegistry.nominated_camper) {
		CPRINT0(ent, "Nobody has been nominated.  Use 'decamp nominate NAME'\n");
		return(0);
	}
	voterobj = FindCamperVoter(ent);
	if (voterobj == NULL)
	{
		CPRINT0(ent, "You are not part of this vote.\n");
		return(0);
	}
	// Cancel out previous votes.

	if (voterobj->vote == DC_VOTE_NO)
		CamperVotingRegistry.no_votes--;
	else if (voterobj->vote == DC_VOTE_YES)
		CamperVotingRegistry.yes_votes--;

	if (vote == DC_VOTE_YES)
	{
		voterobj->vote = DC_VOTE_YES;
		CamperVotingRegistry.yes_votes++;
	}
	else if (vote == DC_VOTE_NO)
	{
		voterobj->vote = DC_VOTE_NO;
		CamperVotingRegistry.no_votes++;
	}
	else {
		CPRINT0(ent, "Your vote did not register properly!\n");
	}
}

void ParseDeCamperInitFile ()
{
	FILE *init;
	char line[512], field[256];
	int pos, result;

	//BPRINT0("Starting InitGame stuff\n");
	init = fopen("DeCamper/DeCamper.ini", "r");
	if (init == NULL) {
		DPRINT0("DeCamper error: Could not open DeCamper/DeCamper.ini!!\n");
		return;
	}
	while (fgets(line, 512, init) != NULL)
	{
		if (line[0] == '#') continue;
		pos = strcspn(line,"=");
		if (line[pos] == '\0') continue;
		line[pos] = 0;
		strcpy(field, line);
		result = SetDeCamperOption(field, line + pos + 1);
		if (! result)
			DPRINT2("DeCamper: Could not set option '%s' to '%s'\n", field, line+pos+1);
	}
	fclose(init);
}


void SetDefaultDeCamperOptions ()
{
	CamperVotingRegistry.min_num_yes = 8;
	CamperVotingRegistry.min_perc_yes = 50;
	CamperVotingRegistry.vote_disabled_minutes = 1;
	CamperVotingRegistry.decamp_time_minutes = 1;
	CamperVotingRegistry.immune_time = 30;
	CamperVotingRegistry.frag_reduce = 5;
	CamperVotingRegistry.pickup_powerup = 1;
	CamperVotingRegistry.replace_weapon = 1;
	CamperVotingRegistry.replace_powerup = 1;
	CamperVotingRegistry.replace_ammo = 0;
	CamperVotingRegistry.replace_armor = 1;
	CamperVotingRegistry.damage_perc = 50;
	CamperVotingRegistry.vote_seconds = 30;
}

void InitializeCamperVotingRegistry ()
{
	// The commented lines are set when the game starts in InitGameSetDeCamperOptions
	CamperVotingRegistry.voters = NULL;
	CamperVotingRegistry.nominated_camper = NULL;
	CamperVotingRegistry.num_voters = 0;
	CamperVotingRegistry.yes_votes = 0;
	CamperVotingRegistry.no_votes = 0;
	CamperVotingRegistry.expiration = 0;
	CamperVotingRegistry.next_nomination = 0;
	CamperVotingRegistry.decamp_time_perc = 0;
}


void UnDeCamp (struct Camper *camper)
{
	camper->status = DC_CAMPER_STATUS_IMMUNE;
	camper->change_time = level.time + CamperVotingRegistry.immune_time;
	DeCamperNotifySound(camper->edict);
	DeleteCamperInventory(camper);
	BPRINT1("DeCamper: %s has been UnDeCamped\n", camper->name);
}


int CanVoteCamper (edict_t *ent)
{
	struct Camper *camper, *nominee;

	nominee = CamperVotingRegistry.nominated_camper;
	// The nominated camper can't vote
	// FIXME DECAMPER: uncomment this when voting is checked
	//if (nominee && (nominee->edict == ent))
	//	return(0);

	// If no camper info, entity can vote
	camper = FindCamper(ent);
	if (! camper)
		return(1);
	// Camper has no status or is immune from nomination for camping
	if ((camper->status == DC_CAMPER_STATUS_NONE) || 
		(camper->status == DC_CAMPER_STATUS_IMMUNE))
		return(1);
	// Already DeCamped

	// FIXME DECAMPER: remove this "if" and just return 0 when
	// voting is checked
	//return(1);
	return(0);
}
	

void SetupCamperVotingCommunity (struct Camper *camper)
{
	int n, total;
	edict_t *player;

	struct CamperVoter *lastvoter, *curvoter;

	InitializeCamperVotingRegistry();
	// This next line is needed ASAP.
	CamperVotingRegistry.nominated_camper = camper;
	lastvoter = NULL;
	total = 0;
	//BPRINT0("DeCamper WARNING: including camper nominee in the vote!!!!\n");
	// Code obtained from PlayersRangeFromSpot in p_client.c
	for (n = 1; n <= maxclients->value; n++)
	{
		player = &g_edicts[n];
		if (!player->inuse)
			continue;
		//if (player == camper->edict)
		//	continue;

		if (! CanVoteCamper(player))
			continue;

		curvoter = CreateCamperVoter(player);
		curvoter->prev = lastvoter;
		if (lastvoter) {
			lastvoter->next = curvoter;
		}
		else { // first voter
			CamperVotingRegistry.voters = curvoter;
		}
		total++;
		lastvoter = curvoter;
	}

	if (total == 0)
	{
		CancelCamperVote("there were not enough voters");
		return;
	}

	// Reset this after initial testing
	CamperVotingRegistry.num_voters = total;
	//BPRINT0("DeCamper WARNING: setting default total voters to 10!\n");
	//CamperVotingRegistry.num_voters = 10;
	CamperVotingRegistry.yes_votes = 0;
	CamperVotingRegistry.no_votes = 0;
	// Set expiration for vote
	CamperVotingRegistry.expiration = level.time + CamperVotingRegistry.vote_seconds;
}

void RemoveAllCamperInfo (edict_t *ent)
{
	struct Camper *camper;
	char *name = ent->client->pers.netname;

	if (CamperVoteOccurring())
		DeleteCamperVoter(ent);
	// Also, delete/change camper objects
	camper = FindCamper(ent);
	if (camper) DeleteCamper(camper);
	BPRINT1("DeCamper: %s disconnected\n", name);
	if (CamperVoteOccurring())
		BPRINT1("DeCamper: num voters is now %d\n", CamperVotingRegistry.num_voters);
}

// Called every second (every 10 frames).  Is this too slow?
void CheckCamperStatus (edict_t *ent)
{
	struct Camper *camper;

	camper = FindCamper(ent);
	if (!camper) return;
	if (camper->change_time > level.time) return;
	// make sure to reset name each time camper does.
	camper->name = ent->client->pers.netname;
	if (camper->status == DC_CAMPER_STATUS_IMMUNE)
	{
		DeCamperNotifySound(camper->edict);
		BPRINT1("DeCamper: %s is no longer immune from nomination\n", camper->name);
		DeleteCamper(camper);
	}
	else if (camper->status == DC_CAMPER_STATUS_DECAMPED) {
		UnDeCamp(camper);
	}
	else {
		// When would these other statuses (statii?) be action items?
	}
}


int NominateDeCamp (edict_t *nominator, edict_t *nominee)
{
	struct Camper *cur_camper;

	cur_camper = CamperVotingRegistry.nominated_camper;
	if (! (cur_camper == NULL))
	{
		CPRINT1(nominator, "A vote to DeCamp %s is already taking place.\n",
				cur_camper->name);
		return(0);
	}
	else {
		cur_camper = FindCamper(nominee);
		if (cur_camper)
		{
			if (cur_camper->status == DC_CAMPER_STATUS_IMMUNE)
			{
				CPRINT1(nominator, "%s is temporarily immune from camping nominations\n",
						cur_camper->name);
				return(0);
			}
			else if (cur_camper->status == DC_CAMPER_STATUS_DECAMPED)
			{
				CPRINT1(nominator, "%s is already DeCamped\n",
						cur_camper->name);
				return(0);
			}
		}
		else {
			cur_camper = CreateCamper(nominee);
		}
		cur_camper->status = DC_CAMPER_STATUS_NOMINATED;
		SetupCamperVotingCommunity(cur_camper);
		DeCamperNotifySound(cur_camper->edict);
		BPRINT1("A vote has been called to DeCamp %s!\n", cur_camper->edict->client->pers.netname);
		BPRINT0("To vote, say 'cmd decamp yes' or 'cmd decamp no' in your console\n");
	}
}

void DeCampHelp (edict_t *ent)
{
	CPRINT0(ent, "*** DeCamper Help ***\nTo nominate: cmd decamp nominate NAME\nTo nominate matching a name: cmd decamp nom_match PART_OF_NAME\nTo vote yes: cmd decamp yes\nTo vote no: cmd decamp no\nTo see options: cmd decamp options\n");
}


void DeCamperGreeting (edict_t *ent)
{
	gi.sound(ent, CHAN_VOICE, gi.soundindex("misc/keytry.wav"),
			 1, ATTN_STATIC, 0);
	gi.centerprintf(ent, "*** DeCamper 1.0 **\nFor DeCamper help, type:\ncmd decamp help\n");
}


void DeCamperSetPassword (edict_t *ent, char *pass)
{
	if (DeCamperPassword) {
		CPRINT0(ent, "DeCamper: Password has already been set.\n");
	}
	else {
		DeCamperPassword = strdup(pass);
		BPRINT1("DeCamper: password has been set to '%s'\n", DeCamperPassword);
	}
}

void DeCamperOptions (edict_t *ent)
{
	char options[512];

	sprintf(options, "FragReduce: %d; DamagePerc: %d; DecampTime: %d;\nMinNumYes: %d; MinPercYes: %d;\n\
PickupPowerup: %d; ReplacePower: %d; RepWeap: %d\n\
RepArmor: %d; RepAmmo: %d\n\
Immune: %d; VoteDisabled: %d; VoteSeconds: %d\n",
			CamperVotingRegistry.frag_reduce,
			CamperVotingRegistry.damage_perc,
			CamperVotingRegistry.decamp_time_minutes,
			CamperVotingRegistry.min_num_yes,
			CamperVotingRegistry.min_perc_yes,
			CamperVotingRegistry.pickup_powerup,
			CamperVotingRegistry.replace_powerup,
			CamperVotingRegistry.replace_weapon,
			CamperVotingRegistry.replace_armor,
			CamperVotingRegistry.replace_ammo,
			CamperVotingRegistry.immune_time,
			CamperVotingRegistry.vote_disabled_minutes,
			CamperVotingRegistry.vote_seconds);
	CPRINT0(ent, options);
}

void DeCamperAdminCommand (edict_t *ent, char *name)
{
	char *arg1, *arg2, *arg3, *arg4;

	arg1 = arg2 = arg3 = arg4;
	if (gi.argc() > 0)
		arg1 = gi.argv(1);
	if (gi.argc() > 1)
		arg2 = gi.argv(2);
	if (gi.argc() > 2)
		arg3 = gi.argv(3);
	if (gi.argc() > 3)
		arg4 = gi.argv(4);

	if (! arg1 || !arg2) {
		CPRINT0(ent, "DeCamper error: Usage: cmd decamp admin PASS parameter value\n");
		return;
	}

	if (! DeCamperPassword) {
		CPRINT1(ent, "DeCamper error: == Wrong password '%s'\n", arg2);
		return;
	}
	// No clues about whether the password has been set or not.
	// Security through obscurity?  You bet.  Don't try this at
	// home, kids, it's stupid.
	if (strcmp(arg2, DeCamperPassword)) {
		CPRINT1(ent, "DeCamper error: Wrong password '%s'\n", arg2);
//		CPRINT1(ent, "DeCamper: official pass is '%s'\n", DeCamperPassword);
//		CPRINT1(ent, "DeCamper: strcmp says %d\n", strcmp(arg2, DeCamperPassword));
		return;
	}
	if (! arg3) {
		CPRINT0(ent, "DeCamper error: Usage: cmd decamp admin PASS parameter value\n");
		return;
	}
	if (! arg4) {
		CPRINT0(ent, "DeCamper error: Usage: cmd decamp admin PASS parameter value\n");
		return;
	}
	// ***** Stick in SetDeCamperOption() here.
	return;
}

void InitializeDeCamper ()
{
	gi.dprintf("** %s **\n", DECAMPER_VERSION);
	DPRINT0("DeCamper: start initialization\n");
	InitializeCamperVotingRegistry();
	SetDefaultDeCamperOptions();
	ParseDeCamperInitFile();
	DPRINT0("DeCamper: finished initialization\n");
}



void Cmd_DeCamp (edict_t *ent)
{
	char *name;
	edict_t *nominee;

	name = gi.args();

	//BPRINT2("%s issued a DeCamp command: |%s|!\n", ent->client->pers.netname, name);

	if (Q_stricmp(name, "yes") == 0)
		VoteDeCamp(ent, DC_VOTE_YES);
	else if (Q_stricmp(name, "no") == 0)
		VoteDeCamp(ent, DC_VOTE_NO);
	else if (Q_stricmp(name, "help") == 0)
		DeCampHelp(ent);
	else if (Q_stricmp(name, "options") == 0)
		DeCamperOptions(ent);
	else if (StrBeginsWith("nominate", name)) {
		if (name[9]) {
			//BPRINT1("Command nomination: |%s|\n", name + 9);
			nominee = FindClientByNetname(name+9);
			if (! nominee) {
				CPRINT1(ent, "DeCamper: Could not find player to nominate: '%s'\n",
						name + 9);
				return;
			}
		}
		else {
			CPRINT0(ent, "DeCamp: no name provided.  Say 'decamp nominate NAME'\n");
			return;
		}
		NominateDeCamp(ent, nominee);
	}
	else if (StrBeginsWith("nom_match", name)) {
		if (strlen(name) > 10) {
			//BPRINT1("Command nomination: |%s|\n", name + 9);
			nominee = FindClientByMatchingNetname(name+10);
			if (! nominee) {
				CPRINT1(ent, "DeCamper: 0 or more than 1 players match '%s'\n",
						name + 10);
				return;
			}
		}
		else {
			CPRINT0(ent, "DeCamp: no name provided.  Say 'decamp nom_match NAME'\n");
			return;
		}
		NominateDeCamp(ent, nominee);
	}
	// ********* Hold off on admin commands for now **********

	//else if (StrBeginsWith("admin_pass", name)) {
	//	if (strlen(name) > 11)
	//		DeCamperSetPassword(ent, name + 11);
	//	else
	//		CPRINT0(ent, "DeCamper: Usage: cmd decamp admin_pass PASSWORD.  PASSWORD cannot have spaces!\n");
	//}
	//else if (StrBeginsWith("admin", name)) {
	//	if (strlen(name) > 6)
	//		DeCamperAdminCommand(ent, name + 5);
	//	else CPRINT0(ent, "DeCamper: Usage: cmd decamp admin COMMAND\n");
	//}
	else
		CPRINT1(ent, "Error: Could not understand command 'decamp %s'\nSay 'cmd decamp help' for help\n", name);
}

