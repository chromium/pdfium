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

class CPDF_AllStates final : public CPDF_GraphicStates {
 public:
  CPDF_AllStates();
  ~CPDF_AllStates() override;

  void Copy(const CPDF_AllStates& src);
  void ProcessExtGS(const CPDF_Dictionary* pGS,
                    CPDF_StreamContentParser* pParser);
  void SetLineDash(const CPDF_Array* pArray, float phase, float scale);

  CFX_PointF GetTransformedTextPosition() const;
  void ResetTextPosition();
  void MoveTextPoint(const CFX_PointF& point);
  void MoveTextToNextLine();
  void IncrementTextPositionX(float value);
  void IncrementTextPositionY(float value);

  const CFX_Matrix& text_matrix() const { return m_TextMatrix; }
  void set_text_matrix(const CFX_Matrix& matrix) { m_TextMatrix = matrix; }

  const CFX_Matrix& current_transformation_matrix() const { return m_CTM; }
  void set_current_transformation_matrix(const CFX_Matrix& matrix) {
    m_CTM = matrix;
  }
  void prepend_to_current_transformation_matrix(const CFX_Matrix& matrix) {
    m_CTM = matrix * m_CTM;
  }

  const CFX_Matrix& parent_matrix() const { return m_ParentMatrix; }
  void set_parent_matrix(const CFX_Matrix& matrix) { m_ParentMatrix = matrix; }

  void set_text_leading(float value) { m_TextLeading = value; }

  void set_text_rise(float value) { m_TextRise = value; }

  float text_horz_scale() const { return m_TextHorzScale; }
  void set_text_horz_scale(float value) { m_TextHorzScale = value; }

 private:
  CFX_Matrix m_TextMatrix;
  CFX_Matrix m_CTM;
  CFX_Matrix m_ParentMatrix;
  CFX_PointF m_TextPos;
  CFX_PointF m_TextLinePos;
  float m_TextLeading = 0.0f;
  float m_TextRise = 0.0f;
  float m_TextHorzScale = 1.0f;
};

#endif  // CORE_FPDFAPI_PAGE_CPDF_ALLSTATES_H_
