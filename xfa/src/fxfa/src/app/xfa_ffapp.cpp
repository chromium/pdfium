// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <algorithm>

#include "xfa/src/foxitlib.h"
#include "xfa/src/fxfa/src/common/xfa_common.h"
#include "xfa_ffdochandler.h"
#include "xfa_fwladapter.h"
#include "xfa_ffdoc.h"
#include "xfa_ffapp.h"
#include "xfa_fwltheme.h"
#include "xfa_fontmgr.h"
#include "xfa_ffwidgethandler.h"

CXFA_FileRead::CXFA_FileRead(const CFX_ArrayTemplate<CPDF_Stream*>& streams) {
  int32_t iCount = streams.GetSize();
  for (int32_t i = 0; i < iCount; i++) {
    CPDF_StreamAcc& acc = m_Data.Add();
    acc.LoadAllData(streams[i]);
  }
}
FX_FILESIZE CXFA_FileRead::GetSize() {
  FX_DWORD dwSize = 0;
  int32_t iCount = m_Data.GetSize();
  for (int32_t i = 0; i < iCount; i++) {
    CPDF_StreamAcc& acc = m_Data[i];
    dwSize += acc.GetSize();
  }
  return dwSize;
}
FX_BOOL CXFA_FileRead::ReadBlock(void* buffer,
                                 FX_FILESIZE offset,
                                 size_t size) {
  int32_t iCount = m_Data.GetSize();
  int32_t index = 0;
  while (index < iCount) {
    CPDF_StreamAcc& acc = m_Data[index];
    FX_FILESIZE dwSize = acc.GetSize();
    if (offset < dwSize) {
      break;
    }
    offset -= dwSize;
    index++;
  }
  while (index < iCount) {
    CPDF_StreamAcc& acc = m_Data[index];
    FX_DWORD dwSize = acc.GetSize();
    size_t dwRead = std::min(size, static_cast<size_t>(dwSize - offset));
    FXSYS_memcpy(buffer, acc.GetData() + offset, dwRead);
    size -= dwRead;
    if (size == 0) {
      return TRUE;
    }
    buffer = (uint8_t*)buffer + dwRead;
    offset = 0;
    index++;
  }
  return FALSE;
}
// static
IXFA_App* IXFA_App::Create(IXFA_AppProvider* pProvider) {
  return new CXFA_FFApp(pProvider);
}
// virtual
IXFA_App::~IXFA_App() {}
CXFA_FFApp::CXFA_FFApp(IXFA_AppProvider* pProvider)
    : m_pDocHandler(nullptr),
      m_pFWLTheme(nullptr),
      m_pProvider(pProvider),
      m_pFontMgr(nullptr),
#if _FXM_PLATFORM_ != _FXM_PLATFORM_WINDOWS_
      m_pFontSource(nullptr),
#endif
      m_pAdapterWidgetMgr(nullptr),
      m_pWidgetMgrDelegate(nullptr),
      m_pFDEFontMgr(nullptr),
      m_pMenuHandler(nullptr),
      m_pAdapterThreadMgr(nullptr) {
  m_pFWLApp = IFWL_App::Create(this);
  FWL_SetApp(m_pFWLApp);
  m_pFWLApp->Initialize();
  IXFA_TimeZoneProvider::Create();
}
CXFA_FFApp::~CXFA_FFApp() {
  if (m_pDocHandler) {
    delete m_pDocHandler;
  }
  if (m_pFWLApp) {
    m_pFWLApp->Finalize();
    m_pFWLApp->Release();
    delete m_pFWLApp;
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
  if (m_pFontSource != NULL) {
    m_pFontSource->Release();
  }
#endif
  if (m_pFDEFontMgr) {
    m_pFDEFontMgr->Release();
  }
}
IXFA_MenuHandler* CXFA_FFApp::GetMenuHandler() {
  if (!m_pMenuHandler) {
    m_pMenuHandler = new CXFA_FFMenuHandler;
  }
  return m_pMenuHandler;
}
IXFA_DocHandler* CXFA_FFApp::GetDocHandler() {
  if (!m_pDocHandler) {
    m_pDocHandler = new CXFA_FFDocHandler;
  }
  return m_pDocHandler;
}
IXFA_Doc* CXFA_FFApp::CreateDoc(IXFA_DocProvider* pProvider,
                                IFX_FileRead* pStream,
                                FX_BOOL bTakeOverFile) {
  CXFA_FFDoc* pDoc = new CXFA_FFDoc(this, pProvider);
  FX_BOOL bSuccess = pDoc->OpenDoc(pStream, bTakeOverFile);
  if (!bSuccess) {
    delete pDoc;
    pDoc = NULL;
  }
  return pDoc;
}
IXFA_Doc* CXFA_FFApp::CreateDoc(IXFA_DocProvider* pProvider,
                                CPDF_Document* pPDFDoc) {
  if (pPDFDoc == NULL) {
    return NULL;
  }
  CXFA_FFDoc* pDoc = new CXFA_FFDoc(this, pProvider);
  FX_BOOL bSuccess = pDoc->OpenDoc(pPDFDoc);
  if (!bSuccess) {
    delete pDoc;
    pDoc = NULL;
  }
  return pDoc;
}

void CXFA_FFApp::SetDefaultFontMgr(IXFA_FontMgr* pFontMgr) {
  if (!m_pFontMgr) {
    m_pFontMgr = new CXFA_FontMgr();
  }
  m_pFontMgr->SetDefFontMgr(pFontMgr);
}
CXFA_FontMgr* CXFA_FFApp::GetXFAFontMgr() {
  return m_pFontMgr;
}
IFX_FontMgr* CXFA_FFApp::GetFDEFontMgr() {
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
CXFA_FWLTheme* CXFA_FFApp::GetFWLTheme() {
  if (!m_pFWLTheme) {
    m_pFWLTheme = new CXFA_FWLTheme(this);
  }
  return m_pFWLTheme;
}
IFWL_AdapterWidgetMgr* CXFA_FFApp::GetWidgetMgr(
    IFWL_WidgetMgrDelegate* pDelegate) {
  if (!m_pAdapterWidgetMgr) {
    m_pAdapterWidgetMgr = new CXFA_FWLAdapterWidgetMgr;
    pDelegate->OnSetCapability(FWL_WGTMGR_DisableThread |
                               FWL_WGTMGR_DisableForm);
    m_pWidgetMgrDelegate = pDelegate;
  }
  return m_pAdapterWidgetMgr;
}
IFWL_AdapterThreadMgr* CXFA_FFApp::GetThreadMgr() {
  if (!m_pAdapterThreadMgr) {
    m_pAdapterThreadMgr = new CFWL_SDAdapterThreadMgr;
  }
  return m_pAdapterThreadMgr;
}
IFWL_AdapterTimerMgr* CXFA_FFApp::GetTimerMgr() {
  return m_pProvider->GetTimerMgr();
}
IFWL_AdapterCursorMgr* CXFA_FFApp::GetCursorMgr() {
  return NULL;
}
IFWL_AdapterMonitorMgr* CXFA_FFApp::GetMonitorMgr() {
  return NULL;
}
IFWL_AdapterClipboardMgr* CXFA_FFApp::GetClipboardMgr() {
  return NULL;
}
