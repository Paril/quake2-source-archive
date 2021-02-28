#ifndef AURAS_H
#define AURAS_H

typedef struct 
{
	edict_t *ent;
	edict_t *owner;
	int		type;
	float	time;
}aura_t;

typedef struct
{
	edict_t *ent;
	float	time;
}que_t;

qboolean que_addent (que_t *que, edict_t *other, float duration);
void que_removetype (que_t *que, int type, qboolean free);
qboolean que_typeexists (que_t *que, int type);
que_t *que_findent (que_t *src, que_t *dst, edict_t *other);
que_t *que_findtype (que_t *src, que_t *dst, int type);
void que_removeent (que_t *que, edict_t *other, qboolean free);
void que_empty (que_t *que);
void CurseRemove (edict_t *ent, int type);
void AuraRemove (edict_t *ent, int type);

#endif