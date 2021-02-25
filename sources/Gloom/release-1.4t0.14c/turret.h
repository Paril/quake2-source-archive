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

void SP_turret (edict_t *self);
void AnglesNormalize(vec3_t vec);

// Model frames, only the gun portion of turret has multiple frames that we need to animate, all other models (base, laser) just use frame 0


//r1: only one frame now, skins change instead of anim frame
#define GUN

#if defined GUN
#define FRAME_build01           0
#define FRAME_build02           1
#define FRAME_build03           2
#define FRAME_build04           3
#define FRAME_build05           4
#define FRAME_idle                              4
#define FRAME_shot01            5
#define FRAME_shot02            6
#define FRAME_shot03            7
#define FRAME_shot04            8
#define FRAME_shot05            9
#define FRAME_shot06            10
#define FRAME_shot07            11
#define FRAME_shot08            12
#define FRAME_maxshot                   FRAME_shot08
#elif defined LGUN
#define FRAME_build01           0
#define FRAME_build02           0
#define FRAME_build03           0
#define FRAME_build04           0
#define FRAME_build05           0
#define FRAME_idle                      0
#define FRAME_shot01            1
#define FRAME_shot02            2
#define FRAME_shot03            3
#define FRAME_shot04            4
#define FRAME_shot05            5
#define FRAME_maxshot                   FRAME_shot05
#endif


#define FRAME_mg_idle                   0
#define FRAME_mg_shot01                 1
/*#define FRAME_mg_shot02               3
#define FRAME_mg_shot03                 2
#define FRAME_mg_shot04                 3
#define FRAME_mg_shot05                 2
#define FRAME_mg_shot06                 3
#define FRAME_mg_shot07                 2
#define FRAME_mg_shot08                 3*/

/*#define FRAME_explode01               13
#define FRAME_explode02         14
#define FRAME_explode03         15
#define FRAME_explode04         16*/

#define MODEL_SCALE             1.000000
