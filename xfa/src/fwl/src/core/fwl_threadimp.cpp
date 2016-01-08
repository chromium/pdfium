// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/foxitlib.h"
#include "xfa/src/fwl/src/core/include/fwl_targetimp.h"
#include "xfa/src/fwl/src/core/include/fwl_noteimp.h"
#include "xfa/src/fwl/src/core/include/fwl_threadimp.h"
#include "xfa/src/fwl/src/core/include/fwl_appimp.h"

void IFWL_Thread::Release() {
  delete m_pImpl;
}
FWL_ERR IFWL_Thread::Run(FWL_HTHREAD hThread) {
  return m_pImpl->Run(hThread);
}
IFWL_NoteDriver* IFWL_NoteThread::GetNoteDriver() {
  return static_cast<CFWL_NoteThreadImp*>(GetImpl())->GetNoteDriver();
}

FWL_ERR CFWL_ThreadImp::Run(FWL_HTHREAD hThread) {
  return FWL_ERR_Succeeded;
}
CFWL_NoteThreadImp::CFWL_NoteThreadImp(IFWL_NoteThread* pIface)
    : CFWL_ThreadImp(pIface), m_pNoteDriver(new CFWL_NoteDriver) {}
CFWL_NoteThreadImp::~CFWL_NoteThreadImp() {
  delete m_pNoteDriver;
}
FWL_ERR CFWL_NoteThreadImp::Run(FWL_HTHREAD hThread) {
  if (!m_pNoteDriver)
    return FWL_ERR_Indefinite;
  return m_pNoteDriver->Run();
}
IFWL_NoteDriver* CFWL_NoteThreadImp::GetNoteDriver() {
  return m_pNoteDriver;
}
