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

const wchar_t* const kLocales[] = {L"en", L"fr", L"jp", L"zh"};
const FX_DATETIMETYPE kTypes[] = {FX_DATETIMETYPE_Date, FX_DATETIMETYPE_Time,
                                  FX_DATETIMETYPE_DateTime,
                                  FX_DATETIMETYPE_TimeDate};

}  // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  if (size < 5 || size > 128)  // Big strings are unlikely to help.
    return 0;

  // Static for speed.
  static std::vector<std::unique_ptr<CXFA_LocaleMgr>> mgrs;
  if (mgrs.empty()) {
    for (const auto* locale : kLocales)
      mgrs.push_back(pdfium::MakeUnique<CXFA_LocaleMgr>(nullptr, locale));
  }

  uint8_t test_selector = data[0] % 10;
  uint8_t locale_selector = data[1] % FX_ArraySize(kLocales);
  uint8_t type_selector = data[2] % FX_ArraySize(kTypes);
  data += 3;
  size -= 3;

  size_t pattern_len = size / 2;
  size_t value_len = size - pattern_len;
  WideString pattern =
      WideString::FromLatin1(ByteStringView(data, pattern_len));
  WideString value =
      WideString::FromLatin1(ByteStringView(data + pattern_len, value_len));

  auto fmt = pdfium::MakeUnique<CFGAS_StringFormatter>(
      mgrs[locale_selector].get(), pattern);

  WideString result;
  CFX_DateTime dt;
  switch (test_selector) {
    case 0:
      fmt->FormatText(value, &result);
      break;
    case 1:
      fmt->FormatNum(value, &result);
      break;
    case 2:
      fmt->FormatDateTime(value, kTypes[type_selector], &result);
      break;
    case 3:
      fmt->FormatNull(&result);
      break;
    case 4:
      fmt->FormatZero(&result);
      break;
    case 5:
      fmt->ParseText(value, &result);
      break;
    case 6:
      fmt->ParseNum(value, &result);
      break;
    case 7:
      fmt->ParseDateTime(value, kTypes[type_selector], &dt);
      break;
    case 8:
      fmt->ParseNull(value);
      break;
    case 9:
      fmt->ParseZero(value);
      break;
  }
  return 0;
}
