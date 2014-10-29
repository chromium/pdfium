// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../foxitlib.h"
#include "../common/xfa_common.h"
#include "xfa_fwladapter.h"
#include "xfa_ffwidget.h"
#include "xfa_fffield.h"
#include "xfa_ffpageview.h"
#include "xfa_fftextedit.h"
#include "xfa_ffbarcode.h"
static XFA_LPCBARCODETYPEENUMINFO XFA_GetBarcodeTypeByName(FX_WSTR wsName);
CXFA_FFBarcode::CXFA_FFBarcode(CXFA_FFPageView* pPageView, CXFA_WidgetAcc* pDataAcc)
    : CXFA_FFTextEdit(pPageView, pDataAcc)
{
}
CXFA_FFBarcode::~CXFA_FFBarcode()
{
}
FX_BOOL CXFA_FFBarcode::LoadWidget()
{
    CFWL_Barcode* pFWLBarcode = CFWL_Barcode::Create();
    if(pFWLBarcode)	{
        pFWLBarcode->Initialize();
    }
    m_pNormalWidget = pFWLBarcode;
    IFWL_Widget* pWidget = m_pNormalWidget->GetWidget();
    m_pNormalWidget->SetPrivateData(pWidget, this, NULL);
    IFWL_NoteDriver *pNoteDriver = FWL_GetApp()->GetNoteDriver();
    pNoteDriver->RegisterEventTarget(pWidget, pWidget);
    m_pOldDelegate = m_pNormalWidget->SetDelegate(this);
    m_pNormalWidget->LockUpdate();
    CFX_WideString wsText;
    m_pDataAcc->GetValue(wsText, XFA_VALUEPICTURE_Display);
    pFWLBarcode->SetText(wsText);
    UpdateWidgetProperty();
    m_pNormalWidget->UnlockUpdate();
    return CXFA_FFField::LoadWidget();
}
void CXFA_FFBarcode::RenderWidget(CFX_Graphics* pGS, CFX_Matrix* pMatrix , FX_DWORD dwStatus , FX_INT32 iRotate )
{
    if (!IsMatchVisibleStatus(dwStatus)) {
        return;
    }
    CFX_Matrix mtRotate;
    GetRotateMatrix(mtRotate);
    if (pMatrix) {
        mtRotate.Concat(*pMatrix);
    }
    CXFA_FFWidget::RenderWidget(pGS, &mtRotate, dwStatus);
    CXFA_Border borderUI = m_pDataAcc->GetUIBorder();
    DrawBorder(pGS, borderUI, m_rtUI, &mtRotate);
    RenderCaption(pGS, &mtRotate);
    CFX_RectF rtWidget;
    m_pNormalWidget->GetWidgetRect(rtWidget);
    CFX_Matrix mt;
    mt.Set(1, 0, 0, 1, rtWidget.left, rtWidget.top);
    mt.Concat(mtRotate);
    m_pNormalWidget->DrawWidget(pGS, &mt);
}
void CXFA_FFBarcode::UpdateWidgetProperty()
{
    CXFA_FFTextEdit::UpdateWidgetProperty();
    CFWL_Barcode* pBarCodeWidget = (CFWL_Barcode*)m_pNormalWidget;
    CFX_WideString wsType = GetDataAcc()->GetBarcodeType();
    XFA_LPCBARCODETYPEENUMINFO pBarcodeTypeInfo = XFA_GetBarcodeTypeByName(wsType);
    pBarCodeWidget->SetType(pBarcodeTypeInfo->eBCType);
    CXFA_WidgetAcc *pAcc = GetDataAcc();
    FX_INT32 intVal;
    FX_CHAR  charVal;
    FX_BOOL  boolVal;
    FX_FLOAT floatVal;
    if (pAcc->GetBarcodeAttribute_CharEncoding(intVal)) {
        pBarCodeWidget->SetCharEncoding((BC_CHAR_ENCODING)intVal);
    }
    if (pAcc->GetBarcodeAttribute_Checksum(intVal)) {
        pBarCodeWidget->SetCalChecksum(intVal);
    }
    if (pAcc->GetBarcodeAttribute_DataLength(intVal)) {
        pBarCodeWidget->SetDataLength(intVal);
    }
    if (pAcc->GetBarcodeAttribute_StartChar(charVal)) {
        pBarCodeWidget->SetStartChar(charVal);
    }
    if (pAcc->GetBarcodeAttribute_EndChar(charVal)) {
        pBarCodeWidget->SetEndChar(charVal);
    }
    if (pAcc->GetBarcodeAttribute_ECLevel(intVal)) {
        pBarCodeWidget->SetErrorCorrectionLevel(intVal);
    }
    if (pAcc->GetBarcodeAttribute_ModuleWidth(intVal)) {
        pBarCodeWidget->SetModuleWidth(intVal);
    }
    if (pAcc->GetBarcodeAttribute_ModuleHeight(intVal)) {
        pBarCodeWidget->SetModuleHeight(intVal);
    }
    if (pAcc->GetBarcodeAttribute_PrintChecksum(boolVal)) {
        pBarCodeWidget->SetPrintChecksum(boolVal);
    }
    if (pAcc->GetBarcodeAttribute_TextLocation(intVal)) {
        pBarCodeWidget->SetTextLocation((BC_TEXT_LOC)intVal);
    }
    if (pAcc->GetBarcodeAttribute_Truncate(boolVal)) {
        pBarCodeWidget->SetTruncated(boolVal);
    }
    if (pAcc->GetBarcodeAttribute_WideNarrowRatio(floatVal))	{
        pBarCodeWidget->SetWideNarrowRatio((FX_INT32)floatVal);
    }
    if (pBarcodeTypeInfo->eName == XFA_BARCODETYPE_code3Of9
            || pBarcodeTypeInfo->eName == XFA_BARCODETYPE_ean8
            || pBarcodeTypeInfo->eName == XFA_BARCODETYPE_ean13
            || pBarcodeTypeInfo->eName == XFA_BARCODETYPE_upcA) {
        pBarCodeWidget->SetPrintChecksum(TRUE);
    }
}
FX_BOOL	CXFA_FFBarcode::OnLButtonDown(FX_DWORD dwFlags, FX_FLOAT fx, FX_FLOAT fy)
{
    CFWL_Barcode* pBarCodeWidget = (CFWL_Barcode*)m_pNormalWidget;
    if (!pBarCodeWidget || pBarCodeWidget->IsProtectedType()) {
        return FALSE;
    }
    if (m_pDataAcc->GetAccess() != XFA_ATTRIBUTEENUM_Open) {
        return FALSE;
    }
    return CXFA_FFTextEdit::OnLButtonDown(dwFlags, fx, fy);
}
FX_BOOL	CXFA_FFBarcode::OnRButtonDown(FX_DWORD dwFlags, FX_FLOAT fx, FX_FLOAT fy)
{
    CFWL_Barcode* pBarCodeWidget = (CFWL_Barcode*)m_pNormalWidget;
    if (!pBarCodeWidget || pBarCodeWidget->IsProtectedType()) {
        return FALSE;
    }
    return CXFA_FFTextEdit::OnRButtonDown(dwFlags, fx, fy);
}
extern const XFA_BARCODETYPEENUMINFO g_XFABarCodeTypeEnumData[] = {
    {0x7fb4a18, (FX_LPCWSTR)L"ean13", XFA_BARCODETYPE_ean13, BC_EAN13},
    {0x8d13a3d, (FX_LPCWSTR)L"code11", XFA_BARCODETYPE_code11, BC_UNKNOWN},
    {0x8d149a8, (FX_LPCWSTR)L"code49", XFA_BARCODETYPE_code49, BC_UNKNOWN},
    {0x8d16347, (FX_LPCWSTR)L"code93", XFA_BARCODETYPE_code93, BC_UNKNOWN},
    {0x91a92e2, (FX_LPCWSTR)L"upsMaxicode", XFA_BARCODETYPE_upsMaxicode, BC_UNKNOWN},
    {0xa7d48dc, (FX_LPCWSTR)L"fim", XFA_BARCODETYPE_fim, BC_UNKNOWN},
    {0xb359fe9, (FX_LPCWSTR)L"msi", XFA_BARCODETYPE_msi, BC_UNKNOWN},
    {0x121f738c, (FX_LPCWSTR)L"code2Of5Matrix", XFA_BARCODETYPE_code2Of5Matrix, BC_UNKNOWN},
    {0x15358616, (FX_LPCWSTR)L"ucc128", XFA_BARCODETYPE_ucc128, BC_UNKNOWN},
    {0x1f4bfa05, (FX_LPCWSTR)L"rfid", XFA_BARCODETYPE_rfid, BC_UNKNOWN},
    {0x1fda71bc, (FX_LPCWSTR)L"rss14Stacked", XFA_BARCODETYPE_rss14Stacked, BC_UNKNOWN},
    {0x22065087, (FX_LPCWSTR)L"ean8add2", XFA_BARCODETYPE_ean8add2, BC_UNKNOWN},
    {0x2206508a, (FX_LPCWSTR)L"ean8add5", XFA_BARCODETYPE_ean8add5, BC_UNKNOWN},
    {0x2278366c, (FX_LPCWSTR)L"codabar", XFA_BARCODETYPE_codabar, BC_CODABAR},
    {0x2a039a8d, (FX_LPCWSTR)L"telepen", XFA_BARCODETYPE_telepen, BC_UNKNOWN},
    {0x323ed337, (FX_LPCWSTR)L"upcApwcd", XFA_BARCODETYPE_upcApwcd, BC_UNKNOWN},
    {0x347a1846, (FX_LPCWSTR)L"postUSIMB", XFA_BARCODETYPE_postUSIMB, BC_UNKNOWN},
    {0x391bb836, (FX_LPCWSTR)L"code128", XFA_BARCODETYPE_code128, BC_CODE128},
    {0x398eddaf, (FX_LPCWSTR)L"dataMatrix", XFA_BARCODETYPE_dataMatrix, BC_DATAMATRIX},
    {0x3cff60a8, (FX_LPCWSTR)L"upcEadd2", XFA_BARCODETYPE_upcEadd2, BC_UNKNOWN},
    {0x3cff60ab, (FX_LPCWSTR)L"upcEadd5", XFA_BARCODETYPE_upcEadd5, BC_UNKNOWN},
    {0x402cb188, (FX_LPCWSTR)L"code2Of5Standard", XFA_BARCODETYPE_code2Of5Standard, BC_UNKNOWN},
    {0x411764f7, (FX_LPCWSTR)L"aztec", XFA_BARCODETYPE_aztec, BC_UNKNOWN},
    {0x44d4e84c, (FX_LPCWSTR)L"ean8", XFA_BARCODETYPE_ean8, BC_EAN8},
    {0x48468902, (FX_LPCWSTR)L"ucc128sscc", XFA_BARCODETYPE_ucc128sscc, BC_UNKNOWN},
    {0x4880aea4, (FX_LPCWSTR)L"upcAadd2", XFA_BARCODETYPE_upcAadd2, BC_UNKNOWN},
    {0x4880aea7, (FX_LPCWSTR)L"upcAadd5", XFA_BARCODETYPE_upcAadd5, BC_UNKNOWN},
    {0x54f18256, (FX_LPCWSTR)L"code2Of5Industrial", XFA_BARCODETYPE_code2Of5Industrial, BC_UNKNOWN},
    {0x58e15f25, (FX_LPCWSTR)L"rss14Limited", XFA_BARCODETYPE_rss14Limited, BC_UNKNOWN},
    {0x5c08d1b9, (FX_LPCWSTR)L"postAUSReplyPaid", XFA_BARCODETYPE_postAUSReplyPaid, BC_UNKNOWN},
    {0x5fa700bd, (FX_LPCWSTR)L"rss14", XFA_BARCODETYPE_rss14, BC_UNKNOWN},
    {0x631a7e35, (FX_LPCWSTR)L"logmars", XFA_BARCODETYPE_logmars, BC_UNKNOWN},
    {0x6a236236, (FX_LPCWSTR)L"pdf417", XFA_BARCODETYPE_pdf417, BC_PDF417},
    {0x6d098ece, (FX_LPCWSTR)L"upcean2", XFA_BARCODETYPE_upcean2, BC_UNKNOWN},
    {0x6d098ed1, (FX_LPCWSTR)L"upcean5", XFA_BARCODETYPE_upcean5, BC_UNKNOWN},
    {0x76b04eed, (FX_LPCWSTR)L"code3Of9extended", XFA_BARCODETYPE_code3Of9extended, BC_UNKNOWN},
    {0x7c7db84a, (FX_LPCWSTR)L"maxicode", XFA_BARCODETYPE_maxicode, BC_UNKNOWN},
    {0x8266f7f7, (FX_LPCWSTR)L"ucc128random", XFA_BARCODETYPE_ucc128random, BC_UNKNOWN},
    {0x83eca147, (FX_LPCWSTR)L"postUSDPBC", XFA_BARCODETYPE_postUSDPBC, BC_UNKNOWN},
    {0x8dd71de0, (FX_LPCWSTR)L"postAUSStandard", XFA_BARCODETYPE_postAUSStandard, BC_UNKNOWN},
    {0x98adad85, (FX_LPCWSTR)L"plessey", XFA_BARCODETYPE_plessey, BC_UNKNOWN},
    {0x9f84cce6, (FX_LPCWSTR)L"ean13pwcd", XFA_BARCODETYPE_ean13pwcd, BC_UNKNOWN},
    {0xb514fbe9, (FX_LPCWSTR)L"upcA", XFA_BARCODETYPE_upcA, BC_UPCA},
    {0xb514fbed, (FX_LPCWSTR)L"upcE", XFA_BARCODETYPE_upcE, BC_UNKNOWN},
    {0xb5c6a853, (FX_LPCWSTR)L"ean13add2", XFA_BARCODETYPE_ean13add2, BC_UNKNOWN},
    {0xb5c6a856, (FX_LPCWSTR)L"ean13add5", XFA_BARCODETYPE_ean13add5, BC_UNKNOWN},
    {0xb81fc512, (FX_LPCWSTR)L"postUKRM4SCC", XFA_BARCODETYPE_postUKRM4SCC, BC_UNKNOWN},
    {0xbad34b22, (FX_LPCWSTR)L"code128SSCC", XFA_BARCODETYPE_code128SSCC, BC_UNKNOWN},
    {0xbfbe0cf6, (FX_LPCWSTR)L"postUS5Zip", XFA_BARCODETYPE_postUS5Zip, BC_UNKNOWN},
    {0xc56618e8, (FX_LPCWSTR)L"pdf417macro", XFA_BARCODETYPE_pdf417macro, BC_UNKNOWN},
    {0xca730f8a, (FX_LPCWSTR)L"code2Of5Interleaved", XFA_BARCODETYPE_code2Of5Interleaved, BC_UNKNOWN},
    {0xd0097ac6, (FX_LPCWSTR)L"rss14Expanded", XFA_BARCODETYPE_rss14Expanded, BC_UNKNOWN},
    {0xd25a0240, (FX_LPCWSTR)L"postAUSCust2", XFA_BARCODETYPE_postAUSCust2, BC_UNKNOWN},
    {0xd25a0241, (FX_LPCWSTR)L"postAUSCust3", XFA_BARCODETYPE_postAUSCust3, BC_UNKNOWN},
    {0xd53ed3e7, (FX_LPCWSTR)L"rss14Truncated", XFA_BARCODETYPE_rss14Truncated, BC_UNKNOWN},
    {0xe72bcd57, (FX_LPCWSTR)L"code128A", XFA_BARCODETYPE_code128A, BC_UNKNOWN},
    {0xe72bcd58, (FX_LPCWSTR)L"code128B", XFA_BARCODETYPE_code128B, BC_CODE128_B},
    {0xe72bcd59, (FX_LPCWSTR)L"code128C", XFA_BARCODETYPE_code128C, BC_CODE128_C},
    {0xee83c50f, (FX_LPCWSTR)L"rss14StackedOmni", XFA_BARCODETYPE_rss14StackedOmni, BC_UNKNOWN},
    {0xf2a18f7e, (FX_LPCWSTR)L"QRCode", XFA_BARCODETYPE_QRCode, BC_QR_CODE},
    {0xfaeaf37f, (FX_LPCWSTR)L"postUSStandard", XFA_BARCODETYPE_postUSStandard, BC_UNKNOWN},
    {0xfb48155c, (FX_LPCWSTR)L"code3Of9", XFA_BARCODETYPE_code3Of9, BC_CODE39},
};
extern const FX_INT32 g_iXFABarcodeTypeCount = sizeof(g_XFABarCodeTypeEnumData) / sizeof(XFA_BARCODETYPEENUMINFO);
static XFA_LPCBARCODETYPEENUMINFO XFA_GetBarcodeTypeByName(FX_WSTR wsName)
{
    FX_INT32 iLength = wsName.GetLength();
    if (iLength == 0) {
        return NULL;
    }
    FX_UINT32 uHash = FX_HashCode_String_GetW(wsName.GetPtr(), iLength, TRUE);
    FX_INT32 iStart = 0, iEnd = g_iXFABarcodeTypeCount - 1;
    do {
        FX_INT32 iMid = (iStart + iEnd) / 2;
        XFA_LPCBARCODETYPEENUMINFO pInfo = g_XFABarCodeTypeEnumData + iMid;
        if (uHash == pInfo->uHash) {
            return pInfo;
        } else if (uHash < pInfo->uHash) {
            iEnd = iMid - 1;
        } else {
            iStart = iMid + 1;
        }
    } while (iStart <= iEnd);
    return NULL;
}
