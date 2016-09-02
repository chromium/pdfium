// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_FPDF_PAGE_INCLUDE_CPDF_GENERALSTATE_H_
#define CORE_FPDFAPI_FPDF_PAGE_INCLUDE_CPDF_GENERALSTATE_H_

#include "core/fpdfapi/fpdf_page/include/cpdf_generalstatedata.h"
#include "core/fxcrt/include/fx_basic.h"

class CPDF_GeneralState {
 public:
  CPDF_GeneralState();
  CPDF_GeneralState(const CPDF_GeneralState& that);
  ~CPDF_GeneralState();

  void Emplace() { m_Ref.Emplace(); }
  operator bool() const { return !!m_Ref; }

  void SetRenderIntent(const CFX_ByteString& ri);

  int GetBlendType() const;
  void SetBlendType(int type);

  FX_FLOAT GetFillAlpha() const;
  void SetFillAlpha(FX_FLOAT alpha);

  FX_FLOAT GetStrokeAlpha() const;
  void SetStrokeAlpha(FX_FLOAT alpha);

  CPDF_Object* GetSoftMask() const;
  void SetSoftMask(CPDF_Object* pObject);

  CPDF_Object* GetTR() const;
  void SetTR(CPDF_Object* pObject);

  CPDF_TransferFunc* GetTransferFunc() const;
  void SetTransferFunc(CPDF_TransferFunc* pFunc);

  void SetBlendMode(const CFX_ByteStringC& mode);

  const FX_FLOAT* GetSMaskMatrix() const;
  FX_FLOAT* GetMutableSMaskMatrix();

  bool GetFillOP() const;
  void SetFillOP(bool op);

  bool GetStrokeOP() const;
  void SetStrokeOP(bool op);

  int GetOPMode() const;
  void SetOPMode(int mode);

  void SetBG(CPDF_Object* pObject);
  void SetUCR(CPDF_Object* pObject);
  void SetHT(CPDF_Object* pObject);

  void SetFlatness(FX_FLOAT flatness);
  void SetSmoothness(FX_FLOAT smoothness);

  bool GetStrokeAdjust() const;
  void SetStrokeAdjust(bool adjust);

  void SetAlphaSource(bool source);
  void SetTextKnockout(bool knockout);

  void SetMatrix(const CFX_Matrix& matrix);
  CFX_Matrix* GetMutableMatrix();

 private:
  CFX_CountRef<CPDF_GeneralStateData> m_Ref;
};

#endif  // CORE_FPDFAPI_FPDF_PAGE_INCLUDE_CPDF_GENERALSTATE_H_
