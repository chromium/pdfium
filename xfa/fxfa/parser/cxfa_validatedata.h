// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_VALIDATEDATA_H_
#define XFA_FXFA_PARSER_CXFA_VALIDATEDATA_H_

#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/fx_system.h"
#include "xfa/fxfa/parser/cxfa_datadata.h"
#include "xfa/fxfa/parser/cxfa_scriptdata.h"

class CXFA_Node;

class CXFA_ValidateData : public CXFA_DataData {
 public:
  explicit CXFA_ValidateData(CXFA_Node* pNode);

  XFA_AttributeEnum GetFormatTest() const;
  WideString GetFormatMessageText() const;
  void SetFormatMessageText(const WideString& wsMessage);

  XFA_AttributeEnum GetNullTest() const;
  void SetNullTest(const WideString& wsValue);

  WideString GetNullMessageText() const;
  void SetNullMessageText(const WideString& wsMessage);

  XFA_AttributeEnum GetScriptTest() const;
  WideString GetScriptMessageText() const;
  void SetScriptMessageText(const WideString& wsMessage);

  WideString GetPicture() const;

  CXFA_ScriptData GetScriptData() const;

 private:
  WideString GetMessageText(const WideString& wsMessageType) const;
  void SetMessageText(const WideString& wsMessageType,
                      const WideString& wsMessage);
};

#endif  // XFA_FXFA_PARSER_CXFA_VALIDATEDATA_H_
