// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/cpdf_page.h"

#include <set>
#include <utility>

#include "constants/page_object.h"
#include "core/fpdfapi/page/cpdf_contentparser.h"
#include "core/fpdfapi/page/cpdf_pageimagecache.h"
#include "core/fpdfapi/page/cpdf_pageobject.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_object.h"
#include "core/fxcrt/check.h"
#include "core/fxcrt/check_op.h"
#include "core/fxcrt/containers/contains.h"

CPDF_Page::CPDF_Page(CPDF_Document* document,
                     RetainPtr<CPDF_Dictionary> pPageDict)
    : CPDF_PageObjectHolder(document, std::move(pPageDict), nullptr, nullptr),
      page_size_(100, 100),
      pdf_document_(document) {
  // Cannot initialize |resources_| and |page_resources_| via the
  // CPDF_PageObjectHolder ctor because GetPageAttr() requires
  // CPDF_PageObjectHolder to finish initializing first.
  RetainPtr<CPDF_Object> pPageAttr =
      GetMutablePageAttr(pdfium::page_object::kResources);
  resources_ = pPageAttr ? pPageAttr->GetMutableDict() : nullptr;
  page_resources_ = resources_;

  UpdateDimensions();
  transparency_.SetIsolated();
  LoadTransparencyInfo();
}

CPDF_Page::~CPDF_Page() = default;

CPDF_Page* CPDF_Page::AsPDFPage() {
  return this;
}

CPDFXFA_Page* CPDF_Page::AsXFAPage() {
  return nullptr;
}

CPDF_Document* CPDF_Page::GetDocument() const {
  return pdf_document_;
}

float CPDF_Page::GetPageWidth() const {
  return page_size_.width;
}

float CPDF_Page::GetPageHeight() const {
  return page_size_.height;
}

bool CPDF_Page::IsPage() const {
  return true;
}

void CPDF_Page::ParseContent() {
  if (GetParseState() == ParseState::kParsed) {
    return;
  }

  if (GetParseState() == ParseState::kNotParsed) {
    StartParse(std::make_unique<CPDF_ContentParser>(this));
  }

  DCHECK_EQ(GetParseState(), ParseState::kParsing);
  ContinueParse(nullptr);
}

RetainPtr<CPDF_Object> CPDF_Page::GetMutablePageAttr(ByteStringView name) {
  return pdfium::WrapRetain(const_cast<CPDF_Object*>(GetPageAttr(name).Get()));
}

RetainPtr<const CPDF_Object> CPDF_Page::GetPageAttr(ByteStringView name) const {
  std::set<RetainPtr<const CPDF_Dictionary>> visited;
  RetainPtr<const CPDF_Dictionary> pPageDict = GetDict();
  while (pPageDict && !pdfium::Contains(visited, pPageDict)) {
    RetainPtr<const CPDF_Object> pObj = pPageDict->GetDirectObjectFor(name);
    if (pObj) {
      return pObj;
    }

    visited.insert(pPageDict);
    pPageDict = pPageDict->GetDictFor(pdfium::page_object::kParent);
  }
  return nullptr;
}

CFX_FloatRect CPDF_Page::GetBox(ByteStringView name) const {
  CFX_FloatRect box;
  RetainPtr<const CPDF_Array> pBox = ToArray(GetPageAttr(name));
  if (pBox) {
    box = pBox->GetRect();
    box.Normalize();
  }
  return box;
}

std::optional<CFX_PointF> CPDF_Page::DeviceToPage(
    const FX_RECT& rect,
    int rotation,
    const CFX_PointF& device_point) const {
  CFX_Matrix page2device = GetDisplayMatrixForRect(rect, rotation);
  return page2device.GetInverse().Transform(device_point);
}

std::optional<CFX_PointF> CPDF_Page::PageToDevice(
    const FX_RECT& rect,
    int rotation,
    const CFX_PointF& page_point) const {
  CFX_Matrix page2device = GetDisplayMatrixForRect(rect, rotation);
  return page2device.Transform(page_point);
}

CFX_Matrix CPDF_Page::GetDisplayMatrixForRect(const FX_RECT& rect,
                                              int rotation) const {
  return GetDisplayMatrixForFloatRect(CFX_FloatRect(rect), rotation);
}

CFX_Matrix CPDF_Page::GetDisplayMatrixForFloatRect(const CFX_FloatRect& rect,
                                                   int rotation) const {
  if (page_size_.width == 0 || page_size_.height == 0) {
    return CFX_Matrix();
  }

  float x0;
  float y0;
  float x1;
  float y1;
  float x2;
  float y2;
  // This code implicitly inverts the y-axis to account for page coordinates
  // pointing up and bitmap coordinates pointing down. (x0, y0) is the base
  // point, (x1, y1) is that point translated on y and (x2, y2) is the point
  // translated on x. On rotation = 0, y0 is rect.top and the translation to get
  // y1 is performed as negative. This results in the desired transformation.
  switch (rotation % 4) {
    case 0:
      x0 = rect.left;
      y0 = rect.top;
      x1 = rect.left;
      y1 = rect.bottom;
      x2 = rect.right;
      y2 = rect.top;
      break;
    case 1:
      x0 = rect.left;
      y0 = rect.bottom;
      x1 = rect.right;
      y1 = rect.bottom;
      x2 = rect.left;
      y2 = rect.top;
      break;
    case 2:
      x0 = rect.right;
      y0 = rect.bottom;
      x1 = rect.right;
      y1 = rect.top;
      x2 = rect.left;
      y2 = rect.bottom;
      break;
    case 3:
      x0 = rect.right;
      y0 = rect.top;
      x1 = rect.left;
      y1 = rect.top;
      x2 = rect.right;
      y2 = rect.bottom;
      break;
    default:
      CHECK_LT(rotation, 0);
      // Handing this with `rotation += 4` breaks public API compatibility. So
      // just return early here without doing all the matrix calculations below.
      return CFX_Matrix(0, 0, 0, 0, 0, 0);
  }
  CFX_Matrix matrix((x2 - x0) / page_size_.width, (y2 - y0) / page_size_.width,
                    (x1 - x0) / page_size_.height,
                    (y1 - y0) / page_size_.height, x0, y0);
  return page_matrix_ * matrix;
}

CFX_Matrix CPDF_Page::GetDisplayMatrix() const {
  const CFX_FloatRect rect(0, 0, GetPageWidth(), GetPageHeight());
  return GetDisplayMatrixForFloatRect(rect, 0);
}

int CPDF_Page::GetPageRotation() const {
  RetainPtr<const CPDF_Object> pRotate =
      GetPageAttr(pdfium::page_object::kRotate);
  int rotation = pRotate ? (pRotate->GetInteger() / 90) % 4 : 0;
  return (rotation < 0) ? (rotation + 4) : rotation;
}

RetainPtr<CPDF_Array> CPDF_Page::GetOrCreateAnnotsArray() {
  return GetMutableDict()->GetOrCreateArrayFor("Annots");
}

RetainPtr<CPDF_Array> CPDF_Page::GetMutableAnnotsArray() {
  return GetMutableDict()->GetMutableArrayFor("Annots");
}

RetainPtr<const CPDF_Array> CPDF_Page::GetAnnotsArray() const {
  return GetDict()->GetArrayFor("Annots");
}

void CPDF_Page::AddPageImageCache() {
  page_image_cache_ = std::make_unique<CPDF_PageImageCache>(this);
}

void CPDF_Page::SetRenderContext(std::unique_ptr<RenderContextIface> context) {
  DCHECK(!render_context_);
  DCHECK(context);
  render_context_ = std::move(context);
}

void CPDF_Page::ClearRenderContext() {
  render_context_.reset();
}

void CPDF_Page::ClearView() {
  if (view_) {
    view_->ClearPage(this);
  }
}

void CPDF_Page::UpdateDimensions() {
  CFX_FloatRect mediabox = GetBox(pdfium::page_object::kMediaBox);
  if (mediabox.IsEmpty()) {
    mediabox = CFX_FloatRect(0, 0, 612, 792);
  }

  bbox_ = GetBox(pdfium::page_object::kCropBox);
  if (bbox_.IsEmpty()) {
    bbox_ = mediabox;
  } else {
    bbox_.Intersect(mediabox);
  }

  page_size_.width = bbox_.Width();
  page_size_.height = bbox_.Height();

  switch (GetPageRotation()) {
    case 0:
      page_matrix_ = CFX_Matrix(1.0f, 0, 0, 1.0f, -bbox_.left, -bbox_.bottom);
      break;
    case 1:
      std::swap(page_size_.width, page_size_.height);
      page_matrix_ = CFX_Matrix(0, -1.0f, 1.0f, 0, -bbox_.bottom, bbox_.right);
      break;
    case 2:
      page_matrix_ = CFX_Matrix(-1.0f, 0, 0, -1.0f, bbox_.right, bbox_.top);
      break;
    case 3:
      std::swap(page_size_.width, page_size_.height);
      page_matrix_ = CFX_Matrix(0, 1.0f, -1.0f, 0, bbox_.top, -bbox_.left);
      break;
  }
}

CPDF_Page::RenderContextClearer::RenderContextClearer(CPDF_Page* pPage)
    : page_(pPage) {}

CPDF_Page::RenderContextClearer::~RenderContextClearer() {
  if (page_) {
    page_->ClearRenderContext();
  }
}
