// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_CPDF_MODULEMGR_H_
#define CORE_FPDFAPI_CPDF_MODULEMGR_H_

class CPDF_ModuleMgr {
 public:
  static void Create();
  static void Destroy();

 private:
  CPDF_ModuleMgr() = delete;
  ~CPDF_ModuleMgr() = delete;
};

#endif  // CORE_FPDFAPI_CPDF_MODULEMGR_H_
