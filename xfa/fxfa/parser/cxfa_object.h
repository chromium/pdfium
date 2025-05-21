// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_OBJECT_H_
#define XFA_FXFA_PARSER_CXFA_OBJECT_H_

#include "core/fxcrt/fx_string.h"
#include "fxjs/gc/heap.h"
#include "fxjs/xfa/fxjse.h"
#include "v8/include/cppgc/garbage-collected.h"
#include "v8/include/cppgc/member.h"
#include "xfa/fxfa/fxfa_basic.h"

namespace cppgc {
class Visitor;
}  // namespace cppgc

enum class XFA_ObjectType {
  Object,
  List,
  Node,
  NodeC,
  NodeV,
  ModelNode,
  TextNode,
  TreeList,
  ContainerNode,
  ContentNode,
  ThisProxy,
};

class CJX_Object;
class CXFA_Document;
class CXFA_List;
class CXFA_Node;
class CXFA_ThisProxy;
class CXFA_TreeList;

class CXFA_Object : public cppgc::GarbageCollected<CXFA_Object> {
 public:
  virtual ~CXFA_Object();

  virtual void Trace(cppgc::Visitor* visitor) const;

  CXFA_Document* GetDocument() const { return document_.Get(); }
  XFA_ObjectType GetObjectType() const { return object_type_; }

  bool IsList() const {
    return object_type_ == XFA_ObjectType::List ||
           object_type_ == XFA_ObjectType::TreeList;
  }
  bool IsNode() const {
    return object_type_ == XFA_ObjectType::Node ||
           object_type_ == XFA_ObjectType::NodeC ||
           object_type_ == XFA_ObjectType::NodeV ||
           object_type_ == XFA_ObjectType::ModelNode ||
           object_type_ == XFA_ObjectType::TextNode ||
           object_type_ == XFA_ObjectType::ContainerNode ||
           object_type_ == XFA_ObjectType::ContentNode ||
           element_type_ == XFA_Element::Delta;
  }
  bool IsTreeList() const { return object_type_ == XFA_ObjectType::TreeList; }
  bool IsContentNode() const {
    return object_type_ == XFA_ObjectType::ContentNode;
  }
  bool IsContainerNode() const {
    return object_type_ == XFA_ObjectType::ContainerNode;
  }
  bool IsModelNode() const { return object_type_ == XFA_ObjectType::ModelNode; }
  bool IsNodeV() const { return object_type_ == XFA_ObjectType::NodeV; }
  bool IsThisProxy() const { return object_type_ == XFA_ObjectType::ThisProxy; }

  CXFA_List* AsList();
  CXFA_Node* AsNode();
  CXFA_TreeList* AsTreeList();
  CXFA_ThisProxy* AsThisProxy();

  CJX_Object* JSObject() { return jsobject_; }
  const CJX_Object* JSObject() const { return jsobject_; }

  bool HasCreatedUIWidget() const {
    return element_type_ == XFA_Element::Field ||
           element_type_ == XFA_Element::Draw ||
           element_type_ == XFA_Element::Subform ||
           element_type_ == XFA_Element::ExclGroup;
  }

  XFA_Element GetElementType() const { return element_type_; }
  ByteStringView GetClassName() const { return element_name_; }
  uint32_t GetClassHashCode() const { return element_name_hash_; }

  WideString GetSOMExpression();

 protected:
  CXFA_Object(CXFA_Document* document,
              XFA_ObjectType objectType,
              XFA_Element eType,
              CJX_Object* jsObject);

  const XFA_ObjectType object_type_;
  const XFA_Element element_type_;
  const ByteStringView element_name_;
  const uint32_t element_name_hash_;
  cppgc::WeakMember<CXFA_Document> document_;
  cppgc::Member<CJX_Object> jsobject_;
};

// Helper functions that permit nullptr arguments.
CXFA_List* ToList(CXFA_Object* pObj);
CXFA_Node* ToNode(CXFA_Object* pObj);
CXFA_TreeList* ToTreeList(CXFA_Object* pObj);
CXFA_ThisProxy* ToThisProxy(CXFA_Object* pObj);

#endif  // XFA_FXFA_PARSER_CXFA_OBJECT_H_
