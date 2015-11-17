// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FWL_TARGETIMP_H_
#define FWL_TARGETIMP_H_

#include "core/include/fxcrt/fx_basic.h"
#include "xfa/include/fwl/core/fwl_target.h"

class CFWL_TargetImp {
 public:
  virtual FX_DWORD Release();
  virtual IFWL_Target* Retain();
  virtual FX_DWORD GetRefCount() const;
  virtual FWL_ERR GetClassName(CFX_WideString& wsClass) const;
  virtual FX_DWORD GetClassID() const;
  virtual FX_BOOL IsInstance(const CFX_WideStringC& wsClass) const;
  virtual FWL_ERR Initialize();
  virtual FWL_ERR Finalize();

 protected:
  CFWL_TargetImp();
  virtual ~CFWL_TargetImp();
  FX_DWORD m_dwRefCount;
};

class IFWL_TargetData : public IFWL_Target {
 public:
  CFWL_TargetImp* GetData() const { return m_pImpl; }
  void SetData(CFWL_TargetImp* pImpl) { m_pImpl = pImpl; }
};

#endif  // FWL_TARGETIMP_H_
