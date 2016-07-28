// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFDOC_INCLUDE_CPDF_VIEWERPREFERENCES_H_
#define CORE_FPDFDOC_INCLUDE_CPDF_VIEWERPREFERENCES_H_

#include "core/fxcrt/include/fx_string.h"
#include "core/fxcrt/include/fx_system.h"

class CPDF_Array;
class CPDF_Dictionary;
class CPDF_Document;

class CPDF_ViewerPreferences {
 public:
  explicit CPDF_ViewerPreferences(CPDF_Document* pDoc);
  ~CPDF_ViewerPreferences();

  FX_BOOL IsDirectionR2L() const;
  FX_BOOL PrintScaling() const;
  int32_t NumCopies() const;
  CPDF_Array* PrintPageRange() const;
  CFX_ByteString Duplex() const;

 private:
  CPDF_Dictionary* GetViewerPreferences() const;

  CPDF_Document* const m_pDoc;
};

#endif  // CORE_FPDFDOC_INCLUDE_CPDF_VIEWERPREFERENCES_H_
