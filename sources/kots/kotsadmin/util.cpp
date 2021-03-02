//*************************************************************************************************************
//*************************************************************************************************************
// File: util.cpp
//*************************************************************************************************************
//*************************************************************************************************************

#include "stdafx.h"
#include <shlobj.h>
#include "kotsadmin.h"

#include "util.h"
#include "doptions.h"

#define KOTS_REG "SOFTWARE\\Norb\\KOTSAdmin\\"

//*************************************************************************************************************
//*************************************************************************************************************
// Function: KOTSLoad
//*************************************************************************************************************
//*************************************************************************************************************

void KOTSLoad( SPref *spref, bool bLoad )
{
	if ( bLoad )
	{
		UTGetRegInt( HKEY_LOCAL_MACHINE, KOTS_REG, "Choice"    , spref->ichoice   );
		UTGetRegInt( HKEY_LOCAL_MACHINE, KOTS_REG, "Minutes"   , spref->iminutes  );
		UTGetRegInt( HKEY_LOCAL_MACHINE, KOTS_REG, "Level"     , spref->ilevel    );
		UTGetRegInt( HKEY_LOCAL_MACHINE, KOTS_REG, "EnableFTP" , spref->bftp      );
		UTGetRegInt( HKEY_LOCAL_MACHINE, KOTS_REG, "EnableHTML", spref->bhtml     );
		UTGetRegInt( HKEY_LOCAL_MACHINE, KOTS_REG, "SplitLvls" , spref->bsplit    );
		UTGetRegInt( HKEY_LOCAL_MACHINE, KOTS_REG, "PerPage"   , spref->iperpage  );
		UTGetRegInt( HKEY_LOCAL_MACHINE, KOTS_REG, "NumPages"  , spref->inumpages );

		UTGetRegValue( HKEY_LOCAL_MACHINE, KOTS_REG, "DataDir", spref->sdatadir );
		UTGetRegValue( HKEY_LOCAL_MACHINE, KOTS_REG, "OutFile", spref->soutdir  );
		UTGetRegValue( HKEY_LOCAL_MACHINE, KOTS_REG, "HTML"   , spref->shtml    );

		UTGetRegValue( HKEY_LOCAL_MACHINE, KOTS_REG, "FTPServer", spref->sserver );
		UTGetRegValue( HKEY_LOCAL_MACHINE, KOTS_REG, "FTPUser"  , spref->suser   );
		UTGetRegValue( HKEY_LOCAL_MACHINE, KOTS_REG, "FTPPass"  , spref->spass   );
		UTGetRegValue( HKEY_LOCAL_MACHINE, KOTS_REG, "FTPDir"   , spref->sdir    );

		if ( spref->ichoice < 0 )
			spref->ichoice = 0;

		if ( spref->iminutes < 1 )
			spref->iminutes = 30;

		if ( spref->iperpage < 10 )
			spref->iperpage = 50;

		if ( spref->inumpages < 0 || spref->inumpages > 1000 )
			spref->inumpages = 0;
	}
	else
	{
		UTSetRegInt( HKEY_LOCAL_MACHINE, KOTS_REG, "Choice"    , spref->ichoice   );
		UTSetRegInt( HKEY_LOCAL_MACHINE, KOTS_REG, "Minutes"   , spref->iminutes  );
		UTSetRegInt( HKEY_LOCAL_MACHINE, KOTS_REG, "Level"     , spref->ilevel    );
		UTSetRegInt( HKEY_LOCAL_MACHINE, KOTS_REG, "EnableFTP" , spref->bftp      );
		UTSetRegInt( HKEY_LOCAL_MACHINE, KOTS_REG, "EnableHTML", spref->bhtml     );
		UTSetRegInt( HKEY_LOCAL_MACHINE, KOTS_REG, "SplitLvls" , spref->bsplit    );
		UTSetRegInt( HKEY_LOCAL_MACHINE, KOTS_REG, "PerPage"   , spref->iperpage  );
		UTSetRegInt( HKEY_LOCAL_MACHINE, KOTS_REG, "NumPages"  , spref->inumpages );

		UTSetRegValue( HKEY_LOCAL_MACHINE, KOTS_REG, "DataDir", spref->sdatadir );
		UTSetRegValue( HKEY_LOCAL_MACHINE, KOTS_REG, "OutFile", spref->soutdir  );
		UTSetRegValue( HKEY_LOCAL_MACHINE, KOTS_REG, "HTML"   , spref->shtml    );

		UTSetRegValue( HKEY_LOCAL_MACHINE, KOTS_REG, "FTPServer", spref->sserver );
		UTSetRegValue( HKEY_LOCAL_MACHINE, KOTS_REG, "FTPUser"  , spref->suser   );
		UTSetRegValue( HKEY_LOCAL_MACHINE, KOTS_REG, "FTPPass"  , spref->spass   );
		UTSetRegValue( HKEY_LOCAL_MACHINE, KOTS_REG, "FTPDir"   , spref->sdir    );
	}
}

//*************************************************************************************************************
//*************************************************************************************************************
// Function: UTGetRegInt
//*************************************************************************************************************
//*************************************************************************************************************

BOOL UTGetRegInt( HKEY start, LPCTSTR key, LPCSTR value, int &val )
{
	BOOL    bRes;
	CString result;

	bRes = UTGetRegValue( start, key, value, result );

	val = atoi( result );

	return bRes;
}

//*************************************************************************************************************
//*************************************************************************************************************
// Function: UTSetRegInt
//*************************************************************************************************************
//*************************************************************************************************************

BOOL UTSetRegInt( HKEY start, LPCTSTR keys, LPCSTR value, int val )
{
	BOOL    bRes;
	CString result;

	result.Format( "%d", val );

	bRes = UTSetRegValue( start, keys, value, result );

	return bRes;
}

//*************************************************************************************************************
//*************************************************************************************************************
// Function: UTGetRegValue
//*************************************************************************************************************
//*************************************************************************************************************

BOOL UTGetRegValue( HKEY start, LPCTSTR key, LPCSTR value, CString &result )
{
	BOOL    bRes   = FALSE;
	HKEY    hKey;
	LONG    retval;
	DWORD   type;
	DWORD   size   = MAX_PATH;
	TCHAR   buff   [ MAX_PATH ];
	CString str;

	result.Empty();

	str = key;

	::ZeroMemory( buff, sizeof buff );

	retval = RegOpenKeyEx( start, str, 0, KEY_READ, &hKey );
	
	if ( retval == ERROR_SUCCESS )
	{
		if ( RegQueryValueEx( hKey, value, NULL, &type, (LPBYTE)buff, &size ) == ERROR_SUCCESS )
			bRes = TRUE;

		RegCloseKey( hKey );

		result = buff;
	}
	return bRes;
}

//*************************************************************************************************************
//*************************************************************************************************************
// Function: UTSetRegValue
//*************************************************************************************************************
//*************************************************************************************************************

BOOL UTSetRegValue( HKEY start, LPCTSTR keys, LPCSTR value, LPCTSTR result )
{
	BOOL bRes = FALSE;
	HKEY hKey;

	if ( RegCreateKey( start, keys, &hKey ) == ERROR_SUCCESS )
	{
		if ( lstrlen( result ) < 1 )
			RegDeleteValue( hKey, value );
		else
		{
			if ( RegSetValueEx( hKey, value, NULL, REG_SZ, (const BYTE *)result, lstrlen( result ) ) == ERROR_SUCCESS )
				bRes = TRUE;
		}
		RegFlushKey( hKey );

		RegCloseKey( hKey );
	}
	return bRes;
}

//*************************************************************************************************************
//*************************************************************************************************************
// Function: UTDoFolderDlg
//
//  defined in shlobj.h
//
//	BIF_BROWSEFORCOMPUTER Only returns computers. If the user selects anything other than a computer, 
//                        the OK button is grayed.
//  BIF_BROWSEFORPRINTER  Only returns printers. If the user selects anything other than a printer, 
//                        the OK button is grayed.
//  BIF_DONTGOBELOWDOMAIN Does not include network folders below the domain level in the tree view control.
//  BIF_RETURNFSANCESTORS Only returns file system ancestors. If the user selects anything other than a file 
//                        system ancestor, the OK button is grayed.
//  BIF_RETURNONLYFSDIRS  Only returns file system directories. If the user selects folders that are not part
//                        of the file system, the OK button is grayed.
//  BIF_STATUSTEXT        Includes a status area in the dialog box. The callback function can set the status 
//                        text by sending messages to the dialog box.
//
//*************************************************************************************************************
//*************************************************************************************************************

BOOL UTDoFolderDlg( LPCTSTR title, CString &buff )
{
  char         pszBuffer[ MAX_PATH ];
	UINT         flags;
	BOOL         bRes;
  LPMALLOC     pMalloc;
  BROWSEINFO   bi;
  LPITEMIDLIST pidl;

	bRes = FALSE;

	flags = BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS;

	if ( ::SHGetMalloc( &pMalloc ) == NOERROR )
	{
		bi.hwndOwner      = AfxGetMainWnd()->GetSafeHwnd();
		bi.pidlRoot       = NULL;
		bi.pszDisplayName = pszBuffer;
		bi.lpszTitle      = _T( title );
		bi.ulFlags        = flags;
		bi.lpfn           = NULL;
		bi.lParam         = 0;

		if ( ( pidl = ::SHBrowseForFolder( &bi ) ) != NULL )
		{
			if ( ::SHGetPathFromIDList( pidl, pszBuffer ) )
			{ 
				buff = pszBuffer;

				bRes = TRUE;
			}
			pMalloc->Free( pidl );
		}
		pMalloc->Release();
  }
	return bRes;
}

//*************************************************************************************************************
//*************************************************************************************************************
//*************************************************************************************************************
//*************************************************************************************************************

