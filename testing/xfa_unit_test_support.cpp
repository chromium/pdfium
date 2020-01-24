// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/xfa_unit_test_support.h"

#include <memory>
#include <utility>

#include "core/fxge/cfx_fontmgr.h"
#include "core/fxge/cfx_gemodule.h"
#include "core/fxge/systemfontinfo_iface.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/base/ptr_util.h"
#include "xfa/fgas/font/cfgas_fontmgr.h"

namespace {

// The loading time of the CFGAS_FontMgr is linear in the number of times it is
// loaded. So, if a test suite has a lot of tests that need a font manager they
// can end up executing very, very slowly.
class XFATestEnvironment final : public testing::Environment {
 public:
  // testing::Environment:
  void SetUp() override {
    auto font_mgr = pdfium::MakeUnique<CFGAS_FontMgr>();
    if (font_mgr->EnumFonts())
      font_mgr_ = std::move(font_mgr);
  }
  void TearDown() override { font_mgr_.reset(); }

  CFGAS_FontMgr* FontManager() const { return font_mgr_.get(); }

 private:
  std::unique_ptr<CFGAS_FontMgr> font_mgr_;
};

XFATestEnvironment* g_env = nullptr;

}  // namespace

void InitializeXFATestEnvironment() {
  // |g_env| will be deleted by gtest.
  g_env = new XFATestEnvironment();
  AddGlobalTestEnvironment(g_env);

  // TODO(dsinclair): This font loading is slow. We should make a test font
  // loader which loads up a single font we use in all tests.
  CFX_GEModule::Get()->GetFontMgr()->SetSystemFontInfo(
      SystemFontInfoIface::CreateDefault(nullptr));
}

CFGAS_FontMgr* GetGlobalFontManager() {
  return g_env->FontManager();
}
