#include "g_local.h"

#define NUM_PIECE_TYPES 7
#define PIECE_SIZE 4
#define PIECE_ROTATIONS 4

#define DEG2RAD( a ) ( a * M_PI ) / 180.0F

typedef struct qt_pair_s {
	int x;
	int y;
} qt_pair_t;

static qt_pair_t qt_piece_shapes[] = {
	// long piece
	{ 0, 0 }, { 0, 1 }, { 0, 2 }, { 0, 3 },
	{ -1, 1}, { 0, 1 }, { 1, 1 }, { 2, 1 },
	{ 0, 0 }, { 0, 1 }, { 0, 2 }, { 0, 3 },
	{ -1, 1}, { 0, 1 }, { 1, 1 }, { 2, 1 },
	// left 'L'
	{ 0, 0 }, { 1, 0 }, { 0, 1 }, { 0, 2 },
	{ -1, 1 }, { -1, 0 }, { 0, 1 }, { 1, 1 },
	{ 0, 2 }, { -1, 2 }, { 0, 1 }, { 0, 0 },
	{ 1, 1 }, { 1, 2 }, { 0, 1 }, { -1, 1 },
	// right 'L'
	{ -1, 0 }, { 0, 0 }, { 0, 1 }, { 0, 2 },
	{ -1, 2 }, { -1, 1 }, { 0, 1 }, { 1, 1 },
	{ 1, 2 }, { 0, 2 }, { 0, 1 }, { 0, 0 },
	{ 1, 0 }, { 1, 1 }, { 0, 1 }, { -1, 1 },
	// square
	{ 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 },
	{ 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 },
	{ 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 },
	{ 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 },
	// left 'Z'
	{ -1, 0 }, { 0, 0 }, { 0, 1 }, { 1, 1 },
	{ 0, 0 }, { 0, 1 }, { -1, 1 }, { -1, 2 },
	{ -1, 0 }, { 0, 0 }, { 0, 1 }, { 1, 1 },
	{ 0, 0 }, { 0, 1 }, { -1, 1 }, { -1, 2 },
	// right 'Z'
	{ 0, 0 }, { 1, 0 }, { 0, 1 }, { -1, 1 },
	{ 0, 1 }, { 0, 0 }, { 1, 1 }, { 1, 2 },
	{ 0, 0 }, { 1, 0 }, { 0, 1 }, { -1, 1 },
	{ 0, 1 }, { 0, 0 }, { 1, 1 }, { 1, 2 },
	// 'T'
	{ 0, 1 }, { 1, 1 }, { 0, 0 }, { -1, 1 },
	{ 0, 1 }, { 0, 0 }, { -1, 1 }, { 0, 2 },
	{ 0, 1 }, { -1, 1 }, { 0, 2 }, { 1, 1 },
	{ 0, 1 }, { 0, 2 }, { 1, 1 }, { 0, 0 },
};

#define pieceStart(shape, rot) (qt_piece_shapes + (shape * PIECE_ROTATIONS * PIECE_SIZE) + (rot * PIECE_SIZE) )

void putPiece(T_arena_t *arena);
void doMove(T_arena_t *arena, int offx, int offy);
void doRotate(T_arena_t *arena, int dir);
qboolean checkMove(T_arena_t *arena, int offx, int offy);
qboolean checkRotate(T_arena_t *arena, int dir);

// qboolean scanRows(arena);

void issueCommand(T_arena_t *arena, T_command_t command) {
	if (arena->player->state != BLOCK_FALLING) return;
	switch(command) {
	case BL_LEFT:
		if (checkMove(arena, 1, 0))
			doMove(arena, 1, 0);
	break;
	case BL_RIGHT:
		if (checkMove(arena, -1, 0))
			doMove(arena, -1, 0);
	break;
	case BL_ROT:
		if (checkRotate(arena, -1))
			doRotate(arena, -1);
	break;
	case BL_DROP:
		arena->player->state = BLOCK_DROPPING;
	break;
	}
}

qboolean checkValid(T_arena_t *arena) { 
	int i, x, y;
	qt_pair_t *pair;

	for (i = 0; i < PIECE_SIZE; i++) {
		pair = pieceStart(arena->cp_shape, arena->cp_rot) + i;
		x = arena->cp_x + pair->x;
		y = arena->cp_y + pair->y;
		if (arena->map[y * arena->width + x] ) return false;
		if (x < 0 || y < 0 || x >= arena->width || y >= arena->height) return false;
	}
	return true;
}	

qboolean checkMove(T_arena_t *arena, int offx, int offy) {
	int oldx, oldy;
	qboolean result;
	oldx = arena->cp_x; oldy = arena->cp_y;
	arena->cp_x += offx;
	arena->cp_y += offy;
	result = checkValid(arena);
	arena->cp_x = oldx; arena->cp_y = oldy;
	return result;
}

void doMove(T_arena_t *arena, int offx, int offy) {
	edict_t *ent;

	ent = arena->current_piece;
	while (ent) {
		gi.unlinkentity(ent);
		ent = ent->qt_next_block;
	}
	arena->cp_x += offx;
	arena->cp_y += offy;
	ent = arena->current_piece;
	putPiece(arena);
}	

void doRotate(T_arena_t *arena, int dir) {
	edict_t *ent;

	ent = arena->current_piece;
	while (ent) {
		gi.unlinkentity(ent);
		ent = ent->qt_next_block;
	}
	arena->cp_rot = (arena->cp_rot + dir) % PIECE_ROTATIONS;
	if (arena->cp_rot < 0) arena->cp_rot = PIECE_ROTATIONS - 1;
	ent = arena->current_piece;
	putPiece(arena);
}	

// +1 == CCW, -1 == CW
qboolean checkRotate(T_arena_t *arena, int direction) {
	int oldrot;
	qboolean result;
	oldrot = arena->cp_rot;
	arena->cp_rot = (arena->cp_rot + direction) % PIECE_ROTATIONS;
	if (arena->cp_rot < 0) arena->cp_rot = PIECE_ROTATIONS - 1;
	result = checkValid(arena);
	arena->cp_rot = oldrot;
	return result;
}	

void fixPiece(T_arena_t* arena) {
	int i, x, y;
	qt_pair_t *pair;
	edict_t* ent;

	i = 0; ent = arena->current_piece;
	while (ent) {
		pair = pieceStart(arena->cp_shape, arena->cp_rot) + i;
		x = arena->cp_x + pair->x;
		y = arena->cp_y + pair->y;
		arena->map[(y * arena->width) + x] = ent;
		ent = ent->qt_next_block; i++;
	}

	// do sounds-- this counts as a FIXME
	gi.sound(arena->current_piece, CHAN_AUTO, gi.soundindex( "player/land1.wav" ), 0.2, ATTN_NORM, 0);

	arena->current_piece = NULL;
}

// returns false if it dropped, true if it fixed
qboolean dropPiece1(T_arena_t *arena) {
	if (checkMove(arena, 0, 1)) {
		doMove(arena, 0, 1);
		return false;
	} else {
		fixPiece(arena);
	}
	return true;
}

void SP_info_blockspawn(edict_t *ent) {
	T_arena_t *arena;
	float angle;
	// temp
	// if (st.arena == 0) st.arena = 1;
	if (st.arena == 0) {
		G_FreeEdict(ent);
		return;
	}
	arena = &level.arenas[st.arena-1];
	arena->block_start = ent;
	angle = ent->s.angles[1];
	VectorSet(arena->xoff, (int) ( (float)20 * -cos ( DEG2RAD(angle) ) ),
		(int) ( (float)20 * -sin ( DEG2RAD(angle) ) ), 0 );
	VectorSet(arena->yoff, 0, 0, -20);
	
	gi.dprintf("arena %d: block start at %f %f %f\n", st.arena-1, ent->s.origin[0],ent->s.origin[1],ent->s.origin[2]);
}

void SP_info_blockend(edict_t *ent) {
	T_arena_t *arena;
	if (st.arena == 0) {
		G_FreeEdict(ent);
		return;
	}
	arena = &level.arenas[st.arena-1];
	arena->block_end = ent;
	
	gi.dprintf("arena %d: block end at %f %f %f\n", st.arena-1, ent->s.origin[0],ent->s.origin[1],ent->s.origin[2]);
}

#define BLOCKSIZE 20;

void SP_prepare_arena( T_arena_t* arena ) {
	vec3_t asize;
	vec3_t zoff;
	vec3_t temp;
	
	float width, height;
	
	if ( !arena->block_start ) return;
	
	if ( arena->map ) return;

	// generate axes
	VectorNormalize( arena->xoff );
	gi.dprintf(" xoff:\t%f\t%f\t%f\n", arena->xoff[0], arena->xoff[1], arena->xoff[2]);
	VectorNormalize( arena->yoff );
	CrossProduct( arena->xoff, arena->yoff, zoff);
	
	// find arena size
	VectorSubtract( arena->block_end->s.origin,
		arena->block_start->s.origin,
		asize);
	gi.dprintf(" asize:\t%f\t%f\t%f\n", asize[0], asize[1], asize[2]);
		
	// get width and height
	arena->blocksize = BLOCKSIZE;
	width = fabs( DotProduct( asize, arena->xoff ) );
	height = fabs( DotProduct( asize, arena->yoff) );

	arena->width = (int) width / BLOCKSIZE;
	arena->height = (int) height / BLOCKSIZE;

	// scale xoff and yoff
	VectorScalarMultiply(arena->xoff, arena->blocksize);
	VectorScalarMultiply(arena->yoff, arena->blocksize);
	
	// reset block_start so that it spawns the pieces in the correct place
	VectorCopy( arena->block_end->s.origin, arena->block_start->s.origin );
		// add width - 10
	VectorCopy( arena->xoff, temp );
	VectorScalarMultiply( temp, (-arena->width + 0.5) );
	VectorAdd( arena->block_start->s.origin, temp, arena->block_start->s.origin );
		// add height - 10
	VectorCopy( arena->yoff, temp );
	VectorScalarMultiply( temp, (-arena->height + 0.5) );
	VectorAdd( arena->block_start->s.origin, temp, arena->block_start->s.origin );
		// add depth + 10
	VectorCopy( zoff, temp );
	VectorScalarMultiply( temp, arena->blocksize / 2 );
	VectorAdd( arena->block_start->s.origin, temp, arena->block_start->s.origin );
	
	gi.dprintf(" Arena prepared with height %d, width %d, blocksize %d \n", 
		arena->height, arena->width, arena->blocksize );

	arena->map = gi.TagMalloc(arena->width * arena->height * sizeof(edict_t*), TAG_LEVEL);
	memset(arena->map, 0, arena->width * arena->height * sizeof(edict_t*));
	arena->current_piece = NULL;
	arena->bstime = 1;
	arena->basedwtime = 8;
	arena->wftime = 10;
}

qboolean spawnPiece(T_arena_t *arena) {
	edict_t *ent, *lastent;
	int i;

	if (arena->current_piece)
		gi.error("QTET: Attempted to spawn piece in arena with active piece.");
	// choose next piece-- mod required since random can give 1.0
	arena->cp_shape = ((int)(random() * NUM_PIECE_TYPES)) % NUM_PIECE_TYPES;
	arena->cp_rot = 0;
	arena->cp_x = 5;
	arena->cp_y = 0;

	if (!checkValid(arena)) {
		// do that thing that you do when you lose
		return true;
	}
	// create an active piece
	lastent = NULL;
	for (i = 0; i < PIECE_SIZE; i++) {
		ent = G_Spawn();
		ent->clipmask = MASK_SHOT;
		ent->solid = SOLID_BBOX;
		VectorClear(ent->mins);
		VectorClear(ent->maxs);
		ent->s.modelindex = gi.modelindex("models/blocks/block.md2");
		ent->s.skinnum = arena->cp_shape;
		ent->qt_next_block = lastent;
		lastent = ent;
	}
	arena->current_piece = lastent;

	putPiece(arena);
	
	return false;
}

void putPiece(T_arena_t *arena) {
	edict_t *ent;
	qt_pair_t* psoff;
	int i, j;

	ent = arena->current_piece; i = 0;
	while (ent) {
		// set position
		VectorCopy(arena->block_start->s.origin, ent->s.origin);
		psoff= pieceStart(arena->cp_shape, arena->cp_rot) + i;
		for (j = 0; j < 3; j++)
			ent->s.origin[j] += arena->xoff[j] * (psoff->x + arena->cp_x) + arena->yoff[j] * (psoff->y + arena->cp_y);
		//VectorCopy(arena->block_start->s.angles, ent->s.angles);
		ent->s.angles[PITCH] = 0;
		ent->s.angles[YAW] = arena->block_start->s.angles[YAW] - 90;
		ent->s.angles[ROLL] = -90 * arena->cp_rot;
		gi.linkentity(ent);
		ent = ent->qt_next_block; i++;
	}
}

int scanRows(T_arena_t *arena) {
	int i, j, k, count, total;
	//int filled[arena->height];
	// FIXME:  make this work with lesser compilers
	int filled[4];
	
	total = 0;

	arena->player->score += (arena->player->level * 9);

	for (i = arena->height - 1; i >=0; i--) {
		for (j = count = 0; j < arena->width; j++)
			if (arena->map[ j + (i * arena->width) ])
				count++;
		if (count == j) filled[total++] = i;
	}
	if (total == 0) return 0;

	// moving pass

	count = 0;
	for (i = arena->height - 1; i >=0; i--) {
		if ( (i < total) ) {
			for (j = 0; j < arena->width; j++) {
				edict_t *ent = arena->map[ j + ( i * arena->width)];
				if (ent) {
					gi.unlinkentity(ent);
					G_FreeEdict(ent);
				}
				arena->map[ j + ( i * arena->width)] = NULL;
			}
		} else if ( (count < total) && (filled[count] == i) ) {
			count++;
			// remove this row
			for (j = 0; j < arena->width; j++) {
				edict_t* ent = arena->map[ j + (i * arena->width)];
				gi.unlinkentity(ent);
				G_FreeEdict(ent);
				arena->map[ j + (i * arena->width)] = NULL;
			}
			// do extra-cool very hype railgun spiral trail effect thingy
			{
				vec3_t vect, front;
				VectorCopy(arena->block_start->s.origin, vect);

				VectorCopy(arena->xoff, front);
				PerpendicularVector(front, arena->yoff);
				// a bit in front of the map as well-- we don't want the
				// pretty little spiral spoilt by those nasty little blocks
				for (k = 0; k < 3; k++)
					vect[k] += ( arena->yoff[k] * i ) + ( front[k] * 15 );				
				gi.WriteByte(svc_temp_entity);
				gi.WriteByte(TE_RAILTRAIL);
				gi.WritePosition(vect);
				for (k = 0; k < 3; k++)
					vect[k] += ( arena->xoff[k] * (arena->width) );
				gi.WritePosition(vect);
				gi.multicast( vect, MULTICAST_PHS );
			}
		} else {
			if (count > 0) {
				// drop this row
				for (j = 0; j < arena->width; j++) {
					edict_t* ent = arena->map[ j + (i * arena->width)];
					if (ent) {
						gi.unlinkentity(ent);
						// set new position of block
						VectorCopy(arena->block_start->s.origin, ent->s.origin);
						for (k = 0; k < 3; k++)
							ent->s.origin[k] += ( arena->xoff[k] * j ) + ( arena->yoff[k] * (i + count) );
						gi.linkentity(ent);
					}
					arena->map[ j + ( (i+count) * arena->width)] =
						arena->map[j + (i * arena->width)];
					arena->map[ j + (i * arena->width)] = NULL;
				}
			}

		}
	}
	// do sounds-- this counts as a FIXME
	if ( total > 0 ) {
		// for now, just a generic railgun *WH00P* at blockspawn is fine
		// clamp max. out at 1.0
		float volume = 0.4 + 0.15 * total;
		if (volume > 1.0) volume = 1.0;
		gi.sound(arena->block_start, CHAN_WEAPON, gi.soundindex( "weapons/RAILGF1A.WAV" ), volume, ATTN_NORM, 0);
	}
	// update rows, etc.
	{ 
		int level = arena->player->level;
		arena->player->score += (total) * 9 + level * level * total * total * 25;
		arena->player->rows += total;		
	}
	return total;
}

void startGame (edict_t* ent) {
	T_arena_t* arena;
	gclient_t* client;
	int i;

	client = ent->client;
	arena = client->arena;

	if (arena == NULL) {
		gi.dprintf("no arena\n");
		return;
	}
	
	client->timer = 0;
	client->state = WAIT_BLOCKSPAWN;
	client->ps.pmove.origin[0] = arena->player_start->s.origin[0]*8;
	client->ps.pmove.origin[1] = arena->player_start->s.origin[1]*8;
	client->ps.pmove.origin[2] = arena->player_start->s.origin[2]*8;

	client->ps.fov = 100;
	client->ps.gunindex = 0;

	ent->client->score = 0;
	ent->client->level = 1;
	ent->client->rows = 0;
	ent->client->ps.stats[3] = 0;
	ent->client->ps.stats[4] = 0;
	ent->client->ps.stats[8] = 0;
	ent->client->ps.stats[9] = 1;

	// clear out arena
	for ( i = 0; i < arena->width * arena->height; i++) {
		edict_t* block = arena->map[ i ];
		if (!block) continue;
		gi.unlinkentity(block);
		G_FreeEdict(block);
		arena->map[ i ] = NULL;
	}
	
	arena->current_piece = NULL;
	arena->bstime = 1;
	arena->basedwtime = 8;
	arena->wftime = 10;

	// do sounds-- this counts as a FIXME
	gi.sound(ent, CHAN_AUTO, gi.soundindex( "gunner/sight1.wav" ), 0.8, ATTN_NORM, 0);
}
