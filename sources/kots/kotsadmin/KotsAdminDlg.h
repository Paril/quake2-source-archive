//*************************************************************************************************************
//*************************************************************************************************************
// File: kotsadmindlg.h
//*************************************************************************************************************
//*************************************************************************************************************

#ifndef __KOTSADMINDLG_H__
#define __KOTSADMINDLG_H__

#define RANKFILE   "kotsdata.htm"
#define KINGFILE   "kotskings.htm"
#define LEVELFILE  "kotslv%d.htm"
#define PLAYERFILE "kotsp%d.htm"

class  CUser;
struct SPref;

//*************************************************************************************************************
//*************************************************************************************************************
// Class: CKotsAdminDlg
//*************************************************************************************************************
//*************************************************************************************************************

class CKotsAdminDlg : public CDialog
{
	private:
		int   m_numlvls;
		int   m_numstat;
		long  m_ltime;
		UINT  m_uTimer;
		SPref *m_pref;

		int  GetSel     ();
		void FillInt    ( int index, int sub, int value );
		void Publish    ( bool bForce = false );
		void CheckKings ( CUser *user );
		void ResetTimer ();

		void CleanName( LPCTSTR name, CString &clean );

		// Player Maxes
		CUser *m_khole;
		CUser *m_kfrag;
		CUser *m_kteles;
		CUser *m_kdeath;
		CUser *m_kratio;
		CUser *m_kcheetah;
		CUser *m_ksuicide;
		CUser *m_kplaytime;
		CUser *m_kfirstseen;

		CUser *m_kspree;
		CUser *m_kspreecount;
		CUser *m_kspreewar;
		CUser *m_kspreebroke;
		CUser *m_kspreewarbroke;
		CUser *m_k2fer;

	private: //kotshtml
		void StringToHex( LPCTSTR source, CString &target );

		void WriteHtmlHeader( CStdioFile &file, bool bKings = false );
		void WriteHtmlPlayer( CStdioFile &file, int index );
		void WriteHtmlFooter( CStdioFile &file, bool bMain, LPCTSTR last = NULL, LPCTSTR next = NULL );

		void WriteKingInfo( CStdioFile &file, LPCTSTR name, LPCTSTR data );

		bool PlayerHtml( int index );
	
	public:
		CKotsAdminDlg ( CWnd *pParent = NULL );
		~CKotsAdminDlg();

		//{{AFX_DATA(CKotsAdminDlg)
		enum { IDD = IDD_KOTSADMIN_DIALOG };
		CListCtrl	m_List;
		//}}AFX_DATA

		static CPtrArray m_users;

		static int CALLBACK CompareFunc( LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort );

		bool m_bFTP;
		bool m_bBusy;
		bool m_bHTML;
		bool m_bLoading;

		void OnFtp      ();
		void OnHtml     ();
		void OnKings    ();
		void Cleanup    ();
		void FillList   ();
		void LoadPlayers();

	protected:
		//{{AFX_VIRTUAL(CKotsAdminDlg)
		virtual void DoDataExchange(CDataExchange* pDX);
		//}}AFX_VIRTUAL

		HICON m_hIcon;

		//{{AFX_MSG(CKotsAdminDlg)
		virtual BOOL OnInitDialog();
		afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
		afx_msg void OnPaint();
		afx_msg void OnOK();
		afx_msg void OnCancel();
		afx_msg HCURSOR OnQueryDragIcon();
		afx_msg void OnSize(UINT nType, int cx, int cy);
		afx_msg void OnAbout();
		afx_msg void OnColumnList(NMHDR* pNMHDR, LRESULT* pResult);
		afx_msg void OnDblclkList1(NMHDR* pNMHDR, LRESULT* pResult);
		afx_msg void OnTimer(UINT nIDEvent);
		afx_msg void OnOptions();
		afx_msg void OnUpdate();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG
		DECLARE_MESSAGE_MAP()
};
#endif

//*************************************************************************************************************
//*************************************************************************************************************
//*************************************************************************************************************
//*************************************************************************************************************

