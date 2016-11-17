// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/render/render_int.h"

#include <memory>

#include "core/fpdfapi/cpdf_modulemgr.h"
#include "core/fpdfapi/font/cpdf_type3char.h"
#include "core/fpdfapi/font/cpdf_type3font.h"
#include "core/fpdfapi/page/cpdf_docpagedata.h"
#include "core/fpdfapi/page/cpdf_form.h"
#include "core/fpdfapi/page/cpdf_formobject.h"
#include "core/fpdfapi/page/cpdf_graphicstates.h"
#include "core/fpdfapi/page/cpdf_image.h"
#include "core/fpdfapi/page/cpdf_imageobject.h"
#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfapi/page/cpdf_pageobject.h"
#include "core/fpdfapi/page/cpdf_pathobject.h"
#include "core/fpdfapi/page/cpdf_textobject.h"
#include "core/fpdfapi/page/pageint.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/render/cpdf_docrenderdata.h"
#include "core/fpdfapi/render/cpdf_pagerendercache.h"
#include "core/fpdfapi/render/cpdf_progressiverenderer.h"
#include "core/fpdfapi/render/cpdf_renderoptions.h"
#include "core/fpdfapi/render/cpdf_renderstatus.h"
#include "core/fpdfapi/render/cpdf_textrenderer.h"
#include "core/fpdfapi/render/cpdf_type3cache.h"
#include "core/fpdfdoc/cpdf_occontext.h"
#include "core/fxge/cfx_fxgedevice.h"
#include "core/fxge/cfx_graphstatedata.h"
#include "core/fxge/cfx_pathdata.h"
#include "core/fxge/cfx_renderdevice.h"

CPDF_RenderOptions::CPDF_RenderOptions()
    : m_ColorMode(RENDER_COLOR_NORMAL),
      m_Flags(RENDER_CLEARTYPE),
      m_Interpolation(0),
      m_AddFlags(0),
      m_pOCContext(nullptr),
      m_dwLimitCacheSize(1024 * 1024 * 100),
      m_HalftoneLimit(-1),
      m_bDrawAnnots(false) {}

CPDF_RenderOptions::CPDF_RenderOptions(const CPDF_RenderOptions& rhs)
    : m_ColorMode(rhs.m_ColorMode),
      m_BackColor(rhs.m_BackColor),
      m_ForeColor(rhs.m_ForeColor),
      m_Flags(rhs.m_Flags),
      m_Interpolation(rhs.m_Interpolation),
      m_AddFlags(rhs.m_AddFlags),
      m_pOCContext(rhs.m_pOCContext),
      m_dwLimitCacheSize(rhs.m_dwLimitCacheSize),
      m_HalftoneLimit(rhs.m_HalftoneLimit),
      m_bDrawAnnots(rhs.m_bDrawAnnots) {}

FX_ARGB CPDF_RenderOptions::TranslateColor(FX_ARGB argb) const {
  if (m_ColorMode == RENDER_COLOR_NORMAL) {
    return argb;
  }
  if (m_ColorMode == RENDER_COLOR_ALPHA) {
    return argb;
  }
  int a, r, g, b;
  ArgbDecode(argb, a, r, g, b);
  int gray = FXRGB2GRAY(r, g, b);
  if (m_ColorMode == RENDER_COLOR_TWOCOLOR) {
    int color = (r - gray) * (r - gray) + (g - gray) * (g - gray) +
                (b - gray) * (b - gray);
    if (gray < 35 && color < 20) {
      return ArgbEncode(a, m_ForeColor);
    }
    if (gray > 221 && color < 20) {
      return ArgbEncode(a, m_BackColor);
    }
    return argb;
  }
  int fr = FXSYS_GetRValue(m_ForeColor);
  int fg = FXSYS_GetGValue(m_ForeColor);
  int fb = FXSYS_GetBValue(m_ForeColor);
  int br = FXSYS_GetRValue(m_BackColor);
  int bg = FXSYS_GetGValue(m_BackColor);
  int bb = FXSYS_GetBValue(m_BackColor);
  r = (br - fr) * gray / 255 + fr;
  g = (bg - fg) * gray / 255 + fg;
  b = (bb - fb) * gray / 255 + fb;
  return ArgbEncode(a, r, g, b);
}

void CPDF_RenderContext::GetBackground(CFX_DIBitmap* pBuffer,
                                       const CPDF_PageObject* pObj,
                                       const CPDF_RenderOptions* pOptions,
                                       CFX_Matrix* pFinalMatrix) {
  CFX_FxgeDevice device;
  device.Attach(pBuffer, false, nullptr, false);

  FX_RECT rect(0, 0, device.GetWidth(), device.GetHeight());
  device.FillRect(&rect, 0xffffffff);
  Render(&device, pObj, pOptions, pFinalMatrix);
}

CPDF_RenderContext::CPDF_RenderContext(CPDF_Page* pPage)
    : m_pDocument(pPage->m_pDocument),
      m_pPageResources(pPage->m_pPageResources),
      m_pPageCache(pPage->GetRenderCache()) {}

CPDF_RenderContext::CPDF_RenderContext(CPDF_Document* pDoc,
                                       CPDF_PageRenderCache* pPageCache)
    : m_pDocument(pDoc), m_pPageResources(nullptr), m_pPageCache(pPageCache) {}

CPDF_RenderContext::~CPDF_RenderContext() {}

void CPDF_RenderContext::AppendLayer(CPDF_PageObjectHolder* pObjectHolder,
                                     const CFX_Matrix* pObject2Device) {
  Layer* pLayer = m_Layers.AddSpace();
  pLayer->m_pObjectHolder = pObjectHolder;
  if (pObject2Device) {
    pLayer->m_Matrix = *pObject2Device;
  } else {
    pLayer->m_Matrix.SetIdentity();
  }
}
void CPDF_RenderContext::Render(CFX_RenderDevice* pDevice,
                                const CPDF_RenderOptions* pOptions,
                                const CFX_Matrix* pLastMatrix) {
  Render(pDevice, nullptr, pOptions, pLastMatrix);
}
void CPDF_RenderContext::Render(CFX_RenderDevice* pDevice,
                                const CPDF_PageObject* pStopObj,
                                const CPDF_RenderOptions* pOptions,
                                const CFX_Matrix* pLastMatrix) {
  int count = m_Layers.GetSize();
  for (int j = 0; j < count; j++) {
    pDevice->SaveState();
    Layer* pLayer = m_Layers.GetDataPtr(j);
    if (pLastMatrix) {
      CFX_Matrix FinalMatrix = pLayer->m_Matrix;
      FinalMatrix.Concat(*pLastMatrix);
      CPDF_RenderStatus status;
      status.Initialize(this, pDevice, pLastMatrix, pStopObj, nullptr, nullptr,
                        pOptions, pLayer->m_pObjectHolder->m_Transparency,
                        false, nullptr);
      status.RenderObjectList(pLayer->m_pObjectHolder, &FinalMatrix);
      if (status.m_Options.m_Flags & RENDER_LIMITEDIMAGECACHE) {
        m_pPageCache->CacheOptimization(status.m_Options.m_dwLimitCacheSize);
      }
      if (status.m_bStopped) {
        pDevice->RestoreState(false);
        break;
      }
    } else {
      CPDF_RenderStatus status;
      status.Initialize(this, pDevice, nullptr, pStopObj, nullptr, nullptr,
                        pOptions, pLayer->m_pObjectHolder->m_Transparency,
                        false, nullptr);
      status.RenderObjectList(pLayer->m_pObjectHolder, &pLayer->m_Matrix);
      if (status.m_Options.m_Flags & RENDER_LIMITEDIMAGECACHE) {
        m_pPageCache->CacheOptimization(status.m_Options.m_dwLimitCacheSize);
      }
      if (status.m_bStopped) {
        pDevice->RestoreState(false);
        break;
      }
    }
    pDevice->RestoreState(false);
  }
}

CPDF_ProgressiveRenderer::CPDF_ProgressiveRenderer(
    CPDF_RenderContext* pContext,
    CFX_RenderDevice* pDevice,
    const CPDF_RenderOptions* pOptions)
    : m_Status(Ready),
      m_pContext(pContext),
      m_pDevice(pDevice),
      m_pOptions(pOptions),
      m_LayerIndex(0),
      m_pCurrentLayer(nullptr) {}

CPDF_ProgressiveRenderer::~CPDF_ProgressiveRenderer() {
  if (m_pRenderStatus)
    m_pDevice->RestoreState(false);
}

void CPDF_ProgressiveRenderer::Start(IFX_Pause* pPause) {
  if (!m_pContext || !m_pDevice || m_Status != Ready) {
    m_Status = Failed;
    return;
  }
  m_Status = ToBeContinued;
  Continue(pPause);
}

void CPDF_ProgressiveRenderer::Continue(IFX_Pause* pPause) {
  while (m_Status == ToBeContinued) {
    if (!m_pCurrentLayer) {
      if (m_LayerIndex >= m_pContext->CountLayers()) {
        m_Status = Done;
        return;
      }
      m_pCurrentLayer = m_pContext->GetLayer(m_LayerIndex);
      m_LastObjectRendered =
          m_pCurrentLayer->m_pObjectHolder->GetPageObjectList()->end();
      m_pRenderStatus.reset(new CPDF_RenderStatus());
      m_pRenderStatus->Initialize(
          m_pContext, m_pDevice, nullptr, nullptr, nullptr, nullptr, m_pOptions,
          m_pCurrentLayer->m_pObjectHolder->m_Transparency, false, nullptr);
      m_pDevice->SaveState();
      m_ClipRect = CFX_FloatRect(m_pDevice->GetClipBox());
      CFX_Matrix device2object;
      device2object.SetReverse(m_pCurrentLayer->m_Matrix);
      device2object.TransformRect(m_ClipRect);
    }
    CPDF_PageObjectList::iterator iter;
    CPDF_PageObjectList::iterator iterEnd =
        m_pCurrentLayer->m_pObjectHolder->GetPageObjectList()->end();
    if (m_LastObjectRendered != iterEnd) {
      iter = m_LastObjectRendered;
      ++iter;
    } else {
      iter = m_pCurrentLayer->m_pObjectHolder->GetPageObjectList()->begin();
    }
    int nObjsToGo = kStepLimit;
    while (iter != iterEnd) {
      CPDF_PageObject* pCurObj = iter->get();
      if (pCurObj && pCurObj->m_Left <= m_ClipRect.right &&
          pCurObj->m_Right >= m_ClipRect.left &&
          pCurObj->m_Bottom <= m_ClipRect.top &&
          pCurObj->m_Top >= m_ClipRect.bottom) {
        if (m_pRenderStatus->ContinueSingleObject(
                pCurObj, &m_pCurrentLayer->m_Matrix, pPause)) {
          return;
        }
        if (pCurObj->IsImage() &&
            m_pRenderStatus->m_Options.m_Flags & RENDER_LIMITEDIMAGECACHE) {
          m_pContext->GetPageCache()->CacheOptimization(
              m_pRenderStatus->m_Options.m_dwLimitCacheSize);
        }
        if (pCurObj->IsForm() || pCurObj->IsShading()) {
          nObjsToGo = 0;
        } else {
          --nObjsToGo;
        }
      }
      m_LastObjectRendered = iter;
      if (nObjsToGo == 0) {
        if (pPause && pPause->NeedToPauseNow())
          return;
        nObjsToGo = kStepLimit;
      }
      ++iter;
    }
    if (m_pCurrentLayer->m_pObjectHolder->IsParsed()) {
      m_pRenderStatus.reset();
      m_pDevice->RestoreState(false);
      m_pCurrentLayer = nullptr;
      m_LayerIndex++;
      if (pPause && pPause->NeedToPauseNow()) {
        return;
      }
    } else {
      m_pCurrentLayer->m_pObjectHolder->ContinueParse(pPause);
      if (!m_pCurrentLayer->m_pObjectHolder->IsParsed())
        return;
    }
  }
}

CPDF_DeviceBuffer::CPDF_DeviceBuffer()
    : m_pDevice(nullptr), m_pContext(nullptr), m_pObject(nullptr) {}

CPDF_DeviceBuffer::~CPDF_DeviceBuffer() {}

bool CPDF_DeviceBuffer::Initialize(CPDF_RenderContext* pContext,
                                   CFX_RenderDevice* pDevice,
                                   FX_RECT* pRect,
                                   const CPDF_PageObject* pObj,
                                   int max_dpi) {
  m_pDevice = pDevice;
  m_pContext = pContext;
  m_Rect = *pRect;
  m_pObject = pObj;
  m_Matrix.TranslateI(-pRect->left, -pRect->top);
#if _FXM_PLATFORM_ != _FXM_PLATFORM_APPLE_
  int horz_size = pDevice->GetDeviceCaps(FXDC_HORZ_SIZE);
  int vert_size = pDevice->GetDeviceCaps(FXDC_VERT_SIZE);
  if (horz_size && vert_size && max_dpi) {
    int dpih =
        pDevice->GetDeviceCaps(FXDC_PIXEL_WIDTH) * 254 / (horz_size * 10);
    int dpiv =
        pDevice->GetDeviceCaps(FXDC_PIXEL_HEIGHT) * 254 / (vert_size * 10);
    if (dpih > max_dpi) {
      m_Matrix.Scale((FX_FLOAT)(max_dpi) / dpih, 1.0f);
    }
    if (dpiv > max_dpi) {
      m_Matrix.Scale(1.0f, (FX_FLOAT)(max_dpi) / (FX_FLOAT)dpiv);
    }
  }
#endif
  CFX_Matrix ctm = m_pDevice->GetCTM();
  FX_FLOAT fScaleX = FXSYS_fabs(ctm.a);
  FX_FLOAT fScaleY = FXSYS_fabs(ctm.d);
  m_Matrix.Concat(fScaleX, 0, 0, fScaleY, 0, 0);
  CFX_FloatRect rect(*pRect);
  m_Matrix.TransformRect(rect);
  FX_RECT bitmap_rect = rect.GetOuterRect();
  m_pBitmap.reset(new CFX_DIBitmap);
  m_pBitmap->Create(bitmap_rect.Width(), bitmap_rect.Height(), FXDIB_Argb);
  return true;
}
void CPDF_DeviceBuffer::OutputToDevice() {
  if (m_pDevice->GetDeviceCaps(FXDC_RENDER_CAPS) & FXRC_GET_BITS) {
    if (m_Matrix.a == 1.0f && m_Matrix.d == 1.0f) {
      m_pDevice->SetDIBits(m_pBitmap.get(), m_Rect.left, m_Rect.top);
    } else {
      m_pDevice->StretchDIBits(m_pBitmap.get(), m_Rect.left, m_Rect.top,
                               m_Rect.Width(), m_Rect.Height());
    }
  } else {
    CFX_DIBitmap buffer;
    m_pDevice->CreateCompatibleBitmap(&buffer, m_pBitmap->GetWidth(),
                                      m_pBitmap->GetHeight());
    m_pContext->GetBackground(&buffer, m_pObject, nullptr, &m_Matrix);
    buffer.CompositeBitmap(0, 0, buffer.GetWidth(), buffer.GetHeight(),
                           m_pBitmap.get(), 0, 0);
    m_pDevice->StretchDIBits(&buffer, m_Rect.left, m_Rect.top, m_Rect.Width(),
                             m_Rect.Height());
  }
}

CPDF_ScaledRenderBuffer::CPDF_ScaledRenderBuffer() {}

CPDF_ScaledRenderBuffer::~CPDF_ScaledRenderBuffer() {}

#define _FPDFAPI_IMAGESIZE_LIMIT_ (30 * 1024 * 1024)
bool CPDF_ScaledRenderBuffer::Initialize(CPDF_RenderContext* pContext,
                                         CFX_RenderDevice* pDevice,
                                         const FX_RECT& pRect,
                                         const CPDF_PageObject* pObj,
                                         const CPDF_RenderOptions* pOptions,
                                         int max_dpi) {
  m_pDevice = pDevice;
  if (m_pDevice->GetDeviceCaps(FXDC_RENDER_CAPS) & FXRC_GET_BITS) {
    return true;
  }
  m_pContext = pContext;
  m_Rect = pRect;
  m_pObject = pObj;
  m_Matrix.TranslateI(-pRect.left, -pRect.top);
  int horz_size = pDevice->GetDeviceCaps(FXDC_HORZ_SIZE);
  int vert_size = pDevice->GetDeviceCaps(FXDC_VERT_SIZE);
  if (horz_size && vert_size && max_dpi) {
    int dpih =
        pDevice->GetDeviceCaps(FXDC_PIXEL_WIDTH) * 254 / (horz_size * 10);
    int dpiv =
        pDevice->GetDeviceCaps(FXDC_PIXEL_HEIGHT) * 254 / (vert_size * 10);
    if (dpih > max_dpi) {
      m_Matrix.Scale((FX_FLOAT)(max_dpi) / dpih, 1.0f);
    }
    if (dpiv > max_dpi) {
      m_Matrix.Scale(1.0f, (FX_FLOAT)(max_dpi) / (FX_FLOAT)dpiv);
    }
  }
  m_pBitmapDevice.reset(new CFX_FxgeDevice);
  FXDIB_Format dibFormat = FXDIB_Rgb;
  int32_t bpp = 24;
  if (m_pDevice->GetDeviceCaps(FXDC_RENDER_CAPS) & FXRC_ALPHA_OUTPUT) {
    dibFormat = FXDIB_Argb;
    bpp = 32;
  }
  while (1) {
    CFX_FloatRect rect(pRect);
    m_Matrix.TransformRect(rect);
    FX_RECT bitmap_rect = rect.GetOuterRect();
    int32_t iWidth = bitmap_rect.Width();
    int32_t iHeight = bitmap_rect.Height();
    int32_t iPitch = (iWidth * bpp + 31) / 32 * 4;
    if (iWidth * iHeight < 1)
      return false;

    if (iPitch * iHeight <= _FPDFAPI_IMAGESIZE_LIMIT_ &&
        m_pBitmapDevice->Create(iWidth, iHeight, dibFormat, nullptr)) {
      break;
    }
    m_Matrix.Scale(0.5f, 0.5f);
  }
  m_pContext->GetBackground(m_pBitmapDevice->GetBitmap(), m_pObject, pOptions,
                            &m_Matrix);
  return true;
}
void CPDF_ScaledRenderBuffer::OutputToDevice() {
  if (m_pBitmapDevice) {
    m_pDevice->StretchDIBits(m_pBitmapDevice->GetBitmap(), m_Rect.left,
                             m_Rect.top, m_Rect.Width(), m_Rect.Height());
  }
}
