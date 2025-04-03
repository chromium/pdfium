// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/cfwl_app.h"

#include "v8/include/cppgc/allocation.h"
#include "xfa/fwl/cfwl_notedriver.h"
#include "xfa/fwl/cfwl_widget.h"
#include "xfa/fwl/cfwl_widgetmgr.h"

namespace pdfium {

CFWL_App::CFWL_App(AdapterIface* pAdapter)
    : adapter_(pAdapter),
      widget_mgr_(cppgc::MakeGarbageCollected<CFWL_WidgetMgr>(
          pAdapter->GetHeap()->GetAllocationHandle(),
          pAdapter->GetWidgetMgrAdapter(),
          this)),
      note_driver_(cppgc::MakeGarbageCollected<CFWL_NoteDriver>(
          pAdapter->GetHeap()->GetAllocationHandle(),
          this)) {}

CFWL_App::~CFWL_App() = default;

void CFWL_App::Trace(cppgc::Visitor* visitor) const {
  visitor->Trace(adapter_);
  visitor->Trace(widget_mgr_);
  visitor->Trace(note_driver_);
}

}  // namespace pdfium
