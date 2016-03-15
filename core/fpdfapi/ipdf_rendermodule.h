// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_IPDF_RENDERMODULE_H_
#define CORE_FPDFAPI_IPDF_RENDERMODULE_H_

class CPDF_DocRenderData;
class CPDF_Document;
class CPDF_Page;
class CPDF_PageRenderCache;

class IPDF_RenderModule {
 public:
  virtual ~IPDF_RenderModule() {}

  virtual CPDF_DocRenderData* CreateDocData(CPDF_Document* pDoc) = 0;
  virtual void DestroyDocData(CPDF_DocRenderData* pDocRenderData) = 0;
  virtual void ClearDocData(CPDF_DocRenderData* pDocRenderData) = 0;

  virtual CPDF_DocRenderData* GetRenderData() = 0;

  virtual CPDF_PageRenderCache* CreatePageCache(CPDF_Page* pPage) = 0;
  virtual void DestroyPageCache(CPDF_PageRenderCache* pCache) = 0;
};

#endif  // CORE_FPDFAPI_IPDF_RENDERMODULE_H_
