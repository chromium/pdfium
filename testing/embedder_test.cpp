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

#include "../fpdfsdk/include/fpdf_ext.h"
#include "../fpdfsdk/include/fpdftext.h"
#include "../fpdfsdk/include/fpdfview.h"
#include "../core/include/fxcrt/fx_system.h"
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
    return NULL;
  }
  (void) fseek(file, 0, SEEK_END);
  size_t file_length = ftell(file);
  if (!file_length) {
    return NULL;
  }
  (void) fseek(file, 0, SEEK_SET);
  char* buffer = (char*) malloc(file_length);
  if (!buffer) {
    return NULL;
  }
  size_t bytes_read = fread(buffer, 1, file_length, file);
  (void) fclose(file);
  if (bytes_read != file_length) {
    fprintf(stderr, "Failed to read: %s\n", filename);
    free(buffer);
    return NULL;
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

int Form_Alert(IPDF_JSPLATFORM*, FPDF_WIDESTRING, FPDF_WIDESTRING, int, int) {
  printf("Form_Alert called.\n");
  return 0;
}

void Unsupported_Handler(UNSUPPORT_INFO*, int type) {
  std::string feature = "Unknown";
  switch (type) {
    case FPDF_UNSP_DOC_XFAFORM:
      feature = "XFA";
      break;
    case FPDF_UNSP_DOC_PORTABLECOLLECTION:
      feature = "Portfolios_Packages";
      break;
    case FPDF_UNSP_DOC_ATTACHMENT:
    case FPDF_UNSP_ANNOT_ATTACHMENT:
      feature = "Attachment";
      break;
    case FPDF_UNSP_DOC_SECURITY:
      feature = "Rights_Management";
      break;
    case FPDF_UNSP_DOC_SHAREDREVIEW:
      feature = "Shared_Review";
      break;
    case FPDF_UNSP_DOC_SHAREDFORM_ACROBAT:
    case FPDF_UNSP_DOC_SHAREDFORM_FILESYSTEM:
    case FPDF_UNSP_DOC_SHAREDFORM_EMAIL:
      feature = "Shared_Form";
      break;
    case FPDF_UNSP_ANNOT_3DANNOT:
      feature = "3D";
      break;
    case FPDF_UNSP_ANNOT_MOVIE:
      feature = "Movie";
      break;
    case FPDF_UNSP_ANNOT_SOUND:
      feature = "Sound";
      break;
    case FPDF_UNSP_ANNOT_SCREEN_MEDIA:
    case FPDF_UNSP_ANNOT_SCREEN_RICHMEDIA:
      feature = "Screen";
      break;
    case FPDF_UNSP_ANNOT_SIG:
      feature = "Digital_Signature";
      break;
  }
  printf("Unsupported feature: %s.\n", feature.c_str());
}

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

bool Is_Data_Avail(FX_FILEAVAIL* pThis, size_t offset, size_t size) {
  return true;
}

void Add_Segment(FX_DOWNLOADHINTS* pThis, size_t offset, size_t size) {
}

void EmbedderTest::SetUp() {
    v8::V8::InitializeICU();

#ifdef V8_USE_EXTERNAL_STARTUP_DATA
    ASSERT_TRUE(GetExternalData(g_exe_path_, "natives_blob.bin", &natives_));
    ASSERT_TRUE(GetExternalData(g_exe_path_, "snapshot_blob.bin", &snapshot_));
    v8::V8::SetNativesDataBlob(&natives_);
    v8::V8::SetSnapshotDataBlob(&snapshot_);
#endif  // V8_USE_EXTERNAL_STARTUP_DATA

    FPDF_InitLibrary();

    UNSUPPORT_INFO unsuppored_info;
    memset(&unsuppored_info, '\0', sizeof(unsuppored_info));
    unsuppored_info.version = 1;
    unsuppored_info.FSDK_UnSupport_Handler = Unsupported_Handler;
    FSDK_SetUnSpObjProcessHandler(&unsuppored_info);
  }

void EmbedderTest::TearDown() {
  if (form_handle_) {
    FORM_DoDocumentAAction(form_handle_, FPDFDOC_AACTION_WC);
    FPDFDOC_ExitFormFillEnvironment(form_handle_);
  }
  if (document_) {
    FPDF_CloseDocument(document_);
  }
  FPDFAvail_Destroy(avail_);
  FPDF_DestroyLibrary();
  if (loader_) {
    delete loader_;
  }
  if (file_contents_) {
    free(file_contents_);
  }
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
    document_ = FPDF_LoadCustomDocument(&file_access_, NULL);
  } else {
    document_ = FPDFAvail_GetDocument(avail_, NULL);
  }

  (void) FPDF_GetDocPermissions(document_);
  (void) FPDFAvail_IsFormAvail(avail_, &hints_);

  IPDF_JSPLATFORM platform_callbacks;
  memset(&platform_callbacks, '\0', sizeof(platform_callbacks));
  platform_callbacks.version = 1;
  platform_callbacks.app_alert = Form_Alert;

  FPDF_FORMFILLINFO form_callbacks;
  memset(&form_callbacks, '\0', sizeof(form_callbacks));
  form_callbacks.version = 1;
  form_callbacks.m_pJsPlatform = &platform_callbacks;

  form_handle_ = FPDFDOC_InitFormFillEnvironment(document_, &form_callbacks);
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

// Can't use gtest-provided main since we need to stash the path to the
// executable in order to find the external V8 binary data files.
int main(int argc, char** argv) {
  g_exe_path_ = argv[0];
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
