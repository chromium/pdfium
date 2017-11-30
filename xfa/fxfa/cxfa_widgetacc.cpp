// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/cxfa_widgetacc.h"

#include <algorithm>
#include <vector>

#include "core/fxcrt/xml/cfx_xmlelement.h"
#include "core/fxcrt/xml/cfx_xmlnode.h"
#include "fxjs/cfxjse_engine.h"
#include "third_party/base/stl_util.h"
#include "xfa/fde/cfde_textout.h"
#include "xfa/fxfa/cxfa_ffapp.h"
#include "xfa/fxfa/cxfa_ffdoc.h"
#include "xfa/fxfa/cxfa_ffdocview.h"
#include "xfa/fxfa/cxfa_ffwidget.h"
#include "xfa/fxfa/cxfa_fontmgr.h"
#include "xfa/fxfa/cxfa_textlayout.h"
#include "xfa/fxfa/cxfa_textprovider.h"
#include "xfa/fxfa/parser/cxfa_layoutprocessor.h"
#include "xfa/fxfa/parser/cxfa_localevalue.h"
#include "xfa/fxfa/parser/cxfa_node.h"
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

    CXFA_ValueData valueData = pAcc->GetFormValueData();
    if (!valueData.HasValidNode())
      return false;

    CXFA_ImageData imageData = valueData.GetImageData();
    if (!imageData.HasValidNode())
      return false;

    CXFA_FFDoc* pFFDoc = pAcc->GetDoc();
    pAcc->SetImageImage(XFA_LoadImageData(pFFDoc, &imageData, m_bNamedImage,
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
    CXFA_CaptionData captionData = pAcc->GetCaptionData();
    if (!captionData.HasValidNode() || captionData.IsHidden())
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

    CXFA_ValueData valueData = pAcc->GetFormValueData();
    if (!valueData.HasValidNode())
      return false;

    CXFA_ImageData imageData = valueData.GetImageData();
    CXFA_FFDoc* pFFDoc = pAcc->GetDoc();
    pAcc->SetImageEditImage(XFA_LoadImageData(pFFDoc, &imageData, m_bNamedImage,
                                              m_iImageXDpi, m_iImageYDpi));
    return !!m_pDIBitmap;
  }

  RetainPtr<CFX_DIBitmap> m_pDIBitmap;
  bool m_bNamedImage;
  int32_t m_iImageXDpi;
  int32_t m_iImageYDpi;
};

}  // namespace

CXFA_WidgetAcc::CXFA_WidgetAcc(CXFA_FFDocView* pDocView, CXFA_Node* pNode)
    : CXFA_WidgetData(pNode), m_pDocView(pDocView), m_nRecursionDepth(0) {}

CXFA_WidgetAcc::~CXFA_WidgetAcc() {}

CXFA_Node* CXFA_WidgetAcc::GetDatasets() {
  return m_pNode->GetBindData();
}

bool CXFA_WidgetAcc::ProcessValueChanged() {
  m_pDocView->AddValidateWidget(this);
  m_pDocView->AddCalculateWidgetAcc(this);
  m_pDocView->RunCalculateWidgets();
  m_pDocView->RunValidate();
  return true;
}

void CXFA_WidgetAcc::ResetData() {
  WideString wsValue;
  XFA_Element eUIType = GetUIType();
  switch (eUIType) {
    case XFA_Element::ImageEdit: {
      CXFA_ValueData imageValueData = GetDefaultValueData();
      CXFA_ImageData imageData = imageValueData.GetImageData();
      WideString wsContentType, wsHref;
      if (imageData.HasValidNode()) {
        wsValue = imageData.GetContent();
        wsContentType = imageData.GetContentType();
        wsHref = imageData.GetHref();
      }
      SetImageEdit(wsContentType, wsHref, wsValue);
      break;
    }
    case XFA_Element::ExclGroup: {
      CXFA_Node* pNextChild = m_pNode->GetNodeItem(
          XFA_NODEITEM_FirstChild, XFA_ObjectType::ContainerNode);
      while (pNextChild) {
        CXFA_Node* pChild = pNextChild;
        CXFA_WidgetAcc* pAcc =
            static_cast<CXFA_WidgetAcc*>(pChild->GetWidgetData());
        if (!pAcc)
          continue;

        bool done = false;
        if (wsValue.IsEmpty()) {
          CXFA_ValueData defValueData = pAcc->GetDefaultValueData();
          if (defValueData.HasValidNode()) {
            wsValue = defValueData.GetChildValueContent();
            SetValue(XFA_VALUEPICTURE_Raw, wsValue);
            pAcc->SetValue(XFA_VALUEPICTURE_Raw, wsValue);
            done = true;
          }
        }
        if (!done) {
          CXFA_Node* pItems = pChild->GetChild(0, XFA_Element::Items, false);
          if (!pItems)
            continue;

          WideString itemText;
          if (pItems->CountChildren(XFA_Element::Unknown, false) > 1) {
            itemText = pItems->GetChild(1, XFA_Element::Unknown, false)
                           ->JSNode()
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
      CXFA_ValueData defValueData = GetDefaultValueData();
      if (defValueData.HasValidNode())
        wsValue = defValueData.GetChildValueContent();

      SetValue(XFA_VALUEPICTURE_Raw, wsValue);
      break;
    }
  }
}

void CXFA_WidgetAcc::SetImageEdit(const WideString& wsContentType,
                                  const WideString& wsHref,
                                  const WideString& wsData) {
  CXFA_ImageData imageData = GetFormValueData().GetImageData();
  if (imageData.HasValidNode()) {
    imageData.SetContentType(WideString(wsContentType));
    imageData.SetHref(wsHref);
  }

  m_pNode->JSNode()->SetContent(wsData, GetFormatDataValue(wsData), true, false,
                                true);

  CXFA_Node* pBind = GetDatasets();
  if (!pBind) {
    imageData.SetTransferEncoding(XFA_AttributeEnum::Base64);
    return;
  }
  pBind->JSNode()->SetCData(XFA_Attribute::ContentType, wsContentType, false,
                            false);
  CXFA_Node* pHrefNode = pBind->GetNodeItem(XFA_NODEITEM_FirstChild);
  if (pHrefNode) {
    pHrefNode->JSNode()->SetCData(XFA_Attribute::Value, wsHref, false, false);
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
  return static_cast<CXFA_WidgetAcc*>(pExcl->GetWidgetData());
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

  std::vector<CXFA_Node*> eventArray =
      GetEventByActivity(iActivity, pEventParam->m_bIsFormReady);
  bool first = true;
  int32_t iRet = XFA_EVENTERROR_NotExist;
  for (CXFA_Node* pNode : eventArray) {
    int32_t result = ProcessEvent(CXFA_EventData(pNode), pEventParam);
    if (first || result == XFA_EVENTERROR_Success)
      iRet = result;
    first = false;
  }
  return iRet;
}

int32_t CXFA_WidgetAcc::ProcessEvent(const CXFA_EventData& eventData,
                                     CXFA_EventParam* pEventParam) {
  if (!eventData.HasValidNode())
    return XFA_EVENTERROR_NotExist;

  switch (eventData.GetEventType()) {
    case XFA_Element::Execute:
      break;
    case XFA_Element::Script:
      return ExecuteScript(eventData.GetScriptData(), pEventParam);
    case XFA_Element::SignData:
      break;
    case XFA_Element::Submit:
      return GetDoc()->GetDocEnvironment()->SubmitData(
          GetDoc(), eventData.GetSubmitData());
    default:
      break;
  }
  return XFA_EVENTERROR_NotExist;
}

int32_t CXFA_WidgetAcc::ProcessCalculate() {
  if (GetElementType() == XFA_Element::Draw)
    return XFA_EVENTERROR_NotExist;

  CXFA_CalculateData calcData = GetCalculateData();
  if (!calcData.HasValidNode())
    return XFA_EVENTERROR_NotExist;
  if (GetNode()->IsUserInteractive())
    return XFA_EVENTERROR_Disabled;

  CXFA_EventParam EventParam;
  EventParam.m_eType = XFA_EVENT_Calculate;
  int32_t iRet = ExecuteScript(calcData.GetScriptData(), &EventParam);
  if (iRet != XFA_EVENTERROR_Success)
    return iRet;

  if (GetRawValue() != EventParam.m_wsResult) {
    SetValue(XFA_VALUEPICTURE_Raw, EventParam.m_wsResult);
    UpdateUIDisplay();
  }
  return XFA_EVENTERROR_Success;
}

void CXFA_WidgetAcc::ProcessScriptTestValidate(CXFA_ValidateData validateData,
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
  WideString wsScriptMsg = validateData.GetScriptMessageText();
  if (validateData.GetScriptTest() == XFA_AttributeEnum::Warning) {
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

int32_t CXFA_WidgetAcc::ProcessFormatTestValidate(
    CXFA_ValidateData validateData,
    bool bVersionFlag) {
  WideString wsRawValue = GetRawValue();
  if (!wsRawValue.IsEmpty()) {
    WideString wsPicture = validateData.GetPicture();
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

      WideString wsFormatMsg = validateData.GetFormatMessageText();
      WideString wsTitle = pAppProvider->GetAppTitle();
      if (validateData.GetFormatTest() == XFA_AttributeEnum::Error) {
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

int32_t CXFA_WidgetAcc::ProcessNullTestValidate(CXFA_ValidateData validateData,
                                                int32_t iFlags,
                                                bool bVersionFlag) {
  if (!GetValue(XFA_VALUEPICTURE_Raw).IsEmpty())
    return XFA_EVENTERROR_Success;
  if (IsNull() && IsPreNull())
    return XFA_EVENTERROR_Success;

  XFA_AttributeEnum eNullTest = validateData.GetNullTest();
  WideString wsNullMsg = validateData.GetNullMessageText();
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
            WideString::Format(L"%s cannot be blank.", wsCaptionName.c_str());
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
            L"%s cannot be blank. To ignore validations for %s, click Ignore.",
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
    CXFA_CaptionData captionData = GetCaptionData();
    if (captionData.HasValidNode()) {
      CXFA_ValueData capValue = captionData.GetValueData();
      if (capValue.HasValidNode()) {
        CXFA_TextData captionTextData = capValue.GetTextData();
        if (captionTextData.HasValidNode())
          wsCaptionName = captionTextData.GetContent();
      }
    }
  }
  if (!wsCaptionName.IsEmpty())
    return wsCaptionName;
  return m_pNode->JSNode()->GetCData(XFA_Attribute::Name);
}

WideString CXFA_WidgetAcc::GetValidateMessage(bool bError, bool bVersionFlag) {
  WideString wsCaptionName = GetValidateCaptionName(bVersionFlag);
  if (bVersionFlag)
    return WideString::Format(L"%s validation failed", wsCaptionName.c_str());
  if (bError) {
    return WideString::Format(L"The value you entered for %s is invalid.",
                              wsCaptionName.c_str());
  }
  return WideString::Format(
      L"The value you entered for %s is invalid. To ignore "
      L"validations for %s, click Ignore.",
      wsCaptionName.c_str(), wsCaptionName.c_str());
}

int32_t CXFA_WidgetAcc::ProcessValidate(int32_t iFlags) {
  if (GetElementType() == XFA_Element::Draw)
    return XFA_EVENTERROR_NotExist;

  CXFA_ValidateData validateData = GetValidateData(false);
  if (!validateData.HasValidNode())
    return XFA_EVENTERROR_NotExist;

  bool bInitDoc = validateData.GetNode()->NeedsInitApp();
  bool bStatus = m_pDocView->GetLayoutStatus() < XFA_DOCVIEW_LAYOUTSTATUS_End;
  int32_t iFormat = 0;
  int32_t iRet = XFA_EVENTERROR_NotExist;
  CXFA_ScriptData scriptData = validateData.GetScriptData();
  bool bRet = false;
  bool hasBoolResult = (bInitDoc || bStatus) && GetRawValue().IsEmpty();
  if (scriptData.HasValidNode()) {
    CXFA_EventParam eParam;
    eParam.m_eType = XFA_EVENT_Validate;
    eParam.m_pTarget = this;
    std::tie(iRet, bRet) = ExecuteBoolScript(scriptData, &eParam);
  }

  XFA_VERSION version = GetDoc()->GetXFADoc()->GetCurVersionMode();
  bool bVersionFlag = false;
  if (version < XFA_VERSION_208)
    bVersionFlag = true;

  if (bInitDoc) {
    validateData.GetNode()->ClearFlag(XFA_NodeFlag_NeedsInitApp);
  } else {
    iFormat = ProcessFormatTestValidate(validateData, bVersionFlag);
    if (!bVersionFlag)
      bVersionFlag = GetDoc()->GetXFADoc()->HasFlag(XFA_DOCFLAG_Scripting);

    iRet |= ProcessNullTestValidate(validateData, iFlags, bVersionFlag);
  }

  if (iFormat != XFA_EVENTERROR_Success && hasBoolResult)
    ProcessScriptTestValidate(validateData, iRet, bRet, bVersionFlag);

  return iRet | iFormat;
}

int32_t CXFA_WidgetAcc::ExecuteScript(const CXFA_ScriptData& scriptData,
                                      CXFA_EventParam* pEventParam) {
  bool bRet;
  int32_t iRet;
  std::tie(iRet, bRet) = ExecuteBoolScript(scriptData, pEventParam);
  return iRet;
}

std::pair<int32_t, bool> CXFA_WidgetAcc::ExecuteBoolScript(
    CXFA_ScriptData scriptData,
    CXFA_EventParam* pEventParam) {
  static const uint32_t MAX_RECURSION_DEPTH = 2;
  if (m_nRecursionDepth > MAX_RECURSION_DEPTH)
    return {XFA_EVENTERROR_Success, false};

  ASSERT(pEventParam);
  if (!scriptData.HasValidNode())
    return {XFA_EVENTERROR_NotExist, false};
  if (scriptData.GetRunAt() == XFA_AttributeEnum::Server)
    return {XFA_EVENTERROR_Disabled, false};

  WideString wsExpression = scriptData.GetExpression();
  if (wsExpression.IsEmpty())
    return {XFA_EVENTERROR_NotExist, false};

  CXFA_ScriptData::Type eScriptType = scriptData.GetContentType();
  if (eScriptType == CXFA_ScriptData::Type::Unknown)
    return {XFA_EVENTERROR_Success, false};

  CXFA_FFDoc* pDoc = GetDoc();
  CFXJSE_Engine* pContext = pDoc->GetXFADoc()->GetScriptContext();
  pContext->SetEventParam(*pEventParam);
  pContext->SetRunAtType(scriptData.GetRunAt());

  std::vector<CXFA_Node*> refNodes;
  if (pEventParam->m_eType == XFA_EVENT_InitCalculate ||
      pEventParam->m_eType == XFA_EVENT_Calculate) {
    pContext->SetNodesOfRunScript(&refNodes);
  }

  auto pTmpRetValue = pdfium::MakeUnique<CFXJSE_Value>(pContext->GetRuntime());
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
        if (static_cast<CXFA_WidgetAcc*>(pRefNode->GetWidgetData()) == this)
          continue;

        CXFA_CalcData* pGlobalData = pRefNode->JSNode()->GetCalcData();
        if (!pGlobalData) {
          pRefNode->JSNode()->SetCalcData(pdfium::MakeUnique<CXFA_CalcData>());
          pGlobalData = pRefNode->JSNode()->GetCalcData();
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
  CXFA_LayoutItem* pLayout = nullptr;
  if (pWidget)
    pLayout = pWidget->GetNext();
  else
    pLayout = m_pDocView->GetXFALayout()->GetLayoutItem(m_pNode);

  return static_cast<CXFA_FFWidget*>(pLayout);
}

void CXFA_WidgetAcc::UpdateUIDisplay(CXFA_FFWidget* pExcept) {
  CXFA_FFWidget* pWidget = nullptr;
  while ((pWidget = GetNextWidget(pWidget)) != nullptr) {
    if (pWidget == pExcept || !pWidget->IsLoaded() ||
        (GetUIType() != XFA_Element::CheckButton && pWidget->IsFocused())) {
      continue;
    }
    pWidget->UpdateFWLData();
    pWidget->AddInvalidateRect();
  }
}

void CXFA_WidgetAcc::CalcCaptionSize(CFX_SizeF& szCap) {
  CXFA_CaptionData captionData = GetCaptionData();
  if (!captionData.HasValidNode() || !captionData.IsVisible())
    return;

  LoadCaption();
  XFA_Element eUIType = GetUIType();
  XFA_AttributeEnum iCapPlacement = captionData.GetPlacementType();
  float fCapReserve = captionData.GetReserve();
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
    CXFA_FontData fontData = captionData.GetFontData();
    if (fontData.HasValidNode()) {
      fFontSize = fontData.GetFontSize();
    } else {
      CXFA_FontData widgetfontData = GetFontData(false);
      if (widgetfontData.HasValidNode())
        fFontSize = widgetfontData.GetFontSize();
    }

    if (bVert) {
      szCap.height = fCapReserve > 0 ? fCapReserve : fFontSize;
    } else {
      szCap.width = fCapReserve > 0 ? fCapReserve : 0;
      szCap.height = fFontSize;
    }
  }

  CXFA_MarginData captionMarginData = captionData.GetMarginData();
  if (captionMarginData.HasValidNode()) {
    float fLeftInset = captionMarginData.GetLeftInset();
    float fTopInset = captionMarginData.GetTopInset();
    float fRightInset = captionMarginData.GetRightInset();
    float fBottomInset = captionMarginData.GetBottomInset();
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
    switch (GetCaptionData().GetPlacementType()) {
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
  CXFA_MarginData marginData = GetMarginData();
  if (marginData.HasValidNode()) {
    size.width += marginData.GetLeftInset() + marginData.GetRightInset();
    size.height += marginData.GetTopInset() + marginData.GetBottomInset();
  }

  CXFA_ParaData paraData = GetParaData();
  if (paraData.HasValidNode())
    size.width += paraData.GetMarginLeft() + paraData.GetTextIndent();

  pdfium::Optional<float> width = TryWidth();
  if (width) {
    size.width = *width;
  } else {
    pdfium::Optional<float> min = TryMinWidth();
    if (min)
      size.width = std::max(size.width, *min);

    pdfium::Optional<float> max = TryMaxWidth();
    if (max && *max > 0)
      size.width = std::min(size.width, *max);
  }

  pdfium::Optional<float> height = TryHeight();
  if (height) {
    size.height = *height;
  } else {
    pdfium::Optional<float> min = TryMinHeight();
    if (min)
      size.height = std::max(size.height, *min);

    pdfium::Optional<float> max = TryMaxHeight();
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
      iCapPlacement = GetCaptionData().GetPlacementType();
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
    CXFA_MarginData marginData = GetMarginData();
    if (marginData.HasValidNode())
      size.width -= marginData.GetLeftInset() + marginData.GetRightInset();

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
  pdfium::Optional<float> width = TryWidth();
  if (width) {
    rtFit.width = *width;
    GetWidthWithoutMargin(rtFit.width);
  } else {
    rtFit.width = rtImage.width;
  }

  pdfium::Optional<float> height = TryHeight();
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
  CXFA_MarginData marginData = GetMarginData();
  if (marginData.HasValidNode())
    fWidthCalc += marginData.GetLeftInset() + marginData.GetRightInset();

  pdfium::Optional<float> min = TryMinWidth();
  if (min)
    fWidthCalc = std::max(fWidthCalc, *min);

  pdfium::Optional<float> max = TryMaxWidth();
  if (max && *max > 0)
    fWidthCalc = std::min(fWidthCalc, *max);

  return fWidthCalc;
}

float CXFA_WidgetAcc::GetWidthWithoutMargin(float fWidthCalc) {
  CXFA_MarginData marginData = GetMarginData();
  if (marginData.HasValidNode())
    fWidthCalc -= marginData.GetLeftInset() + marginData.GetRightInset();
  return fWidthCalc;
}

float CXFA_WidgetAcc::CalculateWidgetAutoHeight(float fHeightCalc) {
  CXFA_MarginData marginData = GetMarginData();
  if (marginData.HasValidNode())
    fHeightCalc += marginData.GetTopInset() + marginData.GetBottomInset();

  pdfium::Optional<float> min = TryMinHeight();
  if (min)
    fHeightCalc = std::max(fHeightCalc, *min);

  pdfium::Optional<float> max = TryMaxHeight();
  if (max && *max > 0)
    fHeightCalc = std::min(fHeightCalc, *max);

  return fHeightCalc;
}

float CXFA_WidgetAcc::GetHeightWithoutMargin(float fHeightCalc) {
  CXFA_MarginData marginData = GetMarginData();
  if (marginData.HasValidNode())
    fHeightCalc -= marginData.GetTopInset() + marginData.GetBottomInset();
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
    pdfium::Optional<float> height = TryHeight();
    if (height)
      fCalcHeight = *height;
    else
      CalculateAccWidthAndHeight(eUIType, fCalcWidth, fCalcHeight);

    m_pLayoutData->m_fWidgetHeight = fCalcHeight;
    return;
  }
  if (fCalcWidth < 0 && fCalcHeight < 0) {
    pdfium::Optional<float> height;
    pdfium::Optional<float> width = TryWidth();
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
    CXFA_MarginData marginData = GetMarginData();
    if (marginData.HasValidNode()) {
      fTopInset = marginData.GetTopInset();
      fBottomInset = marginData.GetBottomInset();
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
    CXFA_CaptionData captionData = GetCaptionData();
    if (captionData.HasValidNode() && !captionData.IsHidden()) {
      iCapPlacement = captionData.GetPlacementType();
      fCapReserve = captionData.GetReserve();
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
    CXFA_ParaData paraData = GetParaData();
    if (paraData.HasValidNode()) {
      fSpaceAbove = paraData.GetSpaceAbove();
      float fSpaceBelow = paraData.GetSpaceBelow();
      fHeight -= (fSpaceAbove + fSpaceBelow);
      switch (paraData.GetVerticalAlign()) {
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
                                      ->JSNode()
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
    pdfium::Optional<float> width = TryWidth();
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
  CXFA_FontData fontData = GetFontData(false);
  if (fontData.HasValidNode()) {
    if (fontData.IsBold())
      dwFontStyle |= FXFONT_BOLD;
    if (fontData.IsItalic())
      dwFontStyle |= FXFONT_ITALIC;

    wsFontName = fontData.GetTypeface();
  }

  auto* pDoc = GetDoc();
  return pDoc->GetApp()->GetXFAFontMgr()->GetFont(
      pDoc, wsFontName.AsStringView(), dwFontStyle);
}

float CXFA_WidgetAcc::GetFontSize() {
  CXFA_FontData fontData = GetFontData(false);
  float fFontSize = fontData.HasValidNode() ? fontData.GetFontSize() : 10.0f;
  return fFontSize < 0.1f ? 10.0f : fFontSize;
}

float CXFA_WidgetAcc::GetLineHeight() {
  float fLineHeight = 0;
  CXFA_ParaData paraData = GetParaData();
  if (paraData.HasValidNode())
    fLineHeight = paraData.GetLineHeight();
  if (fLineHeight < 1)
    fLineHeight = GetFontSize() * 1.2f;
  return fLineHeight;
}

FX_ARGB CXFA_WidgetAcc::GetTextColor() {
  CXFA_FontData fontData = GetFontData(false);
  return fontData.HasValidNode() ? fontData.GetColor() : 0xFF000000;
}
