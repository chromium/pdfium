// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_THREAD_IMP_H
#define _FWL_THREAD_IMP_H

#include "xfa/include/fwl/core/fwl_thread.h"  // For FWL_HTHREAD.

class CFWL_NoteDriver;
class IFWL_NoteDriver;

class CFWL_Thread {
 public:
  CFWL_Thread();
  virtual ~CFWL_Thread();
  virtual void Release() { delete this; }
  virtual FWL_ERR Run(FWL_HTHREAD hThread);
};

class CFWL_NoteThread : public CFWL_Thread {
 public:
  CFWL_NoteThread();
  virtual ~CFWL_NoteThread();
  virtual FWL_ERR Run(FWL_HTHREAD hThread);
  virtual IFWL_NoteDriver* GetNoteDriver();

 protected:
  CFWL_NoteDriver* m_pNoteDriver;
  FWL_HTHREAD m_hThread;
};
#endif
