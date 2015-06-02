// Copyright (c) 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "embedder_test.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <list>
#include <string>
#include <utility>
#include <vector>

#include "../core/include/fxcrt/fx_system.h"
#include "../public/fpdf_text.h"
#include "../public/fpdfview.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "v8/include/libplatform/libplatform.h"
#include "v8/include/v8.h"

#ifdef _WIN32
#define snprintf _snprintf
#define PATH_SEPARATOR '\\'
#else
#define PATH_SEPARATOR '/'
#endif

namespace {

const char* g_exe_path_ = nullptr;

// Reads the entire contents of a file into a newly malloc'd buffer.
static char* GetFileContents(const char* filename, size_t* retlen) {
  FILE* file = fopen(filename, "rb");
  if (!file) {
    fprintf(stderr, "Failed to open: %s\n", filename);
    return nullptr;
  }
  (void) fseek(file, 0, SEEK_END);
  size_t file_length = ftell(file);
  if (!file_length) {
    return nullptr;
  }
  (void) fseek(file, 0, SEEK_SET);
  char* buffer = (char*) malloc(file_length);
  if (!buffer) {
    return nullptr;
  }
  size_t bytes_read = fread(buffer, 1, file_length, file);
  (void) fclose(file);
  if (bytes_read != file_length) {
    fprintf(stderr, "Failed to read: %s\n", filename);
    free(buffer);
    return nullptr;
  }
  *retlen = bytes_read;
  return buffer;
}

#ifdef V8_USE_EXTERNAL_STARTUP_DATA
// Returns the full path for an external V8 data file based on either
// the currect exectuable path or an explicit override.
static std::string GetFullPathForSnapshotFile(const std::string& exe_path,
                                              const std::string& filename) {
  std::string result;
  if (!exe_path.empty()) {
    size_t last_separator = exe_path.rfind(PATH_SEPARATOR);
    if (last_separator != std::string::npos)  {
      result = exe_path.substr(0, last_separator + 1);
    }
  }
  result += filename;
  return result;
}

// Reads an extenal V8 data file from the |options|-indicated location,
// returing true on success and false on error.
static bool GetExternalData(const std::string& exe_path,
                            const std::string& filename,
                            v8::StartupData* result_data) {
  std::string full_path = GetFullPathForSnapshotFile(exe_path, filename);
  size_t data_length = 0;
  char* data_buffer = GetFileContents(full_path.c_str(), &data_length);
  if (!data_buffer) {
    return false;
  }
  result_data->data = const_cast<const char*>(data_buffer);
  result_data->raw_size = data_length;
  return true;
}
#endif  // V8_USE_EXTERNAL_STARTUP_DATA

}  // namespace

class TestLoader {
 public:
  TestLoader(const char* pBuf, size_t len);

  const char* m_pBuf;
  size_t m_Len;
};

TestLoader::TestLoader(const char* pBuf, size_t len)
    : m_pBuf(pBuf), m_Len(len) {
}

int Get_Block(void* param, unsigned long pos, unsigned char* pBuf,
              unsigned long size) {
  TestLoader* pLoader = (TestLoader*) param;
  if (pos + size < pos || pos + size > pLoader->m_Len) return 0;
  memcpy(pBuf, pLoader->m_pBuf + pos, size);
  return 1;
}

FPDF_BOOL Is_Data_Avail(FX_FILEAVAIL* pThis, size_t offset, size_t size) {
  return true;
}

void Add_Segment(FX_DOWNLOADHINTS* pThis, size_t offset, size_t size) {
}

EmbedderTest::EmbedderTest() :
      document_(nullptr),
      form_handle_(nullptr),
      avail_(nullptr),
      loader_(nullptr),
      file_length_(0),
      file_contents_(nullptr) {
  memset(&hints_, 0, sizeof(hints_));
  memset(&file_access_, 0, sizeof(file_access_));
  memset(&file_avail_, 0, sizeof(file_avail_));
  default_delegate_ = new EmbedderTest::Delegate();
  delegate_ = default_delegate_;
}

EmbedderTest::~EmbedderTest() {
  delete default_delegate_;
}

void EmbedderTest::SetUp() {
    v8::V8::InitializeICU();

    platform_ = v8::platform::CreateDefaultPlatform();
    v8::V8::InitializePlatform(platform_);
    v8::V8::Initialize();

    // By enabling predictable mode, V8 won't post any background tasks.
    const char predictable_flag[] = "--predictable";
    v8::V8::SetFlagsFromString(predictable_flag,
                               static_cast<int>(strlen(predictable_flag)));

#ifdef V8_USE_EXTERNAL_STARTUP_DATA
    ASSERT_TRUE(GetExternalData(g_exe_path_, "natives_blob.bin", &natives_));
    ASSERT_TRUE(GetExternalData(g_exe_path_, "snapshot_blob.bin", &snapshot_));
    v8::V8::SetNativesDataBlob(&natives_);
    v8::V8::SetSnapshotDataBlob(&snapshot_);
#endif  // V8_USE_EXTERNAL_STARTUP_DATA

    FPDF_InitLibrary();

    UNSUPPORT_INFO* info = static_cast<UNSUPPORT_INFO*>(this);
    memset(info, 0, sizeof(UNSUPPORT_INFO));
    info->version = 1;
    info->FSDK_UnSupport_Handler = UnsupportedHandlerTrampoline;
    FSDK_SetUnSpObjProcessHandler(info);
  }

void EmbedderTest::TearDown() {
  if (document_) {
    FORM_DoDocumentAAction(form_handle_, FPDFDOC_AACTION_WC);
    FPDF_CloseDocument(document_);
    FPDFDOC_ExitFormFillEnvironment(form_handle_);
  }
  FPDFAvail_Destroy(avail_);
  FPDF_DestroyLibrary();
  v8::V8::ShutdownPlatform();
  delete platform_;
  delete loader_;
  free(file_contents_);
}

bool EmbedderTest::OpenDocument(const std::string& filename) {
  file_contents_ = GetFileContents(filename.c_str(), &file_length_);
  if (!file_contents_) {
    return false;
  }

  loader_ = new TestLoader(file_contents_, file_length_);
  file_access_.m_FileLen = static_cast<unsigned long>(file_length_);
  file_access_.m_GetBlock = Get_Block;
  file_access_.m_Param = loader_;

  file_avail_.version = 1;
  file_avail_.IsDataAvail = Is_Data_Avail;

  hints_.version = 1;
  hints_.AddSegment = Add_Segment;

  avail_ = FPDFAvail_Create(&file_avail_, &file_access_);
  (void) FPDFAvail_IsDocAvail(avail_, &hints_);

  if (!FPDFAvail_IsLinearized(avail_)) {
    document_ = FPDF_LoadCustomDocument(&file_access_, nullptr);
  } else {
    document_ = FPDFAvail_GetDocument(avail_, nullptr);
  }

  (void) FPDF_GetDocPermissions(document_);
  (void) FPDFAvail_IsFormAvail(avail_, &hints_);

  IPDF_JSPLATFORM* platform = static_cast<IPDF_JSPLATFORM*>(this);
  memset(platform, 0, sizeof(IPDF_JSPLATFORM));
  platform->version = 1;
  platform->app_alert = AlertTrampoline;

  FPDF_FORMFILLINFO* formfillinfo = static_cast<FPDF_FORMFILLINFO*>(this);
  memset(formfillinfo, 0, sizeof(FPDF_FORMFILLINFO));
  formfillinfo->version = 1;
  formfillinfo->FFI_SetTimer = SetTimerTrampoline;
  formfillinfo->FFI_KillTimer = KillTimerTrampoline;
  formfillinfo->m_pJsPlatform = platform;

  form_handle_ = FPDFDOC_InitFormFillEnvironment(document_, formfillinfo);
  FPDF_SetFormFieldHighlightColor(form_handle_, 0, 0xFFE4DD);
  FPDF_SetFormFieldHighlightAlpha(form_handle_, 100);

  return true;
}

void EmbedderTest::DoOpenActions() {
  FORM_DoDocumentJSAction(form_handle_);
  FORM_DoDocumentOpenAction(form_handle_);
}

int EmbedderTest::GetFirstPageNum() {
  int first_page = FPDFAvail_GetFirstPageNum(document_);
  (void) FPDFAvail_IsPageAvail(avail_, first_page, &hints_);
  return first_page;
}

int EmbedderTest::GetPageCount() {
  int page_count = FPDF_GetPageCount(document_);
  for (int i = 0; i < page_count; ++i) {
    (void) FPDFAvail_IsPageAvail(avail_, i, &hints_);
  }
  return page_count;
}

FPDF_PAGE EmbedderTest::LoadPage(int page_number) {
  FPDF_PAGE page = FPDF_LoadPage(document_, page_number);
  if (!page) {
    return nullptr;
  }
  FORM_OnAfterLoadPage(page, form_handle_);
  FORM_DoPageAAction(page, form_handle_, FPDFPAGE_AACTION_OPEN);
  return page;
}

FPDF_BITMAP EmbedderTest::RenderPage(FPDF_PAGE page) {
  int width = static_cast<int>(FPDF_GetPageWidth(page));
  int height = static_cast<int>(FPDF_GetPageHeight(page));
  FPDF_BITMAP bitmap = FPDFBitmap_Create(width, height, 0);
  FPDFBitmap_FillRect(bitmap, 0, 0, width, height, 0xFFFFFFFF);
  FPDF_RenderPageBitmap(bitmap, page, 0, 0, width, height, 0, 0);
  FPDF_FFLDraw(form_handle_, bitmap, page, 0, 0, width, height, 0, 0);
  return bitmap;
}

void EmbedderTest::UnloadPage(FPDF_PAGE page) {
  FORM_DoPageAAction(page, form_handle_, FPDFPAGE_AACTION_CLOSE);
  FORM_OnBeforeClosePage(page, form_handle_);
  FPDF_ClosePage(page);
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
                                     int msecs, TimerCallback fn) {
  EmbedderTest* test = static_cast<EmbedderTest*>(info);
  return test->delegate_->SetTimer(msecs, fn);
}

// static
void EmbedderTest::KillTimerTrampoline(FPDF_FORMFILLINFO* info, int id) {
  EmbedderTest* test = static_cast<EmbedderTest*>(info);
  return test->delegate_->KillTimer(id);
}

// Can't use gtest-provided main since we need to stash the path to the
// executable in order to find the external V8 binary data files.
int main(int argc, char** argv) {
  g_exe_path_ = argv[0];
  testing::InitGoogleTest(&argc, argv);
  testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}
