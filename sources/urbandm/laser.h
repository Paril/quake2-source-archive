// LASER.H


	void		PlaceLaser (edict_t *ent);
	void    	pre_target_laser_think (edict_t *self);

	// controlling parameters
	#define		LASER_TIME								30.0
	#define		CELLS_FOR_LASER							5
	#define		LASER_DAMAGE							35
	#define		LASER_MOUNT_DAMAGE						60
	#define		LASER_MOUNT_DAMAGE_RADIUS				250

	// In-built Quake2 routines
	void		target_laser_use (edict_t *self, edict_t *other, edict_t *activator);
	void		target_laser_think (edict_t *self);
	void		target_laser_on (edict_t *self);
	void		target_laser_off (edict_t *self);

	//### SPK ### 2/08/98
	void TripBomb_Explode (edict_t *self);
