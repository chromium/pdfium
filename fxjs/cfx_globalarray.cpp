// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/cfx_globalarray.h"

#include <utility>

#include "fxjs/cfx_keyvalue.h"
#include "third_party/base/ptr_util.h"

CFX_GlobalArray::CFX_GlobalArray() = default;

CFX_GlobalArray::~CFX_GlobalArray() = default;

CFX_GlobalArray& CFX_GlobalArray::operator=(const CFX_GlobalArray& that) {
  if (this == &that)
    return *this;

  m_Array.clear();
  for (int i = 0, sz = that.Count(); i < sz; i++) {
    CFX_KeyValue* pOldObjData = that.GetAt(i);
    switch (pOldObjData->nType) {
      case JS_GlobalDataType::NUMBER: {
        auto pNewObjData = pdfium::MakeUnique<CFX_KeyValue>();
        pNewObjData->sKey = pOldObjData->sKey;
        pNewObjData->nType = pOldObjData->nType;
        pNewObjData->dData = pOldObjData->dData;
        Add(std::move(pNewObjData));
      } break;
      case JS_GlobalDataType::BOOLEAN: {
        auto pNewObjData = pdfium::MakeUnique<CFX_KeyValue>();
        pNewObjData->sKey = pOldObjData->sKey;
        pNewObjData->nType = pOldObjData->nType;
        pNewObjData->bData = pOldObjData->bData;
        Add(std::move(pNewObjData));
      } break;
      case JS_GlobalDataType::STRING: {
        auto pNewObjData = pdfium::MakeUnique<CFX_KeyValue>();
        pNewObjData->sKey = pOldObjData->sKey;
        pNewObjData->nType = pOldObjData->nType;
        pNewObjData->sData = pOldObjData->sData;
        Add(std::move(pNewObjData));
      } break;
      case JS_GlobalDataType::OBJECT: {
        auto pNewObjData = pdfium::MakeUnique<CFX_KeyValue>();
        pNewObjData->sKey = pOldObjData->sKey;
        pNewObjData->nType = pOldObjData->nType;
        pNewObjData->objData = pOldObjData->objData;
        Add(std::move(pNewObjData));
      } break;
      case JS_GlobalDataType::NULLOBJ: {
        auto pNewObjData = pdfium::MakeUnique<CFX_KeyValue>();
        pNewObjData->sKey = pOldObjData->sKey;
        pNewObjData->nType = pOldObjData->nType;
        Add(std::move(pNewObjData));
      } break;
    }
  }
  return *this;
}

void CFX_GlobalArray::Add(std::unique_ptr<CFX_KeyValue> pKeyValue) {
  m_Array.push_back(std::move(pKeyValue));
}

int CFX_GlobalArray::Count() const {
  return m_Array.size();
}

CFX_KeyValue* CFX_GlobalArray::GetAt(int index) const {
  return m_Array.at(index).get();
}
