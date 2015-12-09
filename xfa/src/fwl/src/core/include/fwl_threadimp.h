// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FWL_THREADIMP_H_
#define FWL_THREADIMP_H_

#include "xfa/include/fwl/core/fwl_thread.h"  // For FWL_HTHREAD.

class CFWL_NoteDriver;
class IFWL_NoteDriver;

class CFWL_ThreadImp {
 public:
  virtual ~CFWL_ThreadImp() {}
  IFWL_Thread* GetInterface() const { return m_pIface; }
  virtual FWL_ERR Run(FWL_HTHREAD hThread);

 protected:
  CFWL_ThreadImp(IFWL_Thread* pIface) : m_pIface(pIface) {}

 private:
  IFWL_Thread* const m_pIface;
};

class CFWL_NoteThreadImp : public CFWL_ThreadImp {
 public:
  CFWL_NoteThreadImp(IFWL_NoteThread* pIface);
  virtual ~CFWL_NoteThreadImp();

  FWL_ERR Run(FWL_HTHREAD hThread) override;
  virtual IFWL_NoteDriver* GetNoteDriver();

 protected:
  CFWL_NoteDriver* const m_pNoteDriver;
};

#endif  // FWL_THREADIMP_H_
