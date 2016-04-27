// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FGAS_XML_FGAS_SAX_H_
#define XFA_FGAS_XML_FGAS_SAX_H_

#include "core/fxcrt/include/fx_basic.h"

#define FX_SAXPARSEMODE_NotConvert_amp 0x0001
#define FX_SAXPARSEMODE_NotConvert_lt 0x0002
#define FX_SAXPARSEMODE_NotConvert_gt 0x0004
#define FX_SAXPARSEMODE_NotConvert_apos 0x0008
#define FX_SAXPARSEMODE_NotConvert_quot 0x0010
#define FX_SAXPARSEMODE_NotConvert_sharp 0x0020
#define FX_SAXPARSEMODE_NotSkipSpace 0x0100

enum FX_SAXNODE {
  FX_SAXNODE_Unknown = 0,
  FX_SAXNODE_Instruction,
  FX_SAXNODE_Declaration,
  FX_SAXNODE_Comment,
  FX_SAXNODE_Tag,
  FX_SAXNODE_Text,
  FX_SAXNODE_CharData,
};

enum FX_SAXMODE {
  FX_SAXMODE_Text = 0,
  FX_SAXMODE_NodeStart,
  FX_SAXMODE_DeclOrComment,
  FX_SAXMODE_DeclNode,
  FX_SAXMODE_Comment,
  FX_SAXMODE_CommentContent,
  FX_SAXMODE_TagName,
  FX_SAXMODE_TagAttributeName,
  FX_SAXMODE_TagAttributeEqual,
  FX_SAXMODE_TagAttributeValue,
  FX_SAXMODE_TagMaybeClose,
  FX_SAXMODE_TagClose,
  FX_SAXMODE_TagEnd,
  FX_SAXMODE_TargetData,
  FX_SAXMODE_MAX,
};

class CXFA_SAXReaderHandler;

class CFX_SAXFile {
 public:
  CFX_SAXFile();
  FX_BOOL StartFile(IFX_FileRead* pFile, uint32_t dwStart, uint32_t dwLen);
  FX_BOOL ReadNextBlock();
  void Reset();
  IFX_FileRead* m_pFile;
  uint32_t m_dwStart;
  uint32_t m_dwEnd;
  uint32_t m_dwCur;
  uint8_t* m_pBuf;
  uint32_t m_dwBufSize;
  uint32_t m_dwBufIndex;
};

class CFX_SAXItem {
 public:
  CFX_SAXItem()
      : m_pNode(NULL),
        m_eNode(FX_SAXNODE_Unknown),
        m_dwID(0),
        m_bSkip(FALSE),
        m_pPrev(NULL),
        m_pNext(NULL) {}
  void* m_pNode;
  FX_SAXNODE m_eNode;
  uint32_t m_dwID;
  FX_BOOL m_bSkip;
  CFX_SAXItem* m_pPrev;
  CFX_SAXItem* m_pNext;
};

class CFX_SAXCommentContext {
 public:
  CFX_SAXCommentContext() : m_iHeaderCount(0), m_iTailCount(0) {}
  int32_t m_iHeaderCount;
  int32_t m_iTailCount;
};

class CFX_SAXReader {
 public:
  CFX_SAXReader();
  ~CFX_SAXReader();

  void Release() { delete this; }
  int32_t StartParse(IFX_FileRead* pFile,
                     uint32_t dwStart = 0,
                     uint32_t dwLen = -1,
                     uint32_t dwParseMode = 0);
  int32_t ContinueParse(IFX_Pause* pPause = NULL);
  void SkipCurrentNode();
  void SetHandler(CXFA_SAXReaderHandler* pHandler);
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

 protected:
  void Reset();
  void Push();
  void Pop();
  FX_BOOL SkipSpace(uint8_t ch);
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
  CXFA_SAXReaderHandler* m_pHandler;
  int32_t m_iState;
  CFX_SAXItem* m_pRoot;
  CFX_SAXItem* m_pCurItem;
  uint32_t m_dwItemID;
  FX_SAXMODE m_eMode;
  FX_SAXMODE m_ePrevMode;
  FX_BOOL m_bCharData;
  uint8_t m_CurByte;
  uint32_t m_dwDataOffset;
  CFX_ByteArray m_SkipStack;
  uint8_t m_SkipChar;
  uint32_t m_dwNodePos;
  uint8_t* m_pszData;
  int32_t m_iDataSize;
  int32_t m_iDataLength;
  int32_t m_iEntityStart;
  int32_t m_iDataPos;
  uint8_t* m_pszName;
  int32_t m_iNameSize;
  int32_t m_iNameLength;
  uint32_t m_dwParseMode;
  CFX_SAXCommentContext* m_pCommentContext;
};

#endif  // XFA_FGAS_XML_FGAS_SAX_H_
