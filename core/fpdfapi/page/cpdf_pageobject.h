// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PAGE_CPDF_PAGEOBJECT_H_
#define CORE_FPDFAPI_PAGE_CPDF_PAGEOBJECT_H_

#include <stdint.h>

#include "core/fpdfapi/page/cpdf_contentmarks.h"
#include "core/fpdfapi/page/cpdf_graphicstates.h"
#include "core/fxcrt/bytestring.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/span.h"

class CPDF_FormObject;
class CPDF_ImageObject;
class CPDF_PathObject;
class CPDF_ShadingObject;
class CPDF_TextObject;

// Represents an object within the page, like a form or image. Not to be
// confused with the PDF spec's page object that lives in a page tree, which is
// represented by CPDF_Page.
class CPDF_PageObject {
 public:
  // Values must match corresponding values in //public.
  enum class Type {
    kText = 1,
    kPath,
    kImage,
    kShading,
    kForm,
  };

  static constexpr int32_t kNoContentStream = -1;

  explicit CPDF_PageObject(int32_t content_stream);
  CPDF_PageObject(const CPDF_PageObject& src) = delete;
  CPDF_PageObject& operator=(const CPDF_PageObject& src) = delete;
  virtual ~CPDF_PageObject();

  virtual Type GetType() const = 0;
  virtual void Transform(const CFX_Matrix& matrix) = 0;
  virtual bool IsText() const;
  virtual bool IsPath() const;
  virtual bool IsImage() const;
  virtual bool IsShading() const;
  virtual bool IsForm() const;
  virtual CPDF_TextObject* AsText();
  virtual const CPDF_TextObject* AsText() const;
  virtual CPDF_PathObject* AsPath();
  virtual const CPDF_PathObject* AsPath() const;
  virtual CPDF_ImageObject* AsImage();
  virtual const CPDF_ImageObject* AsImage() const;
  virtual CPDF_ShadingObject* AsShading();
  virtual const CPDF_ShadingObject* AsShading() const;
  virtual CPDF_FormObject* AsForm();
  virtual const CPDF_FormObject* AsForm() const;

  void SetDirty(bool value) { dirty_ = value; }
  bool IsDirty() const { return dirty_ || matrix_dirty_; }
  void SetMatrixDirty(bool value) { matrix_dirty_ = value; }
  void SetIsActive(bool value);
  bool IsActive() const { return is_active_; }
  void TransformClipPath(const CFX_Matrix& matrix);

  void SetOriginalRect(const CFX_FloatRect& rect) { original_rect_ = rect; }
  const CFX_FloatRect& GetOriginalRect() const { return original_rect_; }
  void SetRect(const CFX_FloatRect& rect) { rect_ = rect; }
  const CFX_FloatRect& GetRect() const { return rect_; }
  FX_RECT GetBBox() const;
  FX_RECT GetTransformedBBox(const CFX_Matrix& matrix) const;

  CPDF_ContentMarks* GetContentMarks() { return &content_marks_; }
  const CPDF_ContentMarks* GetContentMarks() const { return &content_marks_; }
  void SetContentMarks(const CPDF_ContentMarks& marks) {
    content_marks_ = marks;
  }

  // Get what content stream the object was parsed from in its page. This number
  // is the index of the content stream in the "Contents" array, or 0 if there
  // is a single content stream. If the object is newly created,
  // |kNoContentStream| is returned.
  //
  // If the object is spread among more than one content stream, this is the
  // index of the last stream.
  int32_t GetContentStream() const { return content_stream_; }
  void SetContentStream(int32_t new_content_stream) {
    content_stream_ = new_content_stream;
  }

  const ByteString& GetResourceName() const { return resource_name_; }
  void SetResourceName(const ByteString& resource_name) {
    resource_name_ = resource_name;
  }

  pdfium::span<const ByteString> GetGraphicsResourceNames() const;

  const CPDF_ClipPath& clip_path() const { return graphic_states_.clip_path(); }
  CPDF_ClipPath& mutable_clip_path() {
    return graphic_states_.mutable_clip_path();
  }

  const CFX_GraphState& graph_state() const {
    return graphic_states_.graph_state();
  }
  CFX_GraphState& mutable_graph_state() {
    return graphic_states_.mutable_graph_state();
  }

  const CPDF_ColorState& color_state() const {
    return graphic_states_.color_state();
  }
  CPDF_ColorState& mutable_color_state() {
    return graphic_states_.mutable_color_state();
  }

  const CPDF_TextState& text_state() const {
    return graphic_states_.text_state();
  }
  CPDF_TextState& mutable_text_state() {
    return graphic_states_.mutable_text_state();
  }

  const CPDF_GeneralState& general_state() const {
    return graphic_states_.general_state();
  }
  CPDF_GeneralState& mutable_general_state() {
    return graphic_states_.mutable_general_state();
  }

  const CPDF_GraphicStates& graphic_states() const { return graphic_states_; }

  void SetDefaultStates();

  const CFX_Matrix& original_matrix() const { return original_matrix_; }

 protected:
  void CopyData(const CPDF_PageObject* pSrcObject);
  void InitializeOriginalMatrix(const CFX_Matrix& matrix);

 private:
  CPDF_GraphicStates graphic_states_;
  CFX_FloatRect rect_;
  CFX_FloatRect original_rect_;
  // Only used with `CPDF_ImageObject` for now.
  // TODO(thestig): Use with `CPDF_FormObject` and `CPDF_PageObject` as well.
  CFX_Matrix original_matrix_;
  CPDF_ContentMarks content_marks_;
  // Modifying `is_active_` automatically set `dirty_` to be true, but
  // otherwise `dirty_` and `is_active_` are independent.  A
  // `CPDF_PageObject` can remain dirty until page object processing completes
  // and marks it no longer dirty.
  bool dirty_ = false;
  // Separately track if the current matrix is different from
  // `original_matrix_`.
  bool matrix_dirty_ = false;
  bool is_active_ = true;
  int32_t content_stream_;
  // The resource name for this object.
  ByteString resource_name_;
};

#endif  // CORE_FPDFAPI_PAGE_CPDF_PAGEOBJECT_H_
