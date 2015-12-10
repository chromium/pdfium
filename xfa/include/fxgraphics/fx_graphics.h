// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_INCLUDE_FXGRAPHICS_FX_GRAPHICS_H_
#define XFA_INCLUDE_FXGRAPHICS_FX_GRAPHICS_H_

#include "core/include/fpdfapi/fpdf_pageobj.h"

typedef int FX_ERR;
#define FX_ERR_Succeeded 0
#define FX_ERR_Indefinite -1
#define FX_ERR_Parameter_Invalid -100
#define FX_ERR_Property_Invalid -200
#define FX_ERR_Intermediate_Value_Invalid -300
#define FX_ERR_Method_Not_Supported -400
#define FX_ERR_Out_Of_Memory -500
#define _FX_RETURN_IF_FAIL(arg) \
  {                             \
    if (!(arg))                 \
      return;                   \
  }
#define _FX_RETURN_VALUE_IF_FAIL(arg, val) \
  {                                        \
    if (!(arg))                            \
      return val;                          \
  }
#define _FX_GOTO_POSITION_IF_FAIL(arg, pos) \
  {                                         \
    if (!(arg))                             \
      goto pos;                             \
  }
#define _FX_ERR_CHECK_RETURN_IF_FAIL(arg) \
  {                                       \
    if ((arg) != FX_ERR_Succeeded)        \
      return;                             \
  }
#define _FX_ERR_CHECK_RETURN_VALUE_IF_FAIL(arg, val) \
  {                                                  \
    if ((arg) != FX_ERR_Succeeded)                   \
      return val;                                    \
  }
#define _FX_ERR_CHECK_GOTO_POSITION_IF_FAIL(arg, pos) \
  {                                                   \
    if ((arg) != FX_ERR_Succeeded)                    \
      goto pos;                                       \
  }

#define FX_SHADING_Steps 256
typedef int32_t FX_DashStyle;
enum {
  FX_DASHSTYLE_Solid = 0,
  FX_DASHSTYLE_Dash = 1,
  FX_DASHSTYLE_Dot = 2,
  FX_DASHSTYLE_DashDot = 3,
  FX_DASHSTYLE_DashDotDot = 4
};
typedef int32_t FX_StrokeAlignment;
enum {
  FX_STROKEALIGNMENT_Center = 0,
  FX_STROKEALIGNMENT_Inset = 1,
  FX_STROKEALIGNMENT_Outset = 2,
  FX_STROKEALIGNMENT_Left = 3,
  FX_STROKEALIGNMENT_Right = 4
};
typedef int32_t FX_HatchStyle;
enum {
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
typedef int32_t FX_DeviceCap;
typedef int32_t FX_FillMode;
class CFX_RenderDevice;
class CFX_GraphStateData;
class CFX_Matrix;
class CFX_DIBSource;
class CFX_DIBitmap;
class CFX_Font;
class CFX_WideString;
class IFX_FileRead;
class CFX_PathGenerator;
class CAGG_Graphics;
class CFX_Graphics;
class CFX_Color;
class CFX_Path;
class CFX_Pattern;
class CFX_Shading;
class CFX_Graphics {
 public:
  CFX_Graphics();

  FX_ERR Create(CFX_RenderDevice* renderDevice, FX_BOOL isAntialiasing = TRUE);

  FX_ERR Create(int32_t width,
                int32_t height,
                FXDIB_Format format,
                FX_BOOL isNative = TRUE,
                FX_BOOL isAntialiasing = TRUE);

  virtual ~CFX_Graphics();

  FX_ERR GetDeviceCap(const int32_t capID, FX_DeviceCap& capVal);
  FX_ERR IsPrinterDevice(FX_BOOL& isPrinter);
  FX_ERR EnableAntialiasing(FX_BOOL isAntialiasing);

  FX_ERR SaveGraphState();

  FX_ERR RestoreGraphState();

  FX_ERR GetLineCap(CFX_GraphStateData::LineCap& lineCap);

  FX_ERR SetLineCap(CFX_GraphStateData::LineCap lineCap);

  FX_ERR GetDashCount(int32_t& dashCount);

  FX_ERR GetLineDash(FX_FLOAT& dashPhase, FX_FLOAT* dashArray);

  FX_ERR SetLineDash(FX_FLOAT dashPhase,
                     FX_FLOAT* dashArray,
                     int32_t dashCount);

  FX_ERR SetLineDash(FX_DashStyle dashStyle);

  FX_ERR GetLineJoin(CFX_GraphStateData::LineJoin& lineJoin);

  FX_ERR SetLineJoin(CFX_GraphStateData::LineJoin lineJoin);

  FX_ERR GetMiterLimit(FX_FLOAT& miterLimit);

  FX_ERR SetMiterLimit(FX_FLOAT miterLimit);

  FX_ERR GetLineWidth(FX_FLOAT& lineWidth);

  FX_ERR SetLineWidth(FX_FLOAT lineWidth, FX_BOOL isActOnDash = FALSE);

  FX_ERR GetStrokeAlignment(FX_StrokeAlignment& strokeAlignment);

  FX_ERR SetStrokeAlignment(FX_StrokeAlignment strokeAlignment);

  FX_ERR SetStrokeColor(CFX_Color* color);

  FX_ERR SetFillColor(CFX_Color* color);

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

  CFX_Matrix* GetMatrix();

  FX_ERR GetClipRect(CFX_RectF& rect);

  FX_ERR SetClipRect(const CFX_RectF& rect);

  FX_ERR ClearClip();

  FX_ERR SetFont(CFX_Font* font);

  FX_ERR SetFontSize(const FX_FLOAT size);

  FX_ERR SetFontHScale(const FX_FLOAT scale);

  FX_ERR SetCharSpacing(const FX_FLOAT spacing);

  FX_ERR SetTextDrawingMode(const int32_t mode);

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

  CFX_RenderDevice* GetRenderDevice();

  FX_ERR InverseRect(const CFX_RectF& rect);
  FX_ERR XorDIBitmap(const CFX_DIBitmap* srcBitmap, const CFX_RectF& rect);
  FX_ERR EqvDIBitmap(const CFX_DIBitmap* srcBitmap, const CFX_RectF& rect);

 private:
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
                      FX_DWORD* charCodes,
                      FXTEXT_CHARPOS* charPos,
                      CFX_RectF& rect);

 protected:
  int32_t _type;

 private:
  struct TInfo {
    CFX_GraphStateData _graphState;
    FX_BOOL _isAntialiasing;
    FX_StrokeAlignment _strokeAlignment;
    CFX_Matrix _CTM;
    FX_BOOL _isActOnDash;
    CFX_Color* _strokeColor;
    CFX_Color* _fillColor;
    CFX_Font* _font;
    FX_FLOAT _fontSize;
    FX_FLOAT _fontHScale;
    FX_FLOAT _fontSpacing;
  } _info;
  CFX_RenderDevice* _renderDevice;
  CFX_PtrArray _infoStack;
  CAGG_Graphics* _aggGraphics;
  friend class CAGG_Graphics;
};
class CFX_Path {
 public:
  CFX_Path();

  FX_ERR Create();

  virtual ~CFX_Path();

  FX_ERR MoveTo(FX_FLOAT x, FX_FLOAT y);

  FX_ERR LineTo(FX_FLOAT x, FX_FLOAT y);

  FX_ERR BezierTo(FX_FLOAT ctrlX1,
                  FX_FLOAT ctrlY1,
                  FX_FLOAT ctrlX2,
                  FX_FLOAT ctrlY2,
                  FX_FLOAT toX,
                  FX_FLOAT toY);

  FX_ERR ArcTo(FX_FLOAT left,
               FX_FLOAT top,
               FX_FLOAT width,
               FX_FLOAT height,
               FX_FLOAT startAngle,
               FX_FLOAT sweepAngle);

  FX_ERR Close();

  FX_ERR AddLine(FX_FLOAT x1, FX_FLOAT y1, FX_FLOAT x2, FX_FLOAT y2);

  FX_ERR AddBezier(FX_FLOAT startX,
                   FX_FLOAT startY,
                   FX_FLOAT ctrlX1,
                   FX_FLOAT ctrlY1,
                   FX_FLOAT ctrlX2,
                   FX_FLOAT ctrlY2,
                   FX_FLOAT endX,
                   FX_FLOAT endY);

  FX_ERR AddRectangle(FX_FLOAT left,
                      FX_FLOAT top,
                      FX_FLOAT width,
                      FX_FLOAT height);

  FX_ERR AddEllipse(FX_FLOAT left,
                    FX_FLOAT top,
                    FX_FLOAT width,
                    FX_FLOAT height);

  FX_ERR AddEllipse(const CFX_RectF& rect);

  FX_ERR AddArc(FX_FLOAT left,
                FX_FLOAT top,
                FX_FLOAT width,
                FX_FLOAT height,
                FX_FLOAT startAngle,
                FX_FLOAT sweepAngle);

  FX_ERR AddPie(FX_FLOAT left,
                FX_FLOAT top,
                FX_FLOAT width,
                FX_FLOAT height,
                FX_FLOAT startAngle,
                FX_FLOAT sweepAngle);

  FX_ERR AddSubpath(CFX_Path* path);

  FX_ERR Clear();

  FX_BOOL IsEmpty();

  CFX_PathData* GetPathData();

 private:
  CFX_PathGenerator* _generator;
};
class CFX_Color {
 public:
  CFX_Color();

  CFX_Color(const FX_ARGB argb);

  CFX_Color(CFX_Pattern* pattern, const FX_ARGB argb = 0x0);

  CFX_Color(CFX_Shading* shading);

  virtual ~CFX_Color();

  FX_ERR Set(const FX_ARGB argb);

  FX_ERR Set(CFX_Pattern* pattern, const FX_ARGB argb = 0x0);

  FX_ERR Set(CFX_Shading* shading);

 private:
  int32_t _type;
  union {
    struct {
      FX_ARGB _argb;
      CFX_Pattern* _pattern;
    };
    CFX_Shading* _shading;
  };

  friend class CFX_Graphics;
};
class CFX_Pattern {
 public:
  CFX_Pattern();

  FX_ERR Create(CFX_DIBitmap* bitmap,
                const FX_FLOAT xStep,
                const FX_FLOAT yStep,
                CFX_Matrix* matrix = NULL);

  FX_ERR Create(FX_HatchStyle hatchStyle,
                const FX_ARGB foreArgb,
                const FX_ARGB backArgb,
                CFX_Matrix* matrix = NULL);

  virtual ~CFX_Pattern();

 private:
  int32_t _type;
  CFX_Matrix _matrix;
  union {
    struct {
      CFX_RectF _rect;
      FX_FLOAT _xStep;
      FX_FLOAT _yStep;
      FX_BOOL _isColored;
    };
    struct {
      CFX_DIBitmap* _bitmap;
      FX_FLOAT _x1Step;
      FX_FLOAT _y1Step;
    };
    struct {
      FX_HatchStyle _hatchStyle;
      FX_ARGB _foreArgb;
      FX_ARGB _backArgb;
    };
  };
  friend class CFX_Graphics;
};
class CFX_Shading {
 public:
  CFX_Shading();

  FX_ERR CreateAxial(const CFX_PointF& beginPoint,
                     const CFX_PointF& endPoint,
                     FX_BOOL isExtendedBegin,
                     FX_BOOL isExtendedEnd,
                     const FX_ARGB beginArgb,
                     const FX_ARGB endArgb);

  FX_ERR CreateRadial(const CFX_PointF& beginPoint,
                      const CFX_PointF& endPoint,
                      const FX_FLOAT beginRadius,
                      const FX_FLOAT endRadius,
                      FX_BOOL isExtendedBegin,
                      FX_BOOL isExtendedEnd,
                      const FX_ARGB beginArgb,
                      const FX_ARGB endArgb);

  virtual ~CFX_Shading();

 private:
  FX_ERR InitArgbArray();

 private:
  int32_t _type;
  CFX_PointF _beginPoint;
  CFX_PointF _endPoint;
  FX_FLOAT _beginRadius;
  FX_FLOAT _endRadius;
  FX_BOOL _isExtendedBegin;
  FX_BOOL _isExtendedEnd;
  FX_ARGB _beginArgb;
  FX_ARGB _endArgb;
  FX_ARGB _argbArray[FX_SHADING_Steps];
  friend class CFX_Graphics;
};

#endif  // XFA_INCLUDE_FXGRAPHICS_FX_GRAPHICS_H_
