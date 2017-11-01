// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/cjx_logpseudomodel.h"

#include "fxjs/cfxjse_arguments.h"
#include "fxjs/cfxjse_value.h"
#include "xfa/fxfa/parser/cscript_logpseudomodel.h"

CJX_LogPseudoModel::CJX_LogPseudoModel(CScript_LogPseudoModel* model)
    : CJX_Object(model) {}

CJX_LogPseudoModel::~CJX_LogPseudoModel() {}

void CJX_LogPseudoModel::Message(CFXJSE_Arguments* pArguments) {}

void CJX_LogPseudoModel::TraceEnabled(CFXJSE_Arguments* pArguments) {}

void CJX_LogPseudoModel::TraceActivate(CFXJSE_Arguments* pArguments) {}

void CJX_LogPseudoModel::TraceDeactivate(CFXJSE_Arguments* pArguments) {}

void CJX_LogPseudoModel::Trace(CFXJSE_Arguments* pArguments) {}
