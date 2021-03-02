#include "g_local.h"

int pwri_callorders[PWORDER_MAXSLOTS];

// Main functions. Printing, clearing, etc.
pworder_t *pworder;

// Do in order just in caserifiifif/
required_te_t required_list[] = 
{
	{
		TE_GUNSHOT,
		{
			1,
			0,
			1,
			0,
		}
	},
};

/*
typedef enum
{
	TE_GUNSHOT,
	TE_BLOOD,
	TE_BLASTER,
	TE_RAILTRAIL,
	TE_SHOTGUN,
	TE_EXPLOSION1,
	TE_EXPLOSION2,
	TE_ROCKET_EXPLOSION,
	TE_GRENADE_EXPLOSION,
	TE_SPARKS,
	TE_SPLASH,
	TE_BUBBLETRAIL,
	TE_SCREEN_SPARKS,
	TE_SHIELD_SPARKS,
	TE_BULLET_SPARKS,
	TE_LASER_SPARKS,
	TE_PARASITE_ATTACK,
	TE_ROCKET_EXPLOSION_WATER,
	TE_GRENADE_EXPLOSION_WATER,
	TE_MEDIC_CABLE_ATTACK,
	TE_BFG_EXPLOSION,
	TE_BFG_BIGEXPLOSION,
	TE_BOSSTPORT,			// used as '22' in a map, so DON'T RENUMBER!!!
	TE_BFG_LASER,
	TE_GRAPPLE_CABLE,
	TE_WELDING_SPARKS,
	TE_GREENBLOOD,
	TE_BLUEHYPERBLASTER,
	TE_PLASMA_EXPLOSION,
	TE_TUNNEL_SPARKS,
//ROGUE
	TE_BLASTER2,
	TE_RAILTRAIL2,
	TE_FLAME,
	TE_LIGHTNING,
	TE_DEBUGTRAIL,
	TE_PLAIN_EXPLOSION,
	TE_FLASHLIGHT,
	TE_FORCEWALL,
	TE_HEATBEAM,
	TE_MONSTER_HEATBEAM,
	TE_STEAM,
	TE_BUBBLETRAIL2,
	TE_MOREBLOOD,
	TE_HEATBEAM_SPARKS,
	TE_HEATBEAM_STEAM,
	TE_CHAINFIST_SMOKE,
	TE_ELECTRIC_SPARKS,
	TE_TRACKER_EXPLOSION,
	TE_TELEPORT_EFFECT,
	TE_DBALL_GOAL,
	TE_WIDOWBEAMOUT,
	TE_NUKEBLAST,
	TE_WIDOWSPLASH,
	TE_EXPLOSION1_BIG,
	TE_EXPLOSION1_NP,
	TE_FLECHETTE
//ROGUE
} temp_event_t;
*/

void ParilWrite_Initialize ()
{
	pworder_t *temp = (pworder_t *)gi.TagMalloc (sizeof(pworder_t), TAG_GAME);
	memset (temp, 0, sizeof(temp));
	pworder = temp;
	Clear_Structure();
}

cvar_t	*display_writes;

void ParilWrite_DebugPrint (char *fmt, ...)
{
	char	bigbuffer[0x10000];
	va_list		argptr;
	int len;

	if (display_writes->value != 1)
		return;

	va_start (argptr,fmt);
	len = vsprintf (bigbuffer,fmt,argptr);
	va_end (argptr);

	gi.dprintf (bigbuffer);
}

// I don't trust memset.
void Clear_Structure ()
{
	int i = 0;
	ParilWrite_DebugPrint ("Wiping pworder structure...\n");
	pworder->angle1 = -1;
	pworder->angle2 = -1;
	pworder->byte1 = -1;
	pworder->byte2 = -1;
	pworder->byte3 = -1;
	pworder->byte4 = -1;
	pworder->char1 = -1;
	pworder->char2 = -1;
	VectorSet (pworder->dir1, -1, -1, -1);
	VectorSet (pworder->dir2, -1, -1, -1);
	pworder->float1 = -1;
	pworder->float2 = -1;
	pworder->long1 = -1;
	pworder->long2 = -1;
	VectorSet (pworder->pos1, -1, -1, -1);
	VectorSet (pworder->pos2, -1, -1, -1);
//	pworder->short1 = -1;
//	pworder->short2 = -1;
	for (; i < MAX_ITEMS; i++)
		pworder->shorts[i] = -1;
	i = 0;
	pworder->string1 = NULL;
	pworder->string1 = NULL;
	ParilWrite_DebugPrint ("Deleting all call orders...\n");
	for (; i < PWORDER_MAXSLOTS; i++)
		pwri_callorders[i] = -1;
	ParilWrite_DebugPrint ("Done.\n");
}

// Writing itself is done down here.

void ParilWrite_AddOrder (int order)
{
	int i = 0;

	for (; i < PWORDER_MAXSLOTS; i++)
	{
		if (pwri_callorders[i] == -1)
		{
			ParilWrite_DebugPrint ("Adding call order of type %i to array number %i of call orders.\n", order, pwri_callorders[i]);
			pwri_callorders[i] = order;
			break;
		}
		continue;
	}
}

void ParilWrite_AddByte (int data)
{
	if (pworder->byte1 == -1)
		pworder->byte1 = data;
	else if (pworder->byte2 == -1)
		pworder->byte2 = data;
	else if (pworder->byte3 == -1)
		pworder->byte3 = data;
	else if (pworder->byte4 == -1)
		pworder->byte4 = data;
	else
		ParilWrite_DebugPrint ("A byte (data %i) was unable to be written.\n", data);	
}

void ParilWrite_AddChar (int data)
{
	if (pworder->char1 == -1)
		pworder->char1 = data;
	else if (pworder->char2 == -1)
		pworder->char2 = data;
	else
		ParilWrite_DebugPrint ("A char (data %i) was unable to be written.\n", data);	
}

void ParilWrite_AddShort (int data)
{
	int e = 0;

	for (; e < MAX_ITEMS; e++)
	{
		if (pworder->shorts[e] == -1)
		{
			pworder->shorts[e] = data;
			break;
		}
		if (e == 256)
			ParilWrite_DebugPrint ("A short (data %i) was unable to be written.\n", data);	
	}
/*	if (pworder->short1 == -1)
		pworder->short1 = data;
	else if (pworder->short2 == -1)
		pworder->short2 = data;
	else*/
}

void ParilWrite_AddString (char *data)
{
	if (pworder->string1 == NULL)
		pworder->string1 = G_CopyString(data);
	else if (pworder->string2 == NULL)
		pworder->string2 = G_CopyString(data);
	else
		ParilWrite_DebugPrint ("A string (data %s) was unable to be written.\n", data);	
}

void ParilWrite_AddFloat (float data)
{
	if (pworder->float1 == -1)
		pworder->float1 = data;
	else if (pworder->float2 == -1)
		pworder->float2 = data;
	else
		ParilWrite_DebugPrint ("A float (data %f) was unable to be written.\n", data);	
}

void ParilWrite_AddAngle (float data)
{
	if (pworder->angle1 == -1)
		pworder->angle1 = data;
	else if (pworder->angle2 == -1)
		pworder->angle2 = data;
	else
		ParilWrite_DebugPrint ("An angle (data %f) was unable to be written.\n", data);	
}

void ParilWrite_AddDir (vec3_t data)
{
	if (pworder->dir1[0] == -1)
		VectorCopy (data, pworder->dir1);
	else if (pworder->dir2[0] == -1)
		VectorCopy (data, pworder->dir2);
	else
		ParilWrite_DebugPrint ("A direction (data %s) was unable to be written.\n", vtos(data));	
}

void ParilWrite_AddPosition (vec3_t data)
{
	if (pworder->pos1[0] == -1)
		VectorCopy (data, pworder->pos1);
	else if (pworder->pos2[0] == -1)
		VectorCopy (data, pworder->pos2);
	else
		ParilWrite_DebugPrint ("A position (data %s) was unable to be written.\n", vtos(data));	
}

void ParilWrite_AddType (int type, int data, char *data2, float data3, float vectordata0, float vectordata1, float vectordata2)
{
	// Is a char, byte or short
	if (data != -1)
	{
		if (type == PWORDER_BYTE)
			ParilWrite_AddByte (data);
		else if (type == PWORDER_CHAR)
			ParilWrite_AddChar (data);
		else if (type == PWORDER_SHORT)
			ParilWrite_AddShort (data);
	}
	// It's a string
	else if (data2 != NULL)
	{
		if (type == PWORDER_STRING)
			ParilWrite_AddString (data2);
	}
	// Float away!
	else if (data3 != -1)
	{
		if (type == PWORDER_FLOAT)
			ParilWrite_AddFloat (data3);
		else if (type == PWORDER_ANGLE)
			ParilWrite_AddFloat (data3);
	}
	else if (vectordata0 != -1)
	{
		vec3_t vector = {vectordata0, vectordata1, vectordata2}; // Get the vector back into a vector!
	
		if (type == PWORDER_DIR)
			ParilWrite_AddDir (vector);
		else if (type == PWORDER_POSITION)
			ParilWrite_AddPosition (vector);
	}
	else
		ParilWrite_DebugPrint ("Something wasn't able to get written, type %i\n", type);	
}

void WriteChar (int byte)
{
	ParilWrite_DebugPrint ("Writing char %i\n", byte);
	ParilWrite_AddOrder (PWORDER_CHAR);
	ParilWrite_AddType (PWORDER_CHAR, byte, NULL, -1, -1, -1, -1);
}

void WriteByte (int byte)
{
	ParilWrite_DebugPrint ("Writing byte %i\n", byte);
	ParilWrite_AddOrder (PWORDER_BYTE);
	ParilWrite_AddType (PWORDER_BYTE, byte, NULL, -1, -1, -1, -1);
}

void WriteShort (int byte)
{
	ParilWrite_DebugPrint ("Writing short %i\n", byte);
	ParilWrite_AddOrder (PWORDER_SHORT);
	ParilWrite_AddType (PWORDER_SHORT, byte, NULL, -1, -1, -1, -1);
}

void WriteLong (int byte)
{
	ParilWrite_DebugPrint ("Writing long %i\n", byte);
	ParilWrite_AddOrder (PWORDER_LONG);
	ParilWrite_AddType (PWORDER_LONG, byte, NULL, -1, -1, -1, -1);
}

void WriteFloat (float byte)
{
	ParilWrite_DebugPrint ("Writing float %i\n", byte);
	ParilWrite_AddOrder (PWORDER_FLOAT);
	ParilWrite_AddType (PWORDER_FLOAT, -1, NULL, byte, -1, -1, -1);
}

void WriteString (char *string)
{
	ParilWrite_DebugPrint ("Writing string %s\n", string);
	ParilWrite_AddOrder (PWORDER_STRING);
	ParilWrite_AddType (PWORDER_STRING, -1, string, -1, -1, -1, -1);
}

void WritePosition (vec3_t position)
{
	ParilWrite_DebugPrint ("Writing position %s\n", vtos(position));
	ParilWrite_AddOrder (PWORDER_POSITION);
	ParilWrite_AddType (PWORDER_POSITION, -1, NULL, -1, position[0], position[1], position[2]);
}

void WriteDir (vec3_t dir)
{
	if (!dir[0] && !dir[1] && !dir[2])
		return;
	ParilWrite_DebugPrint ("Writing direction %s\n", vtos(dir));
	ParilWrite_AddOrder (PWORDER_DIR);
	ParilWrite_AddType (PWORDER_DIR, -1, NULL, -1, dir[0], dir[1], dir[2]);
}

void WriteAngle (float byte)
{
	ParilWrite_DebugPrint ("Writing angle %i\n", byte);
	ParilWrite_AddOrder (PWORDER_ANGLE);
	ParilWrite_AddType (PWORDER_ANGLE, -1, NULL, byte, -1, -1, -1);
}

void Unicast (edict_t *ent, qboolean reliable)
{
	int i = 0;
	int e = 0;
	
	// Go through the calls in order.
	for (; i < PWORDER_MAXSLOTS; i++)
	{
		if (pwri_callorders[i] == -1)
			break;
		switch (pwri_callorders[i])
		{
			case PWORDER_CHAR:
				if (pworder->char2 != -1 && pworder->char1 == -1)
				{
					gi.old_WriteChar (pworder->char2);
					pworder->char2 = -1;
				}
				else
				{
					gi.old_WriteChar (pworder->char1);
					pworder->char1 = -1;
				}
				break;
			case PWORDER_BYTE:
				if (pworder->byte2 != -1 && pworder->byte1 == -1)
				{
					gi.old_WriteByte (pworder->byte2);
					pworder->byte2 = -1;
				}
				else if (pworder->byte3 != -1 && pworder->byte2 == -1)
				{
					gi.old_WriteByte (pworder->byte3);
					pworder->byte3 = -1;
				}
				else if (pworder->byte4 != -1 && pworder->byte3 == -1)
				{
					gi.old_WriteByte (pworder->byte4);
					pworder->byte4 = -1;
				}
				else
				{
					gi.old_WriteByte (pworder->byte1);
					pworder->byte1 = -1;
				}
				break;
			case PWORDER_SHORT:
				/*if (pworder->short2 != -1 && pworder->short1 == -1)
				{
					gi.old_WriteShort (pworder->short2);
					pworder->short2 = -1;
				}
				else
				{
					gi.old_WriteShort (pworder->short1);
					pworder->short1 = -1;
				}*/
				for (; e < MAX_ITEMS; e++)
				{
					if (pworder->shorts[e] != -1)
					{
						gi.old_WriteShort (pworder->shorts[e]);
						pworder->shorts[e] = -1;
					}
				}
				break;
			case PWORDER_LONG:
				if (pworder->long2 != -1 && pworder->long1 == -1)
				{
					gi.old_WriteLong (pworder->long2);
					pworder->long2 = -1;
				}
				else
				{
					gi.old_WriteLong (pworder->long1);
					pworder->long1 = -1;
				}
				break;
			case PWORDER_FLOAT:
				if (pworder->float2 != -1 && pworder->float1 == -1)
				{
					gi.old_WriteLong (pworder->float2);
					pworder->float2 = -1;
				}
				else
				{
					gi.old_WriteLong (pworder->float1);
					pworder->float1 = -1;
				}
				break;
			case PWORDER_STRING:
				if (pworder->string2 != NULL && pworder->string1 == NULL)
				{
					gi.old_WriteString (pworder->string2);
					pworder->string2 = NULL;
				}
				else
				{
					gi.old_WriteString (pworder->string1);
					pworder->string1 = NULL;
				}
				break;
			case PWORDER_POSITION:
				if (pworder->pos2[0] != -1 && pworder->pos1[0] == -1)
				{
					gi.old_WritePosition (pworder->pos2);
					VectorSet (pworder->pos2, -1, -1, -1);
				}
				else
				{
					gi.old_WritePosition (pworder->pos1);
					VectorSet (pworder->pos1, -1, -1, -1);
				}
				break;
			case PWORDER_DIR:
				if (pworder->dir2[0] != -1 && pworder->dir1[0] == -1)
				{
					gi.old_WriteDir (pworder->dir2);
					VectorSet (pworder->dir2, -1, -1, -1);
				}
				else
				{
					gi.old_WriteDir (pworder->dir1);
					VectorSet (pworder->dir1, -1, -1, -1);
				}
				break;
			case PWORDER_ANGLE:
				if (pworder->angle2 != -1 && pworder->angle1 == -1)
				{
					gi.old_WriteAngle (pworder->angle2);
					pworder->float2 = -1;
				}
				else
				{
					gi.old_WriteAngle (pworder->angle1);
					pworder->angle1 = -1;
				}
				break;
		}
	}

	gi.old_unicast (ent, reliable);
	Clear_Structure();
}

// Returns true if the temporary entity has all of the requirements.
qboolean ParilWrite_HasRequirementsForTE ()
{
	int sofarsogood = 0;
	int sofarsobad = 0;

	if (required_list[pworder->byte2].type == pworder->byte2)
	{
		if (required_list[pworder->byte2].required.needs_pos)
		{
			if (pworder->pos1[0] != -1)
				sofarsogood += 1;
			else
				sofarsobad += 1;
		}
		if (required_list[pworder->byte2].required.needs_pos2)
		{
			if (pworder->pos2[0] != -1)
				sofarsogood += 1;
			else
				sofarsobad += 1;
		}
		if (required_list[pworder->byte2].required.needs_dir)
		{
			if (pworder->dir1[0] != -1)
				sofarsogood += 1;
			else
				sofarsobad += 1;
		}
		// Usually only one short is written for TEs.
		if (required_list[pworder->byte2].required.needs_short)
		{
			if (pworder->shorts[0] != -1)
				sofarsogood += 1;
			else
				sofarsobad += 1;
		}

		if (sofarsobad > 0)
			return false;
		else if (sofarsogood > 0)
			return true;
		else
			return false;
	}
	else
		return true; // Dunno how we got here
}

void Multicast (vec3_t origin, multicast_t to)
{
	int i = 0;
	int e = 0;

	// Second byte is always the actual TE_*, first has to be svc_temp_entity
	/*if (pworder->byte1 == svc_temp_entity)
	{
		if (!ParilWrite_HasRequirementsForTE())
		{
			Clear_Structure();
			return;
		}
	}*/
	
	gi.dprintf ("%i %i %i %i\n", pworder->byte1, pworder->byte2, pworder->byte3, pworder->byte4);

	// Go through the calls in order.
	for (; i < PWORDER_MAXSLOTS; i++)
	{
		if (pwri_callorders[i] == -1)
			break;
		switch (pwri_callorders[i])
		{
			case PWORDER_CHAR:
				if (pworder->char2 != -1 && pworder->char1 == -1)
				{
					gi.old_WriteChar (pworder->char2);
					pworder->char2 = -1;
				}
				else
				{
					gi.old_WriteChar (pworder->char1);
					pworder->char1 = -1;
				}
				break;
			case PWORDER_BYTE:
				if (pworder->byte2 != -1 && pworder->byte1 == -1)
				{
					gi.old_WriteByte (pworder->byte2);
					pworder->byte2 = -1;
				}
				else if (pworder->byte3 != -1 && pworder->byte2 == -1)
				{
					gi.old_WriteByte (pworder->byte3);
					pworder->byte3 = -1;
				}
				else if (pworder->byte4 != -1 && pworder->byte3 == -1)
				{
					gi.old_WriteByte (pworder->byte4);
					pworder->byte4 = -1;
				}
				else
				{
					gi.old_WriteByte (pworder->byte1);
					pworder->byte1 = -1;
				}
				break;
			case PWORDER_SHORT:
				/*if (pworder->short2 != -1 && pworder->short1 == -1)
				{
					gi.old_WriteShort (pworder->short2);
					pworder->short2 = -1;
				}
				else
				{
					gi.old_WriteShort (pworder->short1);
					pworder->short1 = -1;
				}*/
				for (; e < MAX_ITEMS; e++)
				{
					if (pworder->shorts[e] != -1)
					{
						gi.old_WriteShort (pworder->shorts[e]);
						pworder->shorts[e] = -1;
					}
				}
				break;
			case PWORDER_LONG:
				if (pworder->long2 != -1 && pworder->long1 == -1)
				{
					gi.old_WriteLong (pworder->long2);
					pworder->long2 = -1;
				}
				else
				{
					gi.old_WriteLong (pworder->long1);
					pworder->long1 = -1;
				}
				break;
			case PWORDER_FLOAT:
				if (pworder->float2 != -1 && pworder->float1 == -1)
				{
					gi.old_WriteLong (pworder->float2);
					pworder->float2 = -1;
				}
				else
				{
					gi.old_WriteLong (pworder->float1);
					pworder->float1 = -1;
				}
				break;
			case PWORDER_STRING:
				if (pworder->string2 != NULL && pworder->string1 == NULL)
				{
					gi.old_WriteString (pworder->string2);
					pworder->string2 = NULL;
				}
				else
				{
					gi.old_WriteString (pworder->string1);
					pworder->string1 = NULL;
				}
				break;
			case PWORDER_POSITION:
				if (pworder->pos2[0] != -1 && pworder->pos1[0] == -1)
				{
					gi.old_WritePosition (pworder->pos2);
					VectorSet (pworder->pos2, -1, -1, -1);
				}
				else
				{
					gi.old_WritePosition (pworder->pos1);
					VectorSet (pworder->pos1, -1, -1, -1);
				}
				break;
			case PWORDER_DIR:
				if (pworder->dir2[0] != -1 && pworder->dir1[0] == -1)
				{
					gi.old_WriteDir (pworder->dir2);
					VectorSet (pworder->dir2, -1, -1, -1);
				}
				else
				{
					gi.old_WriteDir (pworder->dir1);
					VectorSet (pworder->dir1, -1, -1, -1);
				}
				break;
			case PWORDER_ANGLE:
				if (pworder->angle2 != -1 && pworder->angle1 == -1)
				{
					gi.old_WriteAngle (pworder->angle2);
					pworder->float2 = -1;
				}
				else
				{
					gi.old_WriteAngle (pworder->angle1);
					pworder->angle1 = -1;
				}
				break;
		}
	}

	gi.old_multicast (origin, to);
	Clear_Structure();
}

/*
	void	(*WriteChar) (int c);
	void	(*WriteByte) (int c);
	void	(*WriteShort) (int c);
	void	(*WriteLong) (int c);
	void	(*WriteFloat) (float f);
	void	(*WriteString) (char *s);
	void	(*WritePosition) (vec3_t pos);	// some fractional bits
	void	(*WriteDir) (vec3_t pos);		// single byte encoded, very coarse
	void	(*WriteAngle) (float f);
*/