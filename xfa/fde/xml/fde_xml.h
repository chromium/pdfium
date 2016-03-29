// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_XML_FDE_XML_H_
#define XFA_FDE_XML_FDE_XML_H_

#include "xfa/fgas/crt/fgas_stream.h"
#include "xfa/fgas/crt/fgas_utils.h"

enum FDE_XMLNODETYPE {
  FDE_XMLNODE_Unknown = 0,
  FDE_XMLNODE_Instruction,
  FDE_XMLNODE_Element,
  FDE_XMLNODE_Text,
  FDE_XMLNODE_CharData,
};

struct FDE_XMLNODE {
  int32_t iNodeNum;
  FDE_XMLNODETYPE eNodeType;
};
typedef CFX_StackTemplate<FDE_XMLNODE> CFDE_XMLNodeStack;

FX_BOOL FDE_IsXMLValidChar(FX_WCHAR ch);
FX_BOOL FDE_IsXMLWhiteSpace(FX_WCHAR ch);
FX_BOOL FDE_IsXMLNameChar(FX_WCHAR ch, FX_BOOL bFirstChar);

struct FDE_XMLREADERHANDLER {
  void* pData;
  void (*OnTagEnter)(FDE_XMLREADERHANDLER* pThis,
                     FDE_XMLNODETYPE eType,
                     const CFX_WideString& wsTagName);
  void (*OnTagBreak)(FDE_XMLREADERHANDLER* pThis,
                     const CFX_WideString& wsTagName);
  void (*OnTagClose)(FDE_XMLREADERHANDLER* pThis,
                     const CFX_WideString& wsTagName);
  void (*OnAttribute)(FDE_XMLREADERHANDLER* pThis,
                      const CFX_WideString& wsName,
                      const CFX_WideString& wsValue);
  void (*OnData)(FDE_XMLREADERHANDLER* pThis,
                 FDE_XMLNODETYPE eType,
                 const CFX_WideString& wsValue);
};

#define FDE_XMLSYNTAXSTATUS_None 0x00
#define FDE_XMLSYNTAXSTATUS_InstructionOpen 0x01
#define FDE_XMLSYNTAXSTATUS_InstructionClose 0x02
#define FDE_XMLSYNTAXSTATUS_ElementOpen 0x03
#define FDE_XMLSYNTAXSTATUS_ElementBreak 0x04
#define FDE_XMLSYNTAXSTATUS_ElementClose 0x05
#define FDE_XMLSYNTAXSTATUS_TargetName 0x06
#define FDE_XMLSYNTAXSTATUS_TagName 0x07
#define FDE_XMLSYNTAXSTATUS_AttriName 0x08
#define FDE_XMLSYNTAXSTATUS_AttriValue 0x09
#define FDE_XMLSYNTAXSTATUS_Text 0x0A
#define FDE_XMLSYNTAXSTATUS_CData 0x0B
#define FDE_XMLSYNTAXSTATUS_TargetData 0x0C
#define FDE_XMLSYNTAXSTATUS_Error 0xFE
#define FDE_XMLSYNTAXSTATUS_EOS 0xFF

#endif  // XFA_FDE_XML_FDE_XML_H_
