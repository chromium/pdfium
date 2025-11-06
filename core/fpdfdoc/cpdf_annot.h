// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFDOC_CPDF_ANNOT_H_
#define CORE_FPDFDOC_CPDF_ANNOT_H_

#include <stddef.h>
#include <stdint.h>

#include <map>
#include <memory>
#include <optional>

#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fxcrt/bytestring.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/maybe_owned.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/unowned_ptr.h"

class CFX_RenderDevice;
class CPDF_Array;
class CPDF_Dictionary;
class CPDF_Document;
class CPDF_Form;
class CPDF_Page;
class CPDF_RenderContext;

class CPDF_Annot {
 public:
  enum class AppearanceMode { kNormal, kRollover, kDown };
  enum class Subtype : uint8_t {
    UNKNOWN = 0,
    TEXT,
    LINK,
    FREETEXT,
    LINE,
    SQUARE,
    CIRCLE,
    POLYGON,
    POLYLINE,
    HIGHLIGHT,
    UNDERLINE,
    SQUIGGLY,
    STRIKEOUT,
    STAMP,
    CARET,
    INK,
    POPUP,
    FILEATTACHMENT,
    SOUND,
    MOVIE,
    WIDGET,
    SCREEN,
    PRINTERMARK,
    TRAPNET,
    WATERMARK,
    THREED,
    RICHMEDIA,
    XFAWIDGET,
    REDACT
  };

  static Subtype StringToAnnotSubtype(const ByteString& sSubtype);
  static ByteString AnnotSubtypeToString(Subtype nSubtype);
  static CFX_FloatRect RectFromQuadPointsArray(const CPDF_Array* pArray,
                                               size_t nIndex);
  static CFX_FloatRect BoundingRectFromQuadPoints(
      const CPDF_Dictionary* pAnnotDict);
  static CFX_FloatRect RectFromQuadPoints(const CPDF_Dictionary* pAnnotDict,
                                          size_t nIndex);
  static size_t QuadPointCount(const CPDF_Array* pArray);

  CPDF_Annot(RetainPtr<CPDF_Dictionary> dict, CPDF_Document* document);
  ~CPDF_Annot();

  Subtype GetSubtype() const;
  uint32_t GetFlags() const;
  CFX_FloatRect GetRect() const;
  const CPDF_Dictionary* GetAnnotDict() const { return annot_dict_.Get(); }
  RetainPtr<CPDF_Dictionary> GetMutableAnnotDict() { return annot_dict_; }

  bool IsHidden() const;

  bool DrawAppearance(CPDF_Page* pPage,
                      CFX_RenderDevice* pDevice,
                      const CFX_Matrix& mtUser2Device,
                      AppearanceMode mode);
  bool DrawInContext(CPDF_Page* pPage,
                     CPDF_RenderContext* context,
                     const CFX_Matrix& mtUser2Device,
                     AppearanceMode mode);

  void ClearCachedAP();
  void DrawBorder(CFX_RenderDevice* pDevice, const CFX_Matrix* pUser2Device);
  CPDF_Form* GetAPForm(CPDF_Page* pPage, AppearanceMode mode);
  void SetOpenState(bool bOpenState) { open_state_ = bOpenState; }
  void SetPopupAnnotOpenState(bool bOpenState);
  std::optional<CFX_FloatRect> GetPopupAnnotRect() const;
  void SetPopupAnnot(CPDF_Annot* pAnnot) { popup_annot_ = pAnnot; }

 private:
  void GenerateAPIfNeeded();
  bool ShouldGenerateAP() const;
  bool ShouldDrawAnnotation() const;

  CFX_FloatRect RectForDrawing() const;

  RetainPtr<CPDF_Dictionary> const annot_dict_;
  UnownedPtr<CPDF_Document> const document_;
  std::map<RetainPtr<CPDF_Stream>, std::unique_ptr<CPDF_Form>> ap_map_;
  // If non-null, then this is not a popup annotation.
  UnownedPtr<CPDF_Annot> popup_annot_;
  const Subtype subtype_;
  const bool is_text_markup_annotation_;
  // |open_state_| is only set for popup annotations.
  bool open_state_ = false;
  bool has_generated_ap_;
};

// Get the AP in an annotation dict for a given appearance mode.
// If |eMode| is not Normal and there is not AP for that mode, falls back to
// the Normal AP.
RetainPtr<CPDF_Stream> GetAnnotAP(CPDF_Dictionary* pAnnotDict,
                                  CPDF_Annot::AppearanceMode eMode);

// Get the AP in an annotation dict for a given appearance mode.
// No fallbacks to Normal like in GetAnnotAP.
RetainPtr<CPDF_Stream> GetAnnotAPNoFallback(CPDF_Dictionary* pAnnotDict,
                                            CPDF_Annot::AppearanceMode eMode);

#endif  // CORE_FPDFDOC_CPDF_ANNOT_H_
