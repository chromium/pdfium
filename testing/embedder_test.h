// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_EMBEDDER_TEST_H_
#define TESTING_EMBEDDER_TEST_H_

#include <map>
#include <memory>
#include <string>

#include "public/fpdf_dataavail.h"
#include "public/fpdf_ext.h"
#include "public/fpdf_formfill.h"
#include "public/fpdf_save.h"
#include "public/fpdfview.h"
#include "testing/fake_file_access.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/test_support.h"

#ifdef PDF_ENABLE_V8
#include "v8/include/v8.h"
#endif  // PDF_ENABLE_v8

class TestLoader;

// This class is used to load a PDF document, and then run programatic
// API tests against it.
class EmbedderTest : public ::testing::Test,
                     public UNSUPPORT_INFO,
                     public IPDF_JSPLATFORM,
                     public FPDF_FORMFILLINFO,
                     public FPDF_FILEWRITE {
 public:
  class Delegate {
   public:
    virtual ~Delegate() {}

    // Equivalent to UNSUPPORT_INFO::FSDK_UnSupport_Handler().
    virtual void UnsupportedHandler(int type) {}

    // Equivalent to IPDF_JSPLATFORM::app_alert().
    virtual int Alert(FPDF_WIDESTRING message,
                      FPDF_WIDESTRING title,
                      int type,
                      int icon) {
      return 0;
    }

    // Equivalent to FPDF_FORMFILLINFO::FFI_SetTimer().
    virtual int SetTimer(int msecs, TimerCallback fn) { return 0; }

    // Equivalent to FPDF_FORMFILLINFO::FFI_KillTimer().
    virtual void KillTimer(int id) {}

    // Equivalent to FPDF_FORMFILLINFO::FFI_GetPage().
    virtual FPDF_PAGE GetPage(FPDF_FORMFILLINFO* info,
                              FPDF_DOCUMENT document,
                              int page_index);
  };

  EmbedderTest();
  virtual ~EmbedderTest();

  void SetUp() override;
  void TearDown() override;

#ifdef PDF_ENABLE_V8
  // Call before SetUp to pass shared isolate, otherwise PDFium creates one.
  void SetExternalIsolate(void* isolate) {
    external_isolate_ = static_cast<v8::Isolate*>(isolate);
  }
#endif  // PDF_ENABLE_V8

  void SetDelegate(Delegate* delegate) {
    delegate_ = delegate ? delegate : default_delegate_.get();
  }

  FPDF_DOCUMENT document() { return document_; }
  FPDF_FORMHANDLE form_handle() { return form_handle_; }

  // Create an empty document, and its form fill environment. Returns true
  // on success or false on failure.
  bool CreateEmptyDocument();

  // Open the document specified by |filename|, and create its form fill
  // environment, or return false on failure.
  // The filename is relative to the test data directory where we store all the
  // test files.
  // |password| can be nullptr if there is none.
  virtual bool OpenDocumentWithOptions(const std::string& filename,
                                       const char* password,
                                       bool must_linearize);

  // Variants provided for convenience.
  bool OpenDocument(const std::string& filename);
  bool OpenDocumentLinearized(const std::string& filename);
  bool OpenDocumentWithPassword(const std::string& filename,
                                const char* password);

  // Perform JavaScript actions that are to run at document open time.
  void DoOpenActions();

  // Determine the page numbers present in the document.
  int GetFirstPageNum();
  int GetPageCount();

  // Load a specific page of the open document.
  FPDF_PAGE LoadPage(int page_number);

  // Convert a loaded page into a bitmap.
  FPDF_BITMAP RenderPage(FPDF_PAGE page);

  // Convert a loaded page into a bitmap with page rendering flags specified.
  // See public/fpdfview.h for a list of page rendering flags.
  FPDF_BITMAP RenderPageWithFlags(FPDF_PAGE page,
                                  FPDF_FORMHANDLE handle,
                                  int flags);

  // Relese the resources obtained from LoadPage(). Further use of |page|
  // is prohibited after this call is made.
  void UnloadPage(FPDF_PAGE page);

 protected:
  bool OpenDocumentHelper(const char* password,
                          bool must_linearize,
                          FakeFileAccess* network_simulator,
                          FPDF_DOCUMENT* document,
                          FPDF_AVAIL* avail,
                          FPDF_FORMHANDLE* form_handle);

  FPDF_FORMHANDLE SetupFormFillEnvironment(FPDF_DOCUMENT doc);

  // Return the hash of |bitmap|.
  static std::string HashBitmap(FPDF_BITMAP bitmap);

#ifndef NDEBUG
  // For debugging purposes.
  // Write |bitmap| to a png file.
  static void WriteBitmapToPng(FPDF_BITMAP bitmap, const std::string& filename);
#endif

  // Check |bitmap| to make sure it has the right dimensions and content.
  static void CompareBitmap(FPDF_BITMAP bitmap,
                            int expected_width,
                            int expected_height,
                            const char* expected_md5sum);

  void ClearString() { m_String.clear(); }
  const std::string& GetString() const { return m_String; }

  static int GetBlockFromString(void* param,
                                unsigned long pos,
                                unsigned char* buf,
                                unsigned long size);

  FPDF_DOCUMENT OpenSavedDocument(const char* password = nullptr);
  void CloseSavedDocument();
  FPDF_PAGE LoadSavedPage(int page_number);
  FPDF_BITMAP RenderSavedPage(FPDF_PAGE page);
  void CloseSavedPage(FPDF_PAGE page);
  void VerifySavedRendering(FPDF_PAGE page,
                            int width,
                            int height,
                            const char* md5);
  void VerifySavedDocument(int width, int height, const char* md5);

  void SetWholeFileAvailable();

  Delegate* delegate_;
  std::unique_ptr<Delegate> default_delegate_;
  FPDF_DOCUMENT document_;
  FPDF_FORMHANDLE form_handle_;
  FPDF_AVAIL avail_;
  FPDF_FILEACCESS file_access_;  // must outlive avail_.
  void* external_isolate_;
  TestLoader* loader_;
  size_t file_length_;
  std::unique_ptr<char, pdfium::FreeDeleter> file_contents_;
  std::map<int, FPDF_PAGE> page_map_;
  std::map<FPDF_PAGE, int> page_reverse_map_;
  FPDF_DOCUMENT m_SavedDocument;
  FPDF_FORMHANDLE m_SavedForm;
  FPDF_AVAIL m_SavedAvail;
  FPDF_FILEACCESS saved_file_access_;  // must outlive m_SavedAvail.
  std::unique_ptr<FakeFileAccess> fake_file_access_;  // must outlive avail_.
  std::unique_ptr<FakeFileAccess>
      saved_fake_file_access_;  // must outlive m_SavedAvail.

 private:
  static void UnsupportedHandlerTrampoline(UNSUPPORT_INFO*, int type);
  static int AlertTrampoline(IPDF_JSPLATFORM* plaform,
                             FPDF_WIDESTRING message,
                             FPDF_WIDESTRING title,
                             int type,
                             int icon);
  static int SetTimerTrampoline(FPDF_FORMFILLINFO* info,
                                int msecs,
                                TimerCallback fn);
  static void KillTimerTrampoline(FPDF_FORMFILLINFO* info, int id);
  static FPDF_PAGE GetPageTrampoline(FPDF_FORMFILLINFO* info,
                                     FPDF_DOCUMENT document,
                                     int page_index);
  static int WriteBlockCallback(FPDF_FILEWRITE* pFileWrite,
                                const void* data,
                                unsigned long size);

  std::string m_String;
};

#endif  // TESTING_EMBEDDER_TEST_H_
