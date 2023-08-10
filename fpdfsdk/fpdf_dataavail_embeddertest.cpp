// Copyright 2015 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "core/fxcrt/bytestring.h"
#include "public/fpdf_doc.h"
#include "public/fpdfview.h"
#include "testing/embedder_test.h"
#include "testing/fx_string_testhelpers.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/range_set.h"
#include "testing/utils/file_util.h"
#include "testing/utils/path_service.h"

namespace {

class MockDownloadHints final : public FX_DOWNLOADHINTS {
 public:
  static void SAddSegment(FX_DOWNLOADHINTS* pThis, size_t offset, size_t size) {
  }

  MockDownloadHints() {
    FX_DOWNLOADHINTS::version = 1;
    FX_DOWNLOADHINTS::AddSegment = SAddSegment;
  }

  ~MockDownloadHints() = default;
};

class TestAsyncLoader final : public FX_DOWNLOADHINTS, FX_FILEAVAIL {
 public:
  explicit TestAsyncLoader(const std::string& file_name) {
    std::string file_path;
    if (!PathService::GetTestFilePath(file_name, &file_path))
      return;
    file_contents_ = GetFileContents(file_path.c_str(), &file_length_);
    if (!file_contents_)
      return;

    file_access_.m_FileLen = static_cast<unsigned long>(file_length_);
    file_access_.m_GetBlock = SGetBlock;
    file_access_.m_Param = this;

    FX_DOWNLOADHINTS::version = 1;
    FX_DOWNLOADHINTS::AddSegment = SAddSegment;

    FX_FILEAVAIL::version = 1;
    FX_FILEAVAIL::IsDataAvail = SIsDataAvail;
  }

  bool IsOpened() const { return !!file_contents_; }

  FPDF_FILEACCESS* file_access() { return &file_access_; }
  FX_DOWNLOADHINTS* hints() { return this; }
  FX_FILEAVAIL* file_avail() { return this; }

  const std::vector<std::pair<size_t, size_t>>& requested_segments() const {
    return requested_segments_;
  }

  size_t max_requested_bound() const { return max_requested_bound_; }

  void ClearRequestedSegments() {
    requested_segments_.clear();
    max_requested_bound_ = 0;
  }

  bool is_new_data_available() const { return is_new_data_available_; }
  void set_is_new_data_available(bool is_new_data_available) {
    is_new_data_available_ = is_new_data_available;
  }

  size_t max_already_available_bound() const {
    return available_ranges_.IsEmpty()
               ? 0
               : available_ranges_.ranges().rbegin()->second;
  }

  void FlushRequestedData() {
    for (const auto& it : requested_segments_) {
      SetDataAvailable(it.first, it.second);
    }
    ClearRequestedSegments();
  }

  char* file_contents() { return file_contents_.get(); }
  size_t file_length() const { return file_length_; }

 private:
  void SetDataAvailable(size_t start, size_t size) {
    available_ranges_.Union(RangeSet::Range(start, start + size));
  }

  bool CheckDataAlreadyAvailable(size_t start, size_t size) const {
    return available_ranges_.Contains(RangeSet::Range(start, start + size));
  }

  int GetBlockImpl(unsigned long pos, unsigned char* pBuf, unsigned long size) {
    if (!IsDataAvailImpl(pos, size))
      return 0;
    const unsigned long end =
        std::min(static_cast<unsigned long>(file_length_), pos + size);
    if (end <= pos)
      return 0;
    memcpy(pBuf, file_contents_.get() + pos, end - pos);
    SetDataAvailable(pos, end - pos);
    return static_cast<int>(end - pos);
  }

  void AddSegmentImpl(size_t offset, size_t size) {
    requested_segments_.emplace_back(offset, size);
    max_requested_bound_ = std::max(max_requested_bound_, offset + size);
  }

  bool IsDataAvailImpl(size_t offset, size_t size) {
    if (offset + size > file_length_)
      return false;
    if (is_new_data_available_) {
      SetDataAvailable(offset, size);
      return true;
    }
    return CheckDataAlreadyAvailable(offset, size);
  }

  static int SGetBlock(void* param,
                       unsigned long pos,
                       unsigned char* pBuf,
                       unsigned long size) {
    return static_cast<TestAsyncLoader*>(param)->GetBlockImpl(pos, pBuf, size);
  }

  static void SAddSegment(FX_DOWNLOADHINTS* pThis, size_t offset, size_t size) {
    return static_cast<TestAsyncLoader*>(pThis)->AddSegmentImpl(offset, size);
  }

  static FPDF_BOOL SIsDataAvail(FX_FILEAVAIL* pThis,
                                size_t offset,
                                size_t size) {
    return static_cast<TestAsyncLoader*>(pThis)->IsDataAvailImpl(offset, size);
  }

  FPDF_FILEACCESS file_access_;

  std::unique_ptr<char, pdfium::FreeDeleter> file_contents_;
  size_t file_length_ = 0;
  std::vector<std::pair<size_t, size_t>> requested_segments_;
  size_t max_requested_bound_ = 0;
  bool is_new_data_available_ = true;

  RangeSet available_ranges_;
};

}  // namespace

class FPDFDataAvailEmbedderTest : public EmbedderTest {};

TEST_F(FPDFDataAvailEmbedderTest, TrailerUnterminated) {
  // Document must load without crashing but is too malformed to be available.
  EXPECT_FALSE(OpenDocument("trailer_unterminated.pdf"));
  MockDownloadHints hints;
  EXPECT_FALSE(FPDFAvail_IsDocAvail(avail(), &hints));
}

TEST_F(FPDFDataAvailEmbedderTest, TrailerAsHexstring) {
  // Document must load without crashing but is too malformed to be available.
  EXPECT_FALSE(OpenDocument("trailer_as_hexstring.pdf"));
  MockDownloadHints hints;
  EXPECT_FALSE(FPDFAvail_IsDocAvail(avail(), &hints));
}

TEST_F(FPDFDataAvailEmbedderTest, LoadUsingHintTables) {
  TestAsyncLoader loader("feature_linearized_loading.pdf");
  CreateAvail(loader.file_avail(), loader.file_access());
  ASSERT_EQ(PDF_DATA_AVAIL, FPDFAvail_IsDocAvail(avail(), loader.hints()));
  SetDocumentFromAvail();
  ASSERT_TRUE(document());
  ASSERT_EQ(PDF_DATA_AVAIL, FPDFAvail_IsPageAvail(avail(), 1, loader.hints()));

  // No new data available, to prevent load "Pages" node.
  loader.set_is_new_data_available(false);
  ScopedFPDFPage page(FPDF_LoadPage(document(), 1));
  EXPECT_TRUE(page);
}

TEST_F(FPDFDataAvailEmbedderTest, CheckFormAvailIfLinearized) {
  TestAsyncLoader loader("feature_linearized_loading.pdf");
  CreateAvail(loader.file_avail(), loader.file_access());
  ASSERT_EQ(PDF_DATA_AVAIL, FPDFAvail_IsDocAvail(avail(), loader.hints()));
  SetDocumentFromAvail();
  ASSERT_TRUE(document());

  // Prevent access to non-requested data to coerce the parser to send new
  // request for non available (non-requested before) data.
  loader.set_is_new_data_available(false);
  loader.ClearRequestedSegments();

  int status = PDF_FORM_NOTAVAIL;
  while (status == PDF_FORM_NOTAVAIL) {
    loader.FlushRequestedData();
    status = FPDFAvail_IsFormAvail(avail(), loader.hints());
  }
  EXPECT_NE(PDF_FORM_ERROR, status);
}

TEST_F(FPDFDataAvailEmbedderTest,
       DoNotLoadMainCrossRefForFirstPageIfLinearized) {
  TestAsyncLoader loader("feature_linearized_loading.pdf");
  CreateAvail(loader.file_avail(), loader.file_access());
  ASSERT_EQ(PDF_DATA_AVAIL, FPDFAvail_IsDocAvail(avail(), loader.hints()));
  SetDocumentFromAvail();
  ASSERT_TRUE(document());
  const int first_page_num = FPDFAvail_GetFirstPageNum(document());

  // The main cross ref table should not be processed.
  // (It is always at file end)
  EXPECT_GT(loader.file_access()->m_FileLen,
            loader.max_already_available_bound());

  // Prevent access to non-requested data to coerce the parser to send new
  // request for non available (non-requested before) data.
  loader.set_is_new_data_available(false);
  FPDFAvail_IsPageAvail(avail(), first_page_num, loader.hints());

  // The main cross ref table should not be requested.
  // (It is always at file end)
  EXPECT_GT(loader.file_access()->m_FileLen, loader.max_requested_bound());

  // Allow parse page.
  loader.set_is_new_data_available(true);
  ASSERT_EQ(PDF_DATA_AVAIL,
            FPDFAvail_IsPageAvail(avail(), first_page_num, loader.hints()));

  // The main cross ref table should not be processed.
  // (It is always at file end)
  EXPECT_GT(loader.file_access()->m_FileLen,
            loader.max_already_available_bound());

  // Prevent loading data, while page loading.
  loader.set_is_new_data_available(false);
  ScopedFPDFPage page(FPDF_LoadPage(document(), first_page_num));
  EXPECT_TRUE(page);
}

TEST_F(FPDFDataAvailEmbedderTest, LoadSecondPageIfLinearizedWithHints) {
  TestAsyncLoader loader("feature_linearized_loading.pdf");
  CreateAvail(loader.file_avail(), loader.file_access());
  ASSERT_EQ(PDF_DATA_AVAIL, FPDFAvail_IsDocAvail(avail(), loader.hints()));
  SetDocumentFromAvail();
  ASSERT_TRUE(document());

  static constexpr uint32_t kSecondPageNum = 1;

  // Prevent access to non-requested data to coerce the parser to send new
  // request for non available (non-requested before) data.
  loader.set_is_new_data_available(false);
  loader.ClearRequestedSegments();

  int status = PDF_DATA_NOTAVAIL;
  while (status == PDF_DATA_NOTAVAIL) {
    loader.FlushRequestedData();
    status = FPDFAvail_IsPageAvail(avail(), kSecondPageNum, loader.hints());
  }
  EXPECT_EQ(PDF_DATA_AVAIL, status);

  // Prevent loading data, while page loading.
  loader.set_is_new_data_available(false);
  ScopedFPDFPage page(FPDF_LoadPage(document(), kSecondPageNum));
  EXPECT_TRUE(page);
}

TEST_F(FPDFDataAvailEmbedderTest, LoadInfoAfterReceivingWholeDocument) {
  TestAsyncLoader loader("linearized.pdf");
  loader.set_is_new_data_available(false);
  CreateAvail(loader.file_avail(), loader.file_access());
  while (PDF_DATA_AVAIL != FPDFAvail_IsDocAvail(avail(), loader.hints())) {
    loader.FlushRequestedData();
  }

  SetDocumentFromAvail();
  ASSERT_TRUE(document());

  // The "info" dictionary should still be unavailable.
  EXPECT_FALSE(FPDF_GetMetaText(document(), "CreationDate", nullptr, 0));

  // Simulate receiving whole file.
  loader.set_is_new_data_available(true);
  // Load second page, to parse additional crossref sections.
  EXPECT_EQ(PDF_DATA_AVAIL, FPDFAvail_IsPageAvail(avail(), 1, loader.hints()));

  EXPECT_TRUE(FPDF_GetMetaText(document(), "CreationDate", nullptr, 0));
}

TEST_F(FPDFDataAvailEmbedderTest, LoadInfoAfterReceivingFirstPage) {
  TestAsyncLoader loader("linearized.pdf");
  // Map "Info" to an object within the first section without breaking
  // linearization.
  ByteString data(loader.file_contents(), loader.file_length());
  absl::optional<size_t> index = data.Find("/Info 27 0 R");
  ASSERT_TRUE(index);
  memcpy(loader.file_contents() + *index, "/Info 29 0 R", 12);

  loader.set_is_new_data_available(false);
  CreateAvail(loader.file_avail(), loader.file_access());
  while (PDF_DATA_AVAIL != FPDFAvail_IsDocAvail(avail(), loader.hints())) {
    loader.FlushRequestedData();
  }

  SetDocumentFromAvail();
  ASSERT_TRUE(document());

  // The "Info" dictionary should be available for the linearized document, if
  // it is located in the first page section.
  // Info was remapped to a dictionary with Type "Catalog"
  unsigned short buffer[100] = {0};
  EXPECT_TRUE(FPDF_GetMetaText(document(), "Type", buffer, sizeof(buffer)));
  EXPECT_EQ(L"Catalog", GetPlatformWString(buffer));
}

TEST_F(FPDFDataAvailEmbedderTest, TryLoadInvalidInfo) {
  TestAsyncLoader loader("linearized.pdf");
  // Map "Info" to an invalid object without breaking linearization.
  ByteString data(loader.file_contents(), loader.file_length());
  absl::optional<size_t> index = data.Find("/Info 27 0 R");
  ASSERT_TRUE(index);
  memcpy(loader.file_contents() + *index, "/Info 99 0 R", 12);

  loader.set_is_new_data_available(false);
  CreateAvail(loader.file_avail(), loader.file_access());
  while (PDF_DATA_AVAIL != FPDFAvail_IsDocAvail(avail(), loader.hints())) {
    loader.FlushRequestedData();
  }

  SetDocumentFromAvail();
  ASSERT_TRUE(document());

  // Set all data available.
  loader.set_is_new_data_available(true);
  // Check second page, to load additional crossrefs.
  ASSERT_EQ(PDF_DATA_AVAIL, FPDFAvail_IsPageAvail(avail(), 0, loader.hints()));

  // Test that api is robust enough to handle the bad case.
  EXPECT_FALSE(FPDF_GetMetaText(document(), "Type", nullptr, 0));
}

TEST_F(FPDFDataAvailEmbedderTest, TryLoadNonExistsInfo) {
  TestAsyncLoader loader("linearized.pdf");
  // Break the "Info" parameter without breaking linearization.
  ByteString data(loader.file_contents(), loader.file_length());
  absl::optional<size_t> index = data.Find("/Info 27 0 R");
  ASSERT_TRUE(index);
  memcpy(loader.file_contents() + *index, "/I_fo 27 0 R", 12);

  loader.set_is_new_data_available(false);
  CreateAvail(loader.file_avail(), loader.file_access());
  while (PDF_DATA_AVAIL != FPDFAvail_IsDocAvail(avail(), loader.hints())) {
    loader.FlushRequestedData();
  }

  SetDocumentFromAvail();
  ASSERT_TRUE(document());

  // Set all data available.
  loader.set_is_new_data_available(true);
  // Check second page, to load additional crossrefs.
  ASSERT_EQ(PDF_DATA_AVAIL, FPDFAvail_IsPageAvail(avail(), 0, loader.hints()));

  // Test that api is robust enough to handle the bad case.
  EXPECT_FALSE(FPDF_GetMetaText(document(), "Type", nullptr, 0));
}

TEST_F(FPDFDataAvailEmbedderTest, BadInputsToAPIs) {
  EXPECT_EQ(PDF_DATA_ERROR, FPDFAvail_IsDocAvail(nullptr, nullptr));
  EXPECT_FALSE(FPDFAvail_GetDocument(nullptr, nullptr));
  EXPECT_EQ(0, FPDFAvail_GetFirstPageNum(nullptr));
  EXPECT_EQ(PDF_DATA_ERROR, FPDFAvail_IsPageAvail(nullptr, 0, nullptr));
  EXPECT_EQ(PDF_FORM_ERROR, FPDFAvail_IsFormAvail(nullptr, nullptr));
  EXPECT_EQ(PDF_LINEARIZATION_UNKNOWN, FPDFAvail_IsLinearized(nullptr));
}

TEST_F(FPDFDataAvailEmbedderTest, NegativePageIndex) {
  TestAsyncLoader loader("linearized.pdf");
  CreateAvail(loader.file_avail(), loader.file_access());
  ASSERT_EQ(PDF_DATA_AVAIL, FPDFAvail_IsDocAvail(avail(), loader.hints()));
  EXPECT_EQ(PDF_DATA_NOTAVAIL,
            FPDFAvail_IsPageAvail(avail(), -1, loader.hints()));
}

TEST_F(FPDFDataAvailEmbedderTest, Bug_1324189) {
  // Test passes if it doesn't crash.
  TestAsyncLoader loader("bug_1324189.pdf");
  CreateAvail(loader.file_avail(), loader.file_access());
  ASSERT_EQ(PDF_DATA_NOTAVAIL, FPDFAvail_IsDocAvail(avail(), loader.hints()));
}

TEST_F(FPDFDataAvailEmbedderTest, Bug_1324503) {
  // Test passes if it doesn't crash.
  TestAsyncLoader loader("bug_1324503.pdf");
  CreateAvail(loader.file_avail(), loader.file_access());
  ASSERT_EQ(PDF_DATA_NOTAVAIL, FPDFAvail_IsDocAvail(avail(), loader.hints()));
}
