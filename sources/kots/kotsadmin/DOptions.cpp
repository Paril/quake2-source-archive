//*************************************************************************************************************
//*************************************************************************************************************
// File: doptions.cpp
//*************************************************************************************************************
//*************************************************************************************************************

#include "stdafx.h"
#include "kotsadmin.h"

#include "DOptions.h"
#include "util.h"

//*************************************************************************************************************
//*************************************************************************************************************
// Function: Construction
//*************************************************************************************************************
//*************************************************************************************************************

CDOptions::CDOptions( SPref *spref, CWnd *pParent )	: 
	CDialog( CDOptions::IDD, pParent )
{
	m_pref = spref;

	//{{AFX_DATA_INIT(CDOptions)
	m_level    = m_pref->ilevel;
	m_minutes  = m_pref->iminutes;
	m_datadir  = m_pref->sdatadir;
	m_outdir   = m_pref->soutdir;
	m_choice   = m_pref->ichoice;
	m_html     = m_pref->shtml;
	m_server	 = m_pref->sserver;
	m_user  	 = m_pref->suser;
	m_pass  	 = m_pref->spass;
	m_dir   	 = m_pref->sdir;
	m_bEFTP    = m_pref->bftp;
	m_bEHTML   = m_pref->bhtml;
	m_bSplit   = m_pref->bsplit;
	m_perpage  = m_pref->iperpage;
	m_numpages = m_pref->inumpages;
	//}}AFX_DATA_INIT
}

//*************************************************************************************************************
//*************************************************************************************************************
// Function: DoDataExchange
//*************************************************************************************************************
//*************************************************************************************************************

void CDOptions::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDOptions)
	DDX_Control(pDX, IDC_SPIN4, m_pagespin);
	DDX_Control(pDX, IDC_SPIN3, m_spinpage);
	DDX_Control(pDX, IDC_SPIN1, m_Spin);
	DDX_Text(pDX, IDC_EDIT1, m_minutes);
	DDV_MinMaxInt(pDX, m_minutes, 1, 120);
	DDX_Text(pDX, IDC_EDIT2, m_datadir);
	DDX_Text(pDX, IDC_EDIT3, m_outdir);
	DDX_Radio(pDX, IDC_RADIO1, m_choice);
	DDX_Text(pDX, IDC_HTML, m_html);
	DDX_Text(pDX, IDC_EDIT4, m_server );
	DDX_Text(pDX, IDC_EDIT5, m_user ); 
	DDX_Text(pDX, IDC_EDIT6, m_pass ); 
	DDX_Text(pDX, IDC_EDIT7, m_dir );  
	DDX_Check(pDX, IDC_EFTP, m_bEFTP);
	DDX_Check(pDX, IDC_EHTML, m_bEHTML);
	DDX_Check(pDX, IDC_SPLIT, m_bSplit);
	DDX_Text(pDX, IDC_EDIT10, m_perpage);
	DDV_MinMaxInt(pDX, m_perpage, 10, 10000);
	DDX_Text(pDX, IDC_EDIT11, m_numpages);
	DDV_MinMaxInt(pDX, m_numpages, 0, 1000);
	//}}AFX_DATA_MAP
}

//*************************************************************************************************************
//*************************************************************************************************************
// Function: MessageMap
//*************************************************************************************************************
//*************************************************************************************************************

BEGIN_MESSAGE_MAP(CDOptions, CDialog)
	//{{AFX_MSG_MAP(CDOptions)
	ON_BN_CLICKED(IDC_DATADIR, OnDatadir)
	ON_BN_CLICKED(IDC_OUTFILE, OnOutfile)
	ON_BN_CLICKED(IDC_EFTP, OnEftp)
	ON_BN_CLICKED(IDC_EHTML, OnEhtml)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//*************************************************************************************************************
//*************************************************************************************************************
// Function: OnInitDialog
//*************************************************************************************************************
//*************************************************************************************************************

BOOL CDOptions::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_Spin.SetRange( 1, 120 );

	m_spinpage.SetRange( 10, 10000 );
	m_pagespin.SetRange( 0, 1000 );

	OnEhtml();
	OnEftp ();
	
	return TRUE;
}

//*************************************************************************************************************
//*************************************************************************************************************
// Function: OnOK
//*************************************************************************************************************
//*************************************************************************************************************

void CDOptions::OnOK() 
{
	if ( !UpdateData( TRUE ) )
		return;

	m_pref->ilevel    = m_level;
	m_pref->iminutes  = m_minutes;
	m_pref->sdatadir  = m_datadir;
	m_pref->soutdir   = m_outdir; 
	m_pref->ichoice	  = m_choice; 
	m_pref->shtml	    = m_html; 
	m_pref->iperpage  = m_perpage;
	m_pref->inumpages = m_numpages;

	m_pref->sserver	= m_server;
	m_pref->suser		= m_user;  
	m_pref->spass		= m_pass;  
	m_pref->sdir		= m_dir;   
	m_pref->bftp		=	m_bEFTP;
	m_pref->bhtml		=	m_bEHTML;
	m_pref->bsplit	=	m_bSplit;

	KOTSLoad( m_pref, false );

	CDialog::OnOK();
}

//*************************************************************************************************************
//*************************************************************************************************************
// Function: OnDatadir
//*************************************************************************************************************
//*************************************************************************************************************

void CDOptions::OnDatadir() 
{
	CString str;

	UpdateData( TRUE );

	if ( !UTDoFolderDlg( "Select kotsdata folder, usually quake2\\kots\\kotsdata", str ) )
		return;

	m_datadir = str + '\\';

	UpdateData( FALSE );
}

//*************************************************************************************************************
//*************************************************************************************************************
// Function: OnOutfile
//*************************************************************************************************************
//*************************************************************************************************************

void CDOptions::OnOutfile() 
{
	CString str;

	UpdateData( TRUE );

	if ( !UTDoFolderDlg( "Select folder to store generated HTML files", str ) )
		return;

	m_outdir = str + '\\';

	UpdateData( FALSE );
}

//*************************************************************************************************************
//*************************************************************************************************************
// Function: OnEftp
//*************************************************************************************************************
//*************************************************************************************************************

void CDOptions::OnEftp() 
{
	BOOL bEnable;
	
	UpdateData( TRUE );

	bEnable = ( m_bEFTP == TRUE );

	GetDlgItem( IDC_EDIT4 )->EnableWindow( bEnable );
	GetDlgItem( IDC_EDIT5 )->EnableWindow( bEnable ); 
	GetDlgItem( IDC_EDIT6 )->EnableWindow( bEnable ); 
	GetDlgItem( IDC_EDIT7 )->EnableWindow( bEnable );  
}

//*************************************************************************************************************
//*************************************************************************************************************
// Function: OnEhtml
//*************************************************************************************************************
//*************************************************************************************************************

void CDOptions::OnEhtml() 
{
	BOOL bEnable;
	
	UpdateData( TRUE );

	bEnable = ( m_bEHTML == TRUE );

	GetDlgItem( IDC_SPIN3   )->EnableWindow( bEnable );
	GetDlgItem( IDC_SPIN4   )->EnableWindow( bEnable );
	GetDlgItem( IDC_EDIT3   )->EnableWindow( bEnable ); 
	GetDlgItem( IDC_HTML    )->EnableWindow( bEnable );  
	GetDlgItem( IDC_EFTP    )->EnableWindow( bEnable );  
	GetDlgItem( IDC_SPLIT   )->EnableWindow( bEnable );  
	GetDlgItem( IDC_EDIT10  )->EnableWindow( bEnable ); 
	GetDlgItem( IDC_EDIT11  )->EnableWindow( bEnable ); 

	if ( !bEnable )
	{
		m_bEFTP = FALSE;

		UpdateData( FALSE );

		OnEftp();
	}
}

//*************************************************************************************************************
//*************************************************************************************************************
//*************************************************************************************************************
//*************************************************************************************************************

