// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/cfwl_app.h"

#include "v8/include/cppgc/allocation.h"
#include "xfa/fwl/cfwl_notedriver.h"
#include "xfa/fwl/cfwl_widget.h"
#include "xfa/fwl/cfwl_widgetmgr.h"

CFWL_App::CFWL_App(AdapterIface* pAdapter)
    : m_pAdapter(pAdapter),
      m_pWidgetMgr(cppgc::MakeGarbageCollected<CFWL_WidgetMgr>(
          pAdapter->GetHeap()->GetAllocationHandle(),
          pAdapter->GetWidgetMgrAdapter(),
          this)),
      m_pNoteDriver(cppgc::MakeGarbageCollected<CFWL_NoteDriver>(
          pAdapter->GetHeap()->GetAllocationHandle(),
          this)) {}

CFWL_App::~CFWL_App() = default;

void CFWL_App::Trace(cppgc::Visitor* visitor) const {
  visitor->Trace(m_pAdapter);
  visitor->Trace(m_pWidgetMgr);
  visitor->Trace(m_pNoteDriver);
}
