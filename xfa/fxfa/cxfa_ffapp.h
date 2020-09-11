// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_CXFA_FFAPP_H_
#define XFA_FXFA_CXFA_FFAPP_H_

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
  IXFA_AppProvider* GetAppProvider() const { return m_pProvider.Get(); }
  CFWL_App* GetFWLApp() const { return m_pFWLApp; }
  CXFA_FontMgr* GetXFAFontMgr() const { return m_pXFAFontMgr; }

 private:
  explicit CXFA_FFApp(IXFA_AppProvider* pProvider);

  UnownedPtr<IXFA_AppProvider> const m_pProvider;
  cppgc::Member<CXFA_FontMgr> m_pXFAFontMgr;
  cppgc::Member<CXFA_FWLAdapterWidgetMgr> m_pAdapterWidgetMgr;
  cppgc::Member<CXFA_FWLTheme> m_pFWLTheme;
  cppgc::Member<CFWL_App> m_pFWLApp;
};

#endif  // XFA_FXFA_CXFA_FFAPP_H_
