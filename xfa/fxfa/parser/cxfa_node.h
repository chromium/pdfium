// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_NODE_H_
#define XFA_FXFA_PARSER_CXFA_NODE_H_

#include <map>
#include <memory>
#include <utility>
#include <vector>

#include "core/fxcrt/fx_string.h"
#include "fxjs/cjx_node.h"
#include "third_party/base/optional.h"
#include "xfa/fxfa/parser/cxfa_object.h"

class CFX_XMLNode;
class CXFA_WidgetData;

#define XFA_NODEFILTER_Children 0x01
#define XFA_NODEFILTER_Properties 0x02
#define XFA_NODEFILTER_OneOfProperty 0x04

enum XFA_NodeFlag {
  XFA_NodeFlag_None = 0,
  XFA_NodeFlag_Initialized = 1 << 0,
  XFA_NodeFlag_HasRemovedChildren = 1 << 1,
  XFA_NodeFlag_NeedsInitApp = 1 << 2,
  XFA_NodeFlag_BindFormItems = 1 << 3,
  XFA_NodeFlag_UserInteractive = 1 << 4,
  XFA_NodeFlag_SkipDataBinding = 1 << 5,
  XFA_NodeFlag_OwnXMLNode = 1 << 6,
  XFA_NodeFlag_UnusedNode = 1 << 7,
  XFA_NodeFlag_LayoutGeneratedNode = 1 << 8
};

enum XFA_NODEITEM {
  XFA_NODEITEM_Parent,
  XFA_NODEITEM_FirstChild,
  XFA_NODEITEM_NextSibling,
  XFA_NODEITEM_PrevSibling,
};

class CXFA_Node : public CXFA_Object {
 public:
  struct PropertyData {
    XFA_Element property;
    uint8_t occurance_count;
    uint8_t flags;
  };

  struct AttributeData {
    XFA_Attribute attribute;
    XFA_AttributeType type;
    void* default_value;
  };

#ifndef NDEBUG
  static WideString ElementToName(XFA_Element elem);
#endif  // NDEBUG

  static WideString AttributeEnumToName(XFA_AttributeEnum item);
  static pdfium::Optional<XFA_AttributeEnum> NameToAttributeEnum(
      const WideStringView& name);
  static XFA_Attribute NameToAttribute(const WideStringView& name);
  static WideString AttributeToName(XFA_Attribute attr);
  static XFA_Element NameToElement(const WideString& name);
  static std::unique_ptr<CXFA_Node> Create(CXFA_Document* doc,
                                           XFA_Element element,
                                           XFA_PacketType packet);

  ~CXFA_Node() override;

  bool IsValidInPacket(XFA_PacketType packet) const;

  bool HasProperty(XFA_Element property) const;
  bool HasPropertyFlags(XFA_Element property, uint8_t flags) const;
  uint8_t PropertyOccuranceCount(XFA_Element property) const;

  bool HasAttribute(XFA_Attribute attr) const;
  XFA_Attribute GetAttribute(size_t i) const;
  XFA_AttributeType GetAttributeType(XFA_Attribute type) const;

  XFA_PacketType GetPacketType() const { return m_ePacket; }

  void SetFlag(uint32_t dwFlag, bool bNotify);
  void ClearFlag(uint32_t dwFlag);

  CJX_Node* JSNode() { return static_cast<CJX_Node*>(JSObject()); }
  const CJX_Node* JSNode() const {
    return static_cast<const CJX_Node*>(JSObject());
  }
  CXFA_Node* GetParent() { return m_pParent; }
  CXFA_Node* GetChildNode() { return m_pChild; }

  CXFA_Node* CreateInstance(bool bDataMerge);
  int32_t GetCount();
  CXFA_Node* GetItem(int32_t iIndex);
  void RemoveItem(CXFA_Node* pRemoveInstance, bool bRemoveDataBinding);
  void InsertItem(CXFA_Node* pNewInstance,
                  int32_t iPos,
                  int32_t iCount,
                  bool bMoveDataBindingNodes);

  bool IsInitialized() const { return HasFlag(XFA_NodeFlag_Initialized); }
  bool IsOwnXMLNode() const { return HasFlag(XFA_NodeFlag_OwnXMLNode); }
  bool IsUserInteractive() const {
    return HasFlag(XFA_NodeFlag_UserInteractive);
  }
  bool IsUnusedNode() const { return HasFlag(XFA_NodeFlag_UnusedNode); }
  bool IsLayoutGeneratedNode() const {
    return HasFlag(XFA_NodeFlag_LayoutGeneratedNode);
  }
  void ReleaseBindingNodes();
  bool BindsFormItems() const { return HasFlag(XFA_NodeFlag_BindFormItems); }
  bool HasRemovedChildren() const {
    return HasFlag(XFA_NodeFlag_HasRemovedChildren);
  }
  bool NeedsInitApp() const { return HasFlag(XFA_NodeFlag_NeedsInitApp); }

  bool IsAttributeInXML();
  bool IsFormContainer() const {
    return m_ePacket == XFA_PacketType::Form && IsContainerNode();
  }
  void SetXMLMappingNode(CFX_XMLNode* pXMLNode) { m_pXMLNode = pXMLNode; }
  CFX_XMLNode* GetXMLMappingNode() const { return m_pXMLNode; }
  CFX_XMLNode* CreateXMLMappingNode();
  bool IsNeedSavingXMLNode();
  uint32_t GetNameHash() const { return m_dwNameHash; }
  bool IsUnnamed() const { return m_dwNameHash == 0; }
  CXFA_Node* GetModelNode();
  void UpdateNameHash();

  int32_t CountChildren(XFA_Element eType, bool bOnlyChild);
  CXFA_Node* GetChild(int32_t index, XFA_Element eType, bool bOnlyChild);
  int32_t InsertChild(int32_t index, CXFA_Node* pNode);
  bool InsertChild(CXFA_Node* pNode, CXFA_Node* pBeforeNode);
  bool RemoveChild(CXFA_Node* pNode, bool bNotify);

  CXFA_Node* Clone(bool bRecursive);
  CXFA_Node* GetNodeItem(XFA_NODEITEM eItem) const;
  CXFA_Node* GetNodeItem(XFA_NODEITEM eItem, XFA_ObjectType eType) const;
  std::vector<CXFA_Node*> GetNodeList(uint32_t dwTypeFilter,
                                      XFA_Element eTypeFilter);
  CXFA_Node* CreateSamePacketNode(XFA_Element eType);
  CXFA_Node* CloneTemplateToForm(bool bRecursive);
  CXFA_Node* GetTemplateNode() const;
  void SetTemplateNode(CXFA_Node* pTemplateNode);
  CXFA_Node* GetDataDescriptionNode();
  void SetDataDescriptionNode(CXFA_Node* pDataDescriptionNode);
  CXFA_Node* GetBindData();
  std::vector<UnownedPtr<CXFA_Node>>* GetBindItems();
  int32_t AddBindItem(CXFA_Node* pFormNode);
  int32_t RemoveBindItem(CXFA_Node* pFormNode);
  bool HasBindItem();
  CXFA_WidgetData* GetWidgetData();
  CXFA_WidgetData* GetContainerWidgetData();
  bool GetLocaleName(WideString& wsLocaleName);
  XFA_AttributeEnum GetIntact();
  CXFA_Node* GetFirstChildByName(const WideStringView& wsNodeName) const;
  CXFA_Node* GetFirstChildByName(uint32_t dwNodeNameHash) const;
  CXFA_Node* GetFirstChildByClass(XFA_Element eType) const;
  CXFA_Node* GetNextSameNameSibling(uint32_t dwNodeNameHash) const;
  CXFA_Node* GetNextSameNameSibling(const WideStringView& wsNodeName) const;
  CXFA_Node* GetNextSameClassSibling(XFA_Element eType) const;
  int32_t GetNodeSameNameIndex() const;
  int32_t GetNodeSameClassIndex() const;
  void GetSOMExpression(WideString& wsSOMExpression);
  CXFA_Node* GetInstanceMgrOfSubform();

  CXFA_Node* GetOccurNode();

  pdfium::Optional<bool> GetDefaultBoolean(XFA_Attribute attr) const;
  pdfium::Optional<int32_t> GetDefaultInteger(XFA_Attribute attr) const;
  pdfium::Optional<CXFA_Measurement> GetDefaultMeasurement(
      XFA_Attribute attr) const;
  pdfium::Optional<WideString> GetDefaultCData(XFA_Attribute attr) const;
  pdfium::Optional<XFA_AttributeEnum> GetDefaultEnum(XFA_Attribute attr) const;

 protected:
  CXFA_Node(CXFA_Document* pDoc,
            XFA_PacketType ePacket,
            uint32_t validPackets,
            XFA_ObjectType oType,
            XFA_Element eType,
            const PropertyData* properties,
            const AttributeData* attributes,
            const WideStringView& elementName);

 private:
  bool HasFlag(XFA_NodeFlag dwFlag) const;
  CXFA_Node* Deprecated_GetPrevSibling();
  const PropertyData* GetPropertyData(XFA_Element property) const;
  const AttributeData* GetAttributeData(XFA_Attribute attr) const;
  pdfium::Optional<XFA_Element> GetFirstPropertyWithFlag(uint8_t flag);
  void OnRemoved(bool bNotify);
  pdfium::Optional<void*> GetDefaultValue(XFA_Attribute attr,
                                          XFA_AttributeType eType) const;

  const PropertyData* const m_Properties;
  const AttributeData* const m_Attributes;
  const uint32_t m_ValidPackets;
  CXFA_Node* m_pNext;
  CXFA_Node* m_pChild;
  CXFA_Node* m_pLastChild;
  CXFA_Node* m_pParent;
  CFX_XMLNode* m_pXMLNode;
  const XFA_PacketType m_ePacket;
  uint16_t m_uNodeFlags;
  uint32_t m_dwNameHash;
  CXFA_Node* m_pAuxNode;
};

#endif  // XFA_FXFA_PARSER_CXFA_NODE_H_
