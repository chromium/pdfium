// Copyright 2019 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxge/cfx_fontmapper.h"

#include <memory>
#include <numeric>
#include <utility>

#include "core/fxcrt/fx_codepage.h"
#include "core/fxge/cfx_gemodule.h"
#include "core/fxge/systemfontinfo_iface.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

using testing::_;
using testing::DoAll;
using testing::ElementsAre;
using testing::InSequence;
using testing::Invoke;
using testing::Return;
using testing::WithArg;

class MockSystemFontInfo : public SystemFontInfoIface {
 public:
  MockSystemFontInfo() = default;
  ~MockSystemFontInfo() override = default;

  // SystemFontInfoIface:
  MOCK_METHOD(bool, EnumFontList, (CFX_FontMapper*), (override));
  MOCK_METHOD(void*,
              MapFont,
              (int, bool, FX_Charset, int, const ByteString&),
              (override));
  MOCK_METHOD(void*, GetFont, (const ByteString&), (override));
  MOCK_METHOD(size_t,
              GetFontData,
              (void*, uint32_t, pdfium::span<uint8_t>),
              (override));
  MOCK_METHOD(bool, GetFaceName, (void*, ByteString*), (override));
  MOCK_METHOD(bool, GetFontCharset, (void*, FX_Charset*), (override));
  MOCK_METHOD(void, DeleteFont, (void*), (override));
};

// Class that exposes private CFX_FontMapper methods.
class TestFontMapper : public CFX_FontMapper {
 public:
  TestFontMapper() : CFX_FontMapper(CFX_GEModule::Get()->GetFontMgr()) {}

  RetainPtr<CFX_Face> GetCachedTTCFace(void* font_handle,
                                       size_t ttc_size,
                                       size_t data_size) {
    return CFX_FontMapper::GetCachedTTCFace(font_handle, ttc_size, data_size);
  }

  RetainPtr<CFX_Face> GetCachedFace(void* font_handle,
                                    ByteString subst_name,
                                    int weight,
                                    bool is_italic,
                                    size_t data_size) {
    return CFX_FontMapper::GetCachedFace(font_handle, subst_name, weight,
                                         is_italic, data_size);
  }
};

class CFXFontMapperSystemFontInfoTest : public testing::Test {
 protected:
  CFXFontMapperSystemFontInfoTest() = default;
  ~CFXFontMapperSystemFontInfoTest() override = default;

  void SetUp() override {
    font_mapper_ = std::make_unique<TestFontMapper>();
    auto system_font_info = std::make_unique<MockSystemFontInfo>();
    system_font_info_ = system_font_info.get();
    font_mapper_->SetSystemFontInfo(std::move(system_font_info));
    font_mapper_->AddInstalledFont("dummy", FX_Charset::kANSI);
  }

  TestFontMapper& font_mapper() { return *font_mapper_; }
  MockSystemFontInfo& system_font_info() { return *system_font_info_; }

 private:
  // Must outlive `system_font_info_`.
  std::unique_ptr<TestFontMapper> font_mapper_;
  UnownedPtr<MockSystemFontInfo> system_font_info_;
};

// Deliberately give this global variable external linkage.
char g_maybe_changes = '\xff';

TEST(CFXFontMapperTest, IsStandardFontName) {
  EXPECT_TRUE(CFX_FontMapper::IsStandardFontName("Courier"));
  EXPECT_TRUE(CFX_FontMapper::IsStandardFontName("Courier-Bold"));
  EXPECT_TRUE(CFX_FontMapper::IsStandardFontName("Courier-BoldOblique"));
  EXPECT_TRUE(CFX_FontMapper::IsStandardFontName("Courier-Oblique"));
  EXPECT_TRUE(CFX_FontMapper::IsStandardFontName("Helvetica"));
  EXPECT_TRUE(CFX_FontMapper::IsStandardFontName("Helvetica-Bold"));
  EXPECT_TRUE(CFX_FontMapper::IsStandardFontName("Helvetica-BoldOblique"));
  EXPECT_TRUE(CFX_FontMapper::IsStandardFontName("Helvetica-Oblique"));
  EXPECT_TRUE(CFX_FontMapper::IsStandardFontName("Times-Roman"));
  EXPECT_TRUE(CFX_FontMapper::IsStandardFontName("Times-Bold"));
  EXPECT_TRUE(CFX_FontMapper::IsStandardFontName("Times-BoldItalic"));
  EXPECT_TRUE(CFX_FontMapper::IsStandardFontName("Times-Italic"));
  EXPECT_TRUE(CFX_FontMapper::IsStandardFontName("Symbol"));
  EXPECT_TRUE(CFX_FontMapper::IsStandardFontName("ZapfDingbats"));

  EXPECT_FALSE(CFX_FontMapper::IsStandardFontName("Courie"));
  EXPECT_FALSE(CFX_FontMapper::IsStandardFontName("Courier-"));
  EXPECT_FALSE(CFX_FontMapper::IsStandardFontName("Helvetica+Bold"));
  EXPECT_FALSE(CFX_FontMapper::IsStandardFontName("YapfDingbats"));
}

TEST(CFXFontMapperTest, MakeTag) {
  EXPECT_EQ(0x61626364u, CFX_FontMapper::MakeTag('a', 'b', 'c', 'd'));
  EXPECT_EQ(0x00000000u, CFX_FontMapper::MakeTag('\0', '\0', '\0', '\0'));
  EXPECT_EQ(0xfffe0a08u, CFX_FontMapper::MakeTag('\xff', '\xfe', '\n', '\b'));
  EXPECT_EQ(0xffffffffu,
            CFX_FontMapper::MakeTag('\xff', '\xff', '\xff', '\xff'));
  EXPECT_EQ(0xffffffffu,
            CFX_FontMapper::MakeTag(g_maybe_changes, '\xff', '\xff', '\xff'));
  EXPECT_EQ(0x6e616d65u, CFX_FontMapper::MakeTag('n', 'a', 'm', 'e'));
  EXPECT_EQ(0x4f532f32u, CFX_FontMapper::MakeTag('O', 'S', '/', '2'));
  EXPECT_EQ(FT_MAKE_TAG('G', 'S', 'U', 'B'),
            CFX_FontMapper::MakeTag('G', 'S', 'U', 'B'));
}

TEST(CFXFontMapperTest, AddInstalledFontBasic) {
  const char kFontName[] = "dummy";
  CFX_FontMapper font_mapper(nullptr);
  font_mapper.SetSystemFontInfo(std::make_unique<MockSystemFontInfo>());

  font_mapper.AddInstalledFont(kFontName, FX_Charset::kANSI);
  EXPECT_EQ(1u, font_mapper.GetFaceSize());
  EXPECT_EQ(kFontName, font_mapper.GetFaceName(0));
}

#ifdef PDF_ENABLE_XFA
TEST_F(CFXFontMapperSystemFontInfoTest, RawBytesForIndex) {
  {
    void* const kFontHandle = reinterpret_cast<void*>(12345);

    InSequence s;
    EXPECT_CALL(system_font_info(), MapFont).WillOnce(Return(kFontHandle));
    EXPECT_CALL(system_font_info(), GetFontData(kFontHandle, 0, _))
        .WillOnce(Return(2));
    EXPECT_CALL(system_font_info(), GetFontData(kFontHandle, 0, _))
        .WillOnce(DoAll(WithArg<2>(Invoke([](pdfium::span<uint8_t> buffer) {
                          buffer[0] = '0';
                          buffer[1] = '1';
                        })),
                        Return(2)));
    EXPECT_CALL(system_font_info(), DeleteFont(kFontHandle));
  }

  FixedSizeDataVector<uint8_t> data = font_mapper().RawBytesForIndex(0);
  EXPECT_THAT(data.span(), ElementsAre('0', '1'));
}

TEST_F(CFXFontMapperSystemFontInfoTest, RawBytesForIndexFailToMap) {
  EXPECT_CALL(system_font_info(), MapFont).WillOnce(Return(nullptr));

  FixedSizeDataVector<uint8_t> data = font_mapper().RawBytesForIndex(0);
  EXPECT_TRUE(data.empty());
}

TEST_F(CFXFontMapperSystemFontInfoTest, RawBytesForIndexFailToGetDataSize) {
  {
    void* const kFontHandle = reinterpret_cast<void*>(12345);

    InSequence s;
    EXPECT_CALL(system_font_info(), MapFont).WillOnce(Return(kFontHandle));
    EXPECT_CALL(system_font_info(), GetFontData(kFontHandle, 0, _))
        .WillOnce(Return(0));
    EXPECT_CALL(system_font_info(), DeleteFont(kFontHandle));
  }

  FixedSizeDataVector<uint8_t> data = font_mapper().RawBytesForIndex(0);
  EXPECT_TRUE(data.empty());
}

TEST_F(CFXFontMapperSystemFontInfoTest, RawBytesForIndexFailToGetData) {
  {
    void* const kFontHandle = reinterpret_cast<void*>(12345);

    InSequence s;
    EXPECT_CALL(system_font_info(), MapFont).WillOnce(Return(kFontHandle));
    EXPECT_CALL(system_font_info(), GetFontData(kFontHandle, 0, _))
        .WillOnce(Return(2));
    EXPECT_CALL(system_font_info(), GetFontData(kFontHandle, 0, _))
        .WillOnce(Return(0));
    EXPECT_CALL(system_font_info(), DeleteFont(kFontHandle));
  }

  FixedSizeDataVector<uint8_t> data = font_mapper().RawBytesForIndex(0);
  EXPECT_TRUE(data.empty());
}
#endif  // PDF_ENABLE_XFA

// Regression test for crbug.com/1372234 - should not crash.
TEST_F(CFXFontMapperSystemFontInfoTest, GetCachedTTCFaceFailToGetData) {
  void* const kFontHandle = reinterpret_cast<void*>(12345);
  constexpr size_t kTtcSize = 1024;
  constexpr size_t kDataSize = 2;

  {
    InSequence s;
    EXPECT_CALL(system_font_info(), GetFontData(kFontHandle, kTableTTCF, _))
        .WillOnce(DoAll(WithArg<2>(Invoke([&](pdfium::span<uint8_t> buffer) {
                          EXPECT_EQ(kTtcSize, buffer.size());
                          std::iota(buffer.begin(), buffer.end(), 0);
                        })),
                        Return(kTtcSize)));
    EXPECT_CALL(system_font_info(), GetFontData(kFontHandle, kTableTTCF, _))
        .WillOnce(Return(0));
  }

  EXPECT_FALSE(
      font_mapper().GetCachedTTCFace(kFontHandle, kTtcSize, kDataSize));
}

// Regression test for crbug.com/1372234 - should not crash.
TEST_F(CFXFontMapperSystemFontInfoTest, GetCachedFaceFailToGetData) {
  void* const kFontHandle = reinterpret_cast<void*>(12345);
  constexpr char kSubstName[] = "dummy_font";
  constexpr int kWeight = 400;
  constexpr bool kItalic = false;
  constexpr size_t kDataSize = 2;

  EXPECT_CALL(system_font_info(), GetFontData(kFontHandle, 0, _))
      .WillOnce(Return(0));

  EXPECT_FALSE(font_mapper().GetCachedFace(kFontHandle, kSubstName, kWeight,
                                           kItalic, kDataSize));
}
