// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CFWL_EVENTVALIDATE_H_
#define XFA_FWL_CFWL_EVENTVALIDATE_H_

#include "core/fxcrt/widestring.h"
#include "xfa/fwl/cfwl_event.h"

class CFWL_EventValidate final : public CFWL_Event {
 public:
  CFWL_EventValidate(CFWL_Widget* pSrcTarget, const WideString& wsInsert);
  ~CFWL_EventValidate() override;

  WideString GetInsert() const { return m_wsInsert; }
  bool GetValidate() const { return m_bValidate; }
  void SetValidate(bool bValidate) { m_bValidate = bValidate; }

 protected:
  const WideString m_wsInsert;
  bool m_bValidate = true;
};

#endif  // XFA_FWL_CFWL_EVENTVALIDATE_H_
