
// we later use this to generate a hash.
typedef void (*PlayerCommand)(edict_t* ent);

typedef struct 
{
	const char* FunctionName;
	PlayerCommand Function;
} gameCommand_s;

// we put the hashed objects here- once it's implemented.
// extern gameCommand_s* commandListHashed;

qboolean VortexCommand(char* command, edict_t* ent);