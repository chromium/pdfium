// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CORE_IFWL_TARGET_H_
#define XFA_FWL_CORE_IFWL_TARGET_H_

#include "core/fxcrt/include/fx_basic.h"
#include "xfa/fwl/core/fwl_error.h"

// FWL contains three parallel inheritance hierarchies, which reference each
// other via pointers as follows:
//
//                                           m_pImpl
//      (nonesuch)              IFWL_Target ----------> CFWL_TargetImp
//                                   |                       |
//                                   A                       A
//                   m_pIface        |                       |
//      CFWL_Widget ----------> IFWL_Widget             CFWL_WidgetImp
//           |                       |                       |
//           A                       A                       A
//           |                       |                       |
//      CFWL_...                IFWL_...                CFWL_...Imp
//

class CFWL_TargetImp;

class IFWL_Target {
 public:
  IFWL_Target() : m_pImpl(nullptr) {}
  virtual ~IFWL_Target();

  // These call into equivalent polymorphic methods of m_pImpl. There
  // should be no need to override these in subclasses.
  FWL_ERR GetClassName(CFX_WideString& wsClass) const;
  uint32_t GetClassID() const;
  FX_BOOL IsInstance(const CFX_WideStringC& wsClass) const;
  FWL_ERR Initialize();
  FWL_ERR Finalize();

  CFWL_TargetImp* GetImpl() const { return m_pImpl; }

 protected:
  void SetImpl(CFWL_TargetImp* pImpl) { m_pImpl = pImpl; }

 private:
  CFWL_TargetImp* m_pImpl;
};

#endif  // XFA_FWL_CORE_IFWL_TARGET_H_
