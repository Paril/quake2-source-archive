//*************************************************************************************
//*************************************************************************************
// File: kots.h
//*************************************************************************************
//*************************************************************************************

#ifndef __KOTS_H__
#define __KOTS_H__

#include "nptrarray.h"

class CUser;

//*************************************************************************************
//*************************************************************************************
// Class: CKotsApp
//*************************************************************************************
//*************************************************************************************

class CKotsApp
{
	private:
		CNPtrArray m_users;

		CUser *FindUser( const char *name );

		char m_homedir[ _MAX_PATH ];
		char m_datadir[ _MAX_PATH ];

	public:
		CKotsApp ();
		~CKotsApp();

		CNPtrArray m_packs;

		const char *GetDataDir();

		void SetDataDir( const char *path );

		CUser *AddUser( const char *name, const char *pass, int &x );

		void Cleanup();
		void SaveAll();
		bool DelUser( CUser *user, bool bSave = true );
};
extern CKotsApp theApp;

#endif

//*************************************************************************************
//*************************************************************************************
//*************************************************************************************
//*************************************************************************************

