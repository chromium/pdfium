// ReaderVC.h : main header file for the READERVC application
//

#if !defined(AFX_READERVC_H__7873A15F_A60F_4C05_9DC6_C3914C224EAA__INCLUDED_)
#define AFX_READERVC_H__7873A15F_A60F_4C05_9DC6_C3914C224EAA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

class CReaderVCView;

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CReaderVCApp:
// See ReaderVC.cpp for the implementation of this class
//

class CReaderVCApp : public CWinApp
{
public:
	CReaderVCApp();
	~CReaderVCApp();
	CReaderVCView * m_pActiveView;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CReaderVCApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CReaderVCApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_READERVC_H__7873A15F_A60F_4C05_9DC6_C3914C224EAA__INCLUDED_)
