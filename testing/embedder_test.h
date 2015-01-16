// Copyright (c) 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_EMBEDDER_TEST_H_
#define TESTING_EMBEDDER_TEST_H_

#include <string>

#include "../core/include/fxcrt/fx_system.h"
#include "../fpdfsdk/include/fpdf_dataavail.h"
#include "../fpdfsdk/include/fpdfformfill.h"
#include "../fpdfsdk/include/fpdfview.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "v8/include/v8.h"

class TestLoader;

// This class is used to load a PDF document, and then run programatic
// API tests against it.
class EmbedderTest : public ::testing::Test {
 public:
  EmbedderTest() :
      document_(nullptr),
      avail_(nullptr),
      loader_(nullptr),
      file_length_(0),
      file_contents_(nullptr) {
    memset(&hints_, 0, sizeof(hints_));
    memset(&file_access_, 0, sizeof(file_access_));
    memset(&file_avail_, 0, sizeof(file_avail_));
  }

  virtual ~EmbedderTest() { }

  void SetUp() override;
  void TearDown() override;

  FPDF_DOCUMENT document() { return document_; }

  // Open the document specified by |filename|, or return false on failure.
  virtual bool OpenDocument(const std::string& filename);

  // Create and return a handle to the form fill module for use with the
  // FORM_ family of functions from fpdfformfill.h, or return NULL on failure.
  virtual FPDF_FORMHANDLE SetFormFillEnvironment();

  // Release the resources obtained from SetFormFillEnvironment().
  virtual void ClearFormFillEnvironment(FPDF_FORMHANDLE form);

  // Perform JavaScript actions that are to run at document open time.
  virtual void DoOpenActions(FPDF_FORMHANDLE form);

  // Determine the page numbers present in the document.
  virtual int GetFirstPageNum();
  virtual int GetPageCount();

  // Load a specific page of the open document.
  virtual FPDF_PAGE LoadPage(int page_number, FPDF_FORMHANDLE form);

  // Convert a loaded page into a bitmap.
  virtual FPDF_BITMAP RenderPage(FPDF_PAGE page, FPDF_FORMHANDLE form);

  // Relese the resources obtained from LoadPage(). Further use of |page|
  // is prohibited after this call is made.
  virtual void UnloadPage(FPDF_PAGE page, FPDF_FORMHANDLE form);

 private:
  FPDF_DOCUMENT document_;
  FPDF_AVAIL avail_;
  FX_DOWNLOADHINTS hints_;
  FPDF_FILEACCESS file_access_;
  FX_FILEAVAIL file_avail_;
  v8::StartupData natives_;
  v8::StartupData snapshot_;
  TestLoader* loader_;
  size_t file_length_;
  char* file_contents_;
};

#endif  // TESTING_EMBEDDER_TEST_H_

