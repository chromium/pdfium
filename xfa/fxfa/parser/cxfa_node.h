// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_NODE_H_
#define XFA_FXFA_PARSER_CXFA_NODE_H_

#include <stddef.h>
#include <stdint.h>

#include <utility>
#include <vector>

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/mask.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/unowned_ptr.h"
#include "core/fxcrt/widestring.h"
#include "core/fxge/dib/fx_dib.h"
#include "fxjs/gc/gced_tree_node_mixin.h"
#include "third_party/abseil-cpp/absl/types/optional.h"
#include "third_party/base/span.h"
#include "v8/include/cppgc/member.h"
#include "v8/include/cppgc/visitor.h"
#include "xfa/fxfa/cxfa_ffwidget_type.h"
#include "xfa/fxfa/fxfa.h"
#include "xfa/fxfa/parser/cxfa_object.h"

class CFGAS_GEFont;
class CFX_DIBitmap;
class CFX_XMLDocument;
class CFX_XMLNode;
class CXFA_Bind;
class CXFA_Border;
class CXFA_Calculate;
class CXFA_Caption;
class CXFA_Event;
class CXFA_EventParam;
class CXFA_FFDoc;
class CXFA_FFDocView;
class CXFA_Font;
class CXFA_Keep;
class CXFA_Margin;
class CXFA_Measurement;
class CXFA_Occur;
class CXFA_Para;
class CXFA_Script;
class CXFA_TextLayout;
class CXFA_Ui;
class CXFA_Validate;
class CXFA_Value;
class CXFA_WidgetLayoutData;
class GCedLocaleIface;

enum class XFA_NodeFilter : uint8_t {
  kChildren = 1 << 0,
  kProperties = 1 << 1,
  kOneOfProperty = 1 << 2,
};

enum class XFA_CheckState : uint8_t {
  kOn = 0,
  kOff = 1,
  kNeutral = 2,
};

enum class XFA_ValuePicture : uint8_t {
  kRaw = 0,
  kDisplay,
  kEdit,
  kDataBind,
};

enum class XFA_NodeFlag : uint8_t {
  kNone = 0,
  kInitialized = 1 << 0,
  kHasRemovedChildren = 1 << 1,
  kNeedsInitApp = 1 << 2,
  kBindFormItems = 1 << 3,
  kUserInteractive = 1 << 4,
  kUnusedNode = 1 << 5,
  kLayoutGeneratedNode = 1 << 6
};

enum class XFA_PropertyFlag : uint8_t {
  kOneOf = 1 << 0,
  kDefaultOneOf = 1 << 1,
};

class CXFA_Node : public CXFA_Object, public GCedTreeNodeMixin<CXFA_Node> {
 public:
  struct PropertyData {
    PropertyData() = delete;

    XFA_Element property;
    uint8_t occurrence_count;
    Mask<XFA_PropertyFlag> flags;
  };

  struct AttributeData {
    XFA_Attribute attribute;
    XFA_AttributeType type;
    void* default_value;
  };

  // Node is created from cppgc heap.
  static CXFA_Node* Create(CXFA_Document* doc,
                           XFA_Element element,
                           XFA_PacketType packet);

  ~CXFA_Node() override;

  // CXFA_Object:
  void Trace(cppgc::Visitor* visitor) const override;

  bool HasProperty(XFA_Element property) const;
  bool HasPropertyFlag(XFA_Element property, XFA_PropertyFlag flag) const;
  uint8_t PropertyOccurrenceCount(XFA_Element property) const;

  std::pair<CXFA_Node*, int32_t> GetProperty(int32_t index,
                                             XFA_Element eProperty) const;
  CXFA_Node* GetOrCreateProperty(int32_t index, XFA_Element eProperty);

  void SendAttributeChangeMessage(XFA_Attribute eAttribute, bool bScriptModify);

  bool HasAttribute(XFA_Attribute attr) const;
  XFA_AttributeType GetAttributeType(XFA_Attribute type) const;

  // Note: returns XFA_Attribute::Unknown for invalid indicies.
  XFA_Attribute GetAttribute(size_t i) const;

  XFA_PacketType GetPacketType() const { return m_ePacket; }

  void SetInitializedFlagAndNotify();
  void SetFlag(XFA_NodeFlag dwFlag);
  void ClearFlag(XFA_NodeFlag dwFlag);

  CXFA_Node* CreateInstanceIfPossible(bool bDataMerge);
  int32_t GetCount();
  CXFA_Node* GetItemIfExists(int32_t iIndex);
  void RemoveItem(CXFA_Node* pRemoveInstance, bool bRemoveDataBinding);
  void InsertItem(CXFA_Node* pNewInstance,
                  int32_t iPos,
                  int32_t iCount,
                  bool bMoveDataBindingNodes);

  bool IsInitialized() const { return HasFlag(XFA_NodeFlag::kInitialized); }
  bool IsUserInteractive() const {
    return HasFlag(XFA_NodeFlag::kUserInteractive);
  }
  bool IsUnusedNode() const { return HasFlag(XFA_NodeFlag::kUnusedNode); }
  bool IsLayoutGeneratedNode() const {
    return HasFlag(XFA_NodeFlag::kLayoutGeneratedNode);
  }

  bool PresenceRequiresSpace() const;
  void SetBindingNode(CXFA_Node* node);
  void SetNodeAndDescendantsUnused();

  bool HasRemovedChildren() const {
    return HasFlag(XFA_NodeFlag::kHasRemovedChildren);
  }

  bool IsAttributeInXML();
  bool IsFormContainer() const {
    return m_ePacket == XFA_PacketType::Form && IsContainerNode();
  }

  void SetXMLMappingNode(CFX_XMLNode* node) { xml_node_ = node; }
  CFX_XMLNode* GetXMLMappingNode() const { return xml_node_.Get(); }
  CFX_XMLNode* CreateXMLMappingNode();
  bool IsNeedSavingXMLNode() const;

  void SetToXML(const WideString& value);

  uint32_t GetNameHash() const { return m_dwNameHash; }
  bool IsUnnamed() const { return m_dwNameHash == 0; }
  CXFA_Node* GetModelNode();
  void UpdateNameHash();

  size_t CountChildren(XFA_Element eType, bool bOnlyChild);

  template <typename T>
  T* GetChild(size_t index, XFA_Element eType, bool bOnlyChild) {
    return static_cast<T*>(GetChildInternal(index, eType, bOnlyChild));
  }

  template <typename T>
  const T* GetChild(size_t index, XFA_Element eType, bool bOnlyChild) const {
    return static_cast<const T*>(GetChildInternal(index, eType, bOnlyChild));
  }

  bool IsAncestorOf(const CXFA_Node* that) const;

  void InsertChildAndNotify(int32_t index, CXFA_Node* pNode);
  void InsertChildAndNotify(CXFA_Node* pNode, CXFA_Node* pBeforeNode);
  void RemoveChildAndNotify(CXFA_Node* pNode, bool bNotify);

  CXFA_Node* Clone(bool bRecursive);

  CXFA_Node* GetNextContainerSibling() const;
  CXFA_Node* GetPrevContainerSibling() const;
  CXFA_Node* GetFirstContainerChild() const;
  CXFA_Node* GetContainerParent() const;

  std::vector<CXFA_Node*> GetNodeListForType(XFA_Element eTypeFilter);
  std::vector<CXFA_Node*> GetNodeListWithFilter(Mask<XFA_NodeFilter> dwFilter);
  CXFA_Node* CreateSamePacketNode(XFA_Element eType);
  CXFA_Node* CloneTemplateToForm(bool bRecursive);
  CXFA_Node* GetTemplateNodeIfExists() const;
  void SetTemplateNode(CXFA_Node* pTemplateNode);
  CXFA_Node* GetDataDescriptionNode();
  void SetDataDescriptionNode(CXFA_Node* pDataDescriptionNode);
  CXFA_Node* GetBindData();
  bool HasBindItems() const { return !binding_nodes_.empty(); }
  std::vector<CXFA_Node*> GetBindItemsCopy() const;
  void AddBindItem(CXFA_Node* pFormNode);
  // Returns true if there are still more items.
  bool RemoveBindItem(CXFA_Node* pFormNode);
  bool HasBindItem() const;
  CXFA_Node* GetContainerNode();
  GCedLocaleIface* GetLocale();
  absl::optional<WideString> GetLocaleName();
  XFA_AttributeValue GetIntact();
  WideString GetNameExpression();

  CXFA_Node* GetFirstChildByName(WideStringView wsNodeName) const;
  CXFA_Node* GetFirstChildByName(uint32_t dwNodeNameHash) const;
  template <typename T>
  T* GetFirstChildByClass(XFA_Element eType) const {
    return static_cast<T*>(GetFirstChildByClassInternal(eType));
  }
  CXFA_Node* GetNextSameNameSibling(uint32_t dwNodeNameHash) const;
  template <typename T>
  T* GetNextSameNameSibling(WideStringView wsNodeName) const {
    return static_cast<T*>(GetNextSameNameSiblingInternal(wsNodeName));
  }
  template <typename T>
  T* GetNextSameClassSibling(XFA_Element eType) const {
    return static_cast<T*>(GetNextSameClassSiblingInternal(eType));
  }

  CXFA_Node* GetOneChildNamed(WideStringView wsName);
  CXFA_Node* GetOneChildOfClass(WideStringView wsClass);

  std::vector<CXFA_Node*> GetSiblings(bool bIsClassName);
  size_t GetIndex(bool bIsProperty, bool bIsClassIndex);
  size_t GetIndexByName();
  size_t GetIndexByClassName();

  CXFA_Node* GetInstanceMgrOfSubform();

  absl::optional<bool> GetDefaultBoolean(XFA_Attribute attr) const;
  absl::optional<int32_t> GetDefaultInteger(XFA_Attribute attr) const;
  absl::optional<CXFA_Measurement> GetDefaultMeasurement(
      XFA_Attribute attr) const;
  absl::optional<WideString> GetDefaultCData(XFA_Attribute attr) const;
  absl::optional<XFA_AttributeValue> GetDefaultEnum(XFA_Attribute attr) const;

  bool IsOpenAccess() const;

  CXFA_Occur* GetOccurIfExists();
  CXFA_Border* GetBorderIfExists() const;
  CXFA_Border* GetOrCreateBorderIfPossible();
  CXFA_Caption* GetCaptionIfExists() const;
  CXFA_Font* GetFontIfExists() const;
  CXFA_Font* GetOrCreateFontIfPossible();

  float GetFontSize() const;
  FX_ARGB GetTextColor() const;
  float GetLineHeight() const;

  CXFA_Margin* GetMarginIfExists() const;
  CXFA_Para* GetParaIfExists() const;
  CXFA_Calculate* GetCalculateIfExists() const;
  CXFA_Validate* GetValidateIfExists() const;
  CXFA_Validate* GetOrCreateValidateIfPossible();

  CXFA_Value* GetFormValueIfExists() const;
  WideString GetRawValue() const;

  int32_t GetRotate() const;
  absl::optional<float> TryWidth();

  CXFA_Node* GetExclGroupIfExists();

  XFA_EventError ProcessEvent(CXFA_FFDocView* pDocView,
                              XFA_AttributeValue iActivity,
                              CXFA_EventParam* pEventParam);
  XFA_EventError ProcessCalculate(CXFA_FFDocView* pDocView);
  XFA_EventError ProcessValidate(CXFA_FFDocView* pDocView, int32_t iFlags);
  XFA_EventError ExecuteScript(CXFA_FFDocView* pDocView,
                               CXFA_Script* script,
                               CXFA_EventParam* pEventParam);
  std::pair<XFA_EventError, bool> ExecuteBoolScript(
      CXFA_FFDocView* pDocView,
      CXFA_Script* script,
      CXFA_EventParam* pEventParam);

  CXFA_Node* GetUIChildNode();

  // NOTE: value returned is often determined by child UI node, and
  // can't be used to infer anything about this particual node itself.
  XFA_FFWidgetType GetFFWidgetType();

  CFX_RectF GetUIMargin();
  CXFA_Border* GetUIBorder();

  void SetPreNull(bool val) { m_bPreNull = val; }
  bool IsNull() const { return m_bIsNull; }
  void SetIsNull(bool val) { m_bIsNull = val; }

  void SetWidgetReady() { is_widget_ready_ = true; }
  bool IsWidgetReady() const { return is_widget_ready_; }
  std::vector<CXFA_Event*> GetEventByActivity(XFA_AttributeValue iActivity,
                                              bool bIsFormReady);

  void ResetData();
  void StartWidgetLayout(CXFA_FFDoc* doc,
                         float* pCalcWidth,
                         float* pCalcHeight);
  absl::optional<float> FindSplitPos(CXFA_FFDocView* pDocView,
                                     size_t szBlockIndex,
                                     float fCalcHeight);

  bool LoadCaption(CXFA_FFDoc* doc);
  CXFA_TextLayout* GetCaptionTextLayout();
  CXFA_TextLayout* GetTextLayout();

  bool LoadImageImage(CXFA_FFDoc* doc);
  bool LoadImageEditImage(CXFA_FFDoc* doc);
  CFX_Size GetImageDpi() const;
  CFX_Size GetImageEditDpi() const;

  RetainPtr<CFX_DIBitmap> GetImageImage();
  RetainPtr<CFX_DIBitmap> GetImageEditImage();
  void SetImageImage(const RetainPtr<CFX_DIBitmap>& newImage);
  void SetImageEditImage(const RetainPtr<CFX_DIBitmap>& newImage);

  RetainPtr<CFGAS_GEFont> GetFGASFont(CXFA_FFDoc* doc);

  bool IsListBox();
  bool IsRadioButton();
  bool IsMultiLine();

  bool HasButtonRollover() const;
  bool HasButtonDown() const;

  float GetCheckButtonSize();

  XFA_CheckState GetCheckState();
  void SetCheckState(XFA_CheckState eCheckState);

  CXFA_Node* GetSelectedMember();
  CXFA_Node* SetSelectedMember(WideStringView wsName);
  void SetSelectedMemberByValue(WideStringView wsValue,
                                bool bNotify,
                                bool bScriptModify,
                                bool bSyncData);

  CXFA_Node* GetExclGroupFirstMember();
  CXFA_Node* GetExclGroupNextMember(CXFA_Node* pNode);

  bool IsChoiceListAllowTextEntry();
  size_t CountChoiceListItems(bool bSaveValue);
  absl::optional<WideString> GetChoiceListItem(int32_t nIndex, bool bSaveValue);
  bool IsChoiceListMultiSelect();
  bool IsChoiceListCommitOnSelect();
  std::vector<WideString> GetChoiceListItems(bool bSaveValue);

  int32_t CountSelectedItems();
  int32_t GetSelectedItem(int32_t nIndex);
  std::vector<int32_t> GetSelectedItems();
  std::vector<WideString> GetSelectedItemsValue();
  void SetSelectedItems(const std::vector<int32_t>& iSelArray,
                        bool bNotify,
                        bool bScriptModify,
                        bool bSyncData);
  void InsertItem(const WideString& wsLabel,
                  const WideString& wsValue,
                  bool bNotify);
  bool DeleteItem(int32_t nIndex, bool bNotify, bool bScriptModify);
  void ClearAllSelections();

  bool GetItemState(int32_t nIndex);
  void SetItemState(int32_t nIndex,
                    bool bSelected,
                    bool bNotify,
                    bool bScriptModify,
                    bool bSyncData);

  WideString GetItemValue(WideStringView wsLabel);

  bool IsHorizontalScrollPolicyOff();
  bool IsVerticalScrollPolicyOff();
  absl::optional<int32_t> GetNumberOfCells();

  bool SetValue(XFA_ValuePicture eValueType, const WideString& wsValue);
  WideString GetValue(XFA_ValuePicture eValueType);
  WideString GetPictureContent(XFA_ValuePicture ePicture);
  WideString GetNormalizeDataValue(const WideString& wsValue);
  WideString GetFormatDataValue(const WideString& wsValue);
  WideString NormalizeNumStr(const WideString& wsValue);

  std::pair<XFA_Element, int32_t> GetMaxChars() const;
  int32_t GetFracDigits() const;
  int32_t GetLeadDigits() const;

  WideString NumericLimit(const WideString& wsValue);

  bool IsTransparent() const;
  bool IsProperty() const;

 protected:
  CXFA_Node(CXFA_Document* pDoc,
            XFA_PacketType ePacket,
            Mask<XFA_XDPPACKET> validPackets,
            XFA_ObjectType oType,
            XFA_Element eType,
            pdfium::span<const PropertyData> properties,
            pdfium::span<const AttributeData> attributes,
            CJX_Object* js_object);

  virtual XFA_Element GetValueNodeType() const;
  virtual XFA_FFWidgetType GetDefaultFFWidgetType() const;

 private:
  void ProcessScriptTestValidate(CXFA_FFDocView* pDocView,
                                 CXFA_Validate* validate,
                                 bool bVersionFlag);
  XFA_EventError ProcessFormatTestValidate(CXFA_FFDocView* pDocView,
                                           CXFA_Validate* validate,
                                           bool bVersionFlag);
  XFA_EventError ProcessNullTestValidate(CXFA_FFDocView* pDocView,
                                         CXFA_Validate* validate,
                                         int32_t iFlags,
                                         bool bVersionFlag);
  WideString GetValidateCaptionName(bool bVersionFlag);
  WideString GetValidateMessage(bool bError, bool bVersionFlag);

  bool HasFlag(XFA_NodeFlag dwFlag) const;
  const PropertyData* GetPropertyData(XFA_Element property) const;
  const AttributeData* GetAttributeData(XFA_Attribute attr) const;
  absl::optional<XFA_Element> GetFirstPropertyWithFlag(
      XFA_PropertyFlag flag) const;
  void OnRemoved(bool bNotify) const;
  absl::optional<void*> GetDefaultValue(XFA_Attribute attr,
                                        XFA_AttributeType eType) const;
  CXFA_Node* GetChildInternal(size_t index,
                              XFA_Element eType,
                              bool bOnlyChild) const;
  CXFA_Node* GetFirstChildByClassInternal(XFA_Element eType) const;
  CXFA_Node* GetNextSameNameSiblingInternal(WideStringView wsNodeName) const;
  CXFA_Node* GetNextSameClassSiblingInternal(XFA_Element eType) const;
  void CalcCaptionSize(CXFA_FFDoc* doc, CFX_SizeF* pszCap);
  bool CalculateFieldAutoSize(CXFA_FFDoc* doc, CFX_SizeF* pSize);
  bool CalculateWidgetAutoSize(CFX_SizeF* pSize);
  bool CalculateTextEditAutoSize(CXFA_FFDoc* doc, CFX_SizeF* pSize);
  bool CalculateCheckButtonAutoSize(CXFA_FFDoc* doc, CFX_SizeF* pSize);
  bool CalculatePushButtonAutoSize(CXFA_FFDoc* doc, CFX_SizeF* pSize);
  CFX_SizeF CalculateImageSize(float img_width,
                               float img_height,
                               const CFX_Size& dpi);
  bool CalculateImageEditAutoSize(CXFA_FFDoc* doc, CFX_SizeF* pSize);
  bool CalculateImageAutoSize(CXFA_FFDoc* doc, CFX_SizeF* pSize);
  float CalculateWidgetAutoHeight(float fHeightCalc);
  float CalculateWidgetAutoWidth(float fWidthCalc);
  float GetWidthWithoutMargin(float fWidthCalc) const;
  float GetHeightWithoutMargin(float fHeightCalc) const;
  void CalculateTextContentSize(CXFA_FFDoc* doc, CFX_SizeF* pSize);
  CFX_SizeF CalculateAccWidthAndHeight(CXFA_FFDoc* doc, float fWidth);
  void InitLayoutData(CXFA_FFDoc* doc);
  void StartTextLayout(CXFA_FFDoc* doc, float* pCalcWidth, float* pCalcHeight);

  void InsertListTextItem(CXFA_Node* pItems,
                          const WideString& wsText,
                          int32_t nIndex);
  WideString GetItemLabel(WideStringView wsValue) const;

  std::pair<XFA_FFWidgetType, CXFA_Ui*> CreateChildUIAndValueNodesIfNeeded();
  void CreateValueNodeIfNeeded(CXFA_Value* value, CXFA_Node* pUIChild);
  CXFA_Node* CreateUINodeIfNeeded(CXFA_Ui* ui, XFA_Element type);
  bool IsValidInPacket(XFA_PacketType packet) const;
  void SetImageEdit(const WideString& wsContentType,
                    const WideString& wsHref,
                    const WideString& wsData);
  CXFA_Node* GetBindingNode() const {
    if (binding_nodes_.empty())
      return nullptr;
    return binding_nodes_[0];
  }
  bool BindsFormItems() const { return HasFlag(XFA_NodeFlag::kBindFormItems); }
  bool NeedsInitApp() const { return HasFlag(XFA_NodeFlag::kNeedsInitApp); }
  void SyncValue(const WideString& wsValue, bool bNotify);
  CXFA_Value* GetDefaultValueIfExists();
  CXFA_Bind* GetBindIfExists() const;
  absl::optional<XFA_AttributeValue> GetIntactFromKeep(
      const CXFA_Keep* pKeep,
      XFA_AttributeValue eLayoutType) const;
  CXFA_Node* GetTransparentParent();

  absl::optional<float> TryHeight();
  absl::optional<float> TryMinWidth();
  absl::optional<float> TryMinHeight();
  absl::optional<float> TryMaxWidth();
  absl::optional<float> TryMaxHeight();
  XFA_EventError ProcessEventInternal(CXFA_FFDocView* pDocView,
                                      XFA_AttributeValue iActivity,
                                      CXFA_Event* event,
                                      CXFA_EventParam* pEventParam);

  CFX_XMLDocument* GetXMLDocument() const;

  XFA_FFWidgetType ff_widget_type_ = XFA_FFWidgetType::kNone;
  bool m_bIsNull = true;
  bool m_bPreNull = true;
  bool is_widget_ready_ = false;
  const pdfium::span<const PropertyData> m_Properties;
  const pdfium::span<const AttributeData> m_Attributes;
  const Mask<XFA_XDPPACKET> m_ValidPackets;
  UnownedPtr<CFX_XMLNode> xml_node_;
  const XFA_PacketType m_ePacket;
  uint8_t m_ExecuteRecursionDepth = 0;
  Mask<XFA_NodeFlag> m_uNodeFlags = XFA_NodeFlag::kNone;
  uint32_t m_dwNameHash = 0;
  cppgc::Member<CXFA_Node> m_pAuxNode;
  std::vector<cppgc::Member<CXFA_Node>> binding_nodes_;
  cppgc::Member<CXFA_WidgetLayoutData> m_pLayoutData;
  cppgc::Member<CXFA_Ui> ui_;
};

#endif  // XFA_FXFA_PARSER_CXFA_NODE_H_
