// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <memory>

#include "fx_path_generator.h"
#include "pre.h"

class CAGG_Graphics {
 public:
  CAGG_Graphics();
  FX_ERR Create(CFX_Graphics* owner,
                int32_t width,
                int32_t height,
                FXDIB_Format format);
  virtual ~CAGG_Graphics();

 private:
  CFX_Graphics* _owner;
};
CFX_Graphics::CFX_Graphics() {
  _type = FX_CONTEXT_None;
  _info._graphState.SetDashCount(0);
  _info._isAntialiasing = TRUE;
  _info._strokeAlignment = FX_STROKEALIGNMENT_Center;
  _info._CTM.SetIdentity();
  _info._isActOnDash = FALSE;
  _info._strokeColor = NULL;
  _info._fillColor = NULL;
  _info._font = NULL;
  _info._fontSize = 40.0;
  _info._fontHScale = 1.0;
  _info._fontSpacing = 0.0;
  _renderDevice = NULL;
  _aggGraphics = NULL;
}
FX_ERR CFX_Graphics::Create(CFX_RenderDevice* renderDevice,
                            FX_BOOL isAntialiasing) {
  _FX_RETURN_VALUE_IF_FAIL(renderDevice, FX_ERR_Parameter_Invalid);
  if (_type != FX_CONTEXT_None) {
    return FX_ERR_Property_Invalid;
  }
  _type = FX_CONTEXT_Device;
  _info._isAntialiasing = isAntialiasing;
  _renderDevice = renderDevice;
  if (_renderDevice->GetDeviceCaps(FXDC_RENDER_CAPS) & FXRC_SOFT_CLIP) {
    return FX_ERR_Succeeded;
  }
  return FX_ERR_Indefinite;
}
FX_ERR CFX_Graphics::Create(int32_t width,
                            int32_t height,
                            FXDIB_Format format,
                            FX_BOOL isNative,
                            FX_BOOL isAntialiasing) {
  if (_type != FX_CONTEXT_None) {
    return FX_ERR_Property_Invalid;
  }
  _type = FX_CONTEXT_Device;
  _info._isAntialiasing = isAntialiasing;
  {
    _aggGraphics = new CAGG_Graphics;
    return _aggGraphics->Create(this, width, height, format);
  }
}
CFX_Graphics::~CFX_Graphics() {
  if (_aggGraphics) {
    delete _aggGraphics;
    _aggGraphics = NULL;
  }
  _renderDevice = NULL;
  _info._graphState.SetDashCount(0);
  _type = FX_CONTEXT_None;
}
FX_ERR CFX_Graphics::GetDeviceCap(const int32_t capID, FX_DeviceCap& capVal) {
  switch (_type) {
    case FX_CONTEXT_Device: {
      _FX_RETURN_VALUE_IF_FAIL(_renderDevice, FX_ERR_Property_Invalid);
      capVal = _renderDevice->GetDeviceCaps(capID);
      return FX_ERR_Succeeded;
    }
    default: { return FX_ERR_Property_Invalid; }
  }
}
FX_ERR CFX_Graphics::IsPrinterDevice(FX_BOOL& isPrinter) {
  switch (_type) {
    case FX_CONTEXT_Device: {
      _FX_RETURN_VALUE_IF_FAIL(_renderDevice, FX_ERR_Property_Invalid);
      int32_t deviceClass = _renderDevice->GetDeviceClass();
      if (deviceClass == FXDC_PRINTER) {
        isPrinter = TRUE;
      } else {
        isPrinter = FALSE;
      }
      return FX_ERR_Succeeded;
    }
    default: { return FX_ERR_Property_Invalid; }
  }
}
FX_ERR CFX_Graphics::EnableAntialiasing(FX_BOOL isAntialiasing) {
  switch (_type) {
    case FX_CONTEXT_Device: {
      _FX_RETURN_VALUE_IF_FAIL(_renderDevice, FX_ERR_Property_Invalid);
      _info._isAntialiasing = isAntialiasing;
      return FX_ERR_Succeeded;
    }
    default: { return FX_ERR_Property_Invalid; }
  }
}
FX_ERR CFX_Graphics::SaveGraphState() {
  switch (_type) {
    case FX_CONTEXT_Device: {
      _FX_RETURN_VALUE_IF_FAIL(_renderDevice, FX_ERR_Property_Invalid);
      _renderDevice->SaveState();
      TInfo* info = new TInfo;
      info->_graphState.Copy(_info._graphState);
      info->_isAntialiasing = _info._isAntialiasing;
      info->_strokeAlignment = _info._strokeAlignment;
      info->_CTM = _info._CTM;
      info->_isActOnDash = _info._isActOnDash;
      info->_strokeColor = _info._strokeColor;
      info->_fillColor = _info._fillColor;
      info->_font = _info._font;
      info->_fontSize = _info._fontSize;
      info->_fontHScale = _info._fontHScale;
      info->_fontSpacing = _info._fontSpacing;
      _infoStack.Add(info);
      return FX_ERR_Succeeded;
    }
    default: { return FX_ERR_Property_Invalid; }
  }
}
FX_ERR CFX_Graphics::RestoreGraphState() {
  switch (_type) {
    case FX_CONTEXT_Device: {
      _FX_RETURN_VALUE_IF_FAIL(_renderDevice, FX_ERR_Property_Invalid);
      _renderDevice->RestoreState();
      int32_t size = _infoStack.GetSize();
      if (size <= 0) {
        return FX_ERR_Intermediate_Value_Invalid;
      }
      int32_t topIndex = size - 1;
      TInfo* info = (TInfo*)_infoStack.GetAt(topIndex);
      _FX_RETURN_VALUE_IF_FAIL(info, FX_ERR_Intermediate_Value_Invalid);
      _info._graphState.Copy(info->_graphState);
      _info._isAntialiasing = info->_isAntialiasing;
      _info._strokeAlignment = info->_strokeAlignment;
      _info._CTM = info->_CTM;
      _info._isActOnDash = info->_isActOnDash;
      _info._strokeColor = info->_strokeColor;
      _info._fillColor = info->_fillColor;
      _info._font = info->_font;
      _info._fontSize = info->_fontSize;
      _info._fontHScale = info->_fontHScale;
      _info._fontSpacing = info->_fontSpacing;
      delete info;
      info = NULL;
      _infoStack.RemoveAt(topIndex);
      return FX_ERR_Succeeded;
    }
    default: { return FX_ERR_Property_Invalid; }
  }
}
FX_ERR CFX_Graphics::GetLineCap(CFX_GraphStateData::LineCap& lineCap) {
  switch (_type) {
    case FX_CONTEXT_Device: {
      _FX_RETURN_VALUE_IF_FAIL(_renderDevice, FX_ERR_Property_Invalid);
      lineCap = _info._graphState.m_LineCap;
      return FX_ERR_Succeeded;
    }
    default: { return FX_ERR_Property_Invalid; }
  }
}
FX_ERR CFX_Graphics::SetLineCap(CFX_GraphStateData::LineCap lineCap) {
  switch (_type) {
    case FX_CONTEXT_Device: {
      _FX_RETURN_VALUE_IF_FAIL(_renderDevice, FX_ERR_Property_Invalid);
      _info._graphState.m_LineCap = lineCap;
      return FX_ERR_Succeeded;
    }
    default: { return FX_ERR_Property_Invalid; }
  }
}
FX_ERR CFX_Graphics::GetDashCount(int32_t& dashCount) {
  switch (_type) {
    case FX_CONTEXT_Device: {
      _FX_RETURN_VALUE_IF_FAIL(_renderDevice, FX_ERR_Property_Invalid);
      dashCount = _info._graphState.m_DashCount;
      return FX_ERR_Succeeded;
    }
    default: { return FX_ERR_Property_Invalid; }
  }
}
FX_ERR CFX_Graphics::GetLineDash(FX_FLOAT& dashPhase, FX_FLOAT* dashArray) {
  _FX_RETURN_VALUE_IF_FAIL(dashArray, FX_ERR_Parameter_Invalid);
  switch (_type) {
    case FX_CONTEXT_Device: {
      _FX_RETURN_VALUE_IF_FAIL(_renderDevice, FX_ERR_Property_Invalid);
      dashPhase = _info._graphState.m_DashPhase;
      FXSYS_memcpy(dashArray, _info._graphState.m_DashArray,
                   _info._graphState.m_DashCount * sizeof(FX_FLOAT));
      return FX_ERR_Succeeded;
    }
    default: { return FX_ERR_Property_Invalid; }
  }
}
FX_ERR CFX_Graphics::SetLineDash(FX_FLOAT dashPhase,
                                 FX_FLOAT* dashArray,
                                 int32_t dashCount) {
  if (dashCount > 0 && !dashArray) {
    return FX_ERR_Parameter_Invalid;
  }
  dashCount = dashCount < 0 ? 0 : dashCount;
  switch (_type) {
    case FX_CONTEXT_Device: {
      _FX_RETURN_VALUE_IF_FAIL(_renderDevice, FX_ERR_Property_Invalid);
      FX_FLOAT scale = 1.0;
      if (_info._isActOnDash) {
        scale = _info._graphState.m_LineWidth;
      }
      _info._graphState.m_DashPhase = dashPhase;
      _info._graphState.SetDashCount(dashCount);
      for (int32_t i = 0; i < dashCount; i++) {
        _info._graphState.m_DashArray[i] = dashArray[i] * scale;
      }
      return FX_ERR_Succeeded;
    }
    default: { return FX_ERR_Property_Invalid; }
  }
}
FX_ERR CFX_Graphics::SetLineDash(FX_DashStyle dashStyle) {
  switch (_type) {
    case FX_CONTEXT_Device: {
      _FX_RETURN_VALUE_IF_FAIL(_renderDevice, FX_ERR_Property_Invalid);
      return RenderDeviceSetLineDash(dashStyle);
    }
    default: { return FX_ERR_Property_Invalid; }
  }
}
FX_ERR CFX_Graphics::GetLineJoin(CFX_GraphStateData::LineJoin& lineJoin) {
  switch (_type) {
    case FX_CONTEXT_Device: {
      _FX_RETURN_VALUE_IF_FAIL(_renderDevice, FX_ERR_Property_Invalid);
      lineJoin = _info._graphState.m_LineJoin;
      return FX_ERR_Succeeded;
    }
    default: { return FX_ERR_Property_Invalid; }
  }
}
FX_ERR CFX_Graphics::SetLineJoin(CFX_GraphStateData::LineJoin lineJoin) {
  switch (_type) {
    case FX_CONTEXT_Device: {
      _FX_RETURN_VALUE_IF_FAIL(_renderDevice, FX_ERR_Property_Invalid);
      _info._graphState.m_LineJoin = lineJoin;
      return FX_ERR_Succeeded;
    }
    default: { return FX_ERR_Property_Invalid; }
  }
}
FX_ERR CFX_Graphics::GetMiterLimit(FX_FLOAT& miterLimit) {
  switch (_type) {
    case FX_CONTEXT_Device: {
      _FX_RETURN_VALUE_IF_FAIL(_renderDevice, FX_ERR_Property_Invalid);
      miterLimit = _info._graphState.m_MiterLimit;
      return FX_ERR_Succeeded;
    }
    default: { return FX_ERR_Property_Invalid; }
  }
}
FX_ERR CFX_Graphics::SetMiterLimit(FX_FLOAT miterLimit) {
  switch (_type) {
    case FX_CONTEXT_Device: {
      _FX_RETURN_VALUE_IF_FAIL(_renderDevice, FX_ERR_Property_Invalid);
      _info._graphState.m_MiterLimit = miterLimit;
      return FX_ERR_Succeeded;
    }
    default: { return FX_ERR_Property_Invalid; }
  }
}
FX_ERR CFX_Graphics::GetLineWidth(FX_FLOAT& lineWidth) {
  switch (_type) {
    case FX_CONTEXT_Device: {
      _FX_RETURN_VALUE_IF_FAIL(_renderDevice, FX_ERR_Property_Invalid);
      lineWidth = _info._graphState.m_LineWidth;
      return FX_ERR_Succeeded;
    }
    default: { return FX_ERR_Property_Invalid; }
  }
}
FX_ERR CFX_Graphics::SetLineWidth(FX_FLOAT lineWidth, FX_BOOL isActOnDash) {
  switch (_type) {
    case FX_CONTEXT_Device: {
      _FX_RETURN_VALUE_IF_FAIL(_renderDevice, FX_ERR_Property_Invalid);
      _info._graphState.m_LineWidth = lineWidth;
      _info._isActOnDash = isActOnDash;
      return FX_ERR_Succeeded;
    }
    default: { return FX_ERR_Property_Invalid; }
  }
}
FX_ERR CFX_Graphics::GetStrokeAlignment(FX_StrokeAlignment& strokeAlignment) {
  switch (_type) {
    case FX_CONTEXT_Device: {
      _FX_RETURN_VALUE_IF_FAIL(_renderDevice, FX_ERR_Property_Invalid);
      strokeAlignment = _info._strokeAlignment;
      return FX_ERR_Succeeded;
    }
    default: { return FX_ERR_Property_Invalid; }
  }
}
FX_ERR CFX_Graphics::SetStrokeAlignment(FX_StrokeAlignment strokeAlignment) {
  switch (_type) {
    case FX_CONTEXT_Device: {
      _FX_RETURN_VALUE_IF_FAIL(_renderDevice, FX_ERR_Property_Invalid);
      _info._strokeAlignment = strokeAlignment;
      return FX_ERR_Succeeded;
    }
    default: { return FX_ERR_Property_Invalid; }
  }
}
FX_ERR CFX_Graphics::SetStrokeColor(CFX_Color* color) {
  _FX_RETURN_VALUE_IF_FAIL(color, FX_ERR_Parameter_Invalid);
  switch (_type) {
    case FX_CONTEXT_Device: {
      _FX_RETURN_VALUE_IF_FAIL(_renderDevice, FX_ERR_Property_Invalid);
      _info._strokeColor = color;
      return FX_ERR_Succeeded;
    }
    default: { return FX_ERR_Property_Invalid; }
  }
}
FX_ERR CFX_Graphics::SetFillColor(CFX_Color* color) {
  _FX_RETURN_VALUE_IF_FAIL(color, FX_ERR_Parameter_Invalid);
  switch (_type) {
    case FX_CONTEXT_Device: {
      _FX_RETURN_VALUE_IF_FAIL(_renderDevice, FX_ERR_Property_Invalid);
      _info._fillColor = color;
      return FX_ERR_Succeeded;
    }
    default: { return FX_ERR_Property_Invalid; }
  }
}
FX_ERR CFX_Graphics::StrokePath(CFX_Path* path, CFX_Matrix* matrix) {
  _FX_RETURN_VALUE_IF_FAIL(path, FX_ERR_Parameter_Invalid);
  switch (_type) {
    case FX_CONTEXT_Device: {
      _FX_RETURN_VALUE_IF_FAIL(_renderDevice, FX_ERR_Property_Invalid);
      return RenderDeviceStrokePath(path, matrix);
    }
    default: { return FX_ERR_Property_Invalid; }
  }
}
FX_ERR CFX_Graphics::FillPath(CFX_Path* path,
                              FX_FillMode fillMode,
                              CFX_Matrix* matrix) {
  _FX_RETURN_VALUE_IF_FAIL(path, FX_ERR_Parameter_Invalid);
  switch (_type) {
    case FX_CONTEXT_Device: {
      _FX_RETURN_VALUE_IF_FAIL(_renderDevice, FX_ERR_Property_Invalid);
      return RenderDeviceFillPath(path, fillMode, matrix);
    }
    default: { return FX_ERR_Property_Invalid; }
  }
}
FX_ERR CFX_Graphics::ClipPath(CFX_Path* path,
                              FX_FillMode fillMode,
                              CFX_Matrix* matrix) {
  _FX_RETURN_VALUE_IF_FAIL(path, FX_ERR_Parameter_Invalid);
  switch (_type) {
    case FX_CONTEXT_Device: {
      _FX_RETURN_VALUE_IF_FAIL(_renderDevice, FX_ERR_Property_Invalid);
      FX_BOOL result = _renderDevice->SetClip_PathFill(
          path->GetPathData(), (CFX_Matrix*)matrix, fillMode);
      _FX_RETURN_VALUE_IF_FAIL(result, FX_ERR_Indefinite);
      return FX_ERR_Succeeded;
    }
    default: { return FX_ERR_Property_Invalid; }
  }
}
FX_ERR CFX_Graphics::DrawImage(CFX_DIBSource* source,
                               const CFX_PointF& point,
                               CFX_Matrix* matrix) {
  _FX_RETURN_VALUE_IF_FAIL(source, FX_ERR_Parameter_Invalid);
  switch (_type) {
    case FX_CONTEXT_Device: {
      _FX_RETURN_VALUE_IF_FAIL(_renderDevice, FX_ERR_Property_Invalid);
      return RenderDeviceDrawImage(source, point, matrix);
    }
    default: { return FX_ERR_Property_Invalid; }
  }
}
FX_ERR CFX_Graphics::StretchImage(CFX_DIBSource* source,
                                  const CFX_RectF& rect,
                                  CFX_Matrix* matrix) {
  _FX_RETURN_VALUE_IF_FAIL(source, FX_ERR_Parameter_Invalid);
  switch (_type) {
    case FX_CONTEXT_Device: {
      _FX_RETURN_VALUE_IF_FAIL(_renderDevice, FX_ERR_Property_Invalid);
      return RenderDeviceStretchImage(source, rect, matrix);
    }
    default: { return FX_ERR_Property_Invalid; }
  }
}
FX_ERR CFX_Graphics::ConcatMatrix(const CFX_Matrix* matrix) {
  _FX_RETURN_VALUE_IF_FAIL(matrix, FX_ERR_Parameter_Invalid);
  switch (_type) {
    case FX_CONTEXT_Device: {
      _FX_RETURN_VALUE_IF_FAIL(_renderDevice, FX_ERR_Property_Invalid);
      _info._CTM.Concat(*matrix);
      return FX_ERR_Succeeded;
    }
    default: { return FX_ERR_Property_Invalid; }
  }
}
CFX_Matrix* CFX_Graphics::GetMatrix() {
  switch (_type) {
    case FX_CONTEXT_Device: {
      _FX_RETURN_VALUE_IF_FAIL(_renderDevice, NULL);
      return &_info._CTM;
    }
    default: { return NULL; }
  }
}
FX_ERR CFX_Graphics::GetClipRect(CFX_RectF& rect) {
  switch (_type) {
    case FX_CONTEXT_Device: {
      _FX_RETURN_VALUE_IF_FAIL(_renderDevice, FX_ERR_Property_Invalid);
      FX_RECT r = _renderDevice->GetClipBox();
      rect.left = (FX_FLOAT)r.left;
      rect.top = (FX_FLOAT)r.top;
      rect.width = (FX_FLOAT)r.Width();
      rect.height = (FX_FLOAT)r.Height();
      return FX_ERR_Succeeded;
    }
    default: { return FX_ERR_Property_Invalid; }
  }
}
FX_ERR CFX_Graphics::SetClipRect(const CFX_RectF& rect) {
  switch (_type) {
    case FX_CONTEXT_Device: {
      _FX_RETURN_VALUE_IF_FAIL(_renderDevice, FX_ERR_Property_Invalid);
      FX_RECT r(FXSYS_round(rect.left), FXSYS_round(rect.top),
                FXSYS_round(rect.right()), FXSYS_round(rect.bottom()));
      FX_BOOL result = _renderDevice->SetClip_Rect(&r);
      _FX_RETURN_VALUE_IF_FAIL(result, FX_ERR_Method_Not_Supported);
      return FX_ERR_Succeeded;
    }
    default: { return FX_ERR_Property_Invalid; }
  }
}
FX_ERR CFX_Graphics::ClearClip() {
  switch (_type) {
    case FX_CONTEXT_Device: {
      _FX_RETURN_VALUE_IF_FAIL(_renderDevice, FX_ERR_Property_Invalid);
      FX_BOOL result = FX_ERR_Succeeded;
      _FX_RETURN_VALUE_IF_FAIL(result, FX_ERR_Method_Not_Supported);
      return FX_ERR_Succeeded;
    }
    default: { return FX_ERR_Property_Invalid; }
  }
}
FX_ERR CFX_Graphics::SetFont(CFX_Font* font) {
  _FX_RETURN_VALUE_IF_FAIL(font, FX_ERR_Parameter_Invalid);
  switch (_type) {
    case FX_CONTEXT_Device: {
      _FX_RETURN_VALUE_IF_FAIL(_renderDevice, FX_ERR_Property_Invalid);
      _info._font = font;
      return FX_ERR_Succeeded;
    }
    default: { return FX_ERR_Property_Invalid; }
  }
}
FX_ERR CFX_Graphics::SetFontSize(const FX_FLOAT size) {
  FX_FLOAT fontSize = size <= 0 ? 1.0f : size;
  switch (_type) {
    case FX_CONTEXT_Device: {
      _FX_RETURN_VALUE_IF_FAIL(_renderDevice, FX_ERR_Property_Invalid);
      _info._fontSize = fontSize;
      return FX_ERR_Succeeded;
    }
    default: { return FX_ERR_Property_Invalid; }
  }
}
FX_ERR CFX_Graphics::SetFontHScale(const FX_FLOAT scale) {
  FX_FLOAT fontHScale = scale <= 0 ? 1.0f : scale;
  switch (_type) {
    case FX_CONTEXT_Device: {
      _FX_RETURN_VALUE_IF_FAIL(_renderDevice, FX_ERR_Property_Invalid);
      _info._fontHScale = fontHScale;
      return FX_ERR_Succeeded;
    }
    default: { return FX_ERR_Property_Invalid; }
  }
}
FX_ERR CFX_Graphics::SetCharSpacing(const FX_FLOAT spacing) {
  FX_FLOAT fontSpacing = spacing < 0 ? 0 : spacing;
  switch (_type) {
    case FX_CONTEXT_Device: {
      _FX_RETURN_VALUE_IF_FAIL(_renderDevice, FX_ERR_Property_Invalid);
      _info._fontSpacing = fontSpacing;
      return FX_ERR_Succeeded;
    }
    default: { return FX_ERR_Property_Invalid; }
  }
}
FX_ERR CFX_Graphics::SetTextDrawingMode(const int32_t mode) {
  switch (_type) {
    case FX_CONTEXT_Device: {
      _FX_RETURN_VALUE_IF_FAIL(_renderDevice, FX_ERR_Property_Invalid);
      return FX_ERR_Succeeded;
    }
    default: { return FX_ERR_Property_Invalid; }
  }
}
FX_ERR CFX_Graphics::ShowText(const CFX_PointF& point,
                              const CFX_WideString& text,
                              CFX_Matrix* matrix) {
  switch (_type) {
    case FX_CONTEXT_Device: {
      _FX_RETURN_VALUE_IF_FAIL(_renderDevice, FX_ERR_Property_Invalid);
      return RenderDeviceShowText(point, text, matrix);
    }
    default: { return FX_ERR_Property_Invalid; }
  }
}
FX_ERR CFX_Graphics::CalcTextRect(CFX_RectF& rect,
                                  const CFX_WideString& text,
                                  FX_BOOL isMultiline,
                                  CFX_Matrix* matrix) {
  switch (_type) {
    case FX_CONTEXT_Device: {
      _FX_RETURN_VALUE_IF_FAIL(_renderDevice, FX_ERR_Property_Invalid);
      int32_t length = text.GetLength();
      FX_DWORD* charCodes = FX_Alloc(FX_DWORD, length);
      FXTEXT_CHARPOS* charPos = FX_Alloc(FXTEXT_CHARPOS, length);
      CalcTextInfo(text, charCodes, charPos, rect);
      FX_Free(charPos);
      FX_Free(charCodes);
      return FX_ERR_Succeeded;
    }
    default: { return FX_ERR_Property_Invalid; }
  }
}
FX_ERR CFX_Graphics::Transfer(CFX_Graphics* graphics,
                              const CFX_Matrix* matrix) {
  _FX_RETURN_VALUE_IF_FAIL(graphics, FX_ERR_Parameter_Invalid);
  CFX_Matrix m;
  m.Set(_info._CTM.a, _info._CTM.b, _info._CTM.c, _info._CTM.d, _info._CTM.e,
        _info._CTM.f);
  if (matrix) {
    m.Concat(*matrix);
  }
  switch (_type) {
    case FX_CONTEXT_Device: {
      _FX_RETURN_VALUE_IF_FAIL(_renderDevice, FX_ERR_Property_Invalid);
      {
        _FX_RETURN_VALUE_IF_FAIL(graphics->_renderDevice,
                                 FX_ERR_Parameter_Invalid);
        CFX_DIBitmap* bitmap = graphics->_renderDevice->GetBitmap();
        FX_BOOL result = _renderDevice->SetDIBits(bitmap, 0, 0);
        _FX_RETURN_VALUE_IF_FAIL(result, FX_ERR_Method_Not_Supported);
      }
    }
    default: { return FX_ERR_Property_Invalid; }
  }
}
FX_ERR CFX_Graphics::Transfer(CFX_Graphics* graphics,
                              FX_FLOAT srcLeft,
                              FX_FLOAT srcTop,
                              const CFX_RectF& dstRect,
                              const CFX_Matrix* matrix) {
  _FX_RETURN_VALUE_IF_FAIL(graphics, FX_ERR_Parameter_Invalid);
  CFX_Matrix m;
  m.Set(_info._CTM.a, _info._CTM.b, _info._CTM.c, _info._CTM.d, _info._CTM.e,
        _info._CTM.f);
  if (matrix) {
    m.Concat(*matrix);
  }
  switch (_type) {
    case FX_CONTEXT_Device: {
      _FX_RETURN_VALUE_IF_FAIL(_renderDevice, FX_ERR_Property_Invalid);
      {
        _FX_RETURN_VALUE_IF_FAIL(graphics->_renderDevice,
                                 FX_ERR_Parameter_Invalid);
        CFX_DIBitmap* bitmap = graphics->_renderDevice->GetBitmap();
        FX_BOOL result = FX_ERR_Indefinite;
        CFX_DIBitmap bmp;
        result = bmp.Create((int32_t)dstRect.width, (int32_t)dstRect.height,
                            bitmap->GetFormat());
        _FX_RETURN_VALUE_IF_FAIL(result, FX_ERR_Intermediate_Value_Invalid);
        result = graphics->_renderDevice->GetDIBits(&bmp, (int32_t)srcLeft,
                                                    (int32_t)srcTop);
        _FX_RETURN_VALUE_IF_FAIL(result, FX_ERR_Method_Not_Supported);
        result = _renderDevice->SetDIBits(&bmp, (int32_t)dstRect.left,
                                          (int32_t)dstRect.top);
        _FX_RETURN_VALUE_IF_FAIL(result, FX_ERR_Method_Not_Supported);
        return FX_ERR_Succeeded;
      }
    }
    default: { return FX_ERR_Property_Invalid; }
  }
}
CFX_RenderDevice* CFX_Graphics::GetRenderDevice() {
  return _renderDevice;
}
FX_ERR CFX_Graphics::InverseRect(const CFX_RectF& rect) {
  _FX_RETURN_VALUE_IF_FAIL(_renderDevice, FX_ERR_Property_Invalid);
  CFX_DIBitmap* bitmap = _renderDevice->GetBitmap();
  _FX_RETURN_VALUE_IF_FAIL(bitmap, FX_ERR_Property_Invalid);
  CFX_RectF temp(rect);
  _info._CTM.TransformRect(temp);
  CFX_RectF r;
  r.Set(0, 0, (FX_FLOAT)bitmap->GetWidth(), (FX_FLOAT)bitmap->GetWidth());
  r.Intersect(temp);
  if (r.IsEmpty()) {
    return FX_ERR_Parameter_Invalid;
  }
  FX_ARGB* pBuf =
      (FX_ARGB*)(bitmap->GetBuffer() + int32_t(r.top) * bitmap->GetPitch());
  int32_t bottom = (int32_t)r.bottom();
  int32_t right = (int32_t)r.right();
  for (int32_t i = (int32_t)r.top; i < bottom; i++) {
    FX_ARGB* pLine = pBuf + (int32_t)r.left;
    for (int32_t j = (int32_t)r.left; j < right; j++) {
      FX_ARGB c = *pLine;
      *pLine++ = (c & 0xFF000000) | (0xFFFFFF - (c & 0x00FFFFFF));
    }
    pBuf = (FX_ARGB*)((uint8_t*)pBuf + bitmap->GetPitch());
  }
  return FX_ERR_Succeeded;
}
FX_ERR CFX_Graphics::XorDIBitmap(const CFX_DIBitmap* srcBitmap,
                                 const CFX_RectF& rect) {
  _FX_RETURN_VALUE_IF_FAIL(_renderDevice, FX_ERR_Property_Invalid);
  CFX_DIBitmap* dst = _renderDevice->GetBitmap();
  _FX_RETURN_VALUE_IF_FAIL(dst, FX_ERR_Property_Invalid);
  CFX_RectF temp(rect);
  _info._CTM.TransformRect(temp);
  CFX_RectF r;
  r.Set(0, 0, (FX_FLOAT)dst->GetWidth(), (FX_FLOAT)dst->GetWidth());
  r.Intersect(temp);
  if (r.IsEmpty()) {
    return FX_ERR_Parameter_Invalid;
  }
  FX_ARGB* pSrcBuf = (FX_ARGB*)(srcBitmap->GetBuffer() +
                                int32_t(r.top) * srcBitmap->GetPitch());
  FX_ARGB* pDstBuf =
      (FX_ARGB*)(dst->GetBuffer() + int32_t(r.top) * dst->GetPitch());
  int32_t bottom = (int32_t)r.bottom();
  int32_t right = (int32_t)r.right();
  for (int32_t i = (int32_t)r.top; i < bottom; i++) {
    FX_ARGB* pSrcLine = pSrcBuf + (int32_t)r.left;
    FX_ARGB* pDstLine = pDstBuf + (int32_t)r.left;
    for (int32_t j = (int32_t)r.left; j < right; j++) {
      FX_ARGB c = *pDstLine;
      *pDstLine++ =
          ArgbEncode(FXARGB_A(c), (c & 0xFFFFFF) ^ (*pSrcLine & 0xFFFFFF));
      pSrcLine++;
    }
    pSrcBuf = (FX_ARGB*)((uint8_t*)pSrcBuf + srcBitmap->GetPitch());
    pDstBuf = (FX_ARGB*)((uint8_t*)pDstBuf + dst->GetPitch());
  }
  return FX_ERR_Succeeded;
}
FX_ERR CFX_Graphics::EqvDIBitmap(const CFX_DIBitmap* srcBitmap,
                                 const CFX_RectF& rect) {
  _FX_RETURN_VALUE_IF_FAIL(_renderDevice, FX_ERR_Property_Invalid);
  CFX_DIBitmap* dst = _renderDevice->GetBitmap();
  _FX_RETURN_VALUE_IF_FAIL(dst, FX_ERR_Property_Invalid);
  CFX_RectF temp(rect);
  _info._CTM.TransformRect(temp);
  CFX_RectF r;
  r.Set(0, 0, (FX_FLOAT)dst->GetWidth(), (FX_FLOAT)dst->GetWidth());
  r.Intersect(temp);
  if (r.IsEmpty()) {
    return FX_ERR_Parameter_Invalid;
  }
  FX_ARGB* pSrcBuf = (FX_ARGB*)(srcBitmap->GetBuffer() +
                                int32_t(r.top) * srcBitmap->GetPitch());
  FX_ARGB* pDstBuf =
      (FX_ARGB*)(dst->GetBuffer() + int32_t(r.top) * dst->GetPitch());
  int32_t bottom = (int32_t)r.bottom();
  int32_t right = (int32_t)r.right();
  for (int32_t i = (int32_t)r.top; i < bottom; i++) {
    FX_ARGB* pSrcLine = pSrcBuf + (int32_t)r.left;
    FX_ARGB* pDstLine = pDstBuf + (int32_t)r.left;
    for (int32_t j = (int32_t)r.left; j < right; j++) {
      FX_ARGB c = *pDstLine;
      *pDstLine++ =
          ArgbEncode(FXARGB_A(c), ~((c & 0xFFFFFF) ^ (*pSrcLine & 0xFFFFFF)));
      pSrcLine++;
    }
    pSrcBuf = (FX_ARGB*)((uint8_t*)pSrcBuf + srcBitmap->GetPitch());
    pDstBuf = (FX_ARGB*)((uint8_t*)pDstBuf + dst->GetPitch());
  }
  return FX_ERR_Succeeded;
}
FX_ERR CFX_Graphics::RenderDeviceSetLineDash(FX_DashStyle dashStyle) {
  switch (dashStyle) {
    case FX_DASHSTYLE_Solid: {
      _info._graphState.SetDashCount(0);
      return FX_ERR_Succeeded;
    }
    case FX_DASHSTYLE_Dash: {
      FX_FLOAT dashArray[] = {3, 1};
      SetLineDash(0, dashArray, 2);
      return FX_ERR_Succeeded;
    }
    case FX_DASHSTYLE_Dot: {
      FX_FLOAT dashArray[] = {1, 1};
      SetLineDash(0, dashArray, 2);
      return FX_ERR_Succeeded;
    }
    case FX_DASHSTYLE_DashDot: {
      FX_FLOAT dashArray[] = {3, 1, 1, 1};
      SetLineDash(0, dashArray, 4);
      return FX_ERR_Succeeded;
    }
    case FX_DASHSTYLE_DashDotDot: {
      FX_FLOAT dashArray[] = {4, 1, 2, 1, 2, 1};
      SetLineDash(0, dashArray, 6);
      return FX_ERR_Succeeded;
    }
    default: { return FX_ERR_Parameter_Invalid; }
  }
}
FX_ERR CFX_Graphics::RenderDeviceStrokePath(CFX_Path* path,
                                            CFX_Matrix* matrix) {
  _FX_RETURN_VALUE_IF_FAIL(_info._strokeColor, FX_ERR_Property_Invalid);
  CFX_Matrix m;
  m.Set(_info._CTM.a, _info._CTM.b, _info._CTM.c, _info._CTM.d, _info._CTM.e,
        _info._CTM.f);
  if (matrix) {
    m.Concat(*matrix);
  }
  switch (_info._strokeColor->_type) {
    case FX_COLOR_Solid: {
      FX_BOOL result = _renderDevice->DrawPath(
          path->GetPathData(), (CFX_Matrix*)&m, &_info._graphState, 0x0,
          _info._strokeColor->_argb, 0);
      _FX_RETURN_VALUE_IF_FAIL(result, FX_ERR_Indefinite);
      return FX_ERR_Succeeded;
    }
    case FX_COLOR_Pattern: {
      return StrokePathWithPattern(path, &m);
    }
    case FX_COLOR_Shading: {
      return StrokePathWithShading(path, &m);
    }
    default: { return FX_ERR_Property_Invalid; }
  }
}
FX_ERR CFX_Graphics::RenderDeviceFillPath(CFX_Path* path,
                                          FX_FillMode fillMode,
                                          CFX_Matrix* matrix) {
  _FX_RETURN_VALUE_IF_FAIL(_info._fillColor, FX_ERR_Property_Invalid);
  CFX_Matrix m;
  m.Set(_info._CTM.a, _info._CTM.b, _info._CTM.c, _info._CTM.d, _info._CTM.e,
        _info._CTM.f);
  if (matrix) {
    m.Concat(*matrix);
  }
  switch (_info._fillColor->_type) {
    case FX_COLOR_Solid: {
      FX_BOOL result = _renderDevice->DrawPath(
          path->GetPathData(), (CFX_Matrix*)&m, &_info._graphState,
          _info._fillColor->_argb, 0x0, fillMode);
      _FX_RETURN_VALUE_IF_FAIL(result, FX_ERR_Indefinite);
      return FX_ERR_Succeeded;
    }
    case FX_COLOR_Pattern: {
      { return FillPathWithPattern(path, fillMode, &m); }
    }
    case FX_COLOR_Shading: {
      { return FillPathWithShading(path, fillMode, &m); }
    }
    default: { return FX_ERR_Property_Invalid; }
  }
}
FX_ERR CFX_Graphics::RenderDeviceDrawImage(CFX_DIBSource* source,
                                           const CFX_PointF& point,
                                           CFX_Matrix* matrix) {
  CFX_Matrix m1;
  m1.Set(_info._CTM.a, _info._CTM.b, _info._CTM.c, _info._CTM.d, _info._CTM.e,
         _info._CTM.f);
  if (matrix) {
    m1.Concat(*matrix);
  }
  CFX_Matrix m2;
  m2.Set((FX_FLOAT)source->GetWidth(), 0.0, 0.0, (FX_FLOAT)source->GetHeight(),
         point.x, point.y);
  m2.Concat(m1);
  int32_t left, top;
  CFX_DIBitmap* bmp1 = source->FlipImage(FALSE, TRUE);
  CFX_DIBitmap* bmp2 = bmp1->TransformTo((CFX_Matrix*)&m2, left, top);
  CFX_RectF r;
  GetClipRect(r);
  FX_ERR result = FX_ERR_Indefinite;
  {
    CFX_DIBitmap* bitmap = _renderDevice->GetBitmap();
    CFX_DIBitmap bmp;
    bmp.Create(bitmap->GetWidth(), bitmap->GetHeight(), FXDIB_Argb);
    _renderDevice->GetDIBits(&bmp, 0, 0);
    bmp.TransferBitmap(FXSYS_round(r.left), FXSYS_round(r.top),
                       FXSYS_round(r.Width()), FXSYS_round(r.Height()), bmp2,
                       FXSYS_round(r.left - left), FXSYS_round(r.top - top));
    _renderDevice->SetDIBits(&bmp, 0, 0);
    result = FX_ERR_Succeeded;
  }
  if (bmp2) {
    delete bmp2;
    bmp2 = NULL;
  }
  if (bmp1) {
    delete bmp1;
    bmp1 = NULL;
  }
  return result;
}
FX_ERR CFX_Graphics::RenderDeviceStretchImage(CFX_DIBSource* source,
                                              const CFX_RectF& rect,
                                              CFX_Matrix* matrix) {
  CFX_Matrix m1;
  m1.Set(_info._CTM.a, _info._CTM.b, _info._CTM.c, _info._CTM.d, _info._CTM.e,
         _info._CTM.f);
  if (matrix) {
    m1.Concat(*matrix);
  }
  CFX_DIBitmap* bmp1 =
      source->StretchTo((int32_t)rect.Width(), (int32_t)rect.Height());
  CFX_Matrix m2;
  m2.Set(rect.Width(), 0.0, 0.0, rect.Height(), rect.left, rect.top);
  m2.Concat(m1);
  int32_t left, top;
  CFX_DIBitmap* bmp2 = bmp1->FlipImage(FALSE, TRUE);
  CFX_DIBitmap* bmp3 = bmp2->TransformTo((CFX_Matrix*)&m2, left, top);
  CFX_RectF r;
  GetClipRect(r);
  FX_ERR result = FX_ERR_Indefinite;
  {
    CFX_DIBitmap* bitmap = _renderDevice->GetBitmap();
    bitmap->CompositeBitmap(FXSYS_round(r.left), FXSYS_round(r.top),
                            FXSYS_round(r.Width()), FXSYS_round(r.Height()),
                            bmp3, FXSYS_round(r.left - left),
                            FXSYS_round(r.top - top));
    result = FX_ERR_Succeeded;
  }
  if (bmp3) {
    delete bmp3;
    bmp3 = NULL;
  }
  if (bmp2) {
    delete bmp2;
    bmp2 = NULL;
  }
  if (bmp1) {
    delete bmp1;
    bmp1 = NULL;
  }
  return result;
}
FX_ERR CFX_Graphics::RenderDeviceShowText(const CFX_PointF& point,
                                          const CFX_WideString& text,
                                          CFX_Matrix* matrix) {
  int32_t length = text.GetLength();
  FX_DWORD* charCodes = FX_Alloc(FX_DWORD, length);
  FXTEXT_CHARPOS* charPos = FX_Alloc(FXTEXT_CHARPOS, length);
  CFX_RectF rect;
  rect.Set(point.x, point.y, 0, 0);
  CalcTextInfo(text, charCodes, charPos, rect);
  CFX_Matrix m;
  m.Set(_info._CTM.a, _info._CTM.b, _info._CTM.c, _info._CTM.d, _info._CTM.e,
        _info._CTM.f);
  m.Translate(0, _info._fontSize * _info._fontHScale);
  if (matrix) {
    m.Concat(*matrix);
  }
  FX_BOOL result = _renderDevice->DrawNormalText(
      length, charPos, _info._font, CFX_GEModule::Get()->GetFontCache(),
      -_info._fontSize * _info._fontHScale, (CFX_Matrix*)&m,
      _info._fillColor->_argb, FXTEXT_CLEARTYPE);
  _FX_RETURN_VALUE_IF_FAIL(result, FX_ERR_Indefinite);
  FX_Free(charPos);
  FX_Free(charCodes);
  return FX_ERR_Succeeded;
}
FX_ERR CFX_Graphics::StrokePathWithPattern(CFX_Path* path, CFX_Matrix* matrix) {
  return FX_ERR_Method_Not_Supported;
}
FX_ERR CFX_Graphics::StrokePathWithShading(CFX_Path* path, CFX_Matrix* matrix) {
  return FX_ERR_Method_Not_Supported;
}
FX_ERR CFX_Graphics::FillPathWithPattern(CFX_Path* path,
                                         FX_FillMode fillMode,
                                         CFX_Matrix* matrix) {
  CFX_Pattern* pattern = _info._fillColor->_pattern;
  CFX_DIBitmap* bitmap = _renderDevice->GetBitmap();
  int32_t width = bitmap->GetWidth();
  int32_t height = bitmap->GetHeight();
  CFX_DIBitmap bmp;
  bmp.Create(width, height, FXDIB_Argb);
  _renderDevice->GetDIBits(&bmp, 0, 0);
  switch (pattern->_type) {
    case FX_PATTERN_Bitmap: {
      int32_t xStep = FXSYS_round(pattern->_x1Step);
      int32_t yStep = FXSYS_round(pattern->_y1Step);
      int32_t xCount = width / xStep + 1;
      int32_t yCount = height / yStep + 1;
      for (int32_t i = 0; i <= yCount; i++) {
        for (int32_t j = 0; j <= xCount; j++) {
          bmp.TransferBitmap(j * xStep, i * yStep, xStep, yStep,
                             pattern->_bitmap, 0, 0);
        }
      }
      break;
    }
    case FX_PATTERN_Hatch: {
      FX_HatchStyle hatchStyle = _info._fillColor->_pattern->_hatchStyle;
      if (hatchStyle < FX_HATCHSTYLE_Horizontal ||
          hatchStyle > FX_HATCHSTYLE_SolidDiamond) {
        return FX_ERR_Intermediate_Value_Invalid;
      }
      const FX_HATCHDATA& data = hatchBitmapData[hatchStyle];
      CFX_DIBitmap mask;
      mask.Create(data.width, data.height, FXDIB_1bppMask);
      FXSYS_memcpy(mask.GetBuffer(), data.maskBits,
                   mask.GetPitch() * data.height);
      CFX_FloatRect rectf = path->GetPathData()->GetBoundingBox();
      if (matrix) {
        rectf.Transform((const CFX_Matrix*)matrix);
      }
      FX_RECT rect(FXSYS_round(rectf.left), FXSYS_round(rectf.top),
                   FXSYS_round(rectf.right), FXSYS_round(rectf.bottom));
      CFX_FxgeDevice device;
      device.Attach(&bmp);
      device.FillRect(&rect, _info._fillColor->_pattern->_backArgb);
      for (int32_t j = rect.bottom; j < rect.top; j += mask.GetHeight()) {
        for (int32_t i = rect.left; i < rect.right; i += mask.GetWidth()) {
          device.SetBitMask(&mask, i, j, _info._fillColor->_pattern->_foreArgb);
        }
      }
      break;
    }
  }
  _renderDevice->SaveState();
  _renderDevice->SetClip_PathFill(path->GetPathData(), (CFX_Matrix*)matrix,
                                  fillMode);
  SetDIBitsWithMatrix(&bmp, &pattern->_matrix);
  _renderDevice->RestoreState();
  return FX_ERR_Succeeded;
}
FX_ERR CFX_Graphics::FillPathWithShading(CFX_Path* path,
                                         FX_FillMode fillMode,
                                         CFX_Matrix* matrix) {
  CFX_DIBitmap* bitmap = _renderDevice->GetBitmap();
  int32_t width = bitmap->GetWidth();
  int32_t height = bitmap->GetHeight();
  FX_FLOAT start_x = _info._fillColor->_shading->_beginPoint.x;
  FX_FLOAT start_y = _info._fillColor->_shading->_beginPoint.y;
  FX_FLOAT end_x = _info._fillColor->_shading->_endPoint.x;
  FX_FLOAT end_y = _info._fillColor->_shading->_endPoint.y;
  CFX_DIBitmap bmp;
  bmp.Create(width, height, FXDIB_Argb);
  _renderDevice->GetDIBits(&bmp, 0, 0);
  int32_t pitch = bmp.GetPitch();
  FX_BOOL result = FALSE;
  switch (_info._fillColor->_shading->_type) {
    case FX_SHADING_Axial: {
      FX_FLOAT x_span = end_x - start_x;
      FX_FLOAT y_span = end_y - start_y;
      FX_FLOAT axis_len_square =
          FXSYS_Mul(x_span, x_span) + FXSYS_Mul(y_span, y_span);
      for (int32_t row = 0; row < height; row++) {
        FX_DWORD* dib_buf = (FX_DWORD*)(bmp.GetBuffer() + row * pitch);
        for (int32_t column = 0; column < width; column++) {
          FX_FLOAT x = (FX_FLOAT)(column);
          FX_FLOAT y = (FX_FLOAT)(row);
          FX_FLOAT scale = FXSYS_Div(
              FXSYS_Mul(x - start_x, x_span) + FXSYS_Mul(y - start_y, y_span),
              axis_len_square);
          if (scale < 0) {
            if (!_info._fillColor->_shading->_isExtendedBegin) {
              continue;
            }
            scale = 0;
          } else if (scale > 1.0f) {
            if (!_info._fillColor->_shading->_isExtendedEnd) {
              continue;
            }
            scale = 1.0f;
          }
          int32_t index = (int32_t)(scale * (FX_SHADING_Steps - 1));
          dib_buf[column] = _info._fillColor->_shading->_argbArray[index];
        }
      }
      result = TRUE;
      break;
    }
    case FX_SHADING_Radial: {
      FX_FLOAT start_r = _info._fillColor->_shading->_beginRadius;
      FX_FLOAT end_r = _info._fillColor->_shading->_endRadius;
      FX_FLOAT a = FXSYS_Mul(start_x - end_x, start_x - end_x) +
                   FXSYS_Mul(start_y - end_y, start_y - end_y) -
                   FXSYS_Mul(start_r - end_r, start_r - end_r);
      for (int32_t row = 0; row < height; row++) {
        FX_DWORD* dib_buf = (FX_DWORD*)(bmp.GetBuffer() + row * pitch);
        for (int32_t column = 0; column < width; column++) {
          FX_FLOAT x = (FX_FLOAT)(column);
          FX_FLOAT y = (FX_FLOAT)(row);
          FX_FLOAT b = -2 * (FXSYS_Mul(x - start_x, end_x - start_x) +
                             FXSYS_Mul(y - start_y, end_y - start_y) +
                             FXSYS_Mul(start_r, end_r - start_r));
          FX_FLOAT c = FXSYS_Mul(x - start_x, x - start_x) +
                       FXSYS_Mul(y - start_y, y - start_y) -
                       FXSYS_Mul(start_r, start_r);
          FX_FLOAT s;
          if (a == 0) {
            s = (FXSYS_Div(-c, b));
          } else {
            FX_FLOAT b2_4ac = FXSYS_Mul(b, b) - 4 * FXSYS_Mul(a, c);
            if (b2_4ac < 0) {
              continue;
            }
            FX_FLOAT root = (FXSYS_sqrt(b2_4ac));
            FX_FLOAT s1, s2;
            if (a > 0) {
              s1 = FXSYS_Div(-b - root, 2 * a);
              s2 = FXSYS_Div(-b + root, 2 * a);
            } else {
              s2 = FXSYS_Div(-b - root, 2 * a);
              s1 = FXSYS_Div(-b + root, 2 * a);
            }
            if (s2 <= 1.0f || _info._fillColor->_shading->_isExtendedEnd) {
              s = (s2);
            } else {
              s = (s1);
            }
            if ((start_r) + s * (end_r - start_r) < 0) {
              continue;
            }
          }
          if (s < 0) {
            if (!_info._fillColor->_shading->_isExtendedBegin) {
              continue;
            }
            s = 0;
          }
          if (s > 1.0f) {
            if (!_info._fillColor->_shading->_isExtendedEnd) {
              continue;
            }
            s = 1.0f;
          }
          int index = (int32_t)(s * (FX_SHADING_Steps - 1));
          dib_buf[column] = _info._fillColor->_shading->_argbArray[index];
        }
      }
      result = TRUE;
      break;
    }
    default: { result = FALSE; }
  }
  if (result) {
    _renderDevice->SaveState();
    _renderDevice->SetClip_PathFill(path->GetPathData(), (CFX_Matrix*)matrix,
                                    fillMode);
    SetDIBitsWithMatrix(&bmp, matrix);
    _renderDevice->RestoreState();
  }
  return result;
}
FX_ERR CFX_Graphics::SetDIBitsWithMatrix(CFX_DIBSource* source,
                                         CFX_Matrix* matrix) {
  if (matrix->IsIdentity()) {
    _renderDevice->SetDIBits(source, 0, 0);
  } else {
    CFX_Matrix m;
    m.Set((FX_FLOAT)source->GetWidth(), 0, 0, (FX_FLOAT)source->GetHeight(), 0,
          0);
    m.Concat(*matrix);
    int32_t left, top;
    CFX_DIBitmap* bmp1 = source->FlipImage(FALSE, TRUE);
    CFX_DIBitmap* bmp2 = bmp1->TransformTo((CFX_Matrix*)&m, left, top);
    _renderDevice->SetDIBits(bmp2, left, top);
    if (bmp2) {
      delete bmp2;
      bmp2 = NULL;
    }
    if (bmp1) {
      delete bmp1;
      bmp1 = NULL;
    }
  }
  return FX_ERR_Succeeded;
}
FX_ERR CFX_Graphics::CalcTextInfo(const CFX_WideString& text,
                                  FX_DWORD* charCodes,
                                  FXTEXT_CHARPOS* charPos,
                                  CFX_RectF& rect) {
  std::unique_ptr<CFX_UnicodeEncoding> encoding(
      new CFX_UnicodeEncoding(_info._font));
  int32_t length = text.GetLength();
  FX_FLOAT penX = (FX_FLOAT)rect.left;
  FX_FLOAT penY = (FX_FLOAT)rect.top;
  FX_FLOAT left = (FX_FLOAT)(0);
  FX_FLOAT top = (FX_FLOAT)(0);
  charCodes[0] = text.GetAt(0);
  charPos[0].m_OriginX = penX + left;
  charPos[0].m_OriginY = penY + top;
  charPos[0].m_GlyphIndex = encoding->GlyphFromCharCode(charCodes[0]);
  charPos[0].m_FontCharWidth = FXSYS_round(
      _info._font->GetGlyphWidth(charPos[0].m_GlyphIndex) * _info._fontHScale);
  charPos[0].m_bGlyphAdjust = TRUE;
  charPos[0].m_AdjustMatrix[0] = -1;
  charPos[0].m_AdjustMatrix[1] = 0;
  charPos[0].m_AdjustMatrix[2] = 0;
  charPos[0].m_AdjustMatrix[3] = 1;
  penX += (FX_FLOAT)(charPos[0].m_FontCharWidth) * _info._fontSize / 1000 +
          _info._fontSpacing;
  for (int32_t i = 1; i < length; i++) {
    charCodes[i] = text.GetAt(i);
    charPos[i].m_OriginX = penX + left;
    charPos[i].m_OriginY = penY + top;
    charPos[i].m_GlyphIndex = encoding->GlyphFromCharCode(charCodes[i]);
    charPos[i].m_FontCharWidth =
        FXSYS_round(_info._font->GetGlyphWidth(charPos[i].m_GlyphIndex) *
                    _info._fontHScale);
    charPos[i].m_bGlyphAdjust = TRUE;
    charPos[i].m_AdjustMatrix[0] = -1;
    charPos[i].m_AdjustMatrix[1] = 0;
    charPos[i].m_AdjustMatrix[2] = 0;
    charPos[i].m_AdjustMatrix[3] = 1;
    penX += (FX_FLOAT)(charPos[i].m_FontCharWidth) * _info._fontSize / 1000 +
            _info._fontSpacing;
  }
  rect.width = (FX_FLOAT)penX - rect.left;
  rect.height = rect.top + _info._fontSize * _info._fontHScale - rect.top;
  return FX_ERR_Succeeded;
}
CAGG_Graphics::CAGG_Graphics() {
  _owner = NULL;
}
FX_ERR CAGG_Graphics::Create(CFX_Graphics* owner,
                             int32_t width,
                             int32_t height,
                             FXDIB_Format format) {
  if (owner->_renderDevice) {
    return FX_ERR_Parameter_Invalid;
  }
  if (_owner) {
    return FX_ERR_Property_Invalid;
  }
  CFX_FxgeDevice* device = new CFX_FxgeDevice;
  device->Create(width, height, format);
  _owner = owner;
  _owner->_renderDevice = device;
  _owner->_renderDevice->GetBitmap()->Clear(0xFFFFFFFF);
  return FX_ERR_Succeeded;
}
CAGG_Graphics::~CAGG_Graphics() {
  if (_owner->_renderDevice) {
    delete (CFX_FxgeDevice*)_owner->_renderDevice;
  }
  _owner = NULL;
}
CFX_Path::CFX_Path() {
  _generator = NULL;
}
FX_ERR CFX_Path::Create() {
  if (_generator) {
    return FX_ERR_Property_Invalid;
  }
  _generator = new CFX_PathGenerator;
  _generator->Create();
  return FX_ERR_Succeeded;
}
CFX_Path::~CFX_Path() {
  if (_generator) {
    delete _generator;
    _generator = NULL;
  }
}
FX_ERR CFX_Path::MoveTo(FX_FLOAT x, FX_FLOAT y) {
  _FX_RETURN_VALUE_IF_FAIL(_generator, FX_ERR_Property_Invalid);
  _generator->MoveTo(x, y);
  return FX_ERR_Succeeded;
}
FX_ERR CFX_Path::LineTo(FX_FLOAT x, FX_FLOAT y) {
  _FX_RETURN_VALUE_IF_FAIL(_generator, FX_ERR_Property_Invalid);
  _generator->LineTo(x, y);
  return FX_ERR_Succeeded;
}
FX_ERR CFX_Path::BezierTo(FX_FLOAT ctrlX1,
                          FX_FLOAT ctrlY1,
                          FX_FLOAT ctrlX2,
                          FX_FLOAT ctrlY2,
                          FX_FLOAT toX,
                          FX_FLOAT toY) {
  _FX_RETURN_VALUE_IF_FAIL(_generator, FX_ERR_Property_Invalid);
  _generator->BezierTo(ctrlX1, ctrlY1, ctrlX2, ctrlY2, toX, toY);
  return FX_ERR_Succeeded;
}
FX_ERR CFX_Path::ArcTo(FX_FLOAT left,
                       FX_FLOAT top,
                       FX_FLOAT width,
                       FX_FLOAT height,
                       FX_FLOAT startAngle,
                       FX_FLOAT sweepAngle) {
  _FX_RETURN_VALUE_IF_FAIL(_generator, FX_ERR_Property_Invalid);
  _generator->ArcTo(left + width / 2, top + height / 2, width / 2, height / 2,
                    startAngle, sweepAngle);
  return FX_ERR_Succeeded;
}
FX_ERR CFX_Path::Close() {
  _FX_RETURN_VALUE_IF_FAIL(_generator, FX_ERR_Property_Invalid);
  _generator->Close();
  return FX_ERR_Succeeded;
}
FX_ERR CFX_Path::AddLine(FX_FLOAT x1, FX_FLOAT y1, FX_FLOAT x2, FX_FLOAT y2) {
  _FX_RETURN_VALUE_IF_FAIL(_generator, FX_ERR_Property_Invalid);
  _generator->AddLine(x1, y1, x2, y2);
  return FX_ERR_Succeeded;
}
FX_ERR CFX_Path::AddBezier(FX_FLOAT startX,
                           FX_FLOAT startY,
                           FX_FLOAT ctrlX1,
                           FX_FLOAT ctrlY1,
                           FX_FLOAT ctrlX2,
                           FX_FLOAT ctrlY2,
                           FX_FLOAT endX,
                           FX_FLOAT endY) {
  _FX_RETURN_VALUE_IF_FAIL(_generator, FX_ERR_Property_Invalid);
  _generator->AddBezier(startX, startY, ctrlX1, ctrlY1, ctrlX2, ctrlY2, endX,
                        endY);
  return FX_ERR_Succeeded;
}
FX_ERR CFX_Path::AddRectangle(FX_FLOAT left,
                              FX_FLOAT top,
                              FX_FLOAT width,
                              FX_FLOAT height) {
  _FX_RETURN_VALUE_IF_FAIL(_generator, FX_ERR_Property_Invalid);
  _generator->AddRectangle(left, top, left + width, top + height);
  return FX_ERR_Succeeded;
}
FX_ERR CFX_Path::AddEllipse(FX_FLOAT left,
                            FX_FLOAT top,
                            FX_FLOAT width,
                            FX_FLOAT height) {
  _FX_RETURN_VALUE_IF_FAIL(_generator, FX_ERR_Property_Invalid);
  _generator->AddEllipse(left + width / 2, top + height / 2, width / 2,
                         height / 2);
  return FX_ERR_Succeeded;
}
FX_ERR CFX_Path::AddEllipse(const CFX_RectF& rect) {
  _FX_RETURN_VALUE_IF_FAIL(_generator, FX_ERR_Property_Invalid);
  _generator->AddEllipse(rect.left + rect.Width() / 2,
                         rect.top + rect.Height() / 2, rect.Width() / 2,
                         rect.Height() / 2);
  return FX_ERR_Succeeded;
}
FX_ERR CFX_Path::AddArc(FX_FLOAT left,
                        FX_FLOAT top,
                        FX_FLOAT width,
                        FX_FLOAT height,
                        FX_FLOAT startAngle,
                        FX_FLOAT sweepAngle) {
  _FX_RETURN_VALUE_IF_FAIL(_generator, FX_ERR_Property_Invalid);
  _generator->AddArc(left + width / 2, top + height / 2, width / 2, height / 2,
                     startAngle, sweepAngle);
  return FX_ERR_Succeeded;
}
FX_ERR CFX_Path::AddPie(FX_FLOAT left,
                        FX_FLOAT top,
                        FX_FLOAT width,
                        FX_FLOAT height,
                        FX_FLOAT startAngle,
                        FX_FLOAT sweepAngle) {
  _FX_RETURN_VALUE_IF_FAIL(_generator, FX_ERR_Property_Invalid);
  _generator->AddPie(left + width / 2, top + height / 2, width / 2, height / 2,
                     startAngle, sweepAngle);
  return FX_ERR_Succeeded;
}
FX_ERR CFX_Path::AddSubpath(CFX_Path* path) {
  _FX_RETURN_VALUE_IF_FAIL(_generator, FX_ERR_Property_Invalid);
  _generator->AddPathData(path->GetPathData());
  return FX_ERR_Succeeded;
}
FX_ERR CFX_Path::Clear() {
  _FX_RETURN_VALUE_IF_FAIL(_generator, FX_ERR_Property_Invalid);
  _generator->GetPathData()->SetPointCount(0);
  return FX_ERR_Succeeded;
}
FX_BOOL CFX_Path::IsEmpty() {
  _FX_RETURN_VALUE_IF_FAIL(_generator, FX_ERR_Property_Invalid);
  if (_generator->GetPathData()->GetPointCount() == 0) {
    return TRUE;
  }
  return FALSE;
}
CFX_PathData* CFX_Path::GetPathData() {
  _FX_RETURN_VALUE_IF_FAIL(_generator, NULL);
  return _generator->GetPathData();
}
CFX_Color::CFX_Color() {
  _type = FX_COLOR_None;
}
CFX_Color::CFX_Color(const FX_ARGB argb) {
  _type = FX_COLOR_None;
  Set(argb);
}
CFX_Color::CFX_Color(CFX_Pattern* pattern, const FX_ARGB argb) {
  _type = FX_COLOR_None;
  Set(pattern, argb);
}
CFX_Color::CFX_Color(CFX_Shading* shading) {
  _type = FX_COLOR_None;
  Set(shading);
}
CFX_Color::~CFX_Color() {
  _type = FX_COLOR_None;
}
FX_ERR CFX_Color::Set(const FX_ARGB argb) {
  _type = FX_COLOR_Solid;
  _argb = argb;
  _pattern = NULL;
  return FX_ERR_Succeeded;
}
FX_ERR CFX_Color::Set(CFX_Pattern* pattern, const FX_ARGB argb) {
  _FX_RETURN_VALUE_IF_FAIL(pattern, FX_ERR_Parameter_Invalid);
  _type = FX_COLOR_Pattern;
  _argb = argb;
  _pattern = pattern;
  return FX_ERR_Succeeded;
}
FX_ERR CFX_Color::Set(CFX_Shading* shading) {
  _FX_RETURN_VALUE_IF_FAIL(shading, FX_ERR_Parameter_Invalid);
  _type = FX_COLOR_Shading;
  _shading = shading;
  return FX_ERR_Succeeded;
}
CFX_Pattern::CFX_Pattern() {
  _type = FX_PATTERN_None;
  _matrix.SetIdentity();
}
FX_ERR CFX_Pattern::Create(CFX_DIBitmap* bitmap,
                           const FX_FLOAT xStep,
                           const FX_FLOAT yStep,
                           CFX_Matrix* matrix) {
  _FX_RETURN_VALUE_IF_FAIL(bitmap, FX_ERR_Parameter_Invalid);
  if (_type != FX_PATTERN_None) {
    return FX_ERR_Property_Invalid;
  }
  _type = FX_PATTERN_Bitmap;
  _bitmap = bitmap;
  _x1Step = xStep;
  _y1Step = yStep;
  if (matrix) {
    _matrix.Set(matrix->a, matrix->b, matrix->c, matrix->d, matrix->e,
                matrix->f);
  }
  return FX_ERR_Succeeded;
}
FX_ERR CFX_Pattern::Create(FX_HatchStyle hatchStyle,
                           const FX_ARGB foreArgb,
                           const FX_ARGB backArgb,
                           CFX_Matrix* matrix) {
  if (hatchStyle < FX_HATCHSTYLE_Horizontal ||
      hatchStyle > FX_HATCHSTYLE_SolidDiamond) {
    return FX_ERR_Parameter_Invalid;
  }
  if (_type != FX_PATTERN_None) {
    return FX_ERR_Property_Invalid;
  }
  _type = FX_PATTERN_Hatch;
  _hatchStyle = hatchStyle;
  _foreArgb = foreArgb;
  _backArgb = backArgb;
  if (matrix) {
    _matrix.Set(matrix->a, matrix->b, matrix->c, matrix->d, matrix->e,
                matrix->f);
  }
  return FX_ERR_Succeeded;
}
CFX_Pattern::~CFX_Pattern() {
  _type = FX_PATTERN_None;
}
CFX_Shading::CFX_Shading() {
  _type = FX_SHADING_None;
}
FX_ERR CFX_Shading::CreateAxial(const CFX_PointF& beginPoint,
                                const CFX_PointF& endPoint,
                                FX_BOOL isExtendedBegin,
                                FX_BOOL isExtendedEnd,
                                const FX_ARGB beginArgb,
                                const FX_ARGB endArgb) {
  if (_type != FX_SHADING_None) {
    return FX_ERR_Property_Invalid;
  }
  _type = FX_SHADING_Axial;
  _beginPoint = beginPoint;
  _endPoint = endPoint;
  _isExtendedBegin = isExtendedBegin;
  _isExtendedEnd = isExtendedEnd;
  _beginArgb = beginArgb;
  _endArgb = endArgb;
  return InitArgbArray();
}
FX_ERR CFX_Shading::CreateRadial(const CFX_PointF& beginPoint,
                                 const CFX_PointF& endPoint,
                                 const FX_FLOAT beginRadius,
                                 const FX_FLOAT endRadius,
                                 FX_BOOL isExtendedBegin,
                                 FX_BOOL isExtendedEnd,
                                 const FX_ARGB beginArgb,
                                 const FX_ARGB endArgb) {
  if (_type != FX_SHADING_None) {
    return FX_ERR_Property_Invalid;
  }
  _type = FX_SHADING_Radial;
  _beginPoint = beginPoint;
  _endPoint = endPoint;
  _beginRadius = beginRadius;
  _endRadius = endRadius;
  _isExtendedBegin = isExtendedBegin;
  _isExtendedEnd = isExtendedEnd;
  _beginArgb = beginArgb;
  _endArgb = endArgb;
  return InitArgbArray();
}
CFX_Shading::~CFX_Shading() {
  _type = FX_SHADING_None;
}
FX_ERR CFX_Shading::InitArgbArray() {
  int32_t a1, r1, g1, b1;
  ArgbDecode(_beginArgb, a1, r1, g1, b1);
  int32_t a2, r2, g2, b2;
  ArgbDecode(_endArgb, a2, r2, g2, b2);
  FX_FLOAT f = (FX_FLOAT)(FX_SHADING_Steps - 1);
  FX_FLOAT aScale = (FX_FLOAT)(1.0 * (a2 - a1) / f);
  FX_FLOAT rScale = (FX_FLOAT)(1.0 * (r2 - r1) / f);
  FX_FLOAT gScale = (FX_FLOAT)(1.0 * (g2 - g1) / f);
  FX_FLOAT bScale = (FX_FLOAT)(1.0 * (b2 - b1) / f);
  int32_t a3, r3, g3, b3;
  for (int32_t i = 0; i < FX_SHADING_Steps; i++) {
    a3 = (int32_t)(i * aScale);
    r3 = (int32_t)(i * rScale);
    g3 = (int32_t)(i * gScale);
    b3 = (int32_t)(i * bScale);
    _argbArray[i] =
        FXARGB_TODIB(FXARGB_MAKE((a1 + a3), (r1 + r3), (g1 + g3), (b1 + b3)));
  }
  return FX_ERR_Succeeded;
}
class CFX_Pause : public IFX_Pause {
 public:
  virtual FX_BOOL NeedToPauseNow() { return TRUE; }
};
