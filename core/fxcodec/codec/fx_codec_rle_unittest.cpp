// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdint.h>

#include <limits>

#include "core/fpdfapi/parser/fpdf_parser_decode.h"
#include "core/fxcodec/codec/ccodec_basicmodule.h"
#include "core/fxcodec/fx_codec.h"
#include "testing/fx_string_testhelpers.h"
#include "testing/gtest/include/gtest/gtest.h"

TEST(fxcodec, RLETestBadInputs) {
  uint8_t src_buf[1] = {1};
  uint8_t* dest_buf = nullptr;
  uint32_t src_size = 4;
  uint32_t dest_size = 0;

  CCodec_BasicModule* pEncoders = CCodec_ModuleMgr().GetBasicModule();
  EXPECT_TRUE(pEncoders);

  // Error codes, not segvs, should callers pass us a nullptr pointer.
  EXPECT_FALSE(
      pEncoders->RunLengthEncode(src_buf, src_size, &dest_buf, nullptr));
  EXPECT_FALSE(
      pEncoders->RunLengthEncode(src_buf, src_size, nullptr, &dest_size));
  EXPECT_FALSE(pEncoders->RunLengthEncode(src_buf, 0, &dest_buf, &dest_size));
  EXPECT_FALSE(
      pEncoders->RunLengthEncode(nullptr, src_size, &dest_buf, &dest_size));
}

// Check length 1 input works. Check terminating character is applied.
TEST(fxcodec, RLETestShortInput) {
  uint8_t src_buf[1] = {1};
  uint8_t* dest_buf = nullptr;
  uint32_t src_size = 1;
  uint32_t dest_size = 0;

  CCodec_BasicModule* pEncoders = CCodec_ModuleMgr().GetBasicModule();
  EXPECT_TRUE(pEncoders);

  EXPECT_TRUE(
      pEncoders->RunLengthEncode(src_buf, src_size, &dest_buf, &dest_size));
  EXPECT_EQ(3u, dest_size);
  EXPECT_EQ(dest_buf[0], 0);
  EXPECT_EQ(dest_buf[1], 1);
  EXPECT_EQ(dest_buf[2], 128);

  FX_Free(dest_buf);
}

// Check a few basic cases (2 matching runs in a row, matching run followed
// by a nonmatching run, and nonmatching run followed by a matching run).
TEST(fxcodec, RLETestNormalInputs) {
  // Match, match
  uint8_t src_buf_1[10] = {2, 2, 2, 2, 4, 4, 4, 4, 4, 4};

  // Match, nonmatch
  uint8_t src_buf_2[10] = {2, 2, 2, 2, 1, 2, 3, 4, 5, 6};

  // Nonmatch, match
  uint8_t src_buf_3[10] = {1, 2, 3, 4, 5, 3, 3, 3, 3, 3};

  uint32_t src_size = 10;
  uint32_t dest_size = 0;
  uint8_t* dest_buf = nullptr;

  CCodec_BasicModule* pEncoders = CCodec_ModuleMgr().GetBasicModule();
  EXPECT_TRUE(pEncoders);

  // Case 1:
  EXPECT_TRUE(
      pEncoders->RunLengthEncode(src_buf_1, src_size, &dest_buf, &dest_size));
  uint8_t* decoded_buf = nullptr;
  uint32_t decoded_size = 0;
  RunLengthDecode(dest_buf, dest_size, decoded_buf, decoded_size);
  EXPECT_EQ(decoded_size, src_size);
  for (uint32_t i = 0; i < src_size; i++)
    EXPECT_EQ(decoded_buf[i], src_buf_1[i]) << " at " << i;
  FX_Free(dest_buf);
  FX_Free(decoded_buf);

  // Case 2:
  dest_buf = nullptr;
  dest_size = 0;
  EXPECT_TRUE(
      pEncoders->RunLengthEncode(src_buf_2, src_size, &dest_buf, &dest_size));
  decoded_buf = nullptr;
  decoded_size = 0;
  RunLengthDecode(dest_buf, dest_size, decoded_buf, decoded_size);
  EXPECT_EQ(decoded_size, src_size);
  for (uint32_t i = 0; i < src_size; i++)
    EXPECT_EQ(decoded_buf[i], src_buf_2[i]) << " at " << i;
  FX_Free(dest_buf);
  FX_Free(decoded_buf);

  // Case 3:
  dest_buf = nullptr;
  dest_size = 0;
  EXPECT_TRUE(
      pEncoders->RunLengthEncode(src_buf_3, src_size, &dest_buf, &dest_size));
  decoded_buf = nullptr;
  decoded_size = 0;
  RunLengthDecode(dest_buf, dest_size, decoded_buf, decoded_size);
  EXPECT_EQ(decoded_size, src_size);
  for (uint32_t i = 0; i < src_size; i++)
    EXPECT_EQ(decoded_buf[i], src_buf_3[i]) << " at " << i;
  FX_Free(dest_buf);
  FX_Free(decoded_buf);
}

// Check that runs longer than 128 are broken up properly, both matched and
// nonmatched.
TEST(fxcodec, RLETestFullLengthInputs) {
  // Match, match
  uint8_t src_buf_1[260] = {1};

  // Match, nonmatch
  uint8_t src_buf_2[260] = {2};
  for (uint16_t i = 128; i < 260; i++)
    src_buf_2[i] = (uint8_t)(i - 125);

  // Nonmatch, match
  uint8_t src_buf_3[260] = {3};
  for (uint8_t i = 0; i < 128; i++)
    src_buf_3[i] = i;

  // Nonmatch, nonmatch
  uint8_t src_buf_4[260];
  for (uint16_t i = 0; i < 260; i++)
    src_buf_4[i] = (uint8_t)(i);

  uint32_t src_size = 260;
  uint32_t dest_size = 0;
  uint8_t* dest_buf = nullptr;

  CCodec_BasicModule* pEncoders = CCodec_ModuleMgr().GetBasicModule();
  EXPECT_TRUE(pEncoders);

  // Case 1:
  EXPECT_TRUE(
      pEncoders->RunLengthEncode(src_buf_1, src_size, &dest_buf, &dest_size));
  uint8_t* decoded_buf = nullptr;
  uint32_t decoded_size = 0;
  RunLengthDecode(dest_buf, dest_size, decoded_buf, decoded_size);
  EXPECT_EQ(decoded_size, src_size);
  for (uint32_t i = 0; i < src_size; i++)
    EXPECT_EQ(decoded_buf[i], src_buf_1[i]) << " at " << i;
  FX_Free(dest_buf);
  FX_Free(decoded_buf);

  // Case 2:
  dest_buf = nullptr;
  dest_size = 0;
  EXPECT_TRUE(
      pEncoders->RunLengthEncode(src_buf_2, src_size, &dest_buf, &dest_size));
  decoded_buf = nullptr;
  decoded_size = 0;
  RunLengthDecode(dest_buf, dest_size, decoded_buf, decoded_size);
  EXPECT_EQ(decoded_size, src_size);
  for (uint32_t i = 0; i < src_size; i++)
    EXPECT_EQ(decoded_buf[i], src_buf_2[i]) << " at " << i;
  FX_Free(dest_buf);
  FX_Free(decoded_buf);

  // Case 3:
  dest_buf = nullptr;
  dest_size = 0;
  EXPECT_TRUE(
      pEncoders->RunLengthEncode(src_buf_3, src_size, &dest_buf, &dest_size));
  decoded_buf = nullptr;
  decoded_size = 0;
  RunLengthDecode(dest_buf, dest_size, decoded_buf, decoded_size);
  EXPECT_EQ(decoded_size, src_size);
  for (uint32_t i = 0; i < src_size; i++)
    EXPECT_EQ(decoded_buf[i], src_buf_3[i]) << " at " << i;
  FX_Free(dest_buf);
  FX_Free(decoded_buf);

  // Case 4:
  dest_buf = nullptr;
  dest_size = 0;
  EXPECT_TRUE(
      pEncoders->RunLengthEncode(src_buf_4, src_size, &dest_buf, &dest_size));
  decoded_buf = nullptr;
  decoded_size = 0;
  RunLengthDecode(dest_buf, dest_size, decoded_buf, decoded_size);
  EXPECT_EQ(decoded_size, src_size);
  for (uint32_t i = 0; i < src_size; i++)
    EXPECT_EQ(decoded_buf[i], src_buf_4[i]) << " at " << i;
  FX_Free(dest_buf);
  FX_Free(decoded_buf);
}
