#define		LASER_TIME									30
#define		CELLS_FOR_LASER								10
#define		LASER_DAMAGE								100
#define		LASER_MOUNT_DAMAGE							50
#define		LASER_MOUNT_DAMAGE_RADIUS					64	// In-built Quake2 routines
#define     MOD_LASER                                   34

void	    PlaceLaser (edict_t *ent, vec3_t plane);
void    	pre_target_laser_think (edict_t *self);	// controlling parameters
void		target_laser_use (edict_t *self, edict_t *other, edict_t *activator);
void		target_laser_think (edict_t *self);
void		target_laser_on (edict_t *self);
void		target_laser_off (edict_t *self);

