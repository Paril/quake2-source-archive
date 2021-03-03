/*
Copyright (c) 2014 Nathan "jitspoe" Wulf, Digital Paint

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "bot_main.h"

/*
============
FS_CreatePath

Creates any directories needed to store the given filename
============
*/
int _mkdir();
int mkdir();

void FS_CreatePath (char *path)
{
	char	*ofs;
	
	for (ofs = path + 1; *ofs; ++ofs)
	{
		if (*ofs == '/' || *ofs == '\\')
		{	// create the directory
			*ofs = 0;
#ifdef WIN32
			_mkdir(path);
#else
			mkdir(path, 0777);
#endif
			*ofs = '/';
		}
	}
}
