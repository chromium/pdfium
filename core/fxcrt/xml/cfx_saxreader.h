// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_XML_CFX_SAXREADER_H_
#define CORE_FXCRT_XML_CFX_SAXREADER_H_

#include <memory>
#include <stack>
#include <vector>

#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/retain_ptr.h"

class CFX_SAXCommentContext;
class CFX_SAXContext;
class IFX_SeekableReadStream;
enum class CFX_SaxMode;

class CFX_SAXItem {
 public:
  enum class Type {
    Unknown = 0,
    Instruction,
    Declaration,
    Comment,
    Tag,
    Text,
    CharData,
  };

  explicit CFX_SAXItem(uint32_t id)
      : m_pNode(nullptr), m_eNode(Type::Unknown), m_dwID(id), m_bSkip(false) {}

  CFX_SAXContext* m_pNode;
  Type m_eNode;
  const uint32_t m_dwID;
  bool m_bSkip;
};

class CFX_SAXFile {
 public:
  CFX_SAXFile();
  ~CFX_SAXFile();

  bool StartFile(const RetainPtr<IFX_SeekableReadStream>& pFile,
                 uint32_t dwStart,
                 uint32_t dwLen);
  bool ReadNextBlock();
  void Reset();

  RetainPtr<IFX_SeekableReadStream> m_pFile;
  uint32_t m_dwStart;
  uint32_t m_dwEnd;
  uint32_t m_dwCur;
  uint8_t* m_pBuf;
  uint32_t m_dwBufSize;
  uint32_t m_dwBufIndex;
};

enum CFX_SaxParseMode {
  CFX_SaxParseMode_NotConvert_amp = 1 << 0,
  CFX_SaxParseMode_NotConvert_lt = 1 << 1,
  CFX_SaxParseMode_NotConvert_gt = 1 << 2,
  CFX_SaxParseMode_NotConvert_apos = 1 << 3,
  CFX_SaxParseMode_NotConvert_quot = 1 << 4,
  CFX_SaxParseMode_NotConvert_sharp = 1 << 5,
  CFX_SaxParseMode_NotSkipSpace = 1 << 6
};

class CFX_SAXReader {
 public:
  class HandlerIface {
   public:
    virtual ~HandlerIface() {}
    virtual CFX_SAXContext* OnTagEnter(const ByteStringView& bsTagName,
                                       CFX_SAXItem::Type eType,
                                       uint32_t dwStartPos) = 0;
    virtual void OnTagAttribute(CFX_SAXContext* pTag,
                                const ByteStringView& bsAttri,
                                const ByteStringView& bsValue) = 0;
    virtual void OnTagBreak(CFX_SAXContext* pTag) = 0;
    virtual void OnTagData(CFX_SAXContext* pTag,
                           CFX_SAXItem::Type eType,
                           const ByteStringView& bsData,
                           uint32_t dwStartPos) = 0;
    virtual void OnTagClose(CFX_SAXContext* pTag, uint32_t dwEndPos) = 0;
    virtual void OnTagEnd(CFX_SAXContext* pTag,
                          const ByteStringView& bsTagName,
                          uint32_t dwEndPos) = 0;
    virtual void OnTargetData(CFX_SAXContext* pTag,
                              CFX_SAXItem::Type eType,
                              const ByteStringView& bsData,
                              uint32_t dwStartPos) = 0;
  };

  CFX_SAXReader();
  ~CFX_SAXReader();

  int32_t StartParse(const RetainPtr<IFX_SeekableReadStream>& pFile,
                     uint32_t dwStart = 0,
                     uint32_t dwLen = -1,
                     uint32_t dwParseMode = 0);
  int32_t ContinueParse();
  void SetHandler(HandlerIface* pHandler) { m_pHandler = pHandler; }

 private:
  void ParseInternal();
  void SkipCurrentNode();
  void AppendData(uint8_t ch);
  void AppendName(uint8_t ch);
  void ParseText();
  void ParseNodeStart();
  void ParseInstruction();
  void ParseDeclOrComment();
  void ParseDeclNode();
  void ParseComment();
  void ParseCommentContent();
  void ParseTagName();
  void ParseTagAttributeName();
  void ParseTagAttributeEqual();
  void ParseTagAttributeValue();
  void ParseMaybeClose();
  void ParseTagClose();
  void ParseTagEnd();
  void ParseTargetData();
  void Reset();
  void ClearData();
  void ClearName();
  void AppendToData(uint8_t ch);
  void AppendToName(uint8_t ch);
  void BackUpAndReplaceDataAt(int32_t index, uint8_t ch);
  bool IsEntityStart(uint8_t ch) const;
  bool IsEntityEnd(uint8_t ch) const;
  int32_t CurrentDataIndex() const;
  void Push();
  void Pop();
  CFX_SAXItem* GetCurrentItem() const;
  bool SkipSpace(uint8_t ch);
  void SkipNode();
  void NotifyData();
  void NotifyEnter();
  void NotifyAttribute();
  void NotifyBreak();
  void NotifyClose();
  void NotifyEnd();
  void NotifyTargetData();
  void ReallocDataBuffer();
  void ReallocNameBuffer();
  void ParseChar(uint8_t ch);

  CFX_SAXFile m_File;
  HandlerIface* m_pHandler;
  int32_t m_iState;
  std::stack<std::unique_ptr<CFX_SAXItem>> m_Stack;
  uint32_t m_dwItemID;
  CFX_SaxMode m_eMode;
  CFX_SaxMode m_ePrevMode;
  bool m_bCharData;
  uint8_t m_CurByte;
  uint32_t m_dwDataOffset;
  std::stack<char> m_SkipStack;
  uint8_t m_SkipChar;
  uint32_t m_dwNodePos;
  std::vector<uint8_t> m_Data;
  int32_t m_iEntityStart;  // Index into m_Data.
  std::vector<uint8_t> m_Name;
  uint32_t m_dwParseMode;
  std::unique_ptr<CFX_SAXCommentContext> m_pCommentContext;
};

#endif  // CORE_FXCRT_XML_CFX_SAXREADER_H_
