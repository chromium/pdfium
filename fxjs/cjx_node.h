// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_CJX_NODE_H_
#define FXJS_CJX_NODE_H_

#include <memory>
#include <utility>
#include <vector>

#include "core/fxcrt/unowned_ptr.h"
#include "fxjs/cjx_object.h"
#include "xfa/fxfa/fxfa_basic.h"

typedef void (*PD_CALLBACK_FREEDATA)(void* pData);
typedef void (*PD_CALLBACK_DUPLICATEDATA)(void*& pData);

struct XFA_MAPDATABLOCKCALLBACKINFO {
  PD_CALLBACK_FREEDATA pFree;
  PD_CALLBACK_DUPLICATEDATA pCopy;
};

enum XFA_SOM_MESSAGETYPE {
  XFA_SOM_ValidationMessage,
  XFA_SOM_FormatMessage,
  XFA_SOM_MandatoryMessage
};

class CFX_XMLElement;
class CFXJSE_Arguments;
class CXFA_CalcData;
class CXFA_LayoutItem;
class CXFA_Node;
class CXFA_WidgetData;

struct XFA_MAPMODULEDATA;

class CJX_Node : public CJX_Object {
 public:
  explicit CJX_Node(CXFA_Node* node);
  ~CJX_Node() override;

  CXFA_Node* GetXFANode();
  const CXFA_Node* GetXFANode() const;

  bool HasAttribute(XFA_Attribute eAttr);
  bool SetAttribute(XFA_Attribute eAttr,
                    const WideStringView& wsValue,
                    bool bNotify);
  bool SetAttribute(const WideStringView& wsAttr,
                    const WideStringView& wsValue,
                    bool bNotify);
  WideString GetAttribute(const WideStringView& attr);
  WideString GetAttribute(XFA_Attribute attr);
  pdfium::Optional<WideString> TryAttribute(const WideStringView& wsAttr,
                                            bool bUseDefault);
  pdfium::Optional<WideString> TryAttribute(XFA_Attribute eAttr,
                                            bool bUseDefault);

  void SetAttributeValue(const WideString& wsValue,
                         const WideString& wsXMLValue,
                         bool bNotify,
                         bool bScriptModify);
  void RemoveAttribute(const WideStringView& wsAttr);

  CXFA_Node* GetProperty(int32_t index,
                         XFA_Element eType,
                         bool bCreateProperty);

  pdfium::Optional<WideString> TryContent(bool bScriptModify, bool bProto);
  bool SetContent(const WideString& wsContent,
                  const WideString& wsXMLValue,
                  bool bNotify,
                  bool bScriptModify,
                  bool bSyncData);
  WideString GetContent(bool bScriptModify);

  pdfium::Optional<int32_t> TryInteger(XFA_Attribute eAttr, bool bUseDefault);
  bool SetInteger(XFA_Attribute eAttr, int32_t iValue, bool bNotify);
  int32_t GetInteger(XFA_Attribute eAttr);

  pdfium::Optional<WideString> TryCData(XFA_Attribute eAttr, bool bUseDefault);
  bool SetCData(XFA_Attribute eAttr,
                const WideString& wsValue,
                bool bNotify,
                bool bScriptModify);
  WideString GetCData(XFA_Attribute eAttr);

  pdfium::Optional<XFA_AttributeEnum> TryEnum(XFA_Attribute eAttr,
                                              bool bUseDefault);
  bool SetEnum(XFA_Attribute eAttr, XFA_AttributeEnum eValue, bool bNotify);
  XFA_AttributeEnum GetEnum(XFA_Attribute eAttr);

  pdfium::Optional<bool> TryBoolean(XFA_Attribute eAttr, bool bUseDefault);
  bool SetBoolean(XFA_Attribute eAttr, bool bValue, bool bNotify);
  bool GetBoolean(XFA_Attribute eAttr);

  pdfium::Optional<CXFA_Measurement> TryMeasure(XFA_Attribute eAttr,
                                                bool bUseDefault) const;
  bool SetMeasure(XFA_Attribute eAttr, CXFA_Measurement mValue, bool bNotify);
  CXFA_Measurement GetMeasure(XFA_Attribute eAttr) const;

  void SetBindingNodes(std::vector<UnownedPtr<CXFA_Node>> nodes) {
    binding_nodes_ = std::move(nodes);
  }
  std::vector<UnownedPtr<CXFA_Node>>* GetBindingNodes() {
    return &binding_nodes_;
  }
  void ReleaseBindingNodes();

  void SetBindingNode(CXFA_Node* node) {
    binding_nodes_.clear();
    if (node)
      binding_nodes_.emplace_back(node);
  }
  CXFA_Node* GetBindingNode() const {
    if (binding_nodes_.empty())
      return nullptr;
    return binding_nodes_[0].Get();
  }

  void SetWidgetData(std::unique_ptr<CXFA_WidgetData> data);
  CXFA_WidgetData* GetWidgetData() const { return widget_data_.get(); }

  void SetLayoutItem(CXFA_LayoutItem* item) { layout_item_ = item; }
  CXFA_LayoutItem* GetLayoutItem() const { return layout_item_.Get(); }

  void SetCalcData(std::unique_ptr<CXFA_CalcData> data);
  CXFA_CalcData* GetCalcData() const { return calc_data_.get(); }
  std::unique_ptr<CXFA_CalcData> ReleaseCalcData();

  void SetCalcRecursionCount(size_t count) { calc_recursion_count_ = count; }
  size_t GetCalcRecursionCount() const { return calc_recursion_count_; }

  pdfium::Optional<WideString> TryNamespace();

  void MergeAllData(CXFA_Node* pDstModule);

  void ThrowMissingPropertyException(const WideString& obj,
                                     const WideString& prop) const;
  void ThrowTooManyOccurancesException(const WideString& obj) const;

  int32_t Subform_and_SubformSet_InstanceIndex();
  int32_t InstanceManager_SetInstances(int32_t iDesired);
  int32_t InstanceManager_MoveInstance(int32_t iTo, int32_t iFrom);

  void Script_TreeClass_ResolveNode(CFXJSE_Arguments* pArguments);
  void Script_TreeClass_ResolveNodes(CFXJSE_Arguments* pArguments);

  void Script_TreeClass_All(CFXJSE_Value* pValue,
                            bool bSetting,
                            XFA_Attribute eAttribute);
  void Script_TreeClass_Nodes(CFXJSE_Value* pValue,
                              bool bSetting,
                              XFA_Attribute eAttribute);
  void Script_TreeClass_ClassAll(CFXJSE_Value* pValue,
                                 bool bSetting,
                                 XFA_Attribute eAttribute);
  void Script_TreeClass_Parent(CFXJSE_Value* pValue,
                               bool bSetting,
                               XFA_Attribute eAttribute);
  void Script_TreeClass_Index(CFXJSE_Value* pValue,
                              bool bSetting,
                              XFA_Attribute eAttribute);
  void Script_TreeClass_ClassIndex(CFXJSE_Value* pValue,
                                   bool bSetting,
                                   XFA_Attribute eAttribute);
  void Script_TreeClass_SomExpression(CFXJSE_Value* pValue,
                                      bool bSetting,
                                      XFA_Attribute eAttribute);
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
                           XFA_Attribute eAttribute);
  void Script_NodeClass_Model(CFXJSE_Value* pValue,
                              bool bSetting,
                              XFA_Attribute eAttribute);
  void Script_NodeClass_IsContainer(CFXJSE_Value* pValue,
                                    bool bSetting,
                                    XFA_Attribute eAttribute);
  void Script_NodeClass_IsNull(CFXJSE_Value* pValue,
                               bool bSetting,
                               XFA_Attribute eAttribute);
  void Script_NodeClass_OneOfChild(CFXJSE_Value* pValue,
                                   bool bSetting,
                                   XFA_Attribute eAttribute);
  void Script_ContainerClass_GetDelta(CFXJSE_Arguments* pArguments);
  void Script_ContainerClass_GetDeltas(CFXJSE_Arguments* pArguments);
  void Script_ModelClass_ClearErrorList(CFXJSE_Arguments* pArguments);
  void Script_ModelClass_CreateNode(CFXJSE_Arguments* pArguments);
  void Script_ModelClass_IsCompatibleNS(CFXJSE_Arguments* pArguments);
  void Script_ModelClass_Context(CFXJSE_Value* pValue,
                                 bool bSetting,
                                 XFA_Attribute eAttribute);
  void Script_ModelClass_AliasNode(CFXJSE_Value* pValue,
                                   bool bSetting,
                                   XFA_Attribute eAttribute);
  void Script_WsdlConnection_Execute(CFXJSE_Arguments* pArguments);
  void Script_Delta_Restore(CFXJSE_Arguments* pArguments);
  void Script_Delta_CurrentValue(CFXJSE_Value* pValue,
                                 bool bSetting,
                                 XFA_Attribute eAttribute);
  void Script_Delta_SavedValue(CFXJSE_Value* pValue,
                               bool bSetting,
                               XFA_Attribute eAttribute);
  void Script_Delta_Target(CFXJSE_Value* pValue,
                           bool bSetting,
                           XFA_Attribute eAttribute);
  void Script_Attribute_Integer(CFXJSE_Value* pValue,
                                bool bSetting,
                                XFA_Attribute eAttribute);
  void Script_Attribute_IntegerRead(CFXJSE_Value* pValue,
                                    bool bSetting,
                                    XFA_Attribute eAttribute);
  void Script_Attribute_BOOL(CFXJSE_Value* pValue,
                             bool bSetting,
                             XFA_Attribute eAttribute);
  void Script_Attribute_BOOLRead(CFXJSE_Value* pValue,
                                 bool bSetting,
                                 XFA_Attribute eAttribute);
  void Script_Attribute_String(CFXJSE_Value* pValue,
                               bool bSetting,
                               XFA_Attribute eAttribute);
  void Script_Attribute_StringRead(CFXJSE_Value* pValue,
                                   bool bSetting,
                                   XFA_Attribute eAttribute);
  void Script_Som_ValidationMessage(CFXJSE_Value* pValue,
                                    bool bSetting,
                                    XFA_Attribute eAttribute);
  void Script_Field_Length(CFXJSE_Value* pValue,
                           bool bSetting,
                           XFA_Attribute eAttribute);
  void Script_Som_DefaultValue(CFXJSE_Value* pValue,
                               bool bSetting,
                               XFA_Attribute eAttribute);
  void Script_Som_DefaultValue_Read(CFXJSE_Value* pValue,
                                    bool bSetting,
                                    XFA_Attribute eAttribute);
  void Script_Boolean_Value(CFXJSE_Value* pValue,
                            bool bSetting,
                            XFA_Attribute eAttribute);
  void Script_Som_Message(CFXJSE_Value* pValue,
                          bool bSetting,
                          XFA_SOM_MESSAGETYPE iMessageType);
  void Script_Som_BorderColor(CFXJSE_Value* pValue,
                              bool bSetting,
                              XFA_Attribute eAttribute);
  void Script_Som_BorderWidth(CFXJSE_Value* pValue,
                              bool bSetting,
                              XFA_Attribute eAttribute);
  void Script_Som_FillColor(CFXJSE_Value* pValue,
                            bool bSetting,
                            XFA_Attribute eAttribute);
  void Script_Som_DataNode(CFXJSE_Value* pValue,
                           bool bSetting,
                           XFA_Attribute eAttribute);
  void Script_Som_FontColor(CFXJSE_Value* pValue,
                            bool bSetting,
                            XFA_Attribute eAttribute);
  void Script_Som_Mandatory(CFXJSE_Value* pValue,
                            bool bSetting,
                            XFA_Attribute eAttribute);
  void Script_Som_MandatoryMessage(CFXJSE_Value* pValue,
                                   bool bSetting,
                                   XFA_Attribute eAttribute);
  void Script_Som_InstanceIndex(CFXJSE_Value* pValue,
                                bool bSetting,
                                XFA_Attribute eAttribute);
  void Script_Draw_DefaultValue(CFXJSE_Value* pValue,
                                bool bSetting,
                                XFA_Attribute eAttribute);
  void Script_Field_DefaultValue(CFXJSE_Value* pValue,
                                 bool bSetting,
                                 XFA_Attribute eAttribute);
  void Script_Field_EditValue(CFXJSE_Value* pValue,
                              bool bSetting,
                              XFA_Attribute eAttribute);
  void Script_Field_FormatMessage(CFXJSE_Value* pValue,
                                  bool bSetting,
                                  XFA_Attribute eAttribute);
  void Script_Field_FormattedValue(CFXJSE_Value* pValue,
                                   bool bSetting,
                                   XFA_Attribute eAttribute);
  void Script_Field_ParentSubform(CFXJSE_Value* pValue,
                                  bool bSetting,
                                  XFA_Attribute eAttribute);
  void Script_Field_SelectedIndex(CFXJSE_Value* pValue,
                                  bool bSetting,
                                  XFA_Attribute eAttribute);
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
                                           XFA_Attribute eAttribute);
  void Script_ExclGroup_ErrorText(CFXJSE_Value* pValue,
                                  bool bSetting,
                                  XFA_Attribute eAttribute);
  void Script_ExclGroup_Transient(CFXJSE_Value* pValue,
                                  bool bSetting,
                                  XFA_Attribute eAttribute);
  void Script_ExclGroup_ExecEvent(CFXJSE_Arguments* pArguments);
  void Script_ExclGroup_SelectedMember(CFXJSE_Arguments* pArguments);
  void Script_ExclGroup_ExecInitialize(CFXJSE_Arguments* pArguments);
  void Script_ExclGroup_ExecCalculate(CFXJSE_Arguments* pArguments);
  void Script_ExclGroup_ExecValidate(CFXJSE_Arguments* pArguments);
  void Script_Subform_InstanceManager(CFXJSE_Value* pValue,
                                      bool bSetting,
                                      XFA_Attribute eAttribute);
  void Script_Subform_Locale(CFXJSE_Value* pValue,
                             bool bSetting,
                             XFA_Attribute eAttribute);
  void Script_Subform_ExecEvent(CFXJSE_Arguments* pArguments);
  void Script_Subform_ExecInitialize(CFXJSE_Arguments* pArguments);
  void Script_Subform_ExecCalculate(CFXJSE_Arguments* pArguments);
  void Script_Subform_ExecValidate(CFXJSE_Arguments* pArguments);
  void Script_Subform_GetInvalidObjects(CFXJSE_Arguments* pArguments);
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
                                    XFA_Attribute eAttribute);
  void Script_InstanceManager_Max(CFXJSE_Value* pValue,
                                  bool bSetting,
                                  XFA_Attribute eAttribute);
  void Script_InstanceManager_Min(CFXJSE_Value* pValue,
                                  bool bSetting,
                                  XFA_Attribute eAttribute);
  void Script_InstanceManager_MoveInstance(CFXJSE_Arguments* pArguments);
  void Script_InstanceManager_RemoveInstance(CFXJSE_Arguments* pArguments);
  void Script_InstanceManager_SetInstances(CFXJSE_Arguments* pArguments);
  void Script_InstanceManager_AddInstance(CFXJSE_Arguments* pArguments);
  void Script_InstanceManager_InsertInstance(CFXJSE_Arguments* pArguments);
  void Script_Occur_Max(CFXJSE_Value* pValue,
                        bool bSetting,
                        XFA_Attribute eAttribute);
  void Script_Occur_Min(CFXJSE_Value* pValue,
                        bool bSetting,
                        XFA_Attribute eAttribute);
  void Script_Desc_Metadata(CFXJSE_Arguments* pArguments);
  void Script_Form_FormNodes(CFXJSE_Arguments* pArguments);
  void Script_Form_Remerge(CFXJSE_Arguments* pArguments);
  void Script_Form_ExecInitialize(CFXJSE_Arguments* pArguments);
  void Script_Form_Recalculate(CFXJSE_Arguments* pArguments);
  void Script_Form_ExecCalculate(CFXJSE_Arguments* pArguments);
  void Script_Form_ExecValidate(CFXJSE_Arguments* pArguments);
  void Script_Form_Checksum(CFXJSE_Value* pValue,
                            bool bSetting,
                            XFA_Attribute eAttribute);
  void Script_Packet_GetAttribute(CFXJSE_Arguments* pArguments);
  void Script_Packet_SetAttribute(CFXJSE_Arguments* pArguments);
  void Script_Packet_RemoveAttribute(CFXJSE_Arguments* pArguments);
  void Script_Packet_Content(CFXJSE_Value* pValue,
                             bool bSetting,
                             XFA_Attribute eAttribute);
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
                        XFA_Attribute eAttribute);
  void Script_Xfa_This(CFXJSE_Value* pValue,
                       bool bSetting,
                       XFA_Attribute eAttribute);
  void Script_Handler_Version(CFXJSE_Value* pValue,
                              bool bSetting,
                              XFA_Attribute eAttribute);
  void Script_SubmitFormat_Mode(CFXJSE_Value* pValue,
                                bool bSetting,
                                XFA_Attribute eAttribute);
  void Script_Extras_Type(CFXJSE_Value* pValue,
                          bool bSetting,
                          XFA_Attribute eAttribute);
  void Script_Encrypt_Format(CFXJSE_Value* pValue,
                             bool bSetting,
                             XFA_Attribute eAttribute);
  void Script_Script_Stateless(CFXJSE_Value* pValue,
                               bool bSetting,
                               XFA_Attribute eAttribute);

 private:
  bool SetUserData(void* pKey,
                   void* pData,
                   XFA_MAPDATABLOCKCALLBACKINFO* pCallbackInfo);

  void ResolveNodeList(CFXJSE_Value* pValue,
                       WideString wsExpression,
                       uint32_t dwFlag,
                       CXFA_Node* refNode);

  void OnChanged(XFA_Attribute eAttr, bool bNotify, bool bScriptModify);
  void OnChanging(XFA_Attribute eAttr, bool bNotify);
  void SendAttributeChangeMessage(XFA_Attribute eAttribute, bool bScriptModify);

  // Returns a pointer to the XML node that needs to be updated with the new
  // attribute value. |nullptr| if no update is needed.
  CFX_XMLElement* SetValue(XFA_Attribute eAttr,
                           XFA_AttributeType eType,
                           void* pValue,
                           bool bNotify);

  bool TryUserData(void* pKey, void*& pData, bool bProtoAlso);

  XFA_MAPMODULEDATA* CreateMapModuleData();
  XFA_MAPMODULEDATA* GetMapModuleData() const;
  void SetMapModuleValue(void* pKey, void* pValue);
  bool GetMapModuleValue(void* pKey, void*& pValue);
  void SetMapModuleString(void* pKey, const WideStringView& wsValue);
  bool GetMapModuleString(void* pKey, WideStringView& wsValue);
  void SetMapModuleBuffer(void* pKey,
                          void* pValue,
                          int32_t iBytes,
                          XFA_MAPDATABLOCKCALLBACKINFO* pCallbackInfo);
  bool GetMapModuleBuffer(void* pKey,
                          void*& pValue,
                          int32_t& iBytes,
                          bool bProtoAlso) const;
  bool HasMapModuleKey(void* pKey);
  void ClearMapModuleBuffer();
  void RemoveMapModuleKey(void* pKey);
  void MoveBufferMapData(CXFA_Node* pDstModule);
  void MoveBufferMapData(CXFA_Node* pSrcModule, CXFA_Node* pDstModule);

  int32_t execSingleEventByName(const WideStringView& wsEventName,
                                XFA_Element eType);

  std::unique_ptr<XFA_MAPMODULEDATA> map_module_data_;
  std::unique_ptr<CXFA_WidgetData> widget_data_;
  std::unique_ptr<CXFA_CalcData> calc_data_;
  UnownedPtr<CXFA_LayoutItem> layout_item_;
  std::vector<UnownedPtr<CXFA_Node>> binding_nodes_;
  size_t calc_recursion_count_ = 0;
};

#endif  // FXJS_CJX_NODE_H_
