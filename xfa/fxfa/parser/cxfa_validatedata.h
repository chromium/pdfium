// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_VALIDATEDATA_H_
#define XFA_FXFA_PARSER_CXFA_VALIDATEDATA_H_

#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/fx_system.h"
#include "xfa/fxfa/parser/cxfa_data.h"
#include "xfa/fxfa/parser/cxfa_scriptdata.h"

class CXFA_Node;

class CXFA_ValidateData : public CXFA_Data {
 public:
  explicit CXFA_ValidateData(CXFA_Node* pNode);

  int32_t GetFormatTest();
  int32_t GetNullTest();
  bool SetNullTest(WideString wsValue);
  int32_t GetScriptTest();
  void GetFormatMessageText(WideString& wsMessage);
  void SetFormatMessageText(WideString wsMessage);
  void GetNullMessageText(WideString& wsMessage);
  void SetNullMessageText(WideString wsMessage);
  void GetScriptMessageText(WideString& wsMessage);
  void SetScriptMessageText(WideString wsMessage);
  void GetPicture(WideString& wsPicture);
  CXFA_ScriptData GetScriptData();

 private:
  void GetMessageText(WideString& wsMessage, const WideString& wsMessageType);
  void SetMessageText(WideString& wsMessage, const WideString& wsMessageType);
};

#endif  // XFA_FXFA_PARSER_CXFA_VALIDATEDATA_H_
