// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFDOC_CPDF_ANNOTLIST_H_
#define CORE_FPDFDOC_CPDF_ANNOTLIST_H_

#include <memory>
#include <vector>

#include "core/fpdfapi/render/cpdf_pagerendercontext.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/unowned_ptr.h"

class CFX_RenderDevice;
class CPDF_Annot;
class CPDF_Document;
class CPDF_Page;
class CPDF_RenderContext;
class CPDF_RenderOptions;

class CPDF_AnnotList : public CPDF_PageRenderContext::AnnotListIface {
 public:
  explicit CPDF_AnnotList(CPDF_Page* pPage);
  ~CPDF_AnnotList() override;

  void DisplayAnnots(CPDF_Page* pPage,
                     CPDF_RenderContext* pContext,
                     bool bPrinting,
                     const CFX_Matrix* pMatrix,
                     bool bShowWidget,
                     CPDF_RenderOptions* pOptions);

  void DisplayAnnots(CPDF_Page* pPage,
                     CFX_RenderDevice* pDevice,
                     CPDF_RenderContext* pContext,
                     bool bPrinting,
                     const CFX_Matrix* pUser2Device,
                     uint32_t dwAnnotFlags,
                     CPDF_RenderOptions* pOptions,
                     FX_RECT* pClipRect);

  size_t Count() const { return m_AnnotList.size(); }
  CPDF_Annot* GetAt(size_t index) const { return m_AnnotList[index].get(); }
  const std::vector<std::unique_ptr<CPDF_Annot>>& All() const {
    return m_AnnotList;
  }

 private:
  void DisplayPass(CPDF_Page* pPage,
                   CFX_RenderDevice* pDevice,
                   CPDF_RenderContext* pContext,
                   bool bPrinting,
                   const CFX_Matrix* pMatrix,
                   bool bWidget,
                   CPDF_RenderOptions* pOptions,
                   FX_RECT* clip_rect);

  UnownedPtr<CPDF_Document> const m_pDocument;

  // The first |m_nAnnotCount| elements are from the PDF itself. The rest are
  // generated pop-up annotations.
  std::vector<std::unique_ptr<CPDF_Annot>> m_AnnotList;
  size_t m_nAnnotCount = 0;
};

#endif  // CORE_FPDFDOC_CPDF_ANNOTLIST_H_
