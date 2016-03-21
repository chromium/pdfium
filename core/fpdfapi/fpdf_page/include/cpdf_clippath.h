// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_FPDF_PAGE_INCLUDE_CPDF_CLIPPATH_H_
#define CORE_FPDFAPI_FPDF_PAGE_INCLUDE_CPDF_CLIPPATH_H_

#include "core/fpdfapi/fpdf_page/cpdf_clippathdata.h"
#include "core/fpdfapi/fpdf_page/include/cpdf_path.h"
#include "core/include/fxcrt/fx_basic.h"
#include "core/include/fxcrt/fx_coordinates.h"
#include "core/include/fxcrt/fx_system.h"

class CPDF_TextObject;

class CPDF_ClipPath : public CFX_CountRef<CPDF_ClipPathData> {
 public:
  FX_DWORD GetPathCount() const { return m_pObject->m_PathCount; }
  CPDF_Path GetPath(int i) const { return m_pObject->m_pPathList[i]; }
  int GetClipType(int i) const { return m_pObject->m_pTypeList[i]; }
  FX_DWORD GetTextCount() const { return m_pObject->m_TextCount; }
  CPDF_TextObject* GetText(int i) const { return m_pObject->m_pTextList[i]; }
  CFX_FloatRect GetClipBox() const;

  void AppendPath(CPDF_Path path, int type, FX_BOOL bAutoMerge);
  void DeletePath(int layer_index);

  void AppendTexts(CPDF_TextObject** pTexts, int count);
  void Transform(const CFX_Matrix& matrix);
};

#endif  // CORE_FPDFAPI_FPDF_PAGE_INCLUDE_CPDF_CLIPPATH_H_
