// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_CXFA_FFAPP_H_
#define XFA_FXFA_CXFA_FFAPP_H_

#include <memory>

#include "core/fxcrt/unowned_ptr.h"
#include "fxjs/gc/heap.h"
#include "v8/include/cppgc/garbage-collected.h"
#include "v8/include/cppgc/member.h"
#include "xfa/fwl/cfwl_app.h"
#include "xfa/fxfa/fxfa.h"

class CFGAS_FontMgr;
class CFWL_WidgetMgr;
class CXFA_FontMgr;
class CXFA_FWLAdapterWidgetMgr;
class CXFA_FWLTheme;

class CXFA_FFApp : public cppgc::GarbageCollected<CXFA_FFApp>,
                   public CFWL_App::AdapterIface {
 public:
  static void SkipFontLoadForTesting(bool skip);

  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FFApp() override;

  // CFWL_App::AdapterIface:
  void Trace(cppgc::Visitor* visitor) const override;
  CFWL_WidgetMgr::AdapterIface* GetWidgetMgrAdapter() override;
  TimerHandlerIface* GetTimerHandler() override;
  IFWL_ThemeProvider* GetThemeProvider() override;
  cppgc::Heap* GetHeap() override;

  bool LoadFWLTheme(CXFA_FFDoc* doc);
  CFWL_WidgetMgr* GetFWLWidgetMgr() const { return m_pFWLApp->GetWidgetMgr(); }
  CFGAS_FontMgr* GetFGASFontMgr();

  IXFA_AppProvider* GetAppProvider() const { return m_pProvider.Get(); }
  CFWL_App* GetFWLApp() const { return m_pFWLApp; }
  CXFA_FontMgr* GetXFAFontMgr() const { return m_pXFAFontMgr.get(); }

 private:
  explicit CXFA_FFApp(IXFA_AppProvider* pProvider);

  UnownedPtr<IXFA_AppProvider> const m_pProvider;

  // The fonts stored in the font manager may have been created by the default
  // font manager. The GEFont::LoadFont call takes the manager as a param and
  // stores it internally. When you destroy the GEFont it tries to unregister
  // from the font manager and if the default font manager was destroyed first
  // you get a use-after-free. The m_pFWLTheme can try to cleanup a GEFont
  // when it frees, so make sure it gets cleaned up first. That requires
  // m_pFWLApp to be cleaned up as well.
  //
  // TODO(dsinclair): The GEFont should have the FontMgr as the pointer instead
  // of the DEFFontMgr so this goes away. Bug 561.
  std::unique_ptr<CFGAS_FontMgr> m_pFGASFontMgr;
  std::unique_ptr<CXFA_FontMgr> m_pXFAFontMgr;
  cppgc::Member<CXFA_FWLAdapterWidgetMgr> m_pAdapterWidgetMgr;
  cppgc::Member<CXFA_FWLTheme> m_pFWLTheme;
  cppgc::Member<CFWL_App> m_pFWLApp;
};

#endif  // XFA_FXFA_CXFA_FFAPP_H_
