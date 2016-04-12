// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_FDE_OBJECT_H_
#define XFA_FDE_FDE_OBJECT_H_

#include <cstdint>

#include "core/fxge/include/fx_dib.h"
#include "xfa/fde/fde_brush.h"
#include "xfa/fde/fde_pen.h"
#include "xfa/fgas/crt/fgas_memory.h"

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

#endif  // XFA_FDE_FDE_OBJECT_H_
