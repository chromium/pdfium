// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xfa/fgas/crt/cfgas_stringformatter.h"

#include <stdint.h>

#include "core/fxcrt/fx_memory.h"
#include "core/fxcrt/fx_string.h"
#include "third_party/base/ptr_util.h"
#include "xfa/fxfa/parser/cxfa_localemgr.h"

namespace {

const wchar_t* const kLocales[] = {L"en", L"fr", L"jp", L"cz"};
const FX_DATETIMETYPE kTypes[] = {FX_DATETIMETYPE_Date, FX_DATETIMETYPE_Time,
                                  FX_DATETIMETYPE_DateTime,
                                  FX_DATETIMETYPE_TimeDate};

}  // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  if (size < 4)
    return 0;

  uint8_t locale_selector = data[0] % FX_ArraySize(kLocales);
  uint8_t type_selector = data[1] % FX_ArraySize(kTypes);
  data += 2;
  size -= 2;

  size_t pattern_len = size / 2;
  size_t value_len = size - pattern_len;
  WideString pattern =
      WideString::FromLatin1(ByteStringView(data, pattern_len));
  WideString value =
      WideString::FromLatin1(ByteStringView(data + pattern_len, value_len));
  WideString result;
  auto mgr =
      pdfium::MakeUnique<CXFA_LocaleMgr>(nullptr, kLocales[locale_selector]);
  auto fmt = pdfium::MakeUnique<CFGAS_StringFormatter>(mgr.get(), pattern);
  fmt->FormatText(value, &result);
  fmt->FormatNum(value, &result);
  fmt->FormatDateTime(value, kTypes[type_selector], &result);
  return 0;
}
