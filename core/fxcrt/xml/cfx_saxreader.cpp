// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/xml/cfx_saxreader.h"

#include <algorithm>
#include <utility>

#include "core/fxcrt/fx_stream.h"
#include "core/fxcrt/xml/cfx_saxreaderhandler.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"

enum class CFX_SaxMode {
  Text = 0,
  NodeStart,
  DeclOrComment,
  DeclNode,
  Comment,
  CommentContent,
  TagName,
  TagAttributeName,
  TagAttributeEqual,
  TagAttributeValue,
  TagMaybeClose,
  TagClose,
  TagEnd,
  TargetData,
};

class CFX_SAXCommentContext {
 public:
  CFX_SAXCommentContext() : m_iHeaderCount(0), m_iTailCount(0) {}
  int32_t m_iHeaderCount;
  int32_t m_iTailCount;
};

namespace {

const uint32_t kSaxFileBufSize = 32768;

}  // namespace

CFX_SAXFile::CFX_SAXFile()
    : m_dwStart(0),
      m_dwEnd(0),
      m_dwCur(0),
      m_pBuf(nullptr),
      m_dwBufSize(0),
      m_dwBufIndex(0) {}

CFX_SAXFile::~CFX_SAXFile() {}

bool CFX_SAXFile::StartFile(const RetainPtr<IFX_SeekableReadStream>& pFile,
                            uint32_t dwStart,
                            uint32_t dwLen) {
  ASSERT(!m_pFile && pFile);
  uint32_t dwSize = pFile->GetSize();
  if (dwStart >= dwSize)
    return false;

  if (dwLen == static_cast<uint32_t>(-1) || dwStart + dwLen > dwSize)
    dwLen = dwSize - dwStart;

  if (dwLen == 0)
    return false;

  m_dwBufSize = std::min(dwLen, kSaxFileBufSize);
  m_pBuf = FX_Alloc(uint8_t, m_dwBufSize);
  if (!pFile->ReadBlock(m_pBuf, dwStart, m_dwBufSize))
    return false;

  m_dwStart = dwStart;
  m_dwEnd = dwStart + dwLen;
  m_dwCur = dwStart;
  m_pFile = pFile;
  m_dwBufIndex = 0;
  return true;
}

bool CFX_SAXFile::ReadNextBlock() {
  ASSERT(m_pFile);
  uint32_t dwSize = m_dwEnd - m_dwCur;
  if (dwSize == 0) {
    return false;
  }
  m_dwBufSize = std::min(dwSize, kSaxFileBufSize);
  if (!m_pFile->ReadBlock(m_pBuf, m_dwCur, m_dwBufSize)) {
    return false;
  }
  m_dwBufIndex = 0;
  return true;
}

void CFX_SAXFile::Reset() {
  if (m_pBuf) {
    FX_Free(m_pBuf);
    m_pBuf = nullptr;
  }
  m_pFile = nullptr;
}

CFX_SAXReader::CFX_SAXReader()
    : m_File(),
      m_pHandler(nullptr),
      m_iState(-1),
      m_dwItemID(0),
      m_dwParseMode(0) {
  m_Data.reserve(256);
  m_Name.reserve(256);
}

CFX_SAXReader::~CFX_SAXReader() {
  Reset();
}

void CFX_SAXReader::Reset() {
  m_File.Reset();
  m_iState = -1;
  m_Stack = std::stack<std::unique_ptr<CFX_SAXItem>>();
  m_dwItemID = 0;
  m_SkipStack = std::stack<char>();
  m_SkipChar = 0;
  m_pCommentContext.reset();
  ClearData();
  ClearName();
}

void CFX_SAXReader::Push() {
  std::unique_ptr<CFX_SAXItem> pNew =
      pdfium::MakeUnique<CFX_SAXItem>(++m_dwItemID);
  if (!m_Stack.empty())
    pNew->m_bSkip = m_Stack.top()->m_bSkip;
  m_Stack.push(std::move(pNew));
}

void CFX_SAXReader::Pop() {
  if (!m_Stack.empty())
    m_Stack.pop();
}

CFX_SAXItem* CFX_SAXReader::GetCurrentItem() const {
  return m_Stack.empty() ? nullptr : m_Stack.top().get();
}

void CFX_SAXReader::ClearData() {
  m_Data.clear();
  m_iEntityStart = -1;
}

void CFX_SAXReader::ClearName() {
  m_Name.clear();
}

void CFX_SAXReader::AppendToData(uint8_t ch) {
  m_Data.push_back(ch);
}

void CFX_SAXReader::AppendToName(uint8_t ch) {
  m_Name.push_back(ch);
}

void CFX_SAXReader::BackUpAndReplaceDataAt(int32_t index, uint8_t ch) {
  ASSERT(index > -1);
  m_Data.erase(m_Data.begin() + index, m_Data.end());
  AppendToData(ch);
}

int32_t CFX_SAXReader::CurrentDataIndex() const {
  return pdfium::CollectionSize<int32_t>(m_Data) - 1;
}

bool CFX_SAXReader::IsEntityStart(uint8_t ch) const {
  return m_iEntityStart == -1 && ch == '&';
}

bool CFX_SAXReader::IsEntityEnd(uint8_t ch) const {
  return m_iEntityStart != -1 && ch == ';';
}

bool CFX_SAXReader::SkipSpace(uint8_t ch) {
  return (m_dwParseMode & CFX_SaxParseMode_NotSkipSpace) == 0 && ch < 0x21;
}

int32_t CFX_SAXReader::StartParse(
    const RetainPtr<IFX_SeekableReadStream>& pFile,
    uint32_t dwStart,
    uint32_t dwLen,
    uint32_t dwParseMode) {
  Reset();
  if (!m_File.StartFile(pFile, dwStart, dwLen))
    return -1;

  m_iState = 0;
  m_eMode = CFX_SaxMode::Text;
  m_ePrevMode = CFX_SaxMode::Text;
  m_bCharData = false;
  m_dwDataOffset = 0;
  m_dwParseMode = dwParseMode;
  m_Stack.push(pdfium::MakeUnique<CFX_SAXItem>(++m_dwItemID));
  return 0;
}

int32_t CFX_SAXReader::ContinueParse() {
  if (m_iState < 0 || m_iState > 99)
    return m_iState;

  while (m_File.m_dwCur < m_File.m_dwEnd) {
    uint32_t& index = m_File.m_dwBufIndex;
    uint32_t size = m_File.m_dwBufSize;
    const uint8_t* pBuf = m_File.m_pBuf;
    while (index < size) {
      m_CurByte = pBuf[index];
      ParseInternal();
      index++;
    }
    m_File.m_dwCur += index;
    m_iState = (m_File.m_dwCur - m_File.m_dwStart) * 100 /
               (m_File.m_dwEnd - m_File.m_dwStart);
    if (m_File.m_dwCur >= m_File.m_dwEnd)
      break;
    if (!m_File.ReadNextBlock()) {
      m_iState = -2;
      break;
    }
    m_dwDataOffset = 0;
  }
  return m_iState;
}

void CFX_SAXReader::ParseInternal() {
  switch (m_eMode) {
    case CFX_SaxMode::Text:
      ParseText();
      break;
    case CFX_SaxMode::NodeStart:
      ParseNodeStart();
      break;
    case CFX_SaxMode::DeclOrComment:
      ParseDeclOrComment();
      break;
    case CFX_SaxMode::DeclNode:
      ParseDeclNode();
      break;
    case CFX_SaxMode::Comment:
      ParseComment();
      break;
    case CFX_SaxMode::CommentContent:
      ParseCommentContent();
      break;
    case CFX_SaxMode::TagName:
      ParseTagName();
      break;
    case CFX_SaxMode::TagAttributeName:
      ParseTagAttributeName();
      break;
    case CFX_SaxMode::TagAttributeEqual:
      ParseTagAttributeEqual();
      break;
    case CFX_SaxMode::TagAttributeValue:
      ParseTagAttributeValue();
      break;
    case CFX_SaxMode::TagMaybeClose:
      ParseMaybeClose();
      break;
    case CFX_SaxMode::TagClose:
      ParseTagClose();
      break;
    case CFX_SaxMode::TagEnd:
      ParseTagEnd();
      break;
    case CFX_SaxMode::TargetData:
      ParseTargetData();
      break;
  }
}

void CFX_SAXReader::ParseChar(uint8_t ch) {
  AppendToData(ch);
  if (IsEntityStart(ch)) {
    m_iEntityStart = CurrentDataIndex();
    return;
  }
  if (!IsEntityEnd(ch))
    return;

  // No matter what, we're no longer in an entity.
  ASSERT(m_iEntityStart > -1);
  int32_t iSaveStart = m_iEntityStart;
  m_iEntityStart = -1;

  // NOTE: Relies on negative lengths being treated as empty strings.
  ByteString csEntity(m_Data.data() + iSaveStart + 1,
                      CurrentDataIndex() - iSaveStart - 1);
  int32_t iLen = csEntity.GetLength();
  if (iLen == 0)
    return;

  if (csEntity[0] == '#') {
    if ((m_dwParseMode & CFX_SaxParseMode_NotConvert_sharp) == 0) {
      ch = 0;
      uint8_t w;
      if (iLen > 1 && csEntity[1] == 'x') {
        for (int32_t i = 2; i < iLen; i++) {
          w = csEntity[i];
          if (w >= '0' && w <= '9')
            ch = (ch << 4) + w - '0';
          else if (w >= 'A' && w <= 'F')
            ch = (ch << 4) + w - 55;
          else if (w >= 'a' && w <= 'f')
            ch = (ch << 4) + w - 87;
          else
            break;
        }
      } else {
        for (int32_t i = 1; i < iLen; i++) {
          w = csEntity[i];
          if (w < '0' || w > '9')
            break;
          ch = ch * 10 + w - '0';
        }
      }
      if (ch != 0)
        BackUpAndReplaceDataAt(iSaveStart, ch);
    }
    return;
  }
  if (csEntity == "amp") {
    if ((m_dwParseMode & CFX_SaxParseMode_NotConvert_amp) == 0)
      BackUpAndReplaceDataAt(iSaveStart, '&');
    return;
  }
  if (csEntity == "lt") {
    if ((m_dwParseMode & CFX_SaxParseMode_NotConvert_lt) == 0)
      BackUpAndReplaceDataAt(iSaveStart, '<');
    return;
  }
  if (csEntity == "gt") {
    if ((m_dwParseMode & CFX_SaxParseMode_NotConvert_gt) == 0)
      BackUpAndReplaceDataAt(iSaveStart, '>');
    return;
  }
  if (csEntity == "apos") {
    if ((m_dwParseMode & CFX_SaxParseMode_NotConvert_apos) == 0)
      BackUpAndReplaceDataAt(iSaveStart, '\'');
    return;
  }
  if (csEntity == "quot") {
    if ((m_dwParseMode & CFX_SaxParseMode_NotConvert_quot) == 0)
      BackUpAndReplaceDataAt(iSaveStart, '\"');
    return;
  }
}

void CFX_SAXReader::ParseText() {
  if (m_CurByte == '<') {
    if (!m_Data.empty()) {
      NotifyData();
      ClearData();
    }
    Push();
    m_dwNodePos = m_File.m_dwCur + m_File.m_dwBufIndex;
    m_eMode = CFX_SaxMode::NodeStart;
    return;
  }
  if (m_Data.empty() && SkipSpace(m_CurByte))
    return;

  ParseChar(m_CurByte);
}

void CFX_SAXReader::ParseNodeStart() {
  if (m_CurByte == '?') {
    GetCurrentItem()->m_eNode = CFX_SAXItem::Type::Instruction;
    m_eMode = CFX_SaxMode::TagName;
    return;
  }
  if (m_CurByte == '!') {
    m_eMode = CFX_SaxMode::DeclOrComment;
    return;
  }
  if (m_CurByte == '/') {
    m_eMode = CFX_SaxMode::TagEnd;
    return;
  }
  if (m_CurByte == '>') {
    Pop();
    m_eMode = CFX_SaxMode::Text;
    return;
  }
  if (m_CurByte > 0x20) {
    m_dwDataOffset = m_File.m_dwBufIndex;
    GetCurrentItem()->m_eNode = CFX_SAXItem::Type::Tag;
    m_eMode = CFX_SaxMode::TagName;
    AppendToData(m_CurByte);
  }
}

void CFX_SAXReader::ParseDeclOrComment() {
  if (m_CurByte == '-') {
    m_eMode = CFX_SaxMode::Comment;
    GetCurrentItem()->m_eNode = CFX_SAXItem::Type::Comment;
    if (!m_pCommentContext)
      m_pCommentContext = pdfium::MakeUnique<CFX_SAXCommentContext>();
    m_pCommentContext->m_iHeaderCount = 1;
    m_pCommentContext->m_iTailCount = 0;
    return;
  }
  m_eMode = CFX_SaxMode::DeclNode;
  m_dwDataOffset = m_File.m_dwBufIndex;
  m_SkipChar = '>';
  m_SkipStack.push('>');
  SkipNode();
}

void CFX_SAXReader::ParseComment() {
  m_pCommentContext->m_iHeaderCount = 2;
  m_dwNodePos = m_File.m_dwCur + m_File.m_dwBufIndex;
  m_eMode = CFX_SaxMode::CommentContent;
}

void CFX_SAXReader::ParseCommentContent() {
  if (m_CurByte == '-') {
    m_pCommentContext->m_iTailCount++;
    return;
  }
  if (m_CurByte == '>' && m_pCommentContext->m_iTailCount == 2) {
    NotifyTargetData();
    ClearData();
    Pop();
    m_eMode = CFX_SaxMode::Text;
    return;
  }
  while (m_pCommentContext->m_iTailCount > 0) {
    AppendToData('-');
    m_pCommentContext->m_iTailCount--;
  }
  AppendToData(m_CurByte);
}

void CFX_SAXReader::ParseDeclNode() {
  SkipNode();
}

void CFX_SAXReader::ParseTagName() {
  if (m_CurByte < 0x21 || m_CurByte == '/' || m_CurByte == '>' ||
      m_CurByte == '?') {
    NotifyEnter();
    ClearData();
    if (m_CurByte < 0x21) {
      ClearName();
      m_eMode = CFX_SaxMode::TagAttributeName;
    } else if (m_CurByte == '/' || m_CurByte == '?') {
      m_ePrevMode = m_eMode;
      m_eMode = CFX_SaxMode::TagMaybeClose;
    } else {
      NotifyBreak();
      m_eMode = CFX_SaxMode::Text;
    }
  } else {
    AppendToData(m_CurByte);
  }
}

void CFX_SAXReader::ParseTagAttributeName() {
  if (m_CurByte < 0x21 || m_CurByte == '=') {
    if (m_Name.empty() && m_CurByte < 0x21)
      return;

    m_SkipChar = 0;
    m_eMode = m_CurByte == '=' ? CFX_SaxMode::TagAttributeValue
                               : CFX_SaxMode::TagAttributeEqual;
    ClearData();
    return;
  }
  if (m_CurByte == '/' || m_CurByte == '>' || m_CurByte == '?') {
    if (m_CurByte == '/' || m_CurByte == '?') {
      m_ePrevMode = m_eMode;
      m_eMode = CFX_SaxMode::TagMaybeClose;
    } else {
      NotifyBreak();
      m_eMode = CFX_SaxMode::Text;
    }
    return;
  }
  if (m_Name.empty())
    m_dwDataOffset = m_File.m_dwBufIndex;
  AppendToName(m_CurByte);
}

void CFX_SAXReader::ParseTagAttributeEqual() {
  if (m_CurByte == '=') {
    m_SkipChar = 0;
    m_eMode = CFX_SaxMode::TagAttributeValue;
    return;
  }
  if (GetCurrentItem()->m_eNode == CFX_SAXItem::Type::Instruction) {
    AppendToName(0x20);
    m_eMode = CFX_SaxMode::TargetData;
    ParseTargetData();
  }
}

void CFX_SAXReader::ParseTagAttributeValue() {
  if (m_SkipChar) {
    if (m_SkipChar == m_CurByte) {
      NotifyAttribute();
      ClearData();
      ClearName();
      m_SkipChar = 0;
      m_eMode = CFX_SaxMode::TagAttributeName;
      return;
    }
    ParseChar(m_CurByte);
    return;
  }
  if (m_CurByte < 0x21) {
    return;
  }
  if (m_Data.empty()) {
    if (m_CurByte == '\'' || m_CurByte == '\"')
      m_SkipChar = m_CurByte;
  }
}

void CFX_SAXReader::ParseMaybeClose() {
  if (m_CurByte == '>') {
    if (GetCurrentItem()->m_eNode == CFX_SAXItem::Type::Instruction) {
      NotifyTargetData();
      ClearData();
      ClearName();
    }
    ParseTagClose();
    m_eMode = CFX_SaxMode::Text;
  } else if (m_ePrevMode == CFX_SaxMode::TagName) {
    AppendToData('/');
    m_eMode = CFX_SaxMode::TagName;
    m_ePrevMode = CFX_SaxMode::Text;
    ParseTagName();
  } else if (m_ePrevMode == CFX_SaxMode::TagAttributeName) {
    AppendToName('/');
    m_eMode = CFX_SaxMode::TagAttributeName;
    m_ePrevMode = CFX_SaxMode::Text;
    ParseTagAttributeName();
  } else if (m_ePrevMode == CFX_SaxMode::TargetData) {
    AppendToName('?');
    m_eMode = CFX_SaxMode::TargetData;
    m_ePrevMode = CFX_SaxMode::Text;
    ParseTargetData();
  }
}
void CFX_SAXReader::ParseTagClose() {
  m_dwNodePos = m_File.m_dwCur + m_File.m_dwBufIndex;
  NotifyClose();
  Pop();
}
void CFX_SAXReader::ParseTagEnd() {
  if (m_CurByte < 0x21) {
    return;
  }
  if (m_CurByte == '>') {
    Pop();
    m_dwNodePos = m_File.m_dwCur + m_File.m_dwBufIndex;
    NotifyEnd();
    ClearData();
    Pop();
    m_eMode = CFX_SaxMode::Text;
  } else {
    ParseChar(m_CurByte);
  }
}
void CFX_SAXReader::ParseTargetData() {
  if (m_CurByte == '?') {
    m_ePrevMode = m_eMode;
    m_eMode = CFX_SaxMode::TagMaybeClose;
  } else {
    AppendToName(m_CurByte);
  }
}
void CFX_SAXReader::SkipNode() {
  if (m_SkipChar == '\'' || m_SkipChar == '\"') {
    if (m_CurByte != m_SkipChar)
      return;

    ASSERT(!m_SkipStack.empty());
    m_SkipStack.pop();
    m_SkipChar = !m_SkipStack.empty() ? m_SkipStack.top() : 0;
    return;
  }
  switch (m_CurByte) {
    case '<':
      m_SkipChar = '>';
      m_SkipStack.push('>');
      break;
    case '[':
      m_SkipChar = ']';
      m_SkipStack.push(']');
      break;
    case '(':
      m_SkipChar = ')';
      m_SkipStack.push(')');
      break;
    case '\'':
      m_SkipChar = '\'';
      m_SkipStack.push('\'');
      break;
    case '\"':
      m_SkipChar = '\"';
      m_SkipStack.push('\"');
      break;
    default:
      if (m_CurByte == m_SkipChar) {
        m_SkipStack.pop();
        m_SkipChar = !m_SkipStack.empty() ? m_SkipStack.top() : 0;
        if (m_SkipStack.empty() && m_CurByte == '>') {
          if (m_Data.size() >= 9 && memcmp(m_Data.data(), "[CDATA[", 7) == 0 &&
              memcmp(m_Data.data() + m_Data.size() - 2, "]]", 2) == 0) {
            Pop();
            m_Data.erase(m_Data.begin(), m_Data.begin() + 7);
            m_Data.erase(m_Data.end() - 2, m_Data.end());
            m_bCharData = true;
            NotifyData();
            m_bCharData = false;
          } else {
            Pop();
          }
          ClearData();
          m_eMode = CFX_SaxMode::Text;
        }
      }
      break;
  }
  if (!m_SkipStack.empty())
    ParseChar(m_CurByte);
}

void CFX_SAXReader::NotifyData() {
  if (!m_pHandler)
    return;

  CFX_SAXItem* pItem = GetCurrentItem();
  if (!pItem)
    return;

  if (pItem->m_eNode == CFX_SAXItem::Type::Tag)
    m_pHandler->OnTagData(
        pItem->m_pNode,
        m_bCharData ? CFX_SAXItem::Type::CharData : CFX_SAXItem::Type::Text,
        ByteStringView(m_Data), m_File.m_dwCur + m_dwDataOffset);
}

void CFX_SAXReader::NotifyEnter() {
  if (!m_pHandler)
    return;

  CFX_SAXItem* pItem = GetCurrentItem();
  if (!pItem)
    return;

  if (pItem->m_eNode == CFX_SAXItem::Type::Tag ||
      pItem->m_eNode == CFX_SAXItem::Type::Instruction) {
    pItem->m_pNode = m_pHandler->OnTagEnter(ByteStringView(m_Data),
                                            pItem->m_eNode, m_dwNodePos);
  }
}

void CFX_SAXReader::NotifyAttribute() {
  if (!m_pHandler)
    return;

  CFX_SAXItem* pItem = GetCurrentItem();
  if (!pItem)
    return;

  if (pItem->m_eNode == CFX_SAXItem::Type::Tag ||
      pItem->m_eNode == CFX_SAXItem::Type::Instruction) {
    m_pHandler->OnTagAttribute(pItem->m_pNode, ByteStringView(m_Name),
                               ByteStringView(m_Data));
  }
}

void CFX_SAXReader::NotifyBreak() {
  if (!m_pHandler)
    return;

  CFX_SAXItem* pItem = GetCurrentItem();
  if (!pItem)
    return;

  if (pItem->m_eNode == CFX_SAXItem::Type::Tag)
    m_pHandler->OnTagBreak(pItem->m_pNode);
}

void CFX_SAXReader::NotifyClose() {
  if (!m_pHandler)
    return;

  CFX_SAXItem* pItem = GetCurrentItem();
  if (!pItem)
    return;

  if (pItem->m_eNode == CFX_SAXItem::Type::Tag ||
      pItem->m_eNode == CFX_SAXItem::Type::Instruction) {
    m_pHandler->OnTagClose(pItem->m_pNode, m_dwNodePos);
  }
}

void CFX_SAXReader::NotifyEnd() {
  if (!m_pHandler)
    return;

  CFX_SAXItem* pItem = GetCurrentItem();
  if (!pItem)
    return;

  if (pItem->m_eNode == CFX_SAXItem::Type::Tag)
    m_pHandler->OnTagEnd(pItem->m_pNode, ByteStringView(m_Data), m_dwNodePos);
}

void CFX_SAXReader::NotifyTargetData() {
  if (!m_pHandler)
    return;

  CFX_SAXItem* pItem = GetCurrentItem();
  if (!pItem)
    return;

  if (pItem->m_eNode == CFX_SAXItem::Type::Instruction) {
    m_pHandler->OnTargetData(pItem->m_pNode, pItem->m_eNode,
                             ByteStringView(m_Name), m_dwNodePos);
  } else if (pItem->m_eNode == CFX_SAXItem::Type::Comment) {
    m_pHandler->OnTargetData(pItem->m_pNode, pItem->m_eNode,
                             ByteStringView(m_Data), m_dwNodePos);
  }
}

void CFX_SAXReader::SkipCurrentNode() {
  CFX_SAXItem* pItem = GetCurrentItem();
  if (pItem)
    pItem->m_bSkip = true;
}
