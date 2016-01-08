// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/foxitlib.h"
#include "xfa/src/fwl/src/core/include/fwl_targetimp.h"
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
CFWL_TargetImp::CFWL_TargetImp() {}
CFWL_TargetImp::~CFWL_TargetImp() {
}
FWL_ERR CFWL_TargetImp::GetClassName(CFX_WideString& wsClass) const {
  wsClass.Empty();
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
