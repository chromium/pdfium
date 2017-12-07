// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cjx_wsdlconnection.h"

#include "fxjs/cfxjse_arguments.h"
#include "fxjs/cfxjse_value.h"
#include "xfa/fxfa/parser/cxfa_wsdlconnection.h"

const CJX_MethodSpec CJX_WsdlConnection::MethodSpecs[] = {
    {"execute", execute_static},
    {"", nullptr}};

CJX_WsdlConnection::CJX_WsdlConnection(CXFA_WsdlConnection* connection)
    : CJX_Node(connection) {
  DefineMethods(MethodSpecs);
}

CJX_WsdlConnection::~CJX_WsdlConnection() {}

void CJX_WsdlConnection::execute(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc != 0 && argc != 1) {
    ThrowParamCountMismatchException(L"execute");
    return;
  }
  pArguments->GetReturnValue()->SetBoolean(false);
}
