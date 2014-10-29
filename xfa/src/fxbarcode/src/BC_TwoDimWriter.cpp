// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "barcode.h"
#include "include/BC_Writer.h"
#include "include/BC_CommonBitMatrix.h"
#include "include/BC_TwoDimWriter.h"
CBC_TwoDimWriter::CBC_TwoDimWriter()
{
    m_iCorrectLevel		= 1;
    m_bFixedSize		= TRUE;
    m_output = NULL;
}
CBC_TwoDimWriter::~CBC_TwoDimWriter()
{
    if (m_output != NULL) {
        delete m_output;
        m_output = NULL;
    }
}
void CBC_TwoDimWriter::RenderDeviceResult(CFX_RenderDevice* device, const CFX_Matrix* matrix)
{
    CFX_GraphStateData stateData;
    CFX_PathData path;
    path.AppendRect(0, 0, (FX_FLOAT)m_Width, (FX_FLOAT)m_Height);
    device->DrawPath(&path, matrix, &stateData, m_backgroundColor, m_backgroundColor, FXFILL_ALTERNATE);
    FX_INT32 leftPos = 0;
    FX_INT32 topPos = 0;
    if ( m_bFixedSize) {
        leftPos = (m_Width - m_output->GetWidth()) / 2;
        topPos = (m_Height - m_output->GetHeight()) / 2;
    }
    CFX_Matrix matri = *matrix;
    if (m_Width < m_output->GetWidth() && m_Height < m_output->GetHeight()) {
        CFX_Matrix matriScale((FX_FLOAT)m_Width / (FX_FLOAT)m_output->GetWidth(), 0.0, 0.0, (FX_FLOAT)m_Height / (FX_FLOAT)m_output->GetHeight(), 0.0, 0.0);
        matriScale.Concat(*matrix);
        matri = matriScale;
    }
    for (FX_INT32 x = 0; x < m_output->GetWidth(); x++) {
        for (FX_INT32 y = 0; y < m_output->GetHeight(); y++) {
            CFX_PathData rect;
            rect.AppendRect((FX_FLOAT)leftPos + x, (FX_FLOAT)topPos + y, (FX_FLOAT)(leftPos + x + 1), (FX_FLOAT)(topPos + y + 1));
            CFX_GraphStateData stateData;
            if(m_output->Get(x, y)) {
                device->DrawPath(&rect, &matri, &stateData, m_barColor, 0, FXFILL_WINDING);
            }
        }
    }
}
void CBC_TwoDimWriter::RenderBitmapResult(CFX_DIBitmap *&pOutBitmap, FX_INT32& e)
{
    if (m_bFixedSize) {
        pOutBitmap = CreateDIBitmap(m_Width, m_Height);
    } else {
        pOutBitmap = CreateDIBitmap(m_output->GetWidth(), m_output->GetHeight());
    }
    if (!pOutBitmap) {
        e = BCExceptionFailToCreateBitmap;
        return;
    }
    pOutBitmap->Clear(m_backgroundColor);
    FX_INT32 leftPos = 0;
    FX_INT32 topPos = 0;
    if ( m_bFixedSize) {
        leftPos = (m_Width - m_output->GetWidth()) / 2;
        topPos = (m_Height - m_output->GetHeight()) / 2;
    }
    for (FX_INT32 x = 0; x < m_output->GetWidth(); x++) {
        for (FX_INT32 y = 0; y < m_output->GetHeight(); y++) {
            if (m_output->Get(x, y)) {
                pOutBitmap->SetPixel(leftPos + x, topPos + y, m_barColor);
            }
        }
    }
    if (!m_bFixedSize) {
        CFX_DIBitmap * pStretchBitmap = pOutBitmap->StretchTo(m_Width, m_Height);
        if (pOutBitmap) {
            delete pOutBitmap;
        }
        pOutBitmap = pStretchBitmap;
    }
}
void CBC_TwoDimWriter::RenderResult(FX_BYTE *code, FX_INT32 codeWidth, FX_INT32 codeHeight, FX_INT32 &e)
{
    FX_INT32 inputWidth = codeWidth;
    FX_INT32 inputHeight = codeHeight;
    FX_INT32 tempWidth = inputWidth + (1 << 1);
    FX_INT32 tempHeight = inputHeight + (1 << 1);
    FX_FLOAT moduleHSize = (FX_FLOAT)FX_MIN(m_ModuleWidth, m_ModuleHeight);
    if (moduleHSize > 8) {
        moduleHSize = 8;
    } else if (moduleHSize < 1) {
        moduleHSize = 1;
    }
    FX_INT32 outputWidth = (FX_INT32)FX_MAX(tempWidth * moduleHSize, tempWidth);
    FX_INT32 outputHeight = (FX_INT32)FX_MAX(tempHeight * moduleHSize, tempHeight);
    FX_INT32 multiX = 1;
    FX_INT32 multiY = 1;
    if (m_bFixedSize) {
        if (m_Width < outputWidth || m_Height < outputHeight) {
            e = BCExceptionBitmapSizeError;
            return;
        }
    } else {
        if (m_Width > outputWidth || m_Height > outputHeight) {
            outputWidth = (FX_INT32)(outputWidth * ceil ( (FX_FLOAT)m_Width / (FX_FLOAT)outputWidth));
            outputHeight = (FX_INT32)(outputHeight * ceil ( (FX_FLOAT)m_Height / (FX_FLOAT)outputHeight));
        }
    }
    multiX = (FX_INT32)ceil((FX_FLOAT)outputWidth / (FX_FLOAT)tempWidth);
    multiY = (FX_INT32)ceil((FX_FLOAT)outputHeight / (FX_FLOAT) tempHeight);
    if (m_bFixedSize) {
        multiX = FX_MIN(multiX, multiY);
        multiY = multiX;
    }
    FX_INT32 leftPadding = (outputWidth - (inputWidth * multiX)) / 2;
    FX_INT32 topPadding = (outputHeight - (inputHeight * multiY)) / 2;
    if (leftPadding < 0) {
        leftPadding = 0;
    }
    if (topPadding < 0) {
        topPadding = 0;
    }
    m_output = FX_NEW CBC_CommonBitMatrix;
    m_output->Init(outputWidth, outputHeight);
    for (FX_INT32 inputY = 0, outputY = topPadding; (inputY < inputHeight) && (outputY < outputHeight - multiY); inputY++, outputY += multiY) {
        for (FX_INT32 inputX = 0, outputX = leftPadding; (inputX < inputWidth) && (outputX < outputWidth - multiX); inputX++, outputX += multiX) {
            if (code[inputX + inputY * inputWidth] == 1) {
                m_output->SetRegion(outputX, outputY, multiX, multiY, e);
                BC_EXCEPTION_CHECK_ReturnVoid(e);
            }
        }
    }
}
