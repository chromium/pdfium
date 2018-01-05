// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <algorithm>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "core/fxcrt/cfx_utf8decoder.h"
#include "core/fxcrt/cfx_widetextbuf.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/xml/cxml_content.h"
#include "core/fxcrt/xml/cxml_element.h"
#include "core/fxcrt/xml/cxml_parser.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"

namespace {

#define FXCRTM_XML_CHARTYPE_Normal 0x00
#define FXCRTM_XML_CHARTYPE_SpaceChar 0x01
#define FXCRTM_XML_CHARTYPE_Letter 0x02
#define FXCRTM_XML_CHARTYPE_Digital 0x04
#define FXCRTM_XML_CHARTYPE_NameIntro 0x08
#define FXCRTM_XML_CHARTYPE_NameChar 0x10
#define FXCRTM_XML_CHARTYPE_HexDigital 0x20
#define FXCRTM_XML_CHARTYPE_HexLowerLetter 0x40
#define FXCRTM_XML_CHARTYPE_HexUpperLetter 0x60
#define FXCRTM_XML_CHARTYPE_HexChar 0x60

const uint8_t g_FXCRT_XML_ByteTypes[256] = {
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0x00,
    0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x08, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x1A,
    0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A,
    0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x00, 0x00, 0x00, 0x00, 0x18,
    0x00, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A,
    0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A,
    0x1A, 0x1A, 0x1A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1A, 0x1A, 0x1A, 0x1A,
    0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A,
    0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A,
    0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A,
    0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A,
    0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A,
    0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A,
    0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A,
    0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A,
    0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A,
    0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A,
    0x1A, 0x1A, 0x01, 0x01,
};

constexpr int kMaxDepth = 1024;

bool g_FXCRT_XML_IsWhiteSpace(uint8_t ch) {
  return !!(g_FXCRT_XML_ByteTypes[ch] & FXCRTM_XML_CHARTYPE_SpaceChar);
}

bool g_FXCRT_XML_IsDigital(uint8_t ch) {
  return !!(g_FXCRT_XML_ByteTypes[ch] & FXCRTM_XML_CHARTYPE_Digital);
}

bool g_FXCRT_XML_IsNameIntro(uint8_t ch) {
  return !!(g_FXCRT_XML_ByteTypes[ch] & FXCRTM_XML_CHARTYPE_NameIntro);
}

bool g_FXCRT_XML_IsNameChar(uint8_t ch) {
  return !!(g_FXCRT_XML_ByteTypes[ch] & FXCRTM_XML_CHARTYPE_NameChar);
}

}  // namespace

CXML_Parser::CXML_Parser()
    : m_nOffset(0),
      m_pBuffer(nullptr),
      m_dwBufferSize(0),
      m_nBufferOffset(0),
      m_dwIndex(0) {}

CXML_Parser::~CXML_Parser() {}

bool CXML_Parser::Init(const uint8_t* pBuffer, size_t size) {
  m_pDataAcc = pdfium::MakeUnique<CXML_DataBufAcc>(pBuffer, size);
  m_nOffset = 0;
  return ReadNextBlock();
}

bool CXML_Parser::ReadNextBlock() {
  if (!m_pDataAcc->ReadNextBlock())
    return false;

  m_pBuffer = m_pDataAcc->GetBlockBuffer();
  m_dwBufferSize = m_pDataAcc->GetBlockSize();
  m_nBufferOffset = 0;
  m_dwIndex = 0;
  return m_dwBufferSize > 0;
}

bool CXML_Parser::IsEOF() {
  return m_pDataAcc->IsEOF() && m_dwIndex >= m_dwBufferSize;
}

void CXML_Parser::SkipWhiteSpaces() {
  m_nOffset = m_nBufferOffset + static_cast<FX_FILESIZE>(m_dwIndex);
  if (IsEOF())
    return;

  do {
    while (m_dwIndex < m_dwBufferSize &&
           g_FXCRT_XML_IsWhiteSpace(m_pBuffer[m_dwIndex])) {
      m_dwIndex++;
    }
    m_nOffset = m_nBufferOffset + static_cast<FX_FILESIZE>(m_dwIndex);
    if (m_dwIndex < m_dwBufferSize || IsEOF())
      break;
  } while (ReadNextBlock());
}

void CXML_Parser::GetName(ByteString* space, ByteString* name) {
  m_nOffset = m_nBufferOffset + static_cast<FX_FILESIZE>(m_dwIndex);
  if (IsEOF())
    return;

  std::ostringstream buf;
  do {
    while (m_dwIndex < m_dwBufferSize) {
      uint8_t ch = m_pBuffer[m_dwIndex];
      if (ch == ':') {
        *space = ByteString(buf);
        buf.str("");
      } else if (g_FXCRT_XML_IsNameChar(ch)) {
        buf << static_cast<char>(ch);
      } else {
        break;
      }
      m_dwIndex++;
    }
    m_nOffset = m_nBufferOffset + static_cast<FX_FILESIZE>(m_dwIndex);
    if (m_dwIndex < m_dwBufferSize || IsEOF())
      break;
  } while (ReadNextBlock());
  *name = ByteString(buf);
}

void CXML_Parser::SkipLiterals(const ByteStringView& str) {
  m_nOffset = m_nBufferOffset + static_cast<FX_FILESIZE>(m_dwIndex);
  if (IsEOF()) {
    return;
  }
  int32_t i = 0, iLen = str.GetLength();
  do {
    while (m_dwIndex < m_dwBufferSize) {
      if (str[i] != m_pBuffer[m_dwIndex++]) {
        i = 0;
        continue;
      }
      i++;
      if (i == iLen)
        break;
    }
    m_nOffset = m_nBufferOffset + static_cast<FX_FILESIZE>(m_dwIndex);
    if (i == iLen)
      return;

    if (m_dwIndex < m_dwBufferSize || IsEOF())
      break;
  } while (ReadNextBlock());
  while (!m_pDataAcc->IsEOF()) {
    ReadNextBlock();
    m_nOffset = m_nBufferOffset + static_cast<FX_FILESIZE>(m_dwBufferSize);
  }
  m_dwIndex = m_dwBufferSize;
}

uint32_t CXML_Parser::GetCharRef() {
  m_nOffset = m_nBufferOffset + static_cast<FX_FILESIZE>(m_dwIndex);
  if (IsEOF())
    return 0;

  uint8_t ch;
  int32_t iState = 0;
  std::ostringstream buf;
  uint32_t code = 0;
  do {
    while (m_dwIndex < m_dwBufferSize) {
      ch = m_pBuffer[m_dwIndex];
      switch (iState) {
        case 0:
          if (ch == '#') {
            m_dwIndex++;
            iState = 2;
            break;
          }
          iState = 1;
        case 1:
          m_dwIndex++;
          if (ch == ';') {
            std::string ref = buf.str();
            if (ref == "gt")
              code = '>';
            else if (ref == "lt")
              code = '<';
            else if (ref == "amp")
              code = '&';
            else if (ref == "apos")
              code = '\'';
            else if (ref == "quot")
              code = '"';
            iState = 10;
            break;
          }
          buf << static_cast<char>(ch);
          break;
        case 2:
          if (ch == 'x') {
            m_dwIndex++;
            iState = 4;
            break;
          }
          iState = 3;
        case 3:
          m_dwIndex++;
          if (ch == ';') {
            iState = 10;
            break;
          }
          if (g_FXCRT_XML_IsDigital(ch))
            code = code * 10 + FXSYS_DecimalCharToInt(static_cast<wchar_t>(ch));
          break;
        case 4:
          m_dwIndex++;
          if (ch == ';') {
            iState = 10;
            break;
          }
          uint8_t nHex =
              g_FXCRT_XML_ByteTypes[ch] & FXCRTM_XML_CHARTYPE_HexChar;
          if (nHex) {
            if (nHex == FXCRTM_XML_CHARTYPE_HexDigital) {
              code = (code << 4) +
                     FXSYS_DecimalCharToInt(static_cast<wchar_t>(ch));
            } else if (nHex == FXCRTM_XML_CHARTYPE_HexLowerLetter) {
              code = (code << 4) + ch - 87;
            } else {
              code = (code << 4) + ch - 55;
            }
          }
          break;
      }
      if (iState == 10)
        break;
    }
    m_nOffset = m_nBufferOffset + static_cast<FX_FILESIZE>(m_dwIndex);
    if (iState == 10 || m_dwIndex < m_dwBufferSize || IsEOF()) {
      break;
    }
  } while (ReadNextBlock());
  return code;
}

WideString CXML_Parser::GetAttrValue() {
  m_nOffset = m_nBufferOffset + static_cast<FX_FILESIZE>(m_dwIndex);
  if (IsEOF())
    return WideString();

  CFX_UTF8Decoder decoder;
  uint8_t mark = 0;
  uint8_t ch = 0;
  do {
    while (m_dwIndex < m_dwBufferSize) {
      ch = m_pBuffer[m_dwIndex];
      if (mark == 0) {
        if (ch != '\'' && ch != '"')
          return WideString();

        mark = ch;
        m_dwIndex++;
        ch = 0;
        continue;
      }
      m_dwIndex++;
      if (ch == mark)
        break;

      if (ch == '&') {
        decoder.AppendCodePoint(GetCharRef());
        if (IsEOF())
          return WideString(decoder.GetResult());
      } else {
        decoder.Input(ch);
      }
    }
    m_nOffset = m_nBufferOffset + static_cast<FX_FILESIZE>(m_dwIndex);
    if (ch == mark || m_dwIndex < m_dwBufferSize || IsEOF())
      break;
  } while (ReadNextBlock());
  return WideString(decoder.GetResult());
}

void CXML_Parser::GetTagName(bool bStartTag,
                             bool* bEndTag,
                             ByteString* space,
                             ByteString* name) {
  m_nOffset = m_nBufferOffset + static_cast<FX_FILESIZE>(m_dwIndex);
  if (IsEOF())
    return;

  *bEndTag = false;
  uint8_t ch;
  int32_t iState = bStartTag ? 1 : 0;
  do {
    while (m_dwIndex < m_dwBufferSize) {
      ch = m_pBuffer[m_dwIndex];
      switch (iState) {
        case 0:
          m_dwIndex++;
          if (ch != '<')
            break;

          iState = 1;
          break;
        case 1:
          if (ch == '?') {
            m_dwIndex++;
            SkipLiterals("?>");
            iState = 0;
            break;
          }
          if (ch == '!') {
            m_dwIndex++;
            SkipLiterals("-->");
            iState = 0;
            break;
          }
          if (ch == '/') {
            m_dwIndex++;
            GetName(space, name);
            *bEndTag = true;
          } else {
            GetName(space, name);
            *bEndTag = false;
          }
          return;
      }
    }
    m_nOffset = m_nBufferOffset + static_cast<FX_FILESIZE>(m_dwIndex);
    if (m_dwIndex < m_dwBufferSize || IsEOF())
      break;
  } while (ReadNextBlock());
}

std::unique_ptr<CXML_Element> CXML_Parser::ParseElement(CXML_Element* pParent,
                                                        bool bStartTag) {
  return ParseElementInternal(pParent, bStartTag, 0);
}

std::unique_ptr<CXML_Element> CXML_Parser::ParseElementInternal(
    CXML_Element* pParent,
    bool bStartTag,
    int nDepth) {
  if (nDepth > kMaxDepth)
    return nullptr;

  m_nOffset = m_nBufferOffset + static_cast<FX_FILESIZE>(m_dwIndex);
  if (IsEOF())
    return nullptr;

  ByteString tag_name;
  ByteString tag_space;
  bool bEndTag;
  GetTagName(bStartTag, &bEndTag, &tag_space, &tag_name);
  if (tag_name.IsEmpty() || bEndTag)
    return nullptr;

  auto pElement = pdfium::MakeUnique<CXML_Element>(
      pParent, tag_space.AsStringView(), tag_name.AsStringView());
  do {
    ByteString attr_space;
    ByteString attr_name;
    while (m_dwIndex < m_dwBufferSize) {
      SkipWhiteSpaces();
      if (IsEOF())
        break;

      if (!g_FXCRT_XML_IsNameIntro(m_pBuffer[m_dwIndex]))
        break;

      GetName(&attr_space, &attr_name);
      SkipWhiteSpaces();
      if (IsEOF())
        break;

      if (m_pBuffer[m_dwIndex] != '=')
        break;

      m_dwIndex++;
      SkipWhiteSpaces();
      if (IsEOF())
        break;

      WideString attr_value = GetAttrValue();
      pElement->SetAttribute(attr_space, attr_name, attr_value);
    }
    m_nOffset = m_nBufferOffset + static_cast<FX_FILESIZE>(m_dwIndex);
    if (m_dwIndex < m_dwBufferSize || IsEOF())
      break;
  } while (ReadNextBlock());
  SkipWhiteSpaces();
  if (IsEOF())
    return pElement;

  uint8_t ch = m_pBuffer[m_dwIndex++];
  if (ch == '/') {
    m_dwIndex++;
    m_nOffset = m_nBufferOffset + static_cast<FX_FILESIZE>(m_dwIndex);
    return pElement;
  }
  if (ch != '>') {
    m_nOffset = m_nBufferOffset + static_cast<FX_FILESIZE>(m_dwIndex);
    return nullptr;
  }
  SkipWhiteSpaces();
  if (IsEOF())
    return pElement;

  CFX_UTF8Decoder decoder;
  CFX_WideTextBuf content;
  bool bCDATA = false;
  int32_t iState = 0;
  do {
    while (m_dwIndex < m_dwBufferSize) {
      ch = m_pBuffer[m_dwIndex++];
      switch (iState) {
        case 0:
          if (ch == '<') {
            iState = 1;
          } else if (ch == '&') {
            decoder.ClearStatus();
            decoder.AppendCodePoint(GetCharRef());
          } else {
            decoder.Input(ch);
          }
          break;
        case 1:
          if (ch == '!') {
            iState = 2;
          } else if (ch == '?') {
            SkipLiterals("?>");
            SkipWhiteSpaces();
            iState = 0;
          } else if (ch == '/') {
            ByteString space;
            ByteString name;
            GetName(&space, &name);
            SkipWhiteSpaces();
            m_dwIndex++;
            iState = 10;
          } else {
            content << decoder.GetResult();
            WideString dataStr = content.MakeString();
            if (!bCDATA)
              dataStr.TrimRight(L" \t\r\n");

            InsertContentSegment(bCDATA, dataStr.AsStringView(),
                                 pElement.get());
            content.Clear();
            decoder.Clear();
            bCDATA = false;
            iState = 0;
            m_dwIndex--;
            std::unique_ptr<CXML_Element> pSubElement =
                ParseElementInternal(pElement.get(), true, nDepth + 1);
            if (!pSubElement)
              break;

            pElement->AppendChild(std::move(pSubElement));
            SkipWhiteSpaces();
          }
          break;
        case 2:
          if (ch == '[') {
            SkipLiterals("]]>");
          } else if (ch == '-') {
            m_dwIndex++;
            SkipLiterals("-->");
          } else {
            SkipLiterals(">");
          }
          decoder.Clear();
          SkipWhiteSpaces();
          iState = 0;
          break;
      }
      if (iState == 10) {
        break;
      }
    }
    m_nOffset = m_nBufferOffset + static_cast<FX_FILESIZE>(m_dwIndex);
    if (iState == 10 || m_dwIndex < m_dwBufferSize || IsEOF())
      break;
  } while (ReadNextBlock());
  content << decoder.GetResult();
  WideString dataStr = content.MakeString();
  dataStr.TrimRight(L" \t\r\n");

  InsertContentSegment(bCDATA, dataStr.AsStringView(), pElement.get());
  content.Clear();
  decoder.Clear();
  bCDATA = false;
  return pElement;
}

void CXML_Parser::InsertContentSegment(bool bCDATA,
                                       const WideStringView& content,
                                       CXML_Element* pElement) {
  if (content.IsEmpty())
    return;

  pElement->AppendChild(pdfium::MakeUnique<CXML_Content>(bCDATA, content));
}
