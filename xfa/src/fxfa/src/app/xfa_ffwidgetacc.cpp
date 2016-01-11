// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <algorithm>

#include "xfa/src/foxitlib.h"
#include "xfa/src/fxfa/src/common/xfa_common.h"
#include "xfa_ffwidget.h"
#include "xfa_ffdoc.h"
#include "xfa_ffdocview.h"
#include "xfa_ffpageview.h"
#include "xfa_ffapp.h"
#include "xfa_textlayout.h"
#include "xfa_fwladapter.h"
#include "xfa_fffield.h"
#include "xfa_ffchoicelist.h"
#include "xfa_ffcheckbutton.h"
#include "xfa_ffwidgetacc.h"
#include "xfa_fontmgr.h"
static void XFA_FFDeleteCalcData(void* pData) {
  if (pData) {
    delete ((CXFA_CalcData*)pData);
  }
}
static XFA_MAPDATABLOCKCALLBACKINFO gs_XFADeleteCalcData = {
    XFA_FFDeleteCalcData, NULL};
class CXFA_WidgetLayoutData {
 public:
  CXFA_WidgetLayoutData() : m_fWidgetHeight(-1) {}
  virtual ~CXFA_WidgetLayoutData() {}
  virtual void Release() { delete this; }
  FX_FLOAT m_fWidgetHeight;
};
class CXFA_TextLayoutData : public CXFA_WidgetLayoutData {
 public:
  CXFA_TextLayoutData() : m_pTextLayout(NULL), m_pTextProvider(NULL) {}
  ~CXFA_TextLayoutData() {
    if (m_pTextLayout) {
      delete m_pTextLayout;
    }
    m_pTextLayout = NULL;
    if (m_pTextProvider) {
      delete m_pTextProvider;
    }
    m_pTextProvider = NULL;
  }
  void LoadText(CXFA_WidgetAcc* pAcc) {
    if (m_pTextLayout)
      return;

    m_pTextProvider = new CXFA_TextProvider(pAcc, XFA_TEXTPROVIDERTYPE_Text);
    m_pTextLayout = new CXFA_TextLayout(m_pTextProvider);
  }
  CXFA_TextLayout* m_pTextLayout;
  CXFA_TextProvider* m_pTextProvider;
};
class CXFA_ImageLayoutData : public CXFA_WidgetLayoutData {
 public:
  CXFA_ImageLayoutData()
      : m_pDIBitmap(NULL),
        m_bNamedImage(FALSE),
        m_iImageXDpi(0),
        m_iImageYDpi(0) {}

  ~CXFA_ImageLayoutData() {
    if (m_pDIBitmap && !m_bNamedImage) {
      delete m_pDIBitmap;
    }
    m_pDIBitmap = NULL;
  }

  FX_BOOL LoadImageData(CXFA_WidgetAcc* pAcc) {
    if (m_pDIBitmap) {
      return TRUE;
    }
    CXFA_Value value = pAcc->GetFormValue();
    if (!value) {
      return FALSE;
    }
    CXFA_Image imageObj = value.GetImage();
    if (!imageObj) {
      return FALSE;
    }
    CXFA_FFDoc* pFFDoc = pAcc->GetDoc();
    pAcc->SetImageImage(XFA_LoadImageData(pFFDoc, &imageObj, m_bNamedImage,
                                          m_iImageXDpi, m_iImageYDpi));
    return m_pDIBitmap != NULL;
  }

  CFX_DIBitmap* m_pDIBitmap;
  FX_BOOL m_bNamedImage;
  int32_t m_iImageXDpi;
  int32_t m_iImageYDpi;
};
class CXFA_FieldLayoutData : public CXFA_WidgetLayoutData {
 public:
  CXFA_FieldLayoutData()
      : m_pCapTextLayout(NULL),
        m_pCapTextProvider(NULL),
        m_pTextOut(NULL),
        m_pFieldSplitArray(NULL) {}
  ~CXFA_FieldLayoutData() {
    if (m_pCapTextLayout) {
      delete m_pCapTextLayout;
    }
    m_pCapTextLayout = NULL;
    if (m_pCapTextProvider) {
      delete m_pCapTextProvider;
    }
    m_pCapTextProvider = NULL;
    if (m_pTextOut) {
      m_pTextOut->Release();
    }
    m_pTextOut = NULL;
    if (m_pFieldSplitArray) {
      m_pFieldSplitArray->RemoveAll();
      delete m_pFieldSplitArray;
      m_pFieldSplitArray = NULL;
    }
  }
  FX_BOOL LoadCaption(CXFA_WidgetAcc* pAcc) {
    if (m_pCapTextLayout) {
      return TRUE;
    }
    CXFA_Caption caption = pAcc->GetCaption();
    if (caption.IsExistInXML() &&
        caption.GetPresence() != XFA_ATTRIBUTEENUM_Hidden) {
      m_pCapTextProvider =
          new CXFA_TextProvider(pAcc, XFA_TEXTPROVIDERTYPE_Caption);
      m_pCapTextLayout = new CXFA_TextLayout(m_pCapTextProvider);
      return TRUE;
    }
    return FALSE;
  }
  CXFA_TextLayout* m_pCapTextLayout;
  CXFA_TextProvider* m_pCapTextProvider;
  IFDE_TextOut* m_pTextOut;
  CFX_FloatArray* m_pFieldSplitArray;
};
class CXFA_TextEditData : public CXFA_FieldLayoutData {
 public:
};
class CXFA_ImageEditData : public CXFA_FieldLayoutData {
 public:
  CXFA_ImageEditData()
      : m_pDIBitmap(NULL),
        m_bNamedImage(FALSE),
        m_iImageXDpi(0),
        m_iImageYDpi(0) {}

  ~CXFA_ImageEditData() {
    if (m_pDIBitmap && !m_bNamedImage) {
      delete m_pDIBitmap;
    }
    m_pDIBitmap = NULL;
  }
  FX_BOOL LoadImageData(CXFA_WidgetAcc* pAcc) {
    if (m_pDIBitmap) {
      return TRUE;
    }
    CXFA_Value value = pAcc->GetFormValue();
    if (!value) {
      return FALSE;
    }
    CXFA_Image imageObj = value.GetImage();
    CXFA_FFDoc* pFFDoc = pAcc->GetDoc();
    pAcc->SetImageEditImage(XFA_LoadImageData(pFFDoc, &imageObj, m_bNamedImage,
                                              m_iImageXDpi, m_iImageYDpi));
    return m_pDIBitmap != NULL;
  }
  CFX_DIBitmap* m_pDIBitmap;
  FX_BOOL m_bNamedImage;
  int32_t m_iImageXDpi;
  int32_t m_iImageYDpi;
};
CXFA_WidgetAcc::CXFA_WidgetAcc(CXFA_FFDocView* pDocView, CXFA_Node* pNode)
    : CXFA_WidgetData(pNode),
      m_pDocView(pDocView),
      m_pLayoutData(NULL),
      m_nRecursionDepth(0) {}
CXFA_WidgetAcc::~CXFA_WidgetAcc() {
  if (m_pLayoutData) {
    m_pLayoutData->Release();
    m_pLayoutData = NULL;
  }
}
FX_BOOL CXFA_WidgetAcc::GetName(CFX_WideString& wsName, int32_t iNameType) {
  if (iNameType == 0) {
    m_pNode->TryCData(XFA_ATTRIBUTE_Name, wsName);
    return !wsName.IsEmpty();
  }
  m_pNode->GetSOMExpression(wsName);
  if (iNameType == 2 && wsName.GetLength() >= 15) {
    CFX_WideStringC wsPre = FX_WSTRC(L"xfa[0].form[0].");
    if (wsPre == CFX_WideStringC(wsName, wsPre.GetLength())) {
      wsName.Delete(0, wsPre.GetLength());
    }
  }
  return TRUE;
}
CXFA_Node* CXFA_WidgetAcc::GetDatasets() {
  return m_pNode->GetBindData();
}
FX_BOOL CXFA_WidgetAcc::ProcessValueChanged() {
  m_pDocView->AddValidateWidget(this);
  m_pDocView->AddCalculateWidgetAcc(this);
  m_pDocView->RunCalculateWidgets();
  m_pDocView->RunValidate();
  return TRUE;
}
void CXFA_WidgetAcc::ResetData() {
  CFX_WideString wsValue;
  XFA_ELEMENT eUIType = (XFA_ELEMENT)GetUIType();
  switch (eUIType) {
    case XFA_ELEMENT_ImageEdit: {
      CXFA_Value imageValue = GetDefaultValue();
      CXFA_Image image = imageValue.GetImage();
      CFX_WideString wsContentType, wsHref;
      if (image) {
        image.GetContent(wsValue);
        image.GetContentType(wsContentType);
        image.GetHref(wsHref);
      }
      SetImageEdit(wsContentType, wsHref, wsValue);
    } break;
    case XFA_ELEMENT_ExclGroup: {
      CXFA_Node* pNextChild = m_pNode->GetNodeItem(
          XFA_NODEITEM_FirstChild, XFA_OBJECTTYPE_ContainerNode);
      while (pNextChild) {
        CXFA_Node* pChild = pNextChild;
        CXFA_WidgetAcc* pAcc = (CXFA_WidgetAcc*)pChild->GetWidgetData();
        if (!pAcc) {
          continue;
        }
        CXFA_Value defValue(NULL);
        if (wsValue.IsEmpty() && (defValue = pAcc->GetDefaultValue())) {
          defValue.GetChildValueContent(wsValue);
          this->SetValue(wsValue, XFA_VALUEPICTURE_Raw);
          pAcc->SetValue(wsValue, XFA_VALUEPICTURE_Raw);
        } else {
          CXFA_Node* pItems = pChild->GetChild(0, XFA_ELEMENT_Items);
          if (!pItems) {
            continue;
          }
          CFX_WideString itemText;
          if (pItems->CountChildren(XFA_ELEMENT_UNKNOWN) > 1) {
            itemText = pItems->GetChild(1, XFA_ELEMENT_UNKNOWN)->GetContent();
          }
          pAcc->SetValue(itemText, XFA_VALUEPICTURE_Raw);
        }
        pNextChild = pChild->GetNodeItem(XFA_NODEITEM_NextSibling,
                                         XFA_OBJECTTYPE_ContainerNode);
      }
    } break;
    case XFA_ELEMENT_ChoiceList:
      ClearAllSelections();
    default:
      if (CXFA_Value defValue = GetDefaultValue()) {
        defValue.GetChildValueContent(wsValue);
      }
      SetValue(wsValue, XFA_VALUEPICTURE_Raw);
      break;
  }
}
void CXFA_WidgetAcc::SetImageEdit(const CFX_WideStringC& wsContentType,
                                  const CFX_WideStringC& wsHref,
                                  const CFX_WideStringC& wsData) {
  CXFA_Image image = GetFormValue().GetImage();
  if (image) {
    image.SetContentType(wsContentType);
    image.SetHref(wsHref);
  }
  CFX_WideString wsFormatValue(wsData);
  this->GetFormatDataValue(wsData, wsFormatValue);
  m_pNode->SetContent(wsData, wsFormatValue, TRUE);
  CXFA_Node* pBind = GetDatasets();
  if (!pBind) {
    image.SetTransferEncoding(XFA_ATTRIBUTEENUM_Base64);
    return;
  }
  pBind->SetCData(XFA_ATTRIBUTE_ContentType, wsContentType);
  CXFA_Node* pHrefNode = pBind->GetNodeItem(XFA_NODEITEM_FirstChild);
  if (pHrefNode) {
    pHrefNode->SetCData(XFA_ATTRIBUTE_Value, wsHref);
  } else {
    IFDE_XMLNode* pXMLNode = pBind->GetXMLMappingNode();
    FXSYS_assert(pXMLNode && pXMLNode->GetType() == FDE_XMLNODE_Element);
    ((IFDE_XMLElement*)pXMLNode)->SetString(FX_WSTRC(L"href"), wsHref);
  }
}

CXFA_WidgetAcc* CXFA_WidgetAcc::GetExclGroup() {
  CXFA_Node* pExcl = m_pNode->GetNodeItem(XFA_NODEITEM_Parent);
  if (!pExcl || pExcl->GetClassID() != XFA_ELEMENT_ExclGroup) {
    return NULL;
  }
  return (CXFA_WidgetAcc*)pExcl->GetWidgetData();
}
CXFA_FFDocView* CXFA_WidgetAcc::GetDocView() {
  return m_pDocView;
}
CXFA_FFDoc* CXFA_WidgetAcc::GetDoc() {
  return (CXFA_FFDoc*)m_pDocView->GetDoc();
}
CXFA_FFApp* CXFA_WidgetAcc::GetApp() {
  return GetDoc()->GetApp();
}
IXFA_AppProvider* CXFA_WidgetAcc::GetAppProvider() {
  return GetApp()->GetAppProvider();
}
int32_t CXFA_WidgetAcc::ProcessEvent(int32_t iActivity,
                                     CXFA_EventParam* pEventParam) {
  if (this->GetClassID() == XFA_ELEMENT_Draw) {
    return XFA_EVENTERROR_NotExist;
  }
  int32_t iRet = XFA_EVENTERROR_NotExist;
  CXFA_NodeArray eventArray;
  int32_t iCounts =
      GetEventByActivity(iActivity, eventArray, pEventParam->m_bIsFormReady);
  for (int32_t i = 0; i < iCounts; i++) {
    CXFA_Event event(eventArray[i]);
    int32_t result = ProcessEvent(event, pEventParam);
    if (i == 0) {
      iRet = result;
    } else if (result == XFA_EVENTERROR_Sucess) {
      iRet = result;
    }
  }
  return iRet;
}
int32_t CXFA_WidgetAcc::ProcessEvent(CXFA_Event& event,
                                     CXFA_EventParam* pEventParam) {
  if (!event) {
    return XFA_EVENTERROR_NotExist;
  }
  switch (event.GetEventType()) {
    case XFA_ELEMENT_Execute:
      break;
    case XFA_ELEMENT_Script: {
      CXFA_Script script = event.GetScript();
      return ExecuteScript(script, pEventParam);
    } break;
    case XFA_ELEMENT_SignData:
      break;
    case XFA_ELEMENT_Submit: {
      CXFA_Submit submit = event.GetSubmit();
      return GetDoc()->GetDocProvider()->SubmitData(GetDoc(), submit);
    }
    default:
      break;
  }
  return XFA_EVENTERROR_NotExist;
}
int32_t CXFA_WidgetAcc::ProcessCalculate() {
  if (this->GetClassID() == XFA_ELEMENT_Draw) {
    return XFA_EVENTERROR_NotExist;
  }
  CXFA_Calculate calc = this->GetCalculate();
  if (!calc) {
    return XFA_EVENTERROR_NotExist;
  }
  if (this->GetNode()->HasFlag(XFA_NODEFLAG_UserInteractive)) {
    return XFA_EVENTERROR_Disabled;
  }
  CXFA_EventParam EventParam;
  EventParam.m_eType = XFA_EVENT_Calculate;
  CXFA_Script script = calc.GetScript();
  int32_t iRet = ExecuteScript(script, &EventParam);
  if (iRet == XFA_EVENTERROR_Sucess) {
    if (GetRawValue() != EventParam.m_wsResult) {
      FX_BOOL bNotify = GetDoc()->GetDocType() == XFA_DOCTYPE_Static;
      SetValue(EventParam.m_wsResult, XFA_VALUEPICTURE_Raw);
      UpdateUIDisplay();
      if (bNotify) {
        NotifyEvent(XFA_WIDGETEVENT_PostContentChanged, NULL, NULL, NULL);
      }
      iRet = XFA_EVENTERROR_Sucess;
    }
  }
  return iRet;
}
void CXFA_WidgetAcc::ProcessScriptTestValidate(CXFA_Validate validate,
                                               int32_t iRet,
                                               FXJSE_HVALUE pRetValue,
                                               FX_BOOL bVersionFlag) {
  if (iRet == XFA_EVENTERROR_Sucess && pRetValue) {
    if (FXJSE_Value_IsBoolean(pRetValue) && !FXJSE_Value_ToBoolean(pRetValue)) {
      IXFA_AppProvider* pAppProvider = GetAppProvider();
      if (!pAppProvider) {
        return;
      }
      CFX_WideString wsTitle;
      pAppProvider->LoadString(XFA_IDS_AppName, wsTitle);
      CFX_WideString wsScriptMsg;
      validate.GetScriptMessageText(wsScriptMsg);
      int32_t eScriptTest = validate.GetScriptTest();
      if (eScriptTest == XFA_ATTRIBUTEENUM_Warning) {
        if (this->GetNode()->HasFlag(XFA_NODEFLAG_UserInteractive)) {
          return;
        }
        if (wsScriptMsg.IsEmpty()) {
          GetValidateMessage(pAppProvider, wsScriptMsg, FALSE, bVersionFlag);
        }
        if (bVersionFlag) {
          pAppProvider->MsgBox(wsScriptMsg, wsTitle, XFA_MBICON_Warning,
                               XFA_MB_OK);
          return;
        }
        if (pAppProvider->MsgBox(wsScriptMsg, wsTitle, XFA_MBICON_Warning,
                                 XFA_MB_YesNo) == XFA_IDYes) {
          this->GetNode()->SetFlag(XFA_NODEFLAG_UserInteractive, TRUE, FALSE);
        }
      } else {
        if (wsScriptMsg.IsEmpty()) {
          GetValidateMessage(pAppProvider, wsScriptMsg, TRUE, bVersionFlag);
        }
        pAppProvider->MsgBox(wsScriptMsg, wsTitle, XFA_MBICON_Error, XFA_MB_OK);
      }
    }
  }
}
int32_t CXFA_WidgetAcc::ProcessFormatTestValidate(CXFA_Validate validate,
                                                  FX_BOOL bVersionFlag) {
  CFX_WideString wsRawValue = GetRawValue();
  if (!wsRawValue.IsEmpty()) {
    CFX_WideString wsPicture;
    validate.GetPicture(wsPicture);
    if (wsPicture.IsEmpty()) {
      return XFA_EVENTERROR_NotExist;
    }
    IFX_Locale* pLocale = GetLocal();
    if (!pLocale) {
      return XFA_EVENTERROR_NotExist;
    }
    CXFA_LocaleValue lcValue = XFA_GetLocaleValue(this);
    if (!lcValue.ValidateValue(lcValue.GetValue(), wsPicture, pLocale)) {
      IXFA_AppProvider* pAppProvider = GetAppProvider();
      if (!pAppProvider) {
        return XFA_EVENTERROR_NotExist;
      }
      CFX_WideString wsFormatMsg;
      validate.GetFormatMessageText(wsFormatMsg);
      CFX_WideString wsTitle;
      pAppProvider->LoadString(XFA_IDS_AppName, wsTitle);
      int32_t eFormatTest = validate.GetFormatTest();
      if (eFormatTest == XFA_ATTRIBUTEENUM_Error) {
        if (wsFormatMsg.IsEmpty()) {
          GetValidateMessage(pAppProvider, wsFormatMsg, TRUE, bVersionFlag);
        }
        pAppProvider->MsgBox(wsFormatMsg, wsTitle, XFA_MBICON_Error, XFA_MB_OK);
        return XFA_EVENTERROR_Sucess;
      }
      if (this->GetNode()->HasFlag(XFA_NODEFLAG_UserInteractive)) {
        return XFA_EVENTERROR_NotExist;
      }
      if (wsFormatMsg.IsEmpty()) {
        GetValidateMessage(pAppProvider, wsFormatMsg, FALSE, bVersionFlag);
      }
      if (bVersionFlag) {
        pAppProvider->MsgBox(wsFormatMsg, wsTitle, XFA_MBICON_Warning,
                             XFA_MB_OK);
        return XFA_EVENTERROR_Sucess;
      }
      if (pAppProvider->MsgBox(wsFormatMsg, wsTitle, XFA_MBICON_Warning,
                               XFA_MB_YesNo) == XFA_IDYes) {
        this->GetNode()->SetFlag(XFA_NODEFLAG_UserInteractive, TRUE, FALSE);
      }
      return XFA_EVENTERROR_Sucess;
    }
  }
  return XFA_EVENTERROR_NotExist;
}
int32_t CXFA_WidgetAcc::ProcessNullTestValidate(CXFA_Validate validate,
                                                int32_t iFlags,
                                                FX_BOOL bVersionFlag) {
  CFX_WideString wsValue;
  this->GetValue(wsValue, XFA_VALUEPICTURE_Raw);
  if (!wsValue.IsEmpty()) {
    return XFA_EVENTERROR_Sucess;
  }
  if (this->m_bIsNull && (this->m_bPreNull == this->m_bIsNull)) {
    return XFA_EVENTERROR_Sucess;
  }
  int32_t eNullTest = validate.GetNullTest();
  CFX_WideString wsNullMsg;
  validate.GetNullMessageText(wsNullMsg);
  if (iFlags & 0x01) {
    int32_t iRet = XFA_EVENTERROR_Sucess;
    if (eNullTest != XFA_ATTRIBUTEENUM_Disabled) {
      iRet = XFA_EVENTERROR_Error;
    }
    if (!wsNullMsg.IsEmpty()) {
      if (eNullTest != XFA_ATTRIBUTEENUM_Disabled) {
        m_pDocView->m_arrNullTestMsg.Add(wsNullMsg);
        return XFA_EVENTERROR_Error;
      }
      return XFA_EVENTERROR_Sucess;
    }
    return iRet;
  }
  if (wsNullMsg.IsEmpty() && bVersionFlag &&
      eNullTest != XFA_ATTRIBUTEENUM_Disabled) {
    return XFA_EVENTERROR_Error;
  }
  IXFA_AppProvider* pAppProvider = GetAppProvider();
  if (!pAppProvider) {
    return XFA_EVENTERROR_NotExist;
  }
  CFX_WideString wsCaptionName;
  CFX_WideString wsTitle;
  pAppProvider->LoadString(XFA_IDS_AppName, wsTitle);
  switch (eNullTest) {
    case XFA_ATTRIBUTEENUM_Error: {
      if (wsNullMsg.IsEmpty()) {
        GetValidateCaptionName(wsCaptionName, bVersionFlag);
        CFX_WideString wsError;
        pAppProvider->LoadString(XFA_IDS_ValidateNullError, wsError);
        wsNullMsg.Format(wsError, (const FX_WCHAR*)wsCaptionName);
      }
      pAppProvider->MsgBox(wsNullMsg, wsTitle, XFA_MBICON_Status, XFA_MB_OK);
      return XFA_EVENTERROR_Error;
    }
    case XFA_ATTRIBUTEENUM_Warning: {
      if (this->GetNode()->HasFlag(XFA_NODEFLAG_UserInteractive)) {
        return TRUE;
      }
      if (wsNullMsg.IsEmpty()) {
        GetValidateCaptionName(wsCaptionName, bVersionFlag);
        CFX_WideString wsWarning;
        pAppProvider->LoadString(XFA_IDS_ValidateNullWarning, wsWarning);
        wsNullMsg.Format(wsWarning, (const FX_WCHAR*)wsCaptionName,
                         (const FX_WCHAR*)wsCaptionName);
      }
      if (pAppProvider->MsgBox(wsNullMsg, wsTitle, XFA_MBICON_Warning,
                               XFA_MB_YesNo) == XFA_IDYes) {
        this->GetNode()->SetFlag(XFA_NODEFLAG_UserInteractive, TRUE, FALSE);
      }
      return XFA_EVENTERROR_Error;
    }
    case XFA_ATTRIBUTEENUM_Disabled:
    default:
      break;
  }
  return XFA_EVENTERROR_Sucess;
}
void CXFA_WidgetAcc::GetValidateCaptionName(CFX_WideString& wsCaptionName,
                                            FX_BOOL bVersionFlag) {
  if (!bVersionFlag) {
    CXFA_Caption caption = GetCaption();
    if (caption) {
      CXFA_Value capValue = caption.GetValue();
      if (capValue) {
        CXFA_Text capText = capValue.GetText();
        if (capText) {
          capText.GetContent(wsCaptionName);
        }
      }
    }
  }
  if (wsCaptionName.IsEmpty()) {
    GetName(wsCaptionName);
  }
}
void CXFA_WidgetAcc::GetValidateMessage(IXFA_AppProvider* pAppProvider,
                                        CFX_WideString& wsMessage,
                                        FX_BOOL bError,
                                        FX_BOOL bVersionFlag) {
  CFX_WideString wsCaptionName;
  GetValidateCaptionName(wsCaptionName, bVersionFlag);
  CFX_WideString wsError;
  if (bVersionFlag) {
    pAppProvider->LoadString(XFA_IDS_ValidateFailed, wsError);
    wsMessage.Format(wsError, (const FX_WCHAR*)wsCaptionName);
    return;
  }
  if (bError) {
    pAppProvider->LoadString(XFA_IDS_ValidateError, wsError);
    wsMessage.Format(wsError, (const FX_WCHAR*)wsCaptionName);
    return;
  }
  CFX_WideString wsWarning;
  pAppProvider->LoadString(XFA_IDS_ValidateWarning, wsWarning);
  wsMessage.Format(wsWarning, (const FX_WCHAR*)wsCaptionName,
                   (const FX_WCHAR*)wsCaptionName);
}
int32_t CXFA_WidgetAcc::ProcessValidate(int32_t iFlags) {
  if (this->GetClassID() == XFA_ELEMENT_Draw) {
    return XFA_EVENTERROR_NotExist;
  }
  CXFA_Validate validate = this->GetValidate();
  if (!validate) {
    return XFA_EVENTERROR_NotExist;
  }
  FX_BOOL bInitDoc = ((CXFA_Node*)validate)->HasFlag(XFA_NODEFLAG_NeedsInitApp);
  FX_BOOL bStatus =
      m_pDocView->GetLayoutStatus() < XFA_DOCVIEW_LAYOUTSTATUS_End;
  int32_t iFormat = 0;
  FXJSE_HVALUE pRetValue = NULL;
  int32_t iRet = XFA_EVENTERROR_NotExist;
  CXFA_Script script = validate.GetScript();
  if (script) {
    CXFA_EventParam eParam;
    eParam.m_eType = XFA_EVENT_Validate;
    eParam.m_pTarget = this;
    iRet = ExecuteScript(
        script, &eParam,
        ((bInitDoc || bStatus) && this->GetRawValue().IsEmpty()) ? NULL
                                                                 : &pRetValue);
  }
  XFA_VERSION version = GetDoc()->GetXFADoc()->GetCurVersionMode();
  FX_BOOL bVersionFlag = FALSE;
  if (version < XFA_VERSION_208) {
    bVersionFlag = TRUE;
  }
  if (bInitDoc) {
    ((CXFA_Node*)validate)->SetFlag(XFA_NODEFLAG_NeedsInitApp, FALSE, FALSE);
  } else {
    iFormat = ProcessFormatTestValidate(validate, bVersionFlag);
    if (!bVersionFlag) {
      bVersionFlag = GetDoc()->GetXFADoc()->HasFlag(XFA_DOCFLAG_Scripting);
    }
    iRet |= ProcessNullTestValidate(validate, iFlags, bVersionFlag);
  }
  if (iFormat != XFA_EVENTERROR_Sucess) {
    ProcessScriptTestValidate(validate, iRet, pRetValue, bVersionFlag);
  }
  if (pRetValue) {
    FXJSE_Value_Release(pRetValue);
  }
  return iRet | iFormat;
}
int32_t CXFA_WidgetAcc::ExecuteScript(CXFA_Script script,
                                      CXFA_EventParam* pEventParam,
                                      FXJSE_HVALUE* pRetValue) {
  static const uint32_t MAX_RECURSION_DEPTH = 2;
  if (m_nRecursionDepth > MAX_RECURSION_DEPTH)
    return XFA_EVENTERROR_Sucess;
  FXSYS_assert(pEventParam);
  if (!script) {
    return XFA_EVENTERROR_NotExist;
  }
  if (script.GetRunAt() == XFA_ATTRIBUTEENUM_Server) {
    return XFA_EVENTERROR_Disabled;
  }
  CFX_WideString wsExpression;
  script.GetExpression(wsExpression);
  if (wsExpression.IsEmpty()) {
    return XFA_EVENTERROR_NotExist;
  }
  XFA_SCRIPTTYPE eScriptType = script.GetContentType();
  if (eScriptType == XFA_SCRIPTTYPE_Unkown) {
    return XFA_EVENTERROR_Sucess;
  }
  CXFA_FFDoc* pDoc = GetDoc();
  IXFA_ScriptContext* pContext = pDoc->GetXFADoc()->GetScriptContext();
  pContext->SetEventParam(pEventParam);
  pContext->SetRunAtType((XFA_ATTRIBUTEENUM)script.GetRunAt());
  CXFA_NodeArray refNodes;
  if (pEventParam->m_eType == XFA_EVENT_InitCalculate ||
      pEventParam->m_eType == XFA_EVENT_Calculate) {
    pContext->SetNodesOfRunScript(&refNodes);
  }
  FXJSE_HVALUE hRetValue = FXJSE_Value_Create(pContext->GetRuntime());
  ++m_nRecursionDepth;
  FX_BOOL bRet = pContext->RunScript((XFA_SCRIPTLANGTYPE)eScriptType,
                                     wsExpression, hRetValue, m_pNode);
  --m_nRecursionDepth;
  int32_t iRet = XFA_EVENTERROR_Error;
  if (bRet) {
    iRet = XFA_EVENTERROR_Sucess;
    if (pEventParam->m_eType == XFA_EVENT_Calculate ||
        pEventParam->m_eType == XFA_EVENT_InitCalculate) {
      if (!FXJSE_Value_IsUndefined(hRetValue)) {
        if (!FXJSE_Value_IsNull(hRetValue)) {
          CFX_ByteString bsString;
          FXJSE_Value_ToUTF8String(hRetValue, bsString);
          pEventParam->m_wsResult =
              CFX_WideString::FromUTF8(bsString, bsString.GetLength());
        }
        iRet = XFA_EVENTERROR_Sucess;
      } else {
        iRet = XFA_EVENTERROR_Error;
      }
      if (pEventParam->m_eType == XFA_EVENT_InitCalculate) {
        if ((iRet == XFA_EVENTERROR_Sucess) &&
            (GetRawValue() != pEventParam->m_wsResult)) {
          SetValue(pEventParam->m_wsResult, XFA_VALUEPICTURE_Raw);
          m_pDocView->AddValidateWidget(this);
        }
      }
      int32_t iRefs = refNodes.GetSize();
      for (int32_t r = 0; r < iRefs; r++) {
        CXFA_WidgetAcc* pRefAcc = (CXFA_WidgetAcc*)refNodes[r]->GetWidgetData();
        if (pRefAcc && pRefAcc == this) {
          continue;
        }
        CXFA_Node* pRefNode = refNodes[r];
        CXFA_CalcData* pGlobalData =
            (CXFA_CalcData*)pRefNode->GetUserData(XFA_CalcData);
        if (!pGlobalData) {
          pGlobalData = new CXFA_CalcData;
          pRefNode->SetUserData(XFA_CalcData, pGlobalData,
                                &gs_XFADeleteCalcData);
        }
        if (pGlobalData->m_Globals.Find(this) < 0) {
          pGlobalData->m_Globals.Add(this);
        }
      }
    }
  }
  if (pRetValue) {
    *pRetValue = hRetValue;
  } else {
    FXJSE_Value_Release(hRetValue);
  }
  pContext->SetNodesOfRunScript(NULL);
  return iRet;
}
CXFA_FFWidget* CXFA_WidgetAcc::GetNextWidget(CXFA_FFWidget* pWidget) {
  CXFA_LayoutItem* pLayout = nullptr;
  if (pWidget) {
    pLayout = pWidget->GetNext();
  } else {
    pLayout = m_pDocView->GetXFALayout()->GetLayoutItem(m_pNode);
  }
  return static_cast<CXFA_FFWidget*>(pLayout);
}
void CXFA_WidgetAcc::UpdateUIDisplay(CXFA_FFWidget* pExcept) {
  CXFA_FFWidget* pWidget = NULL;
  while ((pWidget = this->GetNextWidget(pWidget)) != NULL) {
    if (pWidget == pExcept || !pWidget->IsLoaded() ||
        (GetUIType() != XFA_ELEMENT_CheckButton && pWidget->IsFocused())) {
      continue;
    }
    pWidget->UpdateFWLData();
    pWidget->AddInvalidateRect();
  }
}
void CXFA_WidgetAcc::NotifyEvent(FX_DWORD dwEvent,
                                 CXFA_FFWidget* pWidget,
                                 void* pParam,
                                 void* pAdditional) {
  IXFA_DocProvider* pDocProvider = GetDoc()->GetDocProvider();
  if (pWidget) {
    pDocProvider->WidgetEvent(pWidget, this, dwEvent, pParam, pAdditional);
  } else {
    pWidget = GetNextWidget(pWidget);
    if (pWidget == NULL) {
      pDocProvider->WidgetEvent(NULL, this, dwEvent, pParam, pAdditional);
      return;
    }
    while (pWidget) {
      pDocProvider->WidgetEvent(pWidget, this, dwEvent, pParam, pAdditional);
      pWidget = GetNextWidget(pWidget);
    }
  }
}
void CXFA_WidgetAcc::CalcCaptionSize(CFX_SizeF& szCap) {
  CXFA_Caption caption = this->GetCaption();
  if (!caption.IsExistInXML() ||
      caption.GetPresence() != XFA_ATTRIBUTEENUM_Visible) {
    return;
  }
  LoadCaption();
  XFA_ELEMENT eUIType = (XFA_ELEMENT)GetUIType();
  int32_t iCapPlacement = caption.GetPlacementType();
  FX_FLOAT fCapReserve = caption.GetReserve();
  FX_BOOL bVert = iCapPlacement == XFA_ATTRIBUTEENUM_Top ||
                  iCapPlacement == XFA_ATTRIBUTEENUM_Bottom;
  FX_BOOL bReserveExit = fCapReserve > 0.01;
  CXFA_TextLayout* pCapTextLayout =
      ((CXFA_FieldLayoutData*)m_pLayoutData)->m_pCapTextLayout;
  if (pCapTextLayout) {
    if (!bVert && eUIType != XFA_ELEMENT_Button) {
      szCap.x = fCapReserve;
    }
    CFX_SizeF minSize;
    minSize.Set(0, 0);
    pCapTextLayout->CalcSize(minSize, szCap, szCap);
    if (bReserveExit) {
      bVert ? szCap.y = fCapReserve : szCap.x = fCapReserve;
    }
  } else {
    FX_FLOAT fFontSize = 10.0f;
    if (CXFA_Font font = caption.GetFont()) {
      fFontSize = font.GetFontSize();
    } else if (CXFA_Font widgetfont = GetFont()) {
      fFontSize = widgetfont.GetFontSize();
    }
    if (bVert) {
      szCap.y = fCapReserve > 0 ? fCapReserve : fFontSize;
    } else {
      szCap.x = fCapReserve > 0 ? fCapReserve : 0;
      szCap.y = fFontSize;
    }
  }
  if (CXFA_Margin mgCap = caption.GetMargin()) {
    FX_FLOAT fLeftInset, fTopInset, fRightInset, fBottomInset;
    mgCap.GetLeftInset(fLeftInset);
    mgCap.GetTopInset(fTopInset);
    mgCap.GetRightInset(fRightInset);
    mgCap.GetBottomInset(fBottomInset);
    if (bReserveExit) {
      bVert ? (szCap.x += fLeftInset + fRightInset)
            : (szCap.y += fTopInset + fBottomInset);
    } else {
      szCap.x += fLeftInset + fRightInset;
      szCap.y += fTopInset + fBottomInset;
    }
  }
}
FX_BOOL CXFA_WidgetAcc::CalculateFieldAutoSize(CFX_SizeF& size) {
  CFX_SizeF szCap;
  szCap.Set(0, 0);
  CalcCaptionSize(szCap);
  CFX_RectF rtUIMargin;
  GetUIMargin(rtUIMargin);
  size.x += rtUIMargin.left + rtUIMargin.width;
  size.y += rtUIMargin.top + rtUIMargin.height;
  if (szCap.x > 0 && szCap.y > 0) {
    int32_t iCapPlacement = this->GetCaption().GetPlacementType();
    switch (iCapPlacement) {
      case XFA_ATTRIBUTEENUM_Left:
      case XFA_ATTRIBUTEENUM_Right:
      case XFA_ATTRIBUTEENUM_Inline: {
        size.x += szCap.x;
        size.y = std::max(size.y, szCap.y);
      } break;
      case XFA_ATTRIBUTEENUM_Top:
      case XFA_ATTRIBUTEENUM_Bottom: {
        size.y += szCap.y;
        size.x = std::max(size.x, szCap.x);
      }
      default:
        break;
    }
  }
  return CalculateWidgetAutoSize(size);
}
FX_BOOL CXFA_WidgetAcc::CalculateWidgetAutoSize(CFX_SizeF& size) {
  CXFA_Margin mgWidget = this->GetMargin();
  if (mgWidget.IsExistInXML()) {
    FX_FLOAT fLeftInset, fTopInset, fRightInset, fBottomInset;
    mgWidget.GetLeftInset(fLeftInset);
    mgWidget.GetTopInset(fTopInset);
    mgWidget.GetRightInset(fRightInset);
    mgWidget.GetBottomInset(fBottomInset);
    size.x += fLeftInset + fRightInset;
    size.y += fTopInset + fBottomInset;
  }
  CXFA_Para para = this->GetPara();
  if (para.IsExistInXML()) {
    size.x += para.GetMarginLeft();
    size.x += para.GetTextIndent();
  }
  FX_FLOAT fVal = 0, fMin = 0, fMax = 0;
  if (this->GetWidth(fVal)) {
    size.x = fVal;
  } else {
    if (this->GetMinWidth(fMin)) {
      size.x = std::max(size.x, fMin);
    }
    if (this->GetMaxWidth(fMax) && fMax > 0) {
      size.x = std::min(size.x, fMax);
    }
  }
  fVal = 0, fMin = 0, fMax = 0;
  if (this->GetHeight(fVal)) {
    size.y = fVal;
  } else {
    if (this->GetMinHeight(fMin)) {
      size.y = std::max(size.y, fMin);
    }
    if (this->GetMaxHeight(fMax) && fMax > 0) {
      size.y = std::min(size.y, fMax);
    }
  }
  return TRUE;
}
void CXFA_WidgetAcc::CalculateTextContentSize(CFX_SizeF& size) {
  FX_FLOAT fFontSize = GetFontSize();
  CFX_WideString wsText;
  this->GetValue(wsText, XFA_VALUEPICTURE_Display);
  if (wsText.IsEmpty()) {
    size.y += fFontSize;
    return;
  }
  FX_WCHAR wcEnter = '\n';
  FX_WCHAR wsLast = wsText.GetAt(wsText.GetLength() - 1);
  if (wsLast == wcEnter) {
    wsText = wsText + wcEnter;
  }
  if (!((CXFA_FieldLayoutData*)m_pLayoutData)->m_pTextOut) {
    ((CXFA_FieldLayoutData*)m_pLayoutData)->m_pTextOut = IFDE_TextOut::Create();
    IFDE_TextOut* pTextOut = ((CXFA_FieldLayoutData*)m_pLayoutData)->m_pTextOut;
    pTextOut->SetFont(GetFDEFont());
    pTextOut->SetFontSize(fFontSize);
    pTextOut->SetLineBreakTolerance(fFontSize * 0.2f);
    pTextOut->SetLineSpace(GetLineHeight());
    FX_DWORD dwStyles = FDE_TTOSTYLE_LastLineHeight;
    if (GetUIType() == XFA_ELEMENT_TextEdit && IsMultiLine()) {
      dwStyles |= FDE_TTOSTYLE_LineWrap;
    }
    pTextOut->SetStyles(dwStyles);
  }
  ((CXFA_FieldLayoutData*)m_pLayoutData)
      ->m_pTextOut->CalcLogicSize(wsText, wsText.GetLength(), size);
}
FX_BOOL CXFA_WidgetAcc::CalculateTextEditAutoSize(CFX_SizeF& size) {
  if (size.x > 0) {
    CFX_SizeF szOrz = size;
    CFX_SizeF szCap;
    szCap.Set(0, 0);
    CalcCaptionSize(szCap);
    FX_BOOL bCapExit = szCap.x > 0.01 && szCap.y > 0.01;
    int32_t iCapPlacement = XFA_ATTRIBUTEENUM_Unknown;
    if (bCapExit) {
      iCapPlacement = this->GetCaption().GetPlacementType();
      switch (iCapPlacement) {
        case XFA_ATTRIBUTEENUM_Left:
        case XFA_ATTRIBUTEENUM_Right:
        case XFA_ATTRIBUTEENUM_Inline: {
          size.x -= szCap.x;
        }
        default:
          break;
      }
    }
    CFX_RectF rtUIMargin;
    GetUIMargin(rtUIMargin);
    size.x -= rtUIMargin.left + rtUIMargin.width;
    CXFA_Margin mgWidget = this->GetMargin();
    if (mgWidget.IsExistInXML()) {
      FX_FLOAT fLeftInset, fRightInset;
      mgWidget.GetLeftInset(fLeftInset);
      mgWidget.GetRightInset(fRightInset);
      size.x -= fLeftInset + fRightInset;
    }
    CalculateTextContentSize(size);
    size.y += rtUIMargin.top + rtUIMargin.height;
    if (bCapExit) {
      switch (iCapPlacement) {
        case XFA_ATTRIBUTEENUM_Left:
        case XFA_ATTRIBUTEENUM_Right:
        case XFA_ATTRIBUTEENUM_Inline: {
          size.y = std::max(size.y, szCap.y);
        } break;
        case XFA_ATTRIBUTEENUM_Top:
        case XFA_ATTRIBUTEENUM_Bottom: {
          size.y += szCap.y;
        }
        default:
          break;
      }
    }
    size.x = szOrz.x;
    return CalculateWidgetAutoSize(size);
  }
  CalculateTextContentSize(size);
  return CalculateFieldAutoSize(size);
}
FX_BOOL CXFA_WidgetAcc::CalculateCheckButtonAutoSize(CFX_SizeF& size) {
  FX_FLOAT fCheckSize = this->GetCheckButtonSize();
  size.x = size.y = fCheckSize;
  return CalculateFieldAutoSize(size);
}
FX_BOOL CXFA_WidgetAcc::CalculatePushButtonAutoSize(CFX_SizeF& size) {
  CalcCaptionSize(size);
  return CalculateWidgetAutoSize(size);
}
FX_BOOL CXFA_WidgetAcc::CalculateImageAutoSize(CFX_SizeF& size) {
  if (!GetImageImage()) {
    LoadImageImage();
  }
  size.Set(0, 0);
  if (CFX_DIBitmap* pBitmap = GetImageImage()) {
    CFX_RectF rtImage, rtFit;
    rtImage.Set(0, 0, 0, 0);
    rtFit.Set(0, 0, 0, 0);
    int32_t iImageXDpi = 0;
    int32_t iImageYDpi = 0;
    GetImageDpi(iImageXDpi, iImageYDpi);
    rtImage.width =
        XFA_UnitPx2Pt((FX_FLOAT)pBitmap->GetWidth(), (FX_FLOAT)iImageXDpi);
    rtImage.height =
        XFA_UnitPx2Pt((FX_FLOAT)pBitmap->GetHeight(), (FX_FLOAT)iImageYDpi);
    if (GetWidth(rtFit.width)) {
      GetWidthWithoutMargin(rtFit.width);
    } else {
      rtFit.width = rtImage.width;
    }
    if (GetHeight(rtFit.height)) {
      GetHeightWithoutMargin(rtFit.height);
    } else {
      rtFit.height = rtImage.height;
    }
    size.x = rtFit.width;
    size.y = rtFit.height;
  }
  return CalculateWidgetAutoSize(size);
}
FX_BOOL CXFA_WidgetAcc::CalculateImageEditAutoSize(CFX_SizeF& size) {
  if (!GetImageEditImage()) {
    LoadImageEditImage();
  }
  size.Set(0, 0);
  if (CFX_DIBitmap* pBitmap = GetImageEditImage()) {
    CFX_RectF rtImage, rtFit;
    rtImage.Set(0, 0, 0, 0);
    rtFit.Set(0, 0, 0, 0);
    int32_t iImageXDpi = 0;
    int32_t iImageYDpi = 0;
    GetImageEditDpi(iImageXDpi, iImageYDpi);
    rtImage.width =
        XFA_UnitPx2Pt((FX_FLOAT)pBitmap->GetWidth(), (FX_FLOAT)iImageXDpi);
    rtImage.height =
        XFA_UnitPx2Pt((FX_FLOAT)pBitmap->GetHeight(), (FX_FLOAT)iImageYDpi);
    if (GetWidth(rtFit.width)) {
      GetWidthWithoutMargin(rtFit.width);
    } else {
      rtFit.width = rtImage.width;
    }
    if (GetHeight(rtFit.height)) {
      GetHeightWithoutMargin(rtFit.height);
    } else {
      rtFit.height = rtImage.height;
    }
    size.x = rtFit.width;
    size.y = rtFit.height;
  }
  return CalculateFieldAutoSize(size);
}
FX_BOOL CXFA_WidgetAcc::LoadImageImage() {
  InitLayoutData();
  return ((CXFA_ImageLayoutData*)m_pLayoutData)->LoadImageData(this);
}
FX_BOOL CXFA_WidgetAcc::LoadImageEditImage() {
  InitLayoutData();
  return ((CXFA_ImageEditData*)m_pLayoutData)->LoadImageData(this);
}
void CXFA_WidgetAcc::GetImageDpi(int32_t& iImageXDpi, int32_t& iImageYDpi) {
  iImageXDpi = ((CXFA_ImageLayoutData*)m_pLayoutData)->m_iImageXDpi;
  iImageYDpi = ((CXFA_ImageLayoutData*)m_pLayoutData)->m_iImageYDpi;
}
void CXFA_WidgetAcc::GetImageEditDpi(int32_t& iImageXDpi, int32_t& iImageYDpi) {
  iImageXDpi = ((CXFA_ImageEditData*)m_pLayoutData)->m_iImageXDpi;
  iImageYDpi = ((CXFA_ImageEditData*)m_pLayoutData)->m_iImageYDpi;
}
FX_BOOL CXFA_WidgetAcc::CalculateTextAutoSize(CFX_SizeF& size) {
  LoadText();
  CXFA_TextLayout* pTextLayout =
      ((CXFA_TextLayoutData*)m_pLayoutData)->m_pTextLayout;
  if (pTextLayout) {
    size.x = pTextLayout->StartLayout(size.x);
    size.y = pTextLayout->GetLayoutHeight();
  }
  return CalculateWidgetAutoSize(size);
}
void CXFA_WidgetAcc::LoadText() {
  InitLayoutData();
  ((CXFA_TextLayoutData*)m_pLayoutData)->LoadText(this);
}
FX_FLOAT CXFA_WidgetAcc::CalculateWidgetAutoWidth(FX_FLOAT fWidthCalc) {
  CXFA_Margin mgWidget = this->GetMargin();
  if (mgWidget.IsExistInXML()) {
    FX_FLOAT fLeftInset, fRightInset;
    mgWidget.GetLeftInset(fLeftInset);
    mgWidget.GetRightInset(fRightInset);
    fWidthCalc += fLeftInset + fRightInset;
  }
  FX_FLOAT fMin = 0, fMax = 0;
  if (this->GetMinWidth(fMin)) {
    fWidthCalc = std::max(fWidthCalc, fMin);
  }
  if (this->GetMaxWidth(fMax) && fMax > 0) {
    fWidthCalc = std::min(fWidthCalc, fMax);
  }
  return fWidthCalc;
}
FX_FLOAT CXFA_WidgetAcc::GetWidthWithoutMargin(FX_FLOAT fWidthCalc) {
  CXFA_Margin mgWidget = this->GetMargin();
  if (mgWidget.IsExistInXML()) {
    FX_FLOAT fLeftInset, fRightInset;
    mgWidget.GetLeftInset(fLeftInset);
    mgWidget.GetRightInset(fRightInset);
    fWidthCalc -= fLeftInset + fRightInset;
  }
  return fWidthCalc;
}
FX_FLOAT CXFA_WidgetAcc::CalculateWidgetAutoHeight(FX_FLOAT fHeightCalc) {
  CXFA_Margin mgWidget = this->GetMargin();
  if (mgWidget.IsExistInXML()) {
    FX_FLOAT fTopInset, fBottomInset;
    mgWidget.GetTopInset(fTopInset);
    mgWidget.GetBottomInset(fBottomInset);
    fHeightCalc += fTopInset + fBottomInset;
  }
  FX_FLOAT fMin = 0, fMax = 0;
  if (this->GetMinHeight(fMin)) {
    fHeightCalc = std::max(fHeightCalc, fMin);
  }
  if (this->GetMaxHeight(fMax) && fMax > 0) {
    fHeightCalc = std::min(fHeightCalc, fMax);
  }
  return fHeightCalc;
}
FX_FLOAT CXFA_WidgetAcc::GetHeightWithoutMargin(FX_FLOAT fHeightCalc) {
  CXFA_Margin mgWidget = this->GetMargin();
  if (mgWidget.IsExistInXML()) {
    FX_FLOAT fTopInset, fBottomInset;
    mgWidget.GetTopInset(fTopInset);
    mgWidget.GetBottomInset(fBottomInset);
    fHeightCalc -= fTopInset + fBottomInset;
  }
  return fHeightCalc;
}
void CXFA_WidgetAcc::StartWidgetLayout(FX_FLOAT& fCalcWidth,
                                       FX_FLOAT& fCalcHeight) {
  InitLayoutData();
  XFA_ELEMENT eUIType = GetUIType();
  if (eUIType == XFA_ELEMENT_Text) {
    m_pLayoutData->m_fWidgetHeight = -1;
    GetHeight(m_pLayoutData->m_fWidgetHeight);
    StartTextLayout(fCalcWidth, fCalcHeight);
    return;
  }
  if (fCalcWidth > 0 && fCalcHeight > 0) {
    return;
  }
  m_pLayoutData->m_fWidgetHeight = -1;
  FX_FLOAT fWidth = 0;
  if (fCalcWidth > 0 && fCalcHeight < 0) {
    if (!GetHeight(fCalcHeight)) {
      CalculateAccWidthAndHeight(eUIType, fCalcWidth, fCalcHeight);
    }
    m_pLayoutData->m_fWidgetHeight = fCalcHeight;
    return;
  }
  if (fCalcWidth < 0 && fCalcHeight < 0) {
    if (!GetWidth(fWidth) || !GetHeight(fCalcHeight)) {
      CalculateAccWidthAndHeight(eUIType, fWidth, fCalcHeight);
    }
    fCalcWidth = fWidth;
  }
  m_pLayoutData->m_fWidgetHeight = fCalcHeight;
}
void CXFA_WidgetAcc::CalculateAccWidthAndHeight(XFA_ELEMENT eUIType,
                                                FX_FLOAT& fWidth,
                                                FX_FLOAT& fCalcHeight) {
  CFX_SizeF sz;
  sz.Set(fWidth, m_pLayoutData->m_fWidgetHeight);
  switch (eUIType) {
    case XFA_ELEMENT_Barcode:
    case XFA_ELEMENT_ChoiceList:
    case XFA_ELEMENT_Signature:
      CalculateFieldAutoSize(sz);
      break;
    case XFA_ELEMENT_ImageEdit:
      CalculateImageEditAutoSize(sz);
      break;
    case XFA_ELEMENT_Button:
      CalculatePushButtonAutoSize(sz);
      break;
    case XFA_ELEMENT_CheckButton:
      CalculateCheckButtonAutoSize(sz);
      break;
    case XFA_ELEMENT_DateTimeEdit:
    case XFA_ELEMENT_NumericEdit:
    case XFA_ELEMENT_PasswordEdit:
    case XFA_ELEMENT_TextEdit:
      CalculateTextEditAutoSize(sz);
      break;
    case XFA_ELEMENT_Image:
      CalculateImageAutoSize(sz);
      break;
    case XFA_ELEMENT_Arc:
    case XFA_ELEMENT_Line:
    case XFA_ELEMENT_Rectangle:
    case XFA_ELEMENT_Subform:
    case XFA_ELEMENT_ExclGroup:
      CalculateWidgetAutoSize(sz);
      break;
    default:
      break;
  }
  fWidth = sz.x;
  m_pLayoutData->m_fWidgetHeight = sz.y;
  fCalcHeight = sz.y;
}
FX_BOOL CXFA_WidgetAcc::FindSplitPos(int32_t iBlockIndex,
                                     FX_FLOAT& fCalcHeight) {
  XFA_ELEMENT eUIType = (XFA_ELEMENT)GetUIType();
  if (eUIType == XFA_ELEMENT_Subform) {
    return FALSE;
  }
  if (eUIType != XFA_ELEMENT_Text && eUIType != XFA_ELEMENT_TextEdit &&
      eUIType != XFA_ELEMENT_NumericEdit &&
      eUIType != XFA_ELEMENT_PasswordEdit) {
    fCalcHeight = 0;
    return TRUE;
  }
  FX_FLOAT fTopInset = 0;
  FX_FLOAT fBottomInset = 0;
  if (iBlockIndex == 0) {
    CXFA_Margin mgWidget = this->GetMargin();
    if (mgWidget.IsExistInXML()) {
      mgWidget.GetTopInset(fTopInset);
      mgWidget.GetBottomInset(fBottomInset);
    }
    CFX_RectF rtUIMargin;
    this->GetUIMargin(rtUIMargin);
    fTopInset += rtUIMargin.top;
    fBottomInset += rtUIMargin.width;
  }
  if (eUIType == XFA_ELEMENT_Text) {
    FX_FLOAT fHeight = fCalcHeight;
    if (iBlockIndex == 0) {
      fCalcHeight = fCalcHeight - fTopInset;
      if (fCalcHeight < 0) {
        fCalcHeight = 0;
      }
    }
    CXFA_TextLayout* pTextLayout =
        ((CXFA_TextLayoutData*)m_pLayoutData)->m_pTextLayout;
    pTextLayout->DoLayout(iBlockIndex, fCalcHeight, fCalcHeight,
                          m_pLayoutData->m_fWidgetHeight - fTopInset);
    if (fCalcHeight != 0) {
      if (iBlockIndex == 0) {
        fCalcHeight = fCalcHeight + fTopInset;
      }
      if (fabs(fHeight - fCalcHeight) < XFA_FLOAT_PERCISION) {
        return FALSE;
      }
    }
    return TRUE;
  }
  XFA_ATTRIBUTEENUM iCapPlacement = XFA_ATTRIBUTEENUM_Unknown;
  FX_FLOAT fCapReserve = 0;
  if (iBlockIndex == 0) {
    CXFA_Caption caption = GetCaption();
    if (caption.IsExistInXML() &&
        caption.GetPresence() != XFA_ATTRIBUTEENUM_Hidden) {
      iCapPlacement = (XFA_ATTRIBUTEENUM)caption.GetPlacementType();
      fCapReserve = caption.GetReserve();
    }
    if (iCapPlacement == XFA_ATTRIBUTEENUM_Top &&
        fCalcHeight < fCapReserve + fTopInset) {
      fCalcHeight = 0;
      return TRUE;
    }
    if (iCapPlacement == XFA_ATTRIBUTEENUM_Bottom &&
        m_pLayoutData->m_fWidgetHeight - fCapReserve - fBottomInset) {
      fCalcHeight = 0;
      return TRUE;
    }
    if (iCapPlacement != XFA_ATTRIBUTEENUM_Top) {
      fCapReserve = 0;
    }
  }
  int32_t iLinesCount = 0;
  FX_FLOAT fHeight = m_pLayoutData->m_fWidgetHeight;
  CFX_WideString wsText;
  this->GetValue(wsText, XFA_VALUEPICTURE_Display);
  if (wsText.IsEmpty()) {
    iLinesCount = 1;
  } else {
    if (!((CXFA_FieldLayoutData*)m_pLayoutData)->m_pTextOut) {
      FX_FLOAT fWidth = 0;
      GetWidth(fWidth);
      CalculateAccWidthAndHeight(eUIType, fWidth, fHeight);
    }
    iLinesCount =
        ((CXFA_FieldLayoutData*)m_pLayoutData)->m_pTextOut->GetTotalLines();
  }
  if (!((CXFA_FieldLayoutData*)m_pLayoutData)->m_pFieldSplitArray) {
    ((CXFA_FieldLayoutData*)m_pLayoutData)->m_pFieldSplitArray =
        new CFX_FloatArray;
  }
  CFX_FloatArray* pFieldArray =
      ((CXFA_FieldLayoutData*)m_pLayoutData)->m_pFieldSplitArray;
  int32_t iFieldSplitCount = pFieldArray->GetSize();
  for (int32_t i = 0; i < iBlockIndex * 3; i += 3) {
    iLinesCount -= (int32_t)pFieldArray->GetAt(i + 1);
    fHeight -= pFieldArray->GetAt(i + 2);
  }
  if (iLinesCount == 0) {
    return FALSE;
  }
  FX_FLOAT fLineHeight = GetLineHeight();
  FX_FLOAT fFontSize = GetFontSize();
  FX_FLOAT fTextHeight = iLinesCount * fLineHeight - fLineHeight + fFontSize;
  FX_FLOAT fSpaceAbove = 0;
  FX_FLOAT fStartOffset = 0;
  if (fHeight > 0.1f && iBlockIndex == 0) {
    fStartOffset = fTopInset;
    fHeight -= (fTopInset + fBottomInset);
    if (CXFA_Para para = this->GetPara()) {
      fSpaceAbove = para.GetSpaceAbove();
      FX_FLOAT fSpaceBelow = para.GetSpaceBelow();
      fHeight -= (fSpaceAbove + fSpaceBelow);
      switch (para.GetVerticalAlign()) {
        case XFA_ATTRIBUTEENUM_Top:
          fStartOffset += fSpaceAbove;
          break;
        case XFA_ATTRIBUTEENUM_Middle:
          fStartOffset += ((fHeight - fTextHeight) / 2 + fSpaceAbove);
          break;
        case XFA_ATTRIBUTEENUM_Bottom:
          fStartOffset += (fHeight - fTextHeight + fSpaceAbove);
          break;
      }
    }
    if (fStartOffset < 0.1f) {
      fStartOffset = 0;
    }
  }
  for (int32_t i = iBlockIndex - 1; iBlockIndex > 0 && i < iBlockIndex; i++) {
    fStartOffset = pFieldArray->GetAt(i * 3) - pFieldArray->GetAt(i * 3 + 2);
    if (fStartOffset < 0.1f) {
      fStartOffset = 0;
    }
  }
  if (iFieldSplitCount / 3 == (iBlockIndex + 1)) {
    pFieldArray->SetAt(0, fStartOffset);
  } else {
    pFieldArray->Add(fStartOffset);
  }
  XFA_VERSION version = GetDoc()->GetXFADoc()->GetCurVersionMode();
  FX_BOOL bCanSplitNoContent = FALSE;
  XFA_ATTRIBUTEENUM eLayoutMode;
  this->GetNode()
      ->GetNodeItem(XFA_NODEITEM_Parent)
      ->TryEnum(XFA_ATTRIBUTE_Layout, eLayoutMode, TRUE);
  if ((eLayoutMode == XFA_ATTRIBUTEENUM_Position ||
       eLayoutMode == XFA_ATTRIBUTEENUM_Tb ||
       eLayoutMode == XFA_ATTRIBUTEENUM_Row ||
       eLayoutMode == XFA_ATTRIBUTEENUM_Table) &&
      version > XFA_VERSION_208) {
    bCanSplitNoContent = TRUE;
  }
  if ((eLayoutMode == XFA_ATTRIBUTEENUM_Tb ||
       eLayoutMode == XFA_ATTRIBUTEENUM_Row ||
       eLayoutMode == XFA_ATTRIBUTEENUM_Table) &&
      version <= XFA_VERSION_208) {
    if (fStartOffset < fCalcHeight) {
      bCanSplitNoContent = TRUE;
    } else {
      fCalcHeight = 0;
      return TRUE;
    }
  }
  if (bCanSplitNoContent) {
    if ((fCalcHeight - fTopInset - fSpaceAbove < fLineHeight)) {
      fCalcHeight = 0;
      return TRUE;
    }
    if (fStartOffset + XFA_FLOAT_PERCISION >= fCalcHeight) {
      if (iFieldSplitCount / 3 == (iBlockIndex + 1)) {
        pFieldArray->SetAt(iBlockIndex * 3 + 1, 0);
        pFieldArray->SetAt(iBlockIndex * 3 + 2, fCalcHeight);
      } else {
        pFieldArray->Add(0);
        pFieldArray->Add(fCalcHeight);
      }
      return FALSE;
    }
    if (fCalcHeight - fStartOffset < fLineHeight) {
      fCalcHeight = fStartOffset;
      if (iFieldSplitCount / 3 == (iBlockIndex + 1)) {
        pFieldArray->SetAt(iBlockIndex * 3 + 1, 0);
        pFieldArray->SetAt(iBlockIndex * 3 + 2, fCalcHeight);
      } else {
        pFieldArray->Add(0);
        pFieldArray->Add(fCalcHeight);
      }
      return TRUE;
    }
    FX_FLOAT fTextNum =
        fCalcHeight + XFA_FLOAT_PERCISION - fCapReserve - fStartOffset;
    int32_t iLineNum =
        (int32_t)((fTextNum + (fLineHeight - fFontSize)) / fLineHeight);
    if (iLineNum >= iLinesCount) {
      if (fCalcHeight - fStartOffset - fTextHeight >= fFontSize) {
        if (iFieldSplitCount / 3 == (iBlockIndex + 1)) {
          pFieldArray->SetAt(iBlockIndex * 3 + 1, (FX_FLOAT)iLinesCount);
          pFieldArray->SetAt(iBlockIndex * 3 + 2, fCalcHeight);
        } else {
          pFieldArray->Add((FX_FLOAT)iLinesCount);
          pFieldArray->Add(fCalcHeight);
        }
        return FALSE;
      }
      if (fHeight - fStartOffset - fTextHeight < fFontSize) {
        iLineNum -= 1;
        if (iLineNum == 0) {
          fCalcHeight = 0;
          return TRUE;
        }
      } else {
        iLineNum = (int32_t)(fTextNum / fLineHeight);
      }
    }
    if (iLineNum > 0) {
      FX_FLOAT fSplitHeight =
          iLineNum * fLineHeight + fCapReserve + fStartOffset;
      if (iFieldSplitCount / 3 == (iBlockIndex + 1)) {
        pFieldArray->SetAt(iBlockIndex * 3 + 1, (FX_FLOAT)iLineNum);
        pFieldArray->SetAt(iBlockIndex * 3 + 2, fSplitHeight);
      } else {
        pFieldArray->Add((FX_FLOAT)iLineNum);
        pFieldArray->Add(fSplitHeight);
      }
      if (fabs(fSplitHeight - fCalcHeight) < XFA_FLOAT_PERCISION) {
        return FALSE;
      }
      fCalcHeight = fSplitHeight;
      return TRUE;
    }
  }
  fCalcHeight = 0;
  return TRUE;
}
void CXFA_WidgetAcc::InitLayoutData() {
  if (m_pLayoutData) {
    return;
  }
  switch (GetUIType()) {
    case XFA_ELEMENT_Text:
      m_pLayoutData = new CXFA_TextLayoutData;
      return;
    case XFA_ELEMENT_TextEdit:
      m_pLayoutData = new CXFA_TextEditData;
      return;
    case XFA_ELEMENT_Image:
      m_pLayoutData = new CXFA_ImageLayoutData;
      return;
    case XFA_ELEMENT_ImageEdit:
      m_pLayoutData = new CXFA_ImageEditData;
      return;
    default:
      break;
  }
  if (GetClassID() == XFA_ELEMENT_Field) {
    m_pLayoutData = new CXFA_FieldLayoutData;
  } else {
    m_pLayoutData = new CXFA_WidgetLayoutData;
  }
}
void CXFA_WidgetAcc::StartTextLayout(FX_FLOAT& fCalcWidth,
                                     FX_FLOAT& fCalcHeight) {
  LoadText();
  CXFA_TextLayout* pTextLayout =
      ((CXFA_TextLayoutData*)m_pLayoutData)->m_pTextLayout;
  FX_FLOAT fTextHeight = 0;
  if (fCalcWidth > 0 && fCalcHeight > 0) {
    FX_FLOAT fWidth = GetWidthWithoutMargin(fCalcWidth);
    pTextLayout->StartLayout(fWidth);
    fTextHeight = fCalcHeight;
    fTextHeight = GetHeightWithoutMargin(fTextHeight);
    pTextLayout->DoLayout(0, fTextHeight, -1, fTextHeight);
    return;
  }
  if (fCalcWidth > 0 && fCalcHeight < 0) {
    FX_FLOAT fWidth = GetWidthWithoutMargin(fCalcWidth);
    pTextLayout->StartLayout(fWidth);
  }
  if (fCalcWidth < 0 && fCalcHeight < 0) {
    FX_FLOAT fMaxWidth = -1;
    FX_BOOL bRet = GetWidth(fMaxWidth);
    if (bRet) {
      FX_FLOAT fWidth = GetWidthWithoutMargin(fMaxWidth);
      pTextLayout->StartLayout(fWidth);
    } else {
      FX_FLOAT fWidth = pTextLayout->StartLayout(fMaxWidth);
      fMaxWidth = CalculateWidgetAutoWidth(fWidth);
      fWidth = GetWidthWithoutMargin(fMaxWidth);
      pTextLayout->StartLayout(fWidth);
    }
    fCalcWidth = fMaxWidth;
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
FX_BOOL CXFA_WidgetAcc::LoadCaption() {
  InitLayoutData();
  return ((CXFA_FieldLayoutData*)m_pLayoutData)->LoadCaption(this);
}
CXFA_TextLayout* CXFA_WidgetAcc::GetCaptionTextLayout() {
  return m_pLayoutData
             ? ((CXFA_FieldLayoutData*)m_pLayoutData)->m_pCapTextLayout
             : NULL;
}
CXFA_TextLayout* CXFA_WidgetAcc::GetTextLayout() {
  return m_pLayoutData ? ((CXFA_TextLayoutData*)m_pLayoutData)->m_pTextLayout
                       : NULL;
}
CFX_DIBitmap* CXFA_WidgetAcc::GetImageImage() {
  return m_pLayoutData ? ((CXFA_ImageLayoutData*)m_pLayoutData)->m_pDIBitmap
                       : NULL;
}
CFX_DIBitmap* CXFA_WidgetAcc::GetImageEditImage() {
  return m_pLayoutData ? ((CXFA_ImageEditData*)m_pLayoutData)->m_pDIBitmap
                       : NULL;
}
void CXFA_WidgetAcc::SetImageImage(CFX_DIBitmap* newImage) {
  if (((CXFA_ImageLayoutData*)m_pLayoutData)->m_pDIBitmap == newImage) {
    return;
  }
  if (((CXFA_ImageLayoutData*)m_pLayoutData)->m_pDIBitmap &&
      !((CXFA_ImageLayoutData*)m_pLayoutData)->m_bNamedImage) {
    delete ((CXFA_ImageLayoutData*)m_pLayoutData)->m_pDIBitmap;
    ((CXFA_ImageLayoutData*)m_pLayoutData)->m_pDIBitmap = NULL;
  }
  ((CXFA_ImageLayoutData*)m_pLayoutData)->m_pDIBitmap = newImage;
}
void CXFA_WidgetAcc::SetImageEditImage(CFX_DIBitmap* newImage) {
  if (((CXFA_ImageEditData*)m_pLayoutData)->m_pDIBitmap == newImage) {
    return;
  }
  if (((CXFA_ImageEditData*)m_pLayoutData)->m_pDIBitmap &&
      !((CXFA_ImageEditData*)m_pLayoutData)->m_bNamedImage) {
    delete ((CXFA_ImageEditData*)m_pLayoutData)->m_pDIBitmap;
    ((CXFA_ImageEditData*)m_pLayoutData)->m_pDIBitmap = NULL;
  }
  ((CXFA_ImageEditData*)m_pLayoutData)->m_pDIBitmap = newImage;
}
CXFA_WidgetLayoutData* CXFA_WidgetAcc::GetWidgetLayoutData() {
  return m_pLayoutData;
}
IFX_Font* CXFA_WidgetAcc::GetFDEFont() {
  CFX_WideStringC wsFontName = FX_WSTRC(L"Courier");
  FX_DWORD dwFontStyle = 0;
  if (CXFA_Font font = this->GetFont()) {
    if (font.IsBold()) {
      dwFontStyle |= FX_FONTSTYLE_Bold;
    }
    if (font.IsItalic()) {
      dwFontStyle |= FX_FONTSTYLE_Italic;
    }
    font.GetTypeface(wsFontName);
  }
  CXFA_FFDoc* pDoc = GetDoc();
  return pDoc->GetApp()->GetXFAFontMgr()->GetFont(pDoc, wsFontName,
                                                  dwFontStyle);
}
FX_FLOAT CXFA_WidgetAcc::GetFontSize() {
  FX_FLOAT fFontSize = 10.0f;
  if (CXFA_Font font = this->GetFont()) {
    fFontSize = font.GetFontSize();
  }
  return fFontSize < 0.1f ? 10.0f : fFontSize;
}
FX_FLOAT CXFA_WidgetAcc::GetLineHeight() {
  FX_FLOAT fLineHeight = 0;
  if (CXFA_Para para = this->GetPara()) {
    fLineHeight = para.GetLineHeight();
  }
  if (fLineHeight < 1) {
    fLineHeight = GetFontSize() * 1.2f;
  }
  return fLineHeight;
}
FX_ARGB CXFA_WidgetAcc::GetTextColor() {
  if (CXFA_Font font = this->GetFont()) {
    return font.GetColor();
  }
  return 0xFF000000;
}
CXFA_Node* CXFA_TextProvider::GetTextNode(FX_BOOL& bRichText) {
  bRichText = FALSE;
  if (m_pTextNode) {
    if (m_pTextNode->GetClassID() == XFA_ELEMENT_ExData) {
      CFX_WideString wsContentType;
      m_pTextNode->GetAttribute(XFA_ATTRIBUTE_ContentType, wsContentType,
                                FALSE);
      if (wsContentType.Equal(FX_WSTRC(L"text/html"))) {
        bRichText = TRUE;
      }
    }
    return m_pTextNode;
  }
  if (m_eType == XFA_TEXTPROVIDERTYPE_Text) {
    CXFA_Node* pElementNode = m_pWidgetAcc->GetNode();
    CXFA_Node* pValueNode = pElementNode->GetChild(0, XFA_ELEMENT_Value);
    if (!pValueNode) {
      return NULL;
    }
    CXFA_Node* pChildNode = pValueNode->GetNodeItem(XFA_NODEITEM_FirstChild);
    if (pChildNode && pChildNode->GetClassID() == XFA_ELEMENT_ExData) {
      CFX_WideString wsContentType;
      pChildNode->GetAttribute(XFA_ATTRIBUTE_ContentType, wsContentType, FALSE);
      if (wsContentType.Equal(FX_WSTRC(L"text/html"))) {
        bRichText = TRUE;
      }
    }
    return pChildNode;
  } else if (m_eType == XFA_TEXTPROVIDERTYPE_Datasets) {
    CXFA_Node* pBind = m_pWidgetAcc->GetDatasets();
    IFDE_XMLNode* pXMLNode = pBind->GetXMLMappingNode();
    FXSYS_assert(pXMLNode);
    for (IFDE_XMLNode* pXMLChild =
             pXMLNode->GetNodeItem(IFDE_XMLNode::FirstChild);
         pXMLChild;
         pXMLChild = pXMLChild->GetNodeItem(IFDE_XMLNode::NextSibling)) {
      if (pXMLChild->GetType() == FDE_XMLNODE_Element) {
        IFDE_XMLElement* pElement = (IFDE_XMLElement*)pXMLChild;
        if (XFA_RecognizeRichText(pElement)) {
          bRichText = TRUE;
        }
      }
    }
    return pBind;
  } else if (m_eType == XFA_TEXTPROVIDERTYPE_Caption) {
    CXFA_Node* pCaptionNode =
        m_pWidgetAcc->GetNode()->GetChild(0, XFA_ELEMENT_Caption);
    if (pCaptionNode == NULL) {
      return NULL;
    }
    CXFA_Node* pValueNode = pCaptionNode->GetChild(0, XFA_ELEMENT_Value);
    if (pValueNode == NULL) {
      return NULL;
    }
    CXFA_Node* pChildNode = pValueNode->GetNodeItem(XFA_NODEITEM_FirstChild);
    if (pChildNode && pChildNode->GetClassID() == XFA_ELEMENT_ExData) {
      CFX_WideString wsContentType;
      pChildNode->GetAttribute(XFA_ATTRIBUTE_ContentType, wsContentType, FALSE);
      if (wsContentType.Equal(FX_WSTRC(L"text/html"))) {
        bRichText = TRUE;
      }
    }
    return pChildNode;
  }
  CXFA_Node* pItemNode =
      m_pWidgetAcc->GetNode()->GetChild(0, XFA_ELEMENT_Items);
  if (pItemNode == NULL) {
    return NULL;
  }
  CXFA_Node* pNode = pItemNode->GetNodeItem(XFA_NODEITEM_FirstChild);
  while (pNode) {
    CFX_WideStringC wsName;
    pNode->TryCData(XFA_ATTRIBUTE_Name, wsName);
    if (m_eType == XFA_TEXTPROVIDERTYPE_Rollover &&
        wsName == FX_WSTRC(L"rollover")) {
      return pNode;
    }
    if (m_eType == XFA_TEXTPROVIDERTYPE_Down && wsName == FX_WSTRC(L"down")) {
      return pNode;
    }
    pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling);
  }
  return NULL;
}
CXFA_Para CXFA_TextProvider::GetParaNode() {
  if (m_eType == XFA_TEXTPROVIDERTYPE_Text) {
    return m_pWidgetAcc->GetPara();
  }
  CXFA_Node* pNode = m_pWidgetAcc->GetNode()->GetChild(0, XFA_ELEMENT_Caption);
  return pNode->GetChild(0, XFA_ELEMENT_Para);
}
CXFA_Font CXFA_TextProvider::GetFontNode() {
  if (m_eType == XFA_TEXTPROVIDERTYPE_Text) {
    return m_pWidgetAcc->GetFont();
  }
  CXFA_Node* pNode = m_pWidgetAcc->GetNode()->GetChild(0, XFA_ELEMENT_Caption);
  pNode = pNode->GetChild(0, XFA_ELEMENT_Font);
  if (pNode) {
    return pNode;
  }
  return m_pWidgetAcc->GetFont();
}
FX_BOOL CXFA_TextProvider::IsCheckButtonAndAutoWidth() {
  XFA_ELEMENT eType = m_pWidgetAcc->GetUIType();
  if (eType == XFA_ELEMENT_CheckButton) {
    FX_FLOAT fWidth = 0;
    return !m_pWidgetAcc->GetWidth(fWidth);
  }
  return FALSE;
}
FX_BOOL CXFA_TextProvider::GetEmbbedObj(FX_BOOL bURI,
                                        FX_BOOL bRaw,
                                        const CFX_WideString& wsAttr,
                                        CFX_WideString& wsValue) {
  if (m_eType != XFA_TEXTPROVIDERTYPE_Text) {
    return FALSE;
  }
  if (bURI) {
    CXFA_Node* pWidgetNode = m_pWidgetAcc->GetNode();
    CXFA_Node* pParent = pWidgetNode->GetNodeItem(XFA_NODEITEM_Parent);
    CXFA_Document* pDocument = pWidgetNode->GetDocument();
    CXFA_Node* pIDNode = NULL;
    CXFA_WidgetAcc* pEmbAcc = NULL;
    if (pParent) {
      pIDNode = pDocument->GetNodeByID(pParent, wsAttr);
    }
    if (!pIDNode) {
      pIDNode = pDocument->GetNodeByID(
          (CXFA_Node*)pDocument->GetXFANode(XFA_HASHCODE_Form), wsAttr);
    }
    if (pIDNode) {
      pEmbAcc = (CXFA_WidgetAcc*)pIDNode->GetWidgetData();
    }
    if (pEmbAcc) {
      pEmbAcc->GetValue(wsValue, XFA_VALUEPICTURE_Display);
      return TRUE;
    }
  }
  return FALSE;
}
