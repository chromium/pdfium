// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FGAS_LAYOUT_CFGAS_LINKUSERDATA_H_
#define XFA_FGAS_LAYOUT_CFGAS_LINKUSERDATA_H_

#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/widestring.h"

class CFGAS_LinkUserData final : public Retainable {
 public:
  CONSTRUCT_VIA_MAKE_RETAIN;

  WideString GetLinkURL() const { return m_wsURLContent; }

 private:
  explicit CFGAS_LinkUserData(const WideString& wsText);
  ~CFGAS_LinkUserData() override;

  WideString m_wsURLContent;
};

#endif  // XFA_FGAS_LAYOUT_CFGAS_LINKUSERDATA_H_
