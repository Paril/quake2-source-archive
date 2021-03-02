//*************************************************************************************************************
//*************************************************************************************************************
// File: dabout.cpp
//*************************************************************************************************************
//*************************************************************************************************************

#include "stdafx.h"
#include "resource.h"

#include "dabout.h"

#include "dib.h"
#include "dibpal.h"

//*************************************************************************************************************
//*************************************************************************************************************
// Function: Construction
//*************************************************************************************************************
//*************************************************************************************************************

CDAbout::CDAbout( CWnd *pParent ) : CDialog( CDAbout::IDD, pParent )
{
	m_pBrush = new CBrush( RGB( 0, 0, 0 ) );

	m_uBitmap = IDB_SPLASH;

	m_pDIB    = NULL;
	m_pDIBPal = NULL;

	//{{AFX_DATA_INIT(CDAbout)
	//}}AFX_DATA_INIT
}

CDAbout::~CDAbout()
{
	if ( m_hWnd )
		DestroyWindow();

	m_pBrush->DeleteObject();
	delete m_pBrush;

	delete m_pDIB;   
	delete m_pDIBPal;
}

//*************************************************************************************************************
//*************************************************************************************************************
// Function: DoDataExchange
//*************************************************************************************************************
//*************************************************************************************************************

void CDAbout::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDAbout)
	//}}AFX_DATA_MAP
}

//*************************************************************************************************************
//*************************************************************************************************************
// Function: MessageMap
//*************************************************************************************************************
//*************************************************************************************************************

BEGIN_MESSAGE_MAP(CDAbout, CDialog)
	//{{AFX_MSG_MAP(CDAbout)
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_KEYUP()
	ON_WM_CTLCOLOR()
	ON_WM_LBUTTONUP()
	ON_WM_QUERYNEWPALETTE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//*************************************************************************************************************
//*************************************************************************************************************
// Function: DoModeless
//*************************************************************************************************************
//*************************************************************************************************************

void CDAbout::DoModeless()
{
	CDialog::Create( m_IDD );
}

//*************************************************************************************************************
//*************************************************************************************************************
// Function: OnInitDialog
//*************************************************************************************************************
//*************************************************************************************************************

BOOL CDAbout::OnInitDialog() 
{
	int     x;
	CWnd    *pDIBFrame;
	CRect   rect;
	CString result;

	CStringArray array;

	CDialog::OnInitDialog();

	m_pDIB    = new CDIB;
	m_pDIBPal = new CDIBPal;

	GetClientRect( rect );

	if ( m_pDIB->Load( (USHORT)m_uBitmap ) )
	{
		m_pDIBPal->Create( m_pDIB );

		pDIBFrame = GetDlgItem( IDC_BITPLACE );
	
		pDIBFrame->GetWindowRect( &m_rcDIBFrame );

		ScreenToClient( &m_rcDIBFrame );

		m_rcDIBFrame.right  = m_rcDIBFrame.left + m_pDIB->GetWidth ();
		m_rcDIBFrame.bottom = m_rcDIBFrame.top  + m_pDIB->GetHeight();

		x = ( rect.Width() - m_rcDIBFrame.Width() ) / 2;

		m_rcDIBFrame.OffsetRect( x - m_rcDIBFrame.left, 0 );
	}
	return TRUE;
}

//*************************************************************************************************************
//*************************************************************************************************************
// Function: OnPaint
//*************************************************************************************************************
//*************************************************************************************************************

void CDAbout::OnPaint()
{
	CPaintDC dc(this);

	if ( m_pDIB && m_pDIBPal )
	{
		CRect rcClip;
		dc.GetClipBox(&rcClip);

		if (!(rcClip & m_rcDIBFrame).IsRectEmpty())
		{
		
			CPalette *pPalOld = dc.SelectPalette(m_pDIBPal, FALSE);
			dc.RealizePalette();
			
			int nOldMode = dc.SetStretchBltMode(COLORONCOLOR);
			m_pDIB->Draw(&dc, m_rcDIBFrame.left, m_rcDIBFrame.top, m_rcDIBFrame.Width(), m_rcDIBFrame.Height());
			dc.SetStretchBltMode(nOldMode);
			dc.SelectPalette(pPalOld, FALSE);
		}
	}
}

//*************************************************************************************************************
//*************************************************************************************************************
// Function: OnCtlColor
//*************************************************************************************************************
//*************************************************************************************************************

HBRUSH CDAbout::OnCtlColor( CDC *pDC, CWnd *pWnd, UINT nCtlColor ) 
{
	HBRUSH hbr = CDialog::OnCtlColor( pDC, pWnd, nCtlColor );

	switch ( nCtlColor )
	{
		case CTLCOLOR_STATIC:
	 		pDC->SetBkMode( TRANSPARENT );
		case CTLCOLOR_DLG:
			hbr = (HBRUSH)m_pBrush->m_hObject;
			break;
		case CTLCOLOR_BTN:
		case CTLCOLOR_EDIT:
		case CTLCOLOR_LISTBOX:
		case CTLCOLOR_MSGBOX:
		case CTLCOLOR_SCROLLBAR:
			break;
	}
	return hbr;
}

//*************************************************************************************************************
//*************************************************************************************************************
// Function: OnKeyUp
//*************************************************************************************************************
//*************************************************************************************************************

void CDAbout::OnKeyUp( UINT nChar, UINT nRepCnt, UINT nFlags ) 
{
	CDialog::OnKeyUp( nChar, nRepCnt, nFlags );

	OnOK();
}

//*************************************************************************************************************
//*************************************************************************************************************
// Function: OnLButtonUp
//*************************************************************************************************************
//*************************************************************************************************************

void CDAbout::OnLButtonUp( UINT nFlags, CPoint point ) 
{
	CDialog::OnLButtonUp( nFlags, point );

	OnOK();
}

//*************************************************************************************************************
//*************************************************************************************************************
// Function: OnQueryNewPalette
//*************************************************************************************************************
//*************************************************************************************************************

BOOL CDAbout::OnQueryNewPalette() 
{
	return CDialog::OnQueryNewPalette();
}

//*************************************************************************************************************
//*************************************************************************************************************
//*************************************************************************************************************
//*************************************************************************************************************

