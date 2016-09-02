// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/fpdf_page/include/cpdf_generalstate.h"

namespace {

int RI_StringToId(const CFX_ByteString& ri) {
  uint32_t id = ri.GetID();
  if (id == FXBSTR_ID('A', 'b', 's', 'o'))
    return 1;

  if (id == FXBSTR_ID('S', 'a', 't', 'u'))
    return 2;

  if (id == FXBSTR_ID('P', 'e', 'r', 'c'))
    return 3;

  return 0;
}

}  // namespace

CPDF_GeneralState::CPDF_GeneralState() {}

CPDF_GeneralState::CPDF_GeneralState(const CPDF_GeneralState& that)
    : m_Ref(that.m_Ref) {}

CPDF_GeneralState::~CPDF_GeneralState() {}

void CPDF_GeneralState::SetRenderIntent(const CFX_ByteString& ri) {
  m_Ref.GetPrivateCopy()->m_RenderIntent = RI_StringToId(ri);
}

int CPDF_GeneralState::GetBlendType() const {
  const CPDF_GeneralStateData* pData = m_Ref.GetObject();
  return pData ? pData->m_BlendType : FXDIB_BLEND_NORMAL;
}

void CPDF_GeneralState::SetBlendType(int type) {
  m_Ref.GetPrivateCopy()->m_BlendType = type;
}

FX_FLOAT CPDF_GeneralState::GetFillAlpha() const {
  const CPDF_GeneralStateData* pData = m_Ref.GetObject();
  return pData ? pData->m_FillAlpha : 1.0f;
}

void CPDF_GeneralState::SetFillAlpha(FX_FLOAT alpha) {
  m_Ref.GetPrivateCopy()->m_FillAlpha = alpha;
}

FX_FLOAT CPDF_GeneralState::GetStrokeAlpha() const {
  const CPDF_GeneralStateData* pData = m_Ref.GetObject();
  return pData ? pData->m_StrokeAlpha : 1.0f;
}

void CPDF_GeneralState::SetStrokeAlpha(FX_FLOAT alpha) {
  m_Ref.GetPrivateCopy()->m_StrokeAlpha = alpha;
}

CPDF_Object* CPDF_GeneralState::GetSoftMask() const {
  const CPDF_GeneralStateData* pData = m_Ref.GetObject();
  return pData ? pData->m_pSoftMask : nullptr;
}

void CPDF_GeneralState::SetSoftMask(CPDF_Object* pObject) {
  m_Ref.GetPrivateCopy()->m_pSoftMask = pObject;
}

CPDF_Object* CPDF_GeneralState::GetTR() const {
  const CPDF_GeneralStateData* pData = m_Ref.GetObject();
  return pData ? pData->m_pTR : nullptr;
}

void CPDF_GeneralState::SetTR(CPDF_Object* pObject) {
  m_Ref.GetPrivateCopy()->m_pTR = pObject;
}

CPDF_TransferFunc* CPDF_GeneralState::GetTransferFunc() const {
  const CPDF_GeneralStateData* pData = m_Ref.GetObject();
  return pData ? pData->m_pTransferFunc : nullptr;
}

void CPDF_GeneralState::SetTransferFunc(CPDF_TransferFunc* pFunc) {
  m_Ref.GetPrivateCopy()->m_pTransferFunc = pFunc;
}

void CPDF_GeneralState::SetBlendMode(const CFX_ByteStringC& mode) {
  m_Ref.GetPrivateCopy()->SetBlendMode(mode);
}

const FX_FLOAT* CPDF_GeneralState::GetSMaskMatrix() const {
  const CPDF_GeneralStateData* pData = m_Ref.GetObject();
  return pData ? pData->m_SMaskMatrix : nullptr;
}

FX_FLOAT* CPDF_GeneralState::GetMutableSMaskMatrix() {
  return m_Ref.GetPrivateCopy()->m_SMaskMatrix;
}

bool CPDF_GeneralState::GetFillOP() const {
  const CPDF_GeneralStateData* pData = m_Ref.GetObject();
  return pData && pData->m_FillOP;
}

void CPDF_GeneralState::SetFillOP(bool op) {
  m_Ref.GetPrivateCopy()->m_FillOP = op;
}

void CPDF_GeneralState::SetStrokeOP(bool op) {
  m_Ref.GetPrivateCopy()->m_StrokeOP = op;
}

bool CPDF_GeneralState::GetStrokeOP() const {
  const CPDF_GeneralStateData* pData = m_Ref.GetObject();
  return pData && pData->m_StrokeOP;
}

int CPDF_GeneralState::GetOPMode() const {
  return m_Ref.GetObject()->m_OPMode;
}

void CPDF_GeneralState::SetOPMode(int mode) {
  m_Ref.GetPrivateCopy()->m_OPMode = mode;
}

void CPDF_GeneralState::SetBG(CPDF_Object* pObject) {
  m_Ref.GetPrivateCopy()->m_pBG = pObject;
}

void CPDF_GeneralState::SetUCR(CPDF_Object* pObject) {
  m_Ref.GetPrivateCopy()->m_pUCR = pObject;
}

void CPDF_GeneralState::SetHT(CPDF_Object* pObject) {
  m_Ref.GetPrivateCopy()->m_pHT = pObject;
}

void CPDF_GeneralState::SetFlatness(FX_FLOAT flatness) {
  m_Ref.GetPrivateCopy()->m_Flatness = flatness;
}

void CPDF_GeneralState::SetSmoothness(FX_FLOAT smoothness) {
  m_Ref.GetPrivateCopy()->m_Smoothness = smoothness;
}

bool CPDF_GeneralState::GetStrokeAdjust() const {
  const CPDF_GeneralStateData* pData = m_Ref.GetObject();
  return pData && pData->m_StrokeAdjust;
}

void CPDF_GeneralState::SetStrokeAdjust(bool adjust) {
  m_Ref.GetPrivateCopy()->m_StrokeAdjust = adjust;
}

void CPDF_GeneralState::SetAlphaSource(bool source) {
  m_Ref.GetPrivateCopy()->m_AlphaSource = source;
}

void CPDF_GeneralState::SetTextKnockout(bool knockout) {
  m_Ref.GetPrivateCopy()->m_TextKnockout = knockout;
}

void CPDF_GeneralState::SetMatrix(const CFX_Matrix& matrix) {
  m_Ref.GetPrivateCopy()->m_Matrix = matrix;
}

CFX_Matrix* CPDF_GeneralState::GetMutableMatrix() {
  return &m_Ref.GetPrivateCopy()->m_Matrix;
}
