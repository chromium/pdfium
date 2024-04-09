// Copyright 2018 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cstdint>
#include <memory>

#include "core/fxcrt/compiler_specific.h"
#include "core/fxcrt/fx_codepage.h"
#include "core/fxcrt/span.h"
#include "core/fxcrt/widestring.h"
#include "core/fxge/cfx_font.h"
#include "core/fxge/fx_font.h"
#include "xfa/fgas/font/cfgas_gefont.h"
#include "xfa/fgas/layout/cfgas_char.h"
#include "xfa/fgas/layout/cfgas_rtfbreak.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  if (size > 8192)
    return 0;

  auto font = std::make_unique<CFX_Font>();
  font->LoadSubst("Arial", true, 0, FXFONT_FW_NORMAL, 0, FX_CodePage::kDefANSI,
                  false);
  assert(font);

  CFGAS_RTFBreak rtf_break(CFGAS_Break::LayoutStyle::kExpandTab);
  rtf_break.SetLineBreakTolerance(1);
  rtf_break.SetFont(CFGAS_GEFont::LoadFont(std::move(font)));
  rtf_break.SetFontSize(12);

  // SAFETY: trusted arguments from fuzzer.
  auto span = UNSAFE_BUFFERS(pdfium::make_span(data, size));
  WideString input = WideString::FromUTF16LE(span);
  for (wchar_t ch : input)
    rtf_break.AppendChar(ch);

  std::vector<CFGAS_Char> chars =
      rtf_break.GetCurrentLineForTesting()->m_LineChars;
  CFGAS_Char::BidiLine(&chars, chars.size());
  return 0;
}
