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

#include "g_local.h"
#include "m_hatchling.h"
        
void HatchlingDie(edict_t *ent)
{
        if(bandwidth_mode->value > 1)
        {
                ThrowGib (ent, ent->gib1, 50, GIB_GREEN);
                ThrowGib (ent, ent->gib2, 50, GIB_GREEN);
        }
}

void kamikaze_explode(edict_t *self, int mod, int dflags);
void KamikazeDie(edict_t *ent)
{
        if (random() < 0.5)
        {
                ent->dmg = 125;
                ent->dmg_radius = 266;
                T_RadiusDamage(ent, ent, ent->dmg, ent, ent->dmg_radius, MOD_R_SPLASH, 0);
        }
}
