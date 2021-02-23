#define	GRAPPLE_OFF			0
#define	GRAPPLE_INAIR		1
#define	GRAPPLE_ATTACHED	2		//attached without grow/shrink
#define	GRAPPLE_STARTSHRINK	3	//start with shrink


#define SCANNER_UNIT                   32
#define SCANNER_RANGE                  64
#define SCANNER_UPDATE_FREQ            3
#define SAFE_STRCAT(org,add,maxlen)    if ((strlen(org) + strlen(add)) < maxlen)    strcat(org,add);
#define LAYOUT_MAX_LENGTH              1400


qboolean Jet_Active(edict_t *ent);
qboolean Jet_AvoidGround(edict_t *ent);
void Jet_BecomeExplosion(edict_t *ent, int damage);
void Jet_ApplyJet(edict_t *ent, usercmd_t *ucmd);

