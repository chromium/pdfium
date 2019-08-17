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

  const wchar_t* buf = m_MultiCharBuf.GetBuffer();
  uint32_t buf_len = m_MultiCharBuf.GetLength();
  if (!buf || buf_len == 0)
    return WideString();

  uint32_t index = value >> 16;
  if (index >= buf_len)
    return WideString();

  uint32_t len = buf[index];
  if (index + len < index || index + len >= buf_len)
    return WideString();

  return WideString(buf + index + 1, len);
}

uint32_t CPDF_ToUnicodeMap::ReverseLookup(wchar_t unicode) const {
  for (const auto& pair : m_Map) {
    if (pair.second == static_cast<uint32_t>(unicode))
      return pair.first;
  }
  return 0;
}

// static
uint32_t CPDF_ToUnicodeMap::StringToCode(ByteStringView str) {
  size_t len = str.GetLength();
  if (len == 0)
    return 0;

  uint32_t result = 0;
  if (str[0] == '<') {
    for (size_t i = 1; i < len && std::isxdigit(str[i]); ++i)
      result = result * 16 + FXSYS_HexCharToInt(str.CharAt(i));
    return result;
  }

  for (size_t i = 0; i < len && std::isdigit(str[i]); ++i)
    result = result * 10 + FXSYS_DecimalCharToInt(str.CharAt(i));

  return result;
}

// static
WideString CPDF_ToUnicodeMap::StringToWideString(ByteStringView str) {
  size_t len = str.GetLength();
  if (len == 0 || str[0] != '<')
    return WideString();

  WideString result;
  int byte_pos = 0;
  wchar_t ch = 0;
  for (size_t i = 1; i < len && std::isxdigit(str[i]); ++i) {
    ch = ch * 16 + FXSYS_HexCharToInt(str.CharAt(i));
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

    SetCode(StringToCode(word), StringToWideString(pParser->GetWord()));
  }
}

void CPDF_ToUnicodeMap::HandleBeginBFRange(CPDF_SimpleParser* pParser) {
  while (1) {
    ByteStringView low = pParser->GetWord();
    if (low.IsEmpty() || low == "endbfrange")
      return;

    ByteStringView high = pParser->GetWord();
    uint32_t lowcode = StringToCode(low);
    uint32_t highcode = (lowcode & 0xffffff00) | (StringToCode(high) & 0xff);
    if (highcode == 0xffffffff)
      return;

    ByteStringView start = pParser->GetWord();
    if (start == "[") {
      for (uint32_t code = lowcode; code <= highcode; code++)
        SetCode(code, StringToWideString(pParser->GetWord()));
      pParser->GetWord();
      continue;
    }

    WideString destcode = StringToWideString(start);
    if (destcode.GetLength() == 1) {
      uint32_t value = StringToCode(start);
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
