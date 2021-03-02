//*************************************************************************************************************
//*************************************************************************************************************
// File: duser.cpp
//*************************************************************************************************************
//*************************************************************************************************************

#include "stdafx.h"
#include "KotsAdmin.h"

#include "DUser.h"

#include "../kuser/user.h"

//*************************************************************************************************************
//*************************************************************************************************************
// Function: Construction
//*************************************************************************************************************
//*************************************************************************************************************

CDUser::CDUser( LPCTSTR path, CUser *user, CWnd *pParent )	: CDialog( CDUser::IDD, pParent )
{
	m_user = new CUser;
 
 	memcpy( m_user, user, sizeof CUser );
	m_qpath = path;

	//{{AFX_DATA_INIT(CDUser)
	//}}AFX_DATA_INIT
}

CDUser::~CDUser()
{
	delete m_user;
}

//*************************************************************************************************************
//*************************************************************************************************************
// Function: DoDataExchange
//*************************************************************************************************************
//*************************************************************************************************************

void CDUser::DoDataExchange( CDataExchange* pDX )
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDUser)
	DDX_Control(pDX, IDC_LIST1, m_List);
	//}}AFX_DATA_MAP
}

//*************************************************************************************************************
//*************************************************************************************************************
// Function: MessageMap
//*************************************************************************************************************
//*************************************************************************************************************

BEGIN_MESSAGE_MAP(CDUser, CDialog)
	//{{AFX_MSG_MAP(CDUser)
	ON_BN_CLICKED(IDC_RESET, OnReset)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//*************************************************************************************************************
//*************************************************************************************************************
// Function: OnInitDialog
//*************************************************************************************************************
//*************************************************************************************************************

BOOL CDUser::OnInitDialog() 
{
	int y;
	int x;

	CString str;

	CDialog::OnInitDialog();

	m_List.InsertColumn(  0, "Variable", LVCFMT_LEFT , 100 );
	m_List.InsertColumn(  1, "Value"   , LVCFMT_RIGHT, 100 );

	y = 0;

	m_List.InsertItem( y++, "Name"  	 );
	m_List.InsertItem( y++, "Version"  );
	m_List.InsertItem( y++, "Level"    );
	m_List.InsertItem( y++, "Score"    );
	m_List.InsertItem( y++, "Points"   );
	m_List.InsertItem( y++, "Hole"     );
	m_List.InsertItem( y++, "Health"   );
	m_List.InsertItem( y++, "Armor"	   );
	m_List.InsertItem( y++, "Frags"		 );
	m_List.InsertItem( y++, "Deaths"	 );
	m_List.InsertItem( y++, "Ratio"	   );
	m_List.InsertItem( y++, "suicide"  );
	m_List.InsertItem( y++, "lastseen" );
	m_List.InsertItem( y++, "playtime" );
	m_List.InsertItem( y++, "blaster"	 );
	m_List.InsertItem( y++, "shot"   	 );
	m_List.InsertItem( y++, "sshot"  	 );
	m_List.InsertItem( y++, "mgun"		 );
	m_List.InsertItem( y++, "cgun"		 );
	m_List.InsertItem( y++, "gl"       );
	m_List.InsertItem( y++, "rl"	     );
	m_List.InsertItem( y++, "hyper"    );
	m_List.InsertItem( y++, "rgun"		 );
	m_List.InsertItem( y++, "bfg"			 );
	m_List.InsertItem( y++, "bullets"	 );
	m_List.InsertItem( y++, "shells"	 );
	m_List.InsertItem( y++, "cells"		 );
	m_List.InsertItem( y++, "grenades" );
	m_List.InsertItem( y++, "rockets"	 );
	m_List.InsertItem( y++, "slugs"		 );
	m_List.InsertItem( y++, "tballs"	 );

	x = 0;

	m_List.SetItemText( x++, 1, m_user->m_name  );

	FillInt( x++, m_user->m_version );
	FillInt( x++, m_user->Level() );
	FillInt( x++, m_user->Score() );
	FillInt( x++, m_user->m_points  );
	FillInt( x++, m_user->m_hole    );
	FillInt( x++, m_user->m_health  );
	FillInt( x++, m_user->m_barmor	);
	FillInt( x++, m_user->m_kills		);
	FillInt( x++, m_user->m_killed	);

	str.Format( "%.1f%%", m_user->Ratio() );

	m_List.SetItemText( x++, 1, str );

	FillInt( x++, m_user->m_suicide	);

	CTime ctime( m_user->m_lastseen );

	str = ctime.Format( "%m/%d/%Y %H:%M" );

	m_List.SetItemText( x++, 1, str );

	CTimeSpan cspan( m_user->m_playtime );

	str = cspan.Format( "%D %H:%M:%S" );

	m_List.SetItemText( x++, 1, str );

	FillInt( x++, m_user->m_blaster			 );
	FillInt( x++, m_user->m_shotgun			 );
	FillInt( x++, m_user->m_sshotgun		 );
	FillInt( x++, m_user->m_mgun				 );
	FillInt( x++, m_user->m_cgun				 );
	FillInt( x++, m_user->m_glauncher		 );
	FillInt( x++, m_user->m_rlauncher		 );
	FillInt( x++, m_user->m_hyperblaster );
	FillInt( x++, m_user->m_rgun				 );
	FillInt( x++, m_user->m_bfg					 );
	FillInt( x++, m_user->m_bullets			 );
	FillInt( x++, m_user->m_shells			 );
	FillInt( x++, m_user->m_cells				 );
	FillInt( x++, m_user->m_grenades		 );
	FillInt( x++, m_user->m_rockets			 );
	FillInt( x++, m_user->m_slugs				 );
	FillInt( x++, m_user->m_tballz			 );
		
	return TRUE;
}

//*************************************************************************************************************
//*************************************************************************************************************
// Function: FillInt
//*************************************************************************************************************
//*************************************************************************************************************

void CDUser::FillInt( int index, int value )
{
	CString str;

	str.Format( "%d", value );

	m_List.SetItemText( index, 1, str );
}

//*************************************************************************************************************
//*************************************************************************************************************
// Function: OnReset
//*************************************************************************************************************
//*************************************************************************************************************

void CDUser::OnReset() 
{
	UINT    res;
	CString str;

	str = "This will reset the user's password to 'changeme'.  Are you sure you want to do this?";

	res = AfxMessageBox( str, MB_YESNO | MB_ICONSTOP );

	if ( res != IDYES )
		return;

  lstrcpy( m_user->m_password, USER_DEFPASS );

	m_user->Save( m_qpath, false );
}

//*************************************************************************************************************
//*************************************************************************************************************
// Function: OnDelete
//*************************************************************************************************************
//*************************************************************************************************************

void CDUser::OnDelete() 
{
	char    path[ _MAX_PATH ];
	UINT    res;
	CString str;

	str = "This will DELETE this user.\nAre you sure you want to do this?";

	res = AfxMessageBox( str, MB_YESNO | MB_ICONSTOP );

	if ( res != IDYES )
		return;

	m_user->GetUserPath( m_qpath, m_user->m_name, path );

	::DeleteFile( path );

	EndDialog( 69 );
}

//*************************************************************************************************************
//*************************************************************************************************************
//*************************************************************************************************************
//*************************************************************************************************************

