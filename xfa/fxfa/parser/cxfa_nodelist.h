// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_NODELIST_H_
#define XFA_FXFA_PARSER_CXFA_NODELIST_H_

#include "xfa/fxfa/fxfa_basic.h"
#include "xfa/fxfa/parser/cxfa_object.h"

class CXFA_Node;
class CFXJSE_Arguments;
class CFXJSE_Value;

class CXFA_NodeList : public CXFA_Object {
 public:
  explicit CXFA_NodeList(CXFA_Document* pDocument);
  ~CXFA_NodeList() override;

  CXFA_Node* NamedItem(const CFX_WideStringC& wsName);
  virtual int32_t GetLength() = 0;
  virtual bool Append(CXFA_Node* pNode) = 0;
  virtual bool Insert(CXFA_Node* pNewNode, CXFA_Node* pBeforeNode) = 0;
  virtual bool Remove(CXFA_Node* pNode) = 0;
  virtual CXFA_Node* Item(int32_t iIndex) = 0;

  void Script_ListClass_Append(CFXJSE_Arguments* pArguments);
  void Script_ListClass_Insert(CFXJSE_Arguments* pArguments);
  void Script_ListClass_Remove(CFXJSE_Arguments* pArguments);
  void Script_ListClass_Item(CFXJSE_Arguments* pArguments);

  void Script_TreelistClass_NamedItem(CFXJSE_Arguments* pArguments);
  void Script_ListClass_Length(CFXJSE_Value* pValue,
                               bool bSetting,
                               XFA_ATTRIBUTE eAttribute);
};

#endif  // XFA_FXFA_PARSER_CXFA_NODELIST_H_
