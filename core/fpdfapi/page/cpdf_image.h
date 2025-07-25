// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PAGE_CPDF_IMAGE_H_
#define CORE_FPDFAPI_PAGE_CPDF_IMAGE_H_

#include <stdint.h>

#include "core/fpdfapi/page/cpdf_colorspace.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/span.h"
#include "core/fxcrt/unowned_ptr.h"

class CFX_DIBBase;
class CFX_DIBitmap;
class CPDF_DIB;
class CPDF_Dictionary;
class CPDF_Document;
class CPDF_Page;
class CPDF_Stream;
class PauseIndicatorIface;
class IFX_SeekableReadStream;

class CPDF_Image final : public Retainable {
 public:
  CONSTRUCT_VIA_MAKE_RETAIN;

  static bool IsValidJpegComponent(int32_t comps);
  static bool IsValidJpegBitsPerComponent(int32_t bpc);

  // Can only be called when `IsInline()` returns true.
  void ConvertStreamToIndirectObject();

  RetainPtr<const CPDF_Dictionary> GetDict() const;
  RetainPtr<const CPDF_Stream> GetStream() const;
  RetainPtr<const CPDF_Dictionary> GetOC() const;

  // Never returns nullptr.
  CPDF_Document* GetDocument() const { return document_; }

  int32_t GetPixelHeight() const { return height_; }
  int32_t GetPixelWidth() const { return width_; }
  uint32_t GetMatteColor() const { return matte_color_; }
  bool IsInline() const { return is_inline_; }
  bool IsMask() const { return is_mask_; }
  bool IsInterpol() const { return interpolate_; }

  RetainPtr<CPDF_DIB> CreateNewDIB() const;
  RetainPtr<CFX_DIBBase> LoadDIBBase() const;

  void SetImage(const RetainPtr<CFX_DIBitmap>& pBitmap);
  void SetJpegImage(RetainPtr<IFX_SeekableReadStream> pFile);
  void SetJpegImageInline(RetainPtr<IFX_SeekableReadStream> pFile);

  void ResetCache(CPDF_Page* pPage);

  void WillBeDestroyed();
  bool IsGoingToBeDestroyed() const { return will_be_destroyed_; }

  // Returns whether to Continue() or not.
  bool StartLoadDIBBase(const CPDF_Dictionary* pFormResource,
                        const CPDF_Dictionary* pPageResource,
                        bool bStdCS,
                        CPDF_ColorSpace::Family GroupFamily,
                        bool bLoadMask,
                        const CFX_Size& max_size_required);

  // Returns whether to Continue() or not.
  bool Continue(PauseIndicatorIface* pPause);

  RetainPtr<CFX_DIBBase> DetachBitmap();
  RetainPtr<CFX_DIBBase> DetachMask();

 private:
  explicit CPDF_Image(CPDF_Document* doc);
  CPDF_Image(CPDF_Document* doc, RetainPtr<CPDF_Stream> pStream);
  CPDF_Image(CPDF_Document* doc, uint32_t dwStreamObjNum);
  ~CPDF_Image() override;

  void FinishInitialization();
  RetainPtr<CPDF_Dictionary> InitJPEG(pdfium::span<uint8_t> src_span);
  RetainPtr<CPDF_Dictionary> CreateXObjectImageDict(int width, int height);

  int32_t height_ = 0;
  int32_t width_ = 0;
  uint32_t matte_color_ = 0;
  bool is_inline_ = false;
  bool is_mask_ = false;
  bool interpolate_ = false;
  bool will_be_destroyed_ = false;
  UnownedPtr<CPDF_Document> const document_;
  RetainPtr<CFX_DIBBase> dibbase_;
  RetainPtr<CFX_DIBBase> mask_;
  RetainPtr<CPDF_Stream> stream_;
  RetainPtr<const CPDF_Dictionary> oc_;
};

#endif  // CORE_FPDFAPI_PAGE_CPDF_IMAGE_H_
