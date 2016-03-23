// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CORE_IFWL_NOTETHREAD_H_
#define XFA_FWL_CORE_IFWL_NOTETHREAD_H_

#include "xfa/fwl/core/ifwl_thread.h"

class IFWL_NoteDriver;

class IFWL_NoteThread : public IFWL_Thread {
 public:
  IFWL_NoteDriver* GetNoteDriver();
};

#endif  // XFA_FWL_CORE_IFWL_NOTETHREAD_H_
