// KotsAdmin.h : main header file for the KOTSADMIN application
//

#if !defined(AFX_KOTSADMIN_H__3CC44753_F10F_11D2_9FD5_0060972CE413__INCLUDED_)
#define AFX_KOTSADMIN_H__3CC44753_F10F_11D2_9FD5_0060972CE413__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CKotsAdminApp:
// See KotsAdmin.cpp for the implementation of this class
//

class CKotsAdminApp : public CWinApp
{
public:
	CKotsAdminApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CKotsAdminApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CKotsAdminApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_KOTSADMIN_H__3CC44753_F10F_11D2_9FD5_0060972CE413__INCLUDED_)
