//*************************************************************************************
//*************************************************************************************
// File: kots.cpp
//*************************************************************************************
//*************************************************************************************

//#include "stdafx.h"
#include "kots.h"

#include "kotscpp.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>

#include "user.h"
#include "shared.h"

CKotsApp theApp;

void CheckInThread( void *lpData );

//*************************************************************************************
//*************************************************************************************
// Function: Construction
//*************************************************************************************
//*************************************************************************************

CKotsApp::CKotsApp()
{
	getcwd( m_homedir, _MAX_PATH );

	strcat( m_homedir, "/kots/kotsdata" );

	mkdir( m_homedir,0666 );

	strcat( m_homedir, "/" );

	memset( m_datadir, 0, sizeof m_datadir );
}

CKotsApp::~CKotsApp()
{
	Cleanup();
}

//*************************************************************************************
//*************************************************************************************
// Function: GetDataDir
//*************************************************************************************
//*************************************************************************************

const char *CKotsApp::GetDataDir()
{
	if ( strlen( m_datadir ) < 1 )
		return m_homedir;

	return m_datadir;
}

void CKotsApp::SetDataDir( const char *path )
{
	if ( strlen( path ) < 3 )
	{
		memset( m_datadir, 0, sizeof m_datadir );
		return;
	}
	strcpy( m_datadir, path );

	if ( path[ strlen( path ) - 1 ] != '/' )
		strcat( m_datadir, "/" );

	mkdir( m_datadir,0666 );
}

//*************************************************************************************
//*************************************************************************************
// Function: Cleanup
// Called at dll unload
//*************************************************************************************
//*************************************************************************************

void CKotsApp::Cleanup()
{
	int   x;
	CUser *user;

	for ( x = 0; x < m_users.GetSize(); x++ )
	{
		user = (CUser *)m_users[x];

		user->GameSave( GetDataDir() );

		delete user;
	}
	m_users.RemoveAll();

	for ( x = 0; x < m_packs.GetSize(); x++ )
	{
		user = (CUser *)m_packs[x];

		delete user;
	}
	m_packs.RemoveAll();
}

//*************************************************************************************
//*************************************************************************************
// Function: SaveAll
// Called at map start
//*************************************************************************************
//*************************************************************************************

void CKotsApp::SaveAll()
{
	int   x;
	long  ltime;
	char  str[ _MAX_PATH ];
	CUser *user;

	struct tm *today;

	time( &ltime );

	today = localtime( &ltime );
	strcpy( str, asctime( today ) );

	strcat( str, "Saving all current players." );

	KOTSMessage( str );

	for ( x = 0; x < m_users.GetSize(); x++ )
	{
		user = (CUser *)m_users[x];

		user->GameSave( GetDataDir() );
	}

	for ( x = 0; x < m_packs.GetSize(); x++ )
	{
		user = (CUser *)m_packs[x];

		delete user;
	}
	m_packs.RemoveAll();
}

//*************************************************************************************
//*************************************************************************************
// Function: FindUser
//*************************************************************************************
//*************************************************************************************

CUser *CKotsApp::FindUser( const char *name )
{
	int   x;
	CUser *user;

	for ( x = 0; x < m_users.GetSize(); x++ )
	{
		user = (CUser *)m_users[x];

		if ( !strcasecmp( user->m_name, name ) )
			return user;
	}
	return NULL;
}

//*************************************************************************************
//*************************************************************************************
// Function: AddUser
//*************************************************************************************
//*************************************************************************************

CUser *CKotsApp::AddUser( const char *name, const char *pass, int &x )
{
	long ltime;
	char  str[ _MAX_PATH ];
	CUser *tuser = NULL;
	CUser *user = NULL;

	struct tm *today;

	x = KOTS_SUCCESS;

	tuser = FindUser( name );

	user = new CUser;

	time( &ltime );

	today = localtime( &ltime );

	if ( tuser )
	{
		memcpy( user, tuser, sizeof(CUser));		

		strcpy( str, asctime( today ) );
		strcat( str, "User exists - " );
		strcat( str, name );

		KOTSMessage( str );
	}
	else
	{
		memset( user, 0, sizeof(CUser) );

		strcpy( str, asctime( today ) );
		strcat( str, "Loading - " );
		strcat( str, name );

		KOTSMessage( str );

		if ( user->Load( GetDataDir(), name ) != KOTS_SUCCESS )
		{
			strcpy( user->m_name    , name );
			strcpy( user->m_password, pass );
		
			user->Respawn();
			user->Save   ( GetDataDir() );
		
			m_users.Add( user );

			return user;
		}
	}
	if ( !user->CheckPass( pass ) )
	{
		x = KOTS_INVALID_P_PASS;

		delete user;

		return NULL;
	}
	m_users.Add( user );

	return user;
}

//*************************************************************************************
//*************************************************************************************
// Function: DelUser
//*************************************************************************************
//*************************************************************************************

bool CKotsApp::DelUser( CUser *user, bool bSave )
{
	int  x;
	bool bfound = false;

	for ( x = 0; x < m_users.GetSize(); x++ )
	{
		if ( user == m_users[x] )
		{
			m_users.RemoveAt( x );
			bfound = true;
			break;
		}
	}

	if ( bSave )
		user->GameSave( GetDataDir() );

	delete user;

	return bfound;
}

//*************************************************************************************
//*************************************************************************************
//*************************************************************************************
//*************************************************************************************

