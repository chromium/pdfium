// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FWL_THREAD_H_
#define FWL_THREAD_H_

class IFWL_NoteDriver;

typedef struct _FWL_HTHREAD { void* pData; } * FWL_HTHREAD;

class IFWL_Thread {
 public:
  virtual void Release() = 0;
  virtual FWL_ERR Run(FWL_HTHREAD hThread) = 0;

 protected:
  virtual ~IFWL_Thread() {}
};

class IFWL_NoteThread : public IFWL_Thread {
 public:
  virtual IFWL_NoteDriver* GetNoteDriver() = 0;
};

#endif  // FWL_THREAD_H_
