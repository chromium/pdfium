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
  CFWL_ThreadImp();
  virtual ~CFWL_ThreadImp();

  virtual FWL_ERR Run(FWL_HTHREAD hThread);
};

class CFWL_NoteThreadImp : public CFWL_ThreadImp {
 public:
  CFWL_NoteThreadImp();
  virtual ~CFWL_NoteThreadImp();

  FWL_ERR Run(FWL_HTHREAD hThread) override;
  virtual IFWL_NoteDriver* GetNoteDriver();

 protected:
  CFWL_NoteDriver* m_pNoteDriver;
};

#endif  // FWL_THREADIMP_H_
