// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _XFA_OBJECT_H_
#define _XFA_OBJECT_H_
class CXFA_Document;
class IXFA_ObjFactory;
class IXFA_Notify;
class CXFA_Object;
class CXFA_OrdinaryObject;
class CXFA_Node;
class CXFA_NodeList;
class CXFA_ArrayNodeList;
class CXFA_AttachNodeList;
enum XFA_OBJECTTYPE {
  XFA_OBJECTTYPE_OrdinaryObject = 0x0,
  XFA_OBJECTTYPE_OrdinaryList = 0x1,
  XFA_OBJECTTYPE_NodeList = 0x2,
  XFA_OBJECTTYPE_Node = 0x4,
  XFA_OBJECTTYPE_NodeC = 0x5,
  XFA_OBJECTTYPE_NodeV = 0x6,
  XFA_OBJECTTYPE_ModelNode = 0x8,
  XFA_OBJECTTYPE_TextNode = 0x9,
  XFA_OBJECTTYPE_ContainerNode = 0xA,
  XFA_OBJECTTYPE_ContentNode = 0xB,
  XFA_OBJECTTYPE_VariablesThis = 0xC,
  XFA_OBJECTTYPEMASK = 0xF,
  XFA_NODEFLAG_Initialized = 0x00020,
  XFA_NODEFLAG_HasRemoved = 0x00200,
  XFA_NODEFLAG_NeedsInitApp = 0x00400,
  XFA_NODEFLAG_BindFormItems = 0x00800,
  XFA_NODEFLAG_UserInteractive = 0x01000,
  XFA_NODEFLAG_SkipDataBinding = 0x02000,
  XFA_NODEFLAG_OwnXMLNode = 0x04000,
  XFA_NODEFLAG_UnusedNode = 0x08000,
  XFA_NODEFLAG_LayoutGeneratedNode = 0x10000,
};
class CXFA_Object {
 public:
  CXFA_Object(CXFA_Document* pDocument, FX_DWORD uFlags);
  inline CXFA_Document* GetDocument() const { return m_pDocument; }
  inline FX_DWORD GetFlag() const { return m_uFlags; }
  inline XFA_OBJECTTYPE GetObjectType() const {
    return (XFA_OBJECTTYPE)(m_uFlags & XFA_OBJECTTYPEMASK);
  }
  inline FX_BOOL IsNode() const {
    return (m_uFlags & XFA_OBJECTTYPEMASK) >= XFA_OBJECTTYPE_Node;
  }
  inline FX_BOOL IsOrdinaryObject() const {
    return (m_uFlags & XFA_OBJECTTYPEMASK) == XFA_OBJECTTYPE_OrdinaryObject;
  }
  inline FX_BOOL IsNodeList() const {
    return (m_uFlags & XFA_OBJECTTYPEMASK) == XFA_OBJECTTYPE_NodeList;
  }
  inline FX_BOOL IsOrdinaryList() const {
    return (m_uFlags & XFA_OBJECTTYPEMASK) == XFA_OBJECTTYPE_OrdinaryList;
  }
  FX_BOOL IsContentNode() const {
    return (m_uFlags & XFA_OBJECTTYPEMASK) == XFA_OBJECTTYPE_ContentNode;
  }
  FX_BOOL IsContainerNode() const {
    return (m_uFlags & XFA_OBJECTTYPEMASK) == XFA_OBJECTTYPE_ContainerNode;
  }
  XFA_ELEMENT GetClassID() const;
  void GetClassName(CFX_WideStringC& wsName) const;
  uint32_t GetClassHashCode() const;
  void Script_ObjectClass_ClassName(FXJSE_HVALUE hValue,
                                    FX_BOOL bSetting,
                                    XFA_ATTRIBUTE eAttribute);
  void ThrowScriptErrorMessage(int32_t iStringID, ...);

 protected:
  CXFA_Document* m_pDocument;
  FX_DWORD m_uFlags;
};
#define XFA_NODEFILTER_Children 0x01
#define XFA_NODEFILTER_Properties 0x02
#define XFA_NODEFILTER_OneOfProperty 0x04
#define XFA_CLONEFLAG_Content 0x01
enum XFA_NODEITEM {
  XFA_NODEITEM_Parent,
  XFA_NODEITEM_FirstChild,
  XFA_NODEITEM_NextSibling,
  XFA_NODEITEM_PrevSibling,
};
enum XFA_SOM_MESSAGETYPE {
  XFA_SOM_ValidationMessage,
  XFA_SOM_FormatMessage,
  XFA_SOM_MandatoryMessage
};
typedef CFX_StackTemplate<CXFA_Node*> CXFA_NodeStack;
typedef CXFA_PtrSetTemplate<CXFA_Node*> CXFA_NodeSet;
typedef void (*PD_CALLBACK_DUPLICATEDATA)(void*& pData);
typedef struct _XFA_MAPDATABLOCKCALLBACKINFO {
  PD_CALLBACK_FREEDATA pFree;
  PD_CALLBACK_DUPLICATEDATA pCopy;
} XFA_MAPDATABLOCKCALLBACKINFO;
typedef struct _XFA_MAPDATABLOCK {
  uint8_t* GetData() const {
    return (uint8_t*)this + sizeof(_XFA_MAPDATABLOCK);
  }
  XFA_MAPDATABLOCKCALLBACKINFO* pCallbackInfo;
  int32_t iBytes;
} XFA_MAPDATABLOCK, *XFA_LPMAPDATABLOCK;
typedef struct _XFA_MAPMODULEDATA {
  CFX_MapPtrToPtr m_ValueMap;
  CFX_MapPtrTemplate<void*, XFA_LPMAPDATABLOCK> m_BufferMap;
} XFA_MAPMODULEDATA, *XFA_LPMAPMODULEDATA;
#define XFA_CalcRefCount (void*)(uintptr_t) FXBSTR_ID('X', 'F', 'A', 'R')
#define XFA_CalcData (void*)(uintptr_t) FXBSTR_ID('X', 'F', 'A', 'C')
#define XFA_LAYOUTITEMKEY (void*)(uintptr_t) FXBSTR_ID('L', 'Y', 'I', 'M')
class CXFA_Node : public CXFA_Object {
 public:
  XFA_ELEMENT GetClassID() const { return (XFA_ELEMENT)m_eNodeClass; }
  FX_DWORD GetPacketID() const { return m_ePacket; }
  FX_BOOL HasFlag(FX_DWORD dwFlag) const;
  void SetFlag(FX_DWORD dwFlag, FX_BOOL bOn = TRUE, FX_BOOL bNotify = TRUE);
  FX_BOOL IsAttributeInXML();
  FX_BOOL IsFormContainer() {
    return m_ePacket == XFA_XDPPACKET_Form && IsContainerNode();
  }
  void SetXMLMappingNode(IFDE_XMLNode* pXMLNode) { m_pXMLNode = pXMLNode; }
  IFDE_XMLNode* GetXMLMappingNode() const { return m_pXMLNode; }
  IFDE_XMLNode* CreateXMLMappingNode();
  FX_BOOL IsNeedSavingXMLNode();
  inline FX_DWORD GetNameHash() const { return m_dwNameHash; }
  inline FX_BOOL IsUnnamed() const { return m_dwNameHash == 0; }
  CXFA_Node* GetModelNode();
  void UpdateNameHash();
  FX_BOOL HasAttribute(XFA_ATTRIBUTE eAttr, FX_BOOL bCanInherit = FALSE);
  FX_BOOL SetAttribute(XFA_ATTRIBUTE eAttr,
                       const CFX_WideStringC& wsValue,
                       FX_BOOL bNotify = FALSE);
  FX_BOOL GetAttribute(XFA_ATTRIBUTE eAttr,
                       CFX_WideString& wsValue,
                       FX_BOOL bUseDefault = TRUE);
  FX_BOOL SetAttribute(const CFX_WideStringC& wsAttr,
                       const CFX_WideStringC& wsValue,
                       FX_BOOL bNotify = FALSE);
  FX_BOOL GetAttribute(const CFX_WideStringC& wsAttr,
                       CFX_WideString& wsValue,
                       FX_BOOL bUseDefault = TRUE);
  FX_BOOL RemoveAttribute(const CFX_WideStringC& wsAttr);
  FX_BOOL SetContent(const CFX_WideString& wsContent,
                     const CFX_WideString& wsXMLValue,
                     FX_BOOL bNotify = FALSE,
                     FX_BOOL bScriptModify = FALSE,
                     FX_BOOL bSyncData = TRUE);
  FX_BOOL TryContent(CFX_WideString& wsContent,
                     FX_BOOL bScriptModify = FALSE,
                     FX_BOOL bProto = TRUE);
  CFX_WideString GetContent();

  FX_BOOL TryNamespace(CFX_WideString& wsNamespace);

  FX_BOOL SetBoolean(XFA_ATTRIBUTE eAttr,
                     FX_BOOL bValue,
                     FX_BOOL bNotify = FALSE) {
    return SetValue(eAttr, XFA_ATTRIBUTETYPE_Boolean, (void*)(uintptr_t)bValue,
                    bNotify);
  }
  FX_BOOL TryBoolean(XFA_ATTRIBUTE eAttr,
                     FX_BOOL& bValue,
                     FX_BOOL bUseDefault = TRUE);
  FX_BOOL GetBoolean(XFA_ATTRIBUTE eAttr) {
    FX_BOOL bValue;
    return TryBoolean(eAttr, bValue, TRUE) ? bValue : FALSE;
  }
  FX_BOOL SetInteger(XFA_ATTRIBUTE eAttr,
                     int32_t iValue,
                     FX_BOOL bNotify = FALSE) {
    return SetValue(eAttr, XFA_ATTRIBUTETYPE_Integer, (void*)(uintptr_t)iValue,
                    bNotify);
  }
  FX_BOOL TryInteger(XFA_ATTRIBUTE eAttr,
                     int32_t& iValue,
                     FX_BOOL bUseDefault = TRUE);
  int32_t GetInteger(XFA_ATTRIBUTE eAttr) {
    int32_t iValue;
    return TryInteger(eAttr, iValue, TRUE) ? iValue : 0;
  }
  FX_BOOL SetEnum(XFA_ATTRIBUTE eAttr,
                  XFA_ATTRIBUTEENUM eValue,
                  FX_BOOL bNotify = FALSE) {
    return SetValue(eAttr, XFA_ATTRIBUTETYPE_Enum, (void*)(uintptr_t)eValue,
                    bNotify);
  }
  FX_BOOL TryEnum(XFA_ATTRIBUTE eAttr,
                  XFA_ATTRIBUTEENUM& eValue,
                  FX_BOOL bUseDefault = TRUE);
  XFA_ATTRIBUTEENUM GetEnum(XFA_ATTRIBUTE eAttr) {
    XFA_ATTRIBUTEENUM eValue;
    return TryEnum(eAttr, eValue, TRUE) ? eValue : XFA_ATTRIBUTEENUM_Unknown;
  }
  FX_BOOL SetCData(XFA_ATTRIBUTE eAttr,
                   const CFX_WideString& wsValue,
                   FX_BOOL bNotify = FALSE,
                   FX_BOOL bScriptModify = FALSE);
  FX_BOOL SetAttributeValue(const CFX_WideString& wsValue,
                            const CFX_WideString& wsXMLValue,
                            FX_BOOL bNotify = FALSE,
                            FX_BOOL bScriptModify = FALSE);
  FX_BOOL TryCData(XFA_ATTRIBUTE eAttr,
                   CFX_WideString& wsValue,
                   FX_BOOL bUseDefault = TRUE,
                   FX_BOOL bProto = TRUE);
  FX_BOOL TryCData(XFA_ATTRIBUTE eAttr,
                   CFX_WideStringC& wsValue,
                   FX_BOOL bUseDefault = TRUE,
                   FX_BOOL bProto = TRUE);
  CFX_WideStringC GetCData(XFA_ATTRIBUTE eAttr) {
    CFX_WideStringC wsValue;
    return TryCData(eAttr, wsValue) ? wsValue : CFX_WideStringC();
  }
  FX_BOOL SetMeasure(XFA_ATTRIBUTE eAttr,
                     CXFA_Measurement mValue,
                     FX_BOOL bNotify = FALSE);
  FX_BOOL TryMeasure(XFA_ATTRIBUTE eAttr,
                     CXFA_Measurement& mValue,
                     FX_BOOL bUseDefault = TRUE);
  CXFA_Measurement GetMeasure(XFA_ATTRIBUTE eAttr) {
    CXFA_Measurement mValue;
    return TryMeasure(eAttr, mValue, TRUE) ? mValue : CXFA_Measurement();
  }
  FX_BOOL SetObject(XFA_ATTRIBUTE eAttr,
                    void* pData,
                    XFA_MAPDATABLOCKCALLBACKINFO* pCallbackInfo = NULL);
  FX_BOOL TryObject(XFA_ATTRIBUTE eAttr, void*& pData);
  void* GetObject(XFA_ATTRIBUTE eAttr) {
    void* pData;
    return TryObject(eAttr, pData) ? pData : NULL;
  }
  FX_BOOL SetUserData(void* pKey,
                      void* pData,
                      XFA_MAPDATABLOCKCALLBACKINFO* pCallbackInfo = NULL);
  FX_BOOL TryUserData(void* pKey, void*& pData, FX_BOOL bProtoAlso = FALSE);
  void* GetUserData(void* pKey, FX_BOOL bProtoAlso = FALSE) {
    void* pData;
    return TryUserData(pKey, pData, bProtoAlso) ? pData : NULL;
  }
  CXFA_Node* GetProperty(int32_t index,
                         XFA_ELEMENT eProperty,
                         FX_BOOL bCreateProperty = TRUE);
  int32_t CountChildren(XFA_ELEMENT eElement, FX_BOOL bOnlyChild = FALSE);
  CXFA_Node* GetChild(int32_t index,
                      XFA_ELEMENT eElement,
                      FX_BOOL bOnlyChild = FALSE);
  int32_t InsertChild(int32_t index, CXFA_Node* pNode);
  FX_BOOL InsertChild(CXFA_Node* pNode, CXFA_Node* pBeforeNode = NULL);
  FX_BOOL RemoveChild(CXFA_Node* pNode, FX_BOOL bNotify = TRUE);
  CXFA_Node* Clone(FX_BOOL bRecursive);
  CXFA_Node* GetNodeItem(XFA_NODEITEM eItem) const;
  CXFA_Node* GetNodeItem(XFA_NODEITEM eItem, XFA_OBJECTTYPE eType) const;
  int32_t GetNodeList(CXFA_NodeArray& nodes,
                      FX_DWORD dwTypeFilter = XFA_NODEFILTER_Children |
                                              XFA_NODEFILTER_Properties,
                      XFA_ELEMENT eElementFilter = XFA_ELEMENT_UNKNOWN,
                      int32_t iLevel = 1);
  CXFA_Node* CreateSamePacketNode(XFA_ELEMENT eElement,
                                  FX_DWORD dwFlags = XFA_NODEFLAG_Initialized);
  CXFA_Node* CloneTemplateToForm(FX_BOOL bRecursive);
  CXFA_Node* GetTemplateNode();
  void SetTemplateNode(CXFA_Node* pTemplateNode);
  CXFA_Node* GetDataDescriptionNode();
  void SetDataDescriptionNode(CXFA_Node* pDataDescriptionNode);
  CXFA_Node* GetBindData();
  int32_t GetBindItems(CXFA_NodeArray& formItems);
  int32_t AddBindItem(CXFA_Node* pFormNode);
  int32_t RemoveBindItem(CXFA_Node* pFormNode);
  FX_BOOL HasBindItem();
  CXFA_WidgetData* GetWidgetData();
  CXFA_WidgetData* GetContainerWidgetData();
  FX_BOOL GetLocaleName(CFX_WideString& wsLocaleName);
  XFA_ATTRIBUTEENUM GetIntact();
  CXFA_Node* GetFirstChildByName(const CFX_WideStringC& wsNodeName) const;
  CXFA_Node* GetFirstChildByName(FX_DWORD dwNodeNameHash) const;
  CXFA_Node* GetFirstChildByClass(XFA_ELEMENT eNodeClass) const;
  CXFA_Node* GetNextSameNameSibling(FX_DWORD dwNodeNameHash) const;
  CXFA_Node* GetNextSameNameSibling(const CFX_WideStringC& wsNodeName) const;
  CXFA_Node* GetNextSameClassSibling(XFA_ELEMENT eNodeClass) const;
  int32_t GetNodeSameNameIndex() const;
  int32_t GetNodeSameClassIndex() const;
  void GetSOMExpression(CFX_WideString& wsSOMExpression);
  CXFA_Node* GetInstanceMgrOfSubform();

  CXFA_Node* GetOccurNode();
  void Script_TreeClass_ResolveNode(CFXJSE_Arguments* pArguments);
  void Script_TreeClass_ResolveNodes(CFXJSE_Arguments* pArguments);
  void Script_Som_ResolveNodeList(FXJSE_HVALUE hValue,
                                  CFX_WideString wsExpression,
                                  FX_DWORD dwFlag,
                                  CXFA_Node* refNode = NULL);
  void Script_TreeClass_All(FXJSE_HVALUE hValue,
                            FX_BOOL bSetting,
                            XFA_ATTRIBUTE eAttribute);
  void Script_TreeClass_Nodes(FXJSE_HVALUE hValue,
                              FX_BOOL bSetting,
                              XFA_ATTRIBUTE eAttribute);
  void Script_TreeClass_ClassAll(FXJSE_HVALUE hValue,
                                 FX_BOOL bSetting,
                                 XFA_ATTRIBUTE eAttribute);
  void Script_TreeClass_Parent(FXJSE_HVALUE hValue,
                               FX_BOOL bSetting,
                               XFA_ATTRIBUTE eAttribute);
  void Script_TreeClass_Index(FXJSE_HVALUE hValue,
                              FX_BOOL bSetting,
                              XFA_ATTRIBUTE eAttribute);
  void Script_TreeClass_ClassIndex(FXJSE_HVALUE hValue,
                                   FX_BOOL bSetting,
                                   XFA_ATTRIBUTE eAttribute);
  void Script_TreeClass_SomExpression(FXJSE_HVALUE hValue,
                                      FX_BOOL bSetting,
                                      XFA_ATTRIBUTE eAttribute);
  void Script_NodeClass_ApplyXSL(CFXJSE_Arguments* pArguments);
  void Script_NodeClass_AssignNode(CFXJSE_Arguments* pArguments);
  void Script_NodeClass_Clone(CFXJSE_Arguments* pArguments);
  void Script_NodeClass_GetAttribute(CFXJSE_Arguments* pArguments);
  void Script_NodeClass_GetElement(CFXJSE_Arguments* pArguments);
  void Script_NodeClass_IsPropertySpecified(CFXJSE_Arguments* pArguments);
  void Script_NodeClass_LoadXML(CFXJSE_Arguments* pArguments);
  void Script_NodeClass_SaveFilteredXML(CFXJSE_Arguments* pArguments);
  void Script_NodeClass_SaveXML(CFXJSE_Arguments* pArguments);
  void Script_NodeClass_SetAttribute(CFXJSE_Arguments* pArguments);
  void Script_NodeClass_SetElement(CFXJSE_Arguments* pArguments);
  void Script_NodeClass_Ns(FXJSE_HVALUE hValue,
                           FX_BOOL bSetting,
                           XFA_ATTRIBUTE eAttribute);
  void Script_NodeClass_Model(FXJSE_HVALUE hValue,
                              FX_BOOL bSetting,
                              XFA_ATTRIBUTE eAttribute);
  void Script_NodeClass_IsContainer(FXJSE_HVALUE hValue,
                                    FX_BOOL bSetting,
                                    XFA_ATTRIBUTE eAttribute);
  void Script_NodeClass_IsNull(FXJSE_HVALUE hValue,
                               FX_BOOL bSetting,
                               XFA_ATTRIBUTE eAttribute);
  void Script_NodeClass_OneOfChild(FXJSE_HVALUE hValue,
                                   FX_BOOL bSetting,
                                   XFA_ATTRIBUTE eAttribute);
  void Script_ContainerClass_GetDelta(CFXJSE_Arguments* pArguments);
  void Script_ContainerClass_GetDeltas(CFXJSE_Arguments* pArguments);
  void Script_ModelClass_ClearErrorList(CFXJSE_Arguments* pArguments);
  void Script_ModelClass_CreateNode(CFXJSE_Arguments* pArguments);
  void Script_ModelClass_IsCompatibleNS(CFXJSE_Arguments* pArguments);
  void Script_ModelClass_Context(FXJSE_HVALUE hValue,
                                 FX_BOOL bSetting,
                                 XFA_ATTRIBUTE eAttribute);
  void Script_ModelClass_AliasNode(FXJSE_HVALUE hValue,
                                   FX_BOOL bSetting,
                                   XFA_ATTRIBUTE eAttribute);
  void Script_WsdlConnection_Execute(CFXJSE_Arguments* pArguments);
  void Script_Delta_Restore(CFXJSE_Arguments* pArguments);
  void Script_Delta_CurrentValue(FXJSE_HVALUE hValue,
                                 FX_BOOL bSetting,
                                 XFA_ATTRIBUTE eAttribute);
  void Script_Delta_SavedValue(FXJSE_HVALUE hValue,
                               FX_BOOL bSetting,
                               XFA_ATTRIBUTE eAttribute);
  void Script_Delta_Target(FXJSE_HVALUE hValue,
                           FX_BOOL bSetting,
                           XFA_ATTRIBUTE eAttribute);
  void Script_Attribute_SendAttributeChangeMessage(void* eAttribute,
                                                   void* eValue,
                                                   FX_BOOL bScriptModify);
  void Script_Attribute_Integer(FXJSE_HVALUE hValue,
                                FX_BOOL bSetting,
                                XFA_ATTRIBUTE eAttribute);
  void Script_Attribute_IntegerRead(FXJSE_HVALUE hValue,
                                    FX_BOOL bSetting,
                                    XFA_ATTRIBUTE eAttribute);
  void Script_Attribute_BOOL(FXJSE_HVALUE hValue,
                             FX_BOOL bSetting,
                             XFA_ATTRIBUTE eAttribute);
  void Script_Attribute_BOOLRead(FXJSE_HVALUE hValue,
                                 FX_BOOL bSetting,
                                 XFA_ATTRIBUTE eAttribute);
  void Script_Attribute_String(FXJSE_HVALUE hValue,
                               FX_BOOL bSetting,
                               XFA_ATTRIBUTE eAttribute);
  void Script_Attribute_StringRead(FXJSE_HVALUE hValue,
                                   FX_BOOL bSetting,
                                   XFA_ATTRIBUTE eAttribute);
  void Script_Som_ValidationMessage(FXJSE_HVALUE hValue,
                                    FX_BOOL bSetting,
                                    XFA_ATTRIBUTE eAttribute);
  void Script_Field_Length(FXJSE_HVALUE hValue,
                           FX_BOOL bSetting,
                           XFA_ATTRIBUTE eAttribute);
  void Script_Som_DefaultValue(FXJSE_HVALUE hValue,
                               FX_BOOL bSetting,
                               XFA_ATTRIBUTE eAttribute);
  void Script_Som_DefaultValue_Read(FXJSE_HVALUE hValue,
                                    FX_BOOL bSetting,
                                    XFA_ATTRIBUTE eAttribute);
  void Script_Boolean_Value(FXJSE_HVALUE hValue,
                            FX_BOOL bSetting,
                            XFA_ATTRIBUTE eAttribute);
  void Script_Som_Message(FXJSE_HVALUE hValue,
                          FX_BOOL bSetting,
                          XFA_SOM_MESSAGETYPE iMessageType);
  void Script_Som_BorderColor(FXJSE_HVALUE hValue,
                              FX_BOOL bSetting,
                              XFA_ATTRIBUTE eAttribute);
  void Script_Som_BorderWidth(FXJSE_HVALUE hValue,
                              FX_BOOL bSetting,
                              XFA_ATTRIBUTE eAttribute);
  void Script_Som_FillColor(FXJSE_HVALUE hValue,
                            FX_BOOL bSetting,
                            XFA_ATTRIBUTE eAttribute);
  void Script_Som_DataNode(FXJSE_HVALUE hValue,
                           FX_BOOL bSetting,
                           XFA_ATTRIBUTE eAttribute);
  void Script_Som_FontColor(FXJSE_HVALUE hValue,
                            FX_BOOL bSetting,
                            XFA_ATTRIBUTE eAttribute);
  void Script_Som_Mandatory(FXJSE_HVALUE hValue,
                            FX_BOOL bSetting,
                            XFA_ATTRIBUTE eAttribute);
  void Script_Som_MandatoryMessage(FXJSE_HVALUE hValue,
                                   FX_BOOL bSetting,
                                   XFA_ATTRIBUTE eAttribute);
  void Script_Som_InstanceIndex(FXJSE_HVALUE hValue,
                                FX_BOOL bSetting,
                                XFA_ATTRIBUTE eAttribute);
  void Script_Draw_DefaultValue(FXJSE_HVALUE hValue,
                                FX_BOOL bSetting,
                                XFA_ATTRIBUTE eAttribute);
  void Script_Field_DefaultValue(FXJSE_HVALUE hValue,
                                 FX_BOOL bSetting,
                                 XFA_ATTRIBUTE eAttribute);
  void Script_Field_EditValue(FXJSE_HVALUE hValue,
                              FX_BOOL bSetting,
                              XFA_ATTRIBUTE eAttribute);
  void Script_Field_FormatMessage(FXJSE_HVALUE hValue,
                                  FX_BOOL bSetting,
                                  XFA_ATTRIBUTE eAttribute);
  void Script_Field_FormattedValue(FXJSE_HVALUE hValue,
                                   FX_BOOL bSetting,
                                   XFA_ATTRIBUTE eAttribute);
  void Script_Field_ParentSubform(FXJSE_HVALUE hValue,
                                  FX_BOOL bSetting,
                                  XFA_ATTRIBUTE eAttribute);
  void Script_Field_SelectedIndex(FXJSE_HVALUE hValue,
                                  FX_BOOL bSetting,
                                  XFA_ATTRIBUTE eAttribute);
  void Script_Field_ClearItems(CFXJSE_Arguments* pArguments);
  void Script_Field_ExecEvent(CFXJSE_Arguments* pArguments);
  void Script_Field_ExecInitialize(CFXJSE_Arguments* pArguments);
  void Script_Field_DeleteItem(CFXJSE_Arguments* pArguments);
  void Script_Field_GetSaveItem(CFXJSE_Arguments* pArguments);
  void Script_Field_BoundItem(CFXJSE_Arguments* pArguments);
  void Script_Field_GetItemState(CFXJSE_Arguments* pArguments);
  void Script_Field_ExecCalculate(CFXJSE_Arguments* pArguments);
  void Script_Field_SetItems(CFXJSE_Arguments* pArguments);
  void Script_Field_GetDisplayItem(CFXJSE_Arguments* pArguments);
  void Script_Field_SetItemState(CFXJSE_Arguments* pArguments);
  void Script_Field_AddItem(CFXJSE_Arguments* pArguments);
  void Script_Field_ExecValidate(CFXJSE_Arguments* pArguments);
  void Script_ExclGroup_DefaultAndRawValue(FXJSE_HVALUE hValue,
                                           FX_BOOL bSetting,
                                           XFA_ATTRIBUTE eAttribute);
  void Script_ExclGroup_ErrorText(FXJSE_HVALUE hValue,
                                  FX_BOOL bSetting,
                                  XFA_ATTRIBUTE eAttribute);
  void Script_ExclGroup_Transient(FXJSE_HVALUE hValue,
                                  FX_BOOL bSetting,
                                  XFA_ATTRIBUTE eAttribute);
  void Script_ExclGroup_ExecEvent(CFXJSE_Arguments* pArguments);
  void Script_ExclGroup_SelectedMember(CFXJSE_Arguments* pArguments);
  void Script_ExclGroup_ExecInitialize(CFXJSE_Arguments* pArguments);
  void Script_ExclGroup_ExecCalculate(CFXJSE_Arguments* pArguments);
  void Script_ExclGroup_ExecValidate(CFXJSE_Arguments* pArguments);
  void Script_Subform_InstanceManager(FXJSE_HVALUE hValue,
                                      FX_BOOL bSetting,
                                      XFA_ATTRIBUTE eAttribute);
  void Script_Subform_Locale(FXJSE_HVALUE hValue,
                             FX_BOOL bSetting,
                             XFA_ATTRIBUTE eAttribute);
  void Script_Subform_ExecEvent(CFXJSE_Arguments* pArguments);
  void Script_Subform_ExecInitialize(CFXJSE_Arguments* pArguments);
  void Script_Subform_ExecCalculate(CFXJSE_Arguments* pArguments);
  void Script_Subform_ExecValidate(CFXJSE_Arguments* pArguments);
  void Script_Subform_GetInvalidObjects(CFXJSE_Arguments* pArguments);

  int32_t Subform_and_SubformSet_InstanceIndex();
  void Script_Template_FormNodes(CFXJSE_Arguments* pArguments);
  void Script_Template_Remerge(CFXJSE_Arguments* pArguments);
  void Script_Template_ExecInitialize(CFXJSE_Arguments* pArguments);
  void Script_Template_CreateNode(CFXJSE_Arguments* pArguments);
  void Script_Template_Recalculate(CFXJSE_Arguments* pArguments);
  void Script_Template_ExecCalculate(CFXJSE_Arguments* pArguments);
  void Script_Template_ExecValidate(CFXJSE_Arguments* pArguments);
  void Script_Manifest_Evaluate(CFXJSE_Arguments* pArguments);
  void Script_InstanceManager_Count(FXJSE_HVALUE hValue,
                                    FX_BOOL bSetting,
                                    XFA_ATTRIBUTE eAttribute);
  void Script_InstanceManager_Max(FXJSE_HVALUE hValue,
                                  FX_BOOL bSetting,
                                  XFA_ATTRIBUTE eAttribute);
  void Script_InstanceManager_Min(FXJSE_HVALUE hValue,
                                  FX_BOOL bSetting,
                                  XFA_ATTRIBUTE eAttribute);
  void Script_InstanceManager_MoveInstance(CFXJSE_Arguments* pArguments);
  void Script_InstanceManager_RemoveInstance(CFXJSE_Arguments* pArguments);
  void Script_InstanceManager_SetInstances(CFXJSE_Arguments* pArguments);
  void Script_InstanceManager_AddInstance(CFXJSE_Arguments* pArguments);
  void Script_InstanceManager_InsertInstance(CFXJSE_Arguments* pArguments);
  int32_t InstanceManager_SetInstances(int32_t iCount);
  int32_t InstanceManager_MoveInstance(int32_t iTo, int32_t iFrom);
  void Script_Occur_Max(FXJSE_HVALUE hValue,
                        FX_BOOL bSetting,
                        XFA_ATTRIBUTE eAttribute);
  void Script_Occur_Min(FXJSE_HVALUE hValue,
                        FX_BOOL bSetting,
                        XFA_ATTRIBUTE eAttribute);
  void Script_Desc_Metadata(CFXJSE_Arguments* pArguments);
  void Script_Form_FormNodes(CFXJSE_Arguments* pArguments);
  void Script_Form_Remerge(CFXJSE_Arguments* pArguments);
  void Script_Form_ExecInitialize(CFXJSE_Arguments* pArguments);
  void Script_Form_Recalculate(CFXJSE_Arguments* pArguments);
  void Script_Form_ExecCalculate(CFXJSE_Arguments* pArguments);
  void Script_Form_ExecValidate(CFXJSE_Arguments* pArguments);
  void Script_Form_Checksum(FXJSE_HVALUE hValue,
                            FX_BOOL bSetting,
                            XFA_ATTRIBUTE eAttribute);
  void Script_Packet_GetAttribute(CFXJSE_Arguments* pArguments);
  void Script_Packet_SetAttribute(CFXJSE_Arguments* pArguments);
  void Script_Packet_RemoveAttribute(CFXJSE_Arguments* pArguments);
  void Script_Packet_Content(FXJSE_HVALUE hValue,
                             FX_BOOL bSetting,
                             XFA_ATTRIBUTE eAttribute);
  void Script_Source_Next(CFXJSE_Arguments* pArguments);
  void Script_Source_CancelBatch(CFXJSE_Arguments* pArguments);
  void Script_Source_First(CFXJSE_Arguments* pArguments);
  void Script_Source_UpdateBatch(CFXJSE_Arguments* pArguments);
  void Script_Source_Previous(CFXJSE_Arguments* pArguments);
  void Script_Source_IsBOF(CFXJSE_Arguments* pArguments);
  void Script_Source_IsEOF(CFXJSE_Arguments* pArguments);
  void Script_Source_Cancel(CFXJSE_Arguments* pArguments);
  void Script_Source_Update(CFXJSE_Arguments* pArguments);
  void Script_Source_Open(CFXJSE_Arguments* pArguments);
  void Script_Source_Delete(CFXJSE_Arguments* pArguments);
  void Script_Source_AddNew(CFXJSE_Arguments* pArguments);
  void Script_Source_Requery(CFXJSE_Arguments* pArguments);
  void Script_Source_Resync(CFXJSE_Arguments* pArguments);
  void Script_Source_Close(CFXJSE_Arguments* pArguments);
  void Script_Source_Last(CFXJSE_Arguments* pArguments);
  void Script_Source_HasDataChanged(CFXJSE_Arguments* pArguments);
  void Script_Source_Db(FXJSE_HVALUE hValue,
                        FX_BOOL bSetting,
                        XFA_ATTRIBUTE eAttribute);
  void Script_Xfa_This(FXJSE_HVALUE hValue,
                       FX_BOOL bSetting,
                       XFA_ATTRIBUTE eAttribute);
  void Script_Handler_Version(FXJSE_HVALUE hValue,
                              FX_BOOL bSetting,
                              XFA_ATTRIBUTE eAttribute);
  void Script_SubmitFormat_Mode(FXJSE_HVALUE hValue,
                                FX_BOOL bSetting,
                                XFA_ATTRIBUTE eAttribute);
  void Script_Extras_Type(FXJSE_HVALUE hValue,
                          FX_BOOL bSetting,
                          XFA_ATTRIBUTE eAttribute);
  void Script_Encrypt_Format(FXJSE_HVALUE hValue,
                             FX_BOOL bSetting,
                             XFA_ATTRIBUTE eAttribute);
  void Script_Script_Stateless(FXJSE_HVALUE hValue,
                               FX_BOOL bSetting,
                               XFA_ATTRIBUTE eAttribute);

 protected:
  CXFA_Node(CXFA_Document* pDoc, FX_WORD ePacket, XFA_ELEMENT eElement);
  ~CXFA_Node();
  friend class CXFA_Document;
  CXFA_Node* Deprecated_GetPrevSibling();
  FX_BOOL SetValue(XFA_ATTRIBUTE eAttr,
                   XFA_ATTRIBUTETYPE eType,
                   void* pValue,
                   FX_BOOL bNotify);
  FX_BOOL GetValue(XFA_ATTRIBUTE eAttr,
                   XFA_ATTRIBUTETYPE eType,
                   FX_BOOL bUseDefault,
                   void*& pValue);
  void OnRemoved(CXFA_Node* pParent, CXFA_Node* pRemoved, FX_BOOL bNotify);
  void OnChanging(XFA_ATTRIBUTE eAttr, void* pNewValue, FX_BOOL bNotify);
  void OnChanged(XFA_ATTRIBUTE eAttr,
                 void* pNewValue,
                 FX_BOOL bNotify,
                 FX_BOOL bScriptModify = FALSE);
  int32_t execSingleEventByName(const CFX_WideStringC& wsEventName,
                                XFA_ELEMENT eElementType);
  FX_BOOL SetScriptContent(const CFX_WideString& wsContent,
                           const CFX_WideString& wsXMLValue,
                           FX_BOOL bNotify = TRUE,
                           FX_BOOL bScriptModify = FALSE,
                           FX_BOOL bSyncData = TRUE);
  CFX_WideString GetScriptContent(FX_BOOL bScriptModify = FALSE);
  XFA_LPMAPMODULEDATA GetMapModuleData(FX_BOOL bCreateNew);
  void SetMapModuleValue(void* pKey, void* pValue);
  FX_BOOL GetMapModuleValue(void* pKey, void*& pValue);
  void SetMapModuleString(void* pKey, const CFX_WideStringC& wsValue);
  FX_BOOL GetMapModuleString(void* pKey, CFX_WideStringC& wsValue);
  void SetMapModuleBuffer(void* pKey,
                          void* pValue,
                          int32_t iBytes,
                          XFA_MAPDATABLOCKCALLBACKINFO* pCallbackInfo = NULL);
  FX_BOOL GetMapModuleBuffer(void* pKey,
                             void*& pValue,
                             int32_t& iBytes,
                             FX_BOOL bProtoAlso = TRUE);
  FX_BOOL HasMapModuleKey(void* pKey, FX_BOOL bProtoAlso = FALSE);
  void RemoveMapModuleKey(void* pKey = NULL);
  void MergeAllData(void* pDstModule, FX_BOOL bUseSrcAttr = TRUE);
  void MoveBufferMapData(CXFA_Node* pDstModule, void* pKey);
  void MoveBufferMapData(CXFA_Node* pSrcModule,
                         CXFA_Node* pDstModule,
                         void* pKey,
                         FX_BOOL bRecursive = FALSE);

  CXFA_Node* m_pNext;
  CXFA_Node* m_pChild;
  CXFA_Node* m_pLastChild;
  CXFA_Node* m_pParent;
  IFDE_XMLNode* m_pXMLNode;
  XFA_ELEMENT m_eNodeClass;
  FX_WORD m_ePacket;
  FX_DWORD m_dwNameHash;
  CXFA_Node* m_pAuxNode;
  XFA_LPMAPMODULEDATA m_pMapModuleData;
};
class CXFA_OrdinaryObject : public CXFA_Object {
 public:
  CXFA_OrdinaryObject(CXFA_Document* pDocument, XFA_ELEMENT eElement)
      : CXFA_Object(pDocument, XFA_OBJECTTYPE_OrdinaryObject),
        m_uScriptHash(0) {
    m_eNodeClass = eElement;
  }
  XFA_ELEMENT GetClassID() const { return (XFA_ELEMENT)m_eNodeClass; }
  uint32_t GetScriptObjHash() { return m_uScriptHash; }

 protected:
  XFA_ELEMENT m_eNodeClass;
  uint32_t m_uScriptHash;
};
class CXFA_ThisProxy : public CXFA_Object {
 public:
  CXFA_ThisProxy(CXFA_Node* pThisNode, CXFA_Node* pScriptNode)
      : CXFA_Object(pThisNode->GetDocument(), XFA_OBJECTTYPE_VariablesThis),
        m_pThisNode(NULL),
        m_pScriptNode(NULL) {
    m_pThisNode = pThisNode;
    m_pScriptNode = pScriptNode;
  }
  virtual ~CXFA_ThisProxy() {}
  CXFA_Node* GetThisNode() { return m_pThisNode; }
  CXFA_Node* GetScriptNode() { return m_pScriptNode; }

 private:
  CXFA_Node* m_pThisNode;
  CXFA_Node* m_pScriptNode;
};
class CXFA_NodeList : public CXFA_Object {
 public:
  CXFA_NodeList(CXFA_Document* pDocument);
  virtual ~CXFA_NodeList() {}
  XFA_ELEMENT GetClassID() const { return XFA_ELEMENT_NodeList; }
  CXFA_Node* NamedItem(const CFX_WideStringC& wsName);
  virtual int32_t GetLength() = 0;
  virtual FX_BOOL Append(CXFA_Node* pNode) = 0;
  virtual FX_BOOL Insert(CXFA_Node* pNewNode, CXFA_Node* pBeforeNode) = 0;
  virtual FX_BOOL Remove(CXFA_Node* pNode) = 0;
  virtual CXFA_Node* Item(int32_t iIndex) = 0;

  void Script_ListClass_Append(CFXJSE_Arguments* pArguments);
  void Script_ListClass_Insert(CFXJSE_Arguments* pArguments);
  void Script_ListClass_Remove(CFXJSE_Arguments* pArguments);
  void Script_ListClass_Item(CFXJSE_Arguments* pArguments);

  void Script_TreelistClass_NamedItem(CFXJSE_Arguments* pArguments);
  void Script_ListClass_Length(FXJSE_HVALUE hValue,
                               FX_BOOL bSetting,
                               XFA_ATTRIBUTE eAttribute);
};
class CXFA_ArrayNodeList : public CXFA_NodeList {
 public:
  CXFA_ArrayNodeList(CXFA_Document* pDocument);
  void SetArrayNodeList(const CXFA_NodeArray& srcArray);
  virtual int32_t GetLength();
  virtual FX_BOOL Append(CXFA_Node* pNode);
  virtual FX_BOOL Insert(CXFA_Node* pNewNode, CXFA_Node* pBeforeNode);
  virtual FX_BOOL Remove(CXFA_Node* pNode);
  virtual CXFA_Node* Item(int32_t iIndex);

 protected:
  CXFA_NodeArray m_array;
};
class CXFA_AttachNodeList : public CXFA_NodeList {
 public:
  CXFA_AttachNodeList(CXFA_Document* pDocument, CXFA_Node* pAttachNode);

  virtual int32_t GetLength();
  virtual FX_BOOL Append(CXFA_Node* pNode);
  virtual FX_BOOL Insert(CXFA_Node* pNewNode, CXFA_Node* pBeforeNode);
  virtual FX_BOOL Remove(CXFA_Node* pNode);
  virtual CXFA_Node* Item(int32_t iIndex);

 protected:
  CXFA_Node* m_pAttachNode;
};
class CXFA_TraverseStrategy_XFAContainerNode {
 public:
  static CXFA_Node* GetFirstChild(CXFA_Node* pTemplateNode,
                                  void* pUserData = NULL) {
    return pTemplateNode->GetNodeItem(XFA_NODEITEM_FirstChild,
                                      XFA_OBJECTTYPE_ContainerNode);
  }
  static CXFA_Node* GetNextSibling(CXFA_Node* pTemplateNode,
                                   void* pUserData = NULL) {
    return pTemplateNode->GetNodeItem(XFA_NODEITEM_NextSibling,
                                      XFA_OBJECTTYPE_ContainerNode);
  }
  static CXFA_Node* GetParent(CXFA_Node* pTemplateNode,
                              void* pUserData = NULL) {
    return pTemplateNode->GetNodeItem(XFA_NODEITEM_Parent,
                                      XFA_OBJECTTYPE_ContainerNode);
  }
};
typedef CXFA_NodeIteratorTemplate<CXFA_Node,
                                  CXFA_TraverseStrategy_XFAContainerNode>
    CXFA_ContainerIterator;
class CXFA_TraverseStrategy_XFANode {
 public:
  static inline CXFA_Node* GetFirstChild(CXFA_Node* pTemplateNode) {
    return pTemplateNode->GetNodeItem(XFA_NODEITEM_FirstChild);
  }
  static inline CXFA_Node* GetNextSibling(CXFA_Node* pTemplateNode) {
    return pTemplateNode->GetNodeItem(XFA_NODEITEM_NextSibling);
  }
  static inline CXFA_Node* GetParent(CXFA_Node* pTemplateNode) {
    return pTemplateNode->GetNodeItem(XFA_NODEITEM_Parent);
  }
};
typedef CXFA_NodeIteratorTemplate<CXFA_Node, CXFA_TraverseStrategy_XFANode>
    CXFA_NodeIterator;
#endif
