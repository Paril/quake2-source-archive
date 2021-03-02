#include "g_local.h"
#include "m_player.h"

//========================================================
//============= BOT TALKING/TAUNTING ROUTINES ============
//========================================================
//=====================================================
// Returns Player with Highest Score.
//=====================================================
edict_t *BestScoreEnt(void) {
	edict_t *bestplayer=NULL;
	int i, bestscore=-999;
	edict_t *ent;

	// Search thru all clients
	for(i=0;i < game.maxclients; i++) {
		ent=g_edicts+i+1;
		if (!G_EntExists(ent)) continue;
		if (ent->client->resp.score > bestscore) {
		bestplayer=ent; // Found one!
		bestscore=ent->client->resp.score; } }

	return bestplayer;
}

//=======================================================
// Taunt your victim! Called from ClientObituary()..
//=======================================================
void bTaunt(edict_t *bot, edict_t *other) {

	if ((rand()%5) >= 2) return;

	if (level.time < bot->last_taunt) return;

	// If killed enemy then Taunt them!!
	if ((other->client) && (random() < 0.4))
		switch (rand()%4) {
			case 0: // flipoff
				bot->s.frame = FRAME_flip01-1;
				bot->client->anim_end = FRAME_flip12;
				break;
			case 1: // salute
				bot->s.frame = FRAME_salute01-1;
				bot->client->anim_end = FRAME_salute11;
				break;
			case 2: // taunt
				bot->s.frame = FRAME_taunt01-1;
				bot->client->anim_end = FRAME_taunt17;
				break;
			case 3: // point
				bot->s.frame = FRAME_point01-1;
				bot->client->anim_end = FRAME_point12;
				break; }

	// Taunt victim but not too often..
	bot->last_taunt = level.time + 60 + 35;
}

//========================================================
void bFakeChat(edict_t *bot) {
	gclient_t *bclient=bot->client;

	if (random() < .1)
		gi_bprintf(PRINT_CHAT, "%s: Looking for targets on intercept\n", bclient->pers.netname);
	else if (random() < .2)
		gi_bprintf(PRINT_CHAT, "%s: On assault.\n", bclient->pers.netname);
	else if (random() < .3)
		gi_bprintf(PRINT_CHAT, "%s: Assault: Move up!\n", bclient->pers.netname);
	else if (random() < .4)
		gi_bprintf(PRINT_CHAT, "%s: Defense: Keep your positions!\n", bclient->pers.netname);
	else if (random() < .5)
		gi_bprintf(PRINT_CHAT, "%s: On point.\n", bclient->pers.netname);
	else
		gi_bprintf(PRINT_CHAT, "%s: Scanning for targets...\n", bclient->pers.netname);

	// Random chats between 2 minutes and 10 minutes
	bot->last_chat = level.time + 120 + (60*(rand()%8));
}

//========================================================
// Done ONCE! Bot introduces itself to Bossman..
//========================================================
void bHello(edict_t *bot)
{
	gclient_t *bclient=bot->client;

	if (random() < .1)
		gi_centerprintf(bot->bossman,"%s: Reporting for duty, Sir!\n", bclient->pers.netname);
	else if (random() < .2)
		gi_centerprintf(bot->bossman,"%s: On guard, Sir!\n", bclient->pers.netname);
	else if (random() < .3)
		gi_centerprintf(bot->bossman,"%s: Standing at the ready, Sir!\n", bclient->pers.netname);
	else if (random() < .4)
		gi_centerprintf(bot->bossman,"%s: Where do we go, Sir?\n", bclient->pers.netname);
	else if (random() < .5)
		gi_centerprintf(bot->bossman,"%s: Let's move, Sir!", bclient->pers.netname);
	else
		gi_centerprintf(bot->bossman,"%s: At your service, Sir!!\n", bclient->pers.netname);

	bot->think=bThink;
	bot->nextthink=level.time+0.1;
}

//========================================================
// Insult the player that the bot just fragged...
//========================================================
void bInsult(edict_t *bot, edict_t *loser) {
	gclient_t *bclient=bot->client;
	gclient_t *lclient=loser->client;

	if (bot==BestScoreEnt()) {
		gi_bprintf(PRINT_CHAT, "%s: Reporting as best commando!\n", bclient->pers.netname);
		return; }

	if ((rand()%5) > 3) return;

	if (level.time < bot->last_insult) return;

	if (bclient->resp.score < lclient->resp.score) {
		if (bclient->resp.score < lclient->resp.score - 20) {
			if (random() < .1)
				gi_bprintf(PRINT_CHAT, "%s: Target Eliminated: %s\n", bclient->pers.netname, lclient->pers.netname);
			else if (random() < .2)
				gi_bprintf(PRINT_CHAT, "%s: Caught up with %s!\n", bclient->pers.netname, lclient->pers.netname);
			else if (random() < .3)
				gi_bprintf(PRINT_CHAT, "%s: Worked on %s for a while!\n", bclient->pers.netname, lclient->pers.netname);
			else if (random() < .4)
				gi_bprintf(PRINT_CHAT, "%s: Losing, but still got you, %s.\n", bclient->pers.netname, lclient->pers.netname);
			else if (random() < .5)
				gi_bprintf(PRINT_CHAT, "%s: Losing badly and STILL got that kill.\n", bclient->pers.netname);
			else
				gi_bprintf(PRINT_CHAT, "%s: Assignment Complete. %s neutralized.\n", bclient->pers.netname, lclient->pers.netname); }
		else if (bclient->resp.score < lclient->resp.score - 10) {
			if (random() < .1)
				gi_bprintf(PRINT_CHAT, "%s: Down a little on score, but not outta the game, %s.\n", bclient->pers.netname, lclient->pers.netname);
			else if (random() < .2)
				gi_bprintf(PRINT_CHAT, "%s: Working on score, and doing an OK job, eh %s?\n", bclient->pers.netname, lclient->pers.netname);
			else if (random() < .3)
				gi_bprintf(PRINT_CHAT, "%s: %s: gone but not forgotten.\n", bclient->pers.netname, lclient->pers.netname);
			else if (random() < .4)
				gi_bprintf(PRINT_CHAT, "%s: Sorry there %s.\n", bclient->pers.netname, lclient->pers.netname);
			else if (random() < .5)
				gi_bprintf(PRINT_CHAT, "%s: What an unfortunate tragedy, %s.\n", bclient->pers.netname, lclient->pers.netname);
			else
				gi_bprintf(PRINT_CHAT, "%s: Wow. I amaze even myself sometimes.\n", bclient->pers.netname); }
		else if (bclient->resp.score < lclient->resp.score - 5) {
			if (random() < .1)
				gi_bprintf(PRINT_CHAT, "%s: I will catch up eventually you know, %s.\n", bclient->pers.netname, lclient->pers.netname);
			else if (random() < .2)
				gi_bprintf(PRINT_CHAT, "%s: Not quite %s.\n", bclient->pers.netname, lclient->pers.netname);
			else if (random() < .3)
				gi_bprintf(PRINT_CHAT, "%s: \n", bclient->pers.netname, lclient->pers.netname);
			else if (random() < .4)
				gi_bprintf(PRINT_CHAT, "%s: You might wanna get that fixed, %s\n", bclient->pers.netname, lclient->pers.netname);
			else if (random() < .5)
				gi_bprintf(PRINT_CHAT, "%s: Close, but no.\n", bclient->pers.netname);
			else
				gi_bprintf(PRINT_CHAT, "%s: The game of war appears to be turning in my favor %s.\n", bclient->pers.netname, lclient->pers.netname); }
		else {
			if (random() < .1)
				gi_bprintf(PRINT_CHAT, "%s: I can still catch up with you, %s, don't get cocky\n", bclient->pers.netname, lclient->pers.netname);
			else if (random() < .2)
				gi_bprintf(PRINT_CHAT, "%s: You're alllll mine, %s.\n", bclient->pers.netname, lclient->pers.netname);
			else if (random() < .3)
				gi_bprintf(PRINT_CHAT, "%s: Come on, %s, just you and me.\n", bclient->pers.netname, lclient->pers.netname);
			else if (random() < .4)
				gi_bprintf(PRINT_CHAT, "%s: That's not gonna cut it %s!\n", bclient->pers.netname, lclient->pers.netname);
			else if (random() < .5)
				gi_bprintf(PRINT_CHAT, "%s: Oh my, %s, that didn't look like it felt very nice.\n", bclient->pers.netname, lclient->pers.netname);
			else
				gi_bprintf(PRINT_CHAT, "%s: Well, %s, looks like things might even up.\n", bclient->pers.netname, lclient->pers.netname); }
		}
	else if (bclient->resp.score > lclient->resp.score) {
		if (bclient->resp.score > lclient->resp.score + 10) {
			if (random() < .1)
				gi_bprintf(PRINT_CHAT, "%s: You're never going to catch up to me, %s. Just give up.\n", bclient->pers.netname, lclient->pers.netname);
			else if (random() < .2)
				gi_bprintf(PRINT_CHAT, "%s: Good attempt %s.\n", bclient->pers.netname, lclient->pers.netname);
			else if (random() < .3)
				gi_bprintf(PRINT_CHAT, "%s: No.\n", bclient->pers.netname);
			else if (random() < .4)
				gi_bprintf(PRINT_CHAT, "%s: Me? Using a bot? No way %s, I'm all skill!\n", bclient->pers.netname, lclient->pers.netname);
			else if (random() < .5)
				gi_bprintf(PRINT_CHAT, "%s: Don't go there %s.\n", bclient->pers.netname, lclient->pers.netname);
			else
				gi_bprintf(PRINT_CHAT, "%s: Running might be a consideration.\n", bclient->pers.netname, lclient->pers.netname); }
		else if (bclient->resp.score > lclient->resp.score + 5) {
			if (random() < .1)
				gi_bprintf(PRINT_CHAT, "%s: Ahh... Yep. I'm good.\n", bclient->pers.netname);
			else if (random() < .2)
				gi_bprintf(PRINT_CHAT, "%s: Don't feel bad %s, you just aren't gifted like me\n", bclient->pers.netname, lclient->pers.netname);
			else if (random() < .3)
				gi_bprintf(PRINT_CHAT, "%s: Come on %s, don't give up now!\n", bclient->pers.netname, lclient->pers.netname);
			else if (random() < .4)
				gi_bprintf(PRINT_CHAT, "%s: Perhaps a larger weapon, %s?\n", bclient->pers.netname, lclient->pers.netname);
			else if (random() < .5)
				gi_bprintf(PRINT_CHAT, "%s: Some new tactics are always a good idea, %s\n", bclient->pers.netname, lclient->pers.netname);
			else
				gi_bprintf(PRINT_CHAT, "%s: Are you trying to get infront of my guns, %s?\n", bclient->pers.netname, lclient->pers.netname); }
		else {
			if (random() < .1)
				gi_bprintf(PRINT_CHAT, "%s: Come on, %s, I can take you\n", bclient->pers.netname, lclient->pers.netname);
			else if (random() < .2)
				gi_bprintf(PRINT_CHAT, "%s: You're goin' down, %s.\n", bclient->pers.netname, lclient->pers.netname);
			else if (random() < .3)
				gi_bprintf(PRINT_CHAT, "%s: Oh, so %s, you want some?\n", bclient->pers.netname, lclient->pers.netname);
			else if (random() < .4)
				gi_bprintf(PRINT_CHAT, "%s: That's right %s, please accept this gunfire\n", bclient->pers.netname, lclient->pers.netname);
			else if (random() < .5)
				gi_bprintf(PRINT_CHAT, "%s: Better get that taken care of, %s. It could get infected.\n", bclient->pers.netname, lclient->pers.netname);
			else
				gi_bprintf(PRINT_CHAT, "%s: Just resign %s. I'm gonna do it again.\n", bclient->pers.netname, lclient->pers.netname); }
		}
	else {
		if (random() < .1)
			gi_bprintf(PRINT_CHAT, "%s: Oh look, a tie! Well %s, we'll just have to fix that!\n", bclient->pers.netname, lclient->pers.netname);
		else if (random() < .2)
			gi_bprintf(PRINT_CHAT, "%s: Time to die, %s\n", bclient->pers.netname, lclient->pers.netname);
		else if (random() < .3)
			gi_bprintf(PRINT_CHAT, "%s: Come on %s, it's time to ride daddy's rocket\n", bclient->pers.netname, lclient->pers.netname);
		else if (random() < .4)
			gi_bprintf(PRINT_CHAT, "%s: Look %s, we're tied... want me to fix that? Ok then!\n", bclient->pers.netname, lclient->pers.netname);
		else if (random() < .5)
			gi_bprintf(PRINT_CHAT, "%s: Damn %s, I thought you were better than this\n", bclient->pers.netname, lclient->pers.netname);
		else
			gi_bprintf(PRINT_CHAT, "%s: Alright, %s, I'm not showing any mercy this time.\n", bclient->pers.netname, lclient->pers.netname);}

	// Next insult between 30 sec and 5 minutes
	bot->last_insult = level.time + 30 + (60*(rand()%5));
}

