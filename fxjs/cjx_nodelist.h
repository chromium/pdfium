// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_CJX_NODELIST_H_
#define FXJS_CJX_NODELIST_H_

#include "fxjs/cjx_object.h"
#include "xfa/fxfa/fxfa_basic.h"

class CFXJSE_Arguments;
class CFXJSE_Value;
class CXFA_NodeList;

class CJX_NodeList : public CJX_Object {
 public:
  explicit CJX_NodeList(CXFA_NodeList* list);
  ~CJX_NodeList() override;

  CXFA_NodeList* GetXFANodeList();

  void Script_ListClass_Append(CFXJSE_Arguments* pArguments);
  void Script_ListClass_Insert(CFXJSE_Arguments* pArguments);
  void Script_ListClass_Remove(CFXJSE_Arguments* pArguments);
  void Script_ListClass_Item(CFXJSE_Arguments* pArguments);

  void Script_TreelistClass_NamedItem(CFXJSE_Arguments* pArguments);
  void Script_ListClass_Length(CFXJSE_Value* pValue,
                               bool bSetting,
                               XFA_Attribute eAttribute);
};

#endif  // FXJS_CJX_NODELIST_H_
