// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_RENDER_CPDF_RENDERCONTEXT_H_
#define CORE_FPDFAPI_RENDER_CPDF_RENDERCONTEXT_H_

#include <vector>

#include "build/build_config.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/unowned_ptr.h"

class CFX_DIBitmap;
class CFX_Matrix;
class CFX_RenderDevice;
class CPDF_Dictionary;
class CPDF_Document;
class CPDF_PageImageCache;
class CPDF_PageObject;
class CPDF_PageObjectHolder;
class CPDF_RenderOptions;

class CPDF_RenderContext {
 public:
  class Layer {
   public:
    Layer(CPDF_PageObjectHolder* pHolder, const CFX_Matrix& matrix);
    Layer(const Layer& that);
    ~Layer();

    CPDF_PageObjectHolder* GetObjectHolder() { return object_holder_; }
    const CFX_Matrix& GetMatrix() const { return matrix_; }

   private:
    UnownedPtr<CPDF_PageObjectHolder> const object_holder_;
    const CFX_Matrix matrix_;
  };

  CPDF_RenderContext(CPDF_Document* doc,
                     RetainPtr<CPDF_Dictionary> pPageResources,
                     CPDF_PageImageCache* pPageCache);
  ~CPDF_RenderContext();

  void AppendLayer(CPDF_PageObjectHolder* pObjectHolder,
                   const CFX_Matrix& mtObject2Device);

  void Render(CFX_RenderDevice* pDevice,
              const CPDF_PageObject* pStopObj,
              const CPDF_RenderOptions* pOptions,
              const CFX_Matrix* pLastMatrix);

  void GetBackgroundToDevice(CFX_RenderDevice* device,
                             const CPDF_PageObject* object,
                             const CPDF_RenderOptions* options,
                             const CFX_Matrix& matrix);
#if BUILDFLAG(IS_WIN)
  void GetBackgroundToBitmap(RetainPtr<CFX_DIBitmap> bitmap,
                             const CPDF_PageObject* object,
                             const CFX_Matrix& matrix);
#endif

  size_t CountLayers() const { return layers_.size(); }
  Layer* GetLayer(uint32_t index) { return &layers_[index]; }

  CPDF_Document* GetDocument() const { return document_; }
  const CPDF_Dictionary* GetPageResources() const {
    return page_resources_.Get();
  }
  RetainPtr<CPDF_Dictionary> GetMutablePageResources() {
    return page_resources_;
  }
  CPDF_PageImageCache* GetPageCache() const { return page_cache_; }

 private:
  UnownedPtr<CPDF_Document> const document_;
  RetainPtr<CPDF_Dictionary> const page_resources_;
  UnownedPtr<CPDF_PageImageCache> const page_cache_;
  std::vector<Layer> layers_;
};

#endif  // CORE_FPDFAPI_RENDER_CPDF_RENDERCONTEXT_H_
