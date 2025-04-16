// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_RENDER_CPDF_RENDERSTATUS_H_
#define CORE_FPDFAPI_RENDER_CPDF_RENDERSTATUS_H_

#include <memory>
#include <utility>
#include <vector>

#include "build/build_config.h"
#include "core/fpdfapi/page/cpdf_clippath.h"
#include "core/fpdfapi/page/cpdf_colorspace.h"
#include "core/fpdfapi/page/cpdf_graphicstates.h"
#include "core/fpdfapi/page/cpdf_transparency.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/render/cpdf_renderoptions.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/unowned_ptr.h"
#include "core/fxge/dib/fx_dib.h"

class CFX_DIBitmap;
class CFX_Path;
class CFX_RenderDevice;
class CPDF_Color;
class CPDF_Font;
class CPDF_FormObject;
class CPDF_ImageObject;
class CPDF_ImageRenderer;
class CPDF_Object;
class CPDF_PageObject;
class CPDF_PageObjectHolder;
class CPDF_PathObject;
class CPDF_RenderContext;
class CPDF_ShadingObject;
class CPDF_ShadingPattern;
class CPDF_TilingPattern;
class CPDF_TransferFunc;
class CPDF_Type3Char;
class CPDF_Type3Font;
class PauseIndicatorIface;

class CPDF_RenderStatus {
 public:
  CPDF_RenderStatus(CPDF_RenderContext* pContext, CFX_RenderDevice* pDevice);
  ~CPDF_RenderStatus();

  // Called prior to Initialize().
  void SetOptions(const CPDF_RenderOptions& options) { options_ = options; }
  void SetDeviceMatrix(const CFX_Matrix& matrix) { device_matrix_ = matrix; }
  void SetStopObject(const CPDF_PageObject* pStopObj) { stop_obj_ = pStopObj; }
  void SetFormResource(RetainPtr<const CPDF_Dictionary> pRes) {
    form_resource_ = std::move(pRes);
  }
  void SetType3Char(CPDF_Type3Char* pType3Char) { type3_char_ = pType3Char; }
  void SetFillColor(FX_ARGB color) { t3_fill_color_ = color; }
  void SetDropObjects(bool bDropObjects) { drop_objects_ = bDropObjects; }
  void SetLoadMask(bool bLoadMask) { load_mask_ = bLoadMask; }
  void SetStdCS(bool bStdCS) { std_cs_ = bStdCS; }
  void SetGroupFamily(CPDF_ColorSpace::Family family) {
    group_family_ = family;
  }
  void SetTransparency(const CPDF_Transparency& transparency) {
    transparency_ = transparency;
  }
  void SetInGroup(bool bInGroup) { in_group_ = bInGroup; }

  void Initialize(const CPDF_RenderStatus* pParentStatus,
                  const CPDF_GraphicStates* pInitialStates);

  void RenderObjectList(const CPDF_PageObjectHolder* pObjectHolder,
                        const CFX_Matrix& mtObj2Device);
  void RenderSingleObject(CPDF_PageObject* pObj,
                          const CFX_Matrix& mtObj2Device);
  bool ContinueSingleObject(CPDF_PageObject* pObj,
                            const CFX_Matrix& mtObj2Device,
                            PauseIndicatorIface* pPause);
  void ProcessClipPath(const CPDF_ClipPath& ClipPath,
                       const CFX_Matrix& mtObj2Device);

  CPDF_ColorSpace::Family GetGroupFamily() const { return group_family_; }
  bool GetLoadMask() const { return load_mask_; }
  bool GetDropObjects() const { return drop_objects_; }
  bool IsPrint() const {
#if BUILDFLAG(IS_WIN)
    return print_;
#else
    return false;
#endif
  }
  bool IsStopped() const { return stopped_; }
  CPDF_RenderContext* GetContext() const { return context_; }
  const CPDF_Dictionary* GetFormResource() const {
    return form_resource_.Get();
  }
  const CPDF_Dictionary* GetPageResource() const {
    return page_resource_.Get();
  }
  CFX_RenderDevice* GetRenderDevice() const { return device_; }
  const CPDF_RenderOptions& GetRenderOptions() const { return options_; }

  RetainPtr<CPDF_TransferFunc> GetTransferFunc(
      RetainPtr<const CPDF_Object> pObject) const;

  FX_ARGB GetFillArgb(CPDF_PageObject* pObj) const;
  FX_ARGB GetFillArgbForType3(CPDF_PageObject* pObj) const;

  void DrawTilingPattern(CPDF_TilingPattern* pattern,
                         CPDF_PageObject* pPageObj,
                         const CFX_Matrix& mtObj2Device,
                         bool stroke);
  void DrawShadingPattern(CPDF_ShadingPattern* pattern,
                          const CPDF_PageObject* pPageObj,
                          const CFX_Matrix& mtObj2Device,
                          bool stroke);
  // `pDIBitmap` must be non-null.
  void CompositeDIBitmap(RetainPtr<CFX_DIBitmap> bitmap,
                         int left,
                         int top,
                         FX_ARGB mask_argb,
                         float alpha,
                         BlendMode blend_mode,
                         const CPDF_Transparency& transparency);

  static std::unique_ptr<CPDF_GraphicStates> CloneObjStates(
      const CPDF_GraphicStates* pSrcStates,
      bool stroke);

 private:
  bool ProcessTransparency(CPDF_PageObject* PageObj,
                           const CFX_Matrix& mtObj2Device);
  void ProcessObjectNoClip(CPDF_PageObject* pObj,
                           const CFX_Matrix& mtObj2Device);
  void DrawObjWithBackground(CPDF_PageObject* pObj,
                             const CFX_Matrix& mtObj2Device);
  void DrawObjWithBackgroundToDevice(CPDF_PageObject* obj,
                                     const CFX_Matrix& object_to_device,
                                     CFX_RenderDevice* device,
                                     const CFX_Matrix& device_matrix);
  bool DrawObjWithBlend(CPDF_PageObject* pObj, const CFX_Matrix& mtObj2Device);
  bool ProcessPath(CPDF_PathObject* path_obj, const CFX_Matrix& mtObj2Device);
  void ProcessPathPattern(CPDF_PathObject* path_obj,
                          const CFX_Matrix& mtObj2Device,
                          CFX_FillRenderOptions::FillType* fill_type,
                          bool* stroke);
  void DrawPathWithPattern(CPDF_PathObject* path_obj,
                           const CFX_Matrix& mtObj2Device,
                           const CPDF_Color* pColor,
                           bool stroke);
  bool ClipPattern(const CPDF_PageObject* page_obj,
                   const CFX_Matrix& mtObj2Device,
                   bool stroke);
  bool SelectClipPath(const CPDF_PathObject* path_obj,
                      const CFX_Matrix& mtObj2Device,
                      bool stroke);
  bool ProcessImage(CPDF_ImageObject* pImageObj,
                    const CFX_Matrix& mtObj2Device);
  void ProcessShading(const CPDF_ShadingObject* pShadingObj,
                      const CFX_Matrix& mtObj2Device);
  bool ProcessType3Text(CPDF_TextObject* textobj,
                        const CFX_Matrix& mtObj2Device);
  bool ProcessText(CPDF_TextObject* textobj,
                   const CFX_Matrix& mtObj2Device,
                   CFX_Path* clipping_path);
  void DrawTextPathWithPattern(const CPDF_TextObject* textobj,
                               const CFX_Matrix& mtObj2Device,
                               CPDF_Font* pFont,
                               float font_size,
                               const CFX_Matrix& mtTextMatrix,
                               bool fill,
                               bool stroke);
  bool ProcessForm(const CPDF_FormObject* pFormObj,
                   const CFX_Matrix& mtObj2Device);
  FX_RECT GetClippedBBox(const FX_RECT& rect) const;
  RetainPtr<CFX_DIBitmap> GetBackdrop(const CPDF_PageObject* pObj,
                                      const FX_RECT& bbox,
                                      bool bBackAlphaRequired);
  RetainPtr<CFX_DIBitmap> LoadSMask(CPDF_Dictionary* smask_dict,
                                    const FX_RECT& clip_rect,
                                    const CFX_Matrix& smask_matrix);
  // Optionally write the colorspace family value into |pCSFamily|.
  FX_ARGB GetBackgroundColor(const CPDF_Dictionary* pSMaskDict,
                             const CPDF_Dictionary* pGroupDict,
                             CPDF_ColorSpace::Family* pCSFamily);
  FX_ARGB GetStrokeArgb(CPDF_PageObject* pObj) const;
  FX_RECT GetObjectClippedRect(const CPDF_PageObject* pObj,
                               const CFX_Matrix& mtObj2Device) const;
  // Returns the format that is compatible with `device_`.
  FXDIB_Format GetCompatibleArgbFormat() const;

  CPDF_RenderOptions options_;
  RetainPtr<const CPDF_Dictionary> form_resource_;
  RetainPtr<const CPDF_Dictionary> page_resource_;
  std::vector<UnownedPtr<const CPDF_Type3Font>> type3_font_cache_;
  UnownedPtr<CPDF_RenderContext> const context_;
  UnownedPtr<CFX_RenderDevice> const device_;
  CFX_Matrix device_matrix_;
  CPDF_ClipPath last_clip_path_;
  UnownedPtr<const CPDF_PageObject> cur_obj_;
  UnownedPtr<const CPDF_PageObject> stop_obj_;
  CPDF_GraphicStates initial_states_;
  std::unique_ptr<CPDF_ImageRenderer> image_renderer_;
  UnownedPtr<const CPDF_Type3Char> type3_char_;
  CPDF_Transparency transparency_;
  bool stopped_ = false;
#if BUILDFLAG(IS_WIN)
  bool print_ = false;
#endif
  bool drop_objects_ = false;
  bool std_cs_ = false;
  bool load_mask_ = false;
  bool in_group_ = false;
  CPDF_ColorSpace::Family group_family_ = CPDF_ColorSpace::Family::kUnknown;
  FX_ARGB t3_fill_color_ = 0;
};

#endif  // CORE_FPDFAPI_RENDER_CPDF_RENDERSTATUS_H_
