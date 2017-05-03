// Copyright (c) 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Originally from chromium's /src/base/md5_unittest.cc.

#include "core/fdrm/crypto/fx_crypt.h"

#include <memory>
#include <string>

#include "core/fxcrt/fx_basic.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/test_support.h"

namespace {

std::string CRYPT_MD5String(const char* str) {
  return GenerateMD5Base16(reinterpret_cast<const uint8_t*>(str), strlen(str));
}

}  // namespace

TEST(FXCRYPT, CryptToBase16) {
  uint8_t data[] = {0xd4, 0x1d, 0x8c, 0xd9, 0x8f, 0x00, 0xb2, 0x04,
                    0xe9, 0x80, 0x09, 0x98, 0xec, 0xf8, 0x42, 0x7e};

  std::string actual = CryptToBase16(data);
  std::string expected = "d41d8cd98f00b204e9800998ecf8427e";

  EXPECT_EQ(expected, actual);
}

TEST(FXCRYPT, MD5GenerateEmtpyData) {
  uint8_t digest[16];
  const char data[] = "";
  uint32_t length = static_cast<uint32_t>(strlen(data));

  CRYPT_MD5Generate(reinterpret_cast<const uint8_t*>(data), length, digest);

  uint8_t expected[] = {0xd4, 0x1d, 0x8c, 0xd9, 0x8f, 0x00, 0xb2, 0x04,
                        0xe9, 0x80, 0x09, 0x98, 0xec, 0xf8, 0x42, 0x7e};

  for (int i = 0; i < 16; ++i)
    EXPECT_EQ(expected[i], digest[i]);
}

TEST(FXCRYPT, MD5GenerateOneByteData) {
  uint8_t digest[16];
  const char data[] = "a";
  uint32_t length = static_cast<uint32_t>(strlen(data));

  CRYPT_MD5Generate(reinterpret_cast<const uint8_t*>(data), length, digest);

  uint8_t expected[] = {0x0c, 0xc1, 0x75, 0xb9, 0xc0, 0xf1, 0xb6, 0xa8,
                        0x31, 0xc3, 0x99, 0xe2, 0x69, 0x77, 0x26, 0x61};

  for (int i = 0; i < 16; ++i)
    EXPECT_EQ(expected[i], digest[i]);
}

TEST(FXCRYPT, MD5GenerateLongData) {
  const uint32_t length = 10 * 1024 * 1024 + 1;
  std::unique_ptr<char[]> data(new char[length]);

  for (uint32_t i = 0; i < length; ++i)
    data[i] = i & 0xFF;

  uint8_t digest[16];
  CRYPT_MD5Generate(reinterpret_cast<const uint8_t*>(data.get()), length,
                    digest);

  uint8_t expected[] = {0x90, 0xbd, 0x6a, 0xd9, 0x0a, 0xce, 0xf5, 0xad,
                        0xaa, 0x92, 0x20, 0x3e, 0x21, 0xc7, 0xa1, 0x3e};

  for (int i = 0; i < 16; ++i)
    EXPECT_EQ(expected[i], digest[i]);
}

TEST(FXCRYPT, ContextWithEmptyData) {
  CRYPT_md5_context ctx;
  CRYPT_MD5Start(&ctx);

  uint8_t digest[16];
  CRYPT_MD5Finish(&ctx, digest);

  uint8_t expected[] = {0xd4, 0x1d, 0x8c, 0xd9, 0x8f, 0x00, 0xb2, 0x04,
                        0xe9, 0x80, 0x09, 0x98, 0xec, 0xf8, 0x42, 0x7e};

  for (int i = 0; i < 16; ++i)
    EXPECT_EQ(expected[i], digest[i]);
}

TEST(FXCRYPT, ContextWithLongData) {
  CRYPT_md5_context ctx;
  CRYPT_MD5Start(&ctx);

  const uint32_t length = 10 * 1024 * 1024 + 1;
  std::unique_ptr<uint8_t[]> data(new uint8_t[length]);

  for (uint32_t i = 0; i < length; ++i)
    data[i] = i & 0xFF;

  uint32_t total = 0;
  while (total < length) {
    uint32_t len = 4097;  // intentionally not 2^k.
    if (len > length - total)
      len = length - total;

    CRYPT_MD5Update(&ctx, data.get() + total, len);
    total += len;
  }

  EXPECT_EQ(length, total);

  uint8_t digest[16];
  CRYPT_MD5Finish(&ctx, digest);

  uint8_t expected[] = {0x90, 0xbd, 0x6a, 0xd9, 0x0a, 0xce, 0xf5, 0xad,
                        0xaa, 0x92, 0x20, 0x3e, 0x21, 0xc7, 0xa1, 0x3e};

  for (int i = 0; i < 16; ++i)
    EXPECT_EQ(expected[i], digest[i]);
}

// Example data from http://www.ietf.org/rfc/rfc1321.txt A.5 Test Suite
TEST(FXCRYPT, MD5StringTestSuite1) {
  std::string actual = CRYPT_MD5String("");
  std::string expected = "d41d8cd98f00b204e9800998ecf8427e";
  EXPECT_EQ(expected, actual);
}

TEST(FXCRYPT, MD5StringTestSuite2) {
  std::string actual = CRYPT_MD5String("a");
  std::string expected = "0cc175b9c0f1b6a831c399e269772661";
  EXPECT_EQ(expected, actual);
}

TEST(FXCRYPT, MD5StringTestSuite3) {
  std::string actual = CRYPT_MD5String("abc");
  std::string expected = "900150983cd24fb0d6963f7d28e17f72";
  EXPECT_EQ(expected, actual);
}

TEST(FXCRYPT, MD5StringTestSuite4) {
  std::string actual = CRYPT_MD5String("message digest");
  std::string expected = "f96b697d7cb7938d525a2f31aaf161d0";
  EXPECT_EQ(expected, actual);
}

TEST(FXCRYPT, MD5StringTestSuite5) {
  std::string actual = CRYPT_MD5String("abcdefghijklmnopqrstuvwxyz");
  std::string expected = "c3fcd3d76192e4007dfb496cca67e13b";
  EXPECT_EQ(expected, actual);
}

TEST(FXCRYPT, MD5StringTestSuite6) {
  std::string actual = CRYPT_MD5String(
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz"
      "0123456789");
  std::string expected = "d174ab98d277d9f5a5611c2c9f419d9f";
  EXPECT_EQ(expected, actual);
}

TEST(FXCRYPT, MD5StringTestSuite7) {
  std::string actual = CRYPT_MD5String(
      "12345678901234567890"
      "12345678901234567890"
      "12345678901234567890"
      "12345678901234567890");
  std::string expected = "57edf4a22be3c955ac49da2e2107b67a";
  EXPECT_EQ(expected, actual);
}

TEST(FXCRYPT, ContextWithStringData) {
  CRYPT_md5_context ctx;
  CRYPT_MD5Start(&ctx);
  CRYPT_MD5Update(&ctx, reinterpret_cast<const uint8_t*>("abc"), 3);

  uint8_t digest[16];
  CRYPT_MD5Finish(&ctx, digest);

  std::string actual = CryptToBase16(digest);
  std::string expected = "900150983cd24fb0d6963f7d28e17f72";
  EXPECT_EQ(expected, actual);
}

TEST(FXCRYPT, Sha256TestB1) {
  // Example B.1 from FIPS 180-2: one-block message.
  const char* input = "abc";
  const uint8_t expected[32] = {0xba, 0x78, 0x16, 0xbf, 0x8f, 0x01, 0xcf, 0xea,
                                0x41, 0x41, 0x40, 0xde, 0x5d, 0xae, 0x22, 0x23,
                                0xb0, 0x03, 0x61, 0xa3, 0x96, 0x17, 0x7a, 0x9c,
                                0xb4, 0x10, 0xff, 0x61, 0xf2, 0x00, 0x15, 0xad};
  uint8_t actual[32];
  CRYPT_SHA256Generate(reinterpret_cast<const uint8_t*>(input), strlen(input),
                       actual);
  for (size_t i = 0; i < 32; ++i)
    EXPECT_EQ(expected[i], actual[i]) << " at byte " << i;
}

TEST(FXCRYPT, Sha256TestB2) {
  // Example B.2 from FIPS 180-2: multi-block message.
  const char* input =
      "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
  const uint8_t expected[32] = {0x24, 0x8d, 0x6a, 0x61, 0xd2, 0x06, 0x38, 0xb8,
                                0xe5, 0xc0, 0x26, 0x93, 0x0c, 0x3e, 0x60, 0x39,
                                0xa3, 0x3c, 0xe4, 0x59, 0x64, 0xff, 0x21, 0x67,
                                0xf6, 0xec, 0xed, 0xd4, 0x19, 0xdb, 0x06, 0xc1};
  uint8_t actual[32];
  CRYPT_SHA256Generate(reinterpret_cast<const uint8_t*>(input), strlen(input),
                       actual);
  for (size_t i = 0; i < 32; ++i)
    EXPECT_EQ(expected[i], actual[i]) << " at byte " << i;
}

TEST(FXCRYPT, CRYPT_ArcFourSetup) {
  {
    const uint8_t kNullPermutation[kRC4ContextPermutationLength] = {
        0,   35,  3,   43,  9,   11,  65,  229, 32,  36,  134, 98,  59,  34,
        173, 153, 214, 200, 64,  161, 191, 62,  6,   25,  56,  234, 49,  246,
        69,  133, 203, 194, 10,  42,  228, 198, 195, 245, 236, 91,  206, 23,
        235, 27,  138, 18,  143, 250, 244, 76,  123, 217, 132, 249, 72,  127,
        94,  151, 33,  60,  248, 85,  177, 210, 142, 83,  110, 140, 41,  135,
        196, 238, 156, 242, 141, 67,  5,   185, 131, 63,  137, 37,  172, 121,
        70,  144, 237, 130, 17,  44,  253, 166, 78,  201, 12,  119, 215, 7,
        126, 114, 97,  192, 53,  4,   254, 45,  102, 122, 230, 88,  193, 129,
        160, 124, 84,  108, 239, 189, 152, 120, 115, 207, 50,  176, 86,  157,
        164, 187, 71,  1,   15,  58,  29,  21,  46,  145, 247, 162, 95,  183,
        13,  226, 159, 175, 221, 100, 96,  202, 101, 178, 154, 47,  205, 106,
        148, 104, 93,  112, 26,  165, 128, 186, 146, 218, 66,  211, 171, 90,
        252, 19,  40,  99,  223, 174, 255, 51,  77,  227, 48,  220, 168, 118,
        224, 103, 75,  105, 125, 199, 73,  82,  57,  181, 81,  149, 68,  52,
        232, 22,  2,   216, 113, 30,  109, 163, 92,  61,  14,  8,   38,  225,
        79,  231, 170, 240, 20,  219, 204, 150, 180, 188, 116, 190, 241, 197,
        179, 87,  74,  147, 80,  54,  212, 16,  167, 222, 136, 213, 55,  182,
        139, 24,  209, 251, 208, 28,  111, 89,  158, 155, 243, 107, 233, 169,
        117, 184, 31,  39};
    CRYPT_rc4_context s;
    CRYPT_ArcFourSetup(&s, nullptr, 0);
    EXPECT_EQ(0, s.x);
    EXPECT_EQ(0, s.y);
    for (int32_t i = 0; i < kRC4ContextPermutationLength; ++i)
      EXPECT_EQ(kNullPermutation[i], s.m[i]) << i;
  }
  {
    const uint8_t kFoobarPermutation[kRC4ContextPermutationLength] = {
        102, 214, 39,  49,  17,  132, 244, 106, 114, 76,  183, 212, 116, 73,
        42,  103, 128, 246, 139, 199, 31,  234, 25,  109, 48,  19,  121, 4,
        20,  54,  134, 77,  163, 38,  61,  101, 145, 78,  215, 96,  92,  80,
        224, 168, 243, 210, 82,  252, 113, 56,  217, 62,  218, 129, 125, 33,
        99,  9,   153, 59,  43,  13,  206, 124, 131, 18,  213, 118, 173, 122,
        193, 172, 177, 105, 148, 207, 186, 5,   85,  32,  68,  220, 79,  84,
        169, 209, 150, 7,   133, 63,  147, 93,  26,  130, 60,  117, 250, 57,
        24,  247, 200, 127, 136, 66,  112, 107, 140, 154, 70,  170, 185, 138,
        248, 236, 88,  86,  44,  216, 241, 35,  100, 151, 156, 74,  119, 55,
        245, 46,  227, 208, 229, 16,  249, 149, 53,  157, 201, 75,  58,  28,
        142, 238, 182, 180, 179, 144, 12,  6,   176, 10,  90,  239, 104, 40,
        181, 194, 137, 69,  221, 205, 165, 188, 191, 87,  1,   91,  2,   171,
        232, 34,  162, 166, 160, 126, 225, 167, 123, 197, 223, 195, 22,  203,
        189, 237, 37,  27,  222, 175, 23,  143, 152, 192, 21,  231, 228, 141,
        30,  204, 158, 240, 120, 98,  89,  83,  135, 251, 81,  196, 161, 3,
        8,   230, 52,  219, 41,  242, 36,  97,  15,  155, 65,  187, 254, 64,
        159, 67,  211, 108, 178, 146, 202, 11,  164, 226, 184, 50,  190, 174,
        71,  233, 235, 198, 95,  51,  110, 255, 253, 72,  115, 0,   47,  94,
        29,  45,  14,  111};
    CRYPT_rc4_context s;
    const uint8_t kFooBar[] = "foobar";
    CRYPT_ArcFourSetup(&s, kFooBar, 6);
    EXPECT_EQ(0, s.x);
    EXPECT_EQ(0, s.y);
    for (int32_t i = 0; i < kRC4ContextPermutationLength; ++i)
      EXPECT_EQ(kFoobarPermutation[i], s.m[i]) << i;
  }
}
