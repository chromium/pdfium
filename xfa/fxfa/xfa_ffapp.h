// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_XFA_FFAPP_H_
#define XFA_FXFA_XFA_FFAPP_H_

#include <memory>
#include <vector>

#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/parser/cpdf_stream_acc.h"
#include "core/fxcrt/cfx_retain_ptr.h"
#include "xfa/fgas/font/cfgas_fontmgr.h"
#include "xfa/fwl/cfwl_app.h"
#include "xfa/fxfa/fxfa.h"

class CXFA_DefFontMgr;
class CXFA_FWLAdapterWidgetMgr;
class CXFA_FWLTheme;
class CXFA_FFDocHandler;
class CXFA_FontMgr;
class IFWL_AdapterTimerMgr;
class CFWL_WidgetMgrDelegate;

// Layering prevents fxcrt from knowing about CPDF_Streams; this could go
// in fpdfsdk, but it is XFA-Only.
CFX_RetainPtr<IFX_SeekableReadStream> MakeSeekableReadStream(
    const std::vector<CPDF_Stream*>& streams);

class CXFA_FFApp {
 public:
  explicit CXFA_FFApp(IXFA_AppProvider* pProvider);
  ~CXFA_FFApp();

  std::unique_ptr<CXFA_FFDoc> CreateDoc(IXFA_DocEnvironment* pDocEnvironment,
                                        CPDF_Document* pPDFDoc);
  void SetDefaultFontMgr(std::unique_ptr<CXFA_DefFontMgr> pFontMgr);

  CXFA_FFDocHandler* GetDocHandler();
  CXFA_FWLAdapterWidgetMgr* GetWidgetMgr(CFWL_WidgetMgrDelegate* pDelegate);
  CFGAS_FontMgr* GetFDEFontMgr();
  CXFA_FWLTheme* GetFWLTheme();

  IXFA_AppProvider* GetAppProvider() const { return m_pProvider; }
  const CFWL_App* GetFWLApp() const { return m_pFWLApp.get(); }
  IFWL_AdapterTimerMgr* GetTimerMgr() const;
  CXFA_FontMgr* GetXFAFontMgr() const;
  CFWL_WidgetMgrDelegate* GetWidgetMgrDelegate() const {
    return m_pWidgetMgrDelegate;
  }

  void ClearEventTargets();

 protected:
  std::unique_ptr<CXFA_FFDocHandler> m_pDocHandler;
  IXFA_AppProvider* const m_pProvider;

  // The fonts stored in the font manager may have been created by the default
  // font manager. The GEFont::LoadFont call takes the manager as a param and
  // stores it internally. When you destroy the GEFont it tries to unregister
  // from the font manager and if the default font manager was destroyed first
  // get get a use-after-free. The m_pFWLTheme can try to cleanup a GEFont
  // when it frees, so make sure it gets cleaned up first. That requires
  // m_pFWLApp to be cleaned up as well.
  //
  // TODO(dsinclair): The GEFont should have the FontMgr as the pointer instead
  // of the DEFFontMgr so this goes away. Bug 561.
  std::unique_ptr<CFGAS_FontMgr> m_pFDEFontMgr;
  std::unique_ptr<CXFA_FontMgr> m_pFontMgr;

#if _FXM_PLATFORM_ != _FXM_PLATFORM_WINDOWS_
  std::unique_ptr<CFX_FontSourceEnum_File> m_pFontSource;
#endif
  std::unique_ptr<CXFA_FWLAdapterWidgetMgr> m_pAdapterWidgetMgr;
  CFWL_WidgetMgrDelegate* m_pWidgetMgrDelegate;  // not owned.

  // |m_pFWLApp| has to be released first, then |m_pFWLTheme| since the former
  // may refers to theme manager and the latter refers to font manager.
  std::unique_ptr<CXFA_FWLTheme> m_pFWLTheme;
  std::unique_ptr<CFWL_App> m_pFWLApp;
};

#endif  // XFA_FXFA_XFA_FFAPP_H_
