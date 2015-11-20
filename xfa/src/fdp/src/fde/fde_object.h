// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FDE_BASEOBJECT_IMP
#define _FDE_BASEOBJECT_IMP
class CFDE_Pen : public IFDE_Pen, public CFX_Target {
 public:
  CFDE_Pen()
      : m_Color(0),
        m_iLineCap(0),
        m_iLineJoin(0),
        m_iDashStyle(0),
        m_fDashPhase(0),
        m_fMiterLimit(10),
        m_bAutoRelease(FALSE),
        m_pBrush(NULL) {}

  ~CFDE_Pen() {
    if (m_pBrush && m_bAutoRelease) {
      m_pBrush->Release();
    }
  }
  virtual void Release() { delete this; }

  virtual int32_t GetType() const {
    return m_pBrush ? m_pBrush->GetType() : FDE_PENTYPE_SolidColor;
  }

  virtual FX_ARGB GetColor() const { return m_Color; }
  virtual void SetColor(FX_ARGB color) { m_Color = color; }
  virtual IFDE_Brush* GetBrush() const { return m_pBrush; }
  virtual void SetBrush(IFDE_Brush* pBrush, FX_BOOL bAutoRelease) {
    m_bAutoRelease = bAutoRelease;
    m_pBrush = pBrush;
    if (m_pBrush && m_pBrush->GetType() == FDE_BRUSHTYPE_Solid) {
      m_Color = ((IFDE_SolidBrush*)m_pBrush)->GetColor();
    }
  }
  virtual int32_t GetLineCap() const { return m_iLineCap; }
  virtual void SetLineCap(int32_t iLineCap) { m_iLineCap = iLineCap; }
  virtual int32_t GetDashStyle() const { return m_iDashStyle; }
  virtual void SetDashStyle(int32_t iDashStyle) { m_iDashStyle = iDashStyle; }
  virtual FX_FLOAT GetDashPhase() const { return m_fDashPhase; }
  virtual void SetDashPhase(FX_FLOAT fPhase) { m_fDashPhase = fPhase; }
  virtual int32_t CountDashArray() const { return m_DashArray.GetSize(); }
  virtual int32_t GetDashArray(CFX_FloatArray& dashArray) const {
    dashArray.Copy(m_DashArray);
    return dashArray.GetSize();
  }
  virtual void SetDashArray(const CFX_FloatArray& dashArray) {
    m_DashArray.Copy(dashArray);
  }
  virtual int32_t GetLineJoin() const { return m_iLineJoin; }
  virtual void SetLineJoin(int32_t iLineJoin) { m_iLineJoin = iLineJoin; }
  virtual FX_FLOAT GetMiterLimit() const { return m_fMiterLimit; }
  virtual void SetMiterLimit(FX_FLOAT fMiterLimit) {
    m_fMiterLimit = fMiterLimit;
  }
  virtual int32_t CountCompoundPatterns() const {
    return m_CompoundPatterns.GetSize();
  }
  virtual FX_BOOL GetCompoundPatterns(
      CFDE_CompoundPatterns& compoundPatterns) const {
    return compoundPatterns.Copy(m_CompoundPatterns), TRUE;
  }
  virtual FX_BOOL SetCompoundPatterns(
      const CFDE_CompoundPatterns& compoundPatterns) {
    return m_CompoundPatterns.Copy(compoundPatterns), TRUE;
  }

  FX_ARGB m_Color;
  int32_t m_iLineCap;
  int32_t m_iLineJoin;
  int32_t m_iDashStyle;
  FX_FLOAT m_fDashPhase;
  FX_FLOAT m_fMiterLimit;
  FX_BOOL m_bAutoRelease;
  IFDE_Brush* m_pBrush;
  CFX_FloatArray m_DashArray;
  CFDE_CompoundPatterns m_CompoundPatterns;
};
class CFDE_SolidBrush : public IFDE_SolidBrush, public CFX_Target {
 public:
  CFDE_SolidBrush() : m_Color(0xFF000000) { m_Matrix.SetIdentity(); }

  virtual void Release() { delete this; }
  virtual int32_t GetType() const { return FDE_BRUSHTYPE_Solid; }
  virtual const CFX_Matrix& GetMatrix() const { return m_Matrix; }
  virtual void ResetMatrix() { m_Matrix.SetIdentity(); }
  virtual void TranslateMatrix(FX_FLOAT dx, FX_FLOAT dy) {
    m_Matrix.Translate(dx, dy);
  }
  virtual void RotateMatrix(FX_FLOAT fRadian) { m_Matrix.Rotate(fRadian); }
  virtual void ScaleMatrix(FX_FLOAT sx, FX_FLOAT sy) { m_Matrix.Scale(sx, sy); }
  virtual void ConcatMatrix(const CFX_Matrix& matrix) {
    m_Matrix.Concat(matrix);
  }
  virtual void SetMatrix(const CFX_Matrix& matrix) { m_Matrix = matrix; }
  virtual FX_ARGB GetColor() const { return m_Color; }
  virtual void SetColor(FX_ARGB color) { m_Color = color; }

  FX_ARGB m_Color;
  CFX_Matrix m_Matrix;
};
class CFDE_HatchBrush : public IFDE_HatchBrush, public CFX_Target {
 public:
  CFDE_HatchBrush() : m_iStyle(-1), m_BackColor(0), m_ForeColor(0) {
    m_Matrix.SetIdentity();
  }

  virtual void Release() { delete this; }
  virtual int32_t GetType() const { return FDE_BRUSHTYPE_Hatch; }
  virtual const CFX_Matrix& GetMatrix() const { return m_Matrix; }
  virtual void ResetMatrix() { m_Matrix.SetIdentity(); }
  virtual void TranslateMatrix(FX_FLOAT dx, FX_FLOAT dy) {
    m_Matrix.Translate(dx, dy);
  }
  virtual void RotateMatrix(FX_FLOAT fRadian) { m_Matrix.Rotate(fRadian); }
  virtual void ScaleMatrix(FX_FLOAT sx, FX_FLOAT sy) { m_Matrix.Scale(sx, sy); }
  virtual void ConcatMatrix(const CFX_Matrix& matrix) {
    m_Matrix.Concat(matrix);
  }
  virtual void SetMatrix(const CFX_Matrix& matrix) { m_Matrix = matrix; }
  virtual FX_ARGB GetColor(FX_BOOL bForegroundColor) const {
    return bForegroundColor ? m_ForeColor : m_BackColor;
  }
  virtual void SetColor(FX_ARGB color, FX_BOOL bForegroundColor) {
    if (bForegroundColor) {
      m_ForeColor = color;
    } else {
      m_BackColor = color;
    }
  }

  virtual int32_t GetHatchStyle() const { return m_iStyle; };
  virtual FX_BOOL SetHatchStyle(int32_t iHatchStyle) {
    m_iStyle = iHatchStyle;
    return m_iStyle >= FDE_HATCHSTYLE_Min && m_iStyle <= FDE_HATCHSTYLE_Max;
  }
  int32_t m_iStyle;
  FX_ARGB m_BackColor;
  FX_ARGB m_ForeColor;
  CFX_Matrix m_Matrix;
};
class CFDE_TextureBrush : public IFDE_TextureBrush, public CFX_Target {
 public:
  CFDE_TextureBrush() : m_iWrap(0), m_pImage(NULL), m_bAutoRelease(FALSE) {
    m_Matrix.SetIdentity();
  }

  virtual void Release() { delete this; }
  virtual int32_t GetType() const { return FDE_BRUSHTYPE_Texture; }
  virtual const CFX_Matrix& GetMatrix() const { return m_Matrix; }
  virtual void ResetMatrix() { m_Matrix.SetIdentity(); }
  virtual void TranslateMatrix(FX_FLOAT dx, FX_FLOAT dy) {
    m_Matrix.Translate(dx, dy);
  }
  virtual void RotateMatrix(FX_FLOAT fRadian) { m_Matrix.Rotate(fRadian); }
  virtual void ScaleMatrix(FX_FLOAT sx, FX_FLOAT sy) { m_Matrix.Scale(sx, sy); }
  virtual void ConcatMatrix(const CFX_Matrix& matrix) {
    m_Matrix.Concat(matrix);
  }
  virtual void SetMatrix(const CFX_Matrix& matrix) { m_Matrix = matrix; }
  virtual IFDE_Image* GetImage() const { return m_pImage; }
  virtual void SetImage(IFDE_Image* pImage, FX_BOOL bAutoRelease) {
    m_pImage = pImage;
    m_bAutoRelease = bAutoRelease;
  }
  virtual int32_t GetWrapMode() const { return m_iWrap; }
  virtual void SetWrapMode(int32_t iWrapMode) { m_iWrap = iWrapMode; }
  int32_t m_iWrap;
  IFDE_Image* m_pImage;
  FX_BOOL m_bAutoRelease;
  CFX_Matrix m_Matrix;
};
class CFDE_LinearBrush : public IFDE_LinearGradientBrush, public CFX_Target {
 public:
  CFDE_LinearBrush() : m_EndColor(0), m_StartColor(0), m_iWrapMode(0) {
    m_StartPoint.x = m_StartPoint.y = m_EndPoint.x = m_EndPoint.y = 0;
    m_Matrix.SetIdentity();
  }

  virtual void Release() { delete this; }
  virtual int32_t GetType() const { return FDE_BRUSHTYPE_LinearGradient; }
  virtual const CFX_Matrix& GetMatrix() const { return m_Matrix; }
  virtual void ResetMatrix() { m_Matrix.SetIdentity(); }
  virtual void TranslateMatrix(FX_FLOAT dx, FX_FLOAT dy) {
    m_Matrix.Translate(dx, dy);
  }
  virtual void RotateMatrix(FX_FLOAT fRadian) { m_Matrix.Rotate(fRadian); }
  virtual void ScaleMatrix(FX_FLOAT sx, FX_FLOAT sy) { m_Matrix.Scale(sx, sy); }
  virtual void ConcatMatrix(const CFX_Matrix& matrix) {
    m_Matrix.Concat(matrix);
  }
  virtual void SetMatrix(const CFX_Matrix& matrix) { m_Matrix = matrix; }
  virtual void GetLinearPoints(CFX_PointF& startingPoint,
                               CFX_PointF& endingPoint) const {
    startingPoint = m_StartPoint;
    endingPoint = m_EndPoint;
  }
  virtual void SetLinearPoints(const CFX_PointF& startingPoint,
                               const CFX_PointF& endingPoint) {
    m_StartPoint = startingPoint;
    m_EndPoint = endingPoint;
  }
  virtual void GetLinearColors(FX_ARGB& startingColor,
                               FX_ARGB& endingColor) const {
    startingColor = m_StartColor;
    endingColor = m_EndColor;
  }
  virtual void SetLinearColors(const FX_ARGB& startingColor,
                               const FX_ARGB& endingColor) {
    m_StartColor = startingColor;
    m_EndColor = endingColor;
  }
  virtual int32_t CountGradientColors() const { return m_GradColors.GetSize(); }
  virtual FX_BOOL GetGradientColors(CFDE_GradientColors& colors) const {
    return colors.Copy(m_GradColors), TRUE;
  }
  virtual FX_BOOL SetGradientColors(const CFDE_GradientColors& colors) {
    return m_GradColors.Copy(colors), TRUE;
  }

  virtual int32_t GetWrapMode() const { return m_iWrapMode; }
  virtual void SetWrapMode(int32_t iWrapMode) { m_iWrapMode = iWrapMode; }
  CFX_PointF m_EndPoint;
  CFX_PointF m_StartPoint;
  FX_ARGB m_EndColor;
  FX_ARGB m_StartColor;
  CFDE_GradientColors m_GradColors;
  int32_t m_iWrapMode;
  CFX_Matrix m_Matrix;
};
#endif
