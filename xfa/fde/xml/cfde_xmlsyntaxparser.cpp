// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/xml/cfde_xmlsyntaxparser.h"

#include <algorithm>

#include "core/fxcrt/fx_ext.h"
#include "core/fxcrt/fx_safe_types.h"

namespace {

const uint32_t kMaxCharRange = 0x10ffff;

bool IsXMLWhiteSpace(wchar_t ch) {
  return ch == L' ' || ch == 0x0A || ch == 0x0D || ch == 0x09;
}

struct FDE_XMLNAMECHAR {
  uint16_t wStart;
  uint16_t wEnd;
  bool bStartChar;
};

const FDE_XMLNAMECHAR g_XMLNameChars[] = {
    {L'-', L'.', false},    {L'0', L'9', false},     {L':', L':', false},
    {L'A', L'Z', true},     {L'_', L'_', true},      {L'a', L'z', true},
    {0xB7, 0xB7, false},    {0xC0, 0xD6, true},      {0xD8, 0xF6, true},
    {0xF8, 0x02FF, true},   {0x0300, 0x036F, false}, {0x0370, 0x037D, true},
    {0x037F, 0x1FFF, true}, {0x200C, 0x200D, true},  {0x203F, 0x2040, false},
    {0x2070, 0x218F, true}, {0x2C00, 0x2FEF, true},  {0x3001, 0xD7FF, true},
    {0xF900, 0xFDCF, true}, {0xFDF0, 0xFFFD, true},
};

bool IsXMLNameChar(wchar_t ch, bool bFirstChar) {
  int32_t iStart = 0;
  int32_t iEnd = FX_ArraySize(g_XMLNameChars) - 1;
  while (iStart <= iEnd) {
    int32_t iMid = (iStart + iEnd) / 2;
    if (ch < g_XMLNameChars[iMid].wStart) {
      iEnd = iMid - 1;
    } else if (ch > g_XMLNameChars[iMid].wEnd) {
      iStart = iMid + 1;
    } else {
      return bFirstChar ? g_XMLNameChars[iMid].bStartChar : true;
    }
  }
  return false;
}

int32_t GetUTF8EncodeLength(const std::vector<wchar_t>& src,
                            FX_FILESIZE iSrcLen) {
  uint32_t unicode = 0;
  int32_t iDstNum = 0;
  const wchar_t* pSrc = src.data();
  while (iSrcLen-- > 0) {
    unicode = *pSrc++;
    int nbytes = 0;
    if ((uint32_t)unicode < 0x80) {
      nbytes = 1;
    } else if ((uint32_t)unicode < 0x800) {
      nbytes = 2;
    } else if ((uint32_t)unicode < 0x10000) {
      nbytes = 3;
    } else if ((uint32_t)unicode < 0x200000) {
      nbytes = 4;
    } else if ((uint32_t)unicode < 0x4000000) {
      nbytes = 5;
    } else {
      nbytes = 6;
    }
    iDstNum += nbytes;
  }
  return iDstNum;
}

}  // namespace

CFDE_XMLSyntaxParser::CFDE_XMLSyntaxParser(
    const CFX_RetainPtr<IFGAS_Stream>& pStream)
    : m_pStream(pStream),
      m_iXMLPlaneSize(32 * 1024),
      m_iCurrentPos(0),
      m_iCurrentNodeNum(-1),
      m_iLastNodeNum(-1),
      m_iParsedBytes(0),
      m_ParsedChars(0),
      m_iBufferChars(0),
      m_bEOS(false),
      m_Start(0),
      m_End(0),
      m_iAllocStep(m_BlockBuffer.GetAllocStep()),
      m_pCurrentBlock(nullptr),
      m_iIndexInBlock(0),
      m_iTextDataLength(0),
      m_syntaxParserResult(FDE_XmlSyntaxResult::None),
      m_syntaxParserState(FDE_XmlSyntaxState::Text),
      m_wQuotationMark(0),
      m_iEntityStart(-1) {
  ASSERT(pStream);

  m_CurNode.iNodeNum = -1;
  m_CurNode.eNodeType = FDE_XMLNODE_Unknown;

  m_iXMLPlaneSize = std::min(m_iXMLPlaneSize, m_pStream->GetLength());
  m_iCurrentPos = m_pStream->GetBOMLength();

  FX_SAFE_INT32 alloc_size_safe = m_iXMLPlaneSize;
  alloc_size_safe += 1;  // For NUL.
  if (!alloc_size_safe.IsValid() || alloc_size_safe.ValueOrDie() <= 0) {
    m_syntaxParserResult = FDE_XmlSyntaxResult::Error;
    return;
  }

  m_Buffer.resize(pdfium::base::ValueOrDieForType<size_t>(alloc_size_safe));

  m_BlockBuffer.InitBuffer();
  std::tie(m_pCurrentBlock, m_iIndexInBlock) =
      m_BlockBuffer.GetAvailableBlock();
}

CFDE_XMLSyntaxParser::~CFDE_XMLSyntaxParser() {}

FDE_XmlSyntaxResult CFDE_XMLSyntaxParser::DoSyntaxParse() {
  if (m_syntaxParserResult == FDE_XmlSyntaxResult::Error ||
      m_syntaxParserResult == FDE_XmlSyntaxResult::EndOfString) {
    return m_syntaxParserResult;
  }

  int32_t iStreamLength = m_pStream->GetLength();
  int32_t iPos;

  FDE_XmlSyntaxResult syntaxParserResult = FDE_XmlSyntaxResult::None;
  while (true) {
    if (m_Start >= m_End) {
      if (m_bEOS || m_iCurrentPos >= iStreamLength) {
        m_syntaxParserResult = FDE_XmlSyntaxResult::EndOfString;
        return m_syntaxParserResult;
      }
      m_ParsedChars += m_End;
      m_iParsedBytes = m_iCurrentPos;
      if (m_pStream->GetPosition() != m_iCurrentPos) {
        m_pStream->Seek(FX_STREAMSEEK_Begin, m_iCurrentPos);
      }
      m_iBufferChars =
          m_pStream->ReadString(m_Buffer.data(), m_iXMLPlaneSize, m_bEOS);
      iPos = m_pStream->GetPosition();
      if (m_iBufferChars < 1) {
        m_iCurrentPos = iStreamLength;
        m_syntaxParserResult = FDE_XmlSyntaxResult::EndOfString;
        return m_syntaxParserResult;
      }
      m_iCurrentPos = iPos;
      m_Start = 0;
      m_End = m_iBufferChars;
    }

    while (m_Start < m_End) {
      wchar_t ch = m_Buffer[m_Start];
      switch (m_syntaxParserState) {
        case FDE_XmlSyntaxState::Text:
          if (ch == L'<') {
            if (!m_BlockBuffer.IsEmpty()) {
              m_iTextDataLength = m_BlockBuffer.GetDataLength();
              m_BlockBuffer.Reset(true);
              std::tie(m_pCurrentBlock, m_iIndexInBlock) =
                  m_BlockBuffer.GetAvailableBlock();
              m_iEntityStart = -1;
              syntaxParserResult = FDE_XmlSyntaxResult::Text;
            } else {
              m_Start++;
              m_syntaxParserState = FDE_XmlSyntaxState::Node;
            }
          } else {
            ParseTextChar(ch);
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
            m_iLastNodeNum++;
            m_iCurrentNodeNum = m_iLastNodeNum;
            m_CurNode.iNodeNum = m_iLastNodeNum;
            m_CurNode.eNodeType = FDE_XMLNODE_Instruction;
            m_XMLNodeStack.push(m_CurNode);
            m_Start++;
            m_syntaxParserState = FDE_XmlSyntaxState::Target;
            syntaxParserResult = FDE_XmlSyntaxResult::InstructionOpen;
          } else {
            m_iLastNodeNum++;
            m_iCurrentNodeNum = m_iLastNodeNum;
            m_CurNode.iNodeNum = m_iLastNodeNum;
            m_CurNode.eNodeType = FDE_XMLNODE_Element;
            m_XMLNodeStack.push(m_CurNode);
            m_syntaxParserState = FDE_XmlSyntaxState::Tag;
            syntaxParserResult = FDE_XmlSyntaxResult::ElementOpen;
          }
          break;
        case FDE_XmlSyntaxState::Target:
        case FDE_XmlSyntaxState::Tag:
          if (!IsXMLNameChar(ch, m_BlockBuffer.IsEmpty())) {
            if (m_BlockBuffer.IsEmpty()) {
              m_syntaxParserResult = FDE_XmlSyntaxResult::Error;
              return m_syntaxParserResult;
            }

            m_iTextDataLength = m_BlockBuffer.GetDataLength();
            m_BlockBuffer.Reset(true);
            std::tie(m_pCurrentBlock, m_iIndexInBlock) =
                m_BlockBuffer.GetAvailableBlock();
            if (m_syntaxParserState != FDE_XmlSyntaxState::Target)
              syntaxParserResult = FDE_XmlSyntaxResult::TagName;
            else
              syntaxParserResult = FDE_XmlSyntaxResult::TargetName;

            m_syntaxParserState = FDE_XmlSyntaxState::AttriName;
          } else {
            if (m_iIndexInBlock == m_iAllocStep) {
              std::tie(m_pCurrentBlock, m_iIndexInBlock) =
                  m_BlockBuffer.GetAvailableBlock();
              if (!m_pCurrentBlock) {
                return FDE_XmlSyntaxResult::Error;
              }
            }
            m_pCurrentBlock[m_iIndexInBlock++] = ch;
            m_BlockBuffer.IncrementDataLength();
            m_Start++;
          }
          break;
        case FDE_XmlSyntaxState::AttriName:
          if (m_BlockBuffer.IsEmpty() && IsXMLWhiteSpace(ch)) {
            m_Start++;
            break;
          }
          if (!IsXMLNameChar(ch, m_BlockBuffer.IsEmpty())) {
            if (m_BlockBuffer.IsEmpty()) {
              if (m_CurNode.eNodeType == FDE_XMLNODE_Element) {
                if (ch == L'>' || ch == L'/') {
                  m_syntaxParserState = FDE_XmlSyntaxState::BreakElement;
                  break;
                }
              } else if (m_CurNode.eNodeType == FDE_XMLNODE_Instruction) {
                if (ch == L'?') {
                  m_syntaxParserState = FDE_XmlSyntaxState::CloseInstruction;
                  m_Start++;
                } else {
                  m_syntaxParserState = FDE_XmlSyntaxState::TargetData;
                }
                break;
              }
              m_syntaxParserResult = FDE_XmlSyntaxResult::Error;
              return m_syntaxParserResult;
            } else {
              if (m_CurNode.eNodeType == FDE_XMLNODE_Instruction) {
                if (ch != '=' && !IsXMLWhiteSpace(ch)) {
                  m_syntaxParserState = FDE_XmlSyntaxState::TargetData;
                  break;
                }
              }
              m_iTextDataLength = m_BlockBuffer.GetDataLength();
              m_BlockBuffer.Reset(true);
              std::tie(m_pCurrentBlock, m_iIndexInBlock) =
                  m_BlockBuffer.GetAvailableBlock();
              m_syntaxParserState = FDE_XmlSyntaxState::AttriEqualSign;
              syntaxParserResult = FDE_XmlSyntaxResult::AttriName;
            }
          } else {
            if (m_iIndexInBlock == m_iAllocStep) {
              std::tie(m_pCurrentBlock, m_iIndexInBlock) =
                  m_BlockBuffer.GetAvailableBlock();
              if (!m_pCurrentBlock) {
                return FDE_XmlSyntaxResult::Error;
              }
            }
            m_pCurrentBlock[m_iIndexInBlock++] = ch;
            m_BlockBuffer.IncrementDataLength();
            m_Start++;
          }
          break;
        case FDE_XmlSyntaxState::AttriEqualSign:
          if (IsXMLWhiteSpace(ch)) {
            m_Start++;
            break;
          }
          if (ch != L'=') {
            if (m_CurNode.eNodeType == FDE_XMLNODE_Instruction) {
              m_syntaxParserState = FDE_XmlSyntaxState::TargetData;
              break;
            }
            m_syntaxParserResult = FDE_XmlSyntaxResult::Error;
            return m_syntaxParserResult;
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
            m_syntaxParserResult = FDE_XmlSyntaxResult::Error;
            return m_syntaxParserResult;
          } else {
            m_wQuotationMark = ch;
            m_syntaxParserState = FDE_XmlSyntaxState::AttriValue;
            m_Start++;
          }
          break;
        case FDE_XmlSyntaxState::AttriValue:
          if (ch == m_wQuotationMark) {
            if (m_iEntityStart > -1) {
              m_syntaxParserResult = FDE_XmlSyntaxResult::Error;
              return m_syntaxParserResult;
            }
            m_iTextDataLength = m_BlockBuffer.GetDataLength();
            m_wQuotationMark = 0;
            m_BlockBuffer.Reset(true);
            std::tie(m_pCurrentBlock, m_iIndexInBlock) =
                m_BlockBuffer.GetAvailableBlock();
            m_Start++;
            m_syntaxParserState = FDE_XmlSyntaxState::AttriName;
            syntaxParserResult = FDE_XmlSyntaxResult::AttriValue;
          } else {
            ParseTextChar(ch);
          }
          break;
        case FDE_XmlSyntaxState::CloseInstruction:
          if (ch != L'>') {
            if (m_iIndexInBlock == m_iAllocStep) {
              std::tie(m_pCurrentBlock, m_iIndexInBlock) =
                  m_BlockBuffer.GetAvailableBlock();
              if (!m_pCurrentBlock) {
                return FDE_XmlSyntaxResult::Error;
              }
            }
            m_pCurrentBlock[m_iIndexInBlock++] = ch;
            m_BlockBuffer.IncrementDataLength();
            m_syntaxParserState = FDE_XmlSyntaxState::TargetData;
          } else if (!m_BlockBuffer.IsEmpty()) {
            m_iTextDataLength = m_BlockBuffer.GetDataLength();
            m_BlockBuffer.Reset(true);
            std::tie(m_pCurrentBlock, m_iIndexInBlock) =
                m_BlockBuffer.GetAvailableBlock();
            syntaxParserResult = FDE_XmlSyntaxResult::TargetData;
          } else {
            m_Start++;
            if (m_XMLNodeStack.empty()) {
              m_syntaxParserResult = FDE_XmlSyntaxResult::Error;
              return m_syntaxParserResult;
            }
            m_XMLNodeStack.pop();
            if (!m_XMLNodeStack.empty()) {
              m_CurNode = m_XMLNodeStack.top();
            } else {
              m_CurNode.iNodeNum = -1;
              m_CurNode.eNodeType = FDE_XMLNODE_Unknown;
            }
            m_iCurrentNodeNum = m_CurNode.iNodeNum;
            m_BlockBuffer.Reset(true);
            std::tie(m_pCurrentBlock, m_iIndexInBlock) =
                m_BlockBuffer.GetAvailableBlock();
            m_syntaxParserState = FDE_XmlSyntaxState::Text;
            syntaxParserResult = FDE_XmlSyntaxResult::InstructionClose;
          }
          break;
        case FDE_XmlSyntaxState::BreakElement:
          if (ch == L'>') {
            m_syntaxParserState = FDE_XmlSyntaxState::Text;
            syntaxParserResult = FDE_XmlSyntaxResult::ElementBreak;
          } else if (ch == L'/') {
            m_syntaxParserState = FDE_XmlSyntaxState::CloseElement;
          } else {
            m_syntaxParserResult = FDE_XmlSyntaxResult::Error;
            return m_syntaxParserResult;
          }
          m_Start++;
          break;
        case FDE_XmlSyntaxState::CloseElement:
          if (!IsXMLNameChar(ch, m_BlockBuffer.IsEmpty())) {
            if (ch == L'>') {
              if (m_XMLNodeStack.empty()) {
                m_syntaxParserResult = FDE_XmlSyntaxResult::Error;
                return m_syntaxParserResult;
              }
              m_XMLNodeStack.pop();
              if (!m_XMLNodeStack.empty()) {
                m_CurNode = m_XMLNodeStack.top();
              } else {
                m_CurNode.iNodeNum = -1;
                m_CurNode.eNodeType = FDE_XMLNODE_Unknown;
              }
              m_iCurrentNodeNum = m_CurNode.iNodeNum;
              m_iTextDataLength = m_BlockBuffer.GetDataLength();
              m_BlockBuffer.Reset(true);
              std::tie(m_pCurrentBlock, m_iIndexInBlock) =
                  m_BlockBuffer.GetAvailableBlock();
              m_syntaxParserState = FDE_XmlSyntaxState::Text;
              syntaxParserResult = FDE_XmlSyntaxResult::ElementClose;
            } else if (!IsXMLWhiteSpace(ch)) {
              m_syntaxParserResult = FDE_XmlSyntaxResult::Error;
              return m_syntaxParserResult;
            }
          } else {
            if (m_iIndexInBlock == m_iAllocStep) {
              std::tie(m_pCurrentBlock, m_iIndexInBlock) =
                  m_BlockBuffer.GetAvailableBlock();
              if (!m_pCurrentBlock) {
                return FDE_XmlSyntaxResult::Error;
              }
            }
            m_pCurrentBlock[m_iIndexInBlock++] = ch;
            m_BlockBuffer.IncrementDataLength();
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
            syntaxParserResult = FDE_XmlSyntaxResult::CData;
            m_iTextDataLength = m_BlockBuffer.GetDataLength();
            m_BlockBuffer.Reset(true);
            std::tie(m_pCurrentBlock, m_iIndexInBlock) =
                m_BlockBuffer.GetAvailableBlock();
            m_syntaxParserState = FDE_XmlSyntaxState::Text;
          } else {
            if (m_iIndexInBlock == m_iAllocStep) {
              std::tie(m_pCurrentBlock, m_iIndexInBlock) =
                  m_BlockBuffer.GetAvailableBlock();
              if (!m_pCurrentBlock)
                return FDE_XmlSyntaxResult::Error;
            }
            m_pCurrentBlock[m_iIndexInBlock++] = ch;
            m_BlockBuffer.IncrementDataLength();
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
                    if (m_BlockBuffer.GetDataLength() >= 9)
                      (void)m_BlockBuffer.GetTextData(0, 7);

                    m_iTextDataLength = m_BlockBuffer.GetDataLength();
                    m_BlockBuffer.Reset(true);
                    std::tie(m_pCurrentBlock, m_iIndexInBlock) =
                        m_BlockBuffer.GetAvailableBlock();
                    m_syntaxParserState = FDE_XmlSyntaxState::Text;
                  } else {
                    m_SkipChar = m_SkipStack.top();
                  }
                }
                break;
            }
            if (!m_SkipStack.empty()) {
              if (m_iIndexInBlock == m_iAllocStep) {
                std::tie(m_pCurrentBlock, m_iIndexInBlock) =
                    m_BlockBuffer.GetAvailableBlock();
                if (!m_pCurrentBlock) {
                  return FDE_XmlSyntaxResult::Error;
                }
              }
              m_pCurrentBlock[m_iIndexInBlock++] = ch;
              m_BlockBuffer.IncrementDataLength();
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
            if (m_BlockBuffer.IsEmpty()) {
              m_Start++;
              break;
            } else if (m_wQuotationMark == 0) {
              m_iTextDataLength = m_BlockBuffer.GetDataLength();
              m_wQuotationMark = 0;
              m_BlockBuffer.Reset(true);
              std::tie(m_pCurrentBlock, m_iIndexInBlock) =
                  m_BlockBuffer.GetAvailableBlock();
              m_Start++;
              syntaxParserResult = FDE_XmlSyntaxResult::TargetData;
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
              m_iTextDataLength = m_BlockBuffer.GetDataLength();
              m_wQuotationMark = 0;
              m_BlockBuffer.Reset(true);
              std::tie(m_pCurrentBlock, m_iIndexInBlock) =
                  m_BlockBuffer.GetAvailableBlock();
              m_Start++;
              syntaxParserResult = FDE_XmlSyntaxResult::TargetData;
            } else {
              m_syntaxParserResult = FDE_XmlSyntaxResult::Error;
              return m_syntaxParserResult;
            }
          } else {
            if (m_iIndexInBlock == m_iAllocStep) {
              std::tie(m_pCurrentBlock, m_iIndexInBlock) =
                  m_BlockBuffer.GetAvailableBlock();
              if (!m_pCurrentBlock) {
                return FDE_XmlSyntaxResult::Error;
              }
            }
            m_pCurrentBlock[m_iIndexInBlock++] = ch;
            m_BlockBuffer.IncrementDataLength();
            m_Start++;
          }
          break;
        default:
          break;
      }
      if (syntaxParserResult != FDE_XmlSyntaxResult::None)
        return syntaxParserResult;
    }
  }
  return FDE_XmlSyntaxResult::Text;
}

int32_t CFDE_XMLSyntaxParser::GetStatus() const {
  if (!m_pStream)
    return -1;

  int32_t iStreamLength = m_pStream->GetLength();
  if (iStreamLength < 1)
    return 100;

  if (m_syntaxParserResult == FDE_XmlSyntaxResult::Error)
    return -1;

  if (m_syntaxParserResult == FDE_XmlSyntaxResult::EndOfString)
    return 100;
  return m_iParsedBytes * 100 / iStreamLength;
}

FX_FILESIZE CFDE_XMLSyntaxParser::GetCurrentBinaryPos() const {
  if (!m_pStream)
    return 0;

  int32_t nDstLen = GetUTF8EncodeLength(m_Buffer, m_Start);
  return m_iParsedBytes + nDstLen;
}

void CFDE_XMLSyntaxParser::ParseTextChar(wchar_t character) {
  if (m_iIndexInBlock == m_iAllocStep) {
    std::tie(m_pCurrentBlock, m_iIndexInBlock) =
        m_BlockBuffer.GetAvailableBlock();
    if (!m_pCurrentBlock)
      return;
  }

  m_pCurrentBlock[m_iIndexInBlock++] = character;
  m_BlockBuffer.IncrementDataLength();
  if (m_iEntityStart > -1 && character == L';') {
    CFX_WideString csEntity = m_BlockBuffer.GetTextData(
        m_iEntityStart + 1,
        m_BlockBuffer.GetDataLength() - 1 - m_iEntityStart - 1);
    int32_t iLen = csEntity.GetLength();
    if (iLen > 0) {
      if (csEntity[0] == L'#') {
        uint32_t ch = 0;
        wchar_t w;
        if (iLen > 1 && csEntity[1] == L'x') {
          for (int32_t i = 2; i < iLen; i++) {
            w = csEntity[i];
            if (w >= L'0' && w <= L'9') {
              ch = (ch << 4) + w - L'0';
            } else if (w >= L'A' && w <= L'F') {
              ch = (ch << 4) + w - 55;
            } else if (w >= L'a' && w <= L'f') {
              ch = (ch << 4) + w - 87;
            } else {
              break;
            }
          }
        } else {
          for (int32_t i = 1; i < iLen; i++) {
            w = csEntity[i];
            if (w < L'0' || w > L'9')
              break;
            ch = ch * 10 + w - L'0';
          }
        }
        if (ch > kMaxCharRange)
          ch = ' ';

        character = static_cast<wchar_t>(ch);
        if (character != 0) {
          m_BlockBuffer.SetTextChar(m_iEntityStart, character);
          m_iEntityStart++;
        }
      } else {
        if (csEntity.Compare(L"amp") == 0) {
          m_BlockBuffer.SetTextChar(m_iEntityStart, L'&');
          m_iEntityStart++;
        } else if (csEntity.Compare(L"lt") == 0) {
          m_BlockBuffer.SetTextChar(m_iEntityStart, L'<');
          m_iEntityStart++;
        } else if (csEntity.Compare(L"gt") == 0) {
          m_BlockBuffer.SetTextChar(m_iEntityStart, L'>');
          m_iEntityStart++;
        } else if (csEntity.Compare(L"apos") == 0) {
          m_BlockBuffer.SetTextChar(m_iEntityStart, L'\'');
          m_iEntityStart++;
        } else if (csEntity.Compare(L"quot") == 0) {
          m_BlockBuffer.SetTextChar(m_iEntityStart, L'\"');
          m_iEntityStart++;
        }
      }
    }
    if (m_iEntityStart >= 0 &&
        m_BlockBuffer.GetDataLength() > static_cast<size_t>(m_iEntityStart)) {
      m_BlockBuffer.DeleteTextChars(m_BlockBuffer.GetDataLength() -
                                    m_iEntityStart);
    }
    std::tie(m_pCurrentBlock, m_iIndexInBlock) =
        m_BlockBuffer.GetAvailableBlock();
    m_iEntityStart = -1;
  } else if (m_iEntityStart < 0 && character == L'&') {
    m_iEntityStart = m_BlockBuffer.GetDataLength() - 1;
  }
  m_Start++;
}
