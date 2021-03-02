//*************************************************************************************************************
//*************************************************************************************************************
// File: doptions.h
//*************************************************************************************************************
//*************************************************************************************************************

#ifndef __DOPTIONS_H__
#define __DOPTIONS_H__

//*************************************************************************************************************
//*************************************************************************************************************
// Struct: SPref
//*************************************************************************************************************
//*************************************************************************************************************

struct SPref
{
	SPref()
	{
		ichoice   = 0;
		iminutes  = 30;
		iperpage  = 100;
		inumpages = 0;
		ilevel    = 100;
		bftp		  = 0;
		bhtml		  = 0;
		bsplit	  = 0;

		shtml    = "";
		sdatadir = "";
		soutdir  = "";
	}
	int ilevel;
	int ichoice;
	int iminutes;
	int iperpage;
	int inumpages;

	int bftp;
	int bhtml;
	int bsplit;

	CString shtml;
	CString sdatadir;
	CString soutdir;

	CString sserver;
	CString suser;
	CString spass;
	CString sdir;
};

//*************************************************************************************************************
//*************************************************************************************************************
// Class: CDOptions
//*************************************************************************************************************
//*************************************************************************************************************

class CDOptions : public CDialog
{
	private:
		SPref *m_pref;

	public:
		CDOptions( SPref *spref, CWnd* pParent = NULL );

		//{{AFX_DATA(CDOptions)
		enum { IDD = IDD_OPTIONS };
		CSpinButtonCtrl	m_pagespin;
		CSpinButtonCtrl	m_spinpage;
		CSpinButtonCtrl	m_Spin;
		int		m_minutes;
		CString	m_datadir;
		CString	m_outdir;
		int		m_choice;
		CString	m_html;
		CString m_server;
		CString m_user;
		CString m_pass;
		CString m_dir;
		BOOL	m_bEFTP;
		BOOL	m_bEHTML;
		BOOL	m_bSplit;
		int m_perpage;
		int		m_numpages;
		int m_level;
		//}}AFX_DATA

	protected:
		//{{AFX_VIRTUAL(CDOptions)
		virtual void DoDataExchange(CDataExchange* pDX);
		//}}AFX_VIRTUAL

	protected:
		//{{AFX_MSG(CDOptions)
		virtual BOOL OnInitDialog();
		virtual void OnOK();
		afx_msg void OnDatadir();
		afx_msg void OnOutfile();
		afx_msg void OnEftp();
		afx_msg void OnEhtml();
		//}}AFX_MSG
		DECLARE_MESSAGE_MAP()
};
#endif

//*************************************************************************************************************
//*************************************************************************************************************
//*************************************************************************************************************
//*************************************************************************************************************

