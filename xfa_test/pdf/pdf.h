// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PDF_PDF_H_
#define PDF_PDF_H_

#include "ppapi/cpp/module.h"

namespace chrome_pdf {

class PDFModule : public pp::Module {
 public:
  PDFModule();
  virtual ~PDFModule();

  // pp::Module implementation.
  virtual bool Init();
  virtual pp::Instance* CreateInstance(PP_Instance instance);
};

}  // namespace chrome_pdf

#endif  // PDF_PDF_H_
