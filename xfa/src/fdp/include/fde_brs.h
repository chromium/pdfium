// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FDE_BRUSH
#define _FDE_BRUSH
class IFDE_Image;
class IFDE_Path;
class IFDE_Brush;
class IFDE_SolidBrush;
class IFDE_HatchBrush;
class IFDE_TextureBrush;
class IFDE_LinearGradientBrush;
#define FDE_BRUSHTYPE_Unknown -1
#define FDE_BRUSHTYPE_Solid 0
#define FDE_BRUSHTYPE_Hatch 1
#define FDE_BRUSHTYPE_Texture 2
#define FDE_BRUSHTYPE_LinearGradient 3
#define FDE_BRUSHTYPE_MAX 3
#define FDE_WRAPMODE_Tile 0
#define FDE_WRAPMODE_TileFlipX 1
#define FDE_WRAPMODE_TileFlipY 2
#define FDE_WRAPMODE_TileFlipXY 3
#define FDE_WRAPMODE_Clamp 4
typedef struct _FDE_GRADIENTCOLOR {
  FX_FLOAT pos;
  FX_ARGB color;
} FDE_GRADIENTCOLOR, *FDE_LPGRADIENTCOLOR;
typedef FDE_GRADIENTCOLOR const* FDE_LPCGRADIENTCOLOR;
typedef CFX_ArrayTemplate<FDE_GRADIENTCOLOR> CFDE_GradientColors;
class IFDE_Brush {
 public:
  static IFDE_Brush* Create(int32_t iType);
  virtual ~IFDE_Brush() {}
  virtual void Release() = 0;
  virtual int32_t GetType() const = 0;
};
class IFDE_SolidBrush : public IFDE_Brush {
 public:
  virtual FX_ARGB GetColor() const = 0;
  virtual void SetColor(FX_ARGB color) = 0;
  virtual const CFX_Matrix& GetMatrix() const = 0;
  virtual void ResetMatrix() = 0;
  virtual void TranslateMatrix(FX_FLOAT dx, FX_FLOAT dy) = 0;
  virtual void RotateMatrix(FX_FLOAT fRadian) = 0;
  virtual void ScaleMatrix(FX_FLOAT sx, FX_FLOAT sy) = 0;
  virtual void ConcatMatrix(const CFX_Matrix& matrix) = 0;
  virtual void SetMatrix(const CFX_Matrix& matrix) = 0;
};
#define FDE_HATCHSTYLE_Horizontal 0
#define FDE_HATCHSTYLE_Vertical 1
#define FDE_HATCHSTYLE_ForwardDiagonal 2
#define FDE_HATCHSTYLE_BackwardDiagonal 3
#define FDE_HATCHSTYLE_Cross 4
#define FDE_HATCHSTYLE_DiagonalCross 5
#define FDE_HATCHSTYLE_05Percent 6
#define FDE_HATCHSTYLE_10Percent 7
#define FDE_HATCHSTYLE_20Percent 8
#define FDE_HATCHSTYLE_25Percent 9
#define FDE_HATCHSTYLE_30Percent 10
#define FDE_HATCHSTYLE_40Percent 11
#define FDE_HATCHSTYLE_50Percent 12
#define FDE_HATCHSTYLE_60Percent 13
#define FDE_HATCHSTYLE_70Percent 14
#define FDE_HATCHSTYLE_75Percent 15
#define FDE_HATCHSTYLE_80Percent 16
#define FDE_HATCHSTYLE_90Percent 17
#define FDE_HATCHSTYLE_LightDownwardDiagonal 18
#define FDE_HATCHSTYLE_LightUpwardDiagonal 19
#define FDE_HATCHSTYLE_DarkDownwardDiagonal 20
#define FDE_HATCHSTYLE_DarkUpwardDiagonal 21
#define FDE_HATCHSTYLE_WideDownwardDiagonal 22
#define FDE_HATCHSTYLE_WideUpwardDiagonal 23
#define FDE_HATCHSTYLE_LightVertical 24
#define FDE_HATCHSTYLE_LightHorizontal 25
#define FDE_HATCHSTYLE_NarrowVertical 26
#define FDE_HATCHSTYLE_NarrowHorizontal 27
#define FDE_HATCHSTYLE_DarkVertical 28
#define FDE_HATCHSTYLE_DarkHorizontal 29
#define FDE_HATCHSTYLE_DashedDownwardDiagonal 30
#define FDE_HATCHSTYLE_DashedUpwardDiagonal 31
#define FDE_HATCHSTYLE_DashedHorizontal 32
#define FDE_HATCHSTYLE_DashedVertical 33
#define FDE_HATCHSTYLE_SmallConfetti 34
#define FDE_HATCHSTYLE_LargeConfetti 35
#define FDE_HATCHSTYLE_ZigZag 36
#define FDE_HATCHSTYLE_Wave 37
#define FDE_HATCHSTYLE_DiagonalBrick 38
#define FDE_HATCHSTYLE_HorizontalBrick 39
#define FDE_HATCHSTYLE_Weave 40
#define FDE_HATCHSTYLE_Plaid 41
#define FDE_HATCHSTYLE_Divot 42
#define FDE_HATCHSTYLE_DottedGrid 43
#define FDE_HATCHSTYLE_DottedDiamond 44
#define FDE_HATCHSTYLE_Shingle 45
#define FDE_HATCHSTYLE_Trellis 46
#define FDE_HATCHSTYLE_Sphere 47
#define FDE_HATCHSTYLE_SmallGrid 48
#define FDE_HATCHSTYLE_SmallCheckerBoard 49
#define FDE_HATCHSTYLE_LargeCheckerBoard 50
#define FDE_HATCHSTYLE_OutlinedDiamond 51
#define FDE_HATCHSTYLE_SolidDiamond 52
#define FDE_HATCHSTYLE_Total 53
#define FDE_HATCHSTYLE_LargeGrid FDE_HATCHSTYLE_Cross
#define FDE_HATCHSTYLE_Min FDE_HATCHSTYLE_Horizontal
#define FDE_HATCHSTYLE_Max (FDE_HATCHSTYLE_Total - 1)
class IFDE_HatchBrush : public IFDE_Brush {
 public:
  virtual FX_ARGB GetColor(FX_BOOL bForegroundColor) const = 0;
  virtual void SetColor(FX_ARGB color, FX_BOOL bForegroundColor) = 0;
  virtual int32_t GetHatchStyle() const = 0;
  virtual FX_BOOL SetHatchStyle(int32_t iHatchStyle) = 0;
};
class IFDE_TextureBrush : public IFDE_Brush {
 public:
  virtual IFDE_Image* GetImage() const = 0;
  virtual void SetImage(IFDE_Image* pImage, FX_BOOL bAutoRelease) = 0;
  virtual int32_t GetWrapMode() const = 0;
  virtual void SetWrapMode(int32_t iWrapMode) = 0;
};
#define FDE_LINEARGRADIENTMODE_Horizontal 0
#define FDE_LINEARGRADIENTMODE_Vertical 1
#define FDE_LINEARGRADIENTMODE_ForwardDiagonal 2
#define FDE_LINEARGRADIENTMODE_BackwardDiagonal 3
class IFDE_LinearGradientBrush : public IFDE_Brush {
 public:
  virtual void GetLinearPoints(CFX_PointF& startingPoint,
                               CFX_PointF& endingPoint) const = 0;
  virtual void SetLinearPoints(const CFX_PointF& startingPoint,
                               const CFX_PointF& endingPoint) = 0;
  virtual void GetLinearColors(FX_ARGB& startingColor,
                               FX_ARGB& endingColor) const = 0;
  virtual void SetLinearColors(const FX_ARGB& startingColor,
                               const FX_ARGB& endingColor) = 0;
  virtual int32_t CountGradientColors() const = 0;
  virtual FX_BOOL GetGradientColors(CFDE_GradientColors& colors) const = 0;
  virtual FX_BOOL SetGradientColors(const CFDE_GradientColors& colors) = 0;
  virtual int32_t GetWrapMode() const = 0;
  virtual void SetWrapMode(int32_t iWrapMode) = 0;
};
#endif
