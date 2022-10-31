// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cjx_source.h"

#include <vector>

#include "fxjs/js_resources.h"
#include "fxjs/xfa/cfxjse_value.h"
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
    {"updateBatch", updateBatch_static}};

CJX_Source::CJX_Source(CXFA_Source* src) : CJX_Node(src) {
  DefineMethods(MethodSpecs);
}

CJX_Source::~CJX_Source() = default;

bool CJX_Source::DynamicTypeIs(TypeTag eType) const {
  return eType == static_type__ || ParentType__::DynamicTypeIs(eType);
}

CJS_Result CJX_Source::next(CFXJSE_Engine* runtime,
                            const std::vector<v8::Local<v8::Value>>& params) {
  if (!params.empty())
    return CJS_Result::Failure(JSMessage::kParamError);

  return CJS_Result::Success();
}

CJS_Result CJX_Source::cancelBatch(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (!params.empty())
    return CJS_Result::Failure(JSMessage::kParamError);

  return CJS_Result::Success();
}

CJS_Result CJX_Source::first(CFXJSE_Engine* runtime,
                             const std::vector<v8::Local<v8::Value>>& params) {
  if (!params.empty())
    return CJS_Result::Failure(JSMessage::kParamError);

  return CJS_Result::Success();
}

CJS_Result CJX_Source::updateBatch(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (!params.empty())
    return CJS_Result::Failure(JSMessage::kParamError);

  return CJS_Result::Success();
}

CJS_Result CJX_Source::previous(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (!params.empty())
    return CJS_Result::Failure(JSMessage::kParamError);

  return CJS_Result::Success();
}

CJS_Result CJX_Source::isBOF(CFXJSE_Engine* runtime,
                             const std::vector<v8::Local<v8::Value>>& params) {
  if (!params.empty())
    return CJS_Result::Failure(JSMessage::kParamError);

  return CJS_Result::Success();
}

CJS_Result CJX_Source::isEOF(CFXJSE_Engine* runtime,
                             const std::vector<v8::Local<v8::Value>>& params) {
  if (!params.empty())
    return CJS_Result::Failure(JSMessage::kParamError);

  return CJS_Result::Success();
}

CJS_Result CJX_Source::cancel(CFXJSE_Engine* runtime,
                              const std::vector<v8::Local<v8::Value>>& params) {
  if (!params.empty())
    return CJS_Result::Failure(JSMessage::kParamError);

  return CJS_Result::Success();
}

CJS_Result CJX_Source::update(CFXJSE_Engine* runtime,
                              const std::vector<v8::Local<v8::Value>>& params) {
  if (!params.empty())
    return CJS_Result::Failure(JSMessage::kParamError);

  return CJS_Result::Success();
}

CJS_Result CJX_Source::open(CFXJSE_Engine* runtime,
                            const std::vector<v8::Local<v8::Value>>& params) {
  if (!params.empty())
    return CJS_Result::Failure(JSMessage::kParamError);

  return CJS_Result::Success();
}

CJS_Result CJX_Source::deleteItem(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (!params.empty())
    return CJS_Result::Failure(JSMessage::kParamError);

  return CJS_Result::Success();
}

CJS_Result CJX_Source::addNew(CFXJSE_Engine* runtime,
                              const std::vector<v8::Local<v8::Value>>& params) {
  if (!params.empty())
    return CJS_Result::Failure(JSMessage::kParamError);

  return CJS_Result::Success();
}

CJS_Result CJX_Source::requery(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (!params.empty())
    return CJS_Result::Failure(JSMessage::kParamError);

  return CJS_Result::Success();
}

CJS_Result CJX_Source::resync(CFXJSE_Engine* runtime,
                              const std::vector<v8::Local<v8::Value>>& params) {
  if (!params.empty())
    return CJS_Result::Failure(JSMessage::kParamError);

  return CJS_Result::Success();
}

CJS_Result CJX_Source::close(CFXJSE_Engine* runtime,
                             const std::vector<v8::Local<v8::Value>>& params) {
  if (!params.empty())
    return CJS_Result::Failure(JSMessage::kParamError);

  return CJS_Result::Success();
}

CJS_Result CJX_Source::last(CFXJSE_Engine* runtime,
                            const std::vector<v8::Local<v8::Value>>& params) {
  if (!params.empty())
    return CJS_Result::Failure(JSMessage::kParamError);

  return CJS_Result::Success();
}

CJS_Result CJX_Source::hasDataChanged(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (!params.empty())
    return CJS_Result::Failure(JSMessage::kParamError);

  return CJS_Result::Success();
}

void CJX_Source::db(v8::Isolate* pIsolate,
                    v8::Local<v8::Value>* pValue,
                    bool bSetting,
                    XFA_Attribute eAttribute) {}
