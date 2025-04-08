// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/cxfa_ffapp.h"

#include "xfa/fwl/cfwl_notedriver.h"
#include "xfa/fwl/cfwl_widgetmgr.h"
#include "xfa/fxfa/cxfa_ffdoc.h"
#include "xfa/fxfa/cxfa_ffwidgethandler.h"
#include "xfa/fxfa/cxfa_fontmgr.h"
#include "xfa/fxfa/cxfa_fwladapterwidgetmgr.h"
#include "xfa/fxfa/cxfa_fwltheme.h"

CXFA_FFApp::CXFA_FFApp(CallbackIface* pProvider) : provider_(pProvider) {
  // Ensure fully initialized before making objects based on |this|.
  xfafont_mgr_ = cppgc::MakeGarbageCollected<CXFA_FontMgr>(
      GetHeap()->GetAllocationHandle());
  fwlapp_ = cppgc::MakeGarbageCollected<CFWL_App>(
      GetHeap()->GetAllocationHandle(), this);
}

CXFA_FFApp::~CXFA_FFApp() = default;

void CXFA_FFApp::Trace(cppgc::Visitor* visitor) const {
  visitor->Trace(xfafont_mgr_);
  visitor->Trace(adapter_widget_mgr_);
  visitor->Trace(fwltheme_);
  visitor->Trace(fwlapp_);
}

bool CXFA_FFApp::LoadFWLTheme(CXFA_FFDoc* doc) {
  auto* fwl_theme = cppgc::MakeGarbageCollected<CXFA_FWLTheme>(
      GetHeap()->GetAllocationHandle(), GetHeap(), this);
  if (!fwl_theme->LoadCalendarFont(doc)) {
    return false;
  }

  fwltheme_ = fwl_theme;
  return true;
}

CFWL_WidgetMgr::AdapterIface* CXFA_FFApp::GetWidgetMgrAdapter() {
  if (!adapter_widget_mgr_) {
    adapter_widget_mgr_ = cppgc::MakeGarbageCollected<CXFA_FWLAdapterWidgetMgr>(
        GetHeap()->GetAllocationHandle());
  }
  return adapter_widget_mgr_;
}

CFX_Timer::HandlerIface* CXFA_FFApp::GetTimerHandler() {
  return provider_->GetTimerHandler();
}

IFWL_ThemeProvider* CXFA_FFApp::GetThemeProvider() {
  return fwltheme_;
}

cppgc::Heap* CXFA_FFApp::GetHeap() {
  return provider_->GetGCHeap();
}
