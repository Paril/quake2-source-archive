/*
    Quake II Glooom, a total conversion mod for Quake II
    Copyright (C) 1999-2007  Gloom Developers

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
void log_death (edict_t *killer, edict_t *killed, int mod);
void log_connection (edict_t *ent);
void log_disconnection (edict_t *ent);
void log_namechange (char *oldname, char *newname);
void log_changeclass (edict_t *ent, int newclass);
void log_killstructure (edict_t *attacker, edict_t *ent);
void log_makestructure (edict_t *creator, char *object, int snumber);
void log_iplog (char *name, char *ip);
void SaveIPLog (char * filename);
void ReadIPLogs (char * filename);
void OpenGamelog(void);
void CloseGamelog(void);
