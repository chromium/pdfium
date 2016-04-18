// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/core/fwl_threadimp.h"

#include "xfa/fwl/core/fwl_appimp.h"
#include "xfa/fwl/core/fwl_noteimp.h"
#include "xfa/fwl/core/fwl_targetimp.h"
#include "xfa/fwl/core/ifwl_thread.h"

void IFWL_Thread::Release() {
  delete m_pImpl;
}

IFWL_NoteDriver* IFWL_Thread::GetNoteDriver() const {
  return static_cast<CFWL_ThreadImp*>(GetImpl())->GetNoteDriver();
}
