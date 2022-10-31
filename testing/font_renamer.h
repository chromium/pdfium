// Copyright 2022 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_FONT_RENAMER_H_
#define TESTING_FONT_RENAMER_H_

#include "core/fxcrt/unowned_ptr.h"
#include "public/fpdf_sysfontinfo.h"

class FontRenamer final : public FPDF_SYSFONTINFO {
 public:
  FontRenamer();
  ~FontRenamer();

  FPDF_SYSFONTINFO* impl() { return impl_; }

 private:
  UnownedPtr<FPDF_SYSFONTINFO> impl_;
};

#endif  // TESTING_FONT_RENAMER_H_
