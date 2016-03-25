// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CORE_CFWL_NOTE_H_
#define XFA_FWL_CORE_CFWL_NOTE_H_

#include "core/fxcrt/include/fx_string.h"
#include "core/fxcrt/include/fx_system.h"
#include "xfa/fwl/core/fwl_error.h"

class IFWL_Widget;

// Separate hierarchy not related to IFWL_* hierarchy. These should not
// get cast to IFWL_* types.
class CFWL_Note {
 public:
  virtual uint32_t Release() {
    m_dwRefCount--;
    uint32_t dwRefCount = m_dwRefCount;
    if (!m_dwRefCount)
      delete this;
    return dwRefCount;
  }

  virtual CFWL_Note* Retain() {
    m_dwRefCount++;
    return this;
  }

  virtual FWL_ERR GetClassName(CFX_WideString& wsClass) const {
    wsClass = L"CFWL_Note";
    return FWL_ERR_Succeeded;
  }

  virtual uint32_t GetClassID() const { return 0; }

  virtual FX_BOOL IsInstance(const CFX_WideStringC& wsClass) const {
    return TRUE;
  }

  virtual CFWL_Note* Clone() { return NULL; }
  FX_BOOL IsEvent() const { return m_bIsEvent; }

  IFWL_Widget* m_pSrcTarget;
  IFWL_Widget* m_pDstTarget;
  uint32_t m_dwExtend;

 protected:
  CFWL_Note(FX_BOOL bIsEvent)
      : m_pSrcTarget(NULL),
        m_pDstTarget(NULL),
        m_dwExtend(0),
        m_dwRefCount(1),
        m_bIsEvent(bIsEvent) {}

  virtual ~CFWL_Note() {}
  virtual FX_BOOL Initialize() { return TRUE; }
  virtual int32_t Finalize() { return 0; }

  uint32_t m_dwRefCount;
  FX_BOOL m_bIsEvent;
};

#endif  // XFA_FWL_CORE_CFWL_NOTE_H_
