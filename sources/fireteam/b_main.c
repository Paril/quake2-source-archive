#include "g_local.h"

//=======================================================
// Bot's collision detection function.
//=======================================================
void bPain(edict_t *bot, edict_t *other, float kickback, int damage) {

	if (other == world) return;

	// Stick with current enemy!!
	if (bot->enemy!=NULL) return;

	// Bossman's attacker becomes bot's new enemy .
	if (other != bot->bossman) {
		bot->enemy = other;
		bot->itemwant = NULL; }
}

//=========================================================
// Player pain function. Send your guards after'em!!
//=========================================================
void player_pain(edict_t *self, edict_t *other, float kick, int damage) {

	if (self->bossman && G_ClientInGame(self->client->guard)) {
		if (self->client->guard->enemy == NULL)
			bPain(self->client->guard, other, kick, damage); }
}

//========================================================
void bTouch(edict_t *bot, edict_t *other, cplane_t *plane, csurface_t *surf)
{}

//=======================================================
// Create the bodyguard as a real player-client.
//=======================================================
void bSpawn(edict_t *ent) {
	edict_t *bot;
	char userinfo[MAX_INFO_STRING];
	int clientnum;

	Random_Userinfo(ent, userinfo);

	clientnum = G_GetFreeEdict();

	if (clientnum < 0) {
		gi.dprintf("Server is full.\n");
		return; }

	bot=g_edicts+clientnum+1;

	G_InitEdict(bot);

	bot->client=&game.clients[clientnum];

	InitClientResp(bot->client);

	InitClientPersistant(bot->client);

	ClientUserinfoChanged(bot, userinfo);

	bot->bossman=ent; // Owner

	ent->client->guard=bot; // Guard

	PutClientInServer(bot);

	bot->client->resp.class = (rndnum(1,9));
	if(ctf->value)
		bot->client->resp.ctf_team = ent->client->resp.ctf_team;
	else if (team_dm->value)
		bot->client->resp.team = ent->client->resp.team;

	G_MuzzleFlash((short)(bot-g_edicts), bot->s.origin, MZ_LOGIN);

	gi.bprintf(PRINT_HIGH, "%s entered the game!\n", bot->client->pers.netname);

	ClientEndServerFrame(bot);
}

//=======================================================
void Cmd_Guard_f(edict_t *ent) {

	// Not available to dead or respawning players!
	if (!G_ClientInGame(ent)) return;

	// Are Bodyguards enabled?
	if (sv_bodyguards->value==0) return;

	if (!ent->client->guard)
		bSpawn(ent);
}
