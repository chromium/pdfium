// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/xfa_test_environment.h"

#include <memory>
#include <utility>

#include "core/fxge/cfx_fontmgr.h"
#include "core/fxge/cfx_gemodule.h"
#include "core/fxge/systemfontinfo_iface.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/base/logging.h"
#include "xfa/fgas/font/cfgas_fontmgr.h"

namespace {

XFATestEnvironment* g_env = nullptr;

}  // namespace

XFATestEnvironment::XFATestEnvironment() {
  ASSERT(!g_env);
  g_env = this;
}

XFATestEnvironment::~XFATestEnvironment() {
  ASSERT(g_env);
  g_env = nullptr;
}

void XFATestEnvironment::SetUp() {
  // This font loading is slow, but we do it only once per binary
  // execution, not once per test.
  CFX_GEModule::Get()->GetFontMgr()->SetSystemFontInfo(
      CFX_GEModule::Get()->GetPlatform()->CreateDefaultSystemFontInfo());

  font_mgr_ = std::make_unique<CFGAS_FontMgr>();
  CHECK(font_mgr_->EnumFonts());
}

void XFATestEnvironment::TearDown() {
  font_mgr_.reset();
}

// static
CFGAS_FontMgr* XFATestEnvironment::GetGlobalFontManager() {
  return g_env->font_mgr_.get();
}
