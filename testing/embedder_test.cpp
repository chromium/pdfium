// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/embedder_test.h"

#include <limits.h>

#include <list>
#include <string>
#include <utility>
#include <vector>

#include "core/fdrm/crypto/fx_crypt.h"
#include "public/fpdf_dataavail.h"
#include "public/fpdf_edit.h"
#include "public/fpdf_text.h"
#include "public/fpdfview.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/test_support.h"
#include "testing/utils/path_service.h"

#ifdef PDF_ENABLE_V8
#include "v8/include/v8-platform.h"
#include "v8/include/v8.h"
#endif  // PDF_ENABLE_V8

namespace {

const char* g_exe_path = nullptr;

#ifdef PDF_ENABLE_V8
#ifdef V8_USE_EXTERNAL_STARTUP_DATA
v8::StartupData* g_v8_natives = nullptr;
v8::StartupData* g_v8_snapshot = nullptr;
#endif  // V8_USE_EXTERNAL_STARTUP_DATA
#endif  // PDF_ENABLE_V8

FPDF_BOOL Is_Data_Avail(FX_FILEAVAIL* pThis, size_t offset, size_t size) {
  return true;
}

void Add_Segment(FX_DOWNLOADHINTS* pThis, size_t offset, size_t size) {}

}  // namespace

EmbedderTest::EmbedderTest()
    : default_delegate_(new EmbedderTest::Delegate()),
      document_(nullptr),
      form_handle_(nullptr),
      avail_(nullptr),
      external_isolate_(nullptr),
      loader_(nullptr),
      file_length_(0),
      file_contents_(nullptr) {
  memset(&hints_, 0, sizeof(hints_));
  memset(&file_access_, 0, sizeof(file_access_));
  memset(&file_avail_, 0, sizeof(file_avail_));
  delegate_ = default_delegate_.get();

#ifdef PDF_ENABLE_V8
#ifdef V8_USE_EXTERNAL_STARTUP_DATA
  if (g_v8_natives && g_v8_snapshot) {
    InitializeV8ForPDFium(g_exe_path, std::string(), nullptr, nullptr,
                          &platform_);
  } else {
    g_v8_natives = new v8::StartupData;
    g_v8_snapshot = new v8::StartupData;
    InitializeV8ForPDFium(g_exe_path, std::string(), g_v8_natives,
                          g_v8_snapshot, &platform_);
  }
#else
  InitializeV8ForPDFium(g_exe_path, &platform_);
#endif  // V8_USE_EXTERNAL_STARTUP_DATA
#endif  // FPDF_ENABLE_V8
  FPDF_FILEWRITE::version = 1;
  FPDF_FILEWRITE::WriteBlock = WriteBlockCallback;
}

EmbedderTest::~EmbedderTest() {
#ifdef PDF_ENABLE_V8
  v8::V8::ShutdownPlatform();
  delete platform_;
#endif  // PDF_ENABLE_V8
}

void EmbedderTest::SetUp() {
  FPDF_LIBRARY_CONFIG config;
  config.version = 2;
  config.m_pUserFontPaths = nullptr;
  config.m_v8EmbedderSlot = 0;
  config.m_pIsolate = external_isolate_;
  FPDF_InitLibraryWithConfig(&config);

  UNSUPPORT_INFO* info = static_cast<UNSUPPORT_INFO*>(this);
  memset(info, 0, sizeof(UNSUPPORT_INFO));
  info->version = 1;
  info->FSDK_UnSupport_Handler = UnsupportedHandlerTrampoline;
  FSDK_SetUnSpObjProcessHandler(info);
}

void EmbedderTest::TearDown() {
  if (document_) {
    FORM_DoDocumentAAction(form_handle_, FPDFDOC_AACTION_WC);
    FPDFDOC_ExitFormFillEnvironment(form_handle_);
    FPDF_CloseDocument(document_);
  }

  FPDFAvail_Destroy(avail_);
  FPDF_DestroyLibrary();
  delete loader_;
}

bool EmbedderTest::CreateEmptyDocument() {
  document_ = FPDF_CreateNewDocument();
  if (!document_)
    return false;

  form_handle_ = SetupFormFillEnvironment(document_);
  return true;
}

bool EmbedderTest::OpenDocument(const std::string& filename,
                                const char* password,
                                bool must_linearize) {
  std::string file_path;
  if (!PathService::GetTestFilePath(filename, &file_path))
    return false;
  file_contents_ = GetFileContents(file_path.c_str(), &file_length_);
  if (!file_contents_)
    return false;

  EXPECT_TRUE(!loader_);
  loader_ = new TestLoader(file_contents_.get(), file_length_);
  file_access_.m_FileLen = static_cast<unsigned long>(file_length_);
  file_access_.m_GetBlock = TestLoader::GetBlock;
  file_access_.m_Param = loader_;
  return OpenDocumentHelper(password, must_linearize, &file_avail_, &hints_,
                            &file_access_, &document_, &avail_, &form_handle_);
}

bool EmbedderTest::OpenDocumentHelper(const char* password,
                                      bool must_linearize,
                                      FX_FILEAVAIL* file_avail,
                                      FX_DOWNLOADHINTS* hints,
                                      FPDF_FILEACCESS* file_access,
                                      FPDF_DOCUMENT* document,
                                      FPDF_AVAIL* avail,
                                      FPDF_FORMHANDLE* form_handle) {
  file_avail->version = 1;
  file_avail->IsDataAvail = Is_Data_Avail;

  hints->version = 1;
  hints->AddSegment = Add_Segment;

  *avail = FPDFAvail_Create(file_avail, file_access);

  if (FPDFAvail_IsLinearized(*avail) == PDF_LINEARIZED) {
    *document = FPDFAvail_GetDocument(*avail, password);
    if (!*document)
      return false;

    int32_t nRet = PDF_DATA_NOTAVAIL;
    while (nRet == PDF_DATA_NOTAVAIL)
      nRet = FPDFAvail_IsDocAvail(*avail, hints);
    if (nRet == PDF_DATA_ERROR)
      return false;

    nRet = FPDFAvail_IsFormAvail(*avail, hints);
    if (nRet == PDF_FORM_ERROR || nRet == PDF_FORM_NOTAVAIL)
      return false;

    int page_count = FPDF_GetPageCount(*document);
    for (int i = 0; i < page_count; ++i) {
      nRet = PDF_DATA_NOTAVAIL;
      while (nRet == PDF_DATA_NOTAVAIL)
        nRet = FPDFAvail_IsPageAvail(*avail, i, hints);

      if (nRet == PDF_DATA_ERROR)
        return false;
    }
  } else {
    if (must_linearize)
      return false;

    *document = FPDF_LoadCustomDocument(file_access, password);
    if (!*document)
      return false;
  }
  *form_handle = SetupFormFillEnvironment(*document);
#ifdef PDF_ENABLE_XFA
  int docType = DOCTYPE_PDF;
  if (FPDF_HasXFAField(*document, &docType)) {
    if (docType != DOCTYPE_PDF)
      (void)FPDF_LoadXFA(*document);
  }
#endif  // PDF_ENABLE_XFA
  (void)FPDF_GetDocPermissions(*document);
  return true;
}

FPDF_FORMHANDLE EmbedderTest::SetupFormFillEnvironment(FPDF_DOCUMENT doc) {
  IPDF_JSPLATFORM* platform = static_cast<IPDF_JSPLATFORM*>(this);
  memset(platform, '\0', sizeof(IPDF_JSPLATFORM));
  platform->version = 2;
  platform->app_alert = AlertTrampoline;
  platform->m_isolate = external_isolate_;

  FPDF_FORMFILLINFO* formfillinfo = static_cast<FPDF_FORMFILLINFO*>(this);
  memset(formfillinfo, 0, sizeof(FPDF_FORMFILLINFO));
#ifdef PDF_ENABLE_XFA
  formfillinfo->version = 2;
#else   // PDF_ENABLE_XFA
  formfillinfo->version = 1;
#endif  // PDF_ENABLE_XFA
  formfillinfo->FFI_SetTimer = SetTimerTrampoline;
  formfillinfo->FFI_KillTimer = KillTimerTrampoline;
  formfillinfo->FFI_GetPage = GetPageTrampoline;
  formfillinfo->m_pJsPlatform = platform;
  FPDF_FORMHANDLE form_handle =
      FPDFDOC_InitFormFillEnvironment(doc, formfillinfo);
  FPDF_SetFormFieldHighlightColor(form_handle, 0, 0xFFE4DD);
  FPDF_SetFormFieldHighlightAlpha(form_handle, 100);
  return form_handle;
}

void EmbedderTest::DoOpenActions() {
  ASSERT(form_handle_);
  FORM_DoDocumentJSAction(form_handle_);
  FORM_DoDocumentOpenAction(form_handle_);
}

int EmbedderTest::GetFirstPageNum() {
  int first_page = FPDFAvail_GetFirstPageNum(document_);
  (void)FPDFAvail_IsPageAvail(avail_, first_page, &hints_);
  return first_page;
}

int EmbedderTest::GetPageCount() {
  int page_count = FPDF_GetPageCount(document_);
  for (int i = 0; i < page_count; ++i)
    (void)FPDFAvail_IsPageAvail(avail_, i, &hints_);
  return page_count;
}

FPDF_PAGE EmbedderTest::LoadPage(int page_number) {
  ASSERT(form_handle_);
  // First check whether it is loaded already.
  auto it = page_map_.find(page_number);
  if (it != page_map_.end())
    return it->second;

  FPDF_PAGE page = FPDF_LoadPage(document_, page_number);
  if (!page)
    return nullptr;

  FORM_OnAfterLoadPage(page, form_handle_);
  FORM_DoPageAAction(page, form_handle_, FPDFPAGE_AACTION_OPEN);
  // Cache the page.
  page_map_[page_number] = page;
  page_reverse_map_[page] = page_number;
  return page;
}

FPDF_BITMAP EmbedderTest::RenderPage(FPDF_PAGE page) {
  return RenderPageWithFlags(page, form_handle_, 0);
}

FPDF_BITMAP EmbedderTest::RenderPageWithFlags(FPDF_PAGE page,
                                              FPDF_FORMHANDLE handle,
                                              int flags) {
  int width = static_cast<int>(FPDF_GetPageWidth(page));
  int height = static_cast<int>(FPDF_GetPageHeight(page));
  int alpha = FPDFPage_HasTransparency(page) ? 1 : 0;
  FPDF_BITMAP bitmap = FPDFBitmap_Create(width, height, alpha);
  FPDF_DWORD fill_color = alpha ? 0x00000000 : 0xFFFFFFFF;
  FPDFBitmap_FillRect(bitmap, 0, 0, width, height, fill_color);
  FPDF_RenderPageBitmap(bitmap, page, 0, 0, width, height, 0, flags);
  FPDF_FFLDraw(handle, bitmap, page, 0, 0, width, height, 0, flags);
  return bitmap;
}

void EmbedderTest::UnloadPage(FPDF_PAGE page) {
  ASSERT(form_handle_);
  FORM_DoPageAAction(page, form_handle_, FPDFPAGE_AACTION_CLOSE);
  FORM_OnBeforeClosePage(page, form_handle_);
  FPDF_ClosePage(page);

  auto it = page_reverse_map_.find(page);
  if (it == page_reverse_map_.end())
    return;

  page_map_.erase(it->second);
  page_reverse_map_.erase(it);
}

void EmbedderTest::TestSaved(int width,
                             int height,
                             const char* md5,
                             const char* password) {
  FPDF_FILEACCESS file_access;
  memset(&file_access, 0, sizeof(file_access));
  file_access.m_FileLen = m_String.size();
  file_access.m_GetBlock = GetBlockFromString;
  file_access.m_Param = &m_String;
  FX_FILEAVAIL file_avail;
  FX_DOWNLOADHINTS hints;

  ASSERT_TRUE(OpenDocumentHelper(password, false, &file_avail, &hints,
                                 &file_access, &m_SavedDocument, &m_SavedAvail,
                                 &m_SavedForm));
  EXPECT_EQ(1, FPDF_GetPageCount(m_SavedDocument));
  m_SavedPage = FPDF_LoadPage(m_SavedDocument, 0);
  ASSERT_TRUE(m_SavedPage);
  FPDF_BITMAP new_bitmap =
      RenderPageWithFlags(m_SavedPage, m_SavedForm, FPDF_ANNOT);
  CompareBitmap(new_bitmap, width, height, md5);
  FPDFBitmap_Destroy(new_bitmap);
}

void EmbedderTest::CloseSaved() {
  FPDF_ClosePage(m_SavedPage);
  FPDFDOC_ExitFormFillEnvironment(m_SavedForm);
  FPDF_CloseDocument(m_SavedDocument);
  FPDFAvail_Destroy(m_SavedAvail);
}

void EmbedderTest::TestAndCloseSaved(int width, int height, const char* md5) {
  TestSaved(width, height, md5);
  CloseSaved();
}

FPDF_PAGE EmbedderTest::Delegate::GetPage(FPDF_FORMFILLINFO* info,
                                          FPDF_DOCUMENT document,
                                          int page_index) {
  EmbedderTest* test = static_cast<EmbedderTest*>(info);
  auto it = test->page_map_.find(page_index);
  return it != test->page_map_.end() ? it->second : nullptr;
}

// static
void EmbedderTest::UnsupportedHandlerTrampoline(UNSUPPORT_INFO* info,
                                                int type) {
  EmbedderTest* test = static_cast<EmbedderTest*>(info);
  test->delegate_->UnsupportedHandler(type);
}

// static
int EmbedderTest::AlertTrampoline(IPDF_JSPLATFORM* platform,
                                  FPDF_WIDESTRING message,
                                  FPDF_WIDESTRING title,
                                  int type,
                                  int icon) {
  EmbedderTest* test = static_cast<EmbedderTest*>(platform);
  return test->delegate_->Alert(message, title, type, icon);
}

// static
int EmbedderTest::SetTimerTrampoline(FPDF_FORMFILLINFO* info,
                                     int msecs,
                                     TimerCallback fn) {
  EmbedderTest* test = static_cast<EmbedderTest*>(info);
  return test->delegate_->SetTimer(msecs, fn);
}

// static
void EmbedderTest::KillTimerTrampoline(FPDF_FORMFILLINFO* info, int id) {
  EmbedderTest* test = static_cast<EmbedderTest*>(info);
  return test->delegate_->KillTimer(id);
}

// static
FPDF_PAGE EmbedderTest::GetPageTrampoline(FPDF_FORMFILLINFO* info,
                                          FPDF_DOCUMENT document,
                                          int page_index) {
  return static_cast<EmbedderTest*>(info)->delegate_->GetPage(info, document,
                                                              page_index);
}

std::string EmbedderTest::HashBitmap(FPDF_BITMAP bitmap,
                                     int expected_width,
                                     int expected_height) {
  uint8_t digest[16];
  CRYPT_MD5Generate(static_cast<uint8_t*>(FPDFBitmap_GetBuffer(bitmap)),
                    expected_width * 4 * expected_height, digest);
  return CryptToBase16(digest);
}

// static
void EmbedderTest::CompareBitmap(FPDF_BITMAP bitmap,
                                 int expected_width,
                                 int expected_height,
                                 const char* expected_md5sum) {
  ASSERT_EQ(expected_width, FPDFBitmap_GetWidth(bitmap));
  ASSERT_EQ(expected_height, FPDFBitmap_GetHeight(bitmap));
  const int expected_stride = expected_width * 4;
  ASSERT_EQ(expected_stride, FPDFBitmap_GetStride(bitmap));

  if (!expected_md5sum)
    return;

  EXPECT_EQ(expected_md5sum,
            HashBitmap(bitmap, expected_width, expected_height));
}

// static
int EmbedderTest::WriteBlockCallback(FPDF_FILEWRITE* pFileWrite,
                                     const void* data,
                                     unsigned long size) {
  EmbedderTest* pThis = static_cast<EmbedderTest*>(pFileWrite);
  pThis->m_String.append(static_cast<const char*>(data), size);
  return 1;
}

// static
int EmbedderTest::GetBlockFromString(void* param,
                                     unsigned long pos,
                                     unsigned char* buf,
                                     unsigned long size) {
  std::string* new_file = static_cast<std::string*>(param);
  if (!new_file || pos + size < pos)
    return 0;

  unsigned long file_size = new_file->size();
  if (pos + size > file_size)
    return 0;

  memcpy(buf, new_file->data() + pos, size);
  return 1;
}

// Can't use gtest-provided main since we need to stash the path to the
// executable in order to find the external V8 binary data files.
int main(int argc, char** argv) {
  g_exe_path = argv[0];
  testing::InitGoogleTest(&argc, argv);
  testing::InitGoogleMock(&argc, argv);
  int ret_val = RUN_ALL_TESTS();

#ifdef PDF_ENABLE_V8
#ifdef V8_USE_EXTERNAL_STARTUP_DATA
  if (g_v8_natives)
    free(const_cast<char*>(g_v8_natives->data));
  if (g_v8_snapshot)
    free(const_cast<char*>(g_v8_snapshot->data));
#endif  // V8_USE_EXTERNAL_STARTUP_DATA
#endif  // PDF_ENABLE_V8

  return ret_val;
}
