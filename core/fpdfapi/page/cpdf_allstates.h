// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PAGE_CPDF_ALLSTATES_H_
#define CORE_FPDFAPI_PAGE_CPDF_ALLSTATES_H_

#include "core/fpdfapi/page/cpdf_graphicstates.h"
#include "core/fxcrt/fx_coordinates.h"

class CPDF_Array;
class CPDF_Dictionary;
class CPDF_StreamContentParser;

class CPDF_AllStates {
 public:
  CPDF_AllStates();
  CPDF_AllStates(const CPDF_AllStates& that);
  CPDF_AllStates& operator=(const CPDF_AllStates& that);
  ~CPDF_AllStates();

  void SetDefaultStates();

  void ProcessExtGS(const CPDF_Dictionary* pGS,
                    CPDF_StreamContentParser* pParser);
  void SetLineDash(const CPDF_Array* pArray, float phase);

  CFX_PointF GetTransformedTextPosition() const;
  void ResetTextPosition();
  void MoveTextPoint(const CFX_PointF& point);
  void MoveTextToNextLine();
  void IncrementTextPositionX(float value);
  void IncrementTextPositionY(float value);

  const CFX_Matrix& text_matrix() const { return text_matrix_; }
  void set_text_matrix(const CFX_Matrix& matrix) { text_matrix_ = matrix; }

  const CFX_Matrix& current_transformation_matrix() const { return ctm_; }
  void set_current_transformation_matrix(const CFX_Matrix& matrix) {
    ctm_ = matrix;
  }
  void prepend_to_current_transformation_matrix(const CFX_Matrix& matrix) {
    ctm_ = matrix * ctm_;
  }

  const CFX_Matrix& parent_matrix() const { return parent_matrix_; }
  void set_parent_matrix(const CFX_Matrix& matrix) { parent_matrix_ = matrix; }

  void set_text_leading(float value) { text_leading_ = value; }

  void set_text_rise(float value) { text_rise_ = value; }

  float text_horz_scale() const { return text_horz_scale_; }
  void set_text_horz_scale(float value) { text_horz_scale_ = value; }

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

 private:
  CPDF_GraphicStates graphic_states_;
  CFX_Matrix text_matrix_;
  CFX_Matrix ctm_;
  CFX_Matrix parent_matrix_;
  CFX_PointF text_pos_;
  CFX_PointF text_line_pos_;
  float text_leading_ = 0.0f;
  float text_rise_ = 0.0f;
  float text_horz_scale_ = 1.0f;
};

#endif  // CORE_FPDFAPI_PAGE_CPDF_ALLSTATES_H_
