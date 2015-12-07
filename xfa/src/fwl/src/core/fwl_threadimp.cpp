// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../foxitlib.h"
#include "include/fwl_targetimp.h"
#include "include/fwl_noteimp.h"
#include "include/fwl_threadimp.h"
#include "include/fwl_appimp.h"
CFWL_ThreadImp::CFWL_ThreadImp() {}
CFWL_ThreadImp::~CFWL_ThreadImp() {}
FWL_ERR CFWL_ThreadImp::Run(FWL_HTHREAD hThread) {
  return FWL_ERR_Succeeded;
}
CFWL_NoteThreadImp::CFWL_NoteThreadImp() {
  m_pNoteDriver = new CFWL_NoteDriver;
}
CFWL_NoteThreadImp::~CFWL_NoteThreadImp() {
  delete m_pNoteDriver;
}
FWL_ERR CFWL_NoteThreadImp::Run(FWL_HTHREAD hThread) {
  if (!m_pNoteDriver)
    return FWL_ERR_Indefinite;
  return m_pNoteDriver->Run();
}
IFWL_NoteDriver* CFWL_NoteThreadImp::GetNoteDriver() {
  return (IFWL_NoteDriver*)m_pNoteDriver;
}
