// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cjx_source.h"

#include "fxjs/cfxjse_arguments.h"
#include "fxjs/cfxjse_value.h"
#include "xfa/fxfa/parser/cxfa_source.h"

const CJX_MethodSpec CJX_Source::MethodSpecs[] = {
    {"addNew", addNew_static},
    {"cancel", cancel_static},
    {"cancelBatch", cancelBatch_static},
    {"close", close_static},
    {"delete", deleteItem_static},
    {"first", first_static},
    {"hasDataChanged", hasDataChanged_static},
    {"isBOF", isBOF_static},
    {"isEOF", isEOF_static},
    {"last", last_static},
    {"next", next_static},
    {"open", open_static},
    {"previous", previous_static},
    {"requery", requery_static},
    {"resync", resync_static},
    {"update", update_static},
    {"updateBatch", updateBatch_static},
    {"", nullptr}};

CJX_Source::CJX_Source(CXFA_Source* src) : CJX_Node(src) {
  DefineMethods(MethodSpecs);
}

CJX_Source::~CJX_Source() {}

void CJX_Source::next(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0)
    ThrowParamCountMismatchException(L"next");
}

void CJX_Source::cancelBatch(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0)
    ThrowParamCountMismatchException(L"cancelBatch");
}

void CJX_Source::first(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0)
    ThrowParamCountMismatchException(L"first");
}

void CJX_Source::updateBatch(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0)
    ThrowParamCountMismatchException(L"updateBatch");
}

void CJX_Source::previous(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0)
    ThrowParamCountMismatchException(L"previous");
}

void CJX_Source::isBOF(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0)
    ThrowParamCountMismatchException(L"isBOF");
}

void CJX_Source::isEOF(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0)
    ThrowParamCountMismatchException(L"isEOF");
}

void CJX_Source::cancel(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0)
    ThrowParamCountMismatchException(L"cancel");
}

void CJX_Source::update(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0)
    ThrowParamCountMismatchException(L"update");
}

void CJX_Source::open(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0)
    ThrowParamCountMismatchException(L"open");
}

void CJX_Source::deleteItem(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0)
    ThrowParamCountMismatchException(L"delete");
}

void CJX_Source::addNew(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0)
    ThrowParamCountMismatchException(L"addNew");
}

void CJX_Source::requery(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0)
    ThrowParamCountMismatchException(L"requery");
}

void CJX_Source::resync(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0)
    ThrowParamCountMismatchException(L"resync");
}

void CJX_Source::close(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0)
    ThrowParamCountMismatchException(L"close");
}

void CJX_Source::last(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0)
    ThrowParamCountMismatchException(L"last");
}

void CJX_Source::hasDataChanged(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0)
    ThrowParamCountMismatchException(L"hasDataChanged");
}
