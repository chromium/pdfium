// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CFWL_EVENTCHECKWORD_H_
#define XFA_FWL_CFWL_EVENTCHECKWORD_H_

#include "xfa/fwl/cfwl_event.h"

class CFWL_EventCheckWord : public CFWL_Event {
 public:
  explicit CFWL_EventCheckWord(CFWL_Widget* pSrcTarget);
  ~CFWL_EventCheckWord() override;

  ByteString bsWord;
  bool bCheckWord;
};

#endif  // XFA_FWL_CFWL_EVENTCHECKWORD_H_
