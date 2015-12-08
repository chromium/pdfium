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
  virtual ~CFWL_TargetImp();

  virtual FWL_ERR GetClassName(CFX_WideString& wsClass) const;
  virtual FX_DWORD GetClassID() const;
  virtual FX_BOOL IsInstance(const CFX_WideStringC& wsClass) const;
  virtual FWL_ERR Initialize();
  virtual FWL_ERR Finalize();

 protected:
  CFWL_TargetImp();
};

#endif  // FWL_TARGETIMP_H_
