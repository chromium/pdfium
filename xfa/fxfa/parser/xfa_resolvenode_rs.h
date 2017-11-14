// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_XFA_RESOLVENODE_RS_H_
#define XFA_FXFA_PARSER_XFA_RESOLVENODE_RS_H_

#include <memory>
#include <utility>
#include <vector>

#include "fxjs/cfxjse_value.h"
#include "third_party/base/ptr_util.h"
#include "xfa/fxfa/fxfa.h"
#include "xfa/fxfa/parser/cxfa_object.h"
#include "xfa/fxfa/parser/cxfa_valuearray.h"

#define XFA_RESOLVENODE_Children 0x0001
#define XFA_RESOLVENODE_Attributes 0x0004
#define XFA_RESOLVENODE_Properties 0x0008
#define XFA_RESOLVENODE_Siblings 0x0020
#define XFA_RESOLVENODE_Parent 0x0040
#define XFA_RESOLVENODE_AnyChild 0x0080
#define XFA_RESOLVENODE_ALL 0x0100
#define XFA_RESOLVENODE_CreateNode 0x0400
#define XFA_RESOLVENODE_Bind 0x0800
#define XFA_RESOLVENODE_BindNew 0x1000

enum XFA_RESOLVENODE_RSTYPE {
  XFA_RESOLVENODE_RSTYPE_Nodes,
  XFA_RESOLVENODE_RSTYPE_Attribute,
  XFA_RESOLVENODE_RSTYPE_CreateNodeOne,
  XFA_RESOLVENODE_RSTYPE_CreateNodeAll,
  XFA_RESOLVENODE_RSTYPE_CreateNodeMidAll,
  XFA_RESOLVENODE_RSTYPE_ExistNodes,
};

struct XFA_RESOLVENODE_RS {
  XFA_RESOLVENODE_RS();
  ~XFA_RESOLVENODE_RS();

  size_t GetAttributeResult(CXFA_ValueArray* valueArray) const {
    if (pScriptAttribute &&
        pScriptAttribute->eValueType == XFA_ScriptType::Object) {
      for (CXFA_Object* pObject : objects) {
        auto pValue = pdfium::MakeUnique<CFXJSE_Value>(valueArray->m_pIsolate);
        CJX_Object* jsObject = pObject->JSObject();
        (jsObject->*(pScriptAttribute->callback))(pValue.get(), false,
                                                  pScriptAttribute->attribute);
        valueArray->m_Values.push_back(std::move(pValue));
      }
    }
    return valueArray->m_Values.size();
  }

  std::vector<CXFA_Object*> objects;  // Not owned.
  XFA_RESOLVENODE_RSTYPE dwFlags;
  const XFA_SCRIPTATTRIBUTEINFO* pScriptAttribute;
};

inline XFA_RESOLVENODE_RS::XFA_RESOLVENODE_RS()
    : dwFlags(XFA_RESOLVENODE_RSTYPE_Nodes), pScriptAttribute(nullptr) {}

inline XFA_RESOLVENODE_RS::~XFA_RESOLVENODE_RS() {}

#endif  // XFA_FXFA_PARSER_XFA_RESOLVENODE_RS_H_
