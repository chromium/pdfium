// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_widgetdata.h"

#include "core/fxcrt/cfx_decimal.h"
#include "core/fxcrt/fx_extension.h"
#include "third_party/base/stl_util.h"
#include "xfa/fxfa/cxfa_ffnotify.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_eventdata.h"
#include "xfa/fxfa/parser/cxfa_localevalue.h"
#include "xfa/fxfa/parser/cxfa_measurement.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/xfa_utils.h"

namespace {

float GetEdgeThickness(const std::vector<CXFA_StrokeData>& strokes,
                       bool b3DStyle,
                       int32_t nIndex) {
  float fThickness = 0;

  if (strokes[nIndex * 2 + 1].IsVisible()) {
    if (nIndex == 0)
      fThickness += 2.5f;

    fThickness += strokes[nIndex * 2 + 1].GetThickness() * (b3DStyle ? 4 : 2);
  }
  return fThickness;
}

bool SplitDateTime(const WideString& wsDateTime,
                   WideString& wsDate,
                   WideString& wsTime) {
  wsDate = L"";
  wsTime = L"";
  if (wsDateTime.IsEmpty())
    return false;

  auto nSplitIndex = wsDateTime.Find('T');
  if (!nSplitIndex.has_value())
    nSplitIndex = wsDateTime.Find(' ');
  if (!nSplitIndex.has_value())
    return false;

  wsDate = wsDateTime.Left(nSplitIndex.value());
  if (!wsDate.IsEmpty()) {
    if (!std::any_of(wsDate.begin(), wsDate.end(), std::iswdigit))
      return false;
  }
  wsTime = wsDateTime.Right(wsDateTime.GetLength() - nSplitIndex.value() - 1);
  if (!wsTime.IsEmpty()) {
    if (!std::any_of(wsTime.begin(), wsTime.end(), std::iswdigit))
      return false;
  }
  return true;
}

CXFA_Node* CreateUIChild(CXFA_Node* pNode, XFA_Element& eWidgetType) {
  XFA_Element eType = pNode->GetElementType();
  eWidgetType = eType;
  if (eType != XFA_Element::Field && eType != XFA_Element::Draw)
    return nullptr;

  eWidgetType = XFA_Element::Unknown;
  XFA_Element eUIType = XFA_Element::Unknown;
  CXFA_ValueData defValueData(
      pNode->JSNode()->GetProperty(0, XFA_Element::Value, true));
  XFA_Element eValueType = defValueData.GetChildValueClassID();
  switch (eValueType) {
    case XFA_Element::Boolean:
      eUIType = XFA_Element::CheckButton;
      break;
    case XFA_Element::Integer:
    case XFA_Element::Decimal:
    case XFA_Element::Float:
      eUIType = XFA_Element::NumericEdit;
      break;
    case XFA_Element::ExData:
    case XFA_Element::Text:
      eUIType = XFA_Element::TextEdit;
      eWidgetType = XFA_Element::Text;
      break;
    case XFA_Element::Date:
    case XFA_Element::Time:
    case XFA_Element::DateTime:
      eUIType = XFA_Element::DateTimeEdit;
      break;
    case XFA_Element::Image:
      eUIType = XFA_Element::ImageEdit;
      eWidgetType = XFA_Element::Image;
      break;
    case XFA_Element::Arc:
    case XFA_Element::Line:
    case XFA_Element::Rectangle:
      eUIType = XFA_Element::DefaultUi;
      eWidgetType = eValueType;
      break;
    default:
      break;
  }

  CXFA_Node* pUIChild = nullptr;
  CXFA_Node* pUI = pNode->JSNode()->GetProperty(0, XFA_Element::Ui, true);
  CXFA_Node* pChild = pUI->GetNodeItem(XFA_NODEITEM_FirstChild);
  for (; pChild; pChild = pChild->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    XFA_Element eChildType = pChild->GetElementType();
    if (eChildType == XFA_Element::Extras ||
        eChildType == XFA_Element::Picture) {
      continue;
    }

    auto node = CXFA_Node::Create(pChild->GetDocument(), XFA_Element::Ui,
                                  XFA_PacketType::Form);
    if (node && node->HasPropertyFlags(eChildType, XFA_PROPERTYFLAG_OneOf)) {
      pUIChild = pChild;
      break;
    }
  }

  if (eType == XFA_Element::Draw) {
    XFA_Element eDraw =
        pUIChild ? pUIChild->GetElementType() : XFA_Element::Unknown;
    switch (eDraw) {
      case XFA_Element::TextEdit:
        eWidgetType = XFA_Element::Text;
        break;
      case XFA_Element::ImageEdit:
        eWidgetType = XFA_Element::Image;
        break;
      default:
        eWidgetType = eWidgetType == XFA_Element::Unknown ? XFA_Element::Text
                                                          : eWidgetType;
        break;
    }
  } else {
    if (pUIChild && pUIChild->GetElementType() == XFA_Element::DefaultUi) {
      eWidgetType = XFA_Element::TextEdit;
    } else {
      eWidgetType =
          pUIChild ? pUIChild->GetElementType()
                   : (eUIType == XFA_Element::Unknown ? XFA_Element::TextEdit
                                                      : eUIType);
    }
  }

  if (!pUIChild) {
    if (eUIType == XFA_Element::Unknown) {
      eUIType = XFA_Element::TextEdit;
      defValueData.GetNode()->JSNode()->GetProperty(0, XFA_Element::Text, true);
    }
    return pUI->JSNode()->GetProperty(0, eUIType, true);
  }

  if (eUIType != XFA_Element::Unknown)
    return pUIChild;

  switch (pUIChild->GetElementType()) {
    case XFA_Element::CheckButton: {
      eValueType = XFA_Element::Text;
      if (CXFA_Node* pItems = pNode->GetChild(0, XFA_Element::Items, false)) {
        if (CXFA_Node* pItem = pItems->GetChild(0, XFA_Element::Unknown, false))
          eValueType = pItem->GetElementType();
      }
      break;
    }
    case XFA_Element::DateTimeEdit:
      eValueType = XFA_Element::DateTime;
      break;
    case XFA_Element::ImageEdit:
      eValueType = XFA_Element::Image;
      break;
    case XFA_Element::NumericEdit:
      eValueType = XFA_Element::Float;
      break;
    case XFA_Element::ChoiceList: {
      eValueType = (pUIChild->JSNode()->GetEnum(XFA_Attribute::Open) ==
                    XFA_AttributeEnum::MultiSelect)
                       ? XFA_Element::ExData
                       : XFA_Element::Text;
      break;
    }
    case XFA_Element::Barcode:
    case XFA_Element::Button:
    case XFA_Element::PasswordEdit:
    case XFA_Element::Signature:
    case XFA_Element::TextEdit:
    default:
      eValueType = XFA_Element::Text;
      break;
  }
  defValueData.GetNode()->JSNode()->GetProperty(0, eValueType, true);

  return pUIChild;
}

}  // namespace

CXFA_WidgetData::CXFA_WidgetData(CXFA_Node* pNode)
    : CXFA_DataData(pNode),
      m_bIsNull(true),
      m_bPreNull(true),
      m_pUiChildNode(nullptr),
      m_eUIType(XFA_Element::Unknown) {}

CXFA_Node* CXFA_WidgetData::GetUIChild() {
  if (m_eUIType == XFA_Element::Unknown)
    m_pUiChildNode = CreateUIChild(m_pNode, m_eUIType);

  return m_pUiChildNode;
}

XFA_Element CXFA_WidgetData::GetUIType() {
  GetUIChild();
  return m_eUIType;
}

WideString CXFA_WidgetData::GetRawValue() const {
  return m_pNode->JSNode()->GetContent(false);
}

bool CXFA_WidgetData::IsOpenAccess() const {
  for (CXFA_Node* pNode = m_pNode; pNode;
       pNode = pNode->GetNodeItem(XFA_NODEITEM_Parent,
                                  XFA_ObjectType::ContainerNode)) {
    XFA_AttributeEnum iAcc = pNode->JSNode()->GetEnum(XFA_Attribute::Access);
    if (iAcc != XFA_AttributeEnum::Open)
      return false;
  }
  return true;
}

int32_t CXFA_WidgetData::GetRotate() const {
  pdfium::Optional<int32_t> degrees =
      m_pNode->JSNode()->TryInteger(XFA_Attribute::Rotate, false);
  return degrees ? XFA_MapRotation(*degrees) / 90 * 90 : 0;
}

CXFA_BorderData CXFA_WidgetData::GetBorderData(bool bModified) {
  return CXFA_BorderData(
      m_pNode->JSNode()->GetProperty(0, XFA_Element::Border, bModified));
}

CXFA_CaptionData CXFA_WidgetData::GetCaptionData() {
  return CXFA_CaptionData(
      m_pNode->JSNode()->GetProperty(0, XFA_Element::Caption, false));
}

CXFA_FontData CXFA_WidgetData::GetFontData(bool bModified) {
  return CXFA_FontData(
      m_pNode->JSNode()->GetProperty(0, XFA_Element::Font, bModified));
}

CXFA_MarginData CXFA_WidgetData::GetMarginData() {
  return CXFA_MarginData(
      m_pNode->JSNode()->GetProperty(0, XFA_Element::Margin, false));
}

CXFA_ParaData CXFA_WidgetData::GetParaData() {
  return CXFA_ParaData(
      m_pNode->JSNode()->GetProperty(0, XFA_Element::Para, false));
}

std::vector<CXFA_Node*> CXFA_WidgetData::GetEventList() {
  return m_pNode->GetNodeList(0, XFA_Element::Event);
}

std::vector<CXFA_Node*> CXFA_WidgetData::GetEventByActivity(
    XFA_AttributeEnum iActivity,
    bool bIsFormReady) {
  std::vector<CXFA_Node*> events;
  for (CXFA_Node* pNode : GetEventList()) {
    CXFA_EventData eventData(pNode);
    if (eventData.GetActivity() == iActivity) {
      if (iActivity == XFA_AttributeEnum::Ready) {
        WideString wsRef = eventData.GetRef();
        if (bIsFormReady) {
          if (wsRef == WideStringView(L"$form"))
            events.push_back(pNode);
        } else {
          if (wsRef == WideStringView(L"$layout"))
            events.push_back(pNode);
        }
      } else {
        events.push_back(pNode);
      }
    }
  }
  return events;
}

CXFA_ValueData CXFA_WidgetData::GetDefaultValueData() {
  CXFA_Node* pTemNode = m_pNode->GetTemplateNode();
  return CXFA_ValueData(
      pTemNode ? pTemNode->JSNode()->GetProperty(0, XFA_Element::Value, false)
               : nullptr);
}

CXFA_ValueData CXFA_WidgetData::GetFormValueData() {
  return CXFA_ValueData(
      m_pNode->JSNode()->GetProperty(0, XFA_Element::Value, false));
}

CXFA_CalculateData CXFA_WidgetData::GetCalculateData() {
  return CXFA_CalculateData(
      m_pNode->JSNode()->GetProperty(0, XFA_Element::Calculate, false));
}

CXFA_ValidateData CXFA_WidgetData::GetValidateData(bool bModified) {
  return CXFA_ValidateData(
      m_pNode->JSNode()->GetProperty(0, XFA_Element::Validate, bModified));
}

CXFA_BindData CXFA_WidgetData::GetBindData() {
  return CXFA_BindData(
      m_pNode->JSNode()->GetProperty(0, XFA_Element::Bind, false));
}

pdfium::Optional<float> CXFA_WidgetData::TryWidth() {
  return TryMeasureAsFloat(XFA_Attribute::W);
}

pdfium::Optional<float> CXFA_WidgetData::TryHeight() {
  return TryMeasureAsFloat(XFA_Attribute::H);
}

pdfium::Optional<float> CXFA_WidgetData::TryMinWidth() {
  return TryMeasureAsFloat(XFA_Attribute::MinW);
}

pdfium::Optional<float> CXFA_WidgetData::TryMinHeight() {
  return TryMeasureAsFloat(XFA_Attribute::MinH);
}

pdfium::Optional<float> CXFA_WidgetData::TryMaxWidth() {
  return TryMeasureAsFloat(XFA_Attribute::MaxW);
}

pdfium::Optional<float> CXFA_WidgetData::TryMaxHeight() {
  return TryMeasureAsFloat(XFA_Attribute::MaxH);
}

CXFA_BorderData CXFA_WidgetData::GetUIBorderData() {
  CXFA_Node* pUIChild = GetUIChild();
  return CXFA_BorderData(
      pUIChild ? pUIChild->JSNode()->GetProperty(0, XFA_Element::Border, false)
               : nullptr);
}

CFX_RectF CXFA_WidgetData::GetUIMargin() {
  CXFA_Node* pUIChild = GetUIChild();
  CXFA_MarginData mgUI = CXFA_MarginData(
      pUIChild ? pUIChild->JSNode()->GetProperty(0, XFA_Element::Margin, false)
               : nullptr);

  if (!mgUI.HasValidNode())
    return CFX_RectF();

  CXFA_BorderData borderData = GetUIBorderData();
  if (borderData.HasValidNode() &&
      borderData.GetPresence() != XFA_AttributeEnum::Visible) {
    return CFX_RectF();
  }

  pdfium::Optional<float> left = mgUI.TryLeftInset();
  pdfium::Optional<float> top = mgUI.TryTopInset();
  pdfium::Optional<float> right = mgUI.TryRightInset();
  pdfium::Optional<float> bottom = mgUI.TryBottomInset();
  if (borderData.HasValidNode()) {
    bool bVisible = false;
    float fThickness = 0;
    XFA_AttributeEnum iType = XFA_AttributeEnum::Unknown;
    std::tie(iType, bVisible, fThickness) = borderData.Get3DStyle();
    if (!left || !top || !right || !bottom) {
      std::vector<CXFA_StrokeData> strokes = borderData.GetStrokes();
      if (!top)
        top = GetEdgeThickness(strokes, bVisible, 0);
      if (!right)
        right = GetEdgeThickness(strokes, bVisible, 1);
      if (!bottom)
        bottom = GetEdgeThickness(strokes, bVisible, 2);
      if (!left)
        left = GetEdgeThickness(strokes, bVisible, 3);
    }
  }
  return CFX_RectF(left.value_or(0.0), top.value_or(0.0), right.value_or(0.0),
                   bottom.value_or(0.0));
}

XFA_AttributeEnum CXFA_WidgetData::GetButtonHighlight() {
  CXFA_Node* pUIChild = GetUIChild();
  if (pUIChild)
    return pUIChild->JSNode()->GetEnum(XFA_Attribute::Highlight);
  return XFA_AttributeEnum::Inverted;
}

bool CXFA_WidgetData::HasButtonRollover() const {
  CXFA_Node* pItems = m_pNode->GetChild(0, XFA_Element::Items, false);
  if (!pItems)
    return false;

  for (CXFA_Node* pText = pItems->GetNodeItem(XFA_NODEITEM_FirstChild); pText;
       pText = pText->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pText->JSNode()->GetCData(XFA_Attribute::Name) == L"rollover")
      return !pText->JSNode()->GetContent(false).IsEmpty();
  }
  return false;
}

bool CXFA_WidgetData::HasButtonDown() const {
  CXFA_Node* pItems = m_pNode->GetChild(0, XFA_Element::Items, false);
  if (!pItems)
    return false;

  for (CXFA_Node* pText = pItems->GetNodeItem(XFA_NODEITEM_FirstChild); pText;
       pText = pText->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pText->JSNode()->GetCData(XFA_Attribute::Name) == L"down")
      return !pText->JSNode()->GetContent(false).IsEmpty();
  }
  return false;
}

bool CXFA_WidgetData::IsCheckButtonRound() {
  CXFA_Node* pUIChild = GetUIChild();
  if (pUIChild)
    return pUIChild->JSNode()->GetEnum(XFA_Attribute::Shape) ==
           XFA_AttributeEnum::Round;
  return false;
}

XFA_AttributeEnum CXFA_WidgetData::GetCheckButtonMark() {
  CXFA_Node* pUIChild = GetUIChild();
  if (pUIChild)
    return pUIChild->JSNode()->GetEnum(XFA_Attribute::Mark);
  return XFA_AttributeEnum::Default;
}

bool CXFA_WidgetData::IsRadioButton() {
  CXFA_Node* pParent = m_pNode->GetNodeItem(XFA_NODEITEM_Parent);
  return pParent && pParent->GetElementType() == XFA_Element::ExclGroup;
}

float CXFA_WidgetData::GetCheckButtonSize() {
  CXFA_Node* pUIChild = GetUIChild();
  if (pUIChild) {
    return pUIChild->JSNode()
        ->GetMeasure(XFA_Attribute::Size)
        .ToUnit(XFA_Unit::Pt);
  }
  return CXFA_Measurement(10, XFA_Unit::Pt).ToUnit(XFA_Unit::Pt);
}

bool CXFA_WidgetData::IsAllowNeutral() {
  CXFA_Node* pUIChild = GetUIChild();
  return pUIChild &&
         pUIChild->JSNode()->GetBoolean(XFA_Attribute::AllowNeutral);
}

XFA_CHECKSTATE CXFA_WidgetData::GetCheckState() {
  WideString wsValue = GetRawValue();
  if (wsValue.IsEmpty())
    return XFA_CHECKSTATE_Off;

  if (CXFA_Node* pItems = m_pNode->GetChild(0, XFA_Element::Items, false)) {
    CXFA_Node* pText = pItems->GetNodeItem(XFA_NODEITEM_FirstChild);
    int32_t i = 0;
    while (pText) {
      pdfium::Optional<WideString> wsContent =
          pText->JSNode()->TryContent(false, true);
      if (wsContent && *wsContent == wsValue)
        return static_cast<XFA_CHECKSTATE>(i);

      i++;
      pText = pText->GetNodeItem(XFA_NODEITEM_NextSibling);
    }
  }
  return XFA_CHECKSTATE_Off;
}

void CXFA_WidgetData::SetCheckState(XFA_CHECKSTATE eCheckState, bool bNotify) {
  CXFA_WidgetData exclGroup(GetExclGroupNode());
  if (exclGroup.HasValidNode()) {
    WideString wsValue;
    if (eCheckState != XFA_CHECKSTATE_Off) {
      if (CXFA_Node* pItems = m_pNode->GetChild(0, XFA_Element::Items, false)) {
        CXFA_Node* pText = pItems->GetNodeItem(XFA_NODEITEM_FirstChild);
        if (pText)
          wsValue = pText->JSNode()->GetContent(false);
      }
    }
    CXFA_Node* pChild =
        exclGroup.GetNode()->GetNodeItem(XFA_NODEITEM_FirstChild);
    for (; pChild; pChild = pChild->GetNodeItem(XFA_NODEITEM_NextSibling)) {
      if (pChild->GetElementType() != XFA_Element::Field)
        continue;

      CXFA_Node* pItem = pChild->GetChild(0, XFA_Element::Items, false);
      if (!pItem)
        continue;

      CXFA_Node* pItemchild = pItem->GetNodeItem(XFA_NODEITEM_FirstChild);
      if (!pItemchild)
        continue;

      WideString text = pItemchild->JSNode()->GetContent(false);
      WideString wsChildValue = text;
      if (wsValue != text) {
        pItemchild = pItemchild->GetNodeItem(XFA_NODEITEM_NextSibling);
        if (pItemchild)
          wsChildValue = pItemchild->JSNode()->GetContent(false);
        else
          wsChildValue.clear();
      }
      CXFA_WidgetData ch(pChild);
      ch.SyncValue(wsChildValue, bNotify);
    }
    exclGroup.SyncValue(wsValue, bNotify);
  } else {
    CXFA_Node* pItems = m_pNode->GetChild(0, XFA_Element::Items, false);
    if (!pItems)
      return;

    int32_t i = -1;
    CXFA_Node* pText = pItems->GetNodeItem(XFA_NODEITEM_FirstChild);
    WideString wsContent;
    while (pText) {
      i++;
      if (i == eCheckState) {
        wsContent = pText->JSNode()->GetContent(false);
        break;
      }
      pText = pText->GetNodeItem(XFA_NODEITEM_NextSibling);
    }
    SyncValue(wsContent, bNotify);
  }
}

CXFA_Node* CXFA_WidgetData::GetExclGroupNode() {
  CXFA_Node* pExcl = ToNode(m_pNode->GetNodeItem(XFA_NODEITEM_Parent));
  if (!pExcl || pExcl->GetElementType() != XFA_Element::ExclGroup)
    return nullptr;
  return pExcl;
}

CXFA_Node* CXFA_WidgetData::GetSelectedMember() {
  CXFA_Node* pSelectedMember = nullptr;
  WideString wsState = GetRawValue();
  if (wsState.IsEmpty())
    return pSelectedMember;

  for (CXFA_Node* pNode = ToNode(m_pNode->GetNodeItem(XFA_NODEITEM_FirstChild));
       pNode; pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    CXFA_WidgetData widgetData(pNode);
    if (widgetData.GetCheckState() == XFA_CHECKSTATE_On) {
      pSelectedMember = pNode;
      break;
    }
  }
  return pSelectedMember;
}

CXFA_Node* CXFA_WidgetData::SetSelectedMember(const WideStringView& wsName,
                                              bool bNotify) {
  uint32_t nameHash = FX_HashCode_GetW(wsName, false);
  for (CXFA_Node* pNode = ToNode(m_pNode->GetNodeItem(XFA_NODEITEM_FirstChild));
       pNode; pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pNode->GetNameHash() == nameHash) {
      CXFA_WidgetData widgetData(pNode);
      widgetData.SetCheckState(XFA_CHECKSTATE_On, bNotify);
      return pNode;
    }
  }
  return nullptr;
}

void CXFA_WidgetData::SetSelectedMemberByValue(const WideStringView& wsValue,
                                               bool bNotify,
                                               bool bScriptModify,
                                               bool bSyncData) {
  WideString wsExclGroup;
  for (CXFA_Node* pNode = m_pNode->GetNodeItem(XFA_NODEITEM_FirstChild); pNode;
       pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pNode->GetElementType() != XFA_Element::Field)
      continue;

    CXFA_Node* pItem = pNode->GetChild(0, XFA_Element::Items, false);
    if (!pItem)
      continue;

    CXFA_Node* pItemchild = pItem->GetNodeItem(XFA_NODEITEM_FirstChild);
    if (!pItemchild)
      continue;

    WideString wsChildValue = pItemchild->JSNode()->GetContent(false);
    if (wsValue != wsChildValue) {
      pItemchild = pItemchild->GetNodeItem(XFA_NODEITEM_NextSibling);
      if (pItemchild)
        wsChildValue = pItemchild->JSNode()->GetContent(false);
      else
        wsChildValue.clear();
    } else {
      wsExclGroup = wsValue;
    }
    pNode->JSNode()->SetContent(wsChildValue, wsChildValue, bNotify,
                                bScriptModify, false);
  }
  if (m_pNode) {
    m_pNode->JSNode()->SetContent(wsExclGroup, wsExclGroup, bNotify,
                                  bScriptModify, bSyncData);
  }
}

CXFA_Node* CXFA_WidgetData::GetExclGroupFirstMember() {
  CXFA_Node* pExcl = GetNode();
  if (!pExcl)
    return nullptr;

  CXFA_Node* pNode = pExcl->GetNodeItem(XFA_NODEITEM_FirstChild);
  while (pNode) {
    if (pNode->GetElementType() == XFA_Element::Field)
      return pNode;

    pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling);
  }
  return nullptr;
}
CXFA_Node* CXFA_WidgetData::GetExclGroupNextMember(CXFA_Node* pNode) {
  if (!pNode)
    return nullptr;

  CXFA_Node* pNodeField = pNode->GetNodeItem(XFA_NODEITEM_NextSibling);
  while (pNodeField) {
    if (pNodeField->GetElementType() == XFA_Element::Field)
      return pNodeField;

    pNodeField = pNodeField->GetNodeItem(XFA_NODEITEM_NextSibling);
  }
  return nullptr;
}

bool CXFA_WidgetData::IsChoiceListCommitOnSelect() {
  CXFA_Node* pUIChild = GetUIChild();
  if (pUIChild) {
    return pUIChild->JSNode()->GetEnum(XFA_Attribute::CommitOn) ==
           XFA_AttributeEnum::Select;
  }
  return true;
}

bool CXFA_WidgetData::IsChoiceListAllowTextEntry() {
  CXFA_Node* pUIChild = GetUIChild();
  return pUIChild && pUIChild->JSNode()->GetBoolean(XFA_Attribute::TextEntry);
}

bool CXFA_WidgetData::IsChoiceListMultiSelect() {
  CXFA_Node* pUIChild = GetUIChild();
  if (pUIChild) {
    return pUIChild->JSNode()->GetEnum(XFA_Attribute::Open) ==
           XFA_AttributeEnum::MultiSelect;
  }
  return false;
}

bool CXFA_WidgetData::IsListBox() {
  CXFA_Node* pUIChild = GetUIChild();
  if (!pUIChild)
    return false;

  XFA_AttributeEnum attr = pUIChild->JSNode()->GetEnum(XFA_Attribute::Open);
  return attr == XFA_AttributeEnum::Always ||
         attr == XFA_AttributeEnum::MultiSelect;
}

int32_t CXFA_WidgetData::CountChoiceListItems(bool bSaveValue) {
  std::vector<CXFA_Node*> pItems;
  int32_t iCount = 0;
  for (CXFA_Node* pNode = m_pNode->GetNodeItem(XFA_NODEITEM_FirstChild); pNode;
       pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pNode->GetElementType() != XFA_Element::Items)
      continue;
    iCount++;
    pItems.push_back(pNode);
    if (iCount == 2)
      break;
  }
  if (iCount == 0)
    return 0;

  CXFA_Node* pItem = pItems[0];
  if (iCount > 1) {
    bool bItemOneHasSave = pItems[0]->JSNode()->GetBoolean(XFA_Attribute::Save);
    bool bItemTwoHasSave = pItems[1]->JSNode()->GetBoolean(XFA_Attribute::Save);
    if (bItemOneHasSave != bItemTwoHasSave && bSaveValue == bItemTwoHasSave)
      pItem = pItems[1];
  }
  return pItem->CountChildren(XFA_Element::Unknown, false);
}

pdfium::Optional<WideString> CXFA_WidgetData::GetChoiceListItem(
    int32_t nIndex,
    bool bSaveValue) {
  std::vector<CXFA_Node*> pItemsArray;
  int32_t iCount = 0;
  for (CXFA_Node* pNode = m_pNode->GetNodeItem(XFA_NODEITEM_FirstChild); pNode;
       pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pNode->GetElementType() != XFA_Element::Items)
      continue;

    ++iCount;
    pItemsArray.push_back(pNode);
    if (iCount == 2)
      break;
  }
  if (iCount == 0)
    return {};

  CXFA_Node* pItems = pItemsArray[0];
  if (iCount > 1) {
    bool bItemOneHasSave =
        pItemsArray[0]->JSNode()->GetBoolean(XFA_Attribute::Save);
    bool bItemTwoHasSave =
        pItemsArray[1]->JSNode()->GetBoolean(XFA_Attribute::Save);
    if (bItemOneHasSave != bItemTwoHasSave && bSaveValue == bItemTwoHasSave)
      pItems = pItemsArray[1];
  }
  if (!pItems)
    return {};

  CXFA_Node* pItem = pItems->GetChild(nIndex, XFA_Element::Unknown, false);
  if (pItem)
    return {pItem->JSNode()->GetContent(false)};
  return {};
}

std::vector<WideString> CXFA_WidgetData::GetChoiceListItems(bool bSaveValue) {
  std::vector<CXFA_Node*> items;
  for (CXFA_Node* pNode = m_pNode->GetNodeItem(XFA_NODEITEM_FirstChild);
       pNode && items.size() < 2;
       pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pNode->GetElementType() == XFA_Element::Items)
      items.push_back(pNode);
  }
  if (items.empty())
    return std::vector<WideString>();

  CXFA_Node* pItem = items.front();
  if (items.size() > 1) {
    bool bItemOneHasSave = items[0]->JSNode()->GetBoolean(XFA_Attribute::Save);
    bool bItemTwoHasSave = items[1]->JSNode()->GetBoolean(XFA_Attribute::Save);
    if (bItemOneHasSave != bItemTwoHasSave && bSaveValue == bItemTwoHasSave)
      pItem = items[1];
  }

  std::vector<WideString> wsTextArray;
  for (CXFA_Node* pNode = pItem->GetNodeItem(XFA_NODEITEM_FirstChild); pNode;
       pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    wsTextArray.emplace_back(pNode->JSNode()->GetContent(false));
  }
  return wsTextArray;
}

int32_t CXFA_WidgetData::CountSelectedItems() {
  std::vector<WideString> wsValueArray = GetSelectedItemsValue();
  if (IsListBox() || !IsChoiceListAllowTextEntry())
    return pdfium::CollectionSize<int32_t>(wsValueArray);

  int32_t iSelected = 0;
  std::vector<WideString> wsSaveTextArray = GetChoiceListItems(true);
  for (const auto& value : wsValueArray) {
    if (pdfium::ContainsValue(wsSaveTextArray, value))
      iSelected++;
  }
  return iSelected;
}

int32_t CXFA_WidgetData::GetSelectedItem(int32_t nIndex) {
  std::vector<WideString> wsValueArray = GetSelectedItemsValue();
  if (!pdfium::IndexInBounds(wsValueArray, nIndex))
    return -1;

  std::vector<WideString> wsSaveTextArray = GetChoiceListItems(true);
  auto it = std::find(wsSaveTextArray.begin(), wsSaveTextArray.end(),
                      wsValueArray[nIndex]);
  return it != wsSaveTextArray.end() ? it - wsSaveTextArray.begin() : -1;
}

std::vector<int32_t> CXFA_WidgetData::GetSelectedItems() {
  std::vector<int32_t> iSelArray;
  std::vector<WideString> wsValueArray = GetSelectedItemsValue();
  std::vector<WideString> wsSaveTextArray = GetChoiceListItems(true);
  for (const auto& value : wsValueArray) {
    auto it = std::find(wsSaveTextArray.begin(), wsSaveTextArray.end(), value);
    if (it != wsSaveTextArray.end())
      iSelArray.push_back(it - wsSaveTextArray.begin());
  }
  return iSelArray;
}

std::vector<WideString> CXFA_WidgetData::GetSelectedItemsValue() {
  std::vector<WideString> wsSelTextArray;
  WideString wsValue = GetRawValue();
  if (IsChoiceListMultiSelect()) {
    if (!wsValue.IsEmpty()) {
      size_t iStart = 0;
      size_t iLength = wsValue.GetLength();
      auto iEnd = wsValue.Find(L'\n', iStart);
      iEnd = (!iEnd.has_value()) ? iLength : iEnd;
      while (iEnd >= iStart) {
        wsSelTextArray.push_back(wsValue.Mid(iStart, iEnd.value() - iStart));
        iStart = iEnd.value() + 1;
        if (iStart >= iLength)
          break;
        iEnd = wsValue.Find(L'\n', iStart);
        if (!iEnd.has_value())
          wsSelTextArray.push_back(wsValue.Mid(iStart, iLength - iStart));
      }
    }
  } else {
    wsSelTextArray.push_back(wsValue);
  }
  return wsSelTextArray;
}

bool CXFA_WidgetData::GetItemState(int32_t nIndex) {
  std::vector<WideString> wsSaveTextArray = GetChoiceListItems(true);
  return pdfium::IndexInBounds(wsSaveTextArray, nIndex) &&
         pdfium::ContainsValue(GetSelectedItemsValue(),
                               wsSaveTextArray[nIndex]);
}

void CXFA_WidgetData::SetItemState(int32_t nIndex,
                                   bool bSelected,
                                   bool bNotify,
                                   bool bScriptModify,
                                   bool bSyncData) {
  std::vector<WideString> wsSaveTextArray = GetChoiceListItems(true);
  if (!pdfium::IndexInBounds(wsSaveTextArray, nIndex))
    return;

  int32_t iSel = -1;
  std::vector<WideString> wsValueArray = GetSelectedItemsValue();
  auto it = std::find(wsValueArray.begin(), wsValueArray.end(),
                      wsSaveTextArray[nIndex]);
  if (it != wsValueArray.end())
    iSel = it - wsValueArray.begin();

  if (IsChoiceListMultiSelect()) {
    if (bSelected) {
      if (iSel < 0) {
        WideString wsValue = GetRawValue();
        if (!wsValue.IsEmpty()) {
          wsValue += L"\n";
        }
        wsValue += wsSaveTextArray[nIndex];
        m_pNode->JSNode()->SetContent(wsValue, wsValue, bNotify, bScriptModify,
                                      bSyncData);
      }
    } else if (iSel >= 0) {
      std::vector<int32_t> iSelArray = GetSelectedItems();
      auto it = std::find(iSelArray.begin(), iSelArray.end(), nIndex);
      if (it != iSelArray.end())
        iSelArray.erase(it);
      SetSelectedItems(iSelArray, bNotify, bScriptModify, bSyncData);
    }
  } else {
    if (bSelected) {
      if (iSel < 0) {
        WideString wsSaveText = wsSaveTextArray[nIndex];
        m_pNode->JSNode()->SetContent(wsSaveText,
                                      GetFormatDataValue(wsSaveText), bNotify,
                                      bScriptModify, bSyncData);
      }
    } else if (iSel >= 0) {
      m_pNode->JSNode()->SetContent(WideString(), WideString(), bNotify,
                                    bScriptModify, bSyncData);
    }
  }
}

void CXFA_WidgetData::SetSelectedItems(const std::vector<int32_t>& iSelArray,
                                       bool bNotify,
                                       bool bScriptModify,
                                       bool bSyncData) {
  WideString wsValue;
  int32_t iSize = pdfium::CollectionSize<int32_t>(iSelArray);
  if (iSize >= 1) {
    std::vector<WideString> wsSaveTextArray = GetChoiceListItems(true);
    WideString wsItemValue;
    for (int32_t i = 0; i < iSize; i++) {
      wsItemValue = (iSize == 1) ? wsSaveTextArray[iSelArray[i]]
                                 : wsSaveTextArray[iSelArray[i]] + L"\n";
      wsValue += wsItemValue;
    }
  }
  WideString wsFormat(wsValue);
  if (!IsChoiceListMultiSelect())
    wsFormat = GetFormatDataValue(wsValue);

  m_pNode->JSNode()->SetContent(wsValue, wsFormat, bNotify, bScriptModify,
                                bSyncData);
}

void CXFA_WidgetData::ClearAllSelections() {
  CXFA_Node* pBind = m_pNode->GetBindData();
  if (!pBind || !IsChoiceListMultiSelect()) {
    SyncValue(WideString(), false);
    return;
  }

  while (CXFA_Node* pChildNode = pBind->GetNodeItem(XFA_NODEITEM_FirstChild))
    pBind->RemoveChild(pChildNode, true);
}

void CXFA_WidgetData::InsertItem(const WideString& wsLabel,
                                 const WideString& wsValue,
                                 bool bNotify) {
  int32_t nIndex = -1;
  WideString wsNewValue(wsValue);
  if (wsNewValue.IsEmpty())
    wsNewValue = wsLabel;

  std::vector<CXFA_Node*> listitems;
  for (CXFA_Node* pItem = m_pNode->GetNodeItem(XFA_NODEITEM_FirstChild); pItem;
       pItem = pItem->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pItem->GetElementType() == XFA_Element::Items)
      listitems.push_back(pItem);
  }
  if (listitems.empty()) {
    CXFA_Node* pItems = m_pNode->CreateSamePacketNode(XFA_Element::Items);
    m_pNode->InsertChild(-1, pItems);
    InsertListTextItem(pItems, wsLabel, nIndex);
    CXFA_Node* pSaveItems = m_pNode->CreateSamePacketNode(XFA_Element::Items);
    m_pNode->InsertChild(-1, pSaveItems);
    pSaveItems->JSNode()->SetBoolean(XFA_Attribute::Save, true, false);
    InsertListTextItem(pSaveItems, wsNewValue, nIndex);
  } else if (listitems.size() > 1) {
    for (int32_t i = 0; i < 2; i++) {
      CXFA_Node* pNode = listitems[i];
      bool bHasSave = pNode->JSNode()->GetBoolean(XFA_Attribute::Save);
      if (bHasSave)
        InsertListTextItem(pNode, wsNewValue, nIndex);
      else
        InsertListTextItem(pNode, wsLabel, nIndex);
    }
  } else {
    CXFA_Node* pNode = listitems[0];
    pNode->JSNode()->SetBoolean(XFA_Attribute::Save, false, false);
    pNode->JSNode()->SetEnum(XFA_Attribute::Presence,
                             XFA_AttributeEnum::Visible, false);
    CXFA_Node* pSaveItems = m_pNode->CreateSamePacketNode(XFA_Element::Items);
    m_pNode->InsertChild(-1, pSaveItems);
    pSaveItems->JSNode()->SetBoolean(XFA_Attribute::Save, true, false);
    pSaveItems->JSNode()->SetEnum(XFA_Attribute::Presence,
                                  XFA_AttributeEnum::Hidden, false);
    CXFA_Node* pListNode = pNode->GetNodeItem(XFA_NODEITEM_FirstChild);
    int32_t i = 0;
    while (pListNode) {
      InsertListTextItem(pSaveItems, pListNode->JSNode()->GetContent(false), i);
      ++i;

      pListNode = pListNode->GetNodeItem(XFA_NODEITEM_NextSibling);
    }
    InsertListTextItem(pNode, wsLabel, nIndex);
    InsertListTextItem(pSaveItems, wsNewValue, nIndex);
  }
  if (!bNotify)
    return;

  m_pNode->GetDocument()->GetNotify()->OnWidgetListItemAdded(
      this, wsLabel.c_str(), wsValue.c_str(), nIndex);
}

void CXFA_WidgetData::GetItemLabel(const WideStringView& wsValue,
                                   WideString& wsLabel) {
  int32_t iCount = 0;
  std::vector<CXFA_Node*> listitems;
  CXFA_Node* pItems = m_pNode->GetNodeItem(XFA_NODEITEM_FirstChild);
  for (; pItems; pItems = pItems->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pItems->GetElementType() != XFA_Element::Items)
      continue;
    iCount++;
    listitems.push_back(pItems);
  }

  if (iCount <= 1) {
    wsLabel = wsValue;
    return;
  }

  CXFA_Node* pLabelItems = listitems[0];
  bool bSave = pLabelItems->JSNode()->GetBoolean(XFA_Attribute::Save);
  CXFA_Node* pSaveItems = nullptr;
  if (bSave) {
    pSaveItems = pLabelItems;
    pLabelItems = listitems[1];
  } else {
    pSaveItems = listitems[1];
  }
  iCount = 0;

  int32_t iSearch = -1;
  for (CXFA_Node* pChildItem = pSaveItems->GetNodeItem(XFA_NODEITEM_FirstChild);
       pChildItem;
       pChildItem = pChildItem->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pChildItem->JSNode()->GetContent(false) == wsValue) {
      iSearch = iCount;
      break;
    }
    iCount++;
  }
  if (iSearch < 0)
    return;

  CXFA_Node* pText =
      pLabelItems->GetChild(iSearch, XFA_Element::Unknown, false);
  if (pText)
    wsLabel = pText->JSNode()->GetContent(false);
}

WideString CXFA_WidgetData::GetItemValue(const WideStringView& wsLabel) {
  int32_t iCount = 0;
  std::vector<CXFA_Node*> listitems;
  for (CXFA_Node* pItems = m_pNode->GetNodeItem(XFA_NODEITEM_FirstChild);
       pItems; pItems = pItems->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pItems->GetElementType() != XFA_Element::Items)
      continue;
    iCount++;
    listitems.push_back(pItems);
  }
  if (iCount <= 1)
    return WideString(wsLabel);

  CXFA_Node* pLabelItems = listitems[0];
  bool bSave = pLabelItems->JSNode()->GetBoolean(XFA_Attribute::Save);
  CXFA_Node* pSaveItems = nullptr;
  if (bSave) {
    pSaveItems = pLabelItems;
    pLabelItems = listitems[1];
  } else {
    pSaveItems = listitems[1];
  }
  iCount = 0;

  int32_t iSearch = -1;
  WideString wsContent;
  CXFA_Node* pChildItem = pLabelItems->GetNodeItem(XFA_NODEITEM_FirstChild);
  for (; pChildItem;
       pChildItem = pChildItem->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pChildItem->JSNode()->GetContent(false) == wsLabel) {
      iSearch = iCount;
      break;
    }
    iCount++;
  }
  if (iSearch < 0)
    return L"";

  CXFA_Node* pText = pSaveItems->GetChild(iSearch, XFA_Element::Unknown, false);
  return pText ? pText->JSNode()->GetContent(false) : L"";
}

bool CXFA_WidgetData::DeleteItem(int32_t nIndex,
                                 bool bNotify,
                                 bool bScriptModify) {
  bool bSetValue = false;
  CXFA_Node* pItems = m_pNode->GetNodeItem(XFA_NODEITEM_FirstChild);
  for (; pItems; pItems = pItems->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pItems->GetElementType() != XFA_Element::Items)
      continue;

    if (nIndex < 0) {
      while (CXFA_Node* pNode = pItems->GetNodeItem(XFA_NODEITEM_FirstChild)) {
        pItems->RemoveChild(pNode, true);
      }
    } else {
      if (!bSetValue && pItems->JSNode()->GetBoolean(XFA_Attribute::Save)) {
        SetItemState(nIndex, false, true, bScriptModify, true);
        bSetValue = true;
      }
      int32_t i = 0;
      CXFA_Node* pNode = pItems->GetNodeItem(XFA_NODEITEM_FirstChild);
      while (pNode) {
        if (i == nIndex) {
          pItems->RemoveChild(pNode, true);
          break;
        }
        i++;
        pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling);
      }
    }
  }
  if (bNotify)
    m_pNode->GetDocument()->GetNotify()->OnWidgetListItemRemoved(this, nIndex);
  return true;
}

bool CXFA_WidgetData::IsHorizontalScrollPolicyOff() {
  CXFA_Node* pUIChild = GetUIChild();
  if (pUIChild) {
    return pUIChild->JSNode()->GetEnum(XFA_Attribute::HScrollPolicy) ==
           XFA_AttributeEnum::Off;
  }
  return false;
}

bool CXFA_WidgetData::IsVerticalScrollPolicyOff() {
  CXFA_Node* pUIChild = GetUIChild();
  if (pUIChild) {
    return pUIChild->JSNode()->GetEnum(XFA_Attribute::VScrollPolicy) ==
           XFA_AttributeEnum::Off;
  }
  return false;
}

pdfium::Optional<int32_t> CXFA_WidgetData::GetNumberOfCells() {
  CXFA_Node* pUIChild = GetUIChild();
  if (!pUIChild)
    return {};
  if (CXFA_Node* pNode = pUIChild->GetChild(0, XFA_Element::Comb, false))
    return {pNode->JSNode()->GetInteger(XFA_Attribute::NumberOfCells)};
  return {};
}

WideString CXFA_WidgetData::GetBarcodeType() {
  CXFA_Node* pUIChild = GetUIChild();
  return pUIChild
             ? WideString(pUIChild->JSNode()->GetCData(XFA_Attribute::Type))
             : WideString();
}

pdfium::Optional<BC_CHAR_ENCODING>
CXFA_WidgetData::GetBarcodeAttribute_CharEncoding() {
  pdfium::Optional<WideString> wsCharEncoding =
      GetUIChild()->JSNode()->TryCData(XFA_Attribute::CharEncoding, true);
  if (!wsCharEncoding)
    return {};
  if (wsCharEncoding->CompareNoCase(L"UTF-16"))
    return {CHAR_ENCODING_UNICODE};
  if (wsCharEncoding->CompareNoCase(L"UTF-8"))
    return {CHAR_ENCODING_UTF8};
  return {};
}

pdfium::Optional<bool> CXFA_WidgetData::GetBarcodeAttribute_Checksum() {
  pdfium::Optional<XFA_AttributeEnum> checksum =
      GetUIChild()->JSNode()->TryEnum(XFA_Attribute::Checksum, true);
  if (!checksum)
    return {};

  switch (*checksum) {
    case XFA_AttributeEnum::None:
      return {false};
    case XFA_AttributeEnum::Auto:
      return {true};
    case XFA_AttributeEnum::Checksum_1mod10:
    case XFA_AttributeEnum::Checksum_1mod10_1mod11:
    case XFA_AttributeEnum::Checksum_2mod10:
    default:
      break;
  }
  return {};
}

pdfium::Optional<int32_t> CXFA_WidgetData::GetBarcodeAttribute_DataLength() {
  pdfium::Optional<WideString> wsDataLength =
      GetUIChild()->JSNode()->TryCData(XFA_Attribute::DataLength, true);
  if (!wsDataLength)
    return {};

  return {FXSYS_wtoi(wsDataLength->c_str())};
}

pdfium::Optional<char> CXFA_WidgetData::GetBarcodeAttribute_StartChar() {
  pdfium::Optional<WideString> wsStartEndChar =
      GetUIChild()->JSNode()->TryCData(XFA_Attribute::StartChar, true);
  if (!wsStartEndChar || wsStartEndChar->IsEmpty())
    return {};

  return {static_cast<char>((*wsStartEndChar)[0])};
}

pdfium::Optional<char> CXFA_WidgetData::GetBarcodeAttribute_EndChar() {
  pdfium::Optional<WideString> wsStartEndChar =
      GetUIChild()->JSNode()->TryCData(XFA_Attribute::EndChar, true);
  if (!wsStartEndChar || wsStartEndChar->IsEmpty())
    return {};

  return {static_cast<char>((*wsStartEndChar)[0])};
}

pdfium::Optional<int32_t> CXFA_WidgetData::GetBarcodeAttribute_ECLevel() {
  pdfium::Optional<WideString> wsECLevel = GetUIChild()->JSNode()->TryCData(
      XFA_Attribute::ErrorCorrectionLevel, true);
  if (!wsECLevel)
    return {};
  return {FXSYS_wtoi(wsECLevel->c_str())};
}

pdfium::Optional<int32_t> CXFA_WidgetData::GetBarcodeAttribute_ModuleWidth() {
  pdfium::Optional<CXFA_Measurement> moduleWidthHeight =
      GetUIChild()->JSNode()->TryMeasure(XFA_Attribute::ModuleWidth, true);
  if (!moduleWidthHeight)
    return {};

  return {static_cast<int32_t>(moduleWidthHeight->ToUnit(XFA_Unit::Pt))};
}

pdfium::Optional<int32_t> CXFA_WidgetData::GetBarcodeAttribute_ModuleHeight() {
  pdfium::Optional<CXFA_Measurement> moduleWidthHeight =
      GetUIChild()->JSNode()->TryMeasure(XFA_Attribute::ModuleHeight, true);
  if (!moduleWidthHeight)
    return {};

  return {static_cast<int32_t>(moduleWidthHeight->ToUnit(XFA_Unit::Pt))};
}

pdfium::Optional<bool> CXFA_WidgetData::GetBarcodeAttribute_PrintChecksum() {
  return GetUIChild()->JSNode()->TryBoolean(XFA_Attribute::PrintCheckDigit,
                                            true);
}

pdfium::Optional<BC_TEXT_LOC>
CXFA_WidgetData::GetBarcodeAttribute_TextLocation() {
  pdfium::Optional<XFA_AttributeEnum> textLocation =
      GetUIChild()->JSNode()->TryEnum(XFA_Attribute::TextLocation, true);
  if (!textLocation)
    return {};

  switch (*textLocation) {
    case XFA_AttributeEnum::None:
      return {BC_TEXT_LOC_NONE};
    case XFA_AttributeEnum::Above:
      return {BC_TEXT_LOC_ABOVE};
    case XFA_AttributeEnum::Below:
      return {BC_TEXT_LOC_BELOW};
    case XFA_AttributeEnum::AboveEmbedded:
      return {BC_TEXT_LOC_ABOVEEMBED};
    case XFA_AttributeEnum::BelowEmbedded:
      return {BC_TEXT_LOC_BELOWEMBED};
    default:
      break;
  }
  return {};
}

pdfium::Optional<bool> CXFA_WidgetData::GetBarcodeAttribute_Truncate() {
  return GetUIChild()->JSNode()->TryBoolean(XFA_Attribute::Truncate, true);
}

pdfium::Optional<int8_t>
CXFA_WidgetData::GetBarcodeAttribute_WideNarrowRatio() {
  pdfium::Optional<WideString> wsWideNarrowRatio =
      GetUIChild()->JSNode()->TryCData(XFA_Attribute::WideNarrowRatio, true);
  if (!wsWideNarrowRatio)
    return {};

  pdfium::Optional<size_t> ptPos = wsWideNarrowRatio->Find(':');
  if (!ptPos)
    return {static_cast<int8_t>(FXSYS_wtoi(wsWideNarrowRatio->c_str()))};

  int32_t fB = FXSYS_wtoi(
      wsWideNarrowRatio->Right(wsWideNarrowRatio->GetLength() - (*ptPos + 1))
          .c_str());
  if (!fB)
    return {0};

  int32_t fA = FXSYS_wtoi(wsWideNarrowRatio->Left(*ptPos).c_str());
  float result = static_cast<float>(fA) / static_cast<float>(fB);
  return {static_cast<int8_t>(result)};
}

WideString CXFA_WidgetData::GetPasswordChar() {
  CXFA_Node* pUIChild = GetUIChild();
  return pUIChild ? pUIChild->JSNode()->GetCData(XFA_Attribute::PasswordChar)
                  : L"*";
}

bool CXFA_WidgetData::IsMultiLine() {
  CXFA_Node* pUIChild = GetUIChild();
  return pUIChild && pUIChild->JSNode()->GetBoolean(XFA_Attribute::MultiLine);
}

std::pair<XFA_Element, int32_t> CXFA_WidgetData::GetMaxChars() {
  if (CXFA_Node* pNode = m_pNode->GetChild(0, XFA_Element::Value, false)) {
    if (CXFA_Node* pChild = pNode->GetNodeItem(XFA_NODEITEM_FirstChild)) {
      switch (pChild->GetElementType()) {
        case XFA_Element::Text:
          return {XFA_Element::Text,
                  pChild->JSNode()->GetInteger(XFA_Attribute::MaxChars)};
        case XFA_Element::ExData: {
          int32_t iMax = pChild->JSNode()->GetInteger(XFA_Attribute::MaxLength);
          return {XFA_Element::ExData, iMax < 0 ? 0 : iMax};
        }
        default:
          break;
      }
    }
  }
  return {XFA_Element::Unknown, 0};
}

int32_t CXFA_WidgetData::GetFracDigits() {
  CXFA_Node* pNode = m_pNode->GetChild(0, XFA_Element::Value, false);
  if (!pNode)
    return -1;

  CXFA_Node* pChild = pNode->GetChild(0, XFA_Element::Decimal, false);
  if (!pChild)
    return -1;

  return pChild->JSNode()
      ->TryInteger(XFA_Attribute::FracDigits, true)
      .value_or(-1);
}

int32_t CXFA_WidgetData::GetLeadDigits() {
  CXFA_Node* pNode = m_pNode->GetChild(0, XFA_Element::Value, false);
  if (!pNode)
    return -1;

  CXFA_Node* pChild = pNode->GetChild(0, XFA_Element::Decimal, false);
  if (!pChild)
    return -1;

  return pChild->JSNode()
      ->TryInteger(XFA_Attribute::LeadDigits, true)
      .value_or(-1);
}

bool CXFA_WidgetData::SetValue(XFA_VALUEPICTURE eValueType,
                               const WideString& wsValue) {
  if (wsValue.IsEmpty()) {
    SyncValue(wsValue, true);
    return true;
  }

  m_bPreNull = m_bIsNull;
  m_bIsNull = false;
  WideString wsNewText(wsValue);
  WideString wsPicture = GetPictureContent(eValueType);
  bool bValidate = true;
  bool bSyncData = false;
  CXFA_Node* pNode = GetUIChild();
  if (!pNode)
    return true;

  XFA_Element eType = pNode->GetElementType();
  if (!wsPicture.IsEmpty()) {
    CXFA_LocaleMgr* pLocalMgr = m_pNode->GetDocument()->GetLocalMgr();
    IFX_Locale* pLocale = GetLocale();
    CXFA_LocaleValue widgetValue = XFA_GetLocaleValue(this);
    bValidate =
        widgetValue.ValidateValue(wsValue, wsPicture, pLocale, &wsPicture);
    if (bValidate) {
      widgetValue = CXFA_LocaleValue(widgetValue.GetType(), wsNewText,
                                     wsPicture, pLocale, pLocalMgr);
      wsNewText = widgetValue.GetValue();
      if (eType == XFA_Element::NumericEdit)
        wsNewText = NumericLimit(wsNewText, GetLeadDigits(), GetFracDigits());

      bSyncData = true;
    }
  } else {
    if (eType == XFA_Element::NumericEdit) {
      if (wsNewText != L"0")
        wsNewText = NumericLimit(wsNewText, GetLeadDigits(), GetFracDigits());

      bSyncData = true;
    }
  }
  if (eType != XFA_Element::NumericEdit || bSyncData)
    SyncValue(wsNewText, true);

  return bValidate;
}

WideString CXFA_WidgetData::GetPictureContent(XFA_VALUEPICTURE ePicture) {
  if (ePicture == XFA_VALUEPICTURE_Raw)
    return L"";

  CXFA_LocaleValue widgetValue = XFA_GetLocaleValue(this);
  switch (ePicture) {
    case XFA_VALUEPICTURE_Display: {
      if (CXFA_Node* pFormat =
              m_pNode->GetChild(0, XFA_Element::Format, false)) {
        if (CXFA_Node* pPicture =
                pFormat->GetChild(0, XFA_Element::Picture, false)) {
          pdfium::Optional<WideString> picture =
              pPicture->JSNode()->TryContent(false, true);
          if (picture)
            return *picture;
        }
      }

      IFX_Locale* pLocale = GetLocale();
      if (!pLocale)
        return L"";

      uint32_t dwType = widgetValue.GetType();
      switch (dwType) {
        case XFA_VT_DATE:
          return pLocale->GetDatePattern(FX_LOCALEDATETIMESUBCATEGORY_Medium);
        case XFA_VT_TIME:
          return pLocale->GetTimePattern(FX_LOCALEDATETIMESUBCATEGORY_Medium);
        case XFA_VT_DATETIME:
          return pLocale->GetDatePattern(FX_LOCALEDATETIMESUBCATEGORY_Medium) +
                 L"T" +
                 pLocale->GetTimePattern(FX_LOCALEDATETIMESUBCATEGORY_Medium);
        case XFA_VT_DECIMAL:
        case XFA_VT_FLOAT:
        default:
          return L"";
      }
    }
    case XFA_VALUEPICTURE_Edit: {
      CXFA_Node* pUI = m_pNode->GetChild(0, XFA_Element::Ui, false);
      if (pUI) {
        if (CXFA_Node* pPicture =
                pUI->GetChild(0, XFA_Element::Picture, false)) {
          pdfium::Optional<WideString> picture =
              pPicture->JSNode()->TryContent(false, true);
          if (picture)
            return *picture;
        }
      }

      IFX_Locale* pLocale = GetLocale();
      if (!pLocale)
        return L"";

      uint32_t dwType = widgetValue.GetType();
      switch (dwType) {
        case XFA_VT_DATE:
          return pLocale->GetDatePattern(FX_LOCALEDATETIMESUBCATEGORY_Short);
        case XFA_VT_TIME:
          return pLocale->GetTimePattern(FX_LOCALEDATETIMESUBCATEGORY_Short);
        case XFA_VT_DATETIME:
          return pLocale->GetDatePattern(FX_LOCALEDATETIMESUBCATEGORY_Short) +
                 L"T" +
                 pLocale->GetTimePattern(FX_LOCALEDATETIMESUBCATEGORY_Short);
        default:
          return L"";
      }
    }
    case XFA_VALUEPICTURE_DataBind: {
      CXFA_BindData bindData = GetBindData();
      if (bindData.HasValidNode())
        return bindData.GetPicture();
      break;
    }
    default:
      break;
  }
  return L"";
}

IFX_Locale* CXFA_WidgetData::GetLocale() {
  if (!m_pNode)
    return nullptr;

  WideString wsLocaleName;
  if (!m_pNode->GetLocaleName(wsLocaleName))
    return nullptr;
  if (wsLocaleName == L"ambient")
    return m_pNode->GetDocument()->GetLocalMgr()->GetDefLocale();
  return m_pNode->GetDocument()->GetLocalMgr()->GetLocaleByName(wsLocaleName);
}

WideString CXFA_WidgetData::GetValue(XFA_VALUEPICTURE eValueType) {
  WideString wsValue = m_pNode->JSNode()->GetContent(false);

  if (eValueType == XFA_VALUEPICTURE_Display)
    GetItemLabel(wsValue.AsStringView(), wsValue);

  WideString wsPicture = GetPictureContent(eValueType);
  CXFA_Node* pNode = GetUIChild();
  if (!pNode)
    return wsValue;

  switch (GetUIChild()->GetElementType()) {
    case XFA_Element::ChoiceList: {
      if (eValueType == XFA_VALUEPICTURE_Display) {
        int32_t iSelItemIndex = GetSelectedItem(0);
        if (iSelItemIndex >= 0) {
          wsValue = GetChoiceListItem(iSelItemIndex, false).value_or(L"");
          wsPicture.clear();
        }
      }
    } break;
    case XFA_Element::NumericEdit:
      if (eValueType != XFA_VALUEPICTURE_Raw && wsPicture.IsEmpty()) {
        IFX_Locale* pLocale = GetLocale();
        if (eValueType == XFA_VALUEPICTURE_Display && pLocale)
          wsValue = FormatNumStr(NormalizeNumStr(wsValue), pLocale);
      }
      break;
    default:
      break;
  }
  if (wsPicture.IsEmpty())
    return wsValue;

  if (IFX_Locale* pLocale = GetLocale()) {
    CXFA_LocaleValue widgetValue = XFA_GetLocaleValue(this);
    CXFA_LocaleMgr* pLocalMgr = m_pNode->GetDocument()->GetLocalMgr();
    switch (widgetValue.GetType()) {
      case XFA_VT_DATE: {
        WideString wsDate, wsTime;
        if (SplitDateTime(wsValue, wsDate, wsTime)) {
          CXFA_LocaleValue date(XFA_VT_DATE, wsDate, pLocalMgr);
          if (date.FormatPatterns(wsValue, wsPicture, pLocale, eValueType))
            return wsValue;
        }
        break;
      }
      case XFA_VT_TIME: {
        WideString wsDate, wsTime;
        if (SplitDateTime(wsValue, wsDate, wsTime)) {
          CXFA_LocaleValue time(XFA_VT_TIME, wsTime, pLocalMgr);
          if (time.FormatPatterns(wsValue, wsPicture, pLocale, eValueType))
            return wsValue;
        }
        break;
      }
      default:
        break;
    }
    widgetValue.FormatPatterns(wsValue, wsPicture, pLocale, eValueType);
  }
  return wsValue;
}

WideString CXFA_WidgetData::GetNormalizeDataValue(const WideString& wsValue) {
  if (wsValue.IsEmpty())
    return L"";

  WideString wsPicture = GetPictureContent(XFA_VALUEPICTURE_DataBind);
  if (wsPicture.IsEmpty())
    return wsValue;

  ASSERT(GetNode());
  CXFA_LocaleMgr* pLocalMgr = GetNode()->GetDocument()->GetLocalMgr();
  IFX_Locale* pLocale = GetLocale();
  CXFA_LocaleValue widgetValue = XFA_GetLocaleValue(this);
  if (widgetValue.ValidateValue(wsValue, wsPicture, pLocale, &wsPicture)) {
    widgetValue = CXFA_LocaleValue(widgetValue.GetType(), wsValue, wsPicture,
                                   pLocale, pLocalMgr);
    return widgetValue.GetValue();
  }
  return wsValue;
}

WideString CXFA_WidgetData::GetFormatDataValue(const WideString& wsValue) {
  if (wsValue.IsEmpty())
    return L"";

  WideString wsPicture = GetPictureContent(XFA_VALUEPICTURE_DataBind);
  if (wsPicture.IsEmpty())
    return wsValue;

  WideString wsFormattedValue = wsValue;
  if (IFX_Locale* pLocale = GetLocale()) {
    ASSERT(GetNode());
    CXFA_Node* pNodeValue = GetNode()->GetChild(0, XFA_Element::Value, false);
    if (!pNodeValue)
      return wsValue;

    CXFA_Node* pValueChild = pNodeValue->GetNodeItem(XFA_NODEITEM_FirstChild);
    if (!pValueChild)
      return wsValue;

    int32_t iVTType = XFA_VT_NULL;
    switch (pValueChild->GetElementType()) {
      case XFA_Element::Decimal:
        iVTType = XFA_VT_DECIMAL;
        break;
      case XFA_Element::Float:
        iVTType = XFA_VT_FLOAT;
        break;
      case XFA_Element::Date:
        iVTType = XFA_VT_DATE;
        break;
      case XFA_Element::Time:
        iVTType = XFA_VT_TIME;
        break;
      case XFA_Element::DateTime:
        iVTType = XFA_VT_DATETIME;
        break;
      case XFA_Element::Boolean:
        iVTType = XFA_VT_BOOLEAN;
        break;
      case XFA_Element::Integer:
        iVTType = XFA_VT_INTEGER;
        break;
      case XFA_Element::Text:
        iVTType = XFA_VT_TEXT;
        break;
      default:
        iVTType = XFA_VT_NULL;
        break;
    }
    CXFA_LocaleMgr* pLocalMgr = GetNode()->GetDocument()->GetLocalMgr();
    CXFA_LocaleValue widgetValue(iVTType, wsValue, pLocalMgr);
    switch (widgetValue.GetType()) {
      case XFA_VT_DATE: {
        WideString wsDate, wsTime;
        if (SplitDateTime(wsValue, wsDate, wsTime)) {
          CXFA_LocaleValue date(XFA_VT_DATE, wsDate, pLocalMgr);
          if (date.FormatPatterns(wsFormattedValue, wsPicture, pLocale,
                                  XFA_VALUEPICTURE_DataBind)) {
            return wsFormattedValue;
          }
        }
        break;
      }
      case XFA_VT_TIME: {
        WideString wsDate, wsTime;
        if (SplitDateTime(wsValue, wsDate, wsTime)) {
          CXFA_LocaleValue time(XFA_VT_TIME, wsTime, pLocalMgr);
          if (time.FormatPatterns(wsFormattedValue, wsPicture, pLocale,
                                  XFA_VALUEPICTURE_DataBind)) {
            return wsFormattedValue;
          }
        }
        break;
      }
      default:
        break;
    }
    widgetValue.FormatPatterns(wsFormattedValue, wsPicture, pLocale,
                               XFA_VALUEPICTURE_DataBind);
  }
  return wsFormattedValue;
}

WideString CXFA_WidgetData::NormalizeNumStr(const WideString& wsValue) {
  if (wsValue.IsEmpty())
    return L"";

  WideString wsOutput = wsValue;
  wsOutput.TrimLeft('0');

  if (!wsOutput.IsEmpty() && wsOutput.Contains('.') && GetFracDigits() != -1) {
    wsOutput.TrimRight(L"0");
    wsOutput.TrimRight(L".");
  }
  if (wsOutput.IsEmpty() || wsOutput[0] == '.')
    wsOutput.InsertAtFront('0');

  return wsOutput;
}

WideString CXFA_WidgetData::FormatNumStr(const WideString& wsValue,
                                         IFX_Locale* pLocale) {
  if (wsValue.IsEmpty())
    return L"";

  WideString wsSrcNum = wsValue;
  WideString wsGroupSymbol =
      pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_Grouping);
  bool bNeg = false;
  if (wsSrcNum[0] == '-') {
    bNeg = true;
    wsSrcNum.Delete(0, 1);
  }

  auto dot_index = wsSrcNum.Find('.');
  dot_index = !dot_index.has_value() ? wsSrcNum.GetLength() : dot_index;

  if (dot_index.value() < 1)
    return L"";

  size_t nPos = dot_index.value() % 3;
  WideString wsOutput;
  for (size_t i = 0; i < dot_index.value(); i++) {
    if (i % 3 == nPos && i != 0)
      wsOutput += wsGroupSymbol;

    wsOutput += wsSrcNum[i];
  }
  if (dot_index.value() < wsSrcNum.GetLength()) {
    wsOutput += pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_Decimal);
    wsOutput += wsSrcNum.Right(wsSrcNum.GetLength() - dot_index.value() - 1);
  }
  if (bNeg)
    return pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_Minus) + wsOutput;

  return wsOutput;
}

void CXFA_WidgetData::SyncValue(const WideString& wsValue, bool bNotify) {
  if (!m_pNode)
    return;

  WideString wsFormatValue(wsValue);
  CXFA_WidgetData* pContainerWidgetData = m_pNode->GetContainerWidgetData();
  if (pContainerWidgetData)
    wsFormatValue = pContainerWidgetData->GetFormatDataValue(wsValue);

  m_pNode->JSNode()->SetContent(wsValue, wsFormatValue, bNotify, false, true);
}

void CXFA_WidgetData::InsertListTextItem(CXFA_Node* pItems,
                                         const WideString& wsText,
                                         int32_t nIndex) {
  CXFA_Node* pText = pItems->CreateSamePacketNode(XFA_Element::Text);
  pItems->InsertChild(nIndex, pText);
  pText->JSNode()->SetContent(wsText, wsText, false, false, false);
}

WideString CXFA_WidgetData::NumericLimit(const WideString& wsValue,
                                         int32_t iLead,
                                         int32_t iTread) const {
  if ((iLead == -1) && (iTread == -1))
    return wsValue;

  WideString wsRet;
  int32_t iLead_ = 0, iTread_ = -1;
  int32_t iCount = wsValue.GetLength();
  if (iCount == 0)
    return wsValue;

  int32_t i = 0;
  if (wsValue[i] == L'-') {
    wsRet += L'-';
    i++;
  }
  for (; i < iCount; i++) {
    wchar_t wc = wsValue[i];
    if (FXSYS_isDecimalDigit(wc)) {
      if (iLead >= 0) {
        iLead_++;
        if (iLead_ > iLead)
          return L"0";
      } else if (iTread_ >= 0) {
        iTread_++;
        if (iTread_ > iTread) {
          if (iTread != -1) {
            CFX_Decimal wsDeci = CFX_Decimal(wsValue.AsStringView());
            wsDeci.SetScale(iTread);
            wsRet = wsDeci;
          }
          return wsRet;
        }
      }
    } else if (wc == L'.') {
      iTread_ = 0;
      iLead = -1;
    }
    wsRet += wc;
  }
  return wsRet;
}
