#ifndef MYSQL_GDS
#define MYSQL_GDS

/* 
So back then there was a GDS 
and it had issues.

But now, we're doing it Right!<tm>

So, with a few pointers from KOTS2007
(ideas borrowed from kots. heheh)
we're having a MYSQL GDS.

						-az

(this will also work with qfusion)
*/
#ifndef NO_GDS

#define GDS_LOAD 1
#define GDS_SAVE 2
#define GDS_SAVECLOSE 3
#define GDS_SAVERUNES 4
#define GDS_EXITTHREAD 5

// For Everyone
// void V_GDS_Load(edict_t *ent);
qboolean V_GDS_StartConn();
void V_GDS_Queue_Add(edict_t *ent, int operation);
qboolean CanUseGDS();
#ifndef GDS_NOMULTITHREADING
void GDS_FinishThread();
void HandleStatus(edict_t *player);
#endif

void Mem_PrepareMutexes();

#endif //NO_GDS

// Wrapped, thread safe mem allocation.
void *V_Malloc(size_t Size, int Tag);
void V_Free (void* mem);

#endif // MYSQL_GDS