// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PAGE_CPDF_GENERALSTATE_H_
#define CORE_FPDFAPI_PAGE_CPDF_GENERALSTATE_H_

#include "core/fxcrt/cfx_shared_copy_on_write.h"
#include "core/fxcrt/cfx_unowned_ptr.h"
#include "core/fxcrt/fx_basic.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxge/fx_dib.h"

class CPDF_Object;
class CPDF_TransferFunc;

class CPDF_GeneralState {
 public:
  CPDF_GeneralState();
  CPDF_GeneralState(const CPDF_GeneralState& that);
  ~CPDF_GeneralState();

  void Emplace() { m_Ref.Emplace(); }
  bool HasRef() const { return !!m_Ref; }

  void SetRenderIntent(const CFX_ByteString& ri);

  CFX_ByteString GetBlendMode() const;
  int GetBlendType() const;
  void SetBlendType(int type);

  float GetFillAlpha() const;
  void SetFillAlpha(float alpha);

  float GetStrokeAlpha() const;
  void SetStrokeAlpha(float alpha);

  CPDF_Object* GetSoftMask() const;
  void SetSoftMask(CPDF_Object* pObject);

  CPDF_Object* GetTR() const;
  void SetTR(CPDF_Object* pObject);

  CFX_RetainPtr<CPDF_TransferFunc> GetTransferFunc() const;
  void SetTransferFunc(const CFX_RetainPtr<CPDF_TransferFunc>& pFunc);

  void SetBlendMode(const CFX_ByteString& mode);

  const CFX_Matrix* GetSMaskMatrix() const;
  void SetSMaskMatrix(const CFX_Matrix& matrix);

  bool GetFillOP() const;
  void SetFillOP(bool op);

  bool GetStrokeOP() const;
  void SetStrokeOP(bool op);

  int GetOPMode() const;
  void SetOPMode(int mode);

  void SetBG(CPDF_Object* pObject);
  void SetUCR(CPDF_Object* pObject);
  void SetHT(CPDF_Object* pObject);

  void SetFlatness(float flatness);
  void SetSmoothness(float smoothness);

  bool GetStrokeAdjust() const;
  void SetStrokeAdjust(bool adjust);

  void SetAlphaSource(bool source);
  void SetTextKnockout(bool knockout);

  void SetMatrix(const CFX_Matrix& matrix);
  CFX_Matrix* GetMutableMatrix();

 private:
  class StateData {
   public:
    StateData();
    StateData(const StateData& that);
    ~StateData();

    CFX_ByteString m_BlendMode;
    int m_BlendType;
    CFX_UnownedPtr<CPDF_Object> m_pSoftMask;
    CFX_Matrix m_SMaskMatrix;
    float m_StrokeAlpha;
    float m_FillAlpha;
    CFX_UnownedPtr<CPDF_Object> m_pTR;
    CFX_RetainPtr<CPDF_TransferFunc> m_pTransferFunc;
    CFX_Matrix m_Matrix;
    int m_RenderIntent;
    bool m_StrokeAdjust;
    bool m_AlphaSource;
    bool m_TextKnockout;
    bool m_StrokeOP;
    bool m_FillOP;
    int m_OPMode;
    CFX_UnownedPtr<CPDF_Object> m_pBG;
    CFX_UnownedPtr<CPDF_Object> m_pUCR;
    CFX_UnownedPtr<CPDF_Object> m_pHT;
    float m_Flatness;
    float m_Smoothness;
  };

  CFX_SharedCopyOnWrite<StateData> m_Ref;
};

#endif  // CORE_FPDFAPI_PAGE_CPDF_GENERALSTATE_H_
