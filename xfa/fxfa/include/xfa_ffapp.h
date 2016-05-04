// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_INCLUDE_XFA_FFAPP_H_
#define XFA_FXFA_INCLUDE_XFA_FFAPP_H_

#include "core/fpdfapi/fpdf_parser/include/cpdf_stream.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_stream_acc.h"
#include "xfa/fgas/font/fgas_font.h"
#include "xfa/fwl/core/ifwl_app.h"
#include "xfa/fxfa/include/fxfa.h"

class CFWL_WidgetMgrDelegate;
class CXFA_DefFontMgr;
class CXFA_FWLAdapterWidgetMgr;
class CXFA_FWLTheme;
class CXFA_FFDocHandler;
class CXFA_FontMgr;
class IFWL_AdapterTimerMgr;

class CXFA_FileRead : public IFX_FileRead {
 public:
  explicit CXFA_FileRead(const CFX_ArrayTemplate<CPDF_Stream*>& streams);

  virtual FX_FILESIZE GetSize();
  virtual FX_BOOL ReadBlock(void* buffer, FX_FILESIZE offset, size_t size);

  virtual void Release() { delete this; }

 protected:
  CFX_ObjectArray<CPDF_StreamAcc> m_Data;
};

class CXFA_FFApp {
 public:
  CXFA_FFApp(IXFA_AppProvider* pProvider);
  ~CXFA_FFApp();

  CXFA_FFDocHandler* GetDocHandler();
  CXFA_FFDoc* CreateDoc(IXFA_DocProvider* pProvider,
                        IFX_FileRead* pStream,
                        FX_BOOL bTakeOverFile);
  CXFA_FFDoc* CreateDoc(IXFA_DocProvider* pProvider, CPDF_Document* pPDFDoc);
  IXFA_AppProvider* GetAppProvider() { return m_pProvider; }
  void SetDefaultFontMgr(CXFA_DefFontMgr* pFontMgr);

  CXFA_FWLAdapterWidgetMgr* GetWidgetMgr(CFWL_WidgetMgrDelegate* pDelegate);
  IFWL_AdapterTimerMgr* GetTimerMgr();

  CXFA_FontMgr* GetXFAFontMgr();
  IFX_FontMgr* GetFDEFontMgr();
  CXFA_FWLTheme* GetFWLTheme();
  CFWL_WidgetMgrDelegate* GetWidgetMgrDelegate() {
    return m_pWidgetMgrDelegate;
  }

 protected:
  CXFA_FFDocHandler* m_pDocHandler;
  IFWL_App* m_pFWLApp;
  CXFA_FWLTheme* m_pFWLTheme;
  IXFA_AppProvider* m_pProvider;
  CXFA_FontMgr* m_pFontMgr;
#if _FXM_PLATFORM_ != _FXM_PLATFORM_WINDOWS_
  CFX_FontSourceEnum_File* m_pFontSource;
#endif
  CXFA_FWLAdapterWidgetMgr* m_pAdapterWidgetMgr;
  CFWL_WidgetMgrDelegate* m_pWidgetMgrDelegate;
  IFX_FontMgr* m_pFDEFontMgr;
};

#endif  // XFA_FXFA_INCLUDE_XFA_FFAPP_H_
