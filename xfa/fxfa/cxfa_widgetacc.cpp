// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/cxfa_widgetacc.h"

#include <algorithm>
#include <vector>

#include "core/fxcrt/cfx_decimal.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/xml/cfx_xmlelement.h"
#include "core/fxcrt/xml/cfx_xmlnode.h"
#include "fxjs/cfxjse_engine.h"
#include "third_party/base/stl_util.h"
#include "xfa/fde/cfde_textout.h"
#include "xfa/fxfa/cxfa_ffapp.h"
#include "xfa/fxfa/cxfa_ffdoc.h"
#include "xfa/fxfa/cxfa_ffdocview.h"
#include "xfa/fxfa/cxfa_ffnotify.h"
#include "xfa/fxfa/cxfa_ffwidget.h"
#include "xfa/fxfa/cxfa_fontmgr.h"
#include "xfa/fxfa/cxfa_textlayout.h"
#include "xfa/fxfa/cxfa_textprovider.h"
#include "xfa/fxfa/parser/cxfa_bind.h"
#include "xfa/fxfa/parser/cxfa_border.h"
#include "xfa/fxfa/parser/cxfa_calculate.h"
#include "xfa/fxfa/parser/cxfa_caption.h"
#include "xfa/fxfa/parser/cxfa_comb.h"
#include "xfa/fxfa/parser/cxfa_decimal.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_event.h"
#include "xfa/fxfa/parser/cxfa_font.h"
#include "xfa/fxfa/parser/cxfa_format.h"
#include "xfa/fxfa/parser/cxfa_image.h"
#include "xfa/fxfa/parser/cxfa_items.h"
#include "xfa/fxfa/parser/cxfa_layoutprocessor.h"
#include "xfa/fxfa/parser/cxfa_localevalue.h"
#include "xfa/fxfa/parser/cxfa_margin.h"
#include "xfa/fxfa/parser/cxfa_measurement.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/cxfa_para.h"
#include "xfa/fxfa/parser/cxfa_picture.h"
#include "xfa/fxfa/parser/cxfa_script.h"
#include "xfa/fxfa/parser/cxfa_stroke.h"
#include "xfa/fxfa/parser/cxfa_ui.h"
#include "xfa/fxfa/parser/cxfa_validate.h"
#include "xfa/fxfa/parser/cxfa_value.h"
#include "xfa/fxfa/parser/xfa_utils.h"

class CXFA_WidgetLayoutData {
 public:
  CXFA_WidgetLayoutData() : m_fWidgetHeight(-1) {}
  virtual ~CXFA_WidgetLayoutData() {}

  float m_fWidgetHeight;
};

namespace {

class CXFA_TextLayoutData : public CXFA_WidgetLayoutData {
 public:
  CXFA_TextLayoutData() {}
  ~CXFA_TextLayoutData() override {}

  CXFA_TextLayout* GetTextLayout() const { return m_pTextLayout.get(); }
  CXFA_TextProvider* GetTextProvider() const { return m_pTextProvider.get(); }

  void LoadText(CXFA_WidgetAcc* pAcc) {
    if (m_pTextLayout)
      return;

    m_pTextProvider =
        pdfium::MakeUnique<CXFA_TextProvider>(pAcc, XFA_TEXTPROVIDERTYPE_Text);
    m_pTextLayout = pdfium::MakeUnique<CXFA_TextLayout>(m_pTextProvider.get());
  }

 private:
  std::unique_ptr<CXFA_TextLayout> m_pTextLayout;
  std::unique_ptr<CXFA_TextProvider> m_pTextProvider;
};

class CXFA_ImageLayoutData : public CXFA_WidgetLayoutData {
 public:
  CXFA_ImageLayoutData()
      : m_bNamedImage(false), m_iImageXDpi(0), m_iImageYDpi(0) {}

  ~CXFA_ImageLayoutData() override {}

  bool LoadImageData(CXFA_WidgetAcc* pAcc) {
    if (m_pDIBitmap)
      return true;

    CXFA_Value* value = pAcc->GetFormValue();
    if (!value)
      return false;

    CXFA_Image* image = value->GetImage();
    if (!image)
      return false;

    CXFA_FFDoc* pFFDoc = pAcc->GetDoc();
    pAcc->SetImageImage(XFA_LoadImageData(pFFDoc, image, m_bNamedImage,
                                          m_iImageXDpi, m_iImageYDpi));
    return !!m_pDIBitmap;
  }

  RetainPtr<CFX_DIBitmap> m_pDIBitmap;
  bool m_bNamedImage;
  int32_t m_iImageXDpi;
  int32_t m_iImageYDpi;
};

class CXFA_FieldLayoutData : public CXFA_WidgetLayoutData {
 public:
  CXFA_FieldLayoutData() {}
  ~CXFA_FieldLayoutData() override {}

  bool LoadCaption(CXFA_WidgetAcc* pAcc) {
    if (m_pCapTextLayout)
      return true;
    CXFA_Caption* caption = pAcc->GetCaption();
    if (!caption || caption->IsHidden())
      return false;

    m_pCapTextProvider = pdfium::MakeUnique<CXFA_TextProvider>(
        pAcc, XFA_TEXTPROVIDERTYPE_Caption);
    m_pCapTextLayout =
        pdfium::MakeUnique<CXFA_TextLayout>(m_pCapTextProvider.get());
    return true;
  }

  std::unique_ptr<CXFA_TextLayout> m_pCapTextLayout;
  std::unique_ptr<CXFA_TextProvider> m_pCapTextProvider;
  std::unique_ptr<CFDE_TextOut> m_pTextOut;
  std::vector<float> m_FieldSplitArray;
};

class CXFA_TextEditData : public CXFA_FieldLayoutData {
 public:
};

class CXFA_ImageEditData : public CXFA_FieldLayoutData {
 public:
  CXFA_ImageEditData()
      : m_bNamedImage(false), m_iImageXDpi(0), m_iImageYDpi(0) {}

  ~CXFA_ImageEditData() override {}

  bool LoadImageData(CXFA_WidgetAcc* pAcc) {
    if (m_pDIBitmap)
      return true;

    CXFA_Value* value = pAcc->GetFormValue();
    if (!value)
      return false;

    CXFA_Image* image = value->GetImage();
    CXFA_FFDoc* pFFDoc = pAcc->GetDoc();
    pAcc->SetImageEditImage(XFA_LoadImageData(pFFDoc, image, m_bNamedImage,
                                              m_iImageXDpi, m_iImageYDpi));
    return !!m_pDIBitmap;
  }

  RetainPtr<CFX_DIBitmap> m_pDIBitmap;
  bool m_bNamedImage;
  int32_t m_iImageXDpi;
  int32_t m_iImageYDpi;
};

float GetEdgeThickness(const std::vector<CXFA_Stroke*>& strokes,
                       bool b3DStyle,
                       int32_t nIndex) {
  float fThickness = 0;

  CXFA_Stroke* stroke = strokes[nIndex * 2 + 1];
  if (stroke->IsVisible()) {
    if (nIndex == 0)
      fThickness += 2.5f;

    fThickness += stroke->GetThickness() * (b3DStyle ? 4 : 2);
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
  auto* defValue =
      pNode->JSObject()->GetProperty<CXFA_Value>(0, XFA_Element::Value, true);
  XFA_Element eValueType =
      defValue ? defValue->GetChildValueClassID() : XFA_Element::Unknown;
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
  CXFA_Ui* pUI =
      pNode->JSObject()->GetProperty<CXFA_Ui>(0, XFA_Element::Ui, true);
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
      defValue->JSObject()->GetProperty<CXFA_Text>(0, XFA_Element::Text, true);
    }
    return pUI->JSObject()->GetProperty<CXFA_Node>(0, eUIType, true);
  }

  if (eUIType != XFA_Element::Unknown)
    return pUIChild;

  switch (pUIChild->GetElementType()) {
    case XFA_Element::CheckButton: {
      eValueType = XFA_Element::Text;
      if (CXFA_Items* pItems =
              pNode->GetChild<CXFA_Items>(0, XFA_Element::Items, false)) {
        if (CXFA_Node* pItem =
                pItems->GetChild<CXFA_Node>(0, XFA_Element::Unknown, false)) {
          eValueType = pItem->GetElementType();
        }
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
      eValueType = (pUIChild->JSObject()->GetEnum(XFA_Attribute::Open) ==
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
  defValue->JSObject()->GetProperty<CXFA_Node>(0, eValueType, true);

  return pUIChild;
}

}  // namespace

CXFA_WidgetAcc::CXFA_WidgetAcc(CXFA_FFDocView* pDocView, CXFA_Node* pNode)
    : m_pDocView(pDocView),
      m_nRecursionDepth(0),
      m_bIsNull(true),
      m_bPreNull(true),
      m_pUiChildNode(nullptr),
      m_eUIType(XFA_Element::Unknown),
      m_pNode(pNode) {}

CXFA_WidgetAcc::~CXFA_WidgetAcc() = default;

XFA_Element CXFA_WidgetAcc::GetElementType() const {
  return m_pNode ? m_pNode->GetElementType() : XFA_Element::Unknown;
}

CXFA_Node* CXFA_WidgetAcc::GetDatasets() {
  return m_pNode->GetBindData();
}

void CXFA_WidgetAcc::ResetData() {
  WideString wsValue;
  XFA_Element eUIType = GetUIType();
  switch (eUIType) {
    case XFA_Element::ImageEdit: {
      CXFA_Value* imageValue = GetDefaultValue();
      CXFA_Image* image = imageValue ? imageValue->GetImage() : nullptr;
      WideString wsContentType, wsHref;
      if (image) {
        wsValue = image->GetContent();
        wsContentType = image->GetContentType();
        wsHref = image->GetHref();
      }
      SetImageEdit(wsContentType, wsHref, wsValue);
      break;
    }
    case XFA_Element::ExclGroup: {
      CXFA_Node* pNextChild = m_pNode->GetNodeItem(
          XFA_NODEITEM_FirstChild, XFA_ObjectType::ContainerNode);
      while (pNextChild) {
        CXFA_Node* pChild = pNextChild;
        CXFA_WidgetAcc* pAcc = pChild->GetWidgetAcc();
        if (!pAcc)
          continue;

        bool done = false;
        if (wsValue.IsEmpty()) {
          CXFA_Value* defValue = pAcc->GetDefaultValue();
          if (defValue) {
            wsValue = defValue->GetChildValueContent();
            SetValue(XFA_VALUEPICTURE_Raw, wsValue);
            pAcc->SetValue(XFA_VALUEPICTURE_Raw, wsValue);
            done = true;
          }
        }
        if (!done) {
          CXFA_Items* pItems =
              pChild->GetChild<CXFA_Items>(0, XFA_Element::Items, false);
          if (!pItems)
            continue;

          WideString itemText;
          if (pItems->CountChildren(XFA_Element::Unknown, false) > 1) {
            itemText =
                pItems->GetChild<CXFA_Node>(1, XFA_Element::Unknown, false)
                    ->JSObject()
                    ->GetContent(false);
          }
          pAcc->SetValue(XFA_VALUEPICTURE_Raw, itemText);
        }
        pNextChild = pChild->GetNodeItem(XFA_NODEITEM_NextSibling,
                                         XFA_ObjectType::ContainerNode);
      }
      break;
    }
    case XFA_Element::ChoiceList:
      ClearAllSelections();
    default: {
      CXFA_Value* defValue = GetDefaultValue();
      if (defValue)
        wsValue = defValue->GetChildValueContent();

      SetValue(XFA_VALUEPICTURE_Raw, wsValue);
      break;
    }
  }
}

void CXFA_WidgetAcc::SetImageEdit(const WideString& wsContentType,
                                  const WideString& wsHref,
                                  const WideString& wsData) {
  CXFA_Image* image = GetFormValue() ? GetFormValue()->GetImage() : nullptr;
  if (image) {
    image->SetContentType(WideString(wsContentType));
    image->SetHref(wsHref);
  }

  m_pNode->JSObject()->SetContent(wsData, GetFormatDataValue(wsData), true,
                                  false, true);

  CXFA_Node* pBind = GetDatasets();
  if (!pBind) {
    image->SetTransferEncoding(XFA_AttributeEnum::Base64);
    return;
  }
  pBind->JSObject()->SetCData(XFA_Attribute::ContentType, wsContentType, false,
                              false);
  CXFA_Node* pHrefNode = pBind->GetNodeItem(XFA_NODEITEM_FirstChild);
  if (pHrefNode) {
    pHrefNode->JSObject()->SetCData(XFA_Attribute::Value, wsHref, false, false);
  } else {
    CFX_XMLNode* pXMLNode = pBind->GetXMLMappingNode();
    ASSERT(pXMLNode && pXMLNode->GetType() == FX_XMLNODE_Element);
    static_cast<CFX_XMLElement*>(pXMLNode)->SetString(L"href", wsHref);
  }
}

CXFA_WidgetAcc* CXFA_WidgetAcc::GetExclGroup() {
  CXFA_Node* pExcl = m_pNode->GetNodeItem(XFA_NODEITEM_Parent);
  if (!pExcl || pExcl->GetElementType() != XFA_Element::ExclGroup)
    return nullptr;
  return pExcl->GetWidgetAcc();
}

CXFA_FFDoc* CXFA_WidgetAcc::GetDoc() {
  return m_pDocView->GetDoc();
}

IXFA_AppProvider* CXFA_WidgetAcc::GetAppProvider() {
  return GetDoc()->GetApp()->GetAppProvider();
}

int32_t CXFA_WidgetAcc::ProcessEvent(XFA_AttributeEnum iActivity,
                                     CXFA_EventParam* pEventParam) {
  if (GetElementType() == XFA_Element::Draw)
    return XFA_EVENTERROR_NotExist;

  std::vector<CXFA_Event*> eventArray =
      GetEventByActivity(iActivity, pEventParam->m_bIsFormReady);
  bool first = true;
  int32_t iRet = XFA_EVENTERROR_NotExist;
  for (CXFA_Event* event : eventArray) {
    int32_t result = ProcessEvent(event, pEventParam);
    if (first || result == XFA_EVENTERROR_Success)
      iRet = result;
    first = false;
  }
  return iRet;
}

int32_t CXFA_WidgetAcc::ProcessEvent(CXFA_Event* event,
                                     CXFA_EventParam* pEventParam) {
  if (!event)
    return XFA_EVENTERROR_NotExist;

  switch (event->GetEventType()) {
    case XFA_Element::Execute:
      break;
    case XFA_Element::Script:
      return ExecuteScript(event->GetScript(), pEventParam);
    case XFA_Element::SignData:
      break;
    case XFA_Element::Submit:
      return GetDoc()->GetDocEnvironment()->Submit(GetDoc(),
                                                   event->GetSubmit());
    default:
      break;
  }
  return XFA_EVENTERROR_NotExist;
}

int32_t CXFA_WidgetAcc::ProcessCalculate() {
  if (GetElementType() == XFA_Element::Draw)
    return XFA_EVENTERROR_NotExist;

  CXFA_Calculate* calc = GetCalculate();
  if (!calc)
    return XFA_EVENTERROR_NotExist;
  if (GetNode()->IsUserInteractive())
    return XFA_EVENTERROR_Disabled;

  CXFA_EventParam EventParam;
  EventParam.m_eType = XFA_EVENT_Calculate;
  int32_t iRet = ExecuteScript(calc->GetScript(), &EventParam);
  if (iRet != XFA_EVENTERROR_Success)
    return iRet;

  if (GetRawValue() != EventParam.m_wsResult) {
    SetValue(XFA_VALUEPICTURE_Raw, EventParam.m_wsResult);
    UpdateUIDisplay(m_pDocView, nullptr);
  }
  return XFA_EVENTERROR_Success;
}

void CXFA_WidgetAcc::ProcessScriptTestValidate(CXFA_Validate* validate,
                                               int32_t iRet,
                                               bool bRetValue,
                                               bool bVersionFlag) {
  if (iRet != XFA_EVENTERROR_Success)
    return;
  if (bRetValue)
    return;

  IXFA_AppProvider* pAppProvider = GetAppProvider();
  if (!pAppProvider)
    return;

  WideString wsTitle = pAppProvider->GetAppTitle();
  WideString wsScriptMsg = validate->GetScriptMessageText();
  if (validate->GetScriptTest() == XFA_AttributeEnum::Warning) {
    if (GetNode()->IsUserInteractive())
      return;
    if (wsScriptMsg.IsEmpty())
      wsScriptMsg = GetValidateMessage(false, bVersionFlag);

    if (bVersionFlag) {
      pAppProvider->MsgBox(wsScriptMsg, wsTitle, XFA_MBICON_Warning, XFA_MB_OK);
      return;
    }
    if (pAppProvider->MsgBox(wsScriptMsg, wsTitle, XFA_MBICON_Warning,
                             XFA_MB_YesNo) == XFA_IDYes) {
      GetNode()->SetFlag(XFA_NodeFlag_UserInteractive, false);
    }
    return;
  }

  if (wsScriptMsg.IsEmpty())
    wsScriptMsg = GetValidateMessage(true, bVersionFlag);
  pAppProvider->MsgBox(wsScriptMsg, wsTitle, XFA_MBICON_Error, XFA_MB_OK);
}

int32_t CXFA_WidgetAcc::ProcessFormatTestValidate(CXFA_Validate* validate,
                                                  bool bVersionFlag) {
  WideString wsRawValue = GetRawValue();
  if (!wsRawValue.IsEmpty()) {
    WideString wsPicture = validate->GetPicture();
    if (wsPicture.IsEmpty())
      return XFA_EVENTERROR_NotExist;

    IFX_Locale* pLocale = GetLocale();
    if (!pLocale)
      return XFA_EVENTERROR_NotExist;

    CXFA_LocaleValue lcValue = XFA_GetLocaleValue(this);
    if (!lcValue.ValidateValue(lcValue.GetValue(), wsPicture, pLocale,
                               nullptr)) {
      IXFA_AppProvider* pAppProvider = GetAppProvider();
      if (!pAppProvider)
        return XFA_EVENTERROR_NotExist;

      WideString wsFormatMsg = validate->GetFormatMessageText();
      WideString wsTitle = pAppProvider->GetAppTitle();
      if (validate->GetFormatTest() == XFA_AttributeEnum::Error) {
        if (wsFormatMsg.IsEmpty())
          wsFormatMsg = GetValidateMessage(true, bVersionFlag);
        pAppProvider->MsgBox(wsFormatMsg, wsTitle, XFA_MBICON_Error, XFA_MB_OK);
        return XFA_EVENTERROR_Success;
      }
      if (GetNode()->IsUserInteractive())
        return XFA_EVENTERROR_NotExist;
      if (wsFormatMsg.IsEmpty())
        wsFormatMsg = GetValidateMessage(false, bVersionFlag);

      if (bVersionFlag) {
        pAppProvider->MsgBox(wsFormatMsg, wsTitle, XFA_MBICON_Warning,
                             XFA_MB_OK);
        return XFA_EVENTERROR_Success;
      }
      if (pAppProvider->MsgBox(wsFormatMsg, wsTitle, XFA_MBICON_Warning,
                               XFA_MB_YesNo) == XFA_IDYes) {
        GetNode()->SetFlag(XFA_NodeFlag_UserInteractive, false);
      }
      return XFA_EVENTERROR_Success;
    }
  }
  return XFA_EVENTERROR_NotExist;
}

int32_t CXFA_WidgetAcc::ProcessNullTestValidate(CXFA_Validate* validate,
                                                int32_t iFlags,
                                                bool bVersionFlag) {
  if (!GetValue(XFA_VALUEPICTURE_Raw).IsEmpty())
    return XFA_EVENTERROR_Success;
  if (IsNull() && IsPreNull())
    return XFA_EVENTERROR_Success;

  XFA_AttributeEnum eNullTest = validate->GetNullTest();
  WideString wsNullMsg = validate->GetNullMessageText();
  if (iFlags & 0x01) {
    int32_t iRet = XFA_EVENTERROR_Success;
    if (eNullTest != XFA_AttributeEnum::Disabled)
      iRet = XFA_EVENTERROR_Error;

    if (!wsNullMsg.IsEmpty()) {
      if (eNullTest != XFA_AttributeEnum::Disabled) {
        m_pDocView->m_arrNullTestMsg.push_back(wsNullMsg);
        return XFA_EVENTERROR_Error;
      }
      return XFA_EVENTERROR_Success;
    }
    return iRet;
  }
  if (wsNullMsg.IsEmpty() && bVersionFlag &&
      eNullTest != XFA_AttributeEnum::Disabled) {
    return XFA_EVENTERROR_Error;
  }
  IXFA_AppProvider* pAppProvider = GetAppProvider();
  if (!pAppProvider)
    return XFA_EVENTERROR_NotExist;

  WideString wsCaptionName;
  WideString wsTitle = pAppProvider->GetAppTitle();
  switch (eNullTest) {
    case XFA_AttributeEnum::Error: {
      if (wsNullMsg.IsEmpty()) {
        wsCaptionName = GetValidateCaptionName(bVersionFlag);
        wsNullMsg =
            WideString::Format(L"%ls cannot be blank.", wsCaptionName.c_str());
      }
      pAppProvider->MsgBox(wsNullMsg, wsTitle, XFA_MBICON_Status, XFA_MB_OK);
      return XFA_EVENTERROR_Error;
    }
    case XFA_AttributeEnum::Warning: {
      if (GetNode()->IsUserInteractive())
        return true;

      if (wsNullMsg.IsEmpty()) {
        wsCaptionName = GetValidateCaptionName(bVersionFlag);
        wsNullMsg = WideString::Format(
            L"%ls cannot be blank. To ignore validations for %ls, click "
            L"Ignore.",
            wsCaptionName.c_str(), wsCaptionName.c_str());
      }
      if (pAppProvider->MsgBox(wsNullMsg, wsTitle, XFA_MBICON_Warning,
                               XFA_MB_YesNo) == XFA_IDYes) {
        GetNode()->SetFlag(XFA_NodeFlag_UserInteractive, false);
      }
      return XFA_EVENTERROR_Error;
    }
    case XFA_AttributeEnum::Disabled:
    default:
      break;
  }
  return XFA_EVENTERROR_Success;
}

WideString CXFA_WidgetAcc::GetValidateCaptionName(bool bVersionFlag) {
  WideString wsCaptionName;

  if (!bVersionFlag) {
    CXFA_Caption* caption = GetCaption();
    if (caption) {
      CXFA_Value* capValue = caption->GetValue();
      if (capValue) {
        CXFA_Text* captionText = capValue->GetText();
        if (captionText)
          wsCaptionName = captionText->GetContent();
      }
    }
  }
  if (!wsCaptionName.IsEmpty())
    return wsCaptionName;
  return m_pNode->JSObject()->GetCData(XFA_Attribute::Name);
}

WideString CXFA_WidgetAcc::GetValidateMessage(bool bError, bool bVersionFlag) {
  WideString wsCaptionName = GetValidateCaptionName(bVersionFlag);
  if (bVersionFlag)
    return WideString::Format(L"%ls validation failed", wsCaptionName.c_str());
  if (bError) {
    return WideString::Format(L"The value you entered for %ls is invalid.",
                              wsCaptionName.c_str());
  }
  return WideString::Format(
      L"The value you entered for %ls is invalid. To ignore "
      L"validations for %ls, click Ignore.",
      wsCaptionName.c_str(), wsCaptionName.c_str());
}

int32_t CXFA_WidgetAcc::ProcessValidate(int32_t iFlags) {
  if (GetElementType() == XFA_Element::Draw)
    return XFA_EVENTERROR_NotExist;

  CXFA_Validate* validate = GetValidate(false);
  if (!validate)
    return XFA_EVENTERROR_NotExist;

  bool bInitDoc = validate->NeedsInitApp();
  bool bStatus = m_pDocView->GetLayoutStatus() < XFA_DOCVIEW_LAYOUTSTATUS_End;
  int32_t iFormat = 0;
  int32_t iRet = XFA_EVENTERROR_NotExist;
  CXFA_Script* script = validate->GetScript();
  bool bRet = false;
  bool hasBoolResult = (bInitDoc || bStatus) && GetRawValue().IsEmpty();
  if (script) {
    CXFA_EventParam eParam;
    eParam.m_eType = XFA_EVENT_Validate;
    eParam.m_pTarget = this;
    std::tie(iRet, bRet) = ExecuteBoolScript(script, &eParam);
  }

  XFA_VERSION version = GetDoc()->GetXFADoc()->GetCurVersionMode();
  bool bVersionFlag = false;
  if (version < XFA_VERSION_208)
    bVersionFlag = true;

  if (bInitDoc) {
    validate->ClearFlag(XFA_NodeFlag_NeedsInitApp);
  } else {
    iFormat = ProcessFormatTestValidate(validate, bVersionFlag);
    if (!bVersionFlag)
      bVersionFlag = GetDoc()->GetXFADoc()->HasFlag(XFA_DOCFLAG_Scripting);

    iRet |= ProcessNullTestValidate(validate, iFlags, bVersionFlag);
  }

  if (iFormat != XFA_EVENTERROR_Success && hasBoolResult)
    ProcessScriptTestValidate(validate, iRet, bRet, bVersionFlag);

  return iRet | iFormat;
}

int32_t CXFA_WidgetAcc::ExecuteScript(CXFA_Script* script,
                                      CXFA_EventParam* pEventParam) {
  bool bRet;
  int32_t iRet;
  std::tie(iRet, bRet) = ExecuteBoolScript(script, pEventParam);
  return iRet;
}

std::pair<int32_t, bool> CXFA_WidgetAcc::ExecuteBoolScript(
    CXFA_Script* script,
    CXFA_EventParam* pEventParam) {
  static const uint32_t MAX_RECURSION_DEPTH = 2;
  if (m_nRecursionDepth > MAX_RECURSION_DEPTH)
    return {XFA_EVENTERROR_Success, false};

  ASSERT(pEventParam);
  if (!script)
    return {XFA_EVENTERROR_NotExist, false};
  if (script->GetRunAt() == XFA_AttributeEnum::Server)
    return {XFA_EVENTERROR_Disabled, false};

  WideString wsExpression = script->GetExpression();
  if (wsExpression.IsEmpty())
    return {XFA_EVENTERROR_NotExist, false};

  CXFA_Script::Type eScriptType = script->GetContentType();
  if (eScriptType == CXFA_Script::Type::Unknown)
    return {XFA_EVENTERROR_Success, false};

  CXFA_FFDoc* pDoc = GetDoc();
  CFXJSE_Engine* pContext = pDoc->GetXFADoc()->GetScriptContext();
  pContext->SetEventParam(*pEventParam);
  pContext->SetRunAtType(script->GetRunAt());

  std::vector<CXFA_Node*> refNodes;
  if (pEventParam->m_eType == XFA_EVENT_InitCalculate ||
      pEventParam->m_eType == XFA_EVENT_Calculate) {
    pContext->SetNodesOfRunScript(&refNodes);
  }

  auto pTmpRetValue = pdfium::MakeUnique<CFXJSE_Value>(pContext->GetIsolate());
  ++m_nRecursionDepth;
  bool bRet = pContext->RunScript(eScriptType, wsExpression.AsStringView(),
                                  pTmpRetValue.get(), m_pNode);
  --m_nRecursionDepth;
  int32_t iRet = XFA_EVENTERROR_Error;
  if (bRet) {
    iRet = XFA_EVENTERROR_Success;
    if (pEventParam->m_eType == XFA_EVENT_Calculate ||
        pEventParam->m_eType == XFA_EVENT_InitCalculate) {
      if (!pTmpRetValue->IsUndefined()) {
        if (!pTmpRetValue->IsNull())
          pEventParam->m_wsResult = pTmpRetValue->ToWideString();

        iRet = XFA_EVENTERROR_Success;
      } else {
        iRet = XFA_EVENTERROR_Error;
      }
      if (pEventParam->m_eType == XFA_EVENT_InitCalculate) {
        if ((iRet == XFA_EVENTERROR_Success) &&
            (GetRawValue() != pEventParam->m_wsResult)) {
          SetValue(XFA_VALUEPICTURE_Raw, pEventParam->m_wsResult);
          m_pDocView->AddValidateWidget(this);
        }
      }
      for (CXFA_Node* pRefNode : refNodes) {
        if (pRefNode->GetWidgetAcc() == this)
          continue;

        CXFA_CalcData* pGlobalData = pRefNode->JSObject()->GetCalcData();
        if (!pGlobalData) {
          pRefNode->JSObject()->SetCalcData(
              pdfium::MakeUnique<CXFA_CalcData>());
          pGlobalData = pRefNode->JSObject()->GetCalcData();
        }
        if (!pdfium::ContainsValue(pGlobalData->m_Globals, this))
          pGlobalData->m_Globals.push_back(this);
      }
    }
  }
  pContext->SetNodesOfRunScript(nullptr);

  return {iRet, pTmpRetValue->IsBoolean() ? pTmpRetValue->ToBoolean() : false};
}

CXFA_FFWidget* CXFA_WidgetAcc::GetNextWidget(CXFA_FFWidget* pWidget) {
  return static_cast<CXFA_FFWidget*>(pWidget->GetNext());
}

void CXFA_WidgetAcc::UpdateUIDisplay(CXFA_FFDocView* docView,
                                     CXFA_FFWidget* pExcept) {
  CXFA_FFWidget* pWidget = docView->GetWidgetForNode(m_pNode);
  for (; pWidget; pWidget = GetNextWidget(pWidget)) {
    if (pWidget == pExcept || !pWidget->IsLoaded() ||
        (GetUIType() != XFA_Element::CheckButton && pWidget->IsFocused())) {
      continue;
    }
    pWidget->UpdateFWLData();
    pWidget->AddInvalidateRect();
  }
}

void CXFA_WidgetAcc::CalcCaptionSize(CFX_SizeF& szCap) {
  CXFA_Caption* caption = GetCaption();
  if (!caption || !caption->IsVisible())
    return;

  LoadCaption();
  XFA_Element eUIType = GetUIType();
  XFA_AttributeEnum iCapPlacement = caption->GetPlacementType();
  float fCapReserve = caption->GetReserve();
  const bool bVert = iCapPlacement == XFA_AttributeEnum::Top ||
                     iCapPlacement == XFA_AttributeEnum::Bottom;
  const bool bReserveExit = fCapReserve > 0.01;
  CXFA_TextLayout* pCapTextLayout =
      static_cast<CXFA_FieldLayoutData*>(m_pLayoutData.get())
          ->m_pCapTextLayout.get();
  if (pCapTextLayout) {
    if (!bVert && eUIType != XFA_Element::Button)
      szCap.width = fCapReserve;

    CFX_SizeF minSize;
    szCap = pCapTextLayout->CalcSize(minSize, szCap);
    if (bReserveExit)
      bVert ? szCap.height = fCapReserve : szCap.width = fCapReserve;
  } else {
    float fFontSize = 10.0f;
    CXFA_Font* font = caption->GetFont();
    if (font) {
      fFontSize = font->GetFontSize();
    } else {
      CXFA_Font* widgetfont = GetFont(false);
      if (widgetfont)
        fFontSize = widgetfont->GetFontSize();
    }

    if (bVert) {
      szCap.height = fCapReserve > 0 ? fCapReserve : fFontSize;
    } else {
      szCap.width = fCapReserve > 0 ? fCapReserve : 0;
      szCap.height = fFontSize;
    }
  }

  CXFA_Margin* captionMargin = caption->GetMargin();
  if (captionMargin) {
    float fLeftInset = captionMargin->GetLeftInset();
    float fTopInset = captionMargin->GetTopInset();
    float fRightInset = captionMargin->GetRightInset();
    float fBottomInset = captionMargin->GetBottomInset();
    if (bReserveExit) {
      bVert ? (szCap.width += fLeftInset + fRightInset)
            : (szCap.height += fTopInset + fBottomInset);
    } else {
      szCap.width += fLeftInset + fRightInset;
      szCap.height += fTopInset + fBottomInset;
    }
  }
}

bool CXFA_WidgetAcc::CalculateFieldAutoSize(CFX_SizeF& size) {
  CFX_SizeF szCap;
  CalcCaptionSize(szCap);
  CFX_RectF rtUIMargin = GetUIMargin();
  size.width += rtUIMargin.left + rtUIMargin.width;
  size.height += rtUIMargin.top + rtUIMargin.height;
  if (szCap.width > 0 && szCap.height > 0) {
    switch (GetCaption()->GetPlacementType()) {
      case XFA_AttributeEnum::Left:
      case XFA_AttributeEnum::Right:
      case XFA_AttributeEnum::Inline: {
        size.width += szCap.width;
        size.height = std::max(size.height, szCap.height);
      } break;
      case XFA_AttributeEnum::Top:
      case XFA_AttributeEnum::Bottom: {
        size.height += szCap.height;
        size.width = std::max(size.width, szCap.width);
      }
      default:
        break;
    }
  }
  return CalculateWidgetAutoSize(size);
}

bool CXFA_WidgetAcc::CalculateWidgetAutoSize(CFX_SizeF& size) {
  CXFA_Margin* margin = GetMargin();
  if (margin) {
    size.width += margin->GetLeftInset() + margin->GetRightInset();
    size.height += margin->GetTopInset() + margin->GetBottomInset();
  }

  CXFA_Para* para = GetPara();
  if (para)
    size.width += para->GetMarginLeft() + para->GetTextIndent();

  Optional<float> width = TryWidth();
  if (width) {
    size.width = *width;
  } else {
    Optional<float> min = TryMinWidth();
    if (min)
      size.width = std::max(size.width, *min);

    Optional<float> max = TryMaxWidth();
    if (max && *max > 0)
      size.width = std::min(size.width, *max);
  }

  Optional<float> height = TryHeight();
  if (height) {
    size.height = *height;
  } else {
    Optional<float> min = TryMinHeight();
    if (min)
      size.height = std::max(size.height, *min);

    Optional<float> max = TryMaxHeight();
    if (max && *max > 0)
      size.height = std::min(size.height, *max);
  }
  return true;
}

void CXFA_WidgetAcc::CalculateTextContentSize(CFX_SizeF& size) {
  float fFontSize = GetFontSize();
  WideString wsText = GetValue(XFA_VALUEPICTURE_Display);
  if (wsText.IsEmpty()) {
    size.height += fFontSize;
    return;
  }

  wchar_t wcEnter = '\n';
  wchar_t wsLast = wsText[wsText.GetLength() - 1];
  if (wsLast == wcEnter)
    wsText = wsText + wcEnter;

  CXFA_FieldLayoutData* layoutData =
      static_cast<CXFA_FieldLayoutData*>(m_pLayoutData.get());
  if (!layoutData->m_pTextOut) {
    layoutData->m_pTextOut = pdfium::MakeUnique<CFDE_TextOut>();
    CFDE_TextOut* pTextOut = layoutData->m_pTextOut.get();
    pTextOut->SetFont(GetFDEFont());
    pTextOut->SetFontSize(fFontSize);
    pTextOut->SetLineBreakTolerance(fFontSize * 0.2f);
    pTextOut->SetLineSpace(GetLineHeight());

    FDE_TextStyle dwStyles;
    dwStyles.last_line_height_ = true;
    if (GetUIType() == XFA_Element::TextEdit && IsMultiLine())
      dwStyles.line_wrap_ = true;

    pTextOut->SetStyles(dwStyles);
  }
  layoutData->m_pTextOut->CalcLogicSize(wsText, size);
}

bool CXFA_WidgetAcc::CalculateTextEditAutoSize(CFX_SizeF& size) {
  if (size.width > 0) {
    CFX_SizeF szOrz = size;
    CFX_SizeF szCap;
    CalcCaptionSize(szCap);
    bool bCapExit = szCap.width > 0.01 && szCap.height > 0.01;
    XFA_AttributeEnum iCapPlacement = XFA_AttributeEnum::Unknown;
    if (bCapExit) {
      iCapPlacement = GetCaption()->GetPlacementType();
      switch (iCapPlacement) {
        case XFA_AttributeEnum::Left:
        case XFA_AttributeEnum::Right:
        case XFA_AttributeEnum::Inline: {
          size.width -= szCap.width;
        }
        default:
          break;
      }
    }
    CFX_RectF rtUIMargin = GetUIMargin();
    size.width -= rtUIMargin.left + rtUIMargin.width;
    CXFA_Margin* margin = GetMargin();
    if (margin)
      size.width -= margin->GetLeftInset() + margin->GetRightInset();

    CalculateTextContentSize(size);
    size.height += rtUIMargin.top + rtUIMargin.height;
    if (bCapExit) {
      switch (iCapPlacement) {
        case XFA_AttributeEnum::Left:
        case XFA_AttributeEnum::Right:
        case XFA_AttributeEnum::Inline: {
          size.height = std::max(size.height, szCap.height);
        } break;
        case XFA_AttributeEnum::Top:
        case XFA_AttributeEnum::Bottom: {
          size.height += szCap.height;
        }
        default:
          break;
      }
    }
    size.width = szOrz.width;
    return CalculateWidgetAutoSize(size);
  }
  CalculateTextContentSize(size);
  return CalculateFieldAutoSize(size);
}

bool CXFA_WidgetAcc::CalculateCheckButtonAutoSize(CFX_SizeF& size) {
  float fCheckSize = GetCheckButtonSize();
  size = CFX_SizeF(fCheckSize, fCheckSize);
  return CalculateFieldAutoSize(size);
}

bool CXFA_WidgetAcc::CalculatePushButtonAutoSize(CFX_SizeF& size) {
  CalcCaptionSize(size);
  return CalculateWidgetAutoSize(size);
}

CFX_SizeF CXFA_WidgetAcc::CalculateImageSize(float img_width,
                                             float img_height,
                                             float dpi_x,
                                             float dpi_y) {
  CFX_RectF rtImage(0, 0, XFA_UnitPx2Pt(img_width, dpi_x),
                    XFA_UnitPx2Pt(img_height, dpi_y));

  CFX_RectF rtFit;
  Optional<float> width = TryWidth();
  if (width) {
    rtFit.width = *width;
    GetWidthWithoutMargin(rtFit.width);
  } else {
    rtFit.width = rtImage.width;
  }

  Optional<float> height = TryHeight();
  if (height) {
    rtFit.height = *height;
    GetHeightWithoutMargin(rtFit.height);
  } else {
    rtFit.height = rtImage.height;
  }

  return rtFit.Size();
}

bool CXFA_WidgetAcc::CalculateImageAutoSize(CFX_SizeF& size) {
  if (!GetImageImage())
    LoadImageImage();

  size.clear();
  RetainPtr<CFX_DIBitmap> pBitmap = GetImageImage();
  if (!pBitmap)
    return CalculateWidgetAutoSize(size);

  int32_t iImageXDpi = 0;
  int32_t iImageYDpi = 0;
  GetImageDpi(iImageXDpi, iImageYDpi);

  size = CalculateImageSize(pBitmap->GetWidth(), pBitmap->GetHeight(),
                            iImageXDpi, iImageYDpi);
  return CalculateWidgetAutoSize(size);
}

bool CXFA_WidgetAcc::CalculateImageEditAutoSize(CFX_SizeF& size) {
  if (!GetImageEditImage())
    LoadImageEditImage();

  size.clear();
  RetainPtr<CFX_DIBitmap> pBitmap = GetImageEditImage();
  if (!pBitmap)
    return CalculateFieldAutoSize(size);

  int32_t iImageXDpi = 0;
  int32_t iImageYDpi = 0;
  GetImageEditDpi(iImageXDpi, iImageYDpi);

  size = CalculateImageSize(pBitmap->GetWidth(), pBitmap->GetHeight(),
                            iImageXDpi, iImageYDpi);
  return CalculateFieldAutoSize(size);
}

bool CXFA_WidgetAcc::LoadImageImage() {
  InitLayoutData();
  return static_cast<CXFA_ImageLayoutData*>(m_pLayoutData.get())
      ->LoadImageData(this);
}

bool CXFA_WidgetAcc::LoadImageEditImage() {
  InitLayoutData();
  return static_cast<CXFA_ImageEditData*>(m_pLayoutData.get())
      ->LoadImageData(this);
}

void CXFA_WidgetAcc::GetImageDpi(int32_t& iImageXDpi, int32_t& iImageYDpi) {
  CXFA_ImageLayoutData* pData =
      static_cast<CXFA_ImageLayoutData*>(m_pLayoutData.get());
  iImageXDpi = pData->m_iImageXDpi;
  iImageYDpi = pData->m_iImageYDpi;
}

void CXFA_WidgetAcc::GetImageEditDpi(int32_t& iImageXDpi, int32_t& iImageYDpi) {
  CXFA_ImageEditData* pData =
      static_cast<CXFA_ImageEditData*>(m_pLayoutData.get());
  iImageXDpi = pData->m_iImageXDpi;
  iImageYDpi = pData->m_iImageYDpi;
}

bool CXFA_WidgetAcc::CalculateTextAutoSize(CFX_SizeF& size) {
  LoadText();
  CXFA_TextLayout* pTextLayout =
      static_cast<CXFA_TextLayoutData*>(m_pLayoutData.get())->GetTextLayout();
  if (pTextLayout) {
    size.width = pTextLayout->StartLayout(size.width);
    size.height = pTextLayout->GetLayoutHeight();
  }
  return CalculateWidgetAutoSize(size);
}

void CXFA_WidgetAcc::LoadText() {
  InitLayoutData();
  static_cast<CXFA_TextLayoutData*>(m_pLayoutData.get())->LoadText(this);
}

float CXFA_WidgetAcc::CalculateWidgetAutoWidth(float fWidthCalc) {
  CXFA_Margin* margin = GetMargin();
  if (margin)
    fWidthCalc += margin->GetLeftInset() + margin->GetRightInset();

  Optional<float> min = TryMinWidth();
  if (min)
    fWidthCalc = std::max(fWidthCalc, *min);

  Optional<float> max = TryMaxWidth();
  if (max && *max > 0)
    fWidthCalc = std::min(fWidthCalc, *max);

  return fWidthCalc;
}

float CXFA_WidgetAcc::GetWidthWithoutMargin(float fWidthCalc) {
  CXFA_Margin* margin = GetMargin();
  if (margin)
    fWidthCalc -= margin->GetLeftInset() + margin->GetRightInset();
  return fWidthCalc;
}

float CXFA_WidgetAcc::CalculateWidgetAutoHeight(float fHeightCalc) {
  CXFA_Margin* margin = GetMargin();
  if (margin)
    fHeightCalc += margin->GetTopInset() + margin->GetBottomInset();

  Optional<float> min = TryMinHeight();
  if (min)
    fHeightCalc = std::max(fHeightCalc, *min);

  Optional<float> max = TryMaxHeight();
  if (max && *max > 0)
    fHeightCalc = std::min(fHeightCalc, *max);

  return fHeightCalc;
}

float CXFA_WidgetAcc::GetHeightWithoutMargin(float fHeightCalc) {
  CXFA_Margin* margin = GetMargin();
  if (margin)
    fHeightCalc -= margin->GetTopInset() + margin->GetBottomInset();
  return fHeightCalc;
}

void CXFA_WidgetAcc::StartWidgetLayout(float& fCalcWidth, float& fCalcHeight) {
  InitLayoutData();
  XFA_Element eUIType = GetUIType();
  if (eUIType == XFA_Element::Text) {
    m_pLayoutData->m_fWidgetHeight = TryHeight().value_or(-1);
    StartTextLayout(fCalcWidth, fCalcHeight);
    return;
  }
  if (fCalcWidth > 0 && fCalcHeight > 0)
    return;

  m_pLayoutData->m_fWidgetHeight = -1;
  float fWidth = 0;
  if (fCalcWidth > 0 && fCalcHeight < 0) {
    Optional<float> height = TryHeight();
    if (height)
      fCalcHeight = *height;
    else
      CalculateAccWidthAndHeight(eUIType, fCalcWidth, fCalcHeight);

    m_pLayoutData->m_fWidgetHeight = fCalcHeight;
    return;
  }
  if (fCalcWidth < 0 && fCalcHeight < 0) {
    Optional<float> height;
    Optional<float> width = TryWidth();
    if (width) {
      fWidth = *width;

      height = TryHeight();
      if (height)
        fCalcHeight = *height;
    }
    if (!width || !height)
      CalculateAccWidthAndHeight(eUIType, fWidth, fCalcHeight);

    fCalcWidth = fWidth;
  }
  m_pLayoutData->m_fWidgetHeight = fCalcHeight;
}

void CXFA_WidgetAcc::CalculateAccWidthAndHeight(XFA_Element eUIType,
                                                float& fWidth,
                                                float& fCalcHeight) {
  CFX_SizeF sz(fWidth, m_pLayoutData->m_fWidgetHeight);
  switch (eUIType) {
    case XFA_Element::Barcode:
    case XFA_Element::ChoiceList:
    case XFA_Element::Signature:
      CalculateFieldAutoSize(sz);
      break;
    case XFA_Element::ImageEdit:
      CalculateImageEditAutoSize(sz);
      break;
    case XFA_Element::Button:
      CalculatePushButtonAutoSize(sz);
      break;
    case XFA_Element::CheckButton:
      CalculateCheckButtonAutoSize(sz);
      break;
    case XFA_Element::DateTimeEdit:
    case XFA_Element::NumericEdit:
    case XFA_Element::PasswordEdit:
    case XFA_Element::TextEdit:
      CalculateTextEditAutoSize(sz);
      break;
    case XFA_Element::Image:
      CalculateImageAutoSize(sz);
      break;
    case XFA_Element::Arc:
    case XFA_Element::Line:
    case XFA_Element::Rectangle:
    case XFA_Element::Subform:
    case XFA_Element::ExclGroup:
      CalculateWidgetAutoSize(sz);
      break;
    default:
      break;
  }
  fWidth = sz.width;
  m_pLayoutData->m_fWidgetHeight = sz.height;
  fCalcHeight = sz.height;
}

bool CXFA_WidgetAcc::FindSplitPos(int32_t iBlockIndex, float& fCalcHeight) {
  XFA_Element eUIType = GetUIType();
  if (eUIType == XFA_Element::Subform)
    return false;

  if (eUIType != XFA_Element::Text && eUIType != XFA_Element::TextEdit &&
      eUIType != XFA_Element::NumericEdit &&
      eUIType != XFA_Element::PasswordEdit) {
    fCalcHeight = 0;
    return true;
  }

  float fTopInset = 0;
  float fBottomInset = 0;
  if (iBlockIndex == 0) {
    CXFA_Margin* margin = GetMargin();
    if (margin) {
      fTopInset = margin->GetTopInset();
      fBottomInset = margin->GetBottomInset();
    }

    CFX_RectF rtUIMargin = GetUIMargin();
    fTopInset += rtUIMargin.top;
    fBottomInset += rtUIMargin.width;
  }
  if (eUIType == XFA_Element::Text) {
    float fHeight = fCalcHeight;
    if (iBlockIndex == 0) {
      fCalcHeight = fCalcHeight - fTopInset;
      if (fCalcHeight < 0)
        fCalcHeight = 0;
    }

    CXFA_TextLayout* pTextLayout =
        static_cast<CXFA_TextLayoutData*>(m_pLayoutData.get())->GetTextLayout();
    fCalcHeight =
        pTextLayout->DoLayout(iBlockIndex, fCalcHeight, fCalcHeight,
                              m_pLayoutData->m_fWidgetHeight - fTopInset);
    if (fCalcHeight != 0) {
      if (iBlockIndex == 0)
        fCalcHeight = fCalcHeight + fTopInset;
      if (fabs(fHeight - fCalcHeight) < XFA_FLOAT_PERCISION)
        return false;
    }
    return true;
  }
  XFA_AttributeEnum iCapPlacement = XFA_AttributeEnum::Unknown;
  float fCapReserve = 0;
  if (iBlockIndex == 0) {
    CXFA_Caption* caption = GetCaption();
    if (caption && !caption->IsHidden()) {
      iCapPlacement = caption->GetPlacementType();
      fCapReserve = caption->GetReserve();
    }
    if (iCapPlacement == XFA_AttributeEnum::Top &&
        fCalcHeight < fCapReserve + fTopInset) {
      fCalcHeight = 0;
      return true;
    }
    if (iCapPlacement == XFA_AttributeEnum::Bottom &&
        m_pLayoutData->m_fWidgetHeight - fCapReserve - fBottomInset) {
      fCalcHeight = 0;
      return true;
    }
    if (iCapPlacement != XFA_AttributeEnum::Top)
      fCapReserve = 0;
  }
  CXFA_FieldLayoutData* pFieldData =
      static_cast<CXFA_FieldLayoutData*>(m_pLayoutData.get());
  int32_t iLinesCount = 0;
  float fHeight = m_pLayoutData->m_fWidgetHeight;
  if (GetValue(XFA_VALUEPICTURE_Display).IsEmpty()) {
    iLinesCount = 1;
  } else {
    if (!pFieldData->m_pTextOut) {
      // TODO(dsinclair): Inline fWidth when the 2nd param of
      // CalculateAccWidthAndHeight isn't a ref-param.
      float fWidth = TryWidth().value_or(0);
      CalculateAccWidthAndHeight(eUIType, fWidth, fHeight);
    }
    iLinesCount = pFieldData->m_pTextOut->GetTotalLines();
  }
  std::vector<float>* pFieldArray = &pFieldData->m_FieldSplitArray;
  int32_t iFieldSplitCount = pdfium::CollectionSize<int32_t>(*pFieldArray);
  for (int32_t i = 0; i < iBlockIndex * 3; i += 3) {
    iLinesCount -= (int32_t)(*pFieldArray)[i + 1];
    fHeight -= (*pFieldArray)[i + 2];
  }
  if (iLinesCount == 0)
    return false;

  float fLineHeight = GetLineHeight();
  float fFontSize = GetFontSize();
  float fTextHeight = iLinesCount * fLineHeight - fLineHeight + fFontSize;
  float fSpaceAbove = 0;
  float fStartOffset = 0;
  if (fHeight > 0.1f && iBlockIndex == 0) {
    fStartOffset = fTopInset;
    fHeight -= (fTopInset + fBottomInset);
    CXFA_Para* para = GetPara();
    if (para) {
      fSpaceAbove = para->GetSpaceAbove();
      float fSpaceBelow = para->GetSpaceBelow();
      fHeight -= (fSpaceAbove + fSpaceBelow);
      switch (para->GetVerticalAlign()) {
        case XFA_AttributeEnum::Top:
          fStartOffset += fSpaceAbove;
          break;
        case XFA_AttributeEnum::Middle:
          fStartOffset += ((fHeight - fTextHeight) / 2 + fSpaceAbove);
          break;
        case XFA_AttributeEnum::Bottom:
          fStartOffset += (fHeight - fTextHeight + fSpaceAbove);
          break;
        default:
          NOTREACHED();
          break;
      }
    }
    if (fStartOffset < 0.1f)
      fStartOffset = 0;
  }
  for (int32_t i = iBlockIndex - 1; iBlockIndex > 0 && i < iBlockIndex; i++) {
    fStartOffset = (*pFieldArray)[i * 3] - (*pFieldArray)[i * 3 + 2];
    if (fStartOffset < 0.1f)
      fStartOffset = 0;
  }
  if (iFieldSplitCount / 3 == (iBlockIndex + 1))
    (*pFieldArray)[0] = fStartOffset;
  else
    pFieldArray->push_back(fStartOffset);

  XFA_VERSION version = GetDoc()->GetXFADoc()->GetCurVersionMode();
  bool bCanSplitNoContent = false;
  XFA_AttributeEnum eLayoutMode = GetNode()
                                      ->GetNodeItem(XFA_NODEITEM_Parent)
                                      ->JSObject()
                                      ->TryEnum(XFA_Attribute::Layout, true)
                                      .value_or(XFA_AttributeEnum::Position);
  if ((eLayoutMode == XFA_AttributeEnum::Position ||
       eLayoutMode == XFA_AttributeEnum::Tb ||
       eLayoutMode == XFA_AttributeEnum::Row ||
       eLayoutMode == XFA_AttributeEnum::Table) &&
      version > XFA_VERSION_208) {
    bCanSplitNoContent = true;
  }
  if ((eLayoutMode == XFA_AttributeEnum::Tb ||
       eLayoutMode == XFA_AttributeEnum::Row ||
       eLayoutMode == XFA_AttributeEnum::Table) &&
      version <= XFA_VERSION_208) {
    if (fStartOffset < fCalcHeight) {
      bCanSplitNoContent = true;
    } else {
      fCalcHeight = 0;
      return true;
    }
  }
  if (bCanSplitNoContent) {
    if ((fCalcHeight - fTopInset - fSpaceAbove < fLineHeight)) {
      fCalcHeight = 0;
      return true;
    }
    if (fStartOffset + XFA_FLOAT_PERCISION >= fCalcHeight) {
      if (iFieldSplitCount / 3 == (iBlockIndex + 1)) {
        (*pFieldArray)[iBlockIndex * 3 + 1] = 0;
        (*pFieldArray)[iBlockIndex * 3 + 2] = fCalcHeight;
      } else {
        pFieldArray->push_back(0);
        pFieldArray->push_back(fCalcHeight);
      }
      return false;
    }
    if (fCalcHeight - fStartOffset < fLineHeight) {
      fCalcHeight = fStartOffset;
      if (iFieldSplitCount / 3 == (iBlockIndex + 1)) {
        (*pFieldArray)[iBlockIndex * 3 + 1] = 0;
        (*pFieldArray)[iBlockIndex * 3 + 2] = fCalcHeight;
      } else {
        pFieldArray->push_back(0);
        pFieldArray->push_back(fCalcHeight);
      }
      return true;
    }
    float fTextNum =
        fCalcHeight + XFA_FLOAT_PERCISION - fCapReserve - fStartOffset;
    int32_t iLineNum =
        (int32_t)((fTextNum + (fLineHeight - fFontSize)) / fLineHeight);
    if (iLineNum >= iLinesCount) {
      if (fCalcHeight - fStartOffset - fTextHeight >= fFontSize) {
        if (iFieldSplitCount / 3 == (iBlockIndex + 1)) {
          (*pFieldArray)[iBlockIndex * 3 + 1] = (float)iLinesCount;
          (*pFieldArray)[iBlockIndex * 3 + 2] = fCalcHeight;
        } else {
          pFieldArray->push_back((float)iLinesCount);
          pFieldArray->push_back(fCalcHeight);
        }
        return false;
      }
      if (fHeight - fStartOffset - fTextHeight < fFontSize) {
        iLineNum -= 1;
        if (iLineNum == 0) {
          fCalcHeight = 0;
          return true;
        }
      } else {
        iLineNum = (int32_t)(fTextNum / fLineHeight);
      }
    }
    if (iLineNum > 0) {
      float fSplitHeight = iLineNum * fLineHeight + fCapReserve + fStartOffset;
      if (iFieldSplitCount / 3 == (iBlockIndex + 1)) {
        (*pFieldArray)[iBlockIndex * 3 + 1] = (float)iLineNum;
        (*pFieldArray)[iBlockIndex * 3 + 2] = fSplitHeight;
      } else {
        pFieldArray->push_back((float)iLineNum);
        pFieldArray->push_back(fSplitHeight);
      }
      if (fabs(fSplitHeight - fCalcHeight) < XFA_FLOAT_PERCISION)
        return false;

      fCalcHeight = fSplitHeight;
      return true;
    }
  }
  fCalcHeight = 0;
  return true;
}

void CXFA_WidgetAcc::InitLayoutData() {
  if (m_pLayoutData)
    return;

  switch (GetUIType()) {
    case XFA_Element::Text:
      m_pLayoutData = pdfium::MakeUnique<CXFA_TextLayoutData>();
      return;
    case XFA_Element::TextEdit:
      m_pLayoutData = pdfium::MakeUnique<CXFA_TextEditData>();
      return;
    case XFA_Element::Image:
      m_pLayoutData = pdfium::MakeUnique<CXFA_ImageLayoutData>();
      return;
    case XFA_Element::ImageEdit:
      m_pLayoutData = pdfium::MakeUnique<CXFA_ImageEditData>();
      return;
    default:
      break;
  }
  if (GetElementType() == XFA_Element::Field) {
    m_pLayoutData = pdfium::MakeUnique<CXFA_FieldLayoutData>();
    return;
  }
  m_pLayoutData = pdfium::MakeUnique<CXFA_WidgetLayoutData>();
}

void CXFA_WidgetAcc::StartTextLayout(float& fCalcWidth, float& fCalcHeight) {
  LoadText();
  CXFA_TextLayout* pTextLayout =
      static_cast<CXFA_TextLayoutData*>(m_pLayoutData.get())->GetTextLayout();
  float fTextHeight = 0;
  if (fCalcWidth > 0 && fCalcHeight > 0) {
    float fWidth = GetWidthWithoutMargin(fCalcWidth);
    pTextLayout->StartLayout(fWidth);
    fTextHeight = fCalcHeight;
    fTextHeight = GetHeightWithoutMargin(fTextHeight);
    pTextLayout->DoLayout(0, fTextHeight, -1, fTextHeight);
    return;
  }
  if (fCalcWidth > 0 && fCalcHeight < 0) {
    float fWidth = GetWidthWithoutMargin(fCalcWidth);
    pTextLayout->StartLayout(fWidth);
  }

  if (fCalcWidth < 0 && fCalcHeight < 0) {
    Optional<float> width = TryWidth();
    if (width) {
      pTextLayout->StartLayout(GetWidthWithoutMargin(*width));
      fCalcWidth = *width;
    } else {
      float fMaxWidth = CalculateWidgetAutoWidth(pTextLayout->StartLayout(-1));
      pTextLayout->StartLayout(GetWidthWithoutMargin(fMaxWidth));
      fCalcWidth = fMaxWidth;
    }
  }

  if (m_pLayoutData->m_fWidgetHeight < 0) {
    m_pLayoutData->m_fWidgetHeight = pTextLayout->GetLayoutHeight();
    m_pLayoutData->m_fWidgetHeight =
        CalculateWidgetAutoHeight(m_pLayoutData->m_fWidgetHeight);
  }
  fTextHeight = m_pLayoutData->m_fWidgetHeight;
  fTextHeight = GetHeightWithoutMargin(fTextHeight);
  pTextLayout->DoLayout(0, fTextHeight, -1, fTextHeight);
  fCalcHeight = m_pLayoutData->m_fWidgetHeight;
}

bool CXFA_WidgetAcc::LoadCaption() {
  InitLayoutData();
  return static_cast<CXFA_FieldLayoutData*>(m_pLayoutData.get())
      ->LoadCaption(this);
}

CXFA_TextLayout* CXFA_WidgetAcc::GetCaptionTextLayout() {
  return m_pLayoutData
             ? static_cast<CXFA_FieldLayoutData*>(m_pLayoutData.get())
                   ->m_pCapTextLayout.get()
             : nullptr;
}

CXFA_TextLayout* CXFA_WidgetAcc::GetTextLayout() {
  return m_pLayoutData
             ? static_cast<CXFA_TextLayoutData*>(m_pLayoutData.get())
                   ->GetTextLayout()
             : nullptr;
}

RetainPtr<CFX_DIBitmap> CXFA_WidgetAcc::GetImageImage() {
  return m_pLayoutData
             ? static_cast<CXFA_ImageLayoutData*>(m_pLayoutData.get())
                   ->m_pDIBitmap
             : nullptr;
}

RetainPtr<CFX_DIBitmap> CXFA_WidgetAcc::GetImageEditImage() {
  return m_pLayoutData
             ? static_cast<CXFA_ImageEditData*>(m_pLayoutData.get())
                   ->m_pDIBitmap
             : nullptr;
}

void CXFA_WidgetAcc::SetImageImage(const RetainPtr<CFX_DIBitmap>& newImage) {
  CXFA_ImageLayoutData* pData =
      static_cast<CXFA_ImageLayoutData*>(m_pLayoutData.get());
  if (pData->m_pDIBitmap != newImage)
    pData->m_pDIBitmap = newImage;
}

void CXFA_WidgetAcc::SetImageEditImage(
    const RetainPtr<CFX_DIBitmap>& newImage) {
  CXFA_ImageEditData* pData =
      static_cast<CXFA_ImageEditData*>(m_pLayoutData.get());
  if (pData->m_pDIBitmap != newImage)
    pData->m_pDIBitmap = newImage;
}

RetainPtr<CFGAS_GEFont> CXFA_WidgetAcc::GetFDEFont() {
  WideString wsFontName = L"Courier";
  uint32_t dwFontStyle = 0;
  CXFA_Font* font = GetFont(false);
  if (font) {
    if (font->IsBold())
      dwFontStyle |= FXFONT_BOLD;
    if (font->IsItalic())
      dwFontStyle |= FXFONT_ITALIC;

    wsFontName = font->GetTypeface();
  }

  auto* pDoc = GetDoc();
  return pDoc->GetApp()->GetXFAFontMgr()->GetFont(
      pDoc, wsFontName.AsStringView(), dwFontStyle);
}

float CXFA_WidgetAcc::GetFontSize() {
  CXFA_Font* font = GetFont(false);
  float fFontSize = font ? font->GetFontSize() : 10.0f;
  return fFontSize < 0.1f ? 10.0f : fFontSize;
}

float CXFA_WidgetAcc::GetLineHeight() {
  float fLineHeight = 0;
  CXFA_Para* para = GetPara();
  if (para)
    fLineHeight = para->GetLineHeight();
  if (fLineHeight < 1)
    fLineHeight = GetFontSize() * 1.2f;
  return fLineHeight;
}

FX_ARGB CXFA_WidgetAcc::GetTextColor() {
  CXFA_Font* font = GetFont(false);
  return font ? font->GetColor() : 0xFF000000;
}

CXFA_Node* CXFA_WidgetAcc::GetUIChild() {
  if (m_eUIType == XFA_Element::Unknown)
    m_pUiChildNode = CreateUIChild(m_pNode, m_eUIType);

  return m_pUiChildNode;
}

XFA_Element CXFA_WidgetAcc::GetUIType() {
  GetUIChild();
  return m_eUIType;
}

WideString CXFA_WidgetAcc::GetRawValue() const {
  return m_pNode->JSObject()->GetContent(false);
}

bool CXFA_WidgetAcc::IsOpenAccess() const {
  for (CXFA_Node* pNode = m_pNode; pNode;
       pNode = pNode->GetNodeItem(XFA_NODEITEM_Parent,
                                  XFA_ObjectType::ContainerNode)) {
    XFA_AttributeEnum iAcc = pNode->JSObject()->GetEnum(XFA_Attribute::Access);
    if (iAcc != XFA_AttributeEnum::Open)
      return false;
  }
  return true;
}

int32_t CXFA_WidgetAcc::GetRotate() const {
  Optional<int32_t> degrees =
      m_pNode->JSObject()->TryInteger(XFA_Attribute::Rotate, false);
  return degrees ? XFA_MapRotation(*degrees) / 90 * 90 : 0;
}

CXFA_Border* CXFA_WidgetAcc::GetBorder(bool bModified) {
  return m_pNode->JSObject()->GetProperty<CXFA_Border>(0, XFA_Element::Border,
                                                       bModified);
}

CXFA_Caption* CXFA_WidgetAcc::GetCaption() {
  return m_pNode->JSObject()->GetProperty<CXFA_Caption>(0, XFA_Element::Caption,
                                                        false);
}

CXFA_Font* CXFA_WidgetAcc::GetFont(bool bModified) {
  return m_pNode->JSObject()->GetProperty<CXFA_Font>(0, XFA_Element::Font,
                                                     bModified);
}

CXFA_Margin* CXFA_WidgetAcc::GetMargin() {
  return m_pNode->JSObject()->GetProperty<CXFA_Margin>(0, XFA_Element::Margin,
                                                       false);
}

CXFA_Para* CXFA_WidgetAcc::GetPara() {
  return m_pNode->JSObject()->GetProperty<CXFA_Para>(0, XFA_Element::Para,
                                                     false);
}

std::vector<CXFA_Event*> CXFA_WidgetAcc::GetEventByActivity(
    XFA_AttributeEnum iActivity,
    bool bIsFormReady) {
  std::vector<CXFA_Event*> events;
  for (CXFA_Node* node : m_pNode->GetNodeList(0, XFA_Element::Event)) {
    auto* event = static_cast<CXFA_Event*>(node);
    if (event->GetActivity() == iActivity) {
      if (iActivity == XFA_AttributeEnum::Ready) {
        WideString wsRef = event->GetRef();
        if (bIsFormReady) {
          if (wsRef == WideStringView(L"$form"))
            events.push_back(event);
        } else {
          if (wsRef == WideStringView(L"$layout"))
            events.push_back(event);
        }
      } else {
        events.push_back(event);
      }
    }
  }
  return events;
}

CXFA_Value* CXFA_WidgetAcc::GetDefaultValue() {
  CXFA_Node* pTemNode = m_pNode->GetTemplateNode();
  return pTemNode->JSObject()->GetProperty<CXFA_Value>(0, XFA_Element::Value,
                                                       false);
}

CXFA_Value* CXFA_WidgetAcc::GetFormValue() {
  return m_pNode->JSObject()->GetProperty<CXFA_Value>(0, XFA_Element::Value,
                                                      false);
}

CXFA_Calculate* CXFA_WidgetAcc::GetCalculate() {
  return m_pNode->JSObject()->GetProperty<CXFA_Calculate>(
      0, XFA_Element::Calculate, false);
}

CXFA_Validate* CXFA_WidgetAcc::GetValidate(bool bModified) {
  return m_pNode->JSObject()->GetProperty<CXFA_Validate>(
      0, XFA_Element::Validate, bModified);
}

CXFA_Bind* CXFA_WidgetAcc::GetBind() {
  return m_pNode->JSObject()->GetProperty<CXFA_Bind>(0, XFA_Element::Bind,
                                                     false);
}

Optional<float> CXFA_WidgetAcc::TryWidth() {
  return m_pNode->JSObject()->TryMeasureAsFloat(XFA_Attribute::W);
}

Optional<float> CXFA_WidgetAcc::TryHeight() {
  return m_pNode->JSObject()->TryMeasureAsFloat(XFA_Attribute::H);
}

Optional<float> CXFA_WidgetAcc::TryMinWidth() {
  return m_pNode->JSObject()->TryMeasureAsFloat(XFA_Attribute::MinW);
}

Optional<float> CXFA_WidgetAcc::TryMinHeight() {
  return m_pNode->JSObject()->TryMeasureAsFloat(XFA_Attribute::MinH);
}

Optional<float> CXFA_WidgetAcc::TryMaxWidth() {
  return m_pNode->JSObject()->TryMeasureAsFloat(XFA_Attribute::MaxW);
}

Optional<float> CXFA_WidgetAcc::TryMaxHeight() {
  return m_pNode->JSObject()->TryMeasureAsFloat(XFA_Attribute::MaxH);
}

CXFA_Border* CXFA_WidgetAcc::GetUIBorder() {
  CXFA_Node* pUIChild = GetUIChild();
  return pUIChild ? pUIChild->JSObject()->GetProperty<CXFA_Border>(
                        0, XFA_Element::Border, false)
                  : nullptr;
}

CFX_RectF CXFA_WidgetAcc::GetUIMargin() {
  CXFA_Node* pUIChild = GetUIChild();
  CXFA_Margin* mgUI = nullptr;
  if (pUIChild) {
    mgUI = pUIChild->JSObject()->GetProperty<CXFA_Margin>(
        0, XFA_Element::Margin, false);
  }

  if (!mgUI)
    return CFX_RectF();

  CXFA_Border* border = GetUIBorder();
  if (border && border->GetPresence() != XFA_AttributeEnum::Visible)
    return CFX_RectF();

  Optional<float> left = mgUI->TryLeftInset();
  Optional<float> top = mgUI->TryTopInset();
  Optional<float> right = mgUI->TryRightInset();
  Optional<float> bottom = mgUI->TryBottomInset();
  if (border) {
    bool bVisible = false;
    float fThickness = 0;
    XFA_AttributeEnum iType = XFA_AttributeEnum::Unknown;
    std::tie(iType, bVisible, fThickness) = border->Get3DStyle();
    if (!left || !top || !right || !bottom) {
      std::vector<CXFA_Stroke*> strokes = border->GetStrokes();
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

XFA_AttributeEnum CXFA_WidgetAcc::GetButtonHighlight() {
  CXFA_Node* pUIChild = GetUIChild();
  if (pUIChild)
    return pUIChild->JSObject()->GetEnum(XFA_Attribute::Highlight);
  return XFA_AttributeEnum::Inverted;
}

bool CXFA_WidgetAcc::HasButtonRollover() const {
  CXFA_Items* pItems =
      m_pNode->GetChild<CXFA_Items>(0, XFA_Element::Items, false);
  if (!pItems)
    return false;

  for (CXFA_Node* pText = pItems->GetNodeItem(XFA_NODEITEM_FirstChild); pText;
       pText = pText->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pText->JSObject()->GetCData(XFA_Attribute::Name) == L"rollover")
      return !pText->JSObject()->GetContent(false).IsEmpty();
  }
  return false;
}

bool CXFA_WidgetAcc::HasButtonDown() const {
  CXFA_Items* pItems =
      m_pNode->GetChild<CXFA_Items>(0, XFA_Element::Items, false);
  if (!pItems)
    return false;

  for (CXFA_Node* pText = pItems->GetNodeItem(XFA_NODEITEM_FirstChild); pText;
       pText = pText->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pText->JSObject()->GetCData(XFA_Attribute::Name) == L"down")
      return !pText->JSObject()->GetContent(false).IsEmpty();
  }
  return false;
}

bool CXFA_WidgetAcc::IsCheckButtonRound() {
  CXFA_Node* pUIChild = GetUIChild();
  if (pUIChild)
    return pUIChild->JSObject()->GetEnum(XFA_Attribute::Shape) ==
           XFA_AttributeEnum::Round;
  return false;
}

XFA_AttributeEnum CXFA_WidgetAcc::GetCheckButtonMark() {
  CXFA_Node* pUIChild = GetUIChild();
  if (pUIChild)
    return pUIChild->JSObject()->GetEnum(XFA_Attribute::Mark);
  return XFA_AttributeEnum::Default;
}

bool CXFA_WidgetAcc::IsRadioButton() {
  CXFA_Node* pParent = m_pNode->GetNodeItem(XFA_NODEITEM_Parent);
  return pParent && pParent->GetElementType() == XFA_Element::ExclGroup;
}

float CXFA_WidgetAcc::GetCheckButtonSize() {
  CXFA_Node* pUIChild = GetUIChild();
  if (pUIChild) {
    return pUIChild->JSObject()
        ->GetMeasure(XFA_Attribute::Size)
        .ToUnit(XFA_Unit::Pt);
  }
  return CXFA_Measurement(10, XFA_Unit::Pt).ToUnit(XFA_Unit::Pt);
}

bool CXFA_WidgetAcc::IsAllowNeutral() {
  CXFA_Node* pUIChild = GetUIChild();
  return pUIChild &&
         pUIChild->JSObject()->GetBoolean(XFA_Attribute::AllowNeutral);
}

XFA_CHECKSTATE CXFA_WidgetAcc::GetCheckState() {
  WideString wsValue = GetRawValue();
  if (wsValue.IsEmpty())
    return XFA_CHECKSTATE_Off;

  auto* pItems = m_pNode->GetChild<CXFA_Items>(0, XFA_Element::Items, false);
  if (!pItems)
    return XFA_CHECKSTATE_Off;

  CXFA_Node* pText = pItems->GetNodeItem(XFA_NODEITEM_FirstChild);
  int32_t i = 0;
  while (pText) {
    Optional<WideString> wsContent = pText->JSObject()->TryContent(false, true);
    if (wsContent && *wsContent == wsValue)
      return static_cast<XFA_CHECKSTATE>(i);

    i++;
    pText = pText->GetNodeItem(XFA_NODEITEM_NextSibling);
  }
  return XFA_CHECKSTATE_Off;
}

void CXFA_WidgetAcc::SetCheckState(XFA_CHECKSTATE eCheckState, bool bNotify) {
  CXFA_Node* node = GetExclGroupNode();
  if (!node) {
    CXFA_Items* pItems =
        m_pNode->GetChild<CXFA_Items>(0, XFA_Element::Items, false);
    if (!pItems)
      return;

    int32_t i = -1;
    CXFA_Node* pText = pItems->GetNodeItem(XFA_NODEITEM_FirstChild);
    WideString wsContent;
    while (pText) {
      i++;
      if (i == eCheckState) {
        wsContent = pText->JSObject()->GetContent(false);
        break;
      }
      pText = pText->GetNodeItem(XFA_NODEITEM_NextSibling);
    }
    if (m_pNode)
      m_pNode->SyncValue(wsContent, bNotify);
    return;
  }

  WideString wsValue;
  if (eCheckState != XFA_CHECKSTATE_Off) {
    if (CXFA_Items* pItems =
            m_pNode->GetChild<CXFA_Items>(0, XFA_Element::Items, false)) {
      CXFA_Node* pText = pItems->GetNodeItem(XFA_NODEITEM_FirstChild);
      if (pText)
        wsValue = pText->JSObject()->GetContent(false);
    }
  }
  CXFA_Node* pChild = node->GetNodeItem(XFA_NODEITEM_FirstChild);
  for (; pChild; pChild = pChild->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pChild->GetElementType() != XFA_Element::Field)
      continue;

    CXFA_Items* pItem =
        pChild->GetChild<CXFA_Items>(0, XFA_Element::Items, false);
    if (!pItem)
      continue;

    CXFA_Node* pItemchild = pItem->GetNodeItem(XFA_NODEITEM_FirstChild);
    if (!pItemchild)
      continue;

    WideString text = pItemchild->JSObject()->GetContent(false);
    WideString wsChildValue = text;
    if (wsValue != text) {
      pItemchild = pItemchild->GetNodeItem(XFA_NODEITEM_NextSibling);
      if (pItemchild)
        wsChildValue = pItemchild->JSObject()->GetContent(false);
      else
        wsChildValue.clear();
    }
    pChild->SyncValue(wsChildValue, bNotify);
  }
  node->SyncValue(wsValue, bNotify);
}

CXFA_Node* CXFA_WidgetAcc::GetExclGroupNode() {
  CXFA_Node* pExcl = ToNode(m_pNode->GetNodeItem(XFA_NODEITEM_Parent));
  if (!pExcl || pExcl->GetElementType() != XFA_Element::ExclGroup)
    return nullptr;
  return pExcl;
}

CXFA_Node* CXFA_WidgetAcc::GetSelectedMember() {
  CXFA_Node* pSelectedMember = nullptr;
  WideString wsState = GetRawValue();
  if (wsState.IsEmpty())
    return pSelectedMember;

  for (CXFA_Node* pNode = ToNode(m_pNode->GetNodeItem(XFA_NODEITEM_FirstChild));
       pNode; pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    CXFA_WidgetAcc widgetData(nullptr, pNode);
    if (widgetData.GetCheckState() == XFA_CHECKSTATE_On) {
      pSelectedMember = pNode;
      break;
    }
  }
  return pSelectedMember;
}

CXFA_Node* CXFA_WidgetAcc::SetSelectedMember(const WideStringView& wsName,
                                             bool bNotify) {
  uint32_t nameHash = FX_HashCode_GetW(wsName, false);
  for (CXFA_Node* pNode = ToNode(m_pNode->GetNodeItem(XFA_NODEITEM_FirstChild));
       pNode; pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pNode->GetNameHash() == nameHash) {
      CXFA_WidgetAcc widgetData(nullptr, pNode);
      widgetData.SetCheckState(XFA_CHECKSTATE_On, bNotify);
      return pNode;
    }
  }
  return nullptr;
}

void CXFA_WidgetAcc::SetSelectedMemberByValue(const WideStringView& wsValue,
                                              bool bNotify,
                                              bool bScriptModify,
                                              bool bSyncData) {
  WideString wsExclGroup;
  for (CXFA_Node* pNode = m_pNode->GetNodeItem(XFA_NODEITEM_FirstChild); pNode;
       pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pNode->GetElementType() != XFA_Element::Field)
      continue;

    CXFA_Items* pItem =
        pNode->GetChild<CXFA_Items>(0, XFA_Element::Items, false);
    if (!pItem)
      continue;

    CXFA_Node* pItemchild = pItem->GetNodeItem(XFA_NODEITEM_FirstChild);
    if (!pItemchild)
      continue;

    WideString wsChildValue = pItemchild->JSObject()->GetContent(false);
    if (wsValue != wsChildValue) {
      pItemchild = pItemchild->GetNodeItem(XFA_NODEITEM_NextSibling);
      if (pItemchild)
        wsChildValue = pItemchild->JSObject()->GetContent(false);
      else
        wsChildValue.clear();
    } else {
      wsExclGroup = wsValue;
    }
    pNode->JSObject()->SetContent(wsChildValue, wsChildValue, bNotify,
                                  bScriptModify, false);
  }
  if (m_pNode) {
    m_pNode->JSObject()->SetContent(wsExclGroup, wsExclGroup, bNotify,
                                    bScriptModify, bSyncData);
  }
}

CXFA_Node* CXFA_WidgetAcc::GetExclGroupFirstMember() {
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
CXFA_Node* CXFA_WidgetAcc::GetExclGroupNextMember(CXFA_Node* pNode) {
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

bool CXFA_WidgetAcc::IsChoiceListCommitOnSelect() {
  CXFA_Node* pUIChild = GetUIChild();
  if (pUIChild) {
    return pUIChild->JSObject()->GetEnum(XFA_Attribute::CommitOn) ==
           XFA_AttributeEnum::Select;
  }
  return true;
}

bool CXFA_WidgetAcc::IsChoiceListAllowTextEntry() {
  CXFA_Node* pUIChild = GetUIChild();
  return pUIChild && pUIChild->JSObject()->GetBoolean(XFA_Attribute::TextEntry);
}

bool CXFA_WidgetAcc::IsChoiceListMultiSelect() {
  CXFA_Node* pUIChild = GetUIChild();
  if (pUIChild) {
    return pUIChild->JSObject()->GetEnum(XFA_Attribute::Open) ==
           XFA_AttributeEnum::MultiSelect;
  }
  return false;
}

bool CXFA_WidgetAcc::IsListBox() {
  CXFA_Node* pUIChild = GetUIChild();
  if (!pUIChild)
    return false;

  XFA_AttributeEnum attr = pUIChild->JSObject()->GetEnum(XFA_Attribute::Open);
  return attr == XFA_AttributeEnum::Always ||
         attr == XFA_AttributeEnum::MultiSelect;
}

int32_t CXFA_WidgetAcc::CountChoiceListItems(bool bSaveValue) {
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
    bool bItemOneHasSave =
        pItems[0]->JSObject()->GetBoolean(XFA_Attribute::Save);
    bool bItemTwoHasSave =
        pItems[1]->JSObject()->GetBoolean(XFA_Attribute::Save);
    if (bItemOneHasSave != bItemTwoHasSave && bSaveValue == bItemTwoHasSave)
      pItem = pItems[1];
  }
  return pItem->CountChildren(XFA_Element::Unknown, false);
}

Optional<WideString> CXFA_WidgetAcc::GetChoiceListItem(int32_t nIndex,
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
        pItemsArray[0]->JSObject()->GetBoolean(XFA_Attribute::Save);
    bool bItemTwoHasSave =
        pItemsArray[1]->JSObject()->GetBoolean(XFA_Attribute::Save);
    if (bItemOneHasSave != bItemTwoHasSave && bSaveValue == bItemTwoHasSave)
      pItems = pItemsArray[1];
  }
  if (!pItems)
    return {};

  CXFA_Node* pItem =
      pItems->GetChild<CXFA_Node>(nIndex, XFA_Element::Unknown, false);
  if (pItem)
    return {pItem->JSObject()->GetContent(false)};
  return {};
}

std::vector<WideString> CXFA_WidgetAcc::GetChoiceListItems(bool bSaveValue) {
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
    bool bItemOneHasSave =
        items[0]->JSObject()->GetBoolean(XFA_Attribute::Save);
    bool bItemTwoHasSave =
        items[1]->JSObject()->GetBoolean(XFA_Attribute::Save);
    if (bItemOneHasSave != bItemTwoHasSave && bSaveValue == bItemTwoHasSave)
      pItem = items[1];
  }

  std::vector<WideString> wsTextArray;
  for (CXFA_Node* pNode = pItem->GetNodeItem(XFA_NODEITEM_FirstChild); pNode;
       pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    wsTextArray.emplace_back(pNode->JSObject()->GetContent(false));
  }
  return wsTextArray;
}

int32_t CXFA_WidgetAcc::CountSelectedItems() {
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

int32_t CXFA_WidgetAcc::GetSelectedItem(int32_t nIndex) {
  std::vector<WideString> wsValueArray = GetSelectedItemsValue();
  if (!pdfium::IndexInBounds(wsValueArray, nIndex))
    return -1;

  std::vector<WideString> wsSaveTextArray = GetChoiceListItems(true);
  auto it = std::find(wsSaveTextArray.begin(), wsSaveTextArray.end(),
                      wsValueArray[nIndex]);
  return it != wsSaveTextArray.end() ? it - wsSaveTextArray.begin() : -1;
}

std::vector<int32_t> CXFA_WidgetAcc::GetSelectedItems() {
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

std::vector<WideString> CXFA_WidgetAcc::GetSelectedItemsValue() {
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

bool CXFA_WidgetAcc::GetItemState(int32_t nIndex) {
  std::vector<WideString> wsSaveTextArray = GetChoiceListItems(true);
  return pdfium::IndexInBounds(wsSaveTextArray, nIndex) &&
         pdfium::ContainsValue(GetSelectedItemsValue(),
                               wsSaveTextArray[nIndex]);
}

void CXFA_WidgetAcc::SetItemState(int32_t nIndex,
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
        m_pNode->JSObject()->SetContent(wsValue, wsValue, bNotify,
                                        bScriptModify, bSyncData);
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
        m_pNode->JSObject()->SetContent(wsSaveText,
                                        GetFormatDataValue(wsSaveText), bNotify,
                                        bScriptModify, bSyncData);
      }
    } else if (iSel >= 0) {
      m_pNode->JSObject()->SetContent(WideString(), WideString(), bNotify,
                                      bScriptModify, bSyncData);
    }
  }
}

void CXFA_WidgetAcc::SetSelectedItems(const std::vector<int32_t>& iSelArray,
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

  m_pNode->JSObject()->SetContent(wsValue, wsFormat, bNotify, bScriptModify,
                                  bSyncData);
}

void CXFA_WidgetAcc::ClearAllSelections() {
  CXFA_Node* pBind = m_pNode->GetBindData();
  if (!pBind || !IsChoiceListMultiSelect()) {
    m_pNode->SyncValue(WideString(), false);
    return;
  }

  while (CXFA_Node* pChildNode = pBind->GetNodeItem(XFA_NODEITEM_FirstChild))
    pBind->RemoveChild(pChildNode, true);
}

void CXFA_WidgetAcc::InsertItem(const WideString& wsLabel,
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
    pSaveItems->JSObject()->SetBoolean(XFA_Attribute::Save, true, false);
    InsertListTextItem(pSaveItems, wsNewValue, nIndex);
  } else if (listitems.size() > 1) {
    for (int32_t i = 0; i < 2; i++) {
      CXFA_Node* pNode = listitems[i];
      bool bHasSave = pNode->JSObject()->GetBoolean(XFA_Attribute::Save);
      if (bHasSave)
        InsertListTextItem(pNode, wsNewValue, nIndex);
      else
        InsertListTextItem(pNode, wsLabel, nIndex);
    }
  } else {
    CXFA_Node* pNode = listitems[0];
    pNode->JSObject()->SetBoolean(XFA_Attribute::Save, false, false);
    pNode->JSObject()->SetEnum(XFA_Attribute::Presence,
                               XFA_AttributeEnum::Visible, false);
    CXFA_Node* pSaveItems = m_pNode->CreateSamePacketNode(XFA_Element::Items);
    m_pNode->InsertChild(-1, pSaveItems);
    pSaveItems->JSObject()->SetBoolean(XFA_Attribute::Save, true, false);
    pSaveItems->JSObject()->SetEnum(XFA_Attribute::Presence,
                                    XFA_AttributeEnum::Hidden, false);
    CXFA_Node* pListNode = pNode->GetNodeItem(XFA_NODEITEM_FirstChild);
    int32_t i = 0;
    while (pListNode) {
      InsertListTextItem(pSaveItems, pListNode->JSObject()->GetContent(false),
                         i);
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

void CXFA_WidgetAcc::GetItemLabel(const WideStringView& wsValue,
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
  bool bSave = pLabelItems->JSObject()->GetBoolean(XFA_Attribute::Save);
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
    if (pChildItem->JSObject()->GetContent(false) == wsValue) {
      iSearch = iCount;
      break;
    }
    iCount++;
  }
  if (iSearch < 0)
    return;

  CXFA_Node* pText =
      pLabelItems->GetChild<CXFA_Node>(iSearch, XFA_Element::Unknown, false);
  if (pText)
    wsLabel = pText->JSObject()->GetContent(false);
}

WideString CXFA_WidgetAcc::GetItemValue(const WideStringView& wsLabel) {
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
  bool bSave = pLabelItems->JSObject()->GetBoolean(XFA_Attribute::Save);
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
    if (pChildItem->JSObject()->GetContent(false) == wsLabel) {
      iSearch = iCount;
      break;
    }
    iCount++;
  }
  if (iSearch < 0)
    return L"";

  CXFA_Node* pText =
      pSaveItems->GetChild<CXFA_Node>(iSearch, XFA_Element::Unknown, false);
  return pText ? pText->JSObject()->GetContent(false) : L"";
}

bool CXFA_WidgetAcc::DeleteItem(int32_t nIndex,
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
      if (!bSetValue && pItems->JSObject()->GetBoolean(XFA_Attribute::Save)) {
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

bool CXFA_WidgetAcc::IsHorizontalScrollPolicyOff() {
  CXFA_Node* pUIChild = GetUIChild();
  if (pUIChild) {
    return pUIChild->JSObject()->GetEnum(XFA_Attribute::HScrollPolicy) ==
           XFA_AttributeEnum::Off;
  }
  return false;
}

bool CXFA_WidgetAcc::IsVerticalScrollPolicyOff() {
  CXFA_Node* pUIChild = GetUIChild();
  if (pUIChild) {
    return pUIChild->JSObject()->GetEnum(XFA_Attribute::VScrollPolicy) ==
           XFA_AttributeEnum::Off;
  }
  return false;
}

Optional<int32_t> CXFA_WidgetAcc::GetNumberOfCells() {
  CXFA_Node* pUIChild = GetUIChild();
  if (!pUIChild)
    return {};
  if (CXFA_Comb* pNode =
          pUIChild->GetChild<CXFA_Comb>(0, XFA_Element::Comb, false))
    return {pNode->JSObject()->GetInteger(XFA_Attribute::NumberOfCells)};
  return {};
}

WideString CXFA_WidgetAcc::GetBarcodeType() {
  CXFA_Node* pUIChild = GetUIChild();
  return pUIChild
             ? WideString(pUIChild->JSObject()->GetCData(XFA_Attribute::Type))
             : WideString();
}

Optional<BC_CHAR_ENCODING> CXFA_WidgetAcc::GetBarcodeAttribute_CharEncoding() {
  Optional<WideString> wsCharEncoding =
      GetUIChild()->JSObject()->TryCData(XFA_Attribute::CharEncoding, true);
  if (!wsCharEncoding)
    return {};
  if (wsCharEncoding->CompareNoCase(L"UTF-16"))
    return {CHAR_ENCODING_UNICODE};
  if (wsCharEncoding->CompareNoCase(L"UTF-8"))
    return {CHAR_ENCODING_UTF8};
  return {};
}

Optional<bool> CXFA_WidgetAcc::GetBarcodeAttribute_Checksum() {
  Optional<XFA_AttributeEnum> checksum =
      GetUIChild()->JSObject()->TryEnum(XFA_Attribute::Checksum, true);
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

Optional<int32_t> CXFA_WidgetAcc::GetBarcodeAttribute_DataLength() {
  Optional<WideString> wsDataLength =
      GetUIChild()->JSObject()->TryCData(XFA_Attribute::DataLength, true);
  if (!wsDataLength)
    return {};

  return {FXSYS_wtoi(wsDataLength->c_str())};
}

Optional<char> CXFA_WidgetAcc::GetBarcodeAttribute_StartChar() {
  Optional<WideString> wsStartEndChar =
      GetUIChild()->JSObject()->TryCData(XFA_Attribute::StartChar, true);
  if (!wsStartEndChar || wsStartEndChar->IsEmpty())
    return {};

  return {static_cast<char>((*wsStartEndChar)[0])};
}

Optional<char> CXFA_WidgetAcc::GetBarcodeAttribute_EndChar() {
  Optional<WideString> wsStartEndChar =
      GetUIChild()->JSObject()->TryCData(XFA_Attribute::EndChar, true);
  if (!wsStartEndChar || wsStartEndChar->IsEmpty())
    return {};

  return {static_cast<char>((*wsStartEndChar)[0])};
}

Optional<int32_t> CXFA_WidgetAcc::GetBarcodeAttribute_ECLevel() {
  Optional<WideString> wsECLevel = GetUIChild()->JSObject()->TryCData(
      XFA_Attribute::ErrorCorrectionLevel, true);
  if (!wsECLevel)
    return {};
  return {FXSYS_wtoi(wsECLevel->c_str())};
}

Optional<int32_t> CXFA_WidgetAcc::GetBarcodeAttribute_ModuleWidth() {
  Optional<CXFA_Measurement> moduleWidthHeight =
      GetUIChild()->JSObject()->TryMeasure(XFA_Attribute::ModuleWidth, true);
  if (!moduleWidthHeight)
    return {};

  return {static_cast<int32_t>(moduleWidthHeight->ToUnit(XFA_Unit::Pt))};
}

Optional<int32_t> CXFA_WidgetAcc::GetBarcodeAttribute_ModuleHeight() {
  Optional<CXFA_Measurement> moduleWidthHeight =
      GetUIChild()->JSObject()->TryMeasure(XFA_Attribute::ModuleHeight, true);
  if (!moduleWidthHeight)
    return {};

  return {static_cast<int32_t>(moduleWidthHeight->ToUnit(XFA_Unit::Pt))};
}

Optional<bool> CXFA_WidgetAcc::GetBarcodeAttribute_PrintChecksum() {
  return GetUIChild()->JSObject()->TryBoolean(XFA_Attribute::PrintCheckDigit,
                                              true);
}

Optional<BC_TEXT_LOC> CXFA_WidgetAcc::GetBarcodeAttribute_TextLocation() {
  Optional<XFA_AttributeEnum> textLocation =
      GetUIChild()->JSObject()->TryEnum(XFA_Attribute::TextLocation, true);
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

Optional<bool> CXFA_WidgetAcc::GetBarcodeAttribute_Truncate() {
  return GetUIChild()->JSObject()->TryBoolean(XFA_Attribute::Truncate, true);
}

Optional<int8_t> CXFA_WidgetAcc::GetBarcodeAttribute_WideNarrowRatio() {
  Optional<WideString> wsWideNarrowRatio =
      GetUIChild()->JSObject()->TryCData(XFA_Attribute::WideNarrowRatio, true);
  if (!wsWideNarrowRatio)
    return {};

  Optional<size_t> ptPos = wsWideNarrowRatio->Find(':');
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

WideString CXFA_WidgetAcc::GetPasswordChar() {
  CXFA_Node* pUIChild = GetUIChild();
  return pUIChild ? pUIChild->JSObject()->GetCData(XFA_Attribute::PasswordChar)
                  : L"*";
}

bool CXFA_WidgetAcc::IsMultiLine() {
  CXFA_Node* pUIChild = GetUIChild();
  return pUIChild && pUIChild->JSObject()->GetBoolean(XFA_Attribute::MultiLine);
}

std::pair<XFA_Element, int32_t> CXFA_WidgetAcc::GetMaxChars() {
  if (CXFA_Value* pNode =
          m_pNode->GetChild<CXFA_Value>(0, XFA_Element::Value, false)) {
    if (CXFA_Node* pChild = pNode->GetNodeItem(XFA_NODEITEM_FirstChild)) {
      switch (pChild->GetElementType()) {
        case XFA_Element::Text:
          return {XFA_Element::Text,
                  pChild->JSObject()->GetInteger(XFA_Attribute::MaxChars)};
        case XFA_Element::ExData: {
          int32_t iMax =
              pChild->JSObject()->GetInteger(XFA_Attribute::MaxLength);
          return {XFA_Element::ExData, iMax < 0 ? 0 : iMax};
        }
        default:
          break;
      }
    }
  }
  return {XFA_Element::Unknown, 0};
}

int32_t CXFA_WidgetAcc::GetFracDigits() {
  CXFA_Value* pNode =
      m_pNode->GetChild<CXFA_Value>(0, XFA_Element::Value, false);
  if (!pNode)
    return -1;

  CXFA_Decimal* pChild =
      pNode->GetChild<CXFA_Decimal>(0, XFA_Element::Decimal, false);
  if (!pChild)
    return -1;

  return pChild->JSObject()
      ->TryInteger(XFA_Attribute::FracDigits, true)
      .value_or(-1);
}

int32_t CXFA_WidgetAcc::GetLeadDigits() {
  CXFA_Value* pNode =
      m_pNode->GetChild<CXFA_Value>(0, XFA_Element::Value, false);
  if (!pNode)
    return -1;

  CXFA_Decimal* pChild =
      pNode->GetChild<CXFA_Decimal>(0, XFA_Element::Decimal, false);
  if (!pChild)
    return -1;

  return pChild->JSObject()
      ->TryInteger(XFA_Attribute::LeadDigits, true)
      .value_or(-1);
}

bool CXFA_WidgetAcc::SetValue(XFA_VALUEPICTURE eValueType,
                              const WideString& wsValue) {
  if (wsValue.IsEmpty()) {
    if (m_pNode)
      m_pNode->SyncValue(wsValue, true);
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
  if (eType != XFA_Element::NumericEdit || bSyncData) {
    if (m_pNode)
      m_pNode->SyncValue(wsNewText, true);
  }

  return bValidate;
}

WideString CXFA_WidgetAcc::GetPictureContent(XFA_VALUEPICTURE ePicture) {
  if (ePicture == XFA_VALUEPICTURE_Raw)
    return L"";

  CXFA_LocaleValue widgetValue = XFA_GetLocaleValue(this);
  switch (ePicture) {
    case XFA_VALUEPICTURE_Display: {
      if (CXFA_Format* pFormat =
              m_pNode->GetChild<CXFA_Format>(0, XFA_Element::Format, false)) {
        if (CXFA_Picture* pPicture = pFormat->GetChild<CXFA_Picture>(
                0, XFA_Element::Picture, false)) {
          Optional<WideString> picture =
              pPicture->JSObject()->TryContent(false, true);
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
      CXFA_Ui* pUI = m_pNode->GetChild<CXFA_Ui>(0, XFA_Element::Ui, false);
      if (pUI) {
        if (CXFA_Picture* pPicture =
                pUI->GetChild<CXFA_Picture>(0, XFA_Element::Picture, false)) {
          Optional<WideString> picture =
              pPicture->JSObject()->TryContent(false, true);
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
      CXFA_Bind* bind = GetBind();
      if (bind)
        return bind->GetPicture();
      break;
    }
    default:
      break;
  }
  return L"";
}

IFX_Locale* CXFA_WidgetAcc::GetLocale() {
  return m_pNode ? m_pNode->GetLocale() : nullptr;
}

WideString CXFA_WidgetAcc::GetValue(XFA_VALUEPICTURE eValueType) {
  WideString wsValue = m_pNode->JSObject()->GetContent(false);

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

WideString CXFA_WidgetAcc::GetNormalizeDataValue(const WideString& wsValue) {
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

WideString CXFA_WidgetAcc::GetFormatDataValue(const WideString& wsValue) {
  if (wsValue.IsEmpty())
    return L"";

  WideString wsPicture = GetPictureContent(XFA_VALUEPICTURE_DataBind);
  if (wsPicture.IsEmpty())
    return wsValue;

  WideString wsFormattedValue = wsValue;
  if (IFX_Locale* pLocale = GetLocale()) {
    ASSERT(GetNode());
    CXFA_Value* pNodeValue =
        GetNode()->GetChild<CXFA_Value>(0, XFA_Element::Value, false);
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

WideString CXFA_WidgetAcc::NormalizeNumStr(const WideString& wsValue) {
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

WideString CXFA_WidgetAcc::FormatNumStr(const WideString& wsValue,
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

void CXFA_WidgetAcc::InsertListTextItem(CXFA_Node* pItems,
                                        const WideString& wsText,
                                        int32_t nIndex) {
  CXFA_Node* pText = pItems->CreateSamePacketNode(XFA_Element::Text);
  pItems->InsertChild(nIndex, pText);
  pText->JSObject()->SetContent(wsText, wsText, false, false, false);
}

WideString CXFA_WidgetAcc::NumericLimit(const WideString& wsValue,
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
