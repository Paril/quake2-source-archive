//*************************************************************************************************************
//*************************************************************************************************************
// File: dabout.h
//*************************************************************************************************************
//*************************************************************************************************************

#ifndef __DABOUT_H__
#define __DABOUT_H__

class CDIB;
class CDIBPal;

//*************************************************************************************************************
//*************************************************************************************************************
// Class: CDAbout
//*************************************************************************************************************
//*************************************************************************************************************

class CDAbout : public CDialog
{
	private:
		int    m_IDD;
		UINT   m_uBitmap;
		CBrush *m_pBrush;

		CDIB    *m_pDIB;
		CRect   m_rcDIBFrame;
		CDIBPal *m_pDIBPal;

	public:
		CDAbout ( CWnd *pParent = NULL );
		~CDAbout();

		//{{AFX_DATA(CDAbout)
		enum { IDD = IDD_ABOUTBOX };
		//}}AFX_DATA

		void DoModeless();

	protected:
		//{{AFX_VIRTUAL(CDAbout)
		virtual void DoDataExchange( CDataExchange *pDX );
		//}}AFX_VIRTUAL

	protected:
		//{{AFX_MSG(CDAbout)
		afx_msg void OnPaint();
		afx_msg BOOL OnQueryNewPalette();
		//}}AFX_MSG
		virtual BOOL   OnInitDialog();
		afx_msg void   OnKeyUp    ( UINT nChar, UINT nRepCnt, UINT nFlags );
		afx_msg void   OnLButtonUp( UINT nFlags, CPoint point );
		afx_msg HBRUSH OnCtlColor ( CDC *pDC, CWnd* pWnd, UINT nCtlColor );
		DECLARE_MESSAGE_MAP()
};
#endif

//*************************************************************************************************************
//*************************************************************************************************************
//*************************************************************************************************************
//*************************************************************************************************************

