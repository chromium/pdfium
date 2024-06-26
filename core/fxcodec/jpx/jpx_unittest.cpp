// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <limits.h>
#include <stdint.h>

#include <limits>
#include <type_traits>

#include "core/fxcodec/jpx/cjpx_decoder.h"
#include "core/fxcodec/jpx/jpx_decode_utils.h"
#include "core/fxcrt/fx_memcpy_wrappers.h"
#include "core/fxcrt/fx_memory.h"
#include "core/fxcrt/stl_util.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/libopenjpeg/opj_malloc.h"

namespace fxcodec {

static const OPJ_OFF_T kSkipError = static_cast<OPJ_OFF_T>(-1);
static const OPJ_SIZE_T kReadError = static_cast<OPJ_SIZE_T>(-1);

static const uint8_t kStreamData[] = {
    0x00, 0x01, 0x02, 0x03,
    0x84, 0x85, 0x86, 0x87,  // Include some hi-bytes, too.
};

TEST(fxcodec, DecodeDataNullDecodeData) {
  uint8_t buffer[16];
  DecodeData* ptr = nullptr;

  // Error codes, not segvs, should callers pass us a nullptr pointer.
  EXPECT_EQ(kReadError, opj_read_from_memory(buffer, sizeof(buffer), ptr));
  EXPECT_EQ(kSkipError, opj_skip_from_memory(1, ptr));
  EXPECT_FALSE(opj_seek_from_memory(1, ptr));
}

TEST(fxcodec, DecodeDataNullStream) {
  DecodeData dd;
  uint8_t buffer[16];

  // Reads of size 0 do nothing but return an error code.
  fxcrt::Fill(buffer, 0xbd);
  EXPECT_EQ(kReadError, opj_read_from_memory(buffer, 0, &dd));
  EXPECT_EQ(0xbd, buffer[0]);

  // Reads of nonzero size do nothing but return an error code.
  fxcrt::Fill(buffer, 0xbd);
  EXPECT_EQ(kReadError, opj_read_from_memory(buffer, sizeof(buffer), &dd));
  EXPECT_EQ(0xbd, buffer[0]);

  // Skips of size 0 always return an error code.
  EXPECT_EQ(kSkipError, opj_skip_from_memory(0, &dd));

  // Skips of nonzero size always return an error code.
  EXPECT_EQ(kSkipError, opj_skip_from_memory(1, &dd));

  // Seeks to 0 offset return in error.
  EXPECT_FALSE(opj_seek_from_memory(0, &dd));

  // Seeks to non-zero offsets return in error.
  EXPECT_FALSE(opj_seek_from_memory(1, &dd));
}

TEST(fxcodec, DecodeDataZeroSize) {
  DecodeData dd(pdfium::make_span(kStreamData).first(0u));
  uint8_t buffer[16];

  // Reads of size 0 do nothing but return an error code.
  fxcrt::Fill(buffer, 0xbd);
  EXPECT_EQ(kReadError, opj_read_from_memory(buffer, 0, &dd));
  EXPECT_EQ(0xbd, buffer[0]);

  // Reads of nonzero size do nothing but return an error code.
  fxcrt::Fill(buffer, 0xbd);
  EXPECT_EQ(kReadError, opj_read_from_memory(buffer, sizeof(buffer), &dd));
  EXPECT_EQ(0xbd, buffer[0]);

  // Skips of size 0 always return an error code.
  EXPECT_EQ(kSkipError, opj_skip_from_memory(0, &dd));

  // Skips of nonzero size always return an error code.
  EXPECT_EQ(kSkipError, opj_skip_from_memory(1, &dd));

  // Seeks to 0 offset return in error.
  EXPECT_FALSE(opj_seek_from_memory(0, &dd));

  // Seeks to non-zero offsets return in error.
  EXPECT_FALSE(opj_seek_from_memory(1, &dd));
}

TEST(fxcodec, DecodeDataReadInBounds) {
  uint8_t buffer[16];
  {
    DecodeData dd(kStreamData);

    // Exact sized read in a single call.
    fxcrt::Fill(buffer, 0xbd);
    EXPECT_EQ(8u, opj_read_from_memory(buffer, sizeof(buffer), &dd));
    EXPECT_EQ(0x00, buffer[0]);
    EXPECT_EQ(0x01, buffer[1]);
    EXPECT_EQ(0x02, buffer[2]);
    EXPECT_EQ(0x03, buffer[3]);
    EXPECT_EQ(0x84, buffer[4]);
    EXPECT_EQ(0x85, buffer[5]);
    EXPECT_EQ(0x86, buffer[6]);
    EXPECT_EQ(0x87, buffer[7]);
    EXPECT_EQ(0xbd, buffer[8]);
  }
  {
    DecodeData dd(kStreamData);

    // Simple read.
    fxcrt::Fill(buffer, 0xbd);
    EXPECT_EQ(2u, opj_read_from_memory(buffer, 2, &dd));
    EXPECT_EQ(0x00, buffer[0]);
    EXPECT_EQ(0x01, buffer[1]);
    EXPECT_EQ(0xbd, buffer[2]);

    // Read of size 0 doesn't affect things.
    fxcrt::Fill(buffer, 0xbd);
    EXPECT_EQ(0u, opj_read_from_memory(buffer, 0, &dd));
    EXPECT_EQ(0xbd, buffer[0]);

    // Read exactly up to end of data.
    fxcrt::Fill(buffer, 0xbd);
    EXPECT_EQ(6u, opj_read_from_memory(buffer, 6, &dd));
    EXPECT_EQ(0x02, buffer[0]);
    EXPECT_EQ(0x03, buffer[1]);
    EXPECT_EQ(0x84, buffer[2]);
    EXPECT_EQ(0x85, buffer[3]);
    EXPECT_EQ(0x86, buffer[4]);
    EXPECT_EQ(0x87, buffer[5]);
    EXPECT_EQ(0xbd, buffer[6]);

    // Read of size 0 at EOF is still an error.
    fxcrt::Fill(buffer, 0xbd);
    EXPECT_EQ(kReadError, opj_read_from_memory(buffer, 0, &dd));
    EXPECT_EQ(0xbd, buffer[0]);
  }
}

TEST(fxcodec, DecodeDataReadBeyondBounds) {
  uint8_t buffer[16];
  {
    DecodeData dd(kStreamData);

    // Read beyond bounds in a single step.
    fxcrt::Fill(buffer, 0xbd);
    EXPECT_EQ(8u, opj_read_from_memory(buffer, sizeof(buffer) + 1, &dd));
    EXPECT_EQ(0x00, buffer[0]);
    EXPECT_EQ(0x01, buffer[1]);
    EXPECT_EQ(0x02, buffer[2]);
    EXPECT_EQ(0x03, buffer[3]);
    EXPECT_EQ(0x84, buffer[4]);
    EXPECT_EQ(0x85, buffer[5]);
    EXPECT_EQ(0x86, buffer[6]);
    EXPECT_EQ(0x87, buffer[7]);
    EXPECT_EQ(0xbd, buffer[8]);
  }
  {
    DecodeData dd(kStreamData);

    // Read well beyond bounds in a single step.
    fxcrt::Fill(buffer, 0xbd);
    EXPECT_EQ(8u, opj_read_from_memory(
                      buffer, std::numeric_limits<OPJ_SIZE_T>::max(), &dd));
    EXPECT_EQ(0x00, buffer[0]);
    EXPECT_EQ(0x01, buffer[1]);
    EXPECT_EQ(0x02, buffer[2]);
    EXPECT_EQ(0x03, buffer[3]);
    EXPECT_EQ(0x84, buffer[4]);
    EXPECT_EQ(0x85, buffer[5]);
    EXPECT_EQ(0x86, buffer[6]);
    EXPECT_EQ(0x87, buffer[7]);
    EXPECT_EQ(0xbd, buffer[8]);
  }
  {
    DecodeData dd(kStreamData);

    // Read of size 6 gets first 6 bytes.
    // rest of buffer intact.
    fxcrt::Fill(buffer, 0xbd);
    EXPECT_EQ(6u, opj_read_from_memory(buffer, 6, &dd));
    EXPECT_EQ(0x00, buffer[0]);
    EXPECT_EQ(0x01, buffer[1]);
    EXPECT_EQ(0x02, buffer[2]);
    EXPECT_EQ(0x03, buffer[3]);
    EXPECT_EQ(0x84, buffer[4]);
    EXPECT_EQ(0x85, buffer[5]);
    EXPECT_EQ(0xbd, buffer[6]);

    // Read of size 6 gets remaining two bytes.
    fxcrt::Fill(buffer, 0xbd);
    EXPECT_EQ(2u, opj_read_from_memory(buffer, 6, &dd));
    EXPECT_EQ(0x86, buffer[0]);
    EXPECT_EQ(0x87, buffer[1]);
    EXPECT_EQ(0xbd, buffer[2]);

    // Read of 6 more gets nothing and leaves rest of buffer intact.
    fxcrt::Fill(buffer, 0xbd);
    EXPECT_EQ(kReadError, opj_read_from_memory(buffer, 6, &dd));
    EXPECT_EQ(0xbd, buffer[0]);
  }
}

// Note: Some care needs to be taken here because the skip/seek functions
// take OPJ_OFF_T's as arguments, which are typically a signed type.
TEST(fxcodec, DecodeDataSkip) {
  uint8_t buffer[16];
  {
    DecodeData dd(kStreamData);

    // Skiping within buffer is allowed.
    fxcrt::Fill(buffer, 0xbd);
    EXPECT_EQ(1u, opj_skip_from_memory(1, &dd));
    EXPECT_EQ(1u, opj_read_from_memory(buffer, 1, &dd));
    EXPECT_EQ(0x01, buffer[0]);
    EXPECT_EQ(0xbd, buffer[1]);

    // Skiping 0 bytes changes nothing.
    fxcrt::Fill(buffer, 0xbd);
    EXPECT_EQ(0, opj_skip_from_memory(0, &dd));
    EXPECT_EQ(1u, opj_read_from_memory(buffer, 1, &dd));
    EXPECT_EQ(0x02, buffer[0]);
    EXPECT_EQ(0xbd, buffer[1]);

    // Skiping to EOS-1 is possible.
    fxcrt::Fill(buffer, 0xbd);
    EXPECT_EQ(4u, opj_skip_from_memory(4, &dd));
    EXPECT_EQ(1u, opj_read_from_memory(buffer, 1, &dd));
    EXPECT_EQ(0x87, buffer[0]);
    EXPECT_EQ(0xbd, buffer[1]);

    // Next read fails.
    fxcrt::Fill(buffer, 0xbd);
    EXPECT_EQ(kReadError, opj_read_from_memory(buffer, 1, &dd));
    EXPECT_EQ(0xbd, buffer[0]);
  }
  {
    DecodeData dd(kStreamData);

    // Skiping directly to EOS is allowed.
    fxcrt::Fill(buffer, 0xbd);
    EXPECT_EQ(8u, opj_skip_from_memory(8, &dd));

    // Next read fails.
    EXPECT_EQ(kReadError, opj_read_from_memory(buffer, 1, &dd));
    EXPECT_EQ(0xbd, buffer[0]);
  }
  {
    DecodeData dd(kStreamData);

    // Skipping beyond end of stream is allowed and returns full distance.
    fxcrt::Fill(buffer, 0xbd);
    EXPECT_EQ(9u, opj_skip_from_memory(9, &dd));

    // Next read fails.
    EXPECT_EQ(kReadError, opj_read_from_memory(buffer, 1, &dd));
    EXPECT_EQ(0xbd, buffer[0]);
  }
  {
    DecodeData dd(kStreamData);

    // Skipping way beyond EOS is allowd, doesn't wrap, and returns
    // full distance.
    fxcrt::Fill(buffer, 0xbd);
    EXPECT_EQ(4u, opj_skip_from_memory(4, &dd));
    EXPECT_EQ(std::numeric_limits<OPJ_OFF_T>::max(),
              opj_skip_from_memory(std::numeric_limits<OPJ_OFF_T>::max(), &dd));

    // Next read fails. If it succeeds, it may mean we wrapped.
    EXPECT_EQ(kReadError, opj_read_from_memory(buffer, 1, &dd));
    EXPECT_EQ(0xbd, buffer[0]);
  }
  {
    DecodeData dd(kStreamData);

    // Negative skip within buffer not is allowed, position unchanged.
    fxcrt::Fill(buffer, 0xbd);
    EXPECT_EQ(4u, opj_skip_from_memory(4, &dd));
    EXPECT_EQ(kSkipError, opj_skip_from_memory(-2, &dd));

    // Next read succeeds as if nothing has happenned.
    EXPECT_EQ(1u, opj_read_from_memory(buffer, 1, &dd));
    EXPECT_EQ(0x84, buffer[0]);
    EXPECT_EQ(0xbd, buffer[1]);

    // Negative skip before buffer is not allowed, position unchanged.
    fxcrt::Fill(buffer, 0xbd);
    EXPECT_EQ(kSkipError, opj_skip_from_memory(-4, &dd));

    // Next read succeeds as if nothing has happenned.
    EXPECT_EQ(1u, opj_read_from_memory(buffer, 1, &dd));
    EXPECT_EQ(0x85, buffer[0]);
    EXPECT_EQ(0xbd, buffer[1]);
  }
  {
    DecodeData dd(kStreamData);

    // Negative skip way before buffer is not allowed, doesn't wrap
    fxcrt::Fill(buffer, 0xbd);
    EXPECT_EQ(4u, opj_skip_from_memory(4, &dd));
    EXPECT_EQ(kSkipError,
              opj_skip_from_memory(std::numeric_limits<OPJ_OFF_T>::min(), &dd));

    // Next read succeeds. If it fails, it may mean we wrapped.
    EXPECT_EQ(1u, opj_read_from_memory(buffer, 1, &dd));
    EXPECT_EQ(0x84, buffer[0]);
    EXPECT_EQ(0xbd, buffer[1]);
  }
  {
    DecodeData dd(kStreamData);

    // Negative skip after EOS isn't alowed, still EOS.
    fxcrt::Fill(buffer, 0xbd);
    EXPECT_EQ(8u, opj_skip_from_memory(8, &dd));
    EXPECT_EQ(kSkipError, opj_skip_from_memory(-4, &dd));

    // Next read fails.
    EXPECT_EQ(kReadError, opj_read_from_memory(buffer, 1, &dd));
    EXPECT_EQ(0xbd, buffer[0]);
  }
}

TEST(fxcodec, DecodeDataSeek) {
  uint8_t buffer[16];
  DecodeData dd(kStreamData);

  // Seeking within buffer is allowed and read succeeds
  fxcrt::Fill(buffer, 0xbd);
  EXPECT_TRUE(opj_seek_from_memory(1, &dd));
  EXPECT_EQ(1u, opj_read_from_memory(buffer, 1, &dd));
  EXPECT_EQ(0x01, buffer[0]);
  EXPECT_EQ(0xbd, buffer[1]);

  // Seeking before start returns error leaving position unchanged.
  fxcrt::Fill(buffer, 0xbd);
  EXPECT_FALSE(opj_seek_from_memory(-1, &dd));
  EXPECT_EQ(1u, opj_read_from_memory(buffer, 1, &dd));
  EXPECT_EQ(0x02, buffer[0]);
  EXPECT_EQ(0xbd, buffer[1]);

  // Seeking way before start returns error leaving position unchanged.
  fxcrt::Fill(buffer, 0xbd);
  EXPECT_FALSE(
      opj_seek_from_memory(std::numeric_limits<OPJ_OFF_T>::min(), &dd));
  EXPECT_EQ(1u, opj_read_from_memory(buffer, 1, &dd));
  EXPECT_EQ(0x03, buffer[0]);
  EXPECT_EQ(0xbd, buffer[1]);

  // Seeking exactly to EOS is allowed but read fails.
  fxcrt::Fill(buffer, 0xbd);
  EXPECT_TRUE(opj_seek_from_memory(8, &dd));
  EXPECT_EQ(kReadError, opj_read_from_memory(buffer, 1, &dd));
  EXPECT_EQ(0xbd, buffer[0]);

  // Seeking back to zero offset is allowed and read succeeds.
  fxcrt::Fill(buffer, 0xbd);
  EXPECT_TRUE(opj_seek_from_memory(0, &dd));
  EXPECT_EQ(1u, opj_read_from_memory(buffer, 1, &dd));
  EXPECT_EQ(0x00, buffer[0]);
  EXPECT_EQ(0xbd, buffer[1]);

  // Seeking beyond end of stream is allowed but read fails.
  fxcrt::Fill(buffer, 0xbd);
  EXPECT_TRUE(opj_seek_from_memory(16, &dd));
  EXPECT_EQ(kReadError, opj_read_from_memory(buffer, 1, &dd));
  EXPECT_EQ(0xbd, buffer[0]);

  // Seeking within buffer after seek past EOF restores good state.
  fxcrt::Fill(buffer, 0xbd);
  EXPECT_TRUE(opj_seek_from_memory(4, &dd));
  EXPECT_EQ(1u, opj_read_from_memory(buffer, 1, &dd));
  EXPECT_EQ(0x84, buffer[0]);
  EXPECT_EQ(0xbd, buffer[1]);

  // Seeking way beyond EOS is allowed, doesn't wrap, and read fails.
  fxcrt::Fill(buffer, 0xbd);
  EXPECT_TRUE(opj_seek_from_memory(std::numeric_limits<OPJ_OFF_T>::max(), &dd));
  EXPECT_EQ(kReadError, opj_read_from_memory(buffer, 1, &dd));
  EXPECT_EQ(0xbd, buffer[0]);
}

TEST(fxcodec, YUV420ToRGB) {
  opj_image_comp_t u = {};  // Aggregate initialization.
  static_assert(std::is_aggregate_v<decltype(u)>);
  u.dx = 1;
  u.dy = 1;
  u.w = 16;
  u.h = 16;
  u.prec = 8;
  u.bpp = 8;
  opj_image_comp_t v = {};  // Aggregate initialization.
  static_assert(std::is_aggregate_v<decltype(v)>);
  v.dx = 1;
  v.dy = 1;
  v.w = 16;
  v.h = 16;
  v.prec = 8;
  v.bpp = 8;
  opj_image_comp_t y = {};  // Aggregate initialization.
  static_assert(std::is_aggregate_v<decltype(y)>);
  y.dx = 1;
  y.dy = 1;
  y.prec = 8;
  y.bpp = 8;
  opj_image_t img = {};  // Aggregate initialization.
  static_assert(std::is_aggregate_v<decltype(img)>);
  img.numcomps = 3;
  img.color_space = OPJ_CLRSPC_SYCC;
  img.comps = FX_Alloc(opj_image_comp_t, 3);
  const struct {
    OPJ_UINT32 w;
    bool expected;
  } cases[] = {{0, false}, {1, false},  {30, false}, {31, true},
               {32, true}, {33, false}, {34, false}, {UINT_MAX, false}};
  for (const auto& testcase : cases) {
    y.w = testcase.w;
    y.h = y.w;
    img.x1 = y.w;
    img.y1 = y.h;
    y.data = static_cast<OPJ_INT32*>(
        opj_image_data_alloc(y.w * y.h * sizeof(OPJ_INT32)));
    v.data = static_cast<OPJ_INT32*>(
        opj_image_data_alloc(v.w * v.h * sizeof(OPJ_INT32)));
    u.data = static_cast<OPJ_INT32*>(
        opj_image_data_alloc(u.w * u.h * sizeof(OPJ_INT32)));

    UNSAFE_TODO({
      FXSYS_memset(y.data, 1, y.w * y.h * sizeof(OPJ_INT32));
      FXSYS_memset(u.data, 0, u.w * u.h * sizeof(OPJ_INT32));
      FXSYS_memset(v.data, 0, v.w * v.h * sizeof(OPJ_INT32));
      img.comps[0] = y;
      img.comps[1] = u;
      img.comps[2] = v;
      CJPX_Decoder::Sycc420ToRgbForTesting(&img);
      if (testcase.expected) {
        EXPECT_EQ(img.comps[0].w, img.comps[1].w);
        EXPECT_EQ(img.comps[0].h, img.comps[1].h);
        EXPECT_EQ(img.comps[0].w, img.comps[2].w);
        EXPECT_EQ(img.comps[0].h, img.comps[2].h);
      } else {
        EXPECT_NE(img.comps[0].w, img.comps[1].w);
        EXPECT_NE(img.comps[0].h, img.comps[1].h);
        EXPECT_NE(img.comps[0].w, img.comps[2].w);
        EXPECT_NE(img.comps[0].h, img.comps[2].h);
      }
      opj_image_data_free(img.comps[0].data);
      opj_image_data_free(img.comps[1].data);
      opj_image_data_free(img.comps[2].data);
    });
  }
  FX_Free(img.comps);
}

}  // namespace fxcodec
