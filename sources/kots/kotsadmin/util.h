//*************************************************************************************************************
//*************************************************************************************************************
// File: util.h
//*************************************************************************************************************
//*************************************************************************************************************

#ifndef __UTIL_H__
#define __UTIL_H__

struct SPref;

//*************************************************************************************************************
//*************************************************************************************************************
// Functions: 
//*************************************************************************************************************
//*************************************************************************************************************

void KOTSLoad( SPref *spref, bool bLoad );

BOOL UTGetRegInt  ( HKEY start, LPCTSTR key, LPCSTR value, int &val );
BOOL UTSetRegInt  ( HKEY start, LPCTSTR key, LPCSTR value, int val  );
BOOL UTGetRegValue( HKEY start, LPCTSTR key, LPCSTR value, CString &result );
BOOL UTSetRegValue( HKEY start, LPCTSTR key, LPCSTR value, LPCTSTR result  );

BOOL UTDoFolderDlg( LPCTSTR title, CString &buff );

//*************************************************************************************************************
//*************************************************************************************************************
// Class: CBuff
//*************************************************************************************************************
//*************************************************************************************************************

class CBuff
{
	public:
		CBuff () { m_buff = NULL; m_size = 0; }
		~CBuff() { delete [] m_buff; };

		char  *m_buff;
		DWORD m_size;

		void Size( DWORD size )
		{
			if ( m_size < size )
			{
				delete [] m_buff;
				m_buff = new char [ size ];
			}
		}
};
#endif

//*************************************************************************************************************
//*************************************************************************************************************
//*************************************************************************************************************
//*************************************************************************************************************
