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
  // TODO(dsinclair): This font loading is slow. We should make a test font
  // loader which loads up a single font we use in all tests.
  CFX_GEModule::Get()->GetFontMgr()->SetSystemFontInfo(
      SystemFontInfoIface::CreateDefault(nullptr));

  auto font_mgr = std::make_unique<CFGAS_FontMgr>();
  if (font_mgr->EnumFonts())
    font_mgr_ = std::move(font_mgr);
}

void XFATestEnvironment::TearDown() {
  font_mgr_.reset();
}

// static
CFGAS_FontMgr* XFATestEnvironment::GetGlobalFontManager() {
  return g_env->font_mgr_.get();
}
