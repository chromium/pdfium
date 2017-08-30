// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_IFX_PAUSEINDICATOR_H_
#define CORE_FXCRT_IFX_PAUSEINDICATOR_H_

class IFX_PauseIndicator {
 public:
  virtual ~IFX_PauseIndicator() {}
  virtual bool NeedToPauseNow() = 0;
};

#endif  // CORE_FXCRT_IFX_PAUSEINDICATOR_H_
