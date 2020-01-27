// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/font/cpdf_tounicodemap.h"

#include <utility>

#include "core/fpdfapi/font/cpdf_cid2unicodemap.h"
#include "core/fpdfapi/font/cpdf_fontglobals.h"
#include "core/fpdfapi/parser/cpdf_simple_parser.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/fx_safe_types.h"
#include "third_party/base/numerics/safe_conversions.h"

namespace {

WideString StringDataAdd(WideString str) {
  WideString ret;
  wchar_t value = 1;
  for (size_t i = str.GetLength(); i > 0; --i) {
    wchar_t ch = str[i - 1] + value;
    if (ch < str[i - 1]) {
      ret.InsertAtFront(0);
    } else {
      ret.InsertAtFront(ch);
      value = 0;
    }
  }
  if (value)
    ret.InsertAtFront(value);
  return ret;
}

}  // namespace

CPDF_ToUnicodeMap::CPDF_ToUnicodeMap(const CPDF_Stream* pStream) {
  Load(pStream);
}

CPDF_ToUnicodeMap::~CPDF_ToUnicodeMap() = default;

WideString CPDF_ToUnicodeMap::Lookup(uint32_t charcode) const {
  auto it = m_Map.find(charcode);
  if (it == m_Map.end()) {
    if (!m_pBaseMap)
      return WideString();
    return m_pBaseMap->UnicodeFromCID(static_cast<uint16_t>(charcode));
  }

  uint32_t value = it->second;
  wchar_t unicode = static_cast<wchar_t>(value & 0xffff);
  if (unicode != 0xffff)
    return unicode;

  WideStringView buf = m_MultiCharBuf.AsStringView();
  size_t index = value >> 16;
  if (!buf.IsValidIndex(index))
    return WideString();
  return WideString(buf.Substr(index + 1, buf[index]));
}

uint32_t CPDF_ToUnicodeMap::ReverseLookup(wchar_t unicode) const {
  for (const auto& pair : m_Map) {
    if (pair.second == static_cast<uint32_t>(unicode))
      return pair.first;
  }
  return 0;
}

// static
pdfium::Optional<uint32_t> CPDF_ToUnicodeMap::StringToCode(ByteStringView str) {
  size_t len = str.GetLength();
  if (len <= 2 || str[0] != '<' || str[len - 1] != '>')
    return pdfium::nullopt;

  FX_SAFE_UINT32 code = 0;
  for (char c : str.Substr(1, len - 2)) {
    if (!FXSYS_IsHexDigit(c))
      return pdfium::nullopt;

    code = code * 16 + FXSYS_HexCharToInt(c);
    if (!code.IsValid())
      return pdfium::nullopt;
  }
  return pdfium::Optional<uint32_t>(code.ValueOrDie());
}

// static
WideString CPDF_ToUnicodeMap::StringToWideString(ByteStringView str) {
  size_t len = str.GetLength();
  if (len <= 2 || str[0] != '<' || str[len - 1] != '>')
    return WideString();

  WideString result;
  int byte_pos = 0;
  wchar_t ch = 0;
  for (char c : str.Substr(1, len - 2)) {
    if (!FXSYS_IsHexDigit(c))
      break;

    ch = ch * 16 + FXSYS_HexCharToInt(c);
    byte_pos++;
    if (byte_pos == 4) {
      result += ch;
      byte_pos = 0;
      ch = 0;
    }
  }
  return result;
}

void CPDF_ToUnicodeMap::Load(const CPDF_Stream* pStream) {
  CIDSet cid_set = CIDSET_UNKNOWN;
  auto pAcc = pdfium::MakeRetain<CPDF_StreamAcc>(pStream);
  pAcc->LoadAllDataFiltered();
  CPDF_SimpleParser parser(pAcc->GetSpan());
  while (1) {
    ByteStringView word = parser.GetWord();
    if (word.IsEmpty())
      break;

    if (word == "beginbfchar")
      HandleBeginBFChar(&parser);
    else if (word == "beginbfrange")
      HandleBeginBFRange(&parser);
    else if (word == "/Adobe-Korea1-UCS2")
      cid_set = CIDSET_KOREA1;
    else if (word == "/Adobe-Japan1-UCS2")
      cid_set = CIDSET_JAPAN1;
    else if (word == "/Adobe-CNS1-UCS2")
      cid_set = CIDSET_CNS1;
    else if (word == "/Adobe-GB1-UCS2")
      cid_set = CIDSET_GB1;
  }
  if (cid_set) {
    auto* manager = CPDF_FontGlobals::GetInstance()->GetCMapManager();
    m_pBaseMap = manager->GetCID2UnicodeMap(cid_set);
  }
}

void CPDF_ToUnicodeMap::HandleBeginBFChar(CPDF_SimpleParser* pParser) {
  while (1) {
    ByteStringView word = pParser->GetWord();
    if (word.IsEmpty() || word == "endbfchar")
      return;

    pdfium::Optional<uint32_t> code = StringToCode(word);
    if (!code.has_value())
      return;

    SetCode(code.value(), StringToWideString(pParser->GetWord()));
  }
}

void CPDF_ToUnicodeMap::HandleBeginBFRange(CPDF_SimpleParser* pParser) {
  while (1) {
    ByteStringView lowcode_str = pParser->GetWord();
    if (lowcode_str.IsEmpty() || lowcode_str == "endbfrange")
      return;

    pdfium::Optional<uint32_t> lowcode_opt = StringToCode(lowcode_str);
    if (!lowcode_opt.has_value())
      return;

    ByteStringView highcode_str = pParser->GetWord();
    pdfium::Optional<uint32_t> highcode_opt = StringToCode(highcode_str);
    if (!highcode_opt.has_value())
      return;

    uint32_t lowcode = lowcode_opt.value();
    uint32_t highcode = (lowcode & 0xffffff00) | (highcode_opt.value() & 0xff);

    ByteStringView start = pParser->GetWord();
    if (start == "[") {
      for (uint32_t code = lowcode; code <= highcode; code++)
        SetCode(code, StringToWideString(pParser->GetWord()));
      pParser->GetWord();
      continue;
    }

    WideString destcode = StringToWideString(start);
    if (destcode.GetLength() == 1) {
      pdfium::Optional<uint32_t> value_or_error = StringToCode(start);
      if (!value_or_error.has_value())
        return;

      uint32_t value = value_or_error.value();
      for (uint32_t code = lowcode; code <= highcode; code++)
        m_Map[code] = value++;
    } else {
      for (uint32_t code = lowcode; code <= highcode; code++) {
        WideString retcode =
            code == lowcode ? destcode : StringDataAdd(destcode);
        m_Map[code] = GetUnicode();
        m_MultiCharBuf.AppendChar(retcode.GetLength());
        m_MultiCharBuf << retcode;
        destcode = std::move(retcode);
      }
    }
  }
}

uint32_t CPDF_ToUnicodeMap::GetUnicode() const {
  FX_SAFE_UINT32 uni = m_MultiCharBuf.GetLength();
  uni = uni * 0x10000 + 0xffff;
  return uni.ValueOrDefault(0);
}

void CPDF_ToUnicodeMap::SetCode(uint32_t srccode, WideString destcode) {
  size_t len = destcode.GetLength();
  if (len == 0)
    return;

  if (len == 1) {
    m_Map[srccode] = destcode[0];
  } else {
    m_Map[srccode] = GetUnicode();
    m_MultiCharBuf.AppendChar(len);
    m_MultiCharBuf << destcode;
  }
}
