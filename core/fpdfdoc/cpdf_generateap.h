// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFDOC_CPDF_GENERATEAP_H_
#define CORE_FPDFDOC_CPDF_GENERATEAP_H_

#include "core/fpdfdoc/cpdf_annot.h"

class CPDF_Dictionary;
class CPDF_Document;
struct CFX_Color;

class CPDF_GenerateAP {
 public:
  enum FormType { kTextField, kComboBox, kListBox };

  static void GenerateFormAP(CPDF_Document* doc,
                             CPDF_Dictionary* pAnnotDict,
                             FormType type);

  static void GenerateEmptyAP(CPDF_Document* doc, CPDF_Dictionary* pAnnotDict);

  static bool GenerateAnnotAP(CPDF_Document* doc,
                              CPDF_Dictionary* pAnnotDict,
                              CPDF_Annot::Subtype subtype);

  static bool GenerateDefaultAppearanceWithColor(CPDF_Document* doc,
                                                 CPDF_Dictionary* annot_dict,
                                                 const CFX_Color& color);

  CPDF_GenerateAP() = delete;
  CPDF_GenerateAP(const CPDF_GenerateAP&) = delete;
  CPDF_GenerateAP& operator=(const CPDF_GenerateAP&) = delete;
};

#endif  // CORE_FPDFDOC_CPDF_GENERATEAP_H_
