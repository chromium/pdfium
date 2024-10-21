// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcodec/gif/cfx_gifcontext.h"

#include <stdint.h>

#include <array>
#include <utility>

#include "core/fxcodec/cfx_codec_memory.h"
#include "core/fxcrt/data_vector.h"
#include "core/fxcrt/stl_util.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace fxcodec {

class CFX_GifContextForTest final : public CFX_GifContext {
 public:
  CFX_GifContextForTest() : CFX_GifContext(nullptr) {}
  ~CFX_GifContextForTest() override = default;

  using CFX_GifContext::ReadAllOrNone;
  using CFX_GifContext::ReadGifSignature;
  using CFX_GifContext::ReadLogicalScreenDescriptor;

  CFX_CodecMemory* InputBuffer() const { return input_buffer_.Get(); }
  void SetTestInputBuffer(pdfium::span<const uint8_t> input) {
    auto pMemory = pdfium::MakeRetain<CFX_CodecMemory>(input.size());
    fxcrt::Copy(input, pMemory->GetBufferSpan());
    SetInputBuffer(std::move(pMemory));
  }
};

TEST(CFXGifContextTest, SetInputBuffer) {
  uint8_t buffer[] = {0x00, 0x01, 0x02};
  CFX_GifContextForTest context;

  context.SetTestInputBuffer({});
  EXPECT_EQ(0u, context.InputBuffer()->GetSize());
  EXPECT_EQ(0u, context.InputBuffer()->GetPosition());

  context.SetTestInputBuffer(pdfium::make_span(buffer).first(0u));
  EXPECT_EQ(0u, context.InputBuffer()->GetSize());
  EXPECT_EQ(0u, context.InputBuffer()->GetPosition());

  context.SetTestInputBuffer(buffer);
  EXPECT_EQ(3u, context.InputBuffer()->GetSize());
  EXPECT_EQ(0u, context.InputBuffer()->GetPosition());
}

TEST(CFXGifContextTest, ReadAllOrNone) {
  CFX_GifContextForTest context;
  context.SetTestInputBuffer({});
  EXPECT_FALSE(context.ReadAllOrNone(pdfium::span<uint8_t>()));

  auto src_buffer = fxcrt::ToArray<const uint8_t>(
      {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09});

  DataVector<uint8_t> dest_buffer(src_buffer.size());
  auto dest_span = pdfium::make_span(dest_buffer);

  context.SetTestInputBuffer(pdfium::make_span(src_buffer).first(1u));
  EXPECT_FALSE(context.ReadAllOrNone(dest_buffer));
  EXPECT_EQ(0u, context.InputBuffer()->GetPosition());
  EXPECT_TRUE(context.ReadAllOrNone(dest_span.first(1u)));
  EXPECT_EQ(src_buffer[0], dest_buffer[0]);

  context.SetTestInputBuffer(src_buffer);
  EXPECT_TRUE(context.ReadAllOrNone(dest_span.first(src_buffer.size())));
  for (size_t i = 0; i < src_buffer.size(); i++) {
    EXPECT_EQ(src_buffer[i], dest_buffer[i]);
  }
  context.SetTestInputBuffer(src_buffer);
  for (size_t i = 0; i < src_buffer.size(); i++) {
    EXPECT_TRUE(context.ReadAllOrNone(dest_span.first(1u)));
    EXPECT_EQ(src_buffer[i], dest_buffer[0]);
  }
}

TEST(CFXGifContextTest, ReadGifSignature) {
  CFX_GifContextForTest context;
  {
    uint8_t data[1];
    context.SetTestInputBuffer(pdfium::make_span(data).first(0u));
    EXPECT_EQ(GifDecoder::Status::kUnfinished, context.ReadGifSignature());
    EXPECT_EQ(0u, context.InputBuffer()->GetPosition());
    context.SetTestInputBuffer({});
  }
  // Make sure testing the entire signature
  {
    uint8_t data[] = {'G', 'I', 'F'};
    context.SetTestInputBuffer(data);
    EXPECT_EQ(GifDecoder::Status::kUnfinished, context.ReadGifSignature());
    EXPECT_EQ(0u, context.InputBuffer()->GetPosition());
    context.SetTestInputBuffer({});
  }
  {
    uint8_t data[] = {'N', 'O', 'T', 'G', 'I', 'F'};
    context.SetTestInputBuffer(data);
    EXPECT_EQ(GifDecoder::Status::kError, context.ReadGifSignature());
    EXPECT_EQ(6u, context.InputBuffer()->GetPosition());
    context.SetTestInputBuffer({});
  }
  // Make sure not matching GIF8*a
  {
    uint8_t data[] = {'G', 'I', 'F', '8', '0', 'a'};
    context.SetTestInputBuffer(data);
    EXPECT_EQ(GifDecoder::Status::kError, context.ReadGifSignature());
    EXPECT_EQ(6u, context.InputBuffer()->GetPosition());
    context.SetTestInputBuffer({});
  }
  // Make sure not matching GIF**a
  {
    uint8_t data[] = {'G', 'I', 'F', '9', '2', 'a'};
    context.SetTestInputBuffer(data);
    EXPECT_EQ(GifDecoder::Status::kError, context.ReadGifSignature());
    EXPECT_EQ(6u, context.InputBuffer()->GetPosition());
    context.SetTestInputBuffer({});
  }
  // One valid signature
  {
    uint8_t data[] = {'G', 'I', 'F', '8', '7', 'a'};
    context.SetTestInputBuffer(data);
    EXPECT_EQ(GifDecoder::Status::kSuccess, context.ReadGifSignature());
    EXPECT_EQ(6u, context.InputBuffer()->GetPosition());
    context.SetTestInputBuffer({});
  }
  // The other valid signature
  {
    uint8_t data[] = {'G', 'I', 'F', '8', '9', 'a'};
    context.SetTestInputBuffer(data);
    EXPECT_EQ(GifDecoder::Status::kSuccess, context.ReadGifSignature());
    EXPECT_EQ(6u, context.InputBuffer()->GetPosition());
    context.SetTestInputBuffer({});
  }
}

TEST(CFXGifContextTest, ReadLocalScreenDescriptor) {
  CFX_GifContextForTest context;
  {
    uint8_t data[1];
    context.SetTestInputBuffer(pdfium::make_span(data).first(0u));
    EXPECT_EQ(GifDecoder::Status::kUnfinished,
              context.ReadLogicalScreenDescriptor());
    context.SetTestInputBuffer({});
  }
  // LSD with all the values zero'd
  {
    uint8_t lsd[sizeof(CFX_GifLocalScreenDescriptor)] = {};
    context.SetTestInputBuffer(lsd);

    EXPECT_EQ(GifDecoder::Status::kSuccess,
              context.ReadLogicalScreenDescriptor());

    EXPECT_EQ(sizeof(CFX_GifLocalScreenDescriptor),
              static_cast<size_t>(context.InputBuffer()->GetPosition()));
    EXPECT_EQ(0, context.width_);
    EXPECT_EQ(0, context.height_);
    EXPECT_EQ(0u, context.bc_index_);
    context.SetTestInputBuffer({});
  }
  // LSD with no global palette
  {
    uint8_t lsd[sizeof(CFX_GifLocalScreenDescriptor)] = {0x0A, 0x00, 0x00, 0x0F,
                                                         0x00, 0x01, 0x02};
    context.SetTestInputBuffer(lsd);

    EXPECT_EQ(GifDecoder::Status::kSuccess,
              context.ReadLogicalScreenDescriptor());

    EXPECT_EQ(sizeof(CFX_GifLocalScreenDescriptor),
              static_cast<size_t>(context.InputBuffer()->GetPosition()));
    EXPECT_EQ(0x000A, context.width_);
    EXPECT_EQ(0x0F00, context.height_);
    EXPECT_EQ(0u, context.bc_index_);  // bc_index_ is 0 if no global palette
    context.SetTestInputBuffer({});
  }
  // LSD with global palette bit set, but no global palette
  {
    uint8_t lsd[sizeof(CFX_GifLocalScreenDescriptor)] = {0x0A, 0x00, 0x00, 0x0F,
                                                         0x80, 0x01, 0x02};
    context.SetTestInputBuffer(lsd);

    EXPECT_EQ(GifDecoder::Status::kUnfinished,
              context.ReadLogicalScreenDescriptor());

    EXPECT_EQ(0u, context.InputBuffer()->GetPosition());
    context.SetTestInputBuffer({});
  }
  // LSD with global palette
  {
    struct {
      uint8_t lsd[sizeof(CFX_GifLocalScreenDescriptor)];
      uint8_t palette[4 * sizeof(CFX_GifPalette)];
    } data = {{0x0A, 0x00, 0x00, 0x0F, 0xA9, 0x01, 0x02},
              {0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1}};
    context.SetTestInputBuffer(pdfium::as_bytes(pdfium::span_from_ref(data)));
    EXPECT_EQ(GifDecoder::Status::kSuccess,
              context.ReadLogicalScreenDescriptor());

    EXPECT_EQ(sizeof(data), context.InputBuffer()->GetPosition());
    EXPECT_EQ(0x000A, context.width_);
    EXPECT_EQ(0x0F00, context.height_);
    EXPECT_EQ(1u, context.bc_index_);
    EXPECT_EQ(1u, context.global_palette_exp_);
    EXPECT_EQ(1, context.global_sort_flag_);
    EXPECT_EQ(2, context.global_color_resolution_);
    EXPECT_EQ(0, memcmp(data.palette, context.global_palette_.data(),
                        sizeof(data.palette)));
    context.SetTestInputBuffer({});
  }
}

TEST(CFXGifContextTest, ReadHeader) {
  CFX_GifContextForTest context;
  // Bad signature
  {
    struct {
      uint8_t signature[6];
      uint8_t lsd[sizeof(CFX_GifLocalScreenDescriptor)];
    } data = {{'N', 'O', 'T', 'G', 'I', 'F'},
              {0x0A, 0x00, 0x00, 0x0F, 0x00, 0x01, 0x02}};
    context.SetTestInputBuffer(pdfium::as_bytes(pdfium::span_from_ref(data)));
    EXPECT_EQ(GifDecoder::Status::kError, context.ReadHeader());
    EXPECT_EQ(sizeof(data.signature), context.InputBuffer()->GetPosition());
    context.SetTestInputBuffer({});
  }
  // Short after signature
  {
    uint8_t signature[] = {'G', 'I', 'F', '8', '7', 'a'};
    context.SetTestInputBuffer(signature);
    EXPECT_EQ(GifDecoder::Status::kUnfinished, context.ReadHeader());
    EXPECT_EQ(sizeof(signature), context.InputBuffer()->GetPosition());
    context.SetTestInputBuffer({});
  }
  // Success without global palette
  {
    struct {
      uint8_t signature[6];
      uint8_t lsd[sizeof(CFX_GifLocalScreenDescriptor)];
    } data = {{'G', 'I', 'F', '8', '7', 'a'},
              {0x0A, 0x00, 0x00, 0x0F, 0x00, 0x01, 0x02}};
    context.SetTestInputBuffer(pdfium::as_bytes(pdfium::span_from_ref(data)));
    EXPECT_EQ(GifDecoder::Status::kSuccess, context.ReadHeader());
    EXPECT_EQ(sizeof(data), context.InputBuffer()->GetPosition());
    EXPECT_EQ(0x000A, context.width_);
    EXPECT_EQ(0x0F00, context.height_);
    EXPECT_EQ(0u, context.bc_index_);  // bc_index_ is 0 if no global palette
    context.SetTestInputBuffer({});
  }
  // Missing Global Palette
  {
    struct {
      uint8_t signature[6];
      uint8_t lsd[sizeof(CFX_GifLocalScreenDescriptor)];
    } data = {{'G', 'I', 'F', '8', '7', 'a'},
              {0x0A, 0x00, 0x00, 0x0F, 0x80, 0x01, 0x02}};
    context.SetTestInputBuffer(pdfium::as_bytes(pdfium::span_from_ref(data)));
    EXPECT_EQ(GifDecoder::Status::kUnfinished, context.ReadHeader());
    EXPECT_EQ(sizeof(data.signature), context.InputBuffer()->GetPosition());
    context.SetTestInputBuffer({});
  }
  // Success with global palette
  {
    struct {
      uint8_t signature[6];
      uint8_t lsd[sizeof(CFX_GifLocalScreenDescriptor)];
      uint8_t palette[4 * sizeof(CFX_GifPalette)];
    } data = {{'G', 'I', 'F', '8', '7', 'a'},
              {0x0A, 0x00, 0x00, 0x0F, 0xA9, 0x01, 0x02},
              {0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1}};
    context.SetTestInputBuffer(pdfium::as_bytes(pdfium::span_from_ref(data)));
    EXPECT_EQ(GifDecoder::Status::kSuccess, context.ReadHeader());
    EXPECT_EQ(sizeof(data), context.InputBuffer()->GetPosition());
    EXPECT_EQ(0x000A, context.width_);
    EXPECT_EQ(0x0F00, context.height_);
    EXPECT_EQ(1u, context.bc_index_);
    EXPECT_EQ(1u, context.global_palette_exp_);
    EXPECT_EQ(1, context.global_sort_flag_);
    EXPECT_EQ(2, context.global_color_resolution_);
    EXPECT_EQ(0, memcmp(data.palette, context.global_palette_.data(),
                        sizeof(data.palette)));
    context.SetTestInputBuffer({});
  }
}

}  // namespace fxcodec
