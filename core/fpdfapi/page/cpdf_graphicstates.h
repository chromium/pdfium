// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PAGE_CPDF_GRAPHICSTATES_H_
#define CORE_FPDFAPI_PAGE_CPDF_GRAPHICSTATES_H_

#include "core/fpdfapi/page/cpdf_clippath.h"
#include "core/fpdfapi/page/cpdf_colorstate.h"
#include "core/fpdfapi/page/cpdf_generalstate.h"
#include "core/fpdfapi/page/cpdf_textstate.h"
#include "core/fxge/cfx_graphstate.h"

class CPDF_GraphicStates {
 public:
  CPDF_GraphicStates();
  CPDF_GraphicStates(const CPDF_GraphicStates& that);
  CPDF_GraphicStates& operator=(const CPDF_GraphicStates& that);
  ~CPDF_GraphicStates();

  void SetDefaultStates();

  const CPDF_ClipPath& clip_path() const { return clip_path_; }
  CPDF_ClipPath& mutable_clip_path() { return clip_path_; }

  const CFX_GraphState& graph_state() const { return graph_state_; }
  CFX_GraphState& mutable_graph_state() { return graph_state_; }

  const CPDF_ColorState& color_state() const { return color_state_; }
  CPDF_ColorState& mutable_color_state() { return color_state_; }

  const CPDF_TextState& text_state() const { return text_state_; }
  CPDF_TextState& mutable_text_state() { return text_state_; }

  const CPDF_GeneralState& general_state() const { return general_state_; }
  CPDF_GeneralState& mutable_general_state() { return general_state_; }

 private:
  CPDF_ClipPath clip_path_;
  CFX_GraphState graph_state_;
  CPDF_ColorState color_state_;
  CPDF_TextState text_state_;
  CPDF_GeneralState general_state_;
};

#endif  // CORE_FPDFAPI_PAGE_CPDF_GRAPHICSTATES_H_
