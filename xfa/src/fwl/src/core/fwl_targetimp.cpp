// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../foxitlib.h"
#include "include/fwl_targetimp.h"
FX_DWORD IFWL_Target::Release() {
  FX_DWORD dwRef = m_pImpl->Release();
  if (!dwRef) {
    m_pImpl = nullptr;
    delete this;
  }
  return dwRef;
}
IFWL_Target* IFWL_Target::Retain() {
  return m_pImpl->Retain();
}
FX_DWORD IFWL_Target::GetRefCount() const {
  return m_pImpl->GetRefCount();
}
FWL_ERR IFWL_Target::GetClassName(CFX_WideString& wsClass) const {
  return m_pImpl->GetClassName(wsClass);
}
FX_DWORD IFWL_Target::GetClassID() const {
  return m_pImpl->GetClassID();
}
FX_BOOL IFWL_Target::IsInstance(const CFX_WideStringC& wsClass) const {
  return m_pImpl->IsInstance(wsClass);
}
FWL_ERR IFWL_Target::Initialize() {
  return m_pImpl->Initialize();
}
FWL_ERR IFWL_Target::Finalize() {
  return m_pImpl->Finalize();
}
IFWL_Target::~IFWL_Target() {
  delete m_pImpl;
}
CFWL_TargetImp::CFWL_TargetImp() : m_dwRefCount(1) {
}
CFWL_TargetImp::~CFWL_TargetImp() {
}
FX_DWORD CFWL_TargetImp::Release() {
  m_dwRefCount--;
  FX_DWORD dwRet = m_dwRefCount;
  if (!m_dwRefCount) {
    delete this;
  }
  return dwRet;
}
IFWL_Target* CFWL_TargetImp::Retain() {
  m_dwRefCount++;
  return (IFWL_Target*)this;
}
FX_DWORD CFWL_TargetImp::GetRefCount() const {
  return m_dwRefCount;
}
FWL_ERR CFWL_TargetImp::GetClassName(CFX_WideString& wsClass) const {
  return FWL_ERR_Succeeded;
}
FX_DWORD CFWL_TargetImp::GetClassID() const {
  return 0;
}
FX_BOOL CFWL_TargetImp::IsInstance(const CFX_WideStringC& wsClass) const {
  return FALSE;
}
FWL_ERR CFWL_TargetImp::Initialize() {
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_TargetImp::Finalize() {
  return FWL_ERR_Succeeded;
}
