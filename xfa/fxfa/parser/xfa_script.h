// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_XFA_SCRIPT_H_
#define XFA_FXFA_PARSER_XFA_SCRIPT_H_

#include "xfa/fxfa/include/fxfa.h"

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

enum XFA_SCRIPTLANGTYPE {
  XFA_SCRIPTLANGTYPE_Formcalc = XFA_SCRIPTTYPE_Formcalc,
  XFA_SCRIPTLANGTYPE_Javascript = XFA_SCRIPTTYPE_Javascript,
  XFA_SCRIPTLANGTYPE_Unkown = XFA_SCRIPTTYPE_Unkown,
};

enum XFA_RESOVENODE_RSTYPE {
  XFA_RESOVENODE_RSTYPE_Nodes,
  XFA_RESOVENODE_RSTYPE_Attribute,
  XFA_RESOLVENODE_RSTYPE_CreateNodeOne,
  XFA_RESOLVENODE_RSTYPE_CreateNodeAll,
  XFA_RESOLVENODE_RSTYPE_CreateNodeMidAll,
  XFA_RESOVENODE_RSTYPE_ExistNodes,
};

class CXFA_HVALUEArray : public CFX_ArrayTemplate<FXJSE_HVALUE> {
 public:
  CXFA_HVALUEArray(FXJSE_HRUNTIME hRunTime) : m_hRunTime(hRunTime) {}
  ~CXFA_HVALUEArray() {
    for (int32_t i = 0; i < GetSize(); i++) {
      FXJSE_Value_Release(GetAt(i));
    }
  }
  void GetAttributeObject(CXFA_ObjArray& objArray) {
    for (int32_t i = 0; i < GetSize(); i++) {
      CXFA_Object* pObject = (CXFA_Object*)FXJSE_Value_ToObject(GetAt(i), NULL);
      objArray.Add(pObject);
    }
  }
  FXJSE_HRUNTIME m_hRunTime;
};

struct XFA_RESOLVENODE_RS {
  XFA_RESOLVENODE_RS()
      : dwFlags(XFA_RESOVENODE_RSTYPE_Nodes), pScriptAttribute(NULL) {}
  ~XFA_RESOLVENODE_RS() { nodes.RemoveAll(); }
  int32_t GetAttributeResult(CXFA_HVALUEArray& hValueArray) const {
    if (pScriptAttribute && pScriptAttribute->eValueType == XFA_SCRIPT_Object) {
      FXJSE_HRUNTIME hRunTime = hValueArray.m_hRunTime;
      for (int32_t i = 0; i < nodes.GetSize(); i++) {
        FXJSE_HVALUE hValue = FXJSE_Value_Create(hRunTime);
        (nodes[i]->*(pScriptAttribute->lpfnCallback))(
            hValue, FALSE, (XFA_ATTRIBUTE)pScriptAttribute->eAttribute);
        hValueArray.Add(hValue);
      }
    }
    return hValueArray.GetSize();
  }

  CXFA_ObjArray nodes;
  XFA_RESOVENODE_RSTYPE dwFlags;
  const XFA_SCRIPTATTRIBUTEINFO* pScriptAttribute;
};

#endif  // XFA_FXFA_PARSER_XFA_SCRIPT_H_
