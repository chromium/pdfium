// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_PWL_IPWL_SYSTEMHANDLER_H_
#define FPDFSDK_PWL_IPWL_SYSTEMHANDLER_H_

#include <memory>

class CFFL_FormField;
class CFX_FloatRect;

class IPWL_SystemHandler {
 public:
  // These must match the values in public/fpdf_formfill.h
  enum CursorStyle {
    kArrow = 0,
    kNESW = 1,
    kNWSE = 2,
    kVBeam = 3,
    kHBeam = 4,
    kHand = 5,
  };

  class PerWindowData {
   public:
    virtual ~PerWindowData() = default;
    virtual std::unique_ptr<PerWindowData> Clone() const = 0;
  };

  virtual ~IPWL_SystemHandler() = default;

  virtual void InvalidateRect(PerWindowData* pWidgetData,
                              const CFX_FloatRect& rect) = 0;
  virtual void OutputSelectedRect(CFFL_FormField* pFormField,
                                  const CFX_FloatRect& rect) = 0;
  virtual bool IsSelectionImplemented() const = 0;
  virtual void SetCursor(CursorStyle nCursorStyle) = 0;
};

#endif  // FPDFSDK_PWL_IPWL_SYSTEMHANDLER_H_
