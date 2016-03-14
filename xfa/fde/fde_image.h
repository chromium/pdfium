// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_FDE_IMAGE_H_
#define XFA_FDE_FDE_IMAGE_H_

#include "xfa/fgas/crt/fgas_memory.h"
#include "xfa/fgas/crt/fgas_stream.h"
#include "xfa/fgas/crt/fgas_utils.h"

class IFDE_Image {
 public:
  virtual ~IFDE_Image() {}
  virtual void Release() = 0;
  virtual FX_BOOL LoadImage() = 0;
  virtual void FreeImage() = 0;
  virtual int32_t CountFrames() const = 0;
  virtual FX_BOOL LoadFrame(int32_t index) = 0;
  virtual CFX_DIBitmap* GetFrameImage() = 0;
  virtual int32_t GetImageFormat() const = 0;
  virtual int32_t GetImageWidth() const = 0;
  virtual int32_t GetImageHeight() const = 0;
  virtual int32_t GetDelayTime(int32_t iFrameIndex) const = 0;
  virtual int32_t GetLoopCount() const = 0;
  virtual FX_BOOL StartLoadImage(CFX_DIBitmap* pDIBitmap,
                                 int32_t dibX,
                                 int32_t dibY,
                                 int32_t dibCX,
                                 int32_t dibCY,
                                 int32_t imgX,
                                 int32_t imgY,
                                 int32_t imgCX,
                                 int32_t imgCY,
                                 int32_t iFrameIndex = 0) = 0;
  virtual int32_t DoLoadImage(IFX_Pause* pPause = NULL) = 0;
  virtual void StopLoadImage() = 0;
};

#endif  // XFA_FDE_FDE_IMAGE_H_
