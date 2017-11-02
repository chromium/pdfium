// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_CJX_NODE_H_
#define FXJS_CJX_NODE_H_

#include <map>

#include "core/fxcrt/unowned_ptr.h"
#include "fxjs/cjx_object.h"
#include "xfa/fxfa/fxfa_basic.h"

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

enum XFA_SOM_MESSAGETYPE {
  XFA_SOM_ValidationMessage,
  XFA_SOM_FormatMessage,
  XFA_SOM_MandatoryMessage
};

class CFXJSE_Arguments;
class CXFA_Node;

class CJX_Node : public CJX_Object {
 public:
  explicit CJX_Node(CXFA_Node* node);
  ~CJX_Node() override;

  CXFA_Node* GetXFANode();
  const CXFA_Node* GetXFANode() const;

  bool HasAttribute(XFA_ATTRIBUTE eAttr);
  bool SetAttribute(XFA_ATTRIBUTE eAttr,
                    const WideStringView& wsValue,
                    bool bNotify);
  bool SetAttribute(const WideStringView& wsAttr,
                    const WideStringView& wsValue,
                    bool bNotify);
  bool GetAttribute(const WideStringView& wsAttr,
                    WideString& wsValue,
                    bool bUseDefault);
  bool GetAttribute(XFA_ATTRIBUTE eAttr, WideString& wsValue, bool bUseDefault);
  bool SetAttributeValue(const WideString& wsValue,
                         const WideString& wsXMLValue,
                         bool bNotify,
                         bool bScriptModify);
  bool RemoveAttribute(const WideStringView& wsAttr);

  CXFA_Node* GetProperty(int32_t index,
                         XFA_Element eType,
                         bool bCreateProperty = true);

  bool SetContent(const WideString& wsContent,
                  const WideString& wsXMLValue,
                  bool bNotify = false,
                  bool bScriptModify = false,
                  bool bSyncData = true);
  WideString GetContent();

  bool TryInteger(XFA_ATTRIBUTE eAttr,
                  int32_t& iValue,
                  bool bUseDefault = true);
  bool SetInteger(XFA_ATTRIBUTE eAttr, int32_t iValue, bool bNotify = false);
  int32_t GetInteger(XFA_ATTRIBUTE eAttr);

  bool TryCData(XFA_ATTRIBUTE eAttr,
                WideStringView& wsValue,
                bool bUseDefault = true,
                bool bProto = true);
  bool TryCData(XFA_ATTRIBUTE eAttr,
                WideString& wsValue,
                bool bUseDefault = true,
                bool bProto = true);
  bool SetCData(XFA_ATTRIBUTE eAttr,
                const WideString& wsValue,
                bool bNotify = false,
                bool bScriptModify = false);
  WideStringView GetCData(XFA_ATTRIBUTE eAttr);

  bool TryContent(WideString& wsContent,
                  bool bScriptModify = false,
                  bool bProto = true);

  bool TryEnum(XFA_ATTRIBUTE eAttr,
               XFA_ATTRIBUTEENUM& eValue,
               bool bUseDefault = true);
  bool SetEnum(XFA_ATTRIBUTE eAttr,
               XFA_ATTRIBUTEENUM eValue,
               bool bNotify = false);
  XFA_ATTRIBUTEENUM GetEnum(XFA_ATTRIBUTE eAttr);

  bool TryBoolean(XFA_ATTRIBUTE eAttr, bool& bValue, bool bUseDefault = true);
  bool SetBoolean(XFA_ATTRIBUTE eAttr, bool bValue, bool bNotify = false);
  bool GetBoolean(XFA_ATTRIBUTE eAttr);

  bool TryMeasure(XFA_ATTRIBUTE eAttr,
                  CXFA_Measurement& mValue,
                  bool bUseDefault = true) const;
  bool SetMeasure(XFA_ATTRIBUTE eAttr,
                  CXFA_Measurement mValue,
                  bool bNotify = false);
  CXFA_Measurement GetMeasure(XFA_ATTRIBUTE eAttr) const;

  bool SetUserData(void* pKey,
                   void* pData,
                   XFA_MAPDATABLOCKCALLBACKINFO* pCallbackInfo = nullptr);
  void* GetUserData(void* pKey, bool bProtoAlso = false);

  bool TryObject(XFA_ATTRIBUTE eAttr, void*& pData);
  bool SetObject(XFA_ATTRIBUTE eAttr,
                 void* pData,
                 XFA_MAPDATABLOCKCALLBACKINFO* pCallbackInfo = nullptr);
  void* GetObject(XFA_ATTRIBUTE eAttr);

  bool TryNamespace(WideString& wsNamespace);

  void MergeAllData(void* pDstModule);

  void ThrowMissingPropertyException(const WideString& obj,
                                     const WideString& prop) const;
  void ThrowTooManyOccurancesException(const WideString& obj) const;

  int32_t Subform_and_SubformSet_InstanceIndex();
  int32_t InstanceManager_SetInstances(int32_t iDesired);
  int32_t InstanceManager_MoveInstance(int32_t iTo, int32_t iFrom);

  void Script_TreeClass_ResolveNode(CFXJSE_Arguments* pArguments);
  void Script_TreeClass_ResolveNodes(CFXJSE_Arguments* pArguments);
  void Script_Som_ResolveNodeList(CFXJSE_Value* pValue,
                                  WideString wsExpression,
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
  bool SetValue(XFA_ATTRIBUTE eAttr,
                XFA_ATTRIBUTETYPE eType,
                void* pValue,
                bool bNotify);
  bool GetValue(XFA_ATTRIBUTE eAttr,
                XFA_ATTRIBUTETYPE eType,
                bool bUseDefault,
                void*& pValue);

  bool TryUserData(void* pKey, void*& pData, bool bProtoAlso = false);

  bool SetScriptContent(const WideString& wsContent,
                        const WideString& wsXMLValue,
                        bool bNotify,
                        bool bScriptModify,
                        bool bSyncData);
  WideString GetScriptContent(bool bScriptModify = false);

  XFA_MAPMODULEDATA* CreateMapModuleData();
  XFA_MAPMODULEDATA* GetMapModuleData() const;
  void SetMapModuleValue(void* pKey, void* pValue);
  bool GetMapModuleValue(void* pKey, void*& pValue);
  void SetMapModuleString(void* pKey, const WideStringView& wsValue);
  bool GetMapModuleString(void* pKey, WideStringView& wsValue);
  void SetMapModuleBuffer(
      void* pKey,
      void* pValue,
      int32_t iBytes,
      XFA_MAPDATABLOCKCALLBACKINFO* pCallbackInfo = nullptr);
  bool GetMapModuleBuffer(void* pKey,
                          void*& pValue,
                          int32_t& iBytes,
                          bool bProtoAlso = true) const;
  bool HasMapModuleKey(void* pKey);
  void RemoveMapModuleKey(void* pKey = nullptr);
  void MoveBufferMapData(CXFA_Node* pDstModule, void* pKey);
  void MoveBufferMapData(CXFA_Node* pSrcModule,
                         CXFA_Node* pDstModule,
                         void* pKey,
                         bool bRecursive = false);

  int32_t execSingleEventByName(const WideStringView& wsEventName,
                                XFA_Element eType);

  XFA_MAPMODULEDATA* map_module_data_;
};

#endif  // FXJS_CJX_NODE_H_
