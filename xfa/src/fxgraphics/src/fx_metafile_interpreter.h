// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef __H_FX_METAFILE_INTERPRETER__
#define __H_FX_METAFILE_INTERPRETER__
class CFX_MetafileInterpreter {
 public:
  CFX_MetafileInterpreter();
  virtual ~CFX_MetafileInterpreter();

  FX_BOOL SetCurrentElement(CXML_Element* element);

  FX_BOOL ParamOf_SetLineCap(int32_t& lineCap);
  FX_BOOL ParamOf_SetLineDash(FX_FLOAT& dashPhase,
                              int32_t& dashArray,
                              int32_t& dashCount);
  FX_BOOL ParamOf_SetLineDash(int32_t& dashStyle);
  FX_BOOL ParamOf_SetLineJoin(int32_t& lineJoin);
  FX_BOOL ParamOf_SetMiterLimit(FX_FLOAT& miterLimit);
  FX_BOOL ParamOf_SetLineWidth(FX_FLOAT& lineWidth, int32_t& isActOnDash);

  FX_BOOL ParamOf_SetStrokeColor(int32_t& color);
  FX_BOOL ParamOf_SetFillColor(int32_t& color);

  FX_BOOL ParamOf_StrokePath(int32_t& path, int32_t& matrix);
  FX_BOOL ParamOf_FillPath(int32_t& path, int32_t& fillMode, int32_t& matrix);
  FX_BOOL ParamOf_ClipPath(int32_t& path, int32_t& fillMode, int32_t& matrix);

  FX_BOOL ParamOf_DrawImage(int32_t& source, int32_t& point, int32_t& matrix);
  FX_BOOL ParamOf_StretchImage(int32_t& source, int32_t& rect, int32_t& matrix);

  FX_BOOL ParamOf_ConcatMatrix(int32_t& matrix);
  FX_BOOL ParamOf_SetClipRect(int32_t& rect);
  FX_BOOL ParamOf_SetFont(int32_t& font);
  FX_BOOL ParamOf_SetFontSize(FX_FLOAT& size);
  FX_BOOL ParamOf_SetFontHScale(FX_FLOAT& scale);
  FX_BOOL ParamOf_SetCharSpacing(FX_FLOAT& spacing);
  FX_BOOL ParamOf_SetTextDrawingMode(int32_t& mode);
  FX_BOOL ParamOf_SetText(int32_t& point, int32_t& text, int32_t& matrix);
  FX_BOOL ParamOf_Transfer(int32_t& graphics, int32_t& matrix);
  FX_BOOL ParamOf_Transfer(int32_t& graphics,
                           FX_FLOAT& srcLeft,
                           FX_FLOAT& srcTop,
                           int32_t& dstRect,
                           int32_t& matrix);

 private:
  CXML_Element* _element;
};
#endif
