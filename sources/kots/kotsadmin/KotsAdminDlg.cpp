//*************************************************************************************************************
//*************************************************************************************************************
// File: kotsadmindlg.cpp
//*************************************************************************************************************
//*************************************************************************************************************

#include "stdafx.h"
#include "KotsAdmin.h"

#include "KotsAdminDlg.h"
#include "dabout.h"
#include "duser.h"
#include "util.h"
#include "doptions.h"

#include "../kuser/user.h"
#include "../kuser/shared.h"

#define MARGIN 4

CPtrArray CKotsAdminDlg::m_users;

//*************************************************************************************************************
//*************************************************************************************************************
// Function: Construction
//*************************************************************************************************************
//*************************************************************************************************************

CKotsAdminDlg::CKotsAdminDlg( CWnd *pParent )	: 
	CDialog( CKotsAdminDlg::IDD, pParent )
{
	m_pref    = new SPref;
	m_bBusy   = false;
	m_uTimer  = 45;
	m_numlvls = 0;

	m_bFTP     = false;
	m_bHTML    = false;
	m_bLoading = false;

	//{{AFX_DATA_INIT(CKotsAdminDlg)
	//}}AFX_DATA_INIT
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CKotsAdminDlg::~CKotsAdminDlg()
{
	Cleanup();

	delete m_pref;
}

//*************************************************************************************************************
//*************************************************************************************************************
// Function: Cleanup
//*************************************************************************************************************
//*************************************************************************************************************

void CKotsAdminDlg::Cleanup()
{
	int   x;
	CUser *user;

	if ( ::IsWindow( m_List.m_hWnd ) )
		m_List.DeleteAllItems();

	for ( x = 0; x < m_users.GetSize(); x++ )
	{
		user = (CUser *)m_users[x];

		delete user;
	}
	m_users.RemoveAll();
}

//*************************************************************************************************************
//*************************************************************************************************************
// Function: DoDataExchange
//*************************************************************************************************************
//*************************************************************************************************************

void CKotsAdminDlg::DoDataExchange( CDataExchange *pDX )
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CKotsAdminDlg)
	DDX_Control(pDX, IDC_LIST1, m_List);
	//}}AFX_DATA_MAP
}

//*************************************************************************************************************
//*************************************************************************************************************
// Function: MessageMap
//*************************************************************************************************************
//*************************************************************************************************************

BEGIN_MESSAGE_MAP(CKotsAdminDlg, CDialog)
	//{{AFX_MSG_MAP(CKotsAdminDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SIZE()
	ON_BN_CLICKED(ID_ABOUT, OnAbout)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST1, OnColumnList)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, OnDblclkList1)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_OPTIONS, OnOptions)
	ON_BN_CLICKED(IDC_UPDATE, OnUpdate)
	ON_WM_RBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//*************************************************************************************************************
//*************************************************************************************************************
// Function: OnInitDialog
//*************************************************************************************************************
//*************************************************************************************************************

BOOL CKotsAdminDlg::OnInitDialog()
{
	int     x;
	CRect   rect;
	CMenu   *pSysMenu;
	CString str;
	CString path;

	CFileStatus stat;

	CDialog::OnInitDialog();

	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	pSysMenu = GetSystemMenu( FALSE );

	if ( pSysMenu != NULL )
	{
		str.LoadString( IDS_ABOUTBOX );

		if ( !str.IsEmpty() )
		{
			pSysMenu->AppendMenu( MF_SEPARATOR );
			pSysMenu->AppendMenu( MF_STRING, IDM_ABOUTBOX, str );
		}
	}
	SetIcon( m_hIcon, TRUE  );
	SetIcon( m_hIcon, FALSE );

	ResetTimer();

	SetDlgItemText( IDC_STATUS, "Startup..." );

	KOTSLoad( m_pref, true );

	x = 0;

	m_List.InsertColumn( x++, "Player"  , LVCFMT_LEFT , 100 );
	m_List.InsertColumn( x++, "Level"   , LVCFMT_RIGHT, 40 );
	m_List.InsertColumn( x++, "Score"   , LVCFMT_RIGHT, 50 );
	m_List.InsertColumn( x++, "Rank"    , LVCFMT_RIGHT, 50 );
	m_List.InsertColumn( x++, "Frags"		, LVCFMT_RIGHT, 40 );
	m_List.InsertColumn( x++, "Deaths"	, LVCFMT_RIGHT, 50 );
	m_List.InsertColumn( x++, "Ratio"	  , LVCFMT_RIGHT, 50 );
	m_List.InsertColumn( x++, "suicide" , LVCFMT_RIGHT, 50 );
	m_List.InsertColumn( x++, "lastseen", LVCFMT_RIGHT, 100 );
	m_List.InsertColumn( x++, "playtime", LVCFMT_RIGHT, 80 );
	m_List.InsertColumn( x++, "start"   , LVCFMT_RIGHT, 80 );

	GetClientRect( rect );

	OnSize( 0, rect.Width(), rect.Height() );

	Publish( true );

	m_uTimer = SetTimer( m_uTimer, 1000, NULL );

	return TRUE;
}

//*************************************************************************************************************
//*************************************************************************************************************
// Function: OnOK
//*************************************************************************************************************
//*************************************************************************************************************

void CKotsAdminDlg::OnCancel() { OnOK(); }

void CKotsAdminDlg::OnOK()
{
	if ( m_bBusy )
	{
		AfxMessageBox( "KOTSAdmin is currently updating.  Please try again later." );
		return;
	}

	if ( m_uTimer )
		KillTimer( m_uTimer );

	CDialog::OnOK();
}

//*************************************************************************************************************
//*************************************************************************************************************
// Function: FillInt
//*************************************************************************************************************
//*************************************************************************************************************

void CKotsAdminDlg::FillInt( int index, int sub, int value )
{
	CString str;

	str.Format( "%d", value );

	m_List.SetItemText( index, sub, str );
}

//*************************************************************************************************************
//*************************************************************************************************************
// Function: FillList
//*************************************************************************************************************
//*************************************************************************************************************

void CKotsAdminDlg::FillList()
{
	int     x;
	int     y;
	int     i;
	CUser   *user;
	CString str;

	for ( y = 0; y < m_users.GetSize(); y++ )
	{
		user = (CUser *)m_users[y];

		i = m_List.InsertItem( y, user->m_name );

		m_List.SetItemData( i, y );

		x = 1;

		FillInt( i, x++, user->Level()  );
		FillInt( i, x++, user->Score()  );
		FillInt( i, x++, y + 1          );
		FillInt( i, x++, user->m_kills		);
		FillInt( i, x++, user->m_killed		);

		str.Format( "%.1f%%", user->Ratio() );

		m_List.SetItemText( i, x++, str );

		FillInt( i, x++, user->m_suicide	);

		CTime ctime( user->m_lastseen );

		str = ctime.Format( "%m/%d/%Y %H:%M" );

		m_List.SetItemText( i, x++, str );

		CTimeSpan cspan( user->m_playtime );

		str = cspan.Format( "%D %H:%M:%S" );

		m_List.SetItemText( i, x++, str );

		CTime ttime( user->m_firstseen );

		str = ttime.Format( "%m/%d/%Y %H:%M:%S" );

		m_List.SetItemText( i, x++, str );
	}
}

//*************************************************************************************************************
//*************************************************************************************************************
// Function: CheckKings
//*************************************************************************************************************
//*************************************************************************************************************

void CKotsAdminDlg::CheckKings( CUser *user )
{
	if ( m_kfrag == NULL )
	{
		m_kfrag = user;
		m_kdeath = user;
		m_ksuicide = user;
		m_kplaytime = user;
		m_khole = user;
		m_kspree = user;
		m_kspreecount = user;
		m_kspreewar = user;
		m_kspreebroke = user;
		m_kspreewarbroke = user;
		m_k2fer = user;
		m_kteles = user;
		m_kfirstseen = user;
	}

	if ( user->m_kills > m_kfrag->m_kills )
		m_kfrag = user;

	if ( user->m_killed > m_kdeath->m_killed )
		m_kdeath = user;

	if ( user->m_suicide > m_ksuicide->m_suicide )
		m_ksuicide = user;

	if ( user->m_playtime > m_kplaytime->m_playtime )
		m_kplaytime = user;

	if ( user->m_hole > m_khole->m_hole )
		m_khole = user;

	if ( user->m_spree > m_kspree->m_spree )
		m_kspree = user;

	if ( user->m_spreecount > m_kspreecount->m_spreecount )
		m_kspreecount = user;

	if ( user->m_spreewar > m_kspreewar->m_spreewar )
		m_kspreewar = user;

	if ( user->m_spreebroke > m_kspreebroke->m_spreebroke )
		m_kspreebroke = user;

	if ( user->m_spreewarbroke > m_kspreewarbroke->m_spreewarbroke )
		m_kspreewarbroke = user;

	if ( user->m_2fer > m_k2fer->m_2fer )
		m_k2fer = user;

	if ( user->m_teles > m_kteles->m_teles )
		m_kteles = user;

	if ( user->m_firstseen < m_kfirstseen->m_firstseen )
		m_kfirstseen = user;

	if ( user->m_playtime > 0 )
	{
		if ( m_kcheetah == NULL )
			m_kcheetah = user;
		else
		{
			if ( user->KillsPerMin() > m_kcheetah->KillsPerMin() )
				m_kcheetah = user;
		}
	}

	if ( user->Level() > 1 )
	{
		if ( m_kratio == NULL )
			m_kratio = user;
		else
		{
			if ( user->Ratio() > m_kratio->Ratio() )
				m_kratio = user;
		}
	}
}

//*************************************************************************************************************
//*************************************************************************************************************
// Function: LoadPlayers
//*************************************************************************************************************
//*************************************************************************************************************

void CKotsAdminDlg::LoadPlayers()
{
	int       x;
	UINT      res;
	BOOL      bOK = TRUE;
	CUser     *oser;
	CUser     *user;
	HANDLE    hSearch;
	CString   path;
	CString   file;

	WIN32_FIND_DATA fdata;

	path  = m_pref->sdatadir;
	path += "*.kots";

	m_khole			 = NULL;
	m_kfrag			 = NULL;
	m_kteles		 = NULL;
	m_kdeath		 = NULL;
	m_kratio		 = NULL;
	m_kcheetah   = NULL;
	m_ksuicide	 = NULL;
	m_kplaytime	 = NULL;
	m_kfirstseen = NULL;

	m_kspree				 = NULL;
	m_kspreecount		 = NULL;
	m_kspreewar			 = NULL;
	m_kspreebroke		 = NULL;
	m_kspreewarbroke = NULL;
	m_k2fer					 = NULL;

	memset( &fdata, 0, sizeof WIN32_FIND_DATA );

	hSearch = ::FindFirstFile( path, &fdata );
  
	while ( hSearch != INVALID_HANDLE_VALUE && bOK )
	{
		file = fdata.cFileName;

		file = m_pref->sdatadir + file;

		user = new CUser;

		res = user->Load( file, NULL );

		if ( res != KOTS_SUCCESS )
			delete user;
		else if ( user->m_bBossFlag )
			delete user;
		else if ( user->m_lastseen == 0 )
		{
			delete user;

			::DeleteFile( file );
		}
		else
		{
		 	CheckKings( user );
		 	
		 	if ( m_users.GetSize() < 1 )
		 		m_users.Add( user );
		 	else
		 	{
		 		for ( x = 0; x < m_users.GetSize(); x++ )
		 		{
		 			oser = (CUser *)m_users[x];

		 			if ( user->Score() > oser->Score() )
		 			{
		 				m_users.InsertAt( x, user );
		 				user = NULL;
		 				break;
		 			}
		 		}
		 		if ( user )
		 			m_users.Add( user );
		 	}
		}
		bOK = ::FindNextFile( hSearch, &fdata );
	}

	if ( hSearch != INVALID_HANDLE_VALUE )
		::FindClose( hSearch );
}

//*************************************************************************************************************
//*************************************************************************************************************
// Function: OnSysCommand
//*************************************************************************************************************
//*************************************************************************************************************

void CKotsAdminDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
		OnAbout();
	else
		CDialog::OnSysCommand(nID, lParam);
}

//*************************************************************************************************************
//*************************************************************************************************************
// Function: OnPaint
//*************************************************************************************************************
//*************************************************************************************************************

void CKotsAdminDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//*************************************************************************************************************
//*************************************************************************************************************
// Function: OnQueryDragIcon
//*************************************************************************************************************
//*************************************************************************************************************

HCURSOR CKotsAdminDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

//*************************************************************************************************************
//*************************************************************************************************************
// Function: OnSize
//*************************************************************************************************************
//*************************************************************************************************************

void CKotsAdminDlg::OnSize( UINT, int cx, int cy ) 
{
	int   x;
	CRect rect;

	if ( !m_hWnd )
		return;
		
	if ( !GetDlgItem( IDC_LIST1 ) )
		return;

	GetDlgItem( ID_ABOUT )->GetWindowRect( rect );

	ScreenToClient( rect );

	x = cx - rect.Width() - MARGIN;

	rect.OffsetRect( x - rect.left, MARGIN - rect.top );

	GetDlgItem( ID_ABOUT )->MoveWindow( rect );

	rect.OffsetRect( 0, rect.Height() + MARGIN );

	GetDlgItem( IDC_OPTIONS )->MoveWindow( rect );

	rect.OffsetRect( 0, rect.Height() + MARGIN );

	GetDlgItem( IDC_UPDATE )->MoveWindow( rect );

	rect.OffsetRect( 0, rect.Height() + MARGIN );

	GetDlgItem( IDOK )->MoveWindow( rect );

	rect.OffsetRect( 0, rect.Height() + MARGIN );

	rect.bottom = rect.top + 2 * rect.Height();

	GetDlgItem( IDC_STATUS )->MoveWindow( rect );

	rect.bottom = cy - MARGIN;
	rect.right  = rect.left - MARGIN;
	rect.left   = MARGIN;
	rect.top    = MARGIN;

	GetDlgItem( IDC_LIST1 )->MoveWindow( rect );

	Invalidate();
}

//*************************************************************************************************************
//*************************************************************************************************************
// Function: OnAbout
//*************************************************************************************************************
//*************************************************************************************************************

void CKotsAdminDlg::OnAbout() 
{
	CDAbout dlg;

	dlg.DoModal();
}

//*************************************************************************************************************
//*************************************************************************************************************
// Function: CompareFunc
//*************************************************************************************************************
//*************************************************************************************************************

int CALLBACK CKotsAdminDlg::CompareFunc( LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort )
{
	int   sub    = (int)lParamSort;
	long  val1;
	long  val2;
	CUser *user1 = (CUser *)m_users[ lParam1 ];
	CUser *user2 = (CUser *)m_users[ lParam2 ];

	if ( !user1 || !user2 )
		return 0;

	switch ( sub )
	{
		case 0:
			return lstrcmp( user1->m_name, user2->m_name );
		case 1:
			val1 = user1->Level();
			val2 = user2->Level();
			break;
		case 2:
			val1 = user1->Score();
			val2 = user2->Score();
			break;
		case 3:
			val1 = lParam1;
			val2 = lParam2;

			if ( val1 < val2 )
				return -1;
			else if ( val1 > val2 )
				return 1;

			break;
		case 4:
			val1 = user1->m_kills;
			val2 = user2->m_kills;
			break;
		case 5:
			val1 = user1->m_killed;
			val2 = user2->m_killed;
			break;
		case 6:
			val1 = (long)user1->Ratio();
			val2 = (long)user2->Ratio();
			break;
		case 7:
			val1 = user1->m_suicide;
			val2 = user2->m_suicide;
			break;
		case 8:
			val1 = user1->m_lastseen;
			val2 = user2->m_lastseen;
			break;
		case 9:
			val1 = user1->m_playtime;
			val2 = user2->m_playtime;
			break;
		case 10:
			val1 = user1->m_firstseen;
			val2 = user2->m_firstseen;
			break;
		default:
			val1 = 0;
			val2 = 0;
			break;				
	}

	if ( val1 > val2 )
		return -1;
	else if ( val1 < val2 )
		return 1;

	return 0;
}

//*************************************************************************************************************
//*************************************************************************************************************
// Function: OnColumnList
//*************************************************************************************************************
//*************************************************************************************************************

void CKotsAdminDlg::OnColumnList( NMHDR *pNMHDR, LRESULT *pResult ) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	
	*pResult = 0;

	if ( pNMListView->iSubItem > 11 )
		return;

	m_List.SortItems( CompareFunc, pNMListView->iSubItem );
}

//********************************************************************************************
//********************************************************************************************
// Function: UTGetListSel
//********************************************************************************************
//********************************************************************************************

int CKotsAdminDlg::GetSel()
{
	int y;
	int flags = LVNI_SELECTED;

	y = m_List.GetNextItem( -1, flags );
	
	return y;
}

//*************************************************************************************************************
//*************************************************************************************************************
// Function: OnDblclkList1
//*************************************************************************************************************
//*************************************************************************************************************

void CKotsAdminDlg::OnDblclkList1( NMHDR *lParam, LRESULT *pResult ) 
{
	int   x;
	int   item;
	CUser *pUser;

  NMLISTVIEW *lpnmlv;

  lpnmlv = (NMLISTVIEW *)lParam;

	item = lpnmlv->iItem;
	
	if ( item < 0 )
		return;

	x = m_List.GetItemData( item );

	if ( x < 0 || x > m_users.GetSize() )
		return;

	pUser = (CUser *)m_users[x];	

	CDUser dlg( m_pref->sdatadir, pUser );

	if ( dlg.DoModal() == 69 )
		m_List.DeleteItem( item );
	
	*pResult = 0;
}

//*************************************************************************************************************
//*************************************************************************************************************
// Function: ResetTimer
//*************************************************************************************************************
//*************************************************************************************************************

void CKotsAdminDlg::ResetTimer() 
{
	time( &m_ltime );
}

//*************************************************************************************************************
//*************************************************************************************************************
// Function: Publish
//*************************************************************************************************************
//*************************************************************************************************************

UINT PubThread( LPVOID lpdata )
{
	CKotsAdminDlg *pDlg = (CKotsAdminDlg *)lpdata;

	pDlg->Cleanup();

	pDlg->m_bLoading = true;

	pDlg->LoadPlayers();
	pDlg->FillList   ();

	pDlg->m_bLoading = false;

	pDlg->m_bHTML = true;

	pDlg->OnHtml();

	pDlg->m_bHTML = false;

	pDlg->m_bFTP = true;

	pDlg->OnFtp();

	pDlg->m_bFTP = false;

	pDlg->m_bBusy = false;

	return 1;	
}

void CKotsAdminDlg::Publish( bool bForce ) 
{
	long ltime;

	time( &ltime );

	if ( !bForce )
		if ( ltime < m_ltime )
			return;

	m_ltime = ltime + 60 * m_pref->iminutes;

	if ( m_bBusy )
		return;

	m_bBusy = true;

	::AfxBeginThread( PubThread, this );
}

//*************************************************************************************************************
//*************************************************************************************************************
// Function: OnUpdate
//*************************************************************************************************************
//*************************************************************************************************************

void CKotsAdminDlg::OnUpdate()
{
	if ( m_bBusy )
	{
		AfxMessageBox( "KOTSAdmin is already updating." );
		return;
	}
	Publish( true );
}

//*************************************************************************************************************
//*************************************************************************************************************
// Function: OnTimer
//*************************************************************************************************************
//*************************************************************************************************************

void CKotsAdminDlg::OnTimer( UINT nIDEvent ) 
{
	CString str;
	CString atr;
	
	if ( nIDEvent != m_uTimer )
		return;

	if ( !m_bBusy )
		str = "Idle";
	else if ( m_bLoading )	
		str = "Loading Players...";
	else if ( m_bHTML )	
		str = "Generating HTML...";
	else if ( m_bFTP )	
		str = "FTP'ing HTML...";
	else
		str = "Updating...";

	GetDlgItemText( IDC_STATUS, atr );

	if ( str.Compare( atr ) )
		SetDlgItemText( IDC_STATUS, str );

	if ( m_pref->ichoice == 0 || m_bBusy )
		return;

	Publish();
}

//*************************************************************************************************************
//*************************************************************************************************************
// Function: OnOptions
//*************************************************************************************************************
//*************************************************************************************************************

void CKotsAdminDlg::OnOptions() 
{
	CDOptions dlg( m_pref );

	if ( dlg.DoModal() == IDOK )
	{
		ResetTimer();

		if ( !m_bBusy )
			Publish( true );
	}
}

//*************************************************************************************************************
//*************************************************************************************************************
// Function: CleanName
//*************************************************************************************************************
//*************************************************************************************************************

void CKotsAdminDlg::CleanName( LPCTSTR name, CString &clean ) 
{
	int i;

	clean.Empty();

	for ( i = 0; i < lstrlen( name ); i++ )
	{
		if ( name[i] == '<' )
			clean += "&lt;";
		else
			clean += name[i];
	}
}

//*************************************************************************************************************
//*************************************************************************************************************
// Function: OnRButtonUp
//*************************************************************************************************************
//*************************************************************************************************************

void CKotsAdminDlg::OnRButtonUp( UINT nFlags, CPoint point ) 
{
	int   x;
	int   i;
	CUser *auser;
	CUser *buser;

	if ( nFlags & MK_CONTROL )
	{
		m_List.SortItems( CompareFunc, 11 );

		for ( x = 0; x < m_List.GetItemCount() - 2; x++ )
		{
			i = m_List.GetItemData( x );

			if ( i < 0 || i > m_users.GetSize() )
				continue;

			auser = (CUser *)m_users[i];	

			i = m_List.GetItemData( x + 1 );

			if ( i < 0 || i > m_users.GetSize() )
				continue;

			buser = (CUser *)m_users[i];	

			if ( auser->m_firstseen == buser->m_firstseen )
				m_List.SetItemState( x, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED );
		}
	}
	CDialog::OnRButtonUp(nFlags, point);
}

//*************************************************************************************************************
//*************************************************************************************************************
//*************************************************************************************************************
//*************************************************************************************************************

