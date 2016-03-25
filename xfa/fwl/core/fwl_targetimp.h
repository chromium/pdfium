// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CORE_FWL_TARGETIMP_H_
#define XFA_FWL_CORE_FWL_TARGETIMP_H_

#include "core/fxcrt/include/fx_basic.h"
#include "xfa/fwl/core/ifwl_target.h"

class CFWL_TargetImp {
 public:
  virtual ~CFWL_TargetImp();

  virtual FWL_ERR GetClassName(CFX_WideString& wsClass) const;
  virtual uint32_t GetClassID() const;
  virtual FX_BOOL IsInstance(const CFX_WideStringC& wsClass) const;
  virtual FWL_ERR Initialize();
  virtual FWL_ERR Finalize();

 protected:
  CFWL_TargetImp();
};

#endif  // XFA_FWL_CORE_FWL_TARGETIMP_H_
