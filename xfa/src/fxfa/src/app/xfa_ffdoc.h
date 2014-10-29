// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FXFA_FORMFILLER_DOC_IMP_H
#define _FXFA_FORMFILLER_DOC_IMP_H
class CXFA_FFApp;
class CXFA_FFNotify;
class CXFA_FFDocView;
class IXFA_Locale;
typedef struct _FX_IMAGEDIB_AND_DPI {
    CFX_DIBSource* pDibSource;
    FX_INT32	   iImageXDpi;
    FX_INT32	   iImageYDpi;
} FX_IMAGEDIB_AND_DPI;
class CXFA_FFDoc : public CFX_Object
{
public:
    CXFA_FFDoc(CXFA_FFApp* pApp, IXFA_DocProvider* pDocProvider);
    ~CXFA_FFDoc();
    IXFA_DocProvider*		GetDocProvider()
    {
        return m_pDocProvider;
    }
    FX_DWORD				GetDocType();
    FX_INT32				StartLoad();
    FX_INT32				DoLoad(IFX_Pause *pPause = NULL);
    void					StopLoad();
    IXFA_DocView*			CreateDocView(FX_DWORD dwView = 0);
    FX_BOOL					OpenDoc(IFX_FileRead* pStream, FX_BOOL bTakeOverFile);
    FX_BOOL					OpenDoc(CPDF_Document* pPDFDoc);
    FX_BOOL					CloseDoc();
    void					SetDocType(FX_DWORD dwType);
    CXFA_Document*			GetXFADoc()
    {
        return m_pDocument;
    }
    CXFA_FFApp*				GetApp()
    {
        return m_pApp;
    }
    CXFA_FFDocView*			GetDocView(IXFA_DocLayout* pLayout);
    CXFA_FFDocView*			GetDocView();
    CPDF_Document*			GetPDFDoc();
    CFX_DIBitmap*			GetPDFNamedImage(FX_WSTR wsName, FX_INT32 &iImageXDpi, FX_INT32 &iImageYDpi);
    IFDE_XMLElement*		GetPackageData(FX_WSTR wsPackage);
    FX_BOOL					SavePackage(FX_WSTR wsPackage, IFX_FileWrite* pFile, IXFA_ChecksumContext *pCSContext = NULL);
    FX_BOOL					ImportData(IFX_FileRead* pStream, FX_BOOL bXDP = TRUE);
protected:
    IXFA_DocProvider*		m_pDocProvider;
    CXFA_Document*			m_pDocument;
    IFX_FileRead*			m_pStream;
    CXFA_FFApp*				m_pApp;
    CXFA_FFNotify*			m_pNotify;
    CPDF_Document*			m_pPDFDoc;
    CFX_MapPtrToPtr			m_mapNamedImages;
    CFX_MapPtrToPtr			m_mapTypeToDocView;
    FX_DWORD				m_dwDocType;
    FX_BOOL					m_bOwnStream;
};
#endif
