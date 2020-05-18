// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/cfwl_app.h"

#include "xfa/fwl/cfwl_notedriver.h"
#include "xfa/fwl/cfwl_widget.h"
#include "xfa/fwl/cfwl_widgetmgr.h"

CFWL_App::CFWL_App(AdapterIface* pAdapter)
    : m_pAdapterNative(pAdapter),
      m_pWidgetMgr(
          std::make_unique<CFWL_WidgetMgr>(pAdapter->GetWidgetMgrAdapter())),
      m_pNoteDriver(std::make_unique<CFWL_NoteDriver>()) {
  ASSERT(m_pAdapterNative);
}

CFWL_App::~CFWL_App() = default;
