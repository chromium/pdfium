// Copyright 2019 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/xfa_test_environment.h"

#include "core/fxge/cfx_fontmgr.h"
#include "core/fxge/cfx_gemodule.h"
#include "core/fxge/systemfontinfo_iface.h"
#include "third_party/base/check.h"
#include "xfa/fgas/font/cfgas_gemodule.h"

namespace {

XFATestEnvironment* g_env = nullptr;

}  // namespace

XFATestEnvironment::XFATestEnvironment() {
  DCHECK(!g_env);
  g_env = this;
}

XFATestEnvironment::~XFATestEnvironment() {
  DCHECK(g_env);
  g_env = nullptr;
}

void XFATestEnvironment::SetUp() {
  CFX_GEModule::Get()->GetFontMgr()->GetBuiltinMapper()->SetSystemFontInfo(
      CFX_GEModule::Get()->GetPlatform()->CreateDefaultSystemFontInfo());

  // The font loading that takes place in CFGAS_GEModule::Create() is slow,
  // but we do it only once per binary execution, not once per test.
  CFGAS_GEModule::Create();
}

void XFATestEnvironment::TearDown() {
  CFGAS_GEModule::Destroy();
}
