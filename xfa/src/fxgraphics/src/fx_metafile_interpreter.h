// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef __H_FX_METAFILE_INTERPRETER__
#define __H_FX_METAFILE_INTERPRETER__
class CFX_MetafileInterpreter : public CFX_Object
{
public:
    CFX_MetafileInterpreter();
    virtual ~CFX_MetafileInterpreter();

    FX_BOOL SetCurrentElement(CXML_Element * element);

    FX_BOOL ParamOf_SetLineCap(FX_INT32 & lineCap);
    FX_BOOL ParamOf_SetLineDash(FX_FLOAT & dashPhase,
                                FX_INT32 & dashArray,
                                FX_INT32 & dashCount);
    FX_BOOL ParamOf_SetLineDash(FX_INT32 & dashStyle);
    FX_BOOL ParamOf_SetLineJoin(FX_INT32 & lineJoin);
    FX_BOOL ParamOf_SetMiterLimit(FX_FLOAT & miterLimit);
    FX_BOOL ParamOf_SetLineWidth(FX_FLOAT & lineWidth, FX_INT32 & isActOnDash);

    FX_BOOL ParamOf_SetStrokeColor(FX_INT32 & color);
    FX_BOOL ParamOf_SetFillColor(FX_INT32 & color);

    FX_BOOL ParamOf_StrokePath(FX_INT32 & path, FX_INT32 & matrix);
    FX_BOOL ParamOf_FillPath(FX_INT32 & path, FX_INT32 & fillMode, FX_INT32 & matrix);
    FX_BOOL ParamOf_ClipPath(FX_INT32 & path, FX_INT32 & fillMode, FX_INT32 & matrix);

    FX_BOOL ParamOf_DrawImage(FX_INT32 & source, FX_INT32 & point, FX_INT32 & matrix);
    FX_BOOL ParamOf_StretchImage(FX_INT32 & source, FX_INT32 & rect, FX_INT32 & matrix);

    FX_BOOL ParamOf_ConcatMatrix(FX_INT32 & matrix);
    FX_BOOL ParamOf_SetClipRect(FX_INT32 & rect);
    FX_BOOL ParamOf_SetFont(FX_INT32 & font);
    FX_BOOL ParamOf_SetFontSize(FX_FLOAT & size);
    FX_BOOL ParamOf_SetFontHScale(FX_FLOAT & scale);
    FX_BOOL ParamOf_SetCharSpacing(FX_FLOAT & spacing);
    FX_BOOL ParamOf_SetTextDrawingMode(FX_INT32 & mode);
    FX_BOOL ParamOf_SetText(FX_INT32 & point, FX_INT32 & text, FX_INT32 & matrix);
    FX_BOOL ParamOf_Transfer(FX_INT32 & graphics, FX_INT32 & matrix);
    FX_BOOL ParamOf_Transfer(FX_INT32 &	graphics,
                             FX_FLOAT &	srcLeft,
                             FX_FLOAT &	srcTop,
                             FX_INT32 &	dstRect,
                             FX_INT32 &	matrix);

private:
    CXML_Element *	_element;
};
#endif
