// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_NODE_H_
#define XFA_FXFA_PARSER_CXFA_NODE_H_

#include <map>
#include <vector>

#include "core/fxcrt/fx_string.h"
#include "xfa/fxfa/parser/cxfa_object.h"

class CFX_XMLNode;
class CFXJSE_Argument;
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

enum XFA_SOM_MESSAGETYPE {
  XFA_SOM_ValidationMessage,
  XFA_SOM_FormatMessage,
  XFA_SOM_MandatoryMessage
};

enum XFA_NODEITEM {
  XFA_NODEITEM_Parent,
  XFA_NODEITEM_FirstChild,
  XFA_NODEITEM_NextSibling,
  XFA_NODEITEM_PrevSibling,
};

typedef void (*PD_CALLBACK_FREEDATA)(void* pData);
typedef void (*PD_CALLBACK_DUPLICATEDATA)(void*& pData);

struct XFA_MAPDATABLOCKCALLBACKINFO {
  PD_CALLBACK_FREEDATA pFree;
  PD_CALLBACK_DUPLICATEDATA pCopy;
};

struct XFA_MAPDATABLOCK {
  uint8_t* GetData() const { return (uint8_t*)this + sizeof(XFA_MAPDATABLOCK); }
  XFA_MAPDATABLOCKCALLBACKINFO* pCallbackInfo;
  int32_t iBytes;
};

struct XFA_MAPMODULEDATA {
  XFA_MAPMODULEDATA();
  ~XFA_MAPMODULEDATA();

  std::map<void*, void*> m_ValueMap;
  std::map<void*, XFA_MAPDATABLOCK*> m_BufferMap;
};

class CXFA_Node : public CXFA_Object {
 public:
  uint32_t GetPacketID() const { return m_ePacket; }

  void SetFlag(uint32_t dwFlag, bool bNotify);
  void ClearFlag(uint32_t dwFlag);

  bool IsInitialized() const { return HasFlag(XFA_NodeFlag_Initialized); }
  bool IsOwnXMLNode() const { return HasFlag(XFA_NodeFlag_OwnXMLNode); }
  bool IsUserInteractive() const {
    return HasFlag(XFA_NodeFlag_UserInteractive);
  }
  bool IsUnusedNode() const { return HasFlag(XFA_NodeFlag_UnusedNode); }
  bool IsLayoutGeneratedNode() const {
    return HasFlag(XFA_NodeFlag_LayoutGeneratedNode);
  }
  bool BindsFormItems() const { return HasFlag(XFA_NodeFlag_BindFormItems); }
  bool HasRemovedChildren() const {
    return HasFlag(XFA_NodeFlag_HasRemovedChildren);
  }
  bool NeedsInitApp() const { return HasFlag(XFA_NodeFlag_NeedsInitApp); }

  bool IsAttributeInXML();
  bool IsFormContainer() const {
    return m_ePacket == XFA_XDPPACKET_Form && IsContainerNode();
  }
  void SetXMLMappingNode(CFX_XMLNode* pXMLNode) { m_pXMLNode = pXMLNode; }
  CFX_XMLNode* GetXMLMappingNode() const { return m_pXMLNode; }
  CFX_XMLNode* CreateXMLMappingNode();
  bool IsNeedSavingXMLNode();
  uint32_t GetNameHash() const { return m_dwNameHash; }
  bool IsUnnamed() const { return m_dwNameHash == 0; }
  CXFA_Node* GetModelNode();
  void UpdateNameHash();
  bool HasAttribute(XFA_ATTRIBUTE eAttr, bool bCanInherit = false);
  bool SetAttribute(XFA_ATTRIBUTE eAttr,
                    const CFX_WideStringC& wsValue,
                    bool bNotify = false);
  bool GetAttribute(XFA_ATTRIBUTE eAttr,
                    CFX_WideString& wsValue,
                    bool bUseDefault = true);
  bool SetAttribute(const CFX_WideStringC& wsAttr,
                    const CFX_WideStringC& wsValue,
                    bool bNotify = false);
  bool GetAttribute(const CFX_WideStringC& wsAttr,
                    CFX_WideString& wsValue,
                    bool bUseDefault = true);
  bool RemoveAttribute(const CFX_WideStringC& wsAttr);
  bool SetContent(const CFX_WideString& wsContent,
                  const CFX_WideString& wsXMLValue,
                  bool bNotify = false,
                  bool bScriptModify = false,
                  bool bSyncData = true);
  bool TryContent(CFX_WideString& wsContent,
                  bool bScriptModify = false,
                  bool bProto = true);
  CFX_WideString GetContent();

  bool TryNamespace(CFX_WideString& wsNamespace);

  bool SetBoolean(XFA_ATTRIBUTE eAttr, bool bValue, bool bNotify = false) {
    return SetValue(eAttr, XFA_ATTRIBUTETYPE_Boolean, (void*)(uintptr_t)bValue,
                    bNotify);
  }
  bool TryBoolean(XFA_ATTRIBUTE eAttr, bool& bValue, bool bUseDefault = true);
  bool GetBoolean(XFA_ATTRIBUTE eAttr) {
    bool bValue;
    return TryBoolean(eAttr, bValue, true) ? bValue : false;
  }
  bool SetInteger(XFA_ATTRIBUTE eAttr, int32_t iValue, bool bNotify = false) {
    return SetValue(eAttr, XFA_ATTRIBUTETYPE_Integer, (void*)(uintptr_t)iValue,
                    bNotify);
  }
  bool TryInteger(XFA_ATTRIBUTE eAttr,
                  int32_t& iValue,
                  bool bUseDefault = true);
  int32_t GetInteger(XFA_ATTRIBUTE eAttr) {
    int32_t iValue;
    return TryInteger(eAttr, iValue, true) ? iValue : 0;
  }
  bool SetEnum(XFA_ATTRIBUTE eAttr,
               XFA_ATTRIBUTEENUM eValue,
               bool bNotify = false) {
    return SetValue(eAttr, XFA_ATTRIBUTETYPE_Enum, (void*)(uintptr_t)eValue,
                    bNotify);
  }
  bool TryEnum(XFA_ATTRIBUTE eAttr,
               XFA_ATTRIBUTEENUM& eValue,
               bool bUseDefault = true);
  XFA_ATTRIBUTEENUM GetEnum(XFA_ATTRIBUTE eAttr) {
    XFA_ATTRIBUTEENUM eValue;
    return TryEnum(eAttr, eValue, true) ? eValue : XFA_ATTRIBUTEENUM_Unknown;
  }
  bool SetCData(XFA_ATTRIBUTE eAttr,
                const CFX_WideString& wsValue,
                bool bNotify = false,
                bool bScriptModify = false);
  bool SetAttributeValue(const CFX_WideString& wsValue,
                         const CFX_WideString& wsXMLValue,
                         bool bNotify = false,
                         bool bScriptModify = false);
  bool TryCData(XFA_ATTRIBUTE eAttr,
                CFX_WideString& wsValue,
                bool bUseDefault = true,
                bool bProto = true);
  bool TryCData(XFA_ATTRIBUTE eAttr,
                CFX_WideStringC& wsValue,
                bool bUseDefault = true,
                bool bProto = true);
  CFX_WideStringC GetCData(XFA_ATTRIBUTE eAttr) {
    CFX_WideStringC wsValue;
    return TryCData(eAttr, wsValue) ? wsValue : CFX_WideStringC();
  }
  bool SetMeasure(XFA_ATTRIBUTE eAttr,
                  CXFA_Measurement mValue,
                  bool bNotify = false);
  bool TryMeasure(XFA_ATTRIBUTE eAttr,
                  CXFA_Measurement& mValue,
                  bool bUseDefault = true) const;
  CXFA_Measurement GetMeasure(XFA_ATTRIBUTE eAttr) const;
  bool SetObject(XFA_ATTRIBUTE eAttr,
                 void* pData,
                 XFA_MAPDATABLOCKCALLBACKINFO* pCallbackInfo = nullptr);
  bool TryObject(XFA_ATTRIBUTE eAttr, void*& pData);
  void* GetObject(XFA_ATTRIBUTE eAttr) {
    void* pData;
    return TryObject(eAttr, pData) ? pData : nullptr;
  }
  bool SetUserData(void* pKey,
                   void* pData,
                   XFA_MAPDATABLOCKCALLBACKINFO* pCallbackInfo = nullptr);
  bool TryUserData(void* pKey, void*& pData, bool bProtoAlso = false);
  void* GetUserData(void* pKey, bool bProtoAlso = false) {
    void* pData;
    return TryUserData(pKey, pData, bProtoAlso) ? pData : nullptr;
  }
  CXFA_Node* GetProperty(int32_t index,
                         XFA_Element eType,
                         bool bCreateProperty = true);
  int32_t CountChildren(XFA_Element eType, bool bOnlyChild = false);
  CXFA_Node* GetChild(int32_t index,
                      XFA_Element eType,
                      bool bOnlyChild = false);
  int32_t InsertChild(int32_t index, CXFA_Node* pNode);
  bool InsertChild(CXFA_Node* pNode, CXFA_Node* pBeforeNode = nullptr);
  bool RemoveChild(CXFA_Node* pNode, bool bNotify = true);
  CXFA_Node* Clone(bool bRecursive);
  CXFA_Node* GetNodeItem(XFA_NODEITEM eItem) const;
  CXFA_Node* GetNodeItem(XFA_NODEITEM eItem, XFA_ObjectType eType) const;
  std::vector<CXFA_Node*> GetNodeList(
      uint32_t dwTypeFilter = XFA_NODEFILTER_Children |
                              XFA_NODEFILTER_Properties,
      XFA_Element eTypeFilter = XFA_Element::Unknown);
  CXFA_Node* CreateSamePacketNode(XFA_Element eType,
                                  uint32_t dwFlags = XFA_NodeFlag_Initialized);
  CXFA_Node* CloneTemplateToForm(bool bRecursive);
  CXFA_Node* GetTemplateNode() const;
  void SetTemplateNode(CXFA_Node* pTemplateNode);
  CXFA_Node* GetDataDescriptionNode();
  void SetDataDescriptionNode(CXFA_Node* pDataDescriptionNode);
  CXFA_Node* GetBindData();
  std::vector<CXFA_Node*> GetBindItems();
  int32_t AddBindItem(CXFA_Node* pFormNode);
  int32_t RemoveBindItem(CXFA_Node* pFormNode);
  bool HasBindItem();
  CXFA_WidgetData* GetWidgetData();
  CXFA_WidgetData* GetContainerWidgetData();
  bool GetLocaleName(CFX_WideString& wsLocaleName);
  XFA_ATTRIBUTEENUM GetIntact();
  CXFA_Node* GetFirstChildByName(const CFX_WideStringC& wsNodeName) const;
  CXFA_Node* GetFirstChildByName(uint32_t dwNodeNameHash) const;
  CXFA_Node* GetFirstChildByClass(XFA_Element eType) const;
  CXFA_Node* GetNextSameNameSibling(uint32_t dwNodeNameHash) const;
  CXFA_Node* GetNextSameNameSibling(const CFX_WideStringC& wsNodeName) const;
  CXFA_Node* GetNextSameClassSibling(XFA_Element eType) const;
  int32_t GetNodeSameNameIndex() const;
  int32_t GetNodeSameClassIndex() const;
  void GetSOMExpression(CFX_WideString& wsSOMExpression);
  CXFA_Node* GetInstanceMgrOfSubform();

  CXFA_Node* GetOccurNode();
  void Script_TreeClass_ResolveNode(CFXJSE_Arguments* pArguments);
  void Script_TreeClass_ResolveNodes(CFXJSE_Arguments* pArguments);
  void Script_Som_ResolveNodeList(CFXJSE_Value* pValue,
                                  CFX_WideString wsExpression,
                                  uint32_t dwFlag,
                                  CXFA_Node* refNode = nullptr);
  void Script_TreeClass_All(CFXJSE_Value* pValue,
                            bool bSetting,
                            XFA_ATTRIBUTE eAttribute);
  void Script_TreeClass_Nodes(CFXJSE_Value* pValue,
                              bool bSetting,
                              XFA_ATTRIBUTE eAttribute);
  void Script_TreeClass_ClassAll(CFXJSE_Value* pValue,
                                 bool bSetting,
                                 XFA_ATTRIBUTE eAttribute);
  void Script_TreeClass_Parent(CFXJSE_Value* pValue,
                               bool bSetting,
                               XFA_ATTRIBUTE eAttribute);
  void Script_TreeClass_Index(CFXJSE_Value* pValue,
                              bool bSetting,
                              XFA_ATTRIBUTE eAttribute);
  void Script_TreeClass_ClassIndex(CFXJSE_Value* pValue,
                                   bool bSetting,
                                   XFA_ATTRIBUTE eAttribute);
  void Script_TreeClass_SomExpression(CFXJSE_Value* pValue,
                                      bool bSetting,
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
  void Script_NodeClass_Ns(CFXJSE_Value* pValue,
                           bool bSetting,
                           XFA_ATTRIBUTE eAttribute);
  void Script_NodeClass_Model(CFXJSE_Value* pValue,
                              bool bSetting,
                              XFA_ATTRIBUTE eAttribute);
  void Script_NodeClass_IsContainer(CFXJSE_Value* pValue,
                                    bool bSetting,
                                    XFA_ATTRIBUTE eAttribute);
  void Script_NodeClass_IsNull(CFXJSE_Value* pValue,
                               bool bSetting,
                               XFA_ATTRIBUTE eAttribute);
  void Script_NodeClass_OneOfChild(CFXJSE_Value* pValue,
                                   bool bSetting,
                                   XFA_ATTRIBUTE eAttribute);
  void Script_ContainerClass_GetDelta(CFXJSE_Arguments* pArguments);
  void Script_ContainerClass_GetDeltas(CFXJSE_Arguments* pArguments);
  void Script_ModelClass_ClearErrorList(CFXJSE_Arguments* pArguments);
  void Script_ModelClass_CreateNode(CFXJSE_Arguments* pArguments);
  void Script_ModelClass_IsCompatibleNS(CFXJSE_Arguments* pArguments);
  void Script_ModelClass_Context(CFXJSE_Value* pValue,
                                 bool bSetting,
                                 XFA_ATTRIBUTE eAttribute);
  void Script_ModelClass_AliasNode(CFXJSE_Value* pValue,
                                   bool bSetting,
                                   XFA_ATTRIBUTE eAttribute);
  void Script_WsdlConnection_Execute(CFXJSE_Arguments* pArguments);
  void Script_Delta_Restore(CFXJSE_Arguments* pArguments);
  void Script_Delta_CurrentValue(CFXJSE_Value* pValue,
                                 bool bSetting,
                                 XFA_ATTRIBUTE eAttribute);
  void Script_Delta_SavedValue(CFXJSE_Value* pValue,
                               bool bSetting,
                               XFA_ATTRIBUTE eAttribute);
  void Script_Delta_Target(CFXJSE_Value* pValue,
                           bool bSetting,
                           XFA_ATTRIBUTE eAttribute);
  void Script_Attribute_SendAttributeChangeMessage(XFA_ATTRIBUTE eAttribute,
                                                   bool bScriptModify);
  void Script_Attribute_Integer(CFXJSE_Value* pValue,
                                bool bSetting,
                                XFA_ATTRIBUTE eAttribute);
  void Script_Attribute_IntegerRead(CFXJSE_Value* pValue,
                                    bool bSetting,
                                    XFA_ATTRIBUTE eAttribute);
  void Script_Attribute_BOOL(CFXJSE_Value* pValue,
                             bool bSetting,
                             XFA_ATTRIBUTE eAttribute);
  void Script_Attribute_BOOLRead(CFXJSE_Value* pValue,
                                 bool bSetting,
                                 XFA_ATTRIBUTE eAttribute);
  void Script_Attribute_String(CFXJSE_Value* pValue,
                               bool bSetting,
                               XFA_ATTRIBUTE eAttribute);
  void Script_Attribute_StringRead(CFXJSE_Value* pValue,
                                   bool bSetting,
                                   XFA_ATTRIBUTE eAttribute);
  void Script_Som_ValidationMessage(CFXJSE_Value* pValue,
                                    bool bSetting,
                                    XFA_ATTRIBUTE eAttribute);
  void Script_Field_Length(CFXJSE_Value* pValue,
                           bool bSetting,
                           XFA_ATTRIBUTE eAttribute);
  void Script_Som_DefaultValue(CFXJSE_Value* pValue,
                               bool bSetting,
                               XFA_ATTRIBUTE eAttribute);
  void Script_Som_DefaultValue_Read(CFXJSE_Value* pValue,
                                    bool bSetting,
                                    XFA_ATTRIBUTE eAttribute);
  void Script_Boolean_Value(CFXJSE_Value* pValue,
                            bool bSetting,
                            XFA_ATTRIBUTE eAttribute);
  void Script_Som_Message(CFXJSE_Value* pValue,
                          bool bSetting,
                          XFA_SOM_MESSAGETYPE iMessageType);
  void Script_Som_BorderColor(CFXJSE_Value* pValue,
                              bool bSetting,
                              XFA_ATTRIBUTE eAttribute);
  void Script_Som_BorderWidth(CFXJSE_Value* pValue,
                              bool bSetting,
                              XFA_ATTRIBUTE eAttribute);
  void Script_Som_FillColor(CFXJSE_Value* pValue,
                            bool bSetting,
                            XFA_ATTRIBUTE eAttribute);
  void Script_Som_DataNode(CFXJSE_Value* pValue,
                           bool bSetting,
                           XFA_ATTRIBUTE eAttribute);
  void Script_Som_FontColor(CFXJSE_Value* pValue,
                            bool bSetting,
                            XFA_ATTRIBUTE eAttribute);
  void Script_Som_Mandatory(CFXJSE_Value* pValue,
                            bool bSetting,
                            XFA_ATTRIBUTE eAttribute);
  void Script_Som_MandatoryMessage(CFXJSE_Value* pValue,
                                   bool bSetting,
                                   XFA_ATTRIBUTE eAttribute);
  void Script_Som_InstanceIndex(CFXJSE_Value* pValue,
                                bool bSetting,
                                XFA_ATTRIBUTE eAttribute);
  void Script_Draw_DefaultValue(CFXJSE_Value* pValue,
                                bool bSetting,
                                XFA_ATTRIBUTE eAttribute);
  void Script_Field_DefaultValue(CFXJSE_Value* pValue,
                                 bool bSetting,
                                 XFA_ATTRIBUTE eAttribute);
  void Script_Field_EditValue(CFXJSE_Value* pValue,
                              bool bSetting,
                              XFA_ATTRIBUTE eAttribute);
  void Script_Field_FormatMessage(CFXJSE_Value* pValue,
                                  bool bSetting,
                                  XFA_ATTRIBUTE eAttribute);
  void Script_Field_FormattedValue(CFXJSE_Value* pValue,
                                   bool bSetting,
                                   XFA_ATTRIBUTE eAttribute);
  void Script_Field_ParentSubform(CFXJSE_Value* pValue,
                                  bool bSetting,
                                  XFA_ATTRIBUTE eAttribute);
  void Script_Field_SelectedIndex(CFXJSE_Value* pValue,
                                  bool bSetting,
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
  void Script_ExclGroup_DefaultAndRawValue(CFXJSE_Value* pValue,
                                           bool bSetting,
                                           XFA_ATTRIBUTE eAttribute);
  void Script_ExclGroup_ErrorText(CFXJSE_Value* pValue,
                                  bool bSetting,
                                  XFA_ATTRIBUTE eAttribute);
  void Script_ExclGroup_Transient(CFXJSE_Value* pValue,
                                  bool bSetting,
                                  XFA_ATTRIBUTE eAttribute);
  void Script_ExclGroup_ExecEvent(CFXJSE_Arguments* pArguments);
  void Script_ExclGroup_SelectedMember(CFXJSE_Arguments* pArguments);
  void Script_ExclGroup_ExecInitialize(CFXJSE_Arguments* pArguments);
  void Script_ExclGroup_ExecCalculate(CFXJSE_Arguments* pArguments);
  void Script_ExclGroup_ExecValidate(CFXJSE_Arguments* pArguments);
  void Script_Subform_InstanceManager(CFXJSE_Value* pValue,
                                      bool bSetting,
                                      XFA_ATTRIBUTE eAttribute);
  void Script_Subform_Locale(CFXJSE_Value* pValue,
                             bool bSetting,
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
  void Script_InstanceManager_Count(CFXJSE_Value* pValue,
                                    bool bSetting,
                                    XFA_ATTRIBUTE eAttribute);
  void Script_InstanceManager_Max(CFXJSE_Value* pValue,
                                  bool bSetting,
                                  XFA_ATTRIBUTE eAttribute);
  void Script_InstanceManager_Min(CFXJSE_Value* pValue,
                                  bool bSetting,
                                  XFA_ATTRIBUTE eAttribute);
  void Script_InstanceManager_MoveInstance(CFXJSE_Arguments* pArguments);
  void Script_InstanceManager_RemoveInstance(CFXJSE_Arguments* pArguments);
  void Script_InstanceManager_SetInstances(CFXJSE_Arguments* pArguments);
  void Script_InstanceManager_AddInstance(CFXJSE_Arguments* pArguments);
  void Script_InstanceManager_InsertInstance(CFXJSE_Arguments* pArguments);
  int32_t InstanceManager_SetInstances(int32_t iCount);
  int32_t InstanceManager_MoveInstance(int32_t iTo, int32_t iFrom);
  void Script_Occur_Max(CFXJSE_Value* pValue,
                        bool bSetting,
                        XFA_ATTRIBUTE eAttribute);
  void Script_Occur_Min(CFXJSE_Value* pValue,
                        bool bSetting,
                        XFA_ATTRIBUTE eAttribute);
  void Script_Desc_Metadata(CFXJSE_Arguments* pArguments);
  void Script_Form_FormNodes(CFXJSE_Arguments* pArguments);
  void Script_Form_Remerge(CFXJSE_Arguments* pArguments);
  void Script_Form_ExecInitialize(CFXJSE_Arguments* pArguments);
  void Script_Form_Recalculate(CFXJSE_Arguments* pArguments);
  void Script_Form_ExecCalculate(CFXJSE_Arguments* pArguments);
  void Script_Form_ExecValidate(CFXJSE_Arguments* pArguments);
  void Script_Form_Checksum(CFXJSE_Value* pValue,
                            bool bSetting,
                            XFA_ATTRIBUTE eAttribute);
  void Script_Packet_GetAttribute(CFXJSE_Arguments* pArguments);
  void Script_Packet_SetAttribute(CFXJSE_Arguments* pArguments);
  void Script_Packet_RemoveAttribute(CFXJSE_Arguments* pArguments);
  void Script_Packet_Content(CFXJSE_Value* pValue,
                             bool bSetting,
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
  void Script_Source_Db(CFXJSE_Value* pValue,
                        bool bSetting,
                        XFA_ATTRIBUTE eAttribute);
  void Script_Xfa_This(CFXJSE_Value* pValue,
                       bool bSetting,
                       XFA_ATTRIBUTE eAttribute);
  void Script_Handler_Version(CFXJSE_Value* pValue,
                              bool bSetting,
                              XFA_ATTRIBUTE eAttribute);
  void Script_SubmitFormat_Mode(CFXJSE_Value* pValue,
                                bool bSetting,
                                XFA_ATTRIBUTE eAttribute);
  void Script_Extras_Type(CFXJSE_Value* pValue,
                          bool bSetting,
                          XFA_ATTRIBUTE eAttribute);
  void Script_Encrypt_Format(CFXJSE_Value* pValue,
                             bool bSetting,
                             XFA_ATTRIBUTE eAttribute);
  void Script_Script_Stateless(CFXJSE_Value* pValue,
                               bool bSetting,
                               XFA_ATTRIBUTE eAttribute);

 private:
  friend class CXFA_Document;

  CXFA_Node(CXFA_Document* pDoc,
            uint16_t ePacket,
            XFA_ObjectType oType,
            XFA_Element eType,
            const CFX_WideStringC& elementName);
  ~CXFA_Node() override;

  bool HasFlag(XFA_NodeFlag dwFlag) const;
  CXFA_Node* Deprecated_GetPrevSibling();
  bool SetValue(XFA_ATTRIBUTE eAttr,
                XFA_ATTRIBUTETYPE eType,
                void* pValue,
                bool bNotify);
  bool GetValue(XFA_ATTRIBUTE eAttr,
                XFA_ATTRIBUTETYPE eType,
                bool bUseDefault,
                void*& pValue);
  void OnRemoved(bool bNotify);
  void OnChanging(XFA_ATTRIBUTE eAttr, bool bNotify);
  void OnChanged(XFA_ATTRIBUTE eAttr, bool bNotify, bool bScriptModify);
  int32_t execSingleEventByName(const CFX_WideStringC& wsEventName,
                                XFA_Element eType);
  bool SetScriptContent(const CFX_WideString& wsContent,
                        const CFX_WideString& wsXMLValue,
                        bool bNotify = true,
                        bool bScriptModify = false,
                        bool bSyncData = true);
  CFX_WideString GetScriptContent(bool bScriptModify = false);
  XFA_MAPMODULEDATA* CreateMapModuleData();
  XFA_MAPMODULEDATA* GetMapModuleData() const;
  void SetMapModuleValue(void* pKey, void* pValue);
  bool GetMapModuleValue(void* pKey, void*& pValue);
  void SetMapModuleString(void* pKey, const CFX_WideStringC& wsValue);
  bool GetMapModuleString(void* pKey, CFX_WideStringC& wsValue);
  void SetMapModuleBuffer(
      void* pKey,
      void* pValue,
      int32_t iBytes,
      XFA_MAPDATABLOCKCALLBACKINFO* pCallbackInfo = nullptr);
  bool GetMapModuleBuffer(void* pKey,
                          void*& pValue,
                          int32_t& iBytes,
                          bool bProtoAlso = true) const;
  bool HasMapModuleKey(void* pKey, bool bProtoAlso = false);
  void RemoveMapModuleKey(void* pKey = nullptr);
  void MergeAllData(void* pDstModule);
  void MoveBufferMapData(CXFA_Node* pDstModule, void* pKey);
  void MoveBufferMapData(CXFA_Node* pSrcModule,
                         CXFA_Node* pDstModule,
                         void* pKey,
                         bool bRecursive = false);

  CXFA_Node* m_pNext;
  CXFA_Node* m_pChild;
  CXFA_Node* m_pLastChild;
  CXFA_Node* m_pParent;
  CFX_XMLNode* m_pXMLNode;
  uint16_t m_ePacket;
  uint16_t m_uNodeFlags;
  uint32_t m_dwNameHash;
  CXFA_Node* m_pAuxNode;
  XFA_MAPMODULEDATA* m_pMapModuleData;

 private:
  void ThrowMissingPropertyException(const CFX_WideString& obj,
                                     const CFX_WideString& prop) const;
  void ThrowTooManyOccurancesException(const CFX_WideString& obj) const;
};

#endif  // XFA_FXFA_PARSER_CXFA_NODE_H_
