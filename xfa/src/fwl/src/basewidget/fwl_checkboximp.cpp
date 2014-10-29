// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../foxitlib.h"
#include "../core/include/fwl_targetimp.h"
#include "../core/include/fwl_noteimp.h"
#include "../core/include/fwl_widgetimp.h"
#include "../core/include/fwl_widgetmgrimp.h"
#include "include/fwl_checkboximp.h"
#define FWL_CKB_CaptionMargin	5
IFWL_CheckBox* IFWL_CheckBox::Create()
{
    return new IFWL_CheckBox;
}
IFWL_CheckBox::IFWL_CheckBox()
{
    m_pData = NULL;
}
IFWL_CheckBox::~IFWL_CheckBox()
{
    if (m_pData) {
        delete (CFWL_CheckBoxImp*)m_pData;
        m_pData = NULL;
    }
}
FWL_ERR IFWL_CheckBox::Initialize(IFWL_Widget *pOuter)
{
    m_pData = FX_NEW CFWL_CheckBoxImp(pOuter);
    ((CFWL_CheckBoxImp*)m_pData)->SetInterface(this);
    return ((CFWL_CheckBoxImp*)m_pData)->Initialize();
}
FWL_ERR	IFWL_CheckBox::Initialize(const CFWL_WidgetImpProperties &properties, IFWL_Widget *pOuter)
{
    m_pData = FX_NEW CFWL_CheckBoxImp(properties, pOuter);
    ((CFWL_CheckBoxImp*)m_pData)->SetInterface(this);
    return ((CFWL_CheckBoxImp*)m_pData)->Initialize();
}
FX_INT32 IFWL_CheckBox::GetCheckState()
{
    return ((CFWL_CheckBoxImp*)m_pData)->GetCheckState();
}
FWL_ERR IFWL_CheckBox::SetCheckState(FX_INT32 iCheck)
{
    return ((CFWL_CheckBoxImp*)m_pData)->SetCheckState(iCheck);
}
IFWL_RadioButton* IFWL_RadioButton::Create()
{
    return new IFWL_RadioButton;
}
IFWL_RadioButton::IFWL_RadioButton()
{
    m_pData = NULL;
}
IFWL_RadioButton::~IFWL_RadioButton()
{
    if (m_pData) {
        delete (CFWL_CheckBoxImp*)m_pData;
        m_pData = NULL;
    }
}
FWL_ERR IFWL_RadioButton::Initialize(IFWL_Widget *pOuter)
{
    m_pData = FX_NEW CFWL_CheckBoxImp(pOuter);
    ((CFWL_CheckBoxImp*)m_pData)->SetInterface(this);
    return ((CFWL_CheckBoxImp*)m_pData)->Initialize();
}
FWL_ERR	IFWL_RadioButton::Initialize(const CFWL_WidgetImpProperties &properties, IFWL_Widget *pOuter)
{
    m_pData = FX_NEW CFWL_CheckBoxImp(properties, pOuter);
    ((CFWL_CheckBoxImp*)m_pData)->SetInterface(this);
    return ((CFWL_CheckBoxImp*)m_pData)->Initialize();
}
CFWL_CheckBoxImp::CFWL_CheckBoxImp(IFWL_Widget *pOuter )
    : CFWL_WidgetImp(pOuter)
    , m_dwTTOStyles(FDE_TTOSTYLE_SingleLine)
    , m_iTTOAlign(FDE_TTOALIGNMENT_Center)
    , m_bBtnDown(FALSE)
{
    m_rtClient.Reset();
    m_rtBox.Reset();
    m_rtCaption.Reset();
    m_rtFocus.Reset();
}
CFWL_CheckBoxImp::CFWL_CheckBoxImp(const CFWL_WidgetImpProperties &properties, IFWL_Widget *pOuter )
    : CFWL_WidgetImp(properties, pOuter)
    , m_dwTTOStyles(FDE_TTOSTYLE_SingleLine)
    , m_iTTOAlign(FDE_TTOALIGNMENT_Center)
    , m_bBtnDown(FALSE)
{
    m_rtClient.Reset();
    m_rtBox.Reset();
    m_rtCaption.Reset();
    m_rtFocus.Reset();
}
CFWL_CheckBoxImp::~CFWL_CheckBoxImp()
{
}
FWL_ERR CFWL_CheckBoxImp::GetClassName(CFX_WideString &wsClass) const
{
    wsClass = (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_CKB_RadioButton) ? FWL_CLASS_RadioButton : FWL_CLASS_CheckBox;
    return FWL_ERR_Succeeded;
}
FX_DWORD CFWL_CheckBoxImp::GetClassID() const
{
    return m_pProperties->m_dwStyleExes & FWL_STYLEEXT_CKB_RadioButton ? FWL_CLASSHASH_RadioButton : FWL_CLASSHASH_CheckBox;
}
FWL_ERR	CFWL_CheckBoxImp::Initialize()
{
    _FWL_ERR_CHECK_RETURN_VALUE_IF_FAIL(CFWL_WidgetImp::Initialize(), FWL_ERR_Indefinite);
    m_pDelegate = (IFWL_WidgetDelegate*) FX_NEW CFWL_CheckBoxImpDelegate(this);
    return FWL_ERR_Succeeded;
}
FWL_ERR	CFWL_CheckBoxImp::Finalize()
{
    if ( m_pDelegate) {
        delete (CFWL_CheckBoxImpDelegate*)m_pDelegate;
        m_pDelegate = NULL;
    }
    return CFWL_WidgetImp::Finalize();
}
FWL_ERR CFWL_CheckBoxImp::GetWidgetRect(CFX_RectF &rect, FX_BOOL bAutoSize )
{
    if (bAutoSize) {
        rect.Set(0, 0, 0, 0);
        if (!m_pProperties->m_pThemeProvider) {
            m_pProperties->m_pThemeProvider = GetAvailableTheme();
        }
        _FWL_RETURN_VALUE_IF_FAIL(m_pProperties->m_pThemeProvider, FWL_ERR_Indefinite);
        _FWL_RETURN_VALUE_IF_FAIL(m_pProperties->m_pDataProvider, FWL_ERR_Indefinite);
        CFX_WideString wsCaption;
        m_pProperties->m_pDataProvider->GetCaption(m_pInterface, wsCaption);
        if (wsCaption.GetLength() > 0) {
            CFX_SizeF sz = CalcTextSize(wsCaption,
                                        m_pProperties->m_pThemeProvider,
                                        m_pProperties->m_dwStyleExes & FWL_STYLEEXT_CKB_MultiLine);
            rect.Set(0, 0, sz.x, sz.y);
        }
        rect.Inflate(FWL_CKB_CaptionMargin, FWL_CKB_CaptionMargin);
        IFWL_CheckBoxDP *pData = (IFWL_CheckBoxDP*)m_pProperties->m_pDataProvider;
        FX_FLOAT fCheckBox = pData->GetBoxSize(m_pInterface);
        rect.width += fCheckBox;
        if (rect.height < fCheckBox) {
            rect.height = fCheckBox;
        }
        CFWL_WidgetImp::GetWidgetRect(rect, TRUE);
    } else {
        rect = m_pProperties->m_rtWidget;
    }
    return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_CheckBoxImp::Update()
{
    if (IsLocked()) {
        return FWL_ERR_Indefinite;
    }
    if (!m_pProperties->m_pThemeProvider) {
        m_pProperties->m_pThemeProvider = GetAvailableTheme();
    }
    UpdateTextOutStyles();
    Layout();
    return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_CheckBoxImp::DrawWidget(CFX_Graphics *pGraphics, const CFX_Matrix *pMatrix )
{
    _FWL_RETURN_VALUE_IF_FAIL(pGraphics, FWL_ERR_Indefinite);
    _FWL_RETURN_VALUE_IF_FAIL(m_pProperties->m_pThemeProvider, FWL_ERR_Indefinite);
    IFWL_ThemeProvider *pTheme = m_pProperties->m_pThemeProvider;
    if (HasBorder()) {
        DrawBorder(pGraphics, FWL_PART_CKB_Border, m_pProperties->m_pThemeProvider, pMatrix);
    }
    if (HasEdge()) {
        DrawEdge(pGraphics, FWL_PART_CKB_Edge, pTheme, pMatrix);
    }
    FX_INT32 dwStates = GetPartStates();
    {
        CFWL_ThemeBackground param;
        param.m_pWidget = m_pInterface;
        param.m_iPart = FWL_PART_CKB_Background;
        param.m_dwStates = dwStates;
        param.m_pGraphics = pGraphics;
        if (pMatrix) {
            param.m_matrix.Concat(*pMatrix);
        }
        param.m_rtPart = m_rtClient;
        if (m_pProperties->m_dwStates & FWL_WGTSTATE_Focused) {
            param.m_pData = &m_rtFocus;
        }
        pTheme->DrawBackground(&param);
        param.m_iPart = FWL_PART_CKB_CheckBox;
        param.m_rtPart = m_rtBox;
        pTheme->DrawBackground(&param);
    }
    _FWL_RETURN_VALUE_IF_FAIL(m_pProperties->m_pDataProvider, FWL_ERR_Indefinite);
    {
        CFX_WideString wsCaption;
        m_pProperties->m_pDataProvider->GetCaption(m_pInterface, wsCaption);
        FX_INT32 iLen = wsCaption.GetLength();
        _FWL_RETURN_VALUE_IF_FAIL(iLen > 0, FWL_ERR_Indefinite);
        CFWL_ThemeText textParam;
        textParam.m_pWidget = m_pInterface;
        textParam.m_iPart = FWL_PART_CKB_Caption;
        textParam.m_dwStates = dwStates;
        textParam.m_pGraphics = pGraphics;
        if(pMatrix) {
            textParam.m_matrix.Concat(*pMatrix);
        }
        textParam.m_rtPart = m_rtCaption;
        textParam.m_wsText = wsCaption;
        textParam.m_dwTTOStyles = m_dwTTOStyles;
        textParam.m_iTTOAlign = m_iTTOAlign;
        pTheme->DrawText(&textParam);
    }
    return FWL_ERR_Succeeded;
}
FX_INT32 CFWL_CheckBoxImp::GetCheckState()
{
    if ((m_pProperties->m_dwStyleExes & FWL_STYLEEXT_CKB_3State)
            && ((m_pProperties->m_dwStates & FWL_STATE_CKB_CheckMask) == FWL_STATE_CKB_Neutral)) {
        return 2;
    }
    if ((m_pProperties->m_dwStates & FWL_STATE_CKB_CheckMask) == FWL_STATE_CKB_Checked) {
        return 1;
    }
    return 0;
}
FWL_ERR CFWL_CheckBoxImp::SetCheckState(FX_INT32 iCheck)
{
    m_pProperties->m_dwStates &= ~FWL_STATE_CKB_CheckMask;
    switch(iCheck) {
        case 0: {
                break;
            }
        case 1: {
                m_pProperties->m_dwStates |= FWL_STATE_CKB_Checked;
                break;
            }
        case 2: {
                if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_CKB_3State) {
                    m_pProperties->m_dwStates |= FWL_STATE_CKB_Neutral;
                }
                break;
            }
        default: {
            }
    }
    Repaint(&m_rtClient);
    return FWL_ERR_Succeeded;
}
void CFWL_CheckBoxImp::Layout()
{
    FX_INT32 width = FX_INT32(m_pProperties->m_rtWidget.width + 0.5f);
    FX_INT32 height = FX_INT32(m_pProperties->m_rtWidget.height + 0.5f);
    m_pProperties->m_rtWidget.width = (FX_FLOAT)width;
    m_pProperties->m_rtWidget.height = (FX_FLOAT)height;
    GetClientRect(m_rtClient);
    FX_FLOAT fBoxTop = m_rtClient.top;
    FX_FLOAT fBoxLeft = m_rtClient.left;
    FX_FLOAT fTextLeft = 0.0, fTextRight = 0.0;
    FX_FLOAT fClientRight = m_rtClient.right();
    FX_FLOAT fClientBottom = m_rtClient.bottom();
    _FWL_RETURN_IF_FAIL(m_pProperties->m_pDataProvider);
    IFWL_CheckBoxDP *pData = (IFWL_CheckBoxDP*)m_pProperties->m_pDataProvider;
    FX_FLOAT fCheckBox = pData->GetBoxSize(m_pInterface);
    switch (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_CKB_VLayoutMask) {
        case FWL_STYLEEXT_CKB_Top: {
                fBoxTop = m_rtClient.top;
                break;
            }
        case FWL_STYLEEXT_CKB_Bottom: {
                fBoxTop = fClientBottom - fCheckBox;
                break;
            }
        case FWL_STYLEEXT_CKB_VCenter:
        default: {
                fBoxTop = m_rtClient.top + (m_rtClient.height - fCheckBox) / 2;
                fBoxTop = FXSYS_floor(fBoxTop);
            }
    }
    if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_CKB_LeftText) {
        fBoxLeft = fClientRight - fCheckBox;
        fTextLeft = m_rtClient.left;
        fTextRight = fBoxLeft;
    } else {
        fTextLeft = fBoxLeft + fCheckBox;
        fTextRight = fClientRight;
    }
    m_rtBox.Set(fBoxLeft, fBoxTop, fCheckBox, fCheckBox);
    m_rtCaption.Set(fTextLeft, m_rtClient.top, fTextRight - fTextLeft, m_rtClient.height);
    m_rtCaption.Inflate(-FWL_CKB_CaptionMargin, -FWL_CKB_CaptionMargin);
    CFX_RectF rtFocus;
    rtFocus.Set(m_rtCaption.left, m_rtCaption.top, m_rtCaption.width, m_rtCaption.height);
    CFX_WideString wsCaption;
    m_pProperties->m_pDataProvider->GetCaption(m_pInterface, wsCaption);
    if (wsCaption.IsEmpty()) {
        m_rtFocus.Set(0, 0, 0, 0);
    } else {
        CalcTextRect(wsCaption, m_pProperties->m_pThemeProvider, m_dwTTOStyles, m_iTTOAlign, rtFocus);
        if ((m_pProperties->m_dwStyleExes & FWL_STYLEEXT_CKB_MultiLine) == 0) {
            FX_FLOAT fWidth = FX_MAX(m_rtCaption.width, rtFocus.width);
            FX_FLOAT fHeight = FX_MIN(m_rtCaption.height, rtFocus.height);
            FX_FLOAT fLeft = m_rtCaption.left;
            FX_FLOAT fTop = m_rtCaption.top;
            if ((m_pProperties->m_dwStyleExes & FWL_STYLEEXT_CKB_HLayoutMask) == FWL_STYLEEXT_CKB_Center) {
                fLeft = m_rtCaption.left + (m_rtCaption.width - fWidth) / 2;
            } else if ((m_pProperties->m_dwStyleExes & FWL_STYLEEXT_CKB_HLayoutMask) == FWL_STYLEEXT_CKB_Right) {
                fLeft = m_rtCaption.right() - fWidth;
            }
            if ((m_pProperties->m_dwStyleExes & FWL_STYLEEXT_CKB_VLayoutMask) == FWL_STYLEEXT_CKB_VCenter) {
                fTop = m_rtCaption.top + (m_rtCaption.height - fHeight) / 2;
            } else if ((m_pProperties->m_dwStyleExes & FWL_STYLEEXT_CKB_VLayoutMask) == FWL_STYLEEXT_CKB_Bottom) {
                fTop = m_rtCaption.bottom() - fHeight;
            }
            m_rtFocus.Set(fLeft, fTop, fWidth, fHeight);
        } else {
            m_rtFocus.Set(rtFocus.left, rtFocus.top, rtFocus.width, rtFocus.height);
        }
        m_rtFocus.Inflate(1, 1);
    }
}
FX_DWORD CFWL_CheckBoxImp::GetPartStates()
{
    FX_INT32 dwStates = FWL_PARTSTATE_CKB_UnChecked;
    if ((m_pProperties->m_dwStates & FWL_STATE_CKB_CheckMask) == FWL_STATE_CKB_Neutral) {
        dwStates = FWL_PARTSTATE_CKB_Neutral;
    } else if ((m_pProperties->m_dwStates & FWL_STATE_CKB_CheckMask) == FWL_STATE_CKB_Checked) {
        dwStates = FWL_PARTSTATE_CKB_Checked;
    }
    if (m_pProperties->m_dwStates & FWL_WGTSTATE_Disabled) {
        dwStates |= FWL_PARTSTATE_CKB_Disabled;
    } else if (m_pProperties->m_dwStates & FWL_STATE_CKB_Hovered ) {
        dwStates |= FWL_PARTSTATE_CKB_Hovered;
    } else if (m_pProperties->m_dwStates & FWL_STATE_CKB_Pressed ) {
        dwStates |= FWL_PARTSTATE_CKB_Pressed;
    } else {
        dwStates |= FWL_PARTSTATE_CKB_Normal;
    }
    if (m_pProperties->m_dwStates & FWL_WGTSTATE_Focused) {
        dwStates |= FWL_PARTSTATE_CKB_Focused;
    }
    return dwStates;
}
void CFWL_CheckBoxImp::UpdateTextOutStyles()
{
    m_iTTOAlign = FDE_TTOALIGNMENT_Center;
    switch (m_pProperties->m_dwStyleExes & (FWL_STYLEEXT_CKB_HLayoutMask | FWL_STYLEEXT_CKB_VLayoutMask)) {
        case FWL_STYLEEXT_CKB_Left | FWL_STYLEEXT_CKB_Top: {
                m_iTTOAlign = FDE_TTOALIGNMENT_TopLeft;
                break;
            }
        case FWL_STYLEEXT_CKB_Center | FWL_STYLEEXT_CKB_Top: {
                m_iTTOAlign = FDE_TTOALIGNMENT_TopCenter;
                break;
            }
        case FWL_STYLEEXT_CKB_Right | FWL_STYLEEXT_CKB_Top: {
                m_iTTOAlign = FDE_TTOALIGNMENT_TopRight;
                break;
            }
        case FWL_STYLEEXT_CKB_Left | FWL_STYLEEXT_CKB_VCenter: {
                m_iTTOAlign = FDE_TTOALIGNMENT_CenterLeft;
                break;
            }
        case FWL_STYLEEXT_CKB_Center | FWL_STYLEEXT_CKB_VCenter: {
                m_iTTOAlign = FDE_TTOALIGNMENT_Center;
                break;
            }
        case FWL_STYLEEXT_CKB_Right | FWL_STYLEEXT_CKB_VCenter: {
                m_iTTOAlign = FDE_TTOALIGNMENT_CenterRight;
                break;
            }
        case FWL_STYLEEXT_CKB_Left | FWL_STYLEEXT_CKB_Bottom: {
                m_iTTOAlign = FDE_TTOALIGNMENT_BottomLeft;
                break;
            }
        case FWL_STYLEEXT_CKB_Center | FWL_STYLEEXT_CKB_Bottom: {
                m_iTTOAlign = FDE_TTOALIGNMENT_BottomCenter;
                break;
            }
        case FWL_STYLEEXT_CKB_Right | FWL_STYLEEXT_CKB_Bottom: {
                m_iTTOAlign = FDE_TTOALIGNMENT_BottomRight;
                break;
            }
        default: {
            }
    }
    m_dwTTOStyles = 0;
    if (m_pProperties->m_dwStyleExes & FWL_WGTSTYLE_RTLReading) {
        m_dwTTOStyles |= FDE_TTOSTYLE_RTL;
    }
    if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_CKB_MultiLine) {
        m_dwTTOStyles |= FDE_TTOSTYLE_LineWrap;
    } else {
        m_dwTTOStyles |= FDE_TTOSTYLE_SingleLine;
    }
}
void CFWL_CheckBoxImp::NextStates()
{
    FX_DWORD dwFirststate = m_pProperties->m_dwStates;
    if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_CKB_RadioButton) {
        if ((m_pProperties->m_dwStates & FWL_STATE_CKB_CheckMask) == FWL_STATE_CKB_Unchecked) {
            IFWL_WidgetMgr *pWMgr = FWL_GetWidgetMgr();
            CFWL_WidgetMgr *pWidgetMgr = (CFWL_WidgetMgr* )pWMgr;
            if (!pWidgetMgr->IsFormDisabled()) {
                CFX_PtrArray radioarr;
                pWidgetMgr->GetSameGroupRadioButton(m_pInterface, radioarr);
                IFWL_CheckBox *pCheckBox = NULL;
                FX_INT32 iCount = radioarr.GetSize();
                for (FX_INT32 i = 0; i < iCount; i++) {
                    pCheckBox = (IFWL_CheckBox *)radioarr[i];
                    if (pCheckBox != m_pInterface && pCheckBox->GetStates() & FWL_STATE_CKB_Checked) {
                        pCheckBox->SetCheckState(0);
                        CFX_RectF rt;
                        pCheckBox->GetWidgetRect(rt);
                        rt.left = rt.top = 0;
                        m_pWidgetMgr->RepaintWidget(pCheckBox, &rt);
                        break;
                    }
                }
            }
            m_pProperties->m_dwStates |= FWL_STATE_CKB_Checked;
        }
    } else {
        if ((m_pProperties->m_dwStates & FWL_STATE_CKB_CheckMask) == FWL_STATE_CKB_Neutral) {
            m_pProperties->m_dwStates &= ~FWL_STATE_CKB_CheckMask;
            if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_CKB_3State) {
                m_pProperties->m_dwStates |= FWL_STATE_CKB_Checked;
            }
        } else if ((m_pProperties->m_dwStates & FWL_STATE_CKB_CheckMask) == FWL_STATE_CKB_Checked) {
            m_pProperties->m_dwStates &= ~FWL_STATE_CKB_CheckMask;
        } else {
            if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_CKB_3State) {
                m_pProperties->m_dwStates |= FWL_STATE_CKB_Neutral;
            } else {
                m_pProperties->m_dwStates |= FWL_STATE_CKB_Checked;
            }
        }
    }
    Repaint(&m_rtClient);
    FX_DWORD dwLaststate = m_pProperties->m_dwStates;
    if (dwFirststate != dwLaststate) {
        CFWL_EvtCkbCheckStateChanged wmCheckBoxState;
        wmCheckBoxState.m_pSrcTarget = m_pInterface;
        DispatchEvent(&wmCheckBoxState);
    }
}
CFWL_CheckBoxImpDelegate::CFWL_CheckBoxImpDelegate(CFWL_CheckBoxImp *pOwner)
    : m_pOwner(pOwner)
{
}
FX_INT32 CFWL_CheckBoxImpDelegate::OnProcessMessage(CFWL_Message *pMessage)
{
    _FWL_RETURN_VALUE_IF_FAIL(pMessage, 0);
    FX_DWORD dwMsgCode = pMessage->GetClassID();
    FX_INT32 iRet = 1;
    switch (dwMsgCode) {
        case FWL_MSGHASH_Activate: {
                OnActivate(pMessage);
                break;
            }
        case FWL_MSGHASH_SetFocus:
        case FWL_MSGHASH_KillFocus: {
                OnFocusChanged(pMessage, dwMsgCode == FWL_MSGHASH_SetFocus);
                break;
            }
        case FWL_MSGHASH_Mouse: {
                CFWL_MsgMouse *pMsg = (CFWL_MsgMouse*)pMessage;
                FX_DWORD dwCmd = pMsg->m_dwCmd;
                switch (dwCmd) {
                    case FWL_MSGMOUSECMD_LButtonDown: {
                            OnLButtonDown(pMsg);
                            break;
                        }
                    case FWL_MSGMOUSECMD_LButtonUp: {
                            OnLButtonUp(pMsg);
                            break;
                        }
                    case FWL_MSGMOUSECMD_MouseMove: {
                            OnMouseMove(pMsg);
                            break;
                        }
                    case FWL_MSGMOUSECMD_MouseLeave: {
                            OnMouseLeave(pMsg);
                            break;
                        }
                    default: {
                        }
                }
                break;
            }
        case FWL_MSGHASH_Key: {
                CFWL_MsgKey *pKey = (CFWL_MsgKey*)pMessage;
                if (pKey->m_dwCmd == FWL_MSGKEYCMD_KeyDown) {
                    OnKeyDown((CFWL_MsgKey*)pKey);
                }
                break;
            }
        default: {
                iRet = 0;
            }
    }
    CFWL_WidgetImpDelegate::OnProcessMessage(pMessage);
    return iRet;
}
FWL_ERR	CFWL_CheckBoxImpDelegate::OnDrawWidget(CFX_Graphics *pGraphics, const CFX_Matrix *pMatrix)
{
    return m_pOwner->DrawWidget(pGraphics, pMatrix);
}
void CFWL_CheckBoxImpDelegate::OnActivate(CFWL_Message *pMsg)
{
    m_pOwner->m_pProperties->m_dwStates &= ~FWL_WGTSTATE_Deactivated;
    m_pOwner->Repaint(&(m_pOwner->m_rtClient));
}
void CFWL_CheckBoxImpDelegate::OnFocusChanged(CFWL_Message *pMsg, FX_BOOL bSet )
{
    if (bSet) {
        m_pOwner->m_pProperties->m_dwStates |= FWL_WGTSTATE_Focused;
    } else {
        m_pOwner->m_pProperties->m_dwStates &= ~FWL_WGTSTATE_Focused;
    }
    m_pOwner->Repaint(&(m_pOwner->m_rtClient));
}
void CFWL_CheckBoxImpDelegate::OnLButtonDown(CFWL_MsgMouse *pMsg)
{
    if (m_pOwner->m_pProperties->m_dwStates & FWL_WGTSTATE_Disabled) {
        return;
    }
    if ((m_pOwner->m_pProperties->m_dwStates & FWL_WGTSTATE_Focused) == 0) {
        m_pOwner->SetFocus(TRUE);
    }
    m_pOwner->m_bBtnDown = TRUE;
    m_pOwner->m_pProperties->m_dwStates &= ~ FWL_STATE_CKB_Hovered;
    m_pOwner->m_pProperties->m_dwStates |= FWL_STATE_CKB_Pressed;
    m_pOwner->Repaint(&(m_pOwner->m_rtClient));
}
void CFWL_CheckBoxImpDelegate::OnLButtonUp(CFWL_MsgMouse *pMsg)
{
    if (!m_pOwner->m_bBtnDown) {
        return;
    }
    m_pOwner->m_bBtnDown = FALSE;
    if (!m_pOwner->m_rtClient.Contains(pMsg->m_fx, pMsg->m_fy)) {
        return;
    }
    m_pOwner->m_pProperties->m_dwStates |= FWL_STATE_CKB_Hovered;
    m_pOwner->m_pProperties->m_dwStates &= ~FWL_STATE_CKB_Pressed;
    m_pOwner->NextStates();
}
void CFWL_CheckBoxImpDelegate::OnMouseMove(CFWL_MsgMouse *pMsg)
{
    if (m_pOwner->m_pProperties->m_dwStates & FWL_WGTSTATE_Disabled) {
        return;
    }
    FX_BOOL bRepaint = FALSE;
    if (m_pOwner->m_bBtnDown) {
        if (m_pOwner->m_rtClient.Contains(pMsg->m_fx, pMsg->m_fy)) {
            if ((m_pOwner->m_pProperties->m_dwStates & FWL_STATE_CKB_Pressed) == 0) {
                bRepaint = TRUE;
                m_pOwner->m_pProperties->m_dwStates |= FWL_STATE_CKB_Pressed;
            }
            if ((m_pOwner->m_pProperties->m_dwStates & FWL_STATE_CKB_Hovered)) {
                bRepaint = TRUE;
                m_pOwner->m_pProperties->m_dwStates &= ~FWL_STATE_CKB_Hovered;
            }
        } else {
            if (m_pOwner->m_pProperties->m_dwStates & FWL_STATE_CKB_Pressed) {
                bRepaint = TRUE;
                m_pOwner->m_pProperties->m_dwStates &= ~FWL_STATE_CKB_Pressed;
            }
            if ((m_pOwner->m_pProperties->m_dwStates & FWL_STATE_CKB_Hovered) == 0) {
                bRepaint = TRUE;
                m_pOwner->m_pProperties->m_dwStates |= FWL_STATE_CKB_Hovered;
            }
        }
    } else {
        if (m_pOwner->m_rtClient.Contains(pMsg->m_fx, pMsg->m_fy)) {
            if ((m_pOwner->m_pProperties->m_dwStates & FWL_STATE_CKB_Hovered) == 0) {
                bRepaint = TRUE;
                m_pOwner->m_pProperties->m_dwStates |= FWL_STATE_CKB_Hovered;
            }
        }
    }
    if (bRepaint) {
        m_pOwner->Repaint(&(m_pOwner->m_rtBox));
    }
}
void CFWL_CheckBoxImpDelegate::OnMouseLeave(CFWL_MsgMouse *pMsg)
{
    if (m_pOwner->m_bBtnDown) {
        m_pOwner->m_pProperties->m_dwStates |= FWL_STATE_CKB_Hovered;
    } else {
        m_pOwner->m_pProperties->m_dwStates &= ~FWL_STATE_CKB_Hovered;
    }
    m_pOwner->Repaint(&(m_pOwner->m_rtBox));
}
void CFWL_CheckBoxImpDelegate::OnKeyDown(CFWL_MsgKey *pMsg)
{
    if (pMsg->m_dwKeyCode == FWL_VKEY_Tab) {
        m_pOwner->DispatchKeyEvent(pMsg);
        return;
    }
    if (pMsg->m_dwKeyCode == FWL_VKEY_Return || pMsg->m_dwKeyCode == FWL_VKEY_Space) {
        m_pOwner->NextStates();
    }
}
