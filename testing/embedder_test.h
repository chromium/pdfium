// Copyright (c) 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_EMBEDDER_TEST_H_
#define TESTING_EMBEDDER_TEST_H_

#include <string>

#include "../core/include/fxcrt/fx_system.h"
#include "../public/fpdf_dataavail.h"
#include "../public/fpdf_ext.h"
#include "../public/fpdf_formfill.h"
#include "../public/fpdfview.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "v8/include/v8.h"

class TestLoader;

// This class is used to load a PDF document, and then run programatic
// API tests against it.
class EmbedderTest : public ::testing::Test,
                     public UNSUPPORT_INFO,
                     public IPDF_JSPLATFORM,
                     public FPDF_FORMFILLINFO {
 public:
  class Delegate {
   public:
    virtual ~Delegate() { }

    // Equivalent to UNSUPPORT_INFO::FSDK_UnSupport_Handler().
    virtual void UnsupportedHandler(int type) { }

    // Equivalent to IPDF_JSPLATFORM::app_alert().
    virtual int Alert(FPDF_WIDESTRING message, FPDF_WIDESTRING title,
                      int type, int icon) {
      return 0;
    }

    // Equivalent to FPDF_FORMFILLINFO::FFI_SetTimer().
    virtual int SetTimer(int msecs, TimerCallback fn) { return 0; }

    // Equivalent to FPDF_FORMFILLINFO::FFI_KillTimer().
    virtual void KillTimer(int id) { }
  };

  EmbedderTest();
  virtual ~EmbedderTest();

  void SetUp() override;
  void TearDown() override;

  void SetDelegate(Delegate* delegate) {
    delegate_ = delegate ? delegate : default_delegate_;
  }

  FPDF_DOCUMENT document() { return document_; }
  FPDF_FORMHANDLE form_handle() { return form_handle_; }

  // Open the document specified by |filename|, and create its form fill
  // environment, or return false on failure.
  virtual bool OpenDocument(const std::string& filename);

  // Perform JavaScript actions that are to run at document open time.
  virtual void DoOpenActions();

  // Determine the page numbers present in the document.
  virtual int GetFirstPageNum();
  virtual int GetPageCount();

  // Load a specific page of the open document.
  virtual FPDF_PAGE LoadPage(int page_number);

  // Convert a loaded page into a bitmap.
  virtual FPDF_BITMAP RenderPage(FPDF_PAGE page);

  // Relese the resources obtained from LoadPage(). Further use of |page|
  // is prohibited after this call is made.
  virtual void UnloadPage(FPDF_PAGE page);

 protected:
  Delegate* delegate_;
  Delegate* default_delegate_;
  FPDF_DOCUMENT document_;
  FPDF_FORMHANDLE form_handle_;
  FPDF_AVAIL avail_;
  FX_DOWNLOADHINTS hints_;
  FPDF_FILEACCESS file_access_;
  FX_FILEAVAIL file_avail_;
  v8::Platform* platform_;
  v8::StartupData natives_;
  v8::StartupData snapshot_;
  TestLoader* loader_;
  size_t file_length_;
  char* file_contents_;

 private:
  static void UnsupportedHandlerTrampoline(UNSUPPORT_INFO*, int type);
  static int AlertTrampoline(IPDF_JSPLATFORM* plaform, FPDF_WIDESTRING message,
                             FPDF_WIDESTRING title, int type, int icon);
  static int SetTimerTrampoline(FPDF_FORMFILLINFO* info, int msecs,
                                TimerCallback fn);
  static void KillTimerTrampoline(FPDF_FORMFILLINFO* info, int id);
};

#endif  // TESTING_EMBEDDER_TEST_H_
