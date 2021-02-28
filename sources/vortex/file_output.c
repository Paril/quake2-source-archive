#include "g_local.h"

qboolean SavePlayer(edict_t *ent);

int OpenConfigFile(edict_t *ent)
{
	if (debuginfo->value)
		gi.dprintf("%s just called OpenConfigFile()\n", ent->client->pers.netname);

	//Load file
	if (openPlayer(ent) == false)
		newPlayer(ent);

	return canJoinGame(ent);
}

#ifndef NO_GDS
void SaveCharacterQuit (edict_t *ent)
{
	int					health, i;
	int					temp[MAX_ITEMS];
	char				userinfo[MAX_INFO_STRING];
	char				oldPassword[24], newPassword[24];
	gitem_t				*item=itemlist;
	client_respawn_t	resp;

	// don't save if they aren't in the game (spectator?), unless they are a boss
	if (!ent || !ent->inuse || !ent->client || G_IsSpectator(ent))
		return;

	if (ent->ai.is_bot) // don't save bots lol
		return;

	if (debuginfo->value)
		gi.dprintf("%s just called SaveMyCharacter()\n", ent->client->pers.netname);

	if (ent->health < 1)
	{
		// use respawn data
		resp = ent->client->resp;
		memcpy (userinfo, ent->client->pers.userinfo, sizeof(userinfo));
		InitClientPersistant(ent->client);
		ClientUserinfoChanged (ent, userinfo);
		ent->client->resp = resp;
		modify_health(ent);
		modify_max(ent);
		Give_respawnweapon(ent, ent->myskills.respawn_weapon);
		Give_respawnitems(ent);
	}

	// copy inventory to temp space
	for (i=0; i<game.num_items; i++, item++)
		temp[ITEM_INDEX(item)] = ent->client->pers.inventory[ITEM_INDEX(item)];

	health = ent->health;
	if (health > ent->max_health)
		health = ent->max_health;

	if (temp[body_armor_index] > MAX_ARMOR(ent))
		temp[body_armor_index] = MAX_ARMOR(ent);

	if (temp[power_cube_index] > MAX_POWERCUBES(ent))
		temp[power_cube_index] = MAX_POWERCUBES(ent);

	temp[flag_index] = 0;

	// copy data to myskills
	ent->myskills.current_health = health;
	for (i=0; i<game.num_items; i++, item++)
		ent->myskills.inventory[ITEM_INDEX(item)] = temp[ITEM_INDEX(item)];

	//Check for a password change
	Q_strncpy (newPassword, Info_ValueForKey(ent->client->pers.userinfo, "vrx_password"), sizeof(newPassword)-1);
	Q_strncpy (oldPassword, CryptString(ent->myskills.password, true), sizeof(oldPassword)-1);

	if(Q_stricmp(oldPassword, newPassword) != 0)
	{
		WriteToLogfile(ent, va("Changed password from '%s' to '%s'.\n", oldPassword, newPassword));
	}

	//Encrypt the password.
	Q_strncpy (ent->myskills.password, CryptString(newPassword, false), sizeof(ent->myskills.password)-1);

	V_GDS_Queue_Add(ent, GDS_SAVECLOSE);

}
#endif

void SaveCharacter (edict_t *ent)
{
	int					health, i;
	int					temp[MAX_ITEMS];
	char				userinfo[MAX_INFO_STRING];
	//char				password[24];
	//char				savedpass[24];
	char				oldPassword[24], newPassword[24];
	gitem_t				*item=itemlist;
	client_respawn_t	resp;

	// don't save if they aren't in the game (spectator?), unless they are a boss
	if (!ent || !ent->inuse || !ent->client || G_IsSpectator(ent))
		return;

	if (ent->ai.is_bot) // don't save bots lol
		return;

	if (debuginfo->value)
		gi.dprintf("%s just called SaveMyCharacter()\n", ent->client->pers.netname);

	if (ent->health < 1)
	{
		// use respawn data
		resp = ent->client->resp;
		memcpy (userinfo, ent->client->pers.userinfo, sizeof(userinfo));
		InitClientPersistant(ent->client);
		ClientUserinfoChanged (ent, userinfo);
		ent->client->resp = resp;
		modify_health(ent);
		modify_max(ent);
		Give_respawnweapon(ent, ent->myskills.respawn_weapon);
		Give_respawnitems(ent);
	}

	// copy inventory to temp space
	for (i=0; i<game.num_items; i++, item++)
		temp[ITEM_INDEX(item)] = ent->client->pers.inventory[ITEM_INDEX(item)];

	health = ent->health;
	if (health > ent->max_health)
		health = ent->max_health;

	if (temp[body_armor_index] > MAX_ARMOR(ent))
		temp[body_armor_index] = MAX_ARMOR(ent);

	if (temp[power_cube_index] > MAX_POWERCUBES(ent))
		temp[power_cube_index] = MAX_POWERCUBES(ent);

	temp[flag_index] = 0;

	// copy data to myskills
	ent->myskills.current_health = health;
	for (i=0; i<game.num_items; i++, item++)
		ent->myskills.inventory[ITEM_INDEX(item)] = temp[ITEM_INDEX(item)];

	//Check for a password change
	Q_strncpy (newPassword, Info_ValueForKey(ent->client->pers.userinfo, "vrx_password"), sizeof(newPassword)-1);
	Q_strncpy (oldPassword, CryptString(ent->myskills.password, true), sizeof(oldPassword)-1);

	if(Q_stricmp(oldPassword, newPassword) != 0)
	{
		WriteToLogfile(ent, va("Changed password from '%s' to '%s'.\n", oldPassword, newPassword));
	}

	//Encrypt the password.
	Q_strncpy (ent->myskills.password, CryptString(newPassword, false), sizeof(ent->myskills.password)-1);

	//3.0 save file
	if (!SavePlayer(ent))
		gi.dprintf("ERROR: SaveCharacter() unable to save player: %s", ent->myskills.player_name);
}