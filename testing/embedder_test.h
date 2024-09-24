// Copyright 2015 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_EMBEDDER_TEST_H_
#define TESTING_EMBEDDER_TEST_H_

#include <stdint.h>

#include <fstream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "build/build_config.h"
#include "core/fxcrt/span.h"
#include "core/fxcrt/unowned_ptr.h"
#include "public/cpp/fpdf_scopers.h"
#include "public/fpdf_dataavail.h"
#include "public/fpdf_ext.h"
#include "public/fpdf_formfill.h"
#include "public/fpdf_save.h"
#include "public/fpdfview.h"
#include "testing/fake_file_access.h"
#include "testing/gtest/include/gtest/gtest.h"

class TestLoader;

// The loading time of the CFGAS_FontMgr is linear in the number of times it is
// loaded. So, if a test suite has a lot of tests that need a font manager they
// can end up executing very, very slowly.

// This class is used to load a PDF document, and then run programatic
// API tests against it.
class EmbedderTest : public ::testing::Test,
                     public UNSUPPORT_INFO,
                     public IPDF_JSPLATFORM,
                     public FPDF_FORMFILLINFO,
                     public FPDF_FILEWRITE {
 public:
  enum class LinearizeOption { kDefaultLinearize, kMustLinearize };
  enum class JavaScriptOption { kDisableJavaScript, kEnableJavaScript };

  class Delegate {
   public:
    virtual ~Delegate() = default;

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

    // Equivalent to FPDF_FORMFILLINFO::FFI_DoURIAction().
    virtual void DoURIAction(FPDF_BYTESTRING uri) {}

    // Equivalent to FPDF_FORMFILLINFO::FFI_DoGoToAction().
    virtual void DoGoToAction(FPDF_FORMFILLINFO* info,
                              int page_index,
                              int zoom_mode,
                              float* pos_arry,
                              int array_size) {}

    // Equivalent to FPDF_FORMFILLINFO::FFI_OnFocusChange().
    virtual void OnFocusChange(FPDF_FORMFILLINFO* info,
                               FPDF_ANNOTATION annot,
                               int page_index) {}

    // Equivalent to FPDF_FORMFILLINFO::FFI_DoURIActionWithKeyboardModifier().
    virtual void DoURIActionWithKeyboardModifier(FPDF_FORMFILLINFO* info,
                                                 FPDF_BYTESTRING uri,
                                                 int modifiers) {}
  };

  class ScopedEmbedderTestPage {
   public:
    ScopedEmbedderTestPage();
    ScopedEmbedderTestPage(EmbedderTest* test, int page_index);
    ScopedEmbedderTestPage(const ScopedEmbedderTestPage&) = delete;
    ScopedEmbedderTestPage& operator=(const ScopedEmbedderTestPage&) = delete;
    ScopedEmbedderTestPage(ScopedEmbedderTestPage&&) noexcept;
    ScopedEmbedderTestPage& operator=(ScopedEmbedderTestPage&&) noexcept;
    ~ScopedEmbedderTestPage();

    FPDF_PAGE get() { return page_; }

    explicit operator bool() const { return !!page_; }

   private:
    UnownedPtr<EmbedderTest> test_;
    FPDF_PAGE page_;
  };

  EmbedderTest();
  ~EmbedderTest() override;

  void SetUp() override;
  void TearDown() override;

  Delegate* GetDelegate() { return delegate_; }
  void SetDelegate(Delegate* delegate) {
    delegate_ = delegate ? delegate : default_delegate_.get();
  }

  void SetFormFillInfoVersion(int form_fill_info_version) {
    form_fill_info_version_ = form_fill_info_version;
  }

  void SetDocumentFromAvail();
  FPDF_DOCUMENT document() const { return document_.get(); }
  FPDF_DOCUMENT saved_document() const { return saved_document_.get(); }
  FPDF_FORMHANDLE form_handle() const { return form_handle_.get(); }
  FPDF_FORMHANDLE saved_form_handle() const { return saved_form_handle_.get(); }

  // Wrapper for FPDFAvail_Create() to set `avail_`.
  void CreateAvail(FX_FILEAVAIL* file_avail, FPDF_FILEACCESS* file);
  FPDF_AVAIL avail() { return avail_.get(); }

  // Create an empty document, and its form fill environment.
  void CreateEmptyDocument();

  // Create an empty document without a form fill environment.
  void CreateEmptyDocumentWithoutFormFillEnvironment();

  // Open the document specified by |filename|, and create its form fill
  // environment, or return false on failure. The |filename| is relative to
  // the test data directory where we store all the test files. |password| can
  // be nullptr if the file is not password protected. If |javascript_opts|
  // is kDisableJavascript, then the document will be given stubs in place
  // of the real JS engine.
  virtual bool OpenDocumentWithOptions(const std::string& filename,
                                       const char* password,
                                       LinearizeOption linearize_option,
                                       JavaScriptOption javascript_option);

  // Variants provided for convenience.
  bool OpenDocument(const std::string& filename);
  bool OpenDocumentLinearized(const std::string& filename);
  bool OpenDocumentWithPassword(const std::string& filename,
                                const char* password);
  bool OpenDocumentWithoutJavaScript(const std::string& filename);

  // Close the document from a previous OpenDocument() call. This happens
  // automatically at tear-down, and is usually not explicitly required,
  // unless testing multiple documents or duplicate destruction.
  void CloseDocument();

  // Perform JavaScript actions that are to run at document open time.
  void DoOpenActions();

  // Determine the page numbers present in the document.
  int GetFirstPageNum();
  int GetPageCount();

  // Load a specific page of the open document with a given non-negative
  // `page_index`. On success, fire form events for the page and return a
  // ScopedEmbedderTestPage with the page handle. On failure, return an empty
  // ScopedEmbedderTestPage.
  // The caller needs to let the ScopedEmbedderTestPage go out of scope to
  // properly unload the page, and must do so before the page's document and
  // `this` get destroyed.
  // The caller cannot call this for a `page_index` if it already obtained and
  // holds the page handle for that page.
  ScopedEmbedderTestPage LoadScopedPage(int page_index);

  // Prefer LoadScopedPage() above.
  //
  // Load a specific page of the open document with a given non-negative
  // `page_index`. On success, fire form events for the page and return a page
  // handle. On failure, return nullptr.
  // The caller does not own the returned page handle, but must call
  // UnloadPage() on it when done.
  // The caller cannot call this for a `page_index` if it already obtained and
  // holds the page handle for that page.
  FPDF_PAGE LoadPage(int page_index);

  // Same as LoadPage(), but does not fire form events.
  FPDF_PAGE LoadPageNoEvents(int page_index);

  // Fire form unload events and release the resources for a |page| obtained
  // from LoadPage(). Further use of |page| is prohibited after calling this.
  void UnloadPage(FPDF_PAGE page);

  // Same as UnloadPage(), but does not fire form events.
  void UnloadPageNoEvents(FPDF_PAGE page);

  // Apply standard highlighting color/alpha to forms.
  void SetInitialFormFieldHighlight(FPDF_FORMHANDLE form);

  // RenderLoadedPageWithFlags() with no flags.
  ScopedFPDFBitmap RenderLoadedPage(FPDF_PAGE page);

  // Convert |page| loaded via LoadPage() into a bitmap with the specified page
  // rendering |flags|.
  //
  // See public/fpdfview.h for a list of page rendering flags.
  ScopedFPDFBitmap RenderLoadedPageWithFlags(FPDF_PAGE page, int flags);

  // RenderSavedPageWithFlags() with no flags.
  ScopedFPDFBitmap RenderSavedPage(FPDF_PAGE page);

  // Convert |page| loaded via LoadSavedPage() into a bitmap with the specified
  // page rendering |flags|.
  //
  // See public/fpdfview.h for a list of page rendering flags.
  ScopedFPDFBitmap RenderSavedPageWithFlags(FPDF_PAGE page, int flags);

  // Convert |page| into a bitmap with the specified page rendering |flags|.
  // The form handle associated with |page| should be passed in via |handle|.
  // If |handle| is nullptr, then forms on the page will not be rendered.
  //
  // See public/fpdfview.h for a list of page rendering flags.
  // If none of the above Render methods are appropriate, then use this one.
  static ScopedFPDFBitmap RenderPageWithFlags(FPDF_PAGE page,
                                              FPDF_FORMHANDLE handle,
                                              int flags);

  // Simplified form of RenderPageWithFlags() with no handle and no flags.
  static ScopedFPDFBitmap RenderPage(FPDF_PAGE page);

#if BUILDFLAG(IS_WIN)
  // Convert |page| into EMF with the specified page rendering |flags|.
  static std::vector<uint8_t> RenderPageWithFlagsToEmf(FPDF_PAGE page,
                                                       int flags);

  // Get the PostScript data from |emf_data|.
  static std::string GetPostScriptFromEmf(pdfium::span<const uint8_t> emf_data);
#endif  // BUILDFLAG(IS_WIN)

  // Return bytes for each of the FPDFBitmap_* format types.
  static int BytesPerPixelForFormat(int format);

 protected:
  using PageNumberToHandleMap = std::map<int, FPDF_PAGE>;

  bool OpenDocumentHelper(const char* password,
                          LinearizeOption linearize_option,
                          JavaScriptOption javascript_option,
                          FakeFileAccess* network_simulator,
                          ScopedFPDFDocument* document,
                          ScopedFPDFAvail* avail,
                          ScopedFPDFFormHandle* form_handle);

  FPDF_FORMHANDLE SetupFormFillEnvironment(FPDF_DOCUMENT doc,
                                           JavaScriptOption javascript_option);

  // Return the hash of only the pixels in |bitmap|. i.e. Excluding the gap, if
  // any, at the end of a row where the stride is larger than width * bpp.
  static std::string HashBitmap(FPDF_BITMAP bitmap);

  // For debugging purposes.
  // Write |bitmap| as a PNG to |filename|.
  static void WriteBitmapToPng(FPDF_BITMAP bitmap, const std::string& filename);

  // Check |bitmap| to make sure it has the right dimensions and content.
  static void CompareBitmap(FPDF_BITMAP bitmap,
                            int expected_width,
                            int expected_height,
                            const char* expected_md5sum);

  void ClearString() { data_string_.clear(); }
  const std::string& GetString() const { return data_string_; }

  static int GetBlockFromString(void* param,
                                unsigned long pos,
                                unsigned char* buf,
                                unsigned long size);

  // See comments in the respective non-Saved versions of these methods.
  FPDF_DOCUMENT OpenSavedDocument();
  FPDF_DOCUMENT OpenSavedDocumentWithPassword(const char* password);
  void CloseSavedDocument();
  FPDF_PAGE LoadSavedPage(int page_index);
  void CloseSavedPage(FPDF_PAGE page);

  void VerifySavedRendering(FPDF_PAGE page,
                            int width,
                            int height,
                            const char* md5);
  void VerifySavedDocument(int width, int height, const char* md5);

  void SetWholeFileAvailable();

#ifndef NDEBUG
  // For debugging purposes.
  // While open, write any data that gets passed to WriteBlockCallback() to
  // |filename|. This is typically used to capture data from FPDF_SaveAsCopy()
  // calls.
  void OpenPDFFileForWrite(const std::string& filename);
  void ClosePDFFileForWrite();
#endif

 private:
  static int WriteBlockCallback(FPDF_FILEWRITE* pFileWrite,
                                const void* data,
                                unsigned long size);

  // Helper method for the methods below.
  static int GetPageNumberForPage(const PageNumberToHandleMap& page_map,
                                  FPDF_PAGE page);
  // Find |page| inside |page_map_| and return the associated page number, or -1
  // if |page| cannot be found.
  int GetPageNumberForLoadedPage(FPDF_PAGE page) const;

  // Same as GetPageNumberForLoadedPage(), but with |saved_page_map_|.
  int GetPageNumberForSavedPage(FPDF_PAGE page) const;

  void UnloadPageCommon(FPDF_PAGE page, bool do_events);
  FPDF_PAGE LoadPageCommon(int page_index, bool do_events);

  std::unique_ptr<Delegate> default_delegate_;
  Delegate* delegate_;

#ifdef PDF_ENABLE_XFA
  int form_fill_info_version_ = 2;
#else   // PDF_ENABLE_XFA
  int form_fill_info_version_ = 1;
#endif  // PDF_ENABLE_XFA

  // must outlive `loader_`.
  std::vector<uint8_t> file_contents_;
  std::unique_ptr<TestLoader> loader_;
  FPDF_FILEACCESS file_access_;                       // must outlive `avail_`.
  std::unique_ptr<FakeFileAccess> fake_file_access_;  // must outlive `avail_`.
  ScopedFPDFAvail avail_;
  ScopedFPDFDocument document_;
  ScopedFPDFFormHandle form_handle_;
  PageNumberToHandleMap page_map_;

  FPDF_FILEACCESS saved_file_access_;  // must outlive `saved_avail_`.
  // must outlive `saved_avail_`.
  std::unique_ptr<FakeFileAccess> saved_fake_file_access_;
  ScopedFPDFAvail saved_avail_;
  ScopedFPDFDocument saved_document_;
  ScopedFPDFFormHandle saved_form_handle_;
  PageNumberToHandleMap saved_page_map_;

  std::string data_string_;
  std::string saved_document_file_data_;
  std::ofstream filestream_;
};

#endif  // TESTING_EMBEDDER_TEST_H_
