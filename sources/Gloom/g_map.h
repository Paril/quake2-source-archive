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

unsigned int AddToMaplist(char * mapname, unsigned int min_players, unsigned int max_players, char * command);
void UpdateCurrentMapFinishCount(int offset);
void UpdateCurrentMapPlayedCount(int offset);
qboolean RemoveFromMaplist(unsigned int index);
void ClearMaplist(void);
char * MapnameByIndex(unsigned int index);
unsigned int IndexByMapname(char * mapname, unsigned int startindex);
unsigned int NextByIndex (unsigned int startindex, unsigned int players);
unsigned int NextByMagicAlgorithm (unsigned int players);
char * CommandByIndex(unsigned int index);
void ShowMapList(void);
unsigned int ReadMapCycle (char *cyclename);
void UpdateMapCycles (void);
