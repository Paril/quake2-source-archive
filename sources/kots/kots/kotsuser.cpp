//*************************************************************************************
//*************************************************************************************
// File: kotsuser.cpp
//*************************************************************************************
//*************************************************************************************

//#include "stdafx.h"
#include "kots.h"

#include "user.h"

extern "C"
{
#include "g_local.h"
#include "exports.h"
}

//*************************************************************************************
//*************************************************************************************
// Function: GameSave
//*************************************************************************************
//*************************************************************************************

bool CUser::GameSave( const char *path )
{
	bool bRes;

	if ( m_bCheat )
		return false;

 	if ( sv_cheats->value )
 		return false;

	m_lastport = kots_port->value;

	Uninit();

	bRes = Save( path );

	return bRes;
}

//*************************************************************************************
//*************************************************************************************
// Function: Respawn
//*************************************************************************************
//*************************************************************************************

void CUser::Respawn()
{
	SafeRespawn();
	
	SetMaxHealth(); 
}

//*************************************************************************************
//*************************************************************************************
// Function: ModScore
//*************************************************************************************
//*************************************************************************************

bool CUser::ModScore( long mod, int killtype )
{
	int  level = Level();
	bool bRes  = false;

	Score( mod );
	m_ent->client->resp.score += mod;

	switch( killtype )
	{
		case USER_SUICIDE:
			m_suicide++;
			break;
		case USER_KILLED:
			m_killed++;
			break;
		case USER_KILL:
			m_kills++;
			break;
		case USER_TELEPORT:
			m_teles++;
			break;
		default:
			break;
	}
	SetMaxAmmo  ();
	SetMaxHealth();

	if ( kots_realtime->value )
		GameSave( theApp.GetDataDir() );

	if ( level != Level() )
		bRes = true;

	return bRes;
}

//*************************************************************************************
//*************************************************************************************
// Function: SetMaxHealth
//*************************************************************************************
//*************************************************************************************

long CUser::SetMaxHealth()
{
	long x;

	x = GetMaxHealth();

	if ( !m_ent )
		return x;

	m_ent->max_health              = x;
	m_ent->client->pers.max_health = x;

	return x;
}

//*************************************************************************************
//*************************************************************************************
// Function: SetMaxAmmo
//*************************************************************************************
//*************************************************************************************

void CUser::SetMaxAmmo()
{
	int x;
	
	x = GetAmmoMulti();

	if ( !m_ent )
		return;

	m_ent->client->pers.max_bullets  = x * USER_BASE_BULLETS ;
	m_ent->client->pers.max_shells   = x * USER_BASE_SHELLS  ;
	m_ent->client->pers.max_rockets  = x * USER_BASE_ROCKETS ;
	m_ent->client->pers.max_grenades = x * USER_BASE_GRENADES;
	m_ent->client->pers.max_cells    = x * USER_BASE_CELLS   ;
	m_ent->client->pers.max_slugs    = x * USER_BASE_SLUGS   ;
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSSetItem
//*************************************************************************************
//*************************************************************************************

bool CUser::SetItem( const char *sitem, long &value, bool bSet )
{
	int		  index;
	gitem_t	*item;

	item = FindItem( (char *)sitem );

	if ( !item )
		return false;

	index = ITEM_INDEX( item );

	if ( bSet )	
		m_ent->client->pers.inventory[index] = value;
	else
		value = m_ent->client->pers.inventory[index];

	return true;
}

//*************************************************************************************
//*************************************************************************************
// Function: Init
//*************************************************************************************
//*************************************************************************************

void CUser::Init()
{
	if ( !m_ent )
		return;

	if ( !m_ent->client )
		return;

	SetAllMax();

	SetItem( "tball"          , m_tballz );
	SetItem( "Power Cube"     , m_power  );

	if ( m_health <= 0 )
		return;

	m_ent->health = m_health;

	m_health = 0;

	SetItem( "Bullets" , m_bullets  );
	SetItem( "Shells"  , m_shells   );
	SetItem( "Cells"   , m_cells    );
	SetItem( "Grenades", m_grenades );
	SetItem( "Rockets" , m_rockets  );
	SetItem( "Slugs"   , m_slugs    );

	SetItem( "Blaster"         , m_blaster      );      
	SetItem( "Shotgun"         , m_shotgun      );      
	SetItem( "Super Shotgun"   , m_sshotgun     );     
	SetItem( "Machinegun"      , m_mgun         );         
	SetItem( "Chaingun"        , m_cgun         );         
	SetItem( "Grenade Launcher", m_glauncher    );     
	SetItem( "Rocket Launcher" , m_rlauncher    );     
	SetItem( "HyperBlaster"    , m_hyperblaster );  
	SetItem( "Railgun"         , m_rgun         );          
	SetItem( "BFG10K"          , m_bfg          );           

	SetItem( "Body Armor"  , m_barmor );
}															

//*************************************************************************************
//*************************************************************************************
// Function: Uninit
//*************************************************************************************
//*************************************************************************************

void CUser::Uninit( bool bForce )
{
	if ( !m_ent )
		return;

	if ( !m_ent->client )
		return;

	m_health = m_ent->health;

	if ( m_health <= 0 && !bForce )
		return;

	SetItem( "tball"          , m_tballz, false );
	SetItem( "Power Cube"     , m_power , false );

	SetItem( "Bullets" , m_bullets , false );
	SetItem( "Shells"  , m_shells  , false );
	SetItem( "Cells"   , m_cells   , false );
	SetItem( "Grenades", m_grenades, false );
	SetItem( "Rockets" , m_rockets , false );
	SetItem( "Slugs"   , m_slugs   , false );

	SetItem( "Blaster"         , m_blaster     , false );      
	SetItem( "Shotgun"         , m_shotgun     , false );      
	SetItem( "Super Shotgun"   , m_sshotgun    , false );     
	SetItem( "Machinegun"      , m_mgun        , false );         
	SetItem( "Chaingun"        , m_cgun        , false );         
	SetItem( "Grenade Launcher", m_glauncher   , false );     
	SetItem( "Rocket Launcher" , m_rlauncher   , false );     
	SetItem( "HyperBlaster"    , m_hyperblaster, false );  
	SetItem( "Railgun"         , m_rgun        , false );          
	SetItem( "BFG10K"          , m_bfg         , false );           

	SetItem( "Body Armor"  , m_barmor, false );
}															

//*************************************************************************************
//*************************************************************************************
//*************************************************************************************
//*************************************************************************************


