// Copyright 2021 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/test_fonts.h"

#include <set>
#include <utility>

#include "core/fxge/cfx_fontmapper.h"
#include "core/fxge/cfx_fontmgr.h"
#include "core/fxge/cfx_gemodule.h"
#include "core/fxge/systemfontinfo_iface.h"
#include "testing/utils/path_service.h"

namespace {

ByteString RenameFontForTesting(const ByteString& face) {
  ByteString result;
  if (face.Contains("Arial") || face.Contains("Calibri") ||
      face.Contains("Helvetica")) {
    // Sans
    result = "Arimo";
  } else if (face.IsEmpty() || face.Contains("Times")) {
    // Serif
    result = "Tinos";
  } else if (face.Contains("Courier")) {
    // Mono
    result = "Cousine";
  } else {
    // Some tests expect the fallback font.
    return face;
  }

  if (face.Contains("Bold"))
    result += " Bold";

  if (face.Contains("Italic") || face.Contains("Oblique"))
    result += " Italic";

  return result;
}

// Intercepts font requests and renames font faces to those in test_fonts.
class SystemFontInfoWrapper : public SystemFontInfoIface {
 public:
  explicit SystemFontInfoWrapper(std::unique_ptr<SystemFontInfoIface> impl)
      : impl_(std::move(impl)) {}
  ~SystemFontInfoWrapper() { CHECK(active_fonts_.empty()); }

  bool EnumFontList(CFX_FontMapper* pMapper) override {
    return impl_->EnumFontList(pMapper);
  }
  void* MapFont(int weight,
                bool bItalic,
                FX_Charset charset,
                int pitch_family,
                const ByteString& face) override {
    void* font = impl_->MapFont(weight, bItalic, charset, pitch_family,
                                RenameFontForTesting(face));
    if (font) {
      bool inserted = active_fonts_.insert(font).second;
      CHECK(inserted);
    }
    return font;
  }
  void* GetFont(const ByteString& face) override {
    return impl_->GetFont(RenameFontForTesting(face));
  }
  size_t GetFontData(void* hFont,
                     uint32_t table,
                     pdfium::span<uint8_t> buffer) override {
    return impl_->GetFontData(hFont, table, buffer);
  }
  bool GetFaceName(void* hFont, ByteString* name) override {
    auto face = RenameFontForTesting(*name);
    return impl_->GetFaceName(hFont, &face);
  }
  bool GetFontCharset(void* hFont, FX_Charset* charset) override {
    return impl_->GetFontCharset(hFont, charset);
  }
  void DeleteFont(void* hFont) override {
    CHECK(active_fonts_.erase(hFont));
    impl_->DeleteFont(hFont);
  }

 private:
  std::unique_ptr<SystemFontInfoIface> impl_;
  std::set<void*> active_fonts_;
};

}  // namespace

TestFonts::TestFonts() {
  if (!PathService::GetExecutableDir(&font_path_))
    return;
  font_path_.push_back(PATH_SEPARATOR);
  font_path_.append("test_fonts");
  font_paths_ = std::make_unique<const char*[]>(2);
  font_paths_[0] = font_path_.c_str();
  font_paths_[1] = nullptr;
}

TestFonts::~TestFonts() = default;

void TestFonts::InstallFontMapper() {
  auto* font_mapper = CFX_GEModule::Get()->GetFontMgr()->GetBuiltinMapper();
  font_mapper->SetSystemFontInfo(std::make_unique<SystemFontInfoWrapper>(
      font_mapper->TakeSystemFontInfo()));
}

// static
std::string TestFonts::RenameFont(const char* face) {
  ByteString renamed_face = RenameFontForTesting(face);
  return std::string(renamed_face.c_str());
}
