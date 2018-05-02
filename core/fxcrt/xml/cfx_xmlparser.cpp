// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/xml/cfx_xmlparser.h"

#include <algorithm>
#include <cwctype>
#include <iterator>
#include <utility>

#include "core/fxcrt/cfx_seekablestreamproxy.h"
#include "core/fxcrt/fx_codepage.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/xml/cfx_xmlchardata.h"
#include "core/fxcrt/xml/cfx_xmldocument.h"
#include "core/fxcrt/xml/cfx_xmlelement.h"
#include "core/fxcrt/xml/cfx_xmlinstruction.h"
#include "core/fxcrt/xml/cfx_xmlnode.h"
#include "core/fxcrt/xml/cfx_xmltext.h"
#include "third_party/base/ptr_util.h"

namespace {

constexpr size_t kCurrentTextReserve = 128;
constexpr uint32_t kMaxCharRange = 0x10ffff;

bool IsXMLWhiteSpace(wchar_t ch) {
  return ch == L' ' || ch == 0x0A || ch == 0x0D || ch == 0x09;
}

struct FX_XMLNAMECHAR {
  uint16_t wStart;
  uint16_t wEnd;
  bool bStartChar;
};

const FX_XMLNAMECHAR g_XMLNameChars[] = {
    {L'-', L'.', false},    {L'0', L'9', false},     {L':', L':', false},
    {L'A', L'Z', true},     {L'_', L'_', true},      {L'a', L'z', true},
    {0xB7, 0xB7, false},    {0xC0, 0xD6, true},      {0xD8, 0xF6, true},
    {0xF8, 0x02FF, true},   {0x0300, 0x036F, false}, {0x0370, 0x037D, true},
    {0x037F, 0x1FFF, true}, {0x200C, 0x200D, true},  {0x203F, 0x2040, false},
    {0x2070, 0x218F, true}, {0x2C00, 0x2FEF, true},  {0x3001, 0xD7FF, true},
    {0xF900, 0xFDCF, true}, {0xFDF0, 0xFFFD, true},
};

}  // namespace

// static
bool CFX_XMLParser::IsXMLNameChar(wchar_t ch, bool bFirstChar) {
  auto* it = std::lower_bound(
      std::begin(g_XMLNameChars), std::end(g_XMLNameChars), ch,
      [](const FX_XMLNAMECHAR& arg, wchar_t ch) { return arg.wEnd < ch; });
  return it != std::end(g_XMLNameChars) && ch >= it->wStart &&
         (!bFirstChar || it->bStartChar);
}

CFX_XMLParser::CFX_XMLParser(const RetainPtr<IFX_SeekableReadStream>& pStream) {
  ASSERT(pStream);

  auto proxy = pdfium::MakeRetain<CFX_SeekableStreamProxy>(pStream);
  uint16_t wCodePage = proxy->GetCodePage();
  if (wCodePage != FX_CODEPAGE_UTF16LE && wCodePage != FX_CODEPAGE_UTF16BE &&
      wCodePage != FX_CODEPAGE_UTF8) {
    proxy->SetCodePage(FX_CODEPAGE_UTF8);
  }
  m_pStream = proxy;

  m_iXMLPlaneSize =
      std::min(m_iXMLPlaneSize,
               pdfium::base::checked_cast<size_t>(m_pStream->GetSize()));

  current_text_.reserve(kCurrentTextReserve);
}

CFX_XMLParser::~CFX_XMLParser() = default;

std::unique_ptr<CFX_XMLDocument> CFX_XMLParser::Parse() {
  auto doc = pdfium::MakeUnique<CFX_XMLDocument>();
  current_node_ = doc->GetRoot();

  FX_SAFE_SIZE_T alloc_size_safe = m_iXMLPlaneSize;
  alloc_size_safe += 1;  // For NUL.
  if (!alloc_size_safe.IsValid() || alloc_size_safe.ValueOrDie() <= 0)
    return nullptr;

  m_Buffer.resize(pdfium::base::ValueOrDieForType<size_t>(alloc_size_safe));

  return DoSyntaxParse(doc.get()) ? std::move(doc) : nullptr;
}

bool CFX_XMLParser::DoSyntaxParse(CFX_XMLDocument* doc) {
  int32_t iCount = 0;
  while (true) {
    if (m_Start >= m_End) {
      if (m_pStream->IsEOF())
        return true;

      size_t buffer_chars =
          m_pStream->ReadBlock(m_Buffer.data(), m_iXMLPlaneSize);
      if (buffer_chars == 0)
        return true;

      m_Start = 0;
      m_End = buffer_chars;
    }

    while (m_Start < m_End) {
      wchar_t ch = m_Buffer[m_Start];
      switch (m_syntaxParserState) {
        case FDE_XmlSyntaxState::Text:
          if (ch == L'<') {
            if (!current_text_.empty()) {
              current_node_->AppendChild(
                  doc->CreateNode<CFX_XMLText>(GetTextData()));
            } else {
              m_Start++;
              m_syntaxParserState = FDE_XmlSyntaxState::Node;
            }
          } else {
            ProcessTextChar(ch);
          }
          break;
        case FDE_XmlSyntaxState::Node:
          if (ch == L'!') {
            m_Start++;
            m_syntaxParserState = FDE_XmlSyntaxState::SkipCommentOrDecl;
          } else if (ch == L'/') {
            m_Start++;
            m_syntaxParserState = FDE_XmlSyntaxState::CloseElement;
          } else if (ch == L'?') {
            m_XMLNodeTypeStack.push(FX_XMLNODE_Instruction);
            m_Start++;
            m_syntaxParserState = FDE_XmlSyntaxState::Target;
          } else {
            m_XMLNodeTypeStack.push(FX_XMLNODE_Element);
            m_syntaxParserState = FDE_XmlSyntaxState::Tag;
          }
          break;
        case FDE_XmlSyntaxState::Target:
          if (!IsXMLNameChar(ch, current_text_.empty())) {
            if (current_text_.empty())
              return false;

            m_syntaxParserState = FDE_XmlSyntaxState::TargetData;

            WideString target_name = GetTextData();
            if (target_name == L"originalXFAVersion" ||
                target_name == L"acrobat") {
              auto* node = doc->CreateNode<CFX_XMLInstruction>(target_name);
              current_node_->AppendChild(node);
              current_node_ = node;
            }
          } else {
            current_text_.push_back(ch);
            m_Start++;
          }
          break;
        case FDE_XmlSyntaxState::Tag:
          if (!IsXMLNameChar(ch, current_text_.empty())) {
            if (current_text_.empty())
              return false;

            m_syntaxParserState = FDE_XmlSyntaxState::AttriName;

            auto* child = doc->CreateNode<CFX_XMLElement>(GetTextData());
            current_node_->AppendChild(child);
            current_node_ = child;
          } else {
            current_text_.push_back(ch);
            m_Start++;
          }
          break;
        case FDE_XmlSyntaxState::AttriName:
          if (current_text_.empty() && IsXMLWhiteSpace(ch)) {
            m_Start++;
            break;
          }
          if (!IsXMLNameChar(ch, current_text_.empty())) {
            if (current_text_.empty()) {
              if (m_XMLNodeTypeStack.top() == FX_XMLNODE_Element) {
                if (ch == L'>' || ch == L'/') {
                  m_syntaxParserState = FDE_XmlSyntaxState::BreakElement;
                  break;
                }
              } else if (m_XMLNodeTypeStack.top() == FX_XMLNODE_Instruction) {
                if (ch == L'?') {
                  m_syntaxParserState = FDE_XmlSyntaxState::CloseInstruction;
                  m_Start++;
                } else {
                  m_syntaxParserState = FDE_XmlSyntaxState::TargetData;
                }
                break;
              }
              return false;
            } else {
              if (m_XMLNodeTypeStack.top() == FX_XMLNODE_Instruction) {
                if (ch != '=' && !IsXMLWhiteSpace(ch)) {
                  m_syntaxParserState = FDE_XmlSyntaxState::TargetData;
                  break;
                }
              }
              m_syntaxParserState = FDE_XmlSyntaxState::AttriEqualSign;
              current_attribute_name_ = GetTextData();
            }
          } else {
            current_text_.push_back(ch);
            m_Start++;
          }
          break;
        case FDE_XmlSyntaxState::AttriEqualSign:
          if (IsXMLWhiteSpace(ch)) {
            m_Start++;
            break;
          }
          if (ch != L'=') {
            if (m_XMLNodeTypeStack.top() == FX_XMLNODE_Instruction) {
              m_syntaxParserState = FDE_XmlSyntaxState::TargetData;
              break;
            }
            return false;
          } else {
            m_syntaxParserState = FDE_XmlSyntaxState::AttriQuotation;
            m_Start++;
          }
          break;
        case FDE_XmlSyntaxState::AttriQuotation:
          if (IsXMLWhiteSpace(ch)) {
            m_Start++;
            break;
          }
          if (ch != L'\"' && ch != L'\'') {
            return false;
          }

          m_wQuotationMark = ch;
          m_syntaxParserState = FDE_XmlSyntaxState::AttriValue;
          m_Start++;
          break;
        case FDE_XmlSyntaxState::AttriValue:
          if (ch == m_wQuotationMark) {
            if (m_iEntityStart > -1)
              return false;

            m_wQuotationMark = 0;
            m_Start++;
            m_syntaxParserState = FDE_XmlSyntaxState::AttriName;

            if (current_node_ &&
                current_node_->GetType() == FX_XMLNODE_Element) {
              static_cast<CFX_XMLElement*>(current_node_)
                  ->SetAttribute(current_attribute_name_, GetTextData());
            }
            current_attribute_name_.clear();
          } else {
            ProcessTextChar(ch);
          }
          break;
        case FDE_XmlSyntaxState::CloseInstruction:
          if (ch != L'>') {
            current_text_.push_back(ch);
            m_syntaxParserState = FDE_XmlSyntaxState::TargetData;
          } else if (!current_text_.empty()) {
            ProcessTargetData();
          } else {
            m_Start++;
            if (m_XMLNodeTypeStack.empty())
              return false;

            m_XMLNodeTypeStack.pop();
            m_syntaxParserState = FDE_XmlSyntaxState::Text;

            if (current_node_ &&
                current_node_->GetType() == FX_XMLNODE_Instruction)
              current_node_ = current_node_->GetParent();
          }
          break;
        case FDE_XmlSyntaxState::BreakElement:
          if (ch == L'>') {
            m_syntaxParserState = FDE_XmlSyntaxState::Text;
          } else if (ch == L'/') {
            m_syntaxParserState = FDE_XmlSyntaxState::CloseElement;
          } else {
            return false;
          }
          m_Start++;
          break;
        case FDE_XmlSyntaxState::CloseElement:
          if (!IsXMLNameChar(ch, current_text_.empty())) {
            if (ch == L'>') {
              if (m_XMLNodeTypeStack.empty())
                return false;

              m_XMLNodeTypeStack.pop();
              m_syntaxParserState = FDE_XmlSyntaxState::Text;

              if (current_node_->GetType() != FX_XMLNODE_Element)
                return false;

              WideString element_name = GetTextData();
              if (element_name.GetLength() > 0 &&
                  element_name !=
                      static_cast<CFX_XMLElement*>(current_node_)->GetName()) {
                return false;
              }

              current_node_ = current_node_->GetParent();
              iCount++;
            } else if (!IsXMLWhiteSpace(ch)) {
              return false;
            }
          } else {
            current_text_.push_back(ch);
          }
          m_Start++;
          break;
        case FDE_XmlSyntaxState::SkipCommentOrDecl:
          if (FXSYS_wcsnicmp(m_Buffer.data() + m_Start, L"--", 2) == 0) {
            m_Start += 2;
            m_syntaxParserState = FDE_XmlSyntaxState::SkipComment;
          } else if (FXSYS_wcsnicmp(m_Buffer.data() + m_Start, L"[CDATA[", 7) ==
                     0) {
            m_Start += 7;
            m_syntaxParserState = FDE_XmlSyntaxState::SkipCData;
          } else {
            m_syntaxParserState = FDE_XmlSyntaxState::SkipDeclNode;
            m_SkipChar = L'>';
            m_SkipStack.push(L'>');
          }
          break;
        case FDE_XmlSyntaxState::SkipCData: {
          if (FXSYS_wcsnicmp(m_Buffer.data() + m_Start, L"]]>", 3) == 0) {
            m_Start += 3;
            m_syntaxParserState = FDE_XmlSyntaxState::Text;

            current_node_->AppendChild(
                doc->CreateNode<CFX_XMLCharData>(GetTextData()));
          } else {
            current_text_.push_back(ch);
            m_Start++;
          }
          break;
        }
        case FDE_XmlSyntaxState::SkipDeclNode:
          if (m_SkipChar == L'\'' || m_SkipChar == L'\"') {
            m_Start++;
            if (ch != m_SkipChar)
              break;

            m_SkipStack.pop();
            if (m_SkipStack.empty())
              m_syntaxParserState = FDE_XmlSyntaxState::Text;
            else
              m_SkipChar = m_SkipStack.top();
          } else {
            switch (ch) {
              case L'<':
                m_SkipChar = L'>';
                m_SkipStack.push(L'>');
                break;
              case L'[':
                m_SkipChar = L']';
                m_SkipStack.push(L']');
                break;
              case L'(':
                m_SkipChar = L')';
                m_SkipStack.push(L')');
                break;
              case L'\'':
                m_SkipChar = L'\'';
                m_SkipStack.push(L'\'');
                break;
              case L'\"':
                m_SkipChar = L'\"';
                m_SkipStack.push(L'\"');
                break;
              default:
                if (ch == m_SkipChar) {
                  m_SkipStack.pop();
                  if (m_SkipStack.empty()) {
                    m_syntaxParserState = FDE_XmlSyntaxState::Text;
                  } else {
                    m_SkipChar = m_SkipStack.top();
                  }
                }
                break;
            }
            m_Start++;
          }
          break;
        case FDE_XmlSyntaxState::SkipComment:
          if (FXSYS_wcsnicmp(m_Buffer.data() + m_Start, L"-->", 3) == 0) {
            m_Start += 2;
            m_syntaxParserState = FDE_XmlSyntaxState::Text;
          }

          m_Start++;
          break;
        case FDE_XmlSyntaxState::TargetData:
          if (IsXMLWhiteSpace(ch)) {
            if (current_text_.empty()) {
              m_Start++;
              break;
            }
            if (m_wQuotationMark == 0) {
              m_Start++;
              ProcessTargetData();
              break;
            }
          }
          if (ch == '?') {
            m_syntaxParserState = FDE_XmlSyntaxState::CloseInstruction;
            m_Start++;
          } else if (ch == '\"') {
            if (m_wQuotationMark == 0) {
              m_wQuotationMark = ch;
              m_Start++;
            } else if (ch == m_wQuotationMark) {
              m_wQuotationMark = 0;
              m_Start++;
              ProcessTargetData();
            } else {
              return false;
            }
          } else {
            current_text_.push_back(ch);
            m_Start++;
          }
          break;
        default:
          break;
      }
    }
  }

  current_node_->AppendChild(doc->CreateNode<CFX_XMLText>(GetTextData()));
  return true;
}

void CFX_XMLParser::ProcessTextChar(wchar_t character) {
  current_text_.push_back(character);

  if (m_iEntityStart > -1 && character == L';') {
    // Copy the entity out into a string and remove from the vector. When we
    // copy the entity we don't want to copy out the & or the ; so we start
    // shifted by one and want to copy 2 less characters in total.
    WideString csEntity(current_text_.data() + m_iEntityStart + 1,
                        current_text_.size() - m_iEntityStart - 2);
    current_text_.erase(current_text_.begin() + m_iEntityStart,
                        current_text_.end());

    int32_t iLen = csEntity.GetLength();
    if (iLen > 0) {
      if (csEntity[0] == L'#') {
        uint32_t ch = 0;
        if (iLen > 1 && csEntity[1] == L'x') {
          for (int32_t i = 2; i < iLen; i++) {
            if (!FXSYS_isHexDigit(csEntity[i]))
              break;
            ch = (ch << 4) + FXSYS_HexCharToInt(csEntity[i]);
          }
        } else {
          for (int32_t i = 1; i < iLen; i++) {
            if (!FXSYS_isDecimalDigit(csEntity[i]))
              break;
            ch = ch * 10 + FXSYS_DecimalCharToInt(csEntity[i]);
          }
        }
        if (ch > kMaxCharRange)
          ch = ' ';

        character = static_cast<wchar_t>(ch);
        if (character != 0)
          current_text_.push_back(character);
      } else {
        if (csEntity.Compare(L"amp") == 0) {
          current_text_.push_back(L'&');
        } else if (csEntity.Compare(L"lt") == 0) {
          current_text_.push_back(L'<');
        } else if (csEntity.Compare(L"gt") == 0) {
          current_text_.push_back(L'>');
        } else if (csEntity.Compare(L"apos") == 0) {
          current_text_.push_back(L'\'');
        } else if (csEntity.Compare(L"quot") == 0) {
          current_text_.push_back(L'"');
        }
      }
    }

    m_iEntityStart = -1;
  } else if (m_iEntityStart < 0 && character == L'&') {
    m_iEntityStart = current_text_.size() - 1;
  }
  m_Start++;
}

void CFX_XMLParser::ProcessTargetData() {
  WideString target_data = GetTextData();
  if (target_data.IsEmpty())
    return;
  if (!current_node_)
    return;
  if (current_node_->GetType() != FX_XMLNODE_Instruction)
    return;

  static_cast<CFX_XMLInstruction*>(current_node_)->AppendData(target_data);
}

WideString CFX_XMLParser::GetTextData() {
  WideString ret(current_text_.data(), current_text_.size());
  m_iEntityStart = -1;
  current_text_.clear();
  current_text_.reserve(kCurrentTextReserve);
  return ret;
}
