// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../foxitlib.h"
#include "include/fxmath_barcodeimp.h"
static CBC_CodeBase* FX_Barcode_CreateBarCodeEngineObject(BC_TYPE type)
{
    switch(type) {
        case BC_CODE39:
            return FX_NEW CBC_Code39();
        case BC_CODABAR:
            return FX_NEW CBC_Codabar();
        case BC_CODE128:
            return FX_NEW CBC_Code128(BC_CODE128_B);
        case BC_CODE128_B:
            return FX_NEW CBC_Code128(BC_CODE128_B);
        case BC_CODE128_C:
            return FX_NEW CBC_Code128(BC_CODE128_C);
        case BC_EAN8:
            return FX_NEW CBC_EAN8();
        case BC_UPCA:
            return FX_NEW CBC_UPCA();
        case BC_EAN13:
            return FX_NEW CBC_EAN13();
        case BC_QR_CODE:
            return FX_NEW CBC_QRCode();
        case BC_PDF417:
            return FX_NEW CBC_PDF417I();
        case BC_DATAMATRIX:
            return FX_NEW CBC_DataMatrix();
        case BC_UNKNOWN:
        default:
            return NULL;
    }
}
CFX_Barcode::CFX_Barcode()
{
}
CFX_Barcode::~CFX_Barcode()
{
    if(m_pBCEngine) {
        delete m_pBCEngine;
        m_pBCEngine = NULL;
    }
}
FX_BOOL CFX_Barcode::Crreate(BC_TYPE type)
{
    m_pBCEngine = FX_Barcode_CreateBarCodeEngineObject(type);
    return m_pBCEngine != NULL;
}
void CFX_Barcode::Release()
{
    delete this;
}
BC_TYPE CFX_Barcode::GetType()
{
    return m_pBCEngine ? m_pBCEngine->GetType() : BC_UNKNOWN;
}
FX_BOOL CFX_Barcode::SetCharEncoding(BC_CHAR_ENCODING encoding)
{
    return m_pBCEngine ? m_pBCEngine->SetCharEncoding(encoding) : FALSE;
}
FX_BOOL CFX_Barcode::SetModuleHeight(FX_INT32 moduleHeight)
{
    return m_pBCEngine ? m_pBCEngine->SetModuleHeight(moduleHeight) : FALSE;
}
FX_BOOL CFX_Barcode::SetModuleWidth(FX_INT32 moduleWidth)
{
    return m_pBCEngine ? m_pBCEngine->SetModuleWidth(moduleWidth) : FALSE;
}
FX_BOOL CFX_Barcode::SetHeight(FX_INT32 height)
{
    return m_pBCEngine ? m_pBCEngine->SetHeight(height) : FALSE;
}
FX_BOOL CFX_Barcode::SetWidth(FX_INT32 width)
{
    return m_pBCEngine ? m_pBCEngine->SetWidth(width) : FALSE;
}
FX_BOOL	CFX_Barcode::CheckContentValidity(FX_WSTR contents)
{
    switch(GetType()) {
        case BC_CODE39:
        case BC_CODABAR:
        case BC_CODE128:
        case BC_CODE128_B:
        case BC_CODE128_C:
        case BC_EAN8:
        case BC_EAN13:
        case BC_UPCA:
            return m_pBCEngine ? ((CBC_OneCode*)m_pBCEngine)->CheckContentValidity(contents)
                   : TRUE;
        default:
            return TRUE;
    }
}
FX_BOOL	CFX_Barcode::SetPrintChecksum(FX_BOOL checksum)
{
    switch(GetType()) {
        case BC_CODE39:
        case BC_CODABAR:
        case BC_CODE128:
        case BC_CODE128_B:
        case BC_CODE128_C:
        case BC_EAN8:
        case BC_EAN13:
        case BC_UPCA:
            return m_pBCEngine ? (((CBC_OneCode*)m_pBCEngine)->SetPrintChecksum(checksum), TRUE)
                   : FALSE;
        default:
            return FALSE;
    }
}
FX_BOOL	CFX_Barcode::SetDataLength(FX_INT32 length)
{
    switch(GetType()) {
        case BC_CODE39:
        case BC_CODABAR:
        case BC_CODE128:
        case BC_CODE128_B:
        case BC_CODE128_C:
        case BC_EAN8:
        case BC_EAN13:
        case BC_UPCA:
            return m_pBCEngine ? (((CBC_OneCode*)m_pBCEngine)->SetDataLength(length), TRUE)
                   : FALSE;
        default:
            return FALSE;
    }
}
FX_BOOL	CFX_Barcode::SetCalChecksum(FX_INT32 state)
{
    switch(GetType()) {
        case BC_CODE39:
        case BC_CODABAR:
        case BC_CODE128:
        case BC_CODE128_B:
        case BC_CODE128_C:
        case BC_EAN8:
        case BC_EAN13:
        case BC_UPCA:
            return m_pBCEngine ? (((CBC_OneCode*)m_pBCEngine)->SetCalChecksum(state), TRUE)
                   : FALSE;
        default:
            return FALSE;
    }
}
FX_BOOL	CFX_Barcode::SetFont(CFX_Font* pFont)
{
    switch(GetType()) {
        case BC_CODE39:
        case BC_CODABAR:
        case BC_CODE128:
        case BC_CODE128_B:
        case BC_CODE128_C:
        case BC_EAN8:
        case BC_EAN13:
        case BC_UPCA:
            return m_pBCEngine ? ((CBC_OneCode*)m_pBCEngine)->SetFont(pFont)
                   : FALSE;
        default:
            return FALSE;
    }
}
FX_BOOL	CFX_Barcode::SetFontSize(FX_FLOAT size)
{
    switch(GetType()) {
        case BC_CODE39:
        case BC_CODABAR:
        case BC_CODE128:
        case BC_CODE128_B:
        case BC_CODE128_C:
        case BC_EAN8:
        case BC_EAN13:
        case BC_UPCA:
            return m_pBCEngine ? (((CBC_OneCode*)m_pBCEngine)->SetFontSize(size), TRUE)
                   : FALSE;
        default:
            return FALSE;
    }
}
FX_BOOL	CFX_Barcode::SetFontStyle(FX_INT32 style)
{
    switch(GetType()) {
        case BC_CODE39:
        case BC_CODABAR:
        case BC_CODE128:
        case BC_CODE128_B:
        case BC_CODE128_C:
        case BC_EAN8:
        case BC_EAN13:
        case BC_UPCA:
            return m_pBCEngine ? (((CBC_OneCode*)m_pBCEngine)->SetFontStyle(style), TRUE)
                   : FALSE;
        default:
            return FALSE;
    }
}
FX_BOOL	CFX_Barcode::SetFontColor(FX_ARGB color)
{
    switch(GetType()) {
        case BC_CODE39:
        case BC_CODABAR:
        case BC_CODE128:
        case BC_CODE128_B:
        case BC_CODE128_C:
        case BC_EAN8:
        case BC_EAN13:
        case BC_UPCA:
            return m_pBCEngine ? (((CBC_OneCode*)m_pBCEngine)->SetFontColor(color), TRUE)
                   : FALSE;
        default:
            return FALSE;
    }
}
FX_BOOL	CFX_Barcode::SetTextLocation(BC_TEXT_LOC location)
{
    typedef FX_BOOL (CBC_CodeBase::*memptrtype)(BC_TEXT_LOC);
    memptrtype memptr = NULL;
    switch(GetType()) {
        case BC_CODE39:
            memptr = (memptrtype)&CBC_Code39::SetTextLocation;
            break;
        case BC_CODABAR:
            memptr = (memptrtype)&CBC_Codabar::SetTextLocation;
            break;
        case BC_CODE128:
        case BC_CODE128_B:
        case BC_CODE128_C:
            memptr = (memptrtype)&CBC_Code128::SetTextLocation;
            break;
        default:
            break;
    }
    return m_pBCEngine && memptr ? (m_pBCEngine->*memptr)(location)
           : FALSE;
}
FX_BOOL	CFX_Barcode::SetWideNarrowRatio(FX_INT32 ratio)
{
    typedef FX_BOOL (CBC_CodeBase::*memptrtype)(FX_INT32);
    memptrtype memptr = NULL;
    switch(GetType()) {
        case BC_CODE39:
            memptr = (memptrtype)&CBC_Code39::SetWideNarrowRatio;
            break;
        case BC_CODABAR:
            memptr = (memptrtype)&CBC_Codabar::SetWideNarrowRatio;
            break;
        default:
            break;
    }
    return m_pBCEngine && memptr ? (m_pBCEngine->*memptr)(ratio)
           : FALSE;
}
FX_BOOL	CFX_Barcode::SetStartChar(FX_CHAR start)
{
    typedef FX_BOOL (CBC_CodeBase::*memptrtype)(FX_CHAR);
    memptrtype memptr = NULL;
    switch(GetType()) {
        case BC_CODABAR:
            memptr = (memptrtype)&CBC_Codabar::SetStartChar;
            break;
        default:
            break;
    }
    return m_pBCEngine && memptr ? (m_pBCEngine->*memptr)(start)
           : FALSE;
}
FX_BOOL	CFX_Barcode::SetEndChar(FX_CHAR end)
{
    typedef FX_BOOL (CBC_CodeBase::*memptrtype)(FX_CHAR);
    memptrtype memptr = NULL;
    switch(GetType()) {
        case BC_CODABAR:
            memptr = (memptrtype)&CBC_Codabar::SetEndChar;
            break;
        default:
            break;
    }
    return m_pBCEngine && memptr ? (m_pBCEngine->*memptr)(end)
           : FALSE;
}
FX_BOOL	CFX_Barcode::SetVersion(FX_INT32 version)
{
    typedef FX_BOOL (CBC_CodeBase::*memptrtype)(FX_INT32);
    memptrtype memptr = NULL;
    switch(GetType()) {
        case BC_QR_CODE:
            memptr = (memptrtype)&CBC_QRCode::SetVersion;
            break;
        default:
            break;
    }
    return m_pBCEngine && memptr ? (m_pBCEngine->*memptr)(version)
           : FALSE;
}
FX_BOOL	CFX_Barcode::SetErrorCorrectionLevel(FX_INT32 level)
{
    typedef FX_BOOL (CBC_CodeBase::*memptrtype)(FX_INT32);
    memptrtype memptr = NULL;
    switch(GetType()) {
        case BC_QR_CODE:
            memptr = (memptrtype)&CBC_QRCode::SetErrorCorrectionLevel;
            break;
        case BC_PDF417:
            memptr = (memptrtype)&CBC_PDF417I::SetErrorCorrectionLevel;
            break;
        default:
            return FALSE;
    }
    return m_pBCEngine && memptr ? (m_pBCEngine->*memptr)(level)
           : FALSE;
}
FX_BOOL	CFX_Barcode::SetTruncated(FX_BOOL truncated)
{
    typedef void (CBC_CodeBase::*memptrtype)(FX_BOOL);
    memptrtype memptr = NULL;
    switch(GetType()) {
        case BC_PDF417:
            memptr = (memptrtype)&CBC_PDF417I::SetTruncated;
            break;
        default:
            break;
    }
    return m_pBCEngine && memptr ? ((m_pBCEngine->*memptr)(truncated), TRUE)
           : FALSE;
}
#ifndef BCExceptionNO
#define	BCExceptionNO											           0
#endif
#ifndef BCExceptionFormatException
#define	BCExceptionFormatException								           8
#endif
#ifndef BCExceptionUnSupportedBarcode
#define	BCExceptionUnSupportedBarcode							          18
#endif
FX_BOOL CFX_Barcode::Encode(FX_WSTR contents, FX_BOOL isDevice, FX_INT32 &e)
{
    if(!m_pBCEngine) {
        return FALSE;
    }
    return m_pBCEngine->Encode(contents, isDevice, e);
}
FX_BOOL CFX_Barcode::RenderDevice(CFX_RenderDevice* device, const CFX_Matrix* matirx, FX_INT32 &e)
{
    if (!m_pBCEngine) {
        return FALSE;
    }
    return m_pBCEngine->RenderDevice(device, matirx, e);
}
FX_BOOL CFX_Barcode::RenderBitmap(CFX_DIBitmap *&pOutBitmap, FX_INT32 &e)
{
    if (!m_pBCEngine) {
        return FALSE;
    }
    return m_pBCEngine->RenderBitmap(pOutBitmap, e);
}
#define BC_TYPE_MIN BC_CODE39
#define BC_TYPE_MAX BC_DATAMATRIX
CFX_WideString	CFX_Barcode::Decode(FX_BYTE* buf, FX_INT32 width, FX_INT32 height, FX_INT32 &errorCode)
{
    for(BC_TYPE t = BC_TYPE_MIN; t <= BC_TYPE_MAX; t = (BC_TYPE)((FX_INT32)t + 1)) {
        CBC_CodeBase* pTmpEngine = FX_Barcode_CreateBarCodeEngineObject(t);
        if(!pTmpEngine) {
            continue;
        }
        CFX_WideString ret = pTmpEngine->Decode(buf, width, height, errorCode);
        if(errorCode == BCExceptionNO) {
            return ret;
        }
    }
    errorCode = BCExceptionUnSupportedBarcode;
    return CFX_WideString();
}
CFX_WideString	CFX_Barcode::Decode(CFX_DIBitmap *pBitmap, FX_INT32 &errorCode)
{
    for(BC_TYPE t = BC_TYPE_MIN; t <= BC_TYPE_MAX; t = (BC_TYPE)((FX_INT32)t + 1)) {
        CBC_CodeBase* pTmpEngine = FX_Barcode_CreateBarCodeEngineObject(t);
        if(!pTmpEngine) {
            continue;
        }
        CFX_WideString ret = pTmpEngine->Decode(pBitmap, errorCode);
        if(errorCode == BCExceptionNO) {
            return ret;
        }
    }
    errorCode = BCExceptionUnSupportedBarcode;
    return CFX_WideString();
}
IFX_Barcode* FX_Barcode_Create(BC_TYPE type)
{
    CFX_Barcode* pBarcode = FX_NEW CFX_Barcode;
    if (pBarcode->Crreate(type)) {
        return pBarcode;
    }
    pBarcode->Release();
    return NULL;
}
