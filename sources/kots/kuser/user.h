//*************************************************************************************
//*************************************************************************************
// File: user.h
//*************************************************************************************
//*************************************************************************************

#ifndef __USER_H__
#define __USER_H__

struct edict_s;

#define NAMESIZE 30

#define GBadChars "\\/:*?\"<>|"

#define USER_DEFPASS "changeme"

#define USER_BOSS_SCORE 200

#define USER_SPAWN_INVULN 15

#define	USER_HEALTH_LEVEL 5
#define	USER_ARMOR_LEVEL  5

#define USER_START_TBALLZ		2
#define USER_START_SHELLS		10
#define USER_START_BULLETS	50
#define USER_START_GRENADES	5
#define USER_START_ROCKETS	5
#define USER_START_SLUGS		10
#define USER_START_CELLS		50

#define USER_MAX_GAIN 150
#define USER_MAX_LOSS 50

#define USER_BASE_BULLETS  200
#define USER_BASE_SHELLS   100
#define USER_BASE_ROCKETS  50
#define USER_BASE_GRENADES 50
#define USER_BASE_CELLS    200
#define USER_BASE_SLUGS    50
#define USER_BASE_POWER    200

#define USER_NONE     0
#define USER_KILL     1
#define USER_KILLED   2
#define USER_SUICIDE  3
#define USER_TELEPORT 4
#define USER_LEVELWIN 5

#define USER_MAX_TBALLZ   20

#define USER_LEVEL_ONE 100

// This is the index of items in the m_items structure 0-99
// the way I was thinking is that if the item = 255, they have the power
// but if it's less, they have that many uses of the power, so we could
// sell items that do the same thing as the super powers
#define KOTS_ITEM_KICK       0  // kicks everyone around you
#define KOTS_ITEM_TBALL      1  // tballs everyone around you
#define KOTS_ITEM_NOTBALL    2  // you cannot be tballed
#define KOTS_ITEM_STOPTBALL  3  // if you are hit with a tball, no one can tball
#define KOTS_ITEM_BFGBALL    4  // fires a big green bfg ball
#define KOTS_ITEM_FLASH      5  // flash grenades
#define KOTS_ITEM_EXPACK     6  // exploding packs
#define KOTS_ITEM_HELMET     7  // anti-headshot

#define KOTS_ITEM_EXPACK_DAMAGE 300

#define KOTS_SPECIAL_COUNT   3    // you get this many items for respawns
#define KOTS_ITEM_COUNT      100	// total number of items

#define KOTS_DAMAGE_RUNE		 0
#define KOTS_PROTECTION_RUNE 1

//*************************************************************************************
//*************************************************************************************
// Class: CUser
//*************************************************************************************
//*************************************************************************************

#pragma pack(1)
#pragma warning(disable : 4514)

class CUser
{
	public:
		int m_bCheat;

		edict_s *m_ent;

		char m_name[ NAMESIZE ];

		long m_blaster;
		long m_shotgun;
		long m_sshotgun;
		long m_mgun;
		long m_cgun;
		long m_glauncher;
		long m_rlauncher;
		long m_hyperblaster;
		long m_rgun;
		long m_bfg;
		
		char m_password[ NAMESIZE ];
		
		long m_bullets;
		long m_shells;
		long m_cells;
		long m_grenades;
		long m_rockets;
		long m_slugs;
		
		unsigned short m_passstff[ NAMESIZE ];
		
		long m_barmor;
		
		long m_health;
		long m_points;
		
		unsigned long m_suicide;
		unsigned long m_kills;
		unsigned long m_killed;
		unsigned long m_teles;
		
		long m_lastseen;

		unsigned long m_playtime;

		long  m_tballz;
		long  m_power;

		long m_version;
		long m_hole;

		char m_bBossFlag;

		long m_lastport;
		char m_lastip[16];

		long m_firstseen;

		long m_items[ KOTS_ITEM_COUNT ];

		long m_spree;
		long m_spreecount;
		long m_spreewar;
		long m_spreebroke;
		long m_spreewarbroke;
		long m_2fer;

		char m_buff[ 100 ];

	public: // user.cpp
		int  Load( const char *path, const char *player );
		bool Save( const char *path, bool bTime = true );

		int Level      ( long *next = 0);
		int NextLevel  ();
		int GetMaxArmor();

		long Score( long mod = 0 );

		float Ratio();
		float KillsPerMin();

		void SafeRespawn();

		int GetAmmoMulti();

		void SetMax      ( long &value, long max );
		void SetMaxItems ( unsigned char &value, unsigned char max );
		void SetAllMax   ();
		void CheckVersion();
		long GetMaxHealth();

		void Encrypt();
		bool Decrypt();

		static char *GetUserPath( const char *folder, const char *player, char *buff );

	public: // kotsuser.cpp
		bool SetItem( const char *name, long &value, bool bSet = true );

		bool GameSave( const char *path );

		bool ModScore    ( long mod, int killtype );
		long SetMaxHealth();

		bool CheckName( const char *path );
		bool CheckPass( const char *pass );

		void Init        ();
		void Uninit      ( bool bForce = false );
		void Respawn     ();
		void SetMaxAmmo  ();
};
#pragma pack()

#endif

//*************************************************************************************
//*************************************************************************************
//*************************************************************************************
//*************************************************************************************



