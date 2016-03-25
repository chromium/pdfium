// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXGRAPHICS_INCLUDE_CFX_GRAPHICS_H_
#define XFA_FXGRAPHICS_INCLUDE_CFX_GRAPHICS_H_

#include "core/fxcrt/include/fx_system.h"
#include "core/include/fxge/fx_dib.h"
#include "core/include/fxge/fx_ge.h"

class CFX_Color;
class CFX_Path;
class CAGG_Graphics;

#define FX_ERR_Succeeded 0
#define FX_ERR_Indefinite -1
#define FX_ERR_Parameter_Invalid -100
#define FX_ERR_Property_Invalid -200
#define FX_ERR_Intermediate_Value_Invalid -300
#define FX_ERR_Method_Not_Supported -400
#define FX_ERR_Out_Of_Memory -500

using FX_ERR = int;
using FX_DeviceCap = int32_t;
using FX_FillMode = int32_t;

enum FX_DashStyle {
  FX_DASHSTYLE_Solid = 0,
  FX_DASHSTYLE_Dash = 1,
  FX_DASHSTYLE_Dot = 2,
  FX_DASHSTYLE_DashDot = 3,
  FX_DASHSTYLE_DashDotDot = 4
};

enum FX_StrokeAlignment {
  FX_STROKEALIGNMENT_Center = 0,
  FX_STROKEALIGNMENT_Inset = 1,
  FX_STROKEALIGNMENT_Outset = 2,
  FX_STROKEALIGNMENT_Left = 3,
  FX_STROKEALIGNMENT_Right = 4
};

enum FX_HatchStyle {
  FX_HATCHSTYLE_Horizontal = 0,
  FX_HATCHSTYLE_Vertical = 1,
  FX_HATCHSTYLE_ForwardDiagonal = 2,
  FX_HATCHSTYLE_BackwardDiagonal = 3,
  FX_HATCHSTYLE_Cross = 4,
  FX_HATCHSTYLE_DiagonalCross = 5,
  FX_HATCHSTYLE_05Percent = 6,
  FX_HATCHSTYLE_10Percent = 7,
  FX_HATCHSTYLE_20Percent = 8,
  FX_HATCHSTYLE_25Percent = 9,
  FX_HATCHSTYLE_30Percent = 10,
  FX_HATCHSTYLE_40Percent = 11,
  FX_HATCHSTYLE_50Percent = 12,
  FX_HATCHSTYLE_60Percent = 13,
  FX_HATCHSTYLE_70Percent = 14,
  FX_HATCHSTYLE_75Percent = 15,
  FX_HATCHSTYLE_80Percent = 16,
  FX_HATCHSTYLE_90Percent = 17,
  FX_HATCHSTYLE_LightDownwardDiagonal = 18,
  FX_HATCHSTYLE_LightUpwardDiagonal = 19,
  FX_HATCHSTYLE_DarkDownwardDiagonal = 20,
  FX_HATCHSTYLE_DarkUpwardDiagonal = 21,
  FX_HATCHSTYLE_WideDownwardDiagonal = 22,
  FX_HATCHSTYLE_WideUpwardDiagonal = 23,
  FX_HATCHSTYLE_LightVertical = 24,
  FX_HATCHSTYLE_LightHorizontal = 25,
  FX_HATCHSTYLE_NarrowVertical = 26,
  FX_HATCHSTYLE_NarrowHorizontal = 27,
  FX_HATCHSTYLE_DarkVertical = 28,
  FX_HATCHSTYLE_DarkHorizontal = 29,
  FX_HATCHSTYLE_DashedDownwardDiagonal = 30,
  FX_HATCHSTYLE_DashedUpwardDiagonal = 31,
  FX_HATCHSTYLE_DashedHorizontal = 32,
  FX_HATCHSTYLE_DashedVertical = 33,
  FX_HATCHSTYLE_SmallConfetti = 34,
  FX_HATCHSTYLE_LargeConfetti = 35,
  FX_HATCHSTYLE_ZigZag = 36,
  FX_HATCHSTYLE_Wave = 37,
  FX_HATCHSTYLE_DiagonalBrick = 38,
  FX_HATCHSTYLE_HorizontalBrick = 39,
  FX_HATCHSTYLE_Weave = 40,
  FX_HATCHSTYLE_Plaid = 41,
  FX_HATCHSTYLE_Divot = 42,
  FX_HATCHSTYLE_DottedGrid = 43,
  FX_HATCHSTYLE_DottedDiamond = 44,
  FX_HATCHSTYLE_Shingle = 45,
  FX_HATCHSTYLE_Trellis = 46,
  FX_HATCHSTYLE_Sphere = 47,
  FX_HATCHSTYLE_SmallGrid = 48,
  FX_HATCHSTYLE_SmallCheckerBoard = 49,
  FX_HATCHSTYLE_LargeCheckerBoard = 50,
  FX_HATCHSTYLE_OutlinedDiamond = 51,
  FX_HATCHSTYLE_SolidDiamond = 52
};

class CFX_RenderDevice;

class CFX_Graphics {
 public:
  CFX_Graphics();
  virtual ~CFX_Graphics();

  FX_ERR Create(CFX_RenderDevice* renderDevice, FX_BOOL isAntialiasing = TRUE);
  FX_ERR Create(int32_t width,
                int32_t height,
                FXDIB_Format format,
                FX_BOOL isNative = TRUE,
                FX_BOOL isAntialiasing = TRUE);

  FX_ERR GetDeviceCap(const int32_t capID, FX_DeviceCap& capVal);
  FX_ERR IsPrinterDevice(FX_BOOL& isPrinter);
  FX_ERR EnableAntialiasing(FX_BOOL isAntialiasing);

  FX_ERR SaveGraphState();
  FX_ERR RestoreGraphState();

  FX_ERR GetLineCap(CFX_GraphStateData::LineCap& lineCap) const;
  FX_ERR GetDashCount(int32_t& dashCount) const;
  FX_ERR GetLineDash(FX_FLOAT& dashPhase, FX_FLOAT* dashArray) const;
  FX_ERR GetLineJoin(CFX_GraphStateData::LineJoin& lineJoin) const;
  FX_ERR GetMiterLimit(FX_FLOAT& miterLimit) const;
  FX_ERR GetLineWidth(FX_FLOAT& lineWidth) const;
  FX_ERR GetStrokeAlignment(FX_StrokeAlignment& strokeAlignment) const;
  FX_ERR GetClipRect(CFX_RectF& rect) const;
  CFX_Matrix* GetMatrix();
  CFX_RenderDevice* GetRenderDevice();

  FX_ERR SetLineCap(CFX_GraphStateData::LineCap lineCap);
  FX_ERR SetLineDash(FX_FLOAT dashPhase,
                     FX_FLOAT* dashArray,
                     int32_t dashCount);
  FX_ERR SetLineDash(FX_DashStyle dashStyle);
  FX_ERR SetLineJoin(CFX_GraphStateData::LineJoin lineJoin);
  FX_ERR SetMiterLimit(FX_FLOAT miterLimit);
  FX_ERR SetLineWidth(FX_FLOAT lineWidth, FX_BOOL isActOnDash = FALSE);
  FX_ERR SetStrokeAlignment(FX_StrokeAlignment strokeAlignment);
  FX_ERR SetStrokeColor(CFX_Color* color);
  FX_ERR SetFillColor(CFX_Color* color);
  FX_ERR SetClipRect(const CFX_RectF& rect);
  FX_ERR SetFont(CFX_Font* font);
  FX_ERR SetFontSize(const FX_FLOAT size);
  FX_ERR SetFontHScale(const FX_FLOAT scale);
  FX_ERR SetCharSpacing(const FX_FLOAT spacing);
  FX_ERR SetTextDrawingMode(const int32_t mode);

  FX_ERR StrokePath(CFX_Path* path, CFX_Matrix* matrix = NULL);
  FX_ERR FillPath(CFX_Path* path,
                  FX_FillMode fillMode = FXFILL_WINDING,
                  CFX_Matrix* matrix = NULL);
  FX_ERR ClipPath(CFX_Path* path,
                  FX_FillMode fillMode = FXFILL_WINDING,
                  CFX_Matrix* matrix = NULL);
  FX_ERR DrawImage(CFX_DIBSource* source,
                   const CFX_PointF& point,
                   CFX_Matrix* matrix = NULL);
  FX_ERR StretchImage(CFX_DIBSource* source,
                      const CFX_RectF& rect,
                      CFX_Matrix* matrix = NULL);
  FX_ERR ConcatMatrix(const CFX_Matrix* matrix);
  FX_ERR ClearClip();
  FX_ERR ShowText(const CFX_PointF& point,
                  const CFX_WideString& text,
                  CFX_Matrix* matrix = NULL);
  FX_ERR CalcTextRect(CFX_RectF& rect,
                      const CFX_WideString& text,
                      FX_BOOL isMultiline = FALSE,
                      CFX_Matrix* matrix = NULL);
  FX_ERR Transfer(CFX_Graphics* graphics, const CFX_Matrix* matrix);
  FX_ERR Transfer(CFX_Graphics* graphics,
                  FX_FLOAT srcLeft,
                  FX_FLOAT srcTop,
                  const CFX_RectF& dstRect,
                  const CFX_Matrix* matrix);

  FX_ERR InverseRect(const CFX_RectF& rect);
  FX_ERR XorDIBitmap(const CFX_DIBitmap* srcBitmap, const CFX_RectF& rect);
  FX_ERR EqvDIBitmap(const CFX_DIBitmap* srcBitmap, const CFX_RectF& rect);

 protected:
  int32_t m_type;

 private:
  struct TInfo {
    TInfo()
        : isAntialiasing(TRUE),
          strokeAlignment(FX_STROKEALIGNMENT_Center),
          isActOnDash(FALSE),
          strokeColor(nullptr),
          fillColor(nullptr),
          font(nullptr),
          fontSize(40.0),
          fontHScale(1.0),
          fontSpacing(0.0) {}
    explicit TInfo(const TInfo& info);
    TInfo& operator=(const TInfo& other);

    CFX_GraphStateData graphState;
    FX_BOOL isAntialiasing;
    FX_StrokeAlignment strokeAlignment;
    CFX_Matrix CTM;
    FX_BOOL isActOnDash;
    CFX_Color* strokeColor;
    CFX_Color* fillColor;
    CFX_Font* font;
    FX_FLOAT fontSize;
    FX_FLOAT fontHScale;
    FX_FLOAT fontSpacing;
  } m_info;

  FX_ERR RenderDeviceSetLineDash(FX_DashStyle dashStyle);
  FX_ERR RenderDeviceStrokePath(CFX_Path* path, CFX_Matrix* matrix);
  FX_ERR RenderDeviceFillPath(CFX_Path* path,
                              FX_FillMode fillMode,
                              CFX_Matrix* matrix);
  FX_ERR RenderDeviceDrawImage(CFX_DIBSource* source,
                               const CFX_PointF& point,
                               CFX_Matrix* matrix);
  FX_ERR RenderDeviceStretchImage(CFX_DIBSource* source,
                                  const CFX_RectF& rect,
                                  CFX_Matrix* matrix);
  FX_ERR RenderDeviceShowText(const CFX_PointF& point,
                              const CFX_WideString& text,
                              CFX_Matrix* matrix);

  FX_ERR StrokePathWithPattern(CFX_Path* path, CFX_Matrix* matrix);
  FX_ERR StrokePathWithShading(CFX_Path* path, CFX_Matrix* matrix);

  FX_ERR FillPathWithPattern(CFX_Path* path,
                             FX_FillMode fillMode,
                             CFX_Matrix* matrix);
  FX_ERR FillPathWithShading(CFX_Path* path,
                             FX_FillMode fillMode,
                             CFX_Matrix* matrix);

  FX_ERR SetDIBitsWithMatrix(CFX_DIBSource* source, CFX_Matrix* matrix);
  FX_ERR CalcTextInfo(const CFX_WideString& text,
                      uint32_t* charCodes,
                      FXTEXT_CHARPOS* charPos,
                      CFX_RectF& rect);

  CFX_RenderDevice* m_renderDevice;
  CFX_PtrArray m_infoStack;
  CAGG_Graphics* m_aggGraphics;
  friend class CAGG_Graphics;
};

#endif  // XFA_FXGRAPHICS_INCLUDE_CFX_GRAPHICS_H_
