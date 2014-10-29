// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../foxitlib.h"
#include "../common/xfa_common.h"
#include "xfa_ffdochandler.h"
#include "xfa_fwladapter.h"
#include "xfa_ffdoc.h"
#include "xfa_ffapp.h"
#include "xfa_fwltheme.h"
#include "xfa_fontmgr.h"
#include "xfa_ffwidgethandler.h"
CXFA_FileRead::CXFA_FileRead(const CFX_ArrayTemplate<CPDF_Stream*> &streams)
    : m_dwSize(0)
{
    m_Streams.Copy(streams);
}
CXFA_FileRead::~CXFA_FileRead()
{
    m_Streams.RemoveAll();
    m_StreamSize.RemoveAll();
}
FX_FILESIZE CXFA_FileRead::GetSize()
{
    if (m_StreamSize.GetSize() > 0) {
        return m_dwSize;
    }
    FX_INT32 iCount = m_Streams.GetSize();
    FX_DWORD iBufferSize = 4096;
    FX_LPBYTE pBuf = FX_Alloc(FX_BYTE, iBufferSize);
    for (FX_INT32 i = 0; i < iCount; i++) {
        CPDF_StreamFilter* pStreamFilter = m_Streams[i]->GetStreamFilter(FALSE);
        FX_DWORD dwCurSize = 0;
        while (TRUE) {
            FX_DWORD dwRead = pStreamFilter->ReadBlock(pBuf, iBufferSize);
            dwCurSize += dwRead;
            if (dwRead < iBufferSize) {
                break;
            }
        }
        m_dwSize += dwCurSize;
        m_StreamSize.Add(dwCurSize);
        delete pStreamFilter;
    }
    FX_Free(pBuf);
    return m_dwSize;
}
FX_BOOL CXFA_FileRead::ReadBlock(void* buffer, FX_FILESIZE offset, size_t size)
{
    FX_FILESIZE dwLen = 0;
    FX_INT32 iCount = m_Streams.GetSize();
    FX_INT32 i = 0;
    for (; i < iCount; i++) {
        dwLen += m_StreamSize[i];
        if (dwLen > offset) {
            dwLen -= m_StreamSize[i];
            break;
        }
    }
    if (i >= iCount) {
        return FALSE;
    }
    CPDF_StreamFilter* pStreamFilter = m_Streams[i]->GetStreamFilter(FALSE);
    if ((offset -= dwLen) > 0) {
        FX_LPBYTE pBuf = FX_Alloc(FX_BYTE, offset);
        FX_DWORD dwRead = pStreamFilter->ReadBlock(pBuf, offset);
        FX_Free(pBuf);
    }
    FX_DWORD dwHadRead = pStreamFilter->ReadBlock((FX_LPBYTE)buffer, size);
    delete pStreamFilter;
    size -= dwHadRead;
    if (size <= 0) {
        return TRUE;
    }
    FX_DWORD dwReadSize = dwHadRead;
    for (FX_INT32 iStart = i + 1; iStart < iCount; iStart++) {
        CPDF_StreamFilter* pStreamFilter = m_Streams[iStart]->GetStreamFilter(FALSE);
        FX_DWORD dwHadRead = pStreamFilter->ReadBlock(((FX_LPBYTE)buffer) + dwReadSize, size);
        delete pStreamFilter;
        size -= dwHadRead;
        if (size <= 0) {
            return TRUE;
        }
        dwReadSize += dwHadRead;
    }
    return FALSE;
}
CXFA_FileRead2::CXFA_FileRead2(const CFX_ArrayTemplate<CPDF_Stream*> &streams)
{
    FX_INT32 iCount = streams.GetSize();
    for (FX_INT32 i = 0; i < iCount; i++) {
        CPDF_StreamAcc &acc = m_Data.Add();
        acc.LoadAllData(streams[i]);
    }
}
FX_FILESIZE CXFA_FileRead2::GetSize()
{
    FX_DWORD dwSize = 0;
    FX_INT32 iCount = m_Data.GetSize();
    for (FX_INT32 i = 0; i < iCount; i++) {
        CPDF_StreamAcc &acc = m_Data[i];
        dwSize += acc.GetSize();
    }
    return dwSize;
}
FX_BOOL CXFA_FileRead2::ReadBlock(void* buffer, FX_FILESIZE offset, size_t size)
{
    FX_INT32 iCount = m_Data.GetSize();
    FX_INT32 index = 0;
    while (index < iCount) {
        CPDF_StreamAcc &acc = m_Data[index];
        FX_FILESIZE dwSize = acc.GetSize();
        if (offset < dwSize) {
            break;
        }
        offset -= dwSize;
        index++;
    }
    while (index < iCount) {
        CPDF_StreamAcc &acc = m_Data[index];
        FX_DWORD dwSize = acc.GetSize();
        FX_DWORD dwRead = FX_MIN(size, dwSize - offset);
        FXSYS_memcpy(buffer, acc.GetData() + offset, dwRead);
        size -= dwRead;
        if (size == 0) {
            return TRUE;
        }
        buffer = (FX_LPBYTE)buffer + dwRead;
        offset = 0;
        index++;
    }
    return FALSE;
}
IXFA_App* IXFA_App::Create(IXFA_AppProvider* pProvider)
{
    return FX_NEW CXFA_FFApp(pProvider);
}
CXFA_FFApp::CXFA_FFApp(IXFA_AppProvider* pProvider)
    : m_pDocHandler(NULL)
    , m_pFWLTheme(NULL)
    , m_pProvider(pProvider)
    , m_pFontMgr(NULL)
    , m_pAdapterWidgetMgr(NULL)
    , m_pFDEFontMgr(NULL)
    , m_pMenuHandler(NULL)
    , m_pAdapterThreadMgr(NULL)
#if _FXM_PLATFORM_ != _FXM_PLATFORM_WINDOWS_
    , m_pFontSource(NULL)
#endif
{
    m_pFWLApp = IFWL_App::Create(this);
    FWL_SetApp(m_pFWLApp);
    m_pFWLApp->Initialize();
    IXFA_TimeZoneProvider::Create();
}
CXFA_FFApp::~CXFA_FFApp()
{
    if (m_pDocHandler) {
        delete m_pDocHandler;
    }
    if (m_pFWLApp) {
        m_pFWLApp->Finalize();
        m_pFWLApp->Release();
    }
    if (m_pFWLTheme) {
        m_pFWLTheme->Release();
    }
    if (m_pAdapterWidgetMgr) {
        delete m_pAdapterWidgetMgr;
    }
    if (m_pAdapterThreadMgr) {
        delete m_pAdapterThreadMgr;
        m_pAdapterThreadMgr = NULL;
    }
    if (m_pMenuHandler) {
        delete m_pMenuHandler;
        m_pMenuHandler = NULL;
    }
    IXFA_TimeZoneProvider::Destroy();
    if (m_pFontMgr != NULL) {
        delete m_pFontMgr;
        m_pFontMgr = NULL;
    }
#if _FXM_PLATFORM_ != _FXM_PLATFORM_WINDOWS_
    if(m_pFontSource != NULL) {
        m_pFontSource->Release();
    }
#endif
    if (m_pFDEFontMgr) {
        m_pFDEFontMgr->Release();
    }
}
IXFA_MenuHandler* CXFA_FFApp::GetMenuHandler()
{
    if (!m_pMenuHandler) {
        m_pMenuHandler = FX_NEW CXFA_FFMenuHandler;
    }
    return m_pMenuHandler;
}
IXFA_DocHandler* CXFA_FFApp::GetDocHandler()
{
    if (!m_pDocHandler) {
        m_pDocHandler = FX_NEW CXFA_FFDocHandler;
    }
    return m_pDocHandler;
}
XFA_HDOC CXFA_FFApp::CreateDoc(IXFA_DocProvider* pProvider, IFX_FileRead* pStream, FX_BOOL bTakeOverFile)
{
    CXFA_FFDoc* pDoc = FX_NEW CXFA_FFDoc(this, pProvider);
    if (!pDoc) {
        return NULL;
    }
    FX_BOOL bSuccess = pDoc->OpenDoc(pStream, bTakeOverFile);
    if (!bSuccess) {
        delete pDoc;
        pDoc = NULL;
    }
    return (XFA_HDOC)pDoc;
}
XFA_HDOC CXFA_FFApp::CreateDoc(IXFA_DocProvider* pProvider, CPDF_Document *pPDFDoc)
{
    if (pPDFDoc == NULL) {
        return NULL;
    }
    CXFA_FFDoc* pDoc = FX_NEW CXFA_FFDoc(this, pProvider);
    if (!pDoc) {
        return NULL;
    }
    FX_BOOL bSuccess = pDoc->OpenDoc(pPDFDoc);
    if (!bSuccess) {
        delete pDoc;
        pDoc = NULL;
    }
    return (XFA_HDOC)pDoc;
}

void CXFA_FFApp::SetDefaultFontMgr(IXFA_FontMgr* pFontMgr)
{
    if (!m_pFontMgr) {
        m_pFontMgr = FX_NEW CXFA_FontMgr();
    }
    m_pFontMgr->SetDefFontMgr(pFontMgr);
}
CXFA_FontMgr* CXFA_FFApp::GetXFAFontMgr()
{
    return m_pFontMgr;
}
IFX_FontMgr* CXFA_FFApp::GetFDEFontMgr()
{
    if (!m_pFDEFontMgr) {
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
        m_pFDEFontMgr = IFX_FontMgr::Create(FX_GetDefFontEnumerator());
#else
        m_pFontSource = FX_CreateDefaultFontSourceEnum();
        m_pFDEFontMgr = IFX_FontMgr::Create(m_pFontSource);
#endif
    }
    return m_pFDEFontMgr;
}
CXFA_FWLTheme* CXFA_FFApp::GetFWLTheme()
{
    if (!m_pFWLTheme) {
        m_pFWLTheme = FX_NEW CXFA_FWLTheme(this);
    }
    return m_pFWLTheme;
}
IFWL_AdapterWidgetMgr* CXFA_FFApp::GetWidgetMgr(IFWL_WidgetMgrDelegate *pDelegate)
{
    if (!m_pAdapterWidgetMgr) {
        m_pAdapterWidgetMgr = FX_NEW CXFA_FWLAdapterWidgetMgr;
        pDelegate->OnSetCapability(FWL_WGTMGR_DisableThread | FWL_WGTMGR_DisableForm);
        m_pWidgetMgrDelegate = pDelegate;
    }
    return m_pAdapterWidgetMgr;
}
IFWL_AdapterThreadMgr* CXFA_FFApp::GetThreadMgr()
{
    if (!m_pAdapterThreadMgr) {
        m_pAdapterThreadMgr = FX_NEW CFWL_SDAdapterThreadMgr;
    }
    return m_pAdapterThreadMgr;
}
IFWL_AdapterTimerMgr* CXFA_FFApp::GetTimerMgr()
{
    return m_pProvider->GetTimerMgr();
}
IFWL_AdapterCursorMgr*	CXFA_FFApp::GetCursorMgr()
{
    return NULL;
}
IFWL_AdapterMonitorMgr* CXFA_FFApp::GetMonitorMgr()
{
    return NULL;
}
IFWL_AdapterClipboardMgr* CXFA_FFApp::GetClipboardMgr()
{
    return NULL;
}
