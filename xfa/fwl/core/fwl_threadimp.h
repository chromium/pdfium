// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CORE_FWL_THREADIMP_H_
#define XFA_FWL_CORE_FWL_THREADIMP_H_

#include "xfa/fwl/core/fwl_noteimp.h"
#include "xfa/fwl/core/ifwl_thread.h"

class IFWL_NoteDriver;
class IFWL_NoteThread;

class CFWL_ThreadImp {
 public:
  CFWL_ThreadImp(IFWL_Thread* pIface)
      : m_pNoteDriver(new CFWL_NoteDriver), m_pIface(pIface) {}

  virtual ~CFWL_ThreadImp() { delete m_pNoteDriver; }

  IFWL_Thread* GetInterface() const { return m_pIface; }
  IFWL_NoteDriver* GetNoteDriver() const { return m_pNoteDriver; }

 protected:
  CFWL_NoteDriver* const m_pNoteDriver;

 private:
  IFWL_Thread* const m_pIface;
};

#endif  // XFA_FWL_CORE_FWL_THREADIMP_H_
