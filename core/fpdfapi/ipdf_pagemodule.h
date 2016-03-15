// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_IPDF_PAGEMODULE_H_
#define CORE_FPDFAPI_IPDF_PAGEMODULE_H_

class CPDF_ColorSpace;
class CPDF_DocPageData;
class CPDF_Document;
class CPDF_FontGlobals;

class IPDF_PageModule {
 public:
  virtual ~IPDF_PageModule() {}

  virtual CPDF_DocPageData* CreateDocData(CPDF_Document* pDoc) = 0;
  virtual void ReleaseDoc(CPDF_Document* pDoc) = 0;
  virtual void ClearDoc(CPDF_Document* pDoc) = 0;
  virtual CPDF_FontGlobals* GetFontGlobals() = 0;
  virtual void ClearStockFont(CPDF_Document* pDoc) = 0;
  virtual void NotifyCJKAvailable() = 0;
  virtual CPDF_ColorSpace* GetStockCS(int family) = 0;
};

#endif  // CORE_FPDFAPI_IPDF_PAGEMODULE_H_
