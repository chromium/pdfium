// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "pre.h"
#include "../include/fx_graphics.h"
#include "fx_metafile_interpreter.h"
CFX_MetafileInterpreter::CFX_MetafileInterpreter()
{
    _element = NULL;
}
CFX_MetafileInterpreter::~CFX_MetafileInterpreter()
{
    _element = NULL;
}
FX_BOOL CFX_MetafileInterpreter::SetCurrentElement(CXML_Element * element)
{
    _FX_RETURN_VALUE_IF_FAIL(element, FALSE);
    _element = element;
    return TRUE;
}
FX_BOOL CFX_MetafileInterpreter::ParamOf_SetLineCap(int32_t & lineCap)
{
    _FX_RETURN_VALUE_IF_FAIL(_element, FALSE);
    CXML_Element * lineCapElement = _element->GetElement(0);
    _FX_RETURN_VALUE_IF_FAIL(lineCapElement, FALSE);
    lineCap = lineCapElement->GetAttrInteger("CFX_GraphStateData::LineCap");
    return TRUE;
}
FX_BOOL CFX_MetafileInterpreter::ParamOf_SetLineDash(FX_FLOAT &	dashPhase,
        int32_t &	dashArray,
        int32_t &	dashCount)
{
    _FX_RETURN_VALUE_IF_FAIL(_element, FALSE);
    CXML_Element * dashPhaseElement = _element->GetElement(0);
    _FX_RETURN_VALUE_IF_FAIL(dashPhaseElement, FALSE);
    dashPhase = dashPhaseElement->GetAttrFloat("FX_FLOAT");
    CXML_Element * dashArrayElement = _element->GetElement(1);
    _FX_RETURN_VALUE_IF_FAIL(dashArrayElement, FALSE);
    dashArray = dashArrayElement->GetAttrInteger("FX_FLOAT *");
    CXML_Element * dashCountElement = _element->GetElement(2);
    _FX_RETURN_VALUE_IF_FAIL(dashCountElement, FALSE);
    dashCount = dashCountElement->GetAttrInteger("int32_t");
    return TRUE;
}
FX_BOOL CFX_MetafileInterpreter::ParamOf_SetLineDash(int32_t & dashStyle)
{
    _FX_RETURN_VALUE_IF_FAIL(_element, FALSE);
    CXML_Element * dashStyleElement = _element->GetElement(0);
    _FX_RETURN_VALUE_IF_FAIL(dashStyleElement, FALSE);
    dashStyle = dashStyleElement->GetAttrInteger("FX_DashStyle");
    return TRUE;
}
FX_BOOL CFX_MetafileInterpreter::ParamOf_SetLineJoin(int32_t & lineJoin)
{
    _FX_RETURN_VALUE_IF_FAIL(_element, FALSE);
    CXML_Element * lineJoinElement = _element->GetElement(0);
    _FX_RETURN_VALUE_IF_FAIL(lineJoinElement, FALSE);
    lineJoin = lineJoinElement->GetAttrInteger("CFX_GraphStateData::LineJoin");
    return TRUE;
}
FX_BOOL CFX_MetafileInterpreter::ParamOf_SetMiterLimit(FX_FLOAT & miterLimit)
{
    _FX_RETURN_VALUE_IF_FAIL(_element, FALSE);
    CXML_Element * miterLimitElement = _element->GetElement(0);
    _FX_RETURN_VALUE_IF_FAIL(miterLimitElement, FALSE);
    miterLimit = miterLimitElement->GetAttrFloat("FX_FLOAT");
    return TRUE;
}
FX_BOOL CFX_MetafileInterpreter::ParamOf_SetLineWidth(FX_FLOAT & lineWidth,
        int32_t & isActOnDash)
{
    _FX_RETURN_VALUE_IF_FAIL(_element, FALSE);
    CXML_Element * lineWidthElement = _element->GetElement(0);
    _FX_RETURN_VALUE_IF_FAIL(lineWidthElement, FALSE);
    lineWidth = lineWidthElement->GetAttrFloat("FX_FLOAT");
    CXML_Element * isActOnDashElement = _element->GetElement(1);
    _FX_RETURN_VALUE_IF_FAIL(isActOnDashElement, FALSE);
    isActOnDash = isActOnDashElement->GetAttrInteger("FX_BOOL");
    return TRUE;
}
FX_BOOL CFX_MetafileInterpreter::ParamOf_SetStrokeColor(int32_t & color)
{
    _FX_RETURN_VALUE_IF_FAIL(_element, FALSE);
    CXML_Element * colorElement = _element->GetElement(0);
    _FX_RETURN_VALUE_IF_FAIL(colorElement, FALSE);
    color = colorElement->GetAttrInteger("CFX_Color *");
    return TRUE;
}
FX_BOOL CFX_MetafileInterpreter::ParamOf_SetFillColor(int32_t & color)
{
    _FX_RETURN_VALUE_IF_FAIL(_element, FALSE);
    CXML_Element * colorElement = _element->GetElement(0);
    _FX_RETURN_VALUE_IF_FAIL(colorElement, FALSE);
    color = colorElement->GetAttrInteger("CFX_Color *");
    return TRUE;
}
FX_BOOL CFX_MetafileInterpreter::ParamOf_StrokePath(int32_t & path, int32_t & matrix)
{
    _FX_RETURN_VALUE_IF_FAIL(_element, FALSE);
    CXML_Element * pathElement = _element->GetElement(0);
    _FX_RETURN_VALUE_IF_FAIL(pathElement, FALSE);
    path = pathElement->GetAttrInteger("CFX_Path *");
    CXML_Element * matrixElement = _element->GetElement(1);
    _FX_RETURN_VALUE_IF_FAIL(matrixElement, FALSE);
    matrix = matrixElement->GetAttrInteger("CFX_Matrix *");
    return TRUE;
}
FX_BOOL CFX_MetafileInterpreter::ParamOf_FillPath(int32_t & path,
        int32_t & fillMode,
        int32_t & matrix)
{
    _FX_RETURN_VALUE_IF_FAIL(_element, FALSE);
    CXML_Element * pathElement = _element->GetElement(0);
    _FX_RETURN_VALUE_IF_FAIL(pathElement, FALSE);
    path = pathElement->GetAttrInteger("CFX_Path *");
    CXML_Element * fillModeElement = _element->GetElement(1);
    _FX_RETURN_VALUE_IF_FAIL(fillModeElement, FALSE);
    fillMode = fillModeElement->GetAttrInteger("FX_FillMode");
    CXML_Element * matrixElement = _element->GetElement(2);
    _FX_RETURN_VALUE_IF_FAIL(matrixElement, FALSE);
    matrix = matrixElement->GetAttrInteger("CFX_Matrix *");
    return TRUE;
}
FX_BOOL CFX_MetafileInterpreter::ParamOf_ClipPath(int32_t & path,
        int32_t & fillMode,
        int32_t & matrix)
{
    _FX_RETURN_VALUE_IF_FAIL(_element, FALSE);
    CXML_Element * pathElement = _element->GetElement(0);
    _FX_RETURN_VALUE_IF_FAIL(pathElement, FALSE);
    path = pathElement->GetAttrInteger("CFX_Path *");
    CXML_Element * fillModeElement = _element->GetElement(1);
    _FX_RETURN_VALUE_IF_FAIL(fillModeElement, FALSE);
    fillMode = fillModeElement->GetAttrInteger("FX_FillMode");
    CXML_Element * matrixElement = _element->GetElement(2);
    _FX_RETURN_VALUE_IF_FAIL(matrixElement, FALSE);
    matrix = matrixElement->GetAttrInteger("CFX_Matrix *");
    return TRUE;
}
FX_BOOL CFX_MetafileInterpreter::ParamOf_DrawImage(int32_t & source,
        int32_t & point,
        int32_t & matrix)
{
    _FX_RETURN_VALUE_IF_FAIL(_element, FALSE);
    CXML_Element * sourceElement = _element->GetElement(0);
    _FX_RETURN_VALUE_IF_FAIL(sourceElement, FALSE);
    source = sourceElement->GetAttrInteger("CFX_DIBSource *");
    CXML_Element * pointElement = _element->GetElement(1);
    _FX_RETURN_VALUE_IF_FAIL(pointElement, FALSE);
    point = pointElement->GetAttrInteger("CFX_PointF *");
    CXML_Element * matrixElement = _element->GetElement(2);
    _FX_RETURN_VALUE_IF_FAIL(matrixElement, FALSE);
    matrix = matrixElement->GetAttrInteger("CFX_Matrix *");
    return TRUE;
}
FX_BOOL CFX_MetafileInterpreter::ParamOf_StretchImage(int32_t & source,
        int32_t & rect,
        int32_t & matrix)
{
    _FX_RETURN_VALUE_IF_FAIL(_element, FALSE);
    CXML_Element * sourceElement = _element->GetElement(0);
    _FX_RETURN_VALUE_IF_FAIL(sourceElement, FALSE);
    source = sourceElement->GetAttrInteger("CFX_DIBSource *");
    CXML_Element * rectElement = _element->GetElement(1);
    _FX_RETURN_VALUE_IF_FAIL(rectElement, FALSE);
    rect = rectElement->GetAttrInteger("CFX_RectF *");
    CXML_Element * matrixElement = _element->GetElement(2);
    _FX_RETURN_VALUE_IF_FAIL(matrixElement, FALSE);
    matrix = matrixElement->GetAttrInteger("CFX_Matrix *");
    return TRUE;
}
FX_BOOL CFX_MetafileInterpreter::ParamOf_ConcatMatrix(int32_t & matrix)
{
    _FX_RETURN_VALUE_IF_FAIL(_element, FALSE);
    CXML_Element * matrixElement = _element->GetElement(0);
    _FX_RETURN_VALUE_IF_FAIL(matrixElement, FALSE);
    matrix = matrixElement->GetAttrInteger("CFX_Matrix *");
    return TRUE;
}
FX_BOOL CFX_MetafileInterpreter::ParamOf_SetClipRect(int32_t & rect)
{
    _FX_RETURN_VALUE_IF_FAIL(_element, FALSE);
    CXML_Element * rectElement = _element->GetElement(0);
    _FX_RETURN_VALUE_IF_FAIL(rectElement, FALSE);
    rect = rectElement->GetAttrInteger("CFX_RectF *");
    return TRUE;
}
FX_BOOL CFX_MetafileInterpreter::ParamOf_SetFont(int32_t & font)
{
    _FX_RETURN_VALUE_IF_FAIL(_element, FALSE);
    CXML_Element * fontElement = _element->GetElement(0);
    _FX_RETURN_VALUE_IF_FAIL(fontElement, FALSE);
    font = fontElement->GetAttrInteger("CFX_Font *");
    return TRUE;
}
FX_BOOL CFX_MetafileInterpreter::ParamOf_SetFontSize(FX_FLOAT & size)
{
    _FX_RETURN_VALUE_IF_FAIL(_element, FALSE);
    CXML_Element * sizeElement = _element->GetElement(0);
    _FX_RETURN_VALUE_IF_FAIL(sizeElement, FALSE);
    size = sizeElement->GetAttrFloat("FX_FLOAT");
    return TRUE;
}
FX_BOOL CFX_MetafileInterpreter::ParamOf_SetFontHScale(FX_FLOAT & scale)
{
    _FX_RETURN_VALUE_IF_FAIL(_element, FALSE);
    CXML_Element * scaleElement = _element->GetElement(0);
    _FX_RETURN_VALUE_IF_FAIL(scaleElement, FALSE);
    scale = scaleElement->GetAttrFloat("FX_FLOAT");
    return TRUE;
}
FX_BOOL CFX_MetafileInterpreter::ParamOf_SetCharSpacing(FX_FLOAT & spacing)
{
    _FX_RETURN_VALUE_IF_FAIL(_element, FALSE);
    CXML_Element * spacingElement = _element->GetElement(0);
    _FX_RETURN_VALUE_IF_FAIL(spacingElement, FALSE);
    spacing = spacingElement->GetAttrFloat("FX_FLOAT");
    return TRUE;
}
FX_BOOL CFX_MetafileInterpreter::ParamOf_SetTextDrawingMode(int32_t & mode)
{
    _FX_RETURN_VALUE_IF_FAIL(_element, FALSE);
    CXML_Element * modeElement = _element->GetElement(0);
    _FX_RETURN_VALUE_IF_FAIL(modeElement, FALSE);
    mode = modeElement->GetAttrInteger("int32_t");
    return TRUE;
}
FX_BOOL CFX_MetafileInterpreter::ParamOf_SetText(int32_t &	point,
        int32_t &	text,
        int32_t &	matrix)
{
    _FX_RETURN_VALUE_IF_FAIL(_element, FALSE);
    CXML_Element * pointElement = _element->GetElement(0);
    _FX_RETURN_VALUE_IF_FAIL(pointElement, FALSE);
    point = pointElement->GetAttrInteger("CFX_PointF *");
    CXML_Element * textElement = _element->GetElement(1);
    _FX_RETURN_VALUE_IF_FAIL(textElement, FALSE);
    text = textElement->GetAttrInteger("CFX_WideString *");
    CXML_Element * matrixElement = _element->GetElement(2);
    _FX_RETURN_VALUE_IF_FAIL(matrixElement, FALSE);
    matrix = matrixElement->GetAttrInteger("CFX_Matrix *");
    return TRUE;
}
FX_BOOL CFX_MetafileInterpreter::ParamOf_Transfer(int32_t & graphics, int32_t & matrix)
{
    _FX_RETURN_VALUE_IF_FAIL(_element, FALSE);
    CXML_Element * graphicsElement = _element->GetElement(0);
    _FX_RETURN_VALUE_IF_FAIL(graphicsElement, FALSE);
    graphics = graphicsElement->GetAttrInteger("CFX_Graphics *");
    CXML_Element * matrixElement = _element->GetElement(1);
    _FX_RETURN_VALUE_IF_FAIL(matrixElement, FALSE);
    matrix = matrixElement->GetAttrInteger("CFX_Matrix *");
    return TRUE;
}
FX_BOOL CFX_MetafileInterpreter::ParamOf_Transfer(int32_t & graphics,
        FX_FLOAT & srcLeft,
        FX_FLOAT & srcTop,
        int32_t & dstRect,
        int32_t & matrix)
{
    _FX_RETURN_VALUE_IF_FAIL(_element, FALSE);
    CXML_Element * graphicsElement = _element->GetElement(0);
    _FX_RETURN_VALUE_IF_FAIL(graphicsElement, FALSE);
    graphics = graphicsElement->GetAttrInteger("CFX_Graphics *");
    CXML_Element * srcLeftElement = _element->GetElement(2);
    _FX_RETURN_VALUE_IF_FAIL(srcLeftElement, FALSE);
    srcLeft = srcLeftElement->GetAttrFloat("FX_FLOAT");
    CXML_Element * srcTopElement = _element->GetElement(3);
    _FX_RETURN_VALUE_IF_FAIL(srcTopElement, FALSE);
    srcTop = srcTopElement->GetAttrFloat("FX_FLOAT");
    CXML_Element * dstRectElement = _element->GetElement(1);
    _FX_RETURN_VALUE_IF_FAIL(dstRectElement, FALSE);
    dstRect = dstRectElement->GetAttrInteger("CFX_RectF *");
    CXML_Element * matrixElement = _element->GetElement(4);
    _FX_RETURN_VALUE_IF_FAIL(matrixElement, FALSE);
    matrix = matrixElement->GetAttrInteger("CFX_Matrix *");
    return TRUE;
}
