// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PAGE_CPDF_PAGEMODULE_H_
#define CORE_FPDFAPI_PAGE_CPDF_PAGEMODULE_H_

class CPDF_Document;

class CPDF_PageModule {
 public:
  // Per-process singleton managed by callers.
  static void Create();
  static void Destroy();
  static CPDF_PageModule* GetInstance();

  void ClearStockFont(CPDF_Document* pDoc);

 private:
  CPDF_PageModule();
  ~CPDF_PageModule();
};

#endif  // CORE_FPDFAPI_PAGE_CPDF_PAGEMODULE_H_
