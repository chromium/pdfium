	// ReaderVCDoc.h : interface of the CReaderVCDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_READERVCDOC_H__6E0B9799_0661_4DB3_9E40_FB2D17F0D5EB__INCLUDED_)
#define AFX_READERVCDOC_H__6E0B9799_0661_4DB3_9E40_FB2D17F0D5EB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CReaderVCDoc : public CDocument
{
protected: // create from serialization only
	CReaderVCDoc();
	DECLARE_DYNCREATE(CReaderVCDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CReaderVCDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	//}}AFX_VIRTUAL

// Implementation
public:
	CString m_strPDFName;
	virtual ~CReaderVCDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CReaderVCDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_READERVCDOC_H__6E0B9799_0661_4DB3_9E40_FB2D17F0D5EB__INCLUDED_)
