// my functions
void            PlaceLaser (edict_t *ent);
void            pre_target_laser_think (edict_t *self);

// controlling parameters
#define         LASER_TIME                                30
#define         CELLS_FOR_LASER                           20
#define         LASER_DAMAGE                             100
#define         LASER_MOUNT_DAMAGE                        50
#define         LASER_MOUNT_DAMAGE_RADIUS                 64

// In-built Quake2 routines
void            target_laser_use (edict_t *self, edict_t *other, edict_t *activator);
void            target_laser_think (edict_t *self);
void            target_laser_on (edict_t *self);
void            target_laser_off (edict_t *self);

#define FENCE_LENGTH 120
// Laser Beam Color Codes
#define Laser_Red 0xf2f2f0f0 // bright red
#define Laser_Green 0xd0d1d2d3 // bright green
#define Laser_Blue 0xf3f3f1f1 // bright blue
#define Laser_Yellow 0xdcdddedf // bright yellow
#define Laser_YellowS 0xe0e1e2e3 // yellow strobe
#define Laser_DkPurple 0x80818283 // dark purple
#define Laser_LtBlue 0x70717273 // light blue
#define Laser_Green2 0x90919293 // different green
#define Laser_Purple 0xb0b1b2b3 // purple
#define Laser_Red2 0x40414243 // different red
#define Laser_Orange 0xe2e5e3e6 // orange
#define Laser_Mix 0xd0f1d3f3 // mixture
#define Laser_RedBlue 0xf2f3f0f1 // inner = red, outer = blue
#define Laser_BlueRed 0xf3f2f1f0 // inner = blue, outer = red
#define Laser_GreenY 0xdad0dcd2 // inner = green, outer = yellow
#define Laser_YellowG 0xd0dad2dc // inner = yellow, outer = green

void SP_LaserSight(edict_t *self);
