// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/embedder_test.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "core/fdrm/fx_crypt.h"
#include "public/cpp/fpdf_scopers.h"
#include "public/fpdf_dataavail.h"
#include "public/fpdf_edit.h"
#include "public/fpdf_text.h"
#include "public/fpdfview.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/test_loader.h"
#include "testing/utils/bitmap_saver.h"
#include "testing/utils/file_util.h"
#include "testing/utils/hash.h"
#include "testing/utils/path_service.h"
#include "third_party/base/check.h"
#include "third_party/base/containers/contains.h"
#include "third_party/base/notreached.h"

#ifdef PDF_ENABLE_V8
#include "testing/v8_test_environment.h"
#include "v8/include/v8-platform.h"
#include "v8/include/v8.h"
#endif  // PDF_ENABLE_V8

namespace {

int GetBitmapBytesPerPixel(FPDF_BITMAP bitmap) {
  return EmbedderTest::BytesPerPixelForFormat(FPDFBitmap_GetFormat(bitmap));
}

#if BUILDFLAG(IS_WIN)
int CALLBACK GetRecordProc(HDC hdc,
                           HANDLETABLE* handle_table,
                           const ENHMETARECORD* record,
                           int objects_count,
                           LPARAM param) {
  auto& records = *reinterpret_cast<std::vector<const ENHMETARECORD*>*>(param);
  records.push_back(record);
  return 1;
}
#endif  // BUILDFLAG(IS_WIN)

}  // namespace

EmbedderTest::EmbedderTest()
    : default_delegate_(std::make_unique<EmbedderTest::Delegate>()),
      delegate_(default_delegate_.get()) {
  FPDF_FILEWRITE::version = 1;
  FPDF_FILEWRITE::WriteBlock = WriteBlockCallback;
}

EmbedderTest::~EmbedderTest() = default;

void EmbedderTest::SetUp() {
  UNSUPPORT_INFO* info = static_cast<UNSUPPORT_INFO*>(this);
  memset(info, 0, sizeof(UNSUPPORT_INFO));
  info->version = 1;
  info->FSDK_UnSupport_Handler = UnsupportedHandlerTrampoline;
  FSDK_SetUnSpObjProcessHandler(info);
}

void EmbedderTest::TearDown() {
  // Use an EXPECT_EQ() here and continue to let TearDown() finish as cleanly as
  // possible. This can fail when an DCHECK test fails in a test case.
  EXPECT_EQ(0U, page_map_.size());
  EXPECT_EQ(0U, saved_page_map_.size());
  if (document())
    CloseDocument();
}

void EmbedderTest::CreateEmptyDocument() {
  CreateEmptyDocumentWithoutFormFillEnvironment();
  form_handle_.reset(SetupFormFillEnvironment(
      document(), JavaScriptOption::kEnableJavaScript));
}

void EmbedderTest::CreateEmptyDocumentWithoutFormFillEnvironment() {
  document_.reset(FPDF_CreateNewDocument());
  DCHECK(document_);
}

bool EmbedderTest::OpenDocument(const std::string& filename) {
  return OpenDocumentWithOptions(filename, nullptr,
                                 LinearizeOption::kDefaultLinearize,
                                 JavaScriptOption::kEnableJavaScript);
}

bool EmbedderTest::OpenDocumentLinearized(const std::string& filename) {
  return OpenDocumentWithOptions(filename, nullptr,
                                 LinearizeOption::kMustLinearize,
                                 JavaScriptOption::kEnableJavaScript);
}

bool EmbedderTest::OpenDocumentWithPassword(const std::string& filename,
                                            const char* password) {
  return OpenDocumentWithOptions(filename, password,
                                 LinearizeOption::kDefaultLinearize,
                                 JavaScriptOption::kEnableJavaScript);
}

bool EmbedderTest::OpenDocumentWithoutJavaScript(const std::string& filename) {
  return OpenDocumentWithOptions(filename, nullptr,
                                 LinearizeOption::kDefaultLinearize,
                                 JavaScriptOption::kDisableJavaScript);
}

bool EmbedderTest::OpenDocumentWithOptions(const std::string& filename,
                                           const char* password,
                                           LinearizeOption linearize_option,
                                           JavaScriptOption javascript_option) {
  std::string file_path;
  if (!PathService::GetTestFilePath(filename, &file_path))
    return false;

  file_contents_ = GetFileContents(file_path.c_str(), &file_length_);
  if (!file_contents_)
    return false;

  EXPECT_TRUE(!loader_);
  loader_ = std::make_unique<TestLoader>(
      pdfium::make_span(file_contents_.get(), file_length_));

  memset(&file_access_, 0, sizeof(file_access_));
  file_access_.m_FileLen = static_cast<unsigned long>(file_length_);
  file_access_.m_GetBlock = TestLoader::GetBlock;
  file_access_.m_Param = loader_.get();

  fake_file_access_ = std::make_unique<FakeFileAccess>(&file_access_);
  return OpenDocumentHelper(password, linearize_option, javascript_option,
                            fake_file_access_.get(), &document_, &avail_,
                            &form_handle_);
}

bool EmbedderTest::OpenDocumentHelper(const char* password,
                                      LinearizeOption linearize_option,
                                      JavaScriptOption javascript_option,
                                      FakeFileAccess* network_simulator,
                                      ScopedFPDFDocument* document,
                                      ScopedFPDFAvail* avail,
                                      ScopedFPDFFormHandle* form_handle) {
  network_simulator->AddSegment(0, 1024);
  network_simulator->SetRequestedDataAvailable();
  avail->reset(FPDFAvail_Create(network_simulator->GetFileAvail(),
                                network_simulator->GetFileAccess()));
  FPDF_AVAIL avail_ptr = avail->get();
  FPDF_DOCUMENT document_ptr = nullptr;
  if (FPDFAvail_IsLinearized(avail_ptr) == PDF_LINEARIZED) {
    int32_t nRet = PDF_DATA_NOTAVAIL;
    while (nRet == PDF_DATA_NOTAVAIL) {
      network_simulator->SetRequestedDataAvailable();
      nRet = FPDFAvail_IsDocAvail(avail_ptr,
                                  network_simulator->GetDownloadHints());
    }
    if (nRet == PDF_DATA_ERROR)
      return false;

    document->reset(FPDFAvail_GetDocument(avail_ptr, password));
    document_ptr = document->get();
    if (!document_ptr)
      return false;

    nRet = PDF_DATA_NOTAVAIL;
    while (nRet == PDF_DATA_NOTAVAIL) {
      network_simulator->SetRequestedDataAvailable();
      nRet = FPDFAvail_IsFormAvail(avail_ptr,
                                   network_simulator->GetDownloadHints());
    }
    if (nRet == PDF_FORM_ERROR)
      return false;

    int page_count = FPDF_GetPageCount(document_ptr);
    for (int i = 0; i < page_count; ++i) {
      nRet = PDF_DATA_NOTAVAIL;
      while (nRet == PDF_DATA_NOTAVAIL) {
        network_simulator->SetRequestedDataAvailable();
        nRet = FPDFAvail_IsPageAvail(avail_ptr, i,
                                     network_simulator->GetDownloadHints());
      }
      if (nRet == PDF_DATA_ERROR)
        return false;
    }
  } else {
    if (linearize_option == LinearizeOption::kMustLinearize)
      return false;
    network_simulator->SetWholeFileAvailable();
    document->reset(
        FPDF_LoadCustomDocument(network_simulator->GetFileAccess(), password));
    document_ptr = document->get();
    if (!document_ptr)
      return false;
  }
  form_handle->reset(SetupFormFillEnvironment(document_ptr, javascript_option));

  int doc_type = FPDF_GetFormType(document_ptr);
  if (doc_type == FORMTYPE_XFA_FULL || doc_type == FORMTYPE_XFA_FOREGROUND)
    FPDF_LoadXFA(document_ptr);

  (void)FPDF_GetDocPermissions(document_ptr);
  return true;
}

void EmbedderTest::CloseDocument() {
  FORM_DoDocumentAAction(form_handle(), FPDFDOC_AACTION_WC);
  form_handle_.reset();
  document_.reset();
  avail_.reset();
  fake_file_access_.reset();
  memset(&file_access_, 0, sizeof(file_access_));
  loader_.reset();
  file_contents_.reset();
}

FPDF_FORMHANDLE EmbedderTest::SetupFormFillEnvironment(
    FPDF_DOCUMENT doc,
    JavaScriptOption javascript_option) {
  IPDF_JSPLATFORM* platform = static_cast<IPDF_JSPLATFORM*>(this);
  memset(platform, '\0', sizeof(IPDF_JSPLATFORM));
  platform->version = 3;
  platform->app_alert = AlertTrampoline;

  FPDF_FORMFILLINFO* formfillinfo = static_cast<FPDF_FORMFILLINFO*>(this);
  memset(formfillinfo, 0, sizeof(FPDF_FORMFILLINFO));
  formfillinfo->version = form_fill_info_version_;
  formfillinfo->FFI_SetTimer = SetTimerTrampoline;
  formfillinfo->FFI_KillTimer = KillTimerTrampoline;
  formfillinfo->FFI_GetPage = GetPageTrampoline;
  formfillinfo->FFI_DoURIAction = DoURIActionTrampoline;
  formfillinfo->FFI_DoGoToAction = DoGoToActionTrampoline;
  formfillinfo->FFI_OnFocusChange = OnFocusChangeTrampoline;
  formfillinfo->FFI_DoURIActionWithKeyboardModifier =
      DoURIActionWithKeyboardModifierTrampoline;

  if (javascript_option == JavaScriptOption::kEnableJavaScript)
    formfillinfo->m_pJsPlatform = platform;

  FPDF_FORMHANDLE form_handle =
      FPDFDOC_InitFormFillEnvironment(doc, formfillinfo);
  SetInitialFormFieldHighlight(form_handle);
  return form_handle;
}

void EmbedderTest::DoOpenActions() {
  DCHECK(form_handle());
  FORM_DoDocumentJSAction(form_handle());
  FORM_DoDocumentOpenAction(form_handle());
}

int EmbedderTest::GetFirstPageNum() {
  int first_page = FPDFAvail_GetFirstPageNum(document());
  (void)FPDFAvail_IsPageAvail(avail(), first_page,
                              fake_file_access_->GetDownloadHints());
  return first_page;
}

int EmbedderTest::GetPageCount() {
  int page_count = FPDF_GetPageCount(document());
  for (int i = 0; i < page_count; ++i)
    (void)FPDFAvail_IsPageAvail(avail(), i,
                                fake_file_access_->GetDownloadHints());
  return page_count;
}

FPDF_PAGE EmbedderTest::LoadPage(int page_number) {
  return LoadPageCommon(page_number, true);
}

FPDF_PAGE EmbedderTest::LoadPageNoEvents(int page_number) {
  return LoadPageCommon(page_number, false);
}

FPDF_PAGE EmbedderTest::LoadPageCommon(int page_number, bool do_events) {
  DCHECK(form_handle());
  DCHECK(page_number >= 0);
  DCHECK(!pdfium::Contains(page_map_, page_number));

  FPDF_PAGE page = FPDF_LoadPage(document(), page_number);
  if (!page)
    return nullptr;

  if (do_events) {
    FORM_OnAfterLoadPage(page, form_handle());
    FORM_DoPageAAction(page, form_handle(), FPDFPAGE_AACTION_OPEN);
  }
  page_map_[page_number] = page;
  return page;
}

void EmbedderTest::UnloadPage(FPDF_PAGE page) {
  UnloadPageCommon(page, true);
}

void EmbedderTest::UnloadPageNoEvents(FPDF_PAGE page) {
  UnloadPageCommon(page, false);
}

void EmbedderTest::UnloadPageCommon(FPDF_PAGE page, bool do_events) {
  DCHECK(form_handle());
  int page_number = GetPageNumberForLoadedPage(page);
  if (page_number < 0) {
    NOTREACHED();
    return;
  }
  if (do_events) {
    FORM_DoPageAAction(page, form_handle(), FPDFPAGE_AACTION_CLOSE);
    FORM_OnBeforeClosePage(page, form_handle());
  }
  FPDF_ClosePage(page);
  page_map_.erase(page_number);
}

void EmbedderTest::SetInitialFormFieldHighlight(FPDF_FORMHANDLE form) {
  FPDF_SetFormFieldHighlightColor(form, FPDF_FORMFIELD_UNKNOWN, 0xFFE4DD);
  FPDF_SetFormFieldHighlightAlpha(form, 100);
}

ScopedFPDFBitmap EmbedderTest::RenderLoadedPage(FPDF_PAGE page) {
  return RenderLoadedPageWithFlags(page, 0);
}

ScopedFPDFBitmap EmbedderTest::RenderLoadedPageWithFlags(FPDF_PAGE page,
                                                         int flags) {
  if (GetPageNumberForLoadedPage(page) < 0) {
    NOTREACHED();
    return nullptr;
  }
  return RenderPageWithFlags(page, form_handle(), flags);
}

ScopedFPDFBitmap EmbedderTest::RenderSavedPage(FPDF_PAGE page) {
  return RenderSavedPageWithFlags(page, 0);
}

ScopedFPDFBitmap EmbedderTest::RenderSavedPageWithFlags(FPDF_PAGE page,
                                                        int flags) {
  if (GetPageNumberForSavedPage(page) < 0) {
    NOTREACHED();
    return nullptr;
  }
  return RenderPageWithFlags(page, saved_form_handle(), flags);
}

// static
ScopedFPDFBitmap EmbedderTest::RenderPageWithFlags(FPDF_PAGE page,
                                                   FPDF_FORMHANDLE handle,
                                                   int flags) {
  int width = static_cast<int>(FPDF_GetPageWidthF(page));
  int height = static_cast<int>(FPDF_GetPageHeightF(page));
  int alpha = FPDFPage_HasTransparency(page) ? 1 : 0;
  ScopedFPDFBitmap bitmap(FPDFBitmap_Create(width, height, alpha));
  FPDF_DWORD fill_color = alpha ? 0x00000000 : 0xFFFFFFFF;
  FPDFBitmap_FillRect(bitmap.get(), 0, 0, width, height, fill_color);
  FPDF_RenderPageBitmap(bitmap.get(), page, 0, 0, width, height, 0, flags);
  FPDF_FFLDraw(handle, bitmap.get(), page, 0, 0, width, height, 0, flags);
  return bitmap;
}

// static
ScopedFPDFBitmap EmbedderTest::RenderPage(FPDF_PAGE page) {
  return RenderPageWithFlags(page, nullptr, 0);
}

#if BUILDFLAG(IS_WIN)
// static
std::vector<uint8_t> EmbedderTest::RenderPageWithFlagsToEmf(FPDF_PAGE page,
                                                            int flags) {
  HDC dc = CreateEnhMetaFileA(nullptr, nullptr, nullptr, nullptr);

  int width = static_cast<int>(FPDF_GetPageWidthF(page));
  int height = static_cast<int>(FPDF_GetPageHeightF(page));
  HRGN rgn = CreateRectRgn(0, 0, width, height);
  SelectClipRgn(dc, rgn);
  DeleteObject(rgn);

  SelectObject(dc, GetStockObject(NULL_PEN));
  SelectObject(dc, GetStockObject(WHITE_BRUSH));
  // If a PS_NULL pen is used, the dimensions of the rectangle are 1 pixel less.
  Rectangle(dc, 0, 0, width + 1, height + 1);

  FPDF_RenderPage(dc, page, 0, 0, width, height, 0, flags);

  HENHMETAFILE emf = CloseEnhMetaFile(dc);
  size_t size_in_bytes = GetEnhMetaFileBits(emf, 0, nullptr);
  std::vector<uint8_t> buffer(size_in_bytes);
  GetEnhMetaFileBits(emf, size_in_bytes, buffer.data());
  DeleteEnhMetaFile(emf);
  return buffer;
}

// static
std::string EmbedderTest::GetPostScriptFromEmf(
    pdfium::span<const uint8_t> emf_data) {
  // This comes from Emf::InitFromData() in Chromium.
  HENHMETAFILE emf = SetEnhMetaFileBits(emf_data.size(), emf_data.data());
  if (!emf)
    return std::string();

  // This comes from Emf::Enumerator::Enumerator() in Chromium.
  std::vector<const ENHMETARECORD*> records;
  if (!EnumEnhMetaFile(nullptr, emf, &GetRecordProc, &records, nullptr)) {
    DeleteEnhMetaFile(emf);
    return std::string();
  }

  // This comes from PostScriptMetaFile::SafePlayback() in Chromium.
  std::string ps_data;
  for (const auto* record : records) {
    if (record->iType != EMR_GDICOMMENT)
      continue;

    // PostScript data is encapsulated inside EMF comment records.
    // The first two bytes of the comment indicate the string length. The rest
    // is the actual string data.
    const auto* comment = reinterpret_cast<const EMRGDICOMMENT*>(record);
    const char* data = reinterpret_cast<const char*>(comment->Data);
    uint16_t size = *reinterpret_cast<const uint16_t*>(data);
    data += 2;
    ps_data.append(data, size);
  }
  DeleteEnhMetaFile(emf);
  return ps_data;
}
#endif  // BUILDFLAG(IS_WIN)

FPDF_DOCUMENT EmbedderTest::OpenSavedDocument() {
  return OpenSavedDocumentWithPassword(nullptr);
}

// static
int EmbedderTest::BytesPerPixelForFormat(int format) {
  switch (format) {
    case FPDFBitmap_Gray:
      return 1;
    case FPDFBitmap_BGR:
      return 3;
    case FPDFBitmap_BGRx:
    case FPDFBitmap_BGRA:
      return 4;
    default:
      NOTREACHED();
      return 0;
  }
}

FPDF_DOCUMENT EmbedderTest::OpenSavedDocumentWithPassword(
    const char* password) {
  memset(&saved_file_access_, 0, sizeof(saved_file_access_));
  saved_file_access_.m_FileLen = data_string_.size();
  saved_file_access_.m_GetBlock = GetBlockFromString;
  // Copy data to prevent clearing it before saved document close.
  saved_document_file_data_ = data_string_;
  saved_file_access_.m_Param = &saved_document_file_data_;

  saved_fake_file_access_ =
      std::make_unique<FakeFileAccess>(&saved_file_access_);

  EXPECT_TRUE(OpenDocumentHelper(
      password, LinearizeOption::kDefaultLinearize,
      JavaScriptOption::kEnableJavaScript, saved_fake_file_access_.get(),
      &saved_document_, &saved_avail_, &saved_form_handle_));
  return saved_document();
}

void EmbedderTest::CloseSavedDocument() {
  DCHECK(saved_document());

  saved_form_handle_.reset();
  saved_document_.reset();
  saved_avail_.reset();
}

FPDF_PAGE EmbedderTest::LoadSavedPage(int page_number) {
  DCHECK(saved_form_handle());
  DCHECK(page_number >= 0);
  DCHECK(!pdfium::Contains(saved_page_map_, page_number));

  FPDF_PAGE page = FPDF_LoadPage(saved_document(), page_number);
  if (!page)
    return nullptr;

  FORM_OnAfterLoadPage(page, saved_form_handle());
  FORM_DoPageAAction(page, saved_form_handle(), FPDFPAGE_AACTION_OPEN);
  saved_page_map_[page_number] = page;
  return page;
}

void EmbedderTest::CloseSavedPage(FPDF_PAGE page) {
  DCHECK(saved_form_handle());

  int page_number = GetPageNumberForSavedPage(page);
  if (page_number < 0) {
    NOTREACHED();
    return;
  }

  FORM_DoPageAAction(page, saved_form_handle(), FPDFPAGE_AACTION_CLOSE);
  FORM_OnBeforeClosePage(page, saved_form_handle());
  FPDF_ClosePage(page);

  saved_page_map_.erase(page_number);
}

void EmbedderTest::VerifySavedRendering(FPDF_PAGE page,
                                        int width,
                                        int height,
                                        const char* md5) {
  DCHECK(saved_document());
  DCHECK(page);

  ScopedFPDFBitmap bitmap = RenderSavedPageWithFlags(page, FPDF_ANNOT);
  CompareBitmap(bitmap.get(), width, height, md5);
}

void EmbedderTest::VerifySavedDocument(int width, int height, const char* md5) {
  ASSERT_TRUE(OpenSavedDocument());
  FPDF_PAGE page = LoadSavedPage(0);
  VerifySavedRendering(page, width, height, md5);
  CloseSavedPage(page);
  CloseSavedDocument();
}

void EmbedderTest::SetWholeFileAvailable() {
  DCHECK(fake_file_access_);
  fake_file_access_->SetWholeFileAvailable();
}

void EmbedderTest::SetDocumentFromAvail() {
  document_.reset(FPDFAvail_GetDocument(avail(), nullptr));
}

void EmbedderTest::CreateAvail(FX_FILEAVAIL* file_avail,
                               FPDF_FILEACCESS* file) {
  avail_.reset(FPDFAvail_Create(file_avail, file));
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

// static
void EmbedderTest::DoURIActionTrampoline(FPDF_FORMFILLINFO* info,
                                         FPDF_BYTESTRING uri) {
  EmbedderTest* test = static_cast<EmbedderTest*>(info);
  return test->delegate_->DoURIAction(uri);
}

// static
void EmbedderTest::DoGoToActionTrampoline(FPDF_FORMFILLINFO* info,
                                          int page_index,
                                          int zoom_mode,
                                          float* pos_array,
                                          int array_size) {
  EmbedderTest* test = static_cast<EmbedderTest*>(info);
  return test->delegate_->DoGoToAction(info, page_index, zoom_mode, pos_array,
                                       array_size);
}

// static
void EmbedderTest::OnFocusChangeTrampoline(FPDF_FORMFILLINFO* info,
                                           FPDF_ANNOTATION annot,
                                           int page_index) {
  EmbedderTest* test = static_cast<EmbedderTest*>(info);
  return test->delegate_->OnFocusChange(info, annot, page_index);
}

// static
void EmbedderTest::DoURIActionWithKeyboardModifierTrampoline(
    FPDF_FORMFILLINFO* info,
    FPDF_BYTESTRING uri,
    int modifiers) {
  EmbedderTest* test = static_cast<EmbedderTest*>(info);
  return test->delegate_->DoURIActionWithKeyboardModifier(info, uri, modifiers);
}

// static
std::string EmbedderTest::HashBitmap(FPDF_BITMAP bitmap) {
  int stride = FPDFBitmap_GetStride(bitmap);
  int usable_bytes_per_row =
      GetBitmapBytesPerPixel(bitmap) * FPDFBitmap_GetWidth(bitmap);
  int height = FPDFBitmap_GetHeight(bitmap);
  auto span = pdfium::make_span(
      static_cast<uint8_t*>(FPDFBitmap_GetBuffer(bitmap)), stride * height);

  CRYPT_md5_context context = CRYPT_MD5Start();
  for (int i = 0; i < height; ++i)
    CRYPT_MD5Update(&context, span.subspan(i * stride, usable_bytes_per_row));
  uint8_t digest[16];
  CRYPT_MD5Finish(&context, digest);
  return CryptToBase16(digest);
}

#ifndef NDEBUG
// static
void EmbedderTest::WriteBitmapToPng(FPDF_BITMAP bitmap,
                                    const std::string& filename) {
  BitmapSaver::WriteBitmapToPng(bitmap, filename);
}
#endif

// static
void EmbedderTest::CompareBitmap(FPDF_BITMAP bitmap,
                                 int expected_width,
                                 int expected_height,
                                 const char* expected_md5sum) {
  ASSERT_EQ(expected_width, FPDFBitmap_GetWidth(bitmap));
  ASSERT_EQ(expected_height, FPDFBitmap_GetHeight(bitmap));

  // The expected stride is calculated using the same formula as in
  // CFX_DIBitmap::CalculatePitchAndSize(), which sets the bitmap stride.
  const int expected_stride =
      (expected_width * GetBitmapBytesPerPixel(bitmap) * 8 + 31) / 32 * 4;
  ASSERT_EQ(expected_stride, FPDFBitmap_GetStride(bitmap));

  if (!expected_md5sum)
    return;

  EXPECT_EQ(expected_md5sum, HashBitmap(bitmap));
}

// static
int EmbedderTest::WriteBlockCallback(FPDF_FILEWRITE* pFileWrite,
                                     const void* data,
                                     unsigned long size) {
  EmbedderTest* pThis = static_cast<EmbedderTest*>(pFileWrite);

  pThis->data_string_.append(static_cast<const char*>(data), size);

  if (pThis->filestream_.is_open())
    pThis->filestream_.write(static_cast<const char*>(data), size);

  return 1;
}

// static
int EmbedderTest::GetBlockFromString(void* param,
                                     unsigned long pos,
                                     unsigned char* buf,
                                     unsigned long size) {
  std::string* new_file = static_cast<std::string*>(param);
  if (!new_file || pos + size < pos) {
    NOTREACHED();
    return 0;
  }

  if (pos + size > new_file->size()) {
    NOTREACHED();
    return 0;
  }

  memcpy(buf, new_file->data() + pos, size);
  return 1;
}

// static
int EmbedderTest::GetPageNumberForPage(const PageNumberToHandleMap& page_map,
                                       FPDF_PAGE page) {
  for (const auto& it : page_map) {
    if (it.second == page) {
      int page_number = it.first;
      DCHECK(page_number >= 0);
      return page_number;
    }
  }
  return -1;
}

int EmbedderTest::GetPageNumberForLoadedPage(FPDF_PAGE page) const {
  return GetPageNumberForPage(page_map_, page);
}

int EmbedderTest::GetPageNumberForSavedPage(FPDF_PAGE page) const {
  return GetPageNumberForPage(saved_page_map_, page);
}

#ifndef NDEBUG
void EmbedderTest::OpenPDFFileForWrite(const std::string& filename) {
  filestream_.open(filename, std::ios_base::binary);
}

void EmbedderTest::ClosePDFFileForWrite() {
  filestream_.close();
}
#endif
