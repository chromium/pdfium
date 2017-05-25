// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_OBJECT_H_
#define XFA_FXFA_PARSER_CXFA_OBJECT_H_

#include "core/fxcrt/fx_string.h"
#include "fxjs/fxjse.h"
#include "xfa/fxfa/fxfa_basic.h"

enum class XFA_ObjectType {
  Object,
  List,
  NodeList,
  Node,
  NodeC,
  NodeV,
  ModelNode,
  TextNode,
  ContainerNode,
  ContentNode,
  VariablesThis
};

class CFXJSE_Value;
class CXFA_Document;
class CXFA_Node;
class CXFA_NodeList;

class CXFA_Object : public CFXJSE_HostObject {
 public:
  CXFA_Object(CXFA_Document* pDocument,
              XFA_ObjectType objectType,
              XFA_Element eType,
              const CFX_WideStringC& elementName);
  ~CXFA_Object() override;

  CXFA_Document* GetDocument() const { return m_pDocument.Get(); }
  XFA_ObjectType GetObjectType() const { return m_objectType; }

  bool IsNode() const {
    return m_objectType == XFA_ObjectType::Node ||
           m_objectType == XFA_ObjectType::NodeC ||
           m_objectType == XFA_ObjectType::NodeV ||
           m_objectType == XFA_ObjectType::ModelNode ||
           m_objectType == XFA_ObjectType::TextNode ||
           m_objectType == XFA_ObjectType::ContainerNode ||
           m_objectType == XFA_ObjectType::ContentNode ||
           m_objectType == XFA_ObjectType::VariablesThis;
  }
  bool IsNodeList() const { return m_objectType == XFA_ObjectType::NodeList; }
  bool IsContentNode() const {
    return m_objectType == XFA_ObjectType::ContentNode;
  }
  bool IsContainerNode() const {
    return m_objectType == XFA_ObjectType::ContainerNode;
  }
  bool IsModelNode() const { return m_objectType == XFA_ObjectType::ModelNode; }
  bool IsNodeV() const { return m_objectType == XFA_ObjectType::NodeV; }
  bool IsVariablesThis() const {
    return m_objectType == XFA_ObjectType::VariablesThis;
  }

  CXFA_Node* AsNode();
  CXFA_NodeList* AsNodeList();

  const CXFA_Node* AsNode() const;
  const CXFA_NodeList* AsNodeList() const;

  XFA_Element GetElementType() const { return m_elementType; }
  CFX_WideStringC GetClassName() const { return m_elementName; }
  uint32_t GetClassHashCode() const { return m_elementNameHash; }

  void Script_ObjectClass_ClassName(CFXJSE_Value* pValue,
                                    bool bSetting,
                                    XFA_ATTRIBUTE eAttribute);

  void ThrowInvalidPropertyException() const;
  void ThrowArgumentMismatchException() const;
  void ThrowIndexOutOfBoundsException() const;
  void ThrowParamCountMismatchException(const CFX_WideString& method) const;

 protected:
  void ThrowException(const wchar_t* str, ...) const;

  CFX_UnownedPtr<CXFA_Document> const m_pDocument;
  const XFA_ObjectType m_objectType;
  const XFA_Element m_elementType;

  const uint32_t m_elementNameHash;
  const CFX_WideStringC m_elementName;
};

CXFA_Node* ToNode(CXFA_Object* pObj);
const CXFA_Node* ToNode(const CXFA_Object* pObj);

#endif  // XFA_FXFA_PARSER_CXFA_OBJECT_H_
