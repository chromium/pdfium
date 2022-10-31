// Copyright 2020 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/embedder_test_environment.h"

#include <ostream>

#include "core/fxcrt/fx_system.h"
#include "public/fpdfview.h"
#include "testing/command_line_helpers.h"
#include "third_party/base/check.h"

#ifdef PDF_ENABLE_V8
#include "testing/v8_test_environment.h"
#endif  // PDF_ENABLE_V8

namespace {

EmbedderTestEnvironment* g_environment = nullptr;

}  // namespace

EmbedderTestEnvironment::EmbedderTestEnvironment()
    : renderer_type_(GetDefaultRendererType()) {
  DCHECK(!g_environment);
  g_environment = this;
}

EmbedderTestEnvironment::~EmbedderTestEnvironment() {
  DCHECK(g_environment);
  g_environment = nullptr;
}

// static
EmbedderTestEnvironment* EmbedderTestEnvironment::GetInstance() {
  return g_environment;
}

void EmbedderTestEnvironment::SetUp() {
  FPDF_LIBRARY_CONFIG config;
  config.version = 4;
  config.m_pUserFontPaths = nullptr;
  config.m_v8EmbedderSlot = 0;
  config.m_pPlatform = nullptr;

  config.m_pUserFontPaths = test_fonts_.font_paths();

#ifdef PDF_ENABLE_V8
  config.m_pIsolate = V8TestEnvironment::GetInstance()->isolate();
  config.m_pPlatform = V8TestEnvironment::GetInstance()->platform();
#else   // PDF_ENABLE_V8
  config.m_pIsolate = nullptr;
  config.m_pPlatform = nullptr;
#endif  // PDF_ENABLE_V8
  config.m_RendererType = renderer_type_;

  FPDF_InitLibraryWithConfig(&config);

  test_fonts_.InstallFontMapper();
}

void EmbedderTestEnvironment::TearDown() {
  FPDF_DestroyLibrary();
}

void EmbedderTestEnvironment::AddFlags(int argc, char** argv) {
  for (int i = 1; i < argc; ++i)
    AddFlag(argv[i]);
}

void EmbedderTestEnvironment::AddFlag(const std::string& flag) {
  if (flag == "--write-pngs") {
    write_pngs_ = true;
    return;
  }
#if defined(_SKIA_SUPPORT_)
  std::string value;
  if (ParseSwitchKeyValue(flag, "--use-renderer=", &value)) {
    if (value == "agg") {
      renderer_type_ = FPDF_RENDERERTYPE_AGG;
    } else if (value == "skia") {
      renderer_type_ = FPDF_RENDERERTYPE_SKIA;
    } else {
      std::cerr << "Invalid --use-renderer argument, value must be one of agg "
                   "or skia\n";
    }
    return;
  }
#endif  // defined(_SKIA_SUPPORT_)

  std::cerr << "Unknown flag: " << flag << "\n";
}
