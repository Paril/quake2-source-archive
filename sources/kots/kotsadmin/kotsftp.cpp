//*************************************************************************************************************
//*************************************************************************************************************
// File: kotsftp.cpp
//*************************************************************************************************************
//*************************************************************************************************************

#include "stdafx.h"
#include "kotsadmin.h"

#include "kotsadmindlg.h"
#include "doptions.h"
#include "util.h"

//*************************************************************************************************************
//*************************************************************************************************************
// Function: SendFile
//*************************************************************************************************************
//*************************************************************************************************************

void SendFile( LPCTSTR path, LPCTSTR name, CFtpConnection *cftp, CBuff &cbuff )
{
	DWORD size;
	CFile file;

	CInternetFile *cfile = NULL;

	if ( !file.Open( path, CFile::modeRead ) )
		return;

	size = file.GetLength();

	if ( size < 1 )
	{
		file.Close();

		return;
	}
	cbuff.Size( size );

	file.ReadHuge( cbuff.m_buff, size );
	
	file.Close();	

	cfile = cftp->OpenFile( name, GENERIC_WRITE );

	if ( !cfile )
		return;

	cfile->Write( cbuff.m_buff, size );

	cfile->Close();

	delete cfile;
}

//*************************************************************************************************************
//*************************************************************************************************************
// Function: KOTSFtp
//*************************************************************************************************************
//*************************************************************************************************************

void CKotsAdminDlg::OnFtp()
{
	int     x;
	CBuff   cbuff;
	CFile   file;
	CString stat;
	CString sfile;

  INTERNET_PORT  nPort = 21;
	CFtpConnection *cftp = NULL;

	if ( m_pref->soutdir.IsEmpty() || m_pref->sserver.IsEmpty() || !m_pref->bftp ||
	     !m_pref->bhtml || m_pref->suser.IsEmpty() )
		return;

	if ( m_users.GetSize() <= 0 )
		return;

	try
	{
		CInternetSession iconn( "Norb's Session" );

		cftp = iconn.GetFtpConnection( m_pref->sserver, m_pref->suser, m_pref->spass, 
		                               nPort );
		if ( !cftp )
		{
			iconn.Close();
		 
		 	return;
		}

		if ( !m_pref->sdir.IsEmpty() )
		{
			if ( !cftp->SetCurrentDirectory( m_pref->sdir ) )
			{
				delete cftp;

				iconn.Close();
		 
			 	return;
			}
		}
		sfile = m_pref->soutdir + RANKFILE;
	
		SendFile( sfile, RANKFILE, cftp, cbuff );

		sfile = m_pref->soutdir + KINGFILE;
	
		SendFile( sfile, KINGFILE, cftp, cbuff );

		for ( x = 0; x < m_numlvls; x++ )
		{
			stat.Format( LEVELFILE, x );

			sfile = m_pref->soutdir + stat;
			
			SendFile( sfile, stat, cftp, cbuff );
		}

		for ( x = 0; x < m_numstat; x++ )
		{
			stat.Format( PLAYERFILE, x );

			sfile = m_pref->soutdir + stat;
			
			SendFile( sfile, stat, cftp, cbuff );
		}
		delete cftp;

		iconn.Close();
	}
	catch (CInternetException* pEx)
	{
		pEx;
//		pEx->ReportError();
	}
}

//*************************************************************************************************************
//*************************************************************************************************************
//*************************************************************************************************************
//*************************************************************************************************************

