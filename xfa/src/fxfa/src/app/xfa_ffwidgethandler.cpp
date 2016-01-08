// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/foxitlib.h"
#include "xfa/src/fxfa/src/common/xfa_common.h"
#include "xfa_fwladapter.h"
#include "xfa_ffwidgethandler.h"
#include "xfa_ffwidget.h"
#include "xfa_fffield.h"
#include "xfa_ffchoicelist.h"
#include "xfa_ffdoc.h"
#include "xfa_ffdocview.h"
CXFA_FFWidgetHandler::CXFA_FFWidgetHandler(CXFA_FFDocView* pDocView)
    : m_pDocView(pDocView) {}
CXFA_FFWidgetHandler::~CXFA_FFWidgetHandler() {}
IXFA_PageView* CXFA_FFWidgetHandler::GetPageView(IXFA_Widget* hWidget) {
  return static_cast<CXFA_FFWidget*>(hWidget)->GetPageView();
}
void CXFA_FFWidgetHandler::GetRect(IXFA_Widget* hWidget, CFX_RectF& rt) {
  static_cast<CXFA_FFWidget*>(hWidget)->GetWidgetRect(rt);
}
FX_DWORD CXFA_FFWidgetHandler::GetStatus(IXFA_Widget* hWidget) {
  return static_cast<CXFA_FFWidget*>(hWidget)->GetStatus();
}
FX_BOOL CXFA_FFWidgetHandler::GetBBox(IXFA_Widget* hWidget,
                                      CFX_RectF& rtBox,
                                      FX_DWORD dwStatus,
                                      FX_BOOL bDrawFocus) {
  return static_cast<CXFA_FFWidget*>(hWidget)
      ->GetBBox(rtBox, dwStatus, bDrawFocus);
}
CXFA_WidgetAcc* CXFA_FFWidgetHandler::GetDataAcc(IXFA_Widget* hWidget) {
  return static_cast<CXFA_FFWidget*>(hWidget)->GetDataAcc();
}
void CXFA_FFWidgetHandler::GetName(IXFA_Widget* hWidget,
                                   CFX_WideString& wsName,
                                   int32_t iNameType) {
  static_cast<CXFA_FFWidget*>(hWidget)->GetDataAcc()->GetName(wsName,
                                                              iNameType);
}
FX_BOOL CXFA_FFWidgetHandler::GetToolTip(IXFA_Widget* hWidget,
                                         CFX_WideString& wsToolTip) {
  return static_cast<CXFA_FFWidget*>(hWidget)->GetToolTip(wsToolTip);
}
void CXFA_FFWidgetHandler::SetPrivateData(IXFA_Widget* hWidget,
                                          void* module_id,
                                          void* pData,
                                          PD_CALLBACK_FREEDATA callback) {
  static_cast<CXFA_FFWidget*>(hWidget)
      ->SetPrivateData(module_id, pData, callback);
}
void* CXFA_FFWidgetHandler::GetPrivateData(IXFA_Widget* hWidget,
                                           void* module_id) {
  return static_cast<CXFA_FFWidget*>(hWidget)->GetPrivateData(module_id);
}
FX_BOOL CXFA_FFWidgetHandler::OnMouseEnter(IXFA_Widget* hWidget) {
  m_pDocView->LockUpdate();
  FX_BOOL bRet = static_cast<CXFA_FFWidget*>(hWidget)->OnMouseEnter();
  m_pDocView->UnlockUpdate();
  m_pDocView->UpdateDocView();
  return bRet;
}
FX_BOOL CXFA_FFWidgetHandler::OnMouseExit(IXFA_Widget* hWidget) {
  m_pDocView->LockUpdate();
  FX_BOOL bRet = static_cast<CXFA_FFWidget*>(hWidget)->OnMouseExit();
  m_pDocView->UnlockUpdate();
  m_pDocView->UpdateDocView();
  return bRet;
}
FX_BOOL CXFA_FFWidgetHandler::OnLButtonDown(IXFA_Widget* hWidget,
                                            FX_DWORD dwFlags,
                                            FX_FLOAT fx,
                                            FX_FLOAT fy) {
  m_pDocView->LockUpdate();
  static_cast<CXFA_FFWidget*>(hWidget)->Rotate2Normal(fx, fy);
  FX_BOOL bRet =
      static_cast<CXFA_FFWidget*>(hWidget)->OnLButtonDown(dwFlags, fx, fy);
  if (bRet && m_pDocView->SetFocus(hWidget)) {
    ((CXFA_FFDoc*)m_pDocView->GetDoc())
        ->GetDocProvider()
        ->SetFocusWidget(m_pDocView->GetDoc(), (IXFA_Widget*)hWidget);
  }
  m_pDocView->UnlockUpdate();
  m_pDocView->UpdateDocView();
  return bRet;
}
FX_BOOL CXFA_FFWidgetHandler::OnLButtonUp(IXFA_Widget* hWidget,
                                          FX_DWORD dwFlags,
                                          FX_FLOAT fx,
                                          FX_FLOAT fy) {
  m_pDocView->LockUpdate();
  static_cast<CXFA_FFWidget*>(hWidget)->Rotate2Normal(fx, fy);
  m_pDocView->m_bLayoutEvent = TRUE;
  FX_BOOL bRet =
      static_cast<CXFA_FFWidget*>(hWidget)->OnLButtonUp(dwFlags, fx, fy);
  m_pDocView->UnlockUpdate();
  m_pDocView->UpdateDocView();
  return bRet;
}
FX_BOOL CXFA_FFWidgetHandler::OnLButtonDblClk(IXFA_Widget* hWidget,
                                              FX_DWORD dwFlags,
                                              FX_FLOAT fx,
                                              FX_FLOAT fy) {
  static_cast<CXFA_FFWidget*>(hWidget)->Rotate2Normal(fx, fy);
  FX_BOOL bRet =
      static_cast<CXFA_FFWidget*>(hWidget)->OnLButtonDblClk(dwFlags, fx, fy);
  m_pDocView->RunInvalidate();
  return bRet;
}
FX_BOOL CXFA_FFWidgetHandler::OnMouseMove(IXFA_Widget* hWidget,
                                          FX_DWORD dwFlags,
                                          FX_FLOAT fx,
                                          FX_FLOAT fy) {
  static_cast<CXFA_FFWidget*>(hWidget)->Rotate2Normal(fx, fy);
  FX_BOOL bRet =
      static_cast<CXFA_FFWidget*>(hWidget)->OnMouseMove(dwFlags, fx, fy);
  m_pDocView->RunInvalidate();
  return bRet;
}
FX_BOOL CXFA_FFWidgetHandler::OnMouseWheel(IXFA_Widget* hWidget,
                                           FX_DWORD dwFlags,
                                           int16_t zDelta,
                                           FX_FLOAT fx,
                                           FX_FLOAT fy) {
  static_cast<CXFA_FFWidget*>(hWidget)->Rotate2Normal(fx, fy);
  FX_BOOL bRet = static_cast<CXFA_FFWidget*>(hWidget)
                     ->OnMouseWheel(dwFlags, zDelta, fx, fy);
  m_pDocView->RunInvalidate();
  return bRet;
}
FX_BOOL CXFA_FFWidgetHandler::OnRButtonDown(IXFA_Widget* hWidget,
                                            FX_DWORD dwFlags,
                                            FX_FLOAT fx,
                                            FX_FLOAT fy) {
  static_cast<CXFA_FFWidget*>(hWidget)->Rotate2Normal(fx, fy);
  FX_BOOL bRet =
      static_cast<CXFA_FFWidget*>(hWidget)->OnRButtonDown(dwFlags, fx, fy);
  if (bRet && m_pDocView->SetFocus(hWidget)) {
    ((CXFA_FFDoc*)m_pDocView->GetDoc())
        ->GetDocProvider()
        ->SetFocusWidget(m_pDocView->GetDoc(), (IXFA_Widget*)hWidget);
  }
  m_pDocView->RunInvalidate();
  return bRet;
}
FX_BOOL CXFA_FFWidgetHandler::OnRButtonUp(IXFA_Widget* hWidget,
                                          FX_DWORD dwFlags,
                                          FX_FLOAT fx,
                                          FX_FLOAT fy) {
  static_cast<CXFA_FFWidget*>(hWidget)->Rotate2Normal(fx, fy);
  FX_BOOL bRet =
      static_cast<CXFA_FFWidget*>(hWidget)->OnRButtonUp(dwFlags, fx, fy);
  m_pDocView->RunInvalidate();
  return bRet;
}
FX_BOOL CXFA_FFWidgetHandler::OnRButtonDblClk(IXFA_Widget* hWidget,
                                              FX_DWORD dwFlags,
                                              FX_FLOAT fx,
                                              FX_FLOAT fy) {
  static_cast<CXFA_FFWidget*>(hWidget)->Rotate2Normal(fx, fy);
  FX_BOOL bRet =
      static_cast<CXFA_FFWidget*>(hWidget)->OnRButtonDblClk(dwFlags, fx, fy);
  m_pDocView->RunInvalidate();
  return bRet;
}
FX_BOOL CXFA_FFWidgetHandler::OnKeyDown(IXFA_Widget* hWidget,
                                        FX_DWORD dwKeyCode,
                                        FX_DWORD dwFlags) {
  FX_BOOL bRet =
      static_cast<CXFA_FFWidget*>(hWidget)->OnKeyDown(dwKeyCode, dwFlags);
  m_pDocView->RunInvalidate();
  m_pDocView->UpdateDocView();
  return bRet;
}
FX_BOOL CXFA_FFWidgetHandler::OnKeyUp(IXFA_Widget* hWidget,
                                      FX_DWORD dwKeyCode,
                                      FX_DWORD dwFlags) {
  FX_BOOL bRet =
      static_cast<CXFA_FFWidget*>(hWidget)->OnKeyUp(dwKeyCode, dwFlags);
  m_pDocView->RunInvalidate();
  return bRet;
}
FX_BOOL CXFA_FFWidgetHandler::OnChar(IXFA_Widget* hWidget,
                                     FX_DWORD dwChar,
                                     FX_DWORD dwFlags) {
  FX_BOOL bRet = static_cast<CXFA_FFWidget*>(hWidget)->OnChar(dwChar, dwFlags);
  m_pDocView->RunInvalidate();
  return bRet;
}
FX_DWORD CXFA_FFWidgetHandler::OnHitTest(IXFA_Widget* hWidget,
                                         FX_FLOAT fx,
                                         FX_FLOAT fy) {
  if (!(static_cast<CXFA_FFWidget*>(hWidget)->GetStatus() &
        XFA_WIDGETSTATUS_Visible)) {
    return FWL_WGTHITTEST_Unknown;
  }
  static_cast<CXFA_FFWidget*>(hWidget)->Rotate2Normal(fx, fy);
  return static_cast<CXFA_FFWidget*>(hWidget)->OnHitTest(fx, fy);
}
FX_BOOL CXFA_FFWidgetHandler::OnSetCursor(IXFA_Widget* hWidget,
                                          FX_FLOAT fx,
                                          FX_FLOAT fy) {
  static_cast<CXFA_FFWidget*>(hWidget)->Rotate2Normal(fx, fy);
  return static_cast<CXFA_FFWidget*>(hWidget)->OnSetCursor(fx, fy);
}
void CXFA_FFWidgetHandler::RenderWidget(IXFA_Widget* hWidget,
                                        CFX_Graphics* pGS,
                                        CFX_Matrix* pMatrix,
                                        FX_BOOL bHighlight) {
  static_cast<CXFA_FFWidget*>(hWidget)->RenderWidget(
      pGS, pMatrix, bHighlight ? XFA_WIDGETSTATUS_Highlight : 0, 0);
}
FX_BOOL CXFA_FFWidgetHandler::HasEvent(CXFA_WidgetAcc* pWidgetAcc,
                                       XFA_EVENTTYPE eEventType) {
  if (!pWidgetAcc || eEventType == XFA_EVENT_Unknown) {
    return FALSE;
  }
  if (pWidgetAcc->GetClassID() == XFA_ELEMENT_Draw) {
    return FALSE;
  }
  switch (eEventType) {
    case XFA_EVENT_Calculate: {
      CXFA_Calculate calc = pWidgetAcc->GetCalculate();
      if (!calc) {
        return FALSE;
      }
      if (calc.GetScript()) {
        return TRUE;
      }
      return FALSE;
    }
    case XFA_EVENT_Validate: {
      CXFA_Validate val = pWidgetAcc->GetValidate();
      if (!val) {
        return FALSE;
      }
      if (val.GetScript()) {
        return TRUE;
      }
      return FALSE;
    }
    default:
      break;
  }
  CXFA_NodeArray eventArray;
  return pWidgetAcc->GetEventByActivity(gs_EventActivity[eEventType],
                                        eventArray);
}
int32_t CXFA_FFWidgetHandler::ProcessEvent(CXFA_WidgetAcc* pWidgetAcc,
                                           CXFA_EventParam* pParam) {
  if (!pParam || pParam->m_eType == XFA_EVENT_Unknown) {
    return XFA_EVENTERROR_NotExist;
  }
  if (!pWidgetAcc || pWidgetAcc->GetClassID() == XFA_ELEMENT_Draw) {
    return XFA_EVENTERROR_NotExist;
  }
  switch (pParam->m_eType) {
    case XFA_EVENT_Calculate:
      return pWidgetAcc->ProcessCalculate();
    case XFA_EVENT_Validate:
      if (((CXFA_FFDoc*)m_pDocView->GetDoc())
              ->GetDocProvider()
              ->IsValidationsEnabled(m_pDocView->GetDoc())) {
        return pWidgetAcc->ProcessValidate();
      }
      return XFA_EVENTERROR_Disabled;
    case XFA_EVENT_InitCalculate: {
      CXFA_Calculate calc = pWidgetAcc->GetCalculate();
      if (!calc) {
        return XFA_EVENTERROR_NotExist;
      }
      if (pWidgetAcc->GetNode()->HasFlag(XFA_NODEFLAG_UserInteractive)) {
        return XFA_EVENTERROR_Disabled;
      }
      CXFA_Script script = calc.GetScript();
      return pWidgetAcc->ExecuteScript(script, pParam);
    }
    default:
      break;
  }
  int32_t iRet =
      pWidgetAcc->ProcessEvent(gs_EventActivity[pParam->m_eType], pParam);
  return iRet;
}
IXFA_Widget* CXFA_FFWidgetHandler::CreateWidget(IXFA_Widget* hParent,
                                                XFA_WIDGETTYPE eType,
                                                IXFA_Widget* hBefore) {
  CXFA_Node* pParentFormItem =
      hParent ? static_cast<CXFA_FFWidget*>(hParent)->GetDataAcc()->GetNode()
              : NULL;
  CXFA_Node* pBeforeFormItem =
      hBefore ? static_cast<CXFA_FFWidget*>(hBefore)->GetDataAcc()->GetNode()
              : NULL;
  CXFA_Node* pNewFormItem =
      CreateWidgetFormItem(eType, pParentFormItem, pBeforeFormItem);
  if (pNewFormItem == NULL) {
    return NULL;
  }
  pNewFormItem->GetTemplateNode()->SetFlag(XFA_NODEFLAG_Initialized);
  pNewFormItem->SetFlag(XFA_NODEFLAG_Initialized);
  m_pDocView->RunLayout();
  CXFA_LayoutItem* pLayout =
      m_pDocView->GetXFALayout()->GetLayoutItem(pNewFormItem);
  return (IXFA_Widget*)pLayout;
}
CXFA_Node* CXFA_FFWidgetHandler::CreateWidgetFormItem(
    XFA_WIDGETTYPE eType,
    CXFA_Node* pParent,
    CXFA_Node* pBefore) const {
  switch (eType) {
    case XFA_WIDGETTYPE_Barcode:
      return NULL;
    case XFA_WIDGETTYPE_PushButton:
      return CreatePushButton(pParent, pBefore);
    case XFA_WIDGETTYPE_CheckButton:
      return CreateCheckButton(pParent, pBefore);
    case XFA_WIDGETTYPE_ExcludeGroup:
      return CreateExclGroup(pParent, pBefore);
    case XFA_WIDGETTYPE_RadioButton:
      return CreateRadioButton(pParent, pBefore);
    case XFA_WIDGETTYPE_Arc:
      return CreateArc(pParent, pBefore);
    case XFA_WIDGETTYPE_Rectangle:
      return CreateRectangle(pParent, pBefore);
    case XFA_WIDGETTYPE_Image:
      return CreateImage(pParent, pBefore);
    case XFA_WIDGETTYPE_Line:
      return CreateLine(pParent, pBefore);
    case XFA_WIDGETTYPE_Text:
      return CreateText(pParent, pBefore);
    case XFA_WIDGETTYPE_DatetimeEdit:
      return CreateDatetimeEdit(pParent, pBefore);
    case XFA_WIDGETTYPE_DecimalField:
      return CreateDecimalField(pParent, pBefore);
    case XFA_WIDGETTYPE_NumericField:
      return CreateNumericField(pParent, pBefore);
    case XFA_WIDGETTYPE_Signature:
      return CreateSignature(pParent, pBefore);
    case XFA_WIDGETTYPE_TextEdit:
      return CreateTextEdit(pParent, pBefore);
    case XFA_WIDGETTYPE_DropdownList:
      return CreateDropdownList(pParent, pBefore);
    case XFA_WIDGETTYPE_ListBox:
      return CreateListBox(pParent, pBefore);
    case XFA_WIDGETTYPE_ImageField:
      return CreateImageField(pParent, pBefore);
    case XFA_WIDGETTYPE_PasswordEdit:
      return CreatePasswordEdit(pParent, pBefore);
    case XFA_WIDGETTYPE_Subform:
      return CreateSubform(pParent, pBefore);
    default:
      break;
  }
  return NULL;
}
CXFA_Node* CXFA_FFWidgetHandler::CreatePushButton(CXFA_Node* pParent,
                                                  CXFA_Node* pBefore) const {
  CXFA_Node* pField = CreateField(XFA_ELEMENT_Button, pParent, pBefore);
  CXFA_Node* pCaption = CreateCopyNode(XFA_ELEMENT_Caption, pField);
  CXFA_Node* pValue = CreateCopyNode(XFA_ELEMENT_Value, pCaption);
  CXFA_Node* pText = CreateCopyNode(XFA_ELEMENT_Text, pValue);
  pText->SetContent(FX_WSTRC(L"Button"), FX_WSTRC(L"Button"), FALSE);
  CXFA_Node* pPara = CreateCopyNode(XFA_ELEMENT_Para, pCaption);
  pPara->SetEnum(XFA_ATTRIBUTE_VAlign, XFA_ATTRIBUTEENUM_Middle, FALSE);
  pPara->SetEnum(XFA_ATTRIBUTE_HAlign, XFA_ATTRIBUTEENUM_Center, FALSE);
  CreateFontNode(pCaption);
  CXFA_Node* pBorder = CreateCopyNode(XFA_ELEMENT_Border, pField);
  pBorder->SetEnum(XFA_ATTRIBUTE_Hand, XFA_ATTRIBUTEENUM_Right, FALSE);
  CXFA_Node* pEdge = CreateCopyNode(XFA_ELEMENT_Edge, pBorder);
  pEdge->SetEnum(XFA_ATTRIBUTE_Stroke, XFA_ATTRIBUTEENUM_Raised, FALSE);
  CXFA_Node* pFill = CreateCopyNode(XFA_ELEMENT_Fill, pBorder);
  CXFA_Node* pColor = CreateCopyNode(XFA_ELEMENT_Color, pFill);
  pColor->SetCData(XFA_ATTRIBUTE_Value, FX_WSTRC(L"212, 208, 200"), FALSE);
  CXFA_Node* pBind = CreateCopyNode(XFA_ELEMENT_Bind, pField);
  pBind->SetEnum(XFA_ATTRIBUTE_Match, XFA_ATTRIBUTEENUM_None);
  return pField;
}
CXFA_Node* CXFA_FFWidgetHandler::CreateCheckButton(CXFA_Node* pParent,
                                                   CXFA_Node* pBefore) const {
  CXFA_Node* pField = CreateField(XFA_ELEMENT_CheckButton, pParent, pBefore);
  return pField;
}
CXFA_Node* CXFA_FFWidgetHandler::CreateExclGroup(CXFA_Node* pParent,
                                                 CXFA_Node* pBefore) const {
  return CreateFormItem(XFA_ELEMENT_ExclGroup, pParent, pBefore);
}
CXFA_Node* CXFA_FFWidgetHandler::CreateRadioButton(CXFA_Node* pParent,
                                                   CXFA_Node* pBefore) const {
  CXFA_Node* pField = CreateField(XFA_ELEMENT_CheckButton, pParent, pBefore);
  CXFA_Node* pUi = pField->GetFirstChildByClass(XFA_ELEMENT_Ui);
  CXFA_Node* pWidget = pUi->GetFirstChildByClass(XFA_ELEMENT_CheckButton);
  pWidget->SetEnum(XFA_ATTRIBUTE_Shape, XFA_ATTRIBUTEENUM_Round);
  return pField;
}
CXFA_Node* CXFA_FFWidgetHandler::CreateDatetimeEdit(CXFA_Node* pParent,
                                                    CXFA_Node* pBefore) const {
  CXFA_Node* pField = CreateField(XFA_ELEMENT_DateTimeEdit, pParent, pBefore);
  CreateValueNode(XFA_ELEMENT_Date, pField);
  return pField;
}
CXFA_Node* CXFA_FFWidgetHandler::CreateDecimalField(CXFA_Node* pParent,
                                                    CXFA_Node* pBefore) const {
  CXFA_Node* pField = CreateNumericField(pParent, pBefore);
  CreateValueNode(XFA_ELEMENT_Decimal, pField);
  return pField;
}
CXFA_Node* CXFA_FFWidgetHandler::CreateNumericField(CXFA_Node* pParent,
                                                    CXFA_Node* pBefore) const {
  CXFA_Node* pField = CreateField(XFA_ELEMENT_NumericEdit, pParent, pBefore);
  return pField;
}
CXFA_Node* CXFA_FFWidgetHandler::CreateSignature(CXFA_Node* pParent,
                                                 CXFA_Node* pBefore) const {
  CXFA_Node* pField = CreateField(XFA_ELEMENT_Signature, pParent, pBefore);
  return pField;
}
CXFA_Node* CXFA_FFWidgetHandler::CreateTextEdit(CXFA_Node* pParent,
                                                CXFA_Node* pBefore) const {
  CXFA_Node* pField = CreateField(XFA_ELEMENT_TextEdit, pParent, pBefore);
  return pField;
}
CXFA_Node* CXFA_FFWidgetHandler::CreateDropdownList(CXFA_Node* pParent,
                                                    CXFA_Node* pBefore) const {
  CXFA_Node* pField = CreateField(XFA_ELEMENT_ChoiceList, pParent, pBefore);
  return pField;
}
CXFA_Node* CXFA_FFWidgetHandler::CreateListBox(CXFA_Node* pParent,
                                               CXFA_Node* pBefore) const {
  CXFA_Node* pField = CreateDropdownList(pParent, pBefore);
  CXFA_Node* pUi = pField->GetNodeItem(XFA_NODEITEM_FirstChild);
  CXFA_Node* pListBox = pUi->GetNodeItem(XFA_NODEITEM_FirstChild);
  pListBox->SetEnum(XFA_ATTRIBUTE_Open, XFA_ATTRIBUTEENUM_Always);
  pListBox->SetEnum(XFA_ATTRIBUTE_CommitOn, XFA_ATTRIBUTEENUM_Exit);
  return pField;
}
CXFA_Node* CXFA_FFWidgetHandler::CreateImageField(CXFA_Node* pParent,
                                                  CXFA_Node* pBefore) const {
  CXFA_Node* pField = CreateField(XFA_ELEMENT_ImageEdit, pParent, pBefore);
  return pField;
}
CXFA_Node* CXFA_FFWidgetHandler::CreatePasswordEdit(CXFA_Node* pParent,
                                                    CXFA_Node* pBefore) const {
  CXFA_Node* pField = CreateField(XFA_ELEMENT_PasswordEdit, pParent, pBefore);
  CXFA_Node* pBind = CreateCopyNode(XFA_ELEMENT_Bind, pField);
  pBind->SetEnum(XFA_ATTRIBUTE_Match, XFA_ATTRIBUTEENUM_None, FALSE);
  return pField;
}
CXFA_Node* CXFA_FFWidgetHandler::CreateField(XFA_ELEMENT eElement,
                                             CXFA_Node* pParent,
                                             CXFA_Node* pBefore) const {
  CXFA_Node* pField = CreateFormItem(XFA_ELEMENT_Field, pParent, pBefore);
  CreateCopyNode(eElement, CreateCopyNode(XFA_ELEMENT_Ui, pField));
  CreateFontNode(pField);
  return pField;
}
CXFA_Node* CXFA_FFWidgetHandler::CreateArc(CXFA_Node* pParent,
                                           CXFA_Node* pBefore) const {
  return CreateDraw(XFA_ELEMENT_Arc, pParent, pBefore);
}
CXFA_Node* CXFA_FFWidgetHandler::CreateRectangle(CXFA_Node* pParent,
                                                 CXFA_Node* pBefore) const {
  return CreateDraw(XFA_ELEMENT_Rectangle, pParent, pBefore);
}
CXFA_Node* CXFA_FFWidgetHandler::CreateImage(CXFA_Node* pParent,
                                             CXFA_Node* pBefore) const {
  CXFA_Node* pField = CreateDraw(XFA_ELEMENT_Image, pParent, pBefore);
  CreateCopyNode(XFA_ELEMENT_ImageEdit, CreateCopyNode(XFA_ELEMENT_Ui, pField));
  return pField;
}
CXFA_Node* CXFA_FFWidgetHandler::CreateLine(CXFA_Node* pParent,
                                            CXFA_Node* pBefore) const {
  CXFA_Node* pField = CreateDraw(XFA_ELEMENT_Line, pParent, pBefore);
  return pField;
}
CXFA_Node* CXFA_FFWidgetHandler::CreateText(CXFA_Node* pParent,
                                            CXFA_Node* pBefore) const {
  CXFA_Node* pField = CreateDraw(XFA_ELEMENT_Text, pParent, pBefore);
  CreateCopyNode(XFA_ELEMENT_TextEdit, CreateCopyNode(XFA_ELEMENT_Ui, pField));
  CreateFontNode(pField);
  return pField;
}
CXFA_Node* CXFA_FFWidgetHandler::CreateDraw(XFA_ELEMENT eElement,
                                            CXFA_Node* pParent,
                                            CXFA_Node* pBefore) const {
  CXFA_Node* pDraw = CreateFormItem(XFA_ELEMENT_Draw, pParent, pBefore);
  CreateValueNode(eElement, pDraw);
  return pDraw;
}
CXFA_Node* CXFA_FFWidgetHandler::CreateSubform(CXFA_Node* pParent,
                                               CXFA_Node* pBefore) const {
  CXFA_Node* pSubform = CreateFormItem(XFA_ELEMENT_Subform, pParent, pBefore);
  return pSubform;
}
CXFA_Node* CXFA_FFWidgetHandler::CreateFormItem(XFA_ELEMENT eElement,
                                                CXFA_Node* pParent,
                                                CXFA_Node* pBefore) const {
  CXFA_Node* pTemplateParent =
      pParent != NULL ? pParent->GetTemplateNode() : NULL;
  CXFA_Node* pNewFormItem = pTemplateParent->CloneTemplateToForm(FALSE);
  if (pParent != NULL) {
    pParent->InsertChild(pNewFormItem, pBefore);
  }
  return pNewFormItem;
}
CXFA_Node* CXFA_FFWidgetHandler::CreateCopyNode(XFA_ELEMENT eElement,
                                                CXFA_Node* pParent,
                                                CXFA_Node* pBefore) const {
  CXFA_Node* pTemplateParent =
      pParent != NULL ? pParent->GetTemplateNode() : NULL;
  CXFA_Node* pNewNode =
      CreateTemplateNode(eElement, pTemplateParent,
                         pBefore ? pBefore->GetTemplateNode() : NULL)
          ->Clone(FALSE);
  if (pParent != NULL) {
    pParent->InsertChild(pNewNode, pBefore);
  }
  return pNewNode;
}
CXFA_Node* CXFA_FFWidgetHandler::CreateTemplateNode(XFA_ELEMENT eElement,
                                                    CXFA_Node* pParent,
                                                    CXFA_Node* pBefore) const {
  CXFA_Document* pXFADoc = GetXFADoc();
  CXFA_Node* pNewTemplateNode = pXFADoc->GetParser()->GetFactory()->CreateNode(
      XFA_XDPPACKET_Template, eElement);
  if (pParent != NULL) {
    pParent->InsertChild(pNewTemplateNode, pBefore);
  }
  return pNewTemplateNode;
}
CXFA_Node* CXFA_FFWidgetHandler::CreateFontNode(CXFA_Node* pParent) const {
  CXFA_Node* pFont = CreateCopyNode(XFA_ELEMENT_Font, pParent);
  pFont->SetCData(XFA_ATTRIBUTE_Typeface, FX_WSTRC(L"Myriad Pro"), FALSE);
  return pFont;
}
CXFA_Node* CXFA_FFWidgetHandler::CreateMarginNode(CXFA_Node* pParent,
                                                  FX_DWORD dwFlags,
                                                  FX_FLOAT fInsets[4]) const {
  CXFA_Node* pMargin = CreateCopyNode(XFA_ELEMENT_Margin, pParent);
  if (dwFlags & 0x01) {
    pMargin->SetMeasure(XFA_ATTRIBUTE_LeftInset,
                        CXFA_Measurement(fInsets[0], XFA_UNIT_Pt), FALSE);
  }
  if (dwFlags & 0x02) {
    pMargin->SetMeasure(XFA_ATTRIBUTE_TopInset,
                        CXFA_Measurement(fInsets[1], XFA_UNIT_Pt), FALSE);
  }
  if (dwFlags & 0x04) {
    pMargin->SetMeasure(XFA_ATTRIBUTE_RightInset,
                        CXFA_Measurement(fInsets[2], XFA_UNIT_Pt), FALSE);
  }
  if (dwFlags & 0x08) {
    pMargin->SetMeasure(XFA_ATTRIBUTE_BottomInset,
                        CXFA_Measurement(fInsets[3], XFA_UNIT_Pt), FALSE);
  }
  return pMargin;
}
CXFA_Node* CXFA_FFWidgetHandler::CreateValueNode(XFA_ELEMENT eValue,
                                                 CXFA_Node* pParent) const {
  CXFA_Node* pValue = CreateCopyNode(XFA_ELEMENT_Value, pParent);
  CreateCopyNode(eValue, pValue);
  return pValue;
}
IXFA_ObjFactory* CXFA_FFWidgetHandler::GetObjFactory() const {
  return GetXFADoc()->GetParser()->GetFactory();
}
CXFA_Document* CXFA_FFWidgetHandler::GetXFADoc() const {
  return ((CXFA_FFDoc*)(m_pDocView->GetDoc()))->GetXFADoc();
}
CXFA_FFMenuHandler::CXFA_FFMenuHandler() {}
CXFA_FFMenuHandler::~CXFA_FFMenuHandler() {}
FX_BOOL CXFA_FFMenuHandler::CanCopy(IXFA_Widget* hWidget) {
  return static_cast<CXFA_FFWidget*>(hWidget)->CanCopy();
}
FX_BOOL CXFA_FFMenuHandler::CanCut(IXFA_Widget* hWidget) {
  return static_cast<CXFA_FFWidget*>(hWidget)->CanCut();
}
FX_BOOL CXFA_FFMenuHandler::CanPaste(IXFA_Widget* hWidget) {
  return static_cast<CXFA_FFWidget*>(hWidget)->CanPaste();
}
FX_BOOL CXFA_FFMenuHandler::CanSelectAll(IXFA_Widget* hWidget) {
  return static_cast<CXFA_FFWidget*>(hWidget)->CanSelectAll();
}
FX_BOOL CXFA_FFMenuHandler::CanDelete(IXFA_Widget* hWidget) {
  return static_cast<CXFA_FFWidget*>(hWidget)->CanDelete();
}
FX_BOOL CXFA_FFMenuHandler::CanDeSelect(IXFA_Widget* hWidget) {
  return static_cast<CXFA_FFWidget*>(hWidget)->CanDeSelect();
}
FX_BOOL CXFA_FFMenuHandler::Copy(IXFA_Widget* hWidget, CFX_WideString& wsText) {
  return static_cast<CXFA_FFWidget*>(hWidget)->Copy(wsText);
}
FX_BOOL CXFA_FFMenuHandler::Cut(IXFA_Widget* hWidget, CFX_WideString& wsText) {
  return static_cast<CXFA_FFWidget*>(hWidget)->Cut(wsText);
}
FX_BOOL CXFA_FFMenuHandler::Paste(IXFA_Widget* hWidget,
                                  const CFX_WideString& wsText) {
  return static_cast<CXFA_FFWidget*>(hWidget)->Paste(wsText);
}
FX_BOOL CXFA_FFMenuHandler::SelectAll(IXFA_Widget* hWidget) {
  return static_cast<CXFA_FFWidget*>(hWidget)->SelectAll();
}
FX_BOOL CXFA_FFMenuHandler::Delete(IXFA_Widget* hWidget) {
  return static_cast<CXFA_FFWidget*>(hWidget)->Delete();
}
FX_BOOL CXFA_FFMenuHandler::DeSelect(IXFA_Widget* hWidget) {
  return static_cast<CXFA_FFWidget*>(hWidget)->DeSelect();
}
FX_BOOL CXFA_FFMenuHandler::CanUndo(IXFA_Widget* hWidget) {
  return static_cast<CXFA_FFWidget*>(hWidget)->CanUndo();
}
FX_BOOL CXFA_FFMenuHandler::CanRedo(IXFA_Widget* hWidget) {
  return static_cast<CXFA_FFWidget*>(hWidget)->CanRedo();
}
FX_BOOL CXFA_FFMenuHandler::Undo(IXFA_Widget* hWidget) {
  return static_cast<CXFA_FFWidget*>(hWidget)->Undo();
}
FX_BOOL CXFA_FFMenuHandler::Redo(IXFA_Widget* hWidget) {
  return static_cast<CXFA_FFWidget*>(hWidget)->Redo();
}
#define FX_EDIT_ISLATINWORD(u)                                     \
  (u == 0x2D || (u <= 0x005A && u >= 0x0041) ||                    \
   (u <= 0x007A && u >= 0x0061) || (u <= 0x02AF && u >= 0x00C0) || \
   u == 0x0027)
FX_BOOL CXFA_FFMenuHandler::GetSuggestWords(IXFA_Widget* hWidget,
                                            CFX_PointF pointf,
                                            CFX_ByteStringArray& sSuggest) {
  return static_cast<CXFA_FFWidget*>(hWidget)
      ->GetSuggestWords(pointf, sSuggest);
}
FX_BOOL CXFA_FFMenuHandler::ReplaceSpellCheckWord(
    IXFA_Widget* hWidget,
    CFX_PointF pointf,
    const CFX_ByteStringC& bsReplace) {
  return static_cast<CXFA_FFWidget*>(hWidget)
      ->ReplaceSpellCheckWord(pointf, bsReplace);
}
