// mainly for z_items.c
// item->tags
typedef enum {
        II_NONE,        // SHOULD NEVER HAPPEN
        II_HANDS,
        II_PISTOL,
        II_TWIN_PISTOL,
        II_SUBMACH,
        II_TWIN_SUBMACH,
        II_SHOTGUN,
        II_TWIN_SHOTGUN,
        II_FRAG_HANDGRENADE,
        II_EMP_HANDGRENADE,
        II_CHAINGUN,
        II_GRENADE_LAUNCHER,
        II_ASSAULT_RIFLE,
        II_RAILGUN,
        II_WEAPON_EDIT,
        II_STROGG_BLASTER,
        II_STROGG_SHOTGUN,
        II_STROGG_SUBMACH,
	II_INF_CHAINGUN,
	II_GUN_CHAINGUN,
	II_MEDIC_HYPER_BLASTER,
	II_BITCH_ROCKET_LAUNCHER,
	II_TANK_ROCKET_LAUNCHER,
        II_MAX_WEAPONS,
        II_PISTOL_CLIP,
        II_SUBMACH_CLIP,
        II_SHOTGUN_CLIP,
        II_CHAINGUN_PACK,
        II_FRAG_GRENADES,
        II_ARIFLE_CLIP,
        II_SLUGS,
        II_EXP_SHOTGUN_CLIP,
        II_SOLID_SHOTGUN_CLIP,
        II_HEP_GRENADES,
        II_EMP_GRENADES,
        II_HV_SUBMACH_CLIP,
        II_HEALTH,
        II_HEALTH_LARGE,
	II_DATA_CD,
	II_POWER_CUBE,
	II_PYRAMID_KEY,
	II_DATA_SPINNER,
	II_SECURITY_PASS,
	II_BLUE_KEY,
	II_RED_KEY,
	II_COM_HEAD,
	II_AIRSTRIKE_MARKER,
	II_JACKET_ARMOUR,
	II_COMBAT_ARMOUR,
	II_BODY_ARMOUR,
	II_BANDOLIER,
	II_BACK_PACK,
       	II_MAX
} itemtags_t;

typedef enum
{
        BA_LEG_ARMOUR,
        BA_CHEST_ARMOUR,
        BA_ARMS_ARMOUR,
        BA_HEAD_ARMOUR,
        BA_PANTS_FL,
        BA_PANTS_FR,
        BA_PANTS_BL,
        BA_PANTS_BR,
        BA_RIGHT_SHOULDER,
        BA_RIGHT_SHOULDER_CLIPA,
        BA_RIGHT_SHOULDER_CLIPB,
        BA_LEFT_SHOULDER,
        BA_LEFT_SHOULDER_CLIPA,
        BA_LEFT_SHOULDER_CLIPB,
        BA_OVER_BACK,
        BA_BANDOLIER,
        BA_BANDOLIER_SLOT1,
        BA_BANDOLIER_SLOT2,
        BA_BANDOLIER_SLOT3,
        BA_BANDOLIER_SLOT4,
        BA_BANDOLIER_SLOT5,
        BA_BANDOLIER_SLOT6,
        BA_BACK_PACK_SLOT1,
        BA_BACK_PACK_SLOT2,
        BA_BACK_PACK_SLOT3,
        BA_BACK_PACK_SLOT4,
        BA_BACK_PACK_SLOT5,
        BA_BACK_PACK_SLOT6,
        BA_BACK_PACK_SLOT7,
        BA_BACK_PACK_SLOT8,
        BA_BACK_PACK_SLOT9,
        BA_BACK_PACK_SLOT10,
        BA_MAX
} bodyarea_t;

// STAT_#HAND_FLAGS
// extra_ammo1-4 must be first
#define SHF_EXTRA_AMMO1         1
#define SHF_EXTRA_AMMO2         2
#define SHF_EXTRA_AMMO3         4
#define SHF_EXTRA_AMMO4         8
#define SHF_ALT_AMMO            16
#define SHF_EXTRA_ALT_AMMO      32
// for dropped items (see 'ThrowRight/LeftHandItem()' in g_cmds.c)
#define SHF_DROPPED             64
#define SHF_EMPTY               128
