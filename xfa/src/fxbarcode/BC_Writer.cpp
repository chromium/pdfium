// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "barcode.h"
#include "BC_Writer.h"
CBC_Writer::CBC_Writer()
{
    m_CharEncoding		= 0;
    m_ModuleHeight		= 1;
    m_ModuleWidth		= 1;
    m_Height			= 320;
    m_Width				= 640;
    m_colorSpace		= FXDIB_Argb;
    m_barColor			= 0xff000000;
    m_backgroundColor	= 0xffffffff;
}
CBC_Writer::~CBC_Writer()
{
}
FX_BOOL CBC_Writer::SetCharEncoding(FX_INT32 encoding)
{
    m_CharEncoding = encoding;
    return TRUE;
}
FX_BOOL CBC_Writer::SetModuleHeight(FX_INT32 moduleHeight)
{
    if (moduleHeight > 10 || moduleHeight < 1) {
        return FALSE;
    }
    m_ModuleHeight = moduleHeight;
    return TRUE;
}
FX_BOOL CBC_Writer::SetModuleWidth(FX_INT32 moduleWidth)
{
    if ( moduleWidth > 10 || moduleWidth < 1) {
        return FALSE;
    }
    m_ModuleWidth = moduleWidth;
    return TRUE;
}
FX_BOOL CBC_Writer::SetHeight(FX_INT32 height)
{
    m_Height = height;
    return TRUE;
}
FX_BOOL CBC_Writer::SetWidth(FX_INT32 width)
{
    m_Width = width;
    return TRUE;
}
void CBC_Writer::SetBackgroundColor(FX_ARGB backgroundColor)
{
    m_backgroundColor = backgroundColor;
}
void CBC_Writer::SetBarcodeColor(FX_ARGB foregroundColor)
{
    m_barColor = foregroundColor;
}
CFX_DIBitmap* CBC_Writer::CreateDIBitmap(FX_INT32 width, FX_INT32 height)
{
    CFX_DIBitmap *pDIBitmap = NULL;
    pDIBitmap = FX_NEW CFX_DIBitmap;
    if(pDIBitmap != NULL) {
        pDIBitmap->Create(width, height, m_colorSpace);
    }
    return pDIBitmap;
}
