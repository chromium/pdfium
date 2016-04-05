// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFDOC_CPVT_GENERATEAP_H_
#define CORE_FPDFDOC_CPVT_GENERATEAP_H_

#include "core/fpdfdoc/cpvt_color.h"
#include "core/fpdfdoc/cpvt_dash.h"
#include "core/fpdfdoc/include/cpdf_variabletext.h"
#include "core/fxcrt/include/fx_coordinates.h"
#include "core/fxcrt/include/fx_string.h"
#include "core/fxcrt/include/fx_system.h"

// border style
#define PBS_SOLID 0
#define PBS_DASH 1
#define PBS_BEVELED 2
#define PBS_INSET 3
#define PBS_UNDERLINED 4
#define PBS_SHADOW 5

class CPDF_Dictionary;
class CPDF_Document;
class IPVT_FontMap;

struct CPVT_WordRange;

FX_BOOL FPDF_GenerateAP(CPDF_Document* pDoc, CPDF_Dictionary* pAnnotDict);

class CPVT_GenerateAP {
 public:
  static FX_BOOL GenerateTextFieldAP(CPDF_Document* pDoc,
                                     CPDF_Dictionary* pAnnotDict);
  static FX_BOOL GenerateComboBoxAP(CPDF_Document* pDoc,
                                    CPDF_Dictionary* pAnnotDict);
  static FX_BOOL GenerateListBoxAP(CPDF_Document* pDoc,
                                   CPDF_Dictionary* pAnnotDict);
  static CFX_ByteString GenerateEditAP(IPVT_FontMap* pFontMap,
                                       CPDF_VariableText::Iterator* pIterator,
                                       const CFX_FloatPoint& ptOffset,
                                       FX_BOOL bContinuous,
                                       uint16_t SubWord = 0,
                                       const CPVT_WordRange* pVisible = NULL);
  static CFX_ByteString GenerateBorderAP(const CFX_FloatRect& rect,
                                         FX_FLOAT fWidth,
                                         const CPVT_Color& color,
                                         const CPVT_Color& crLeftTop,
                                         const CPVT_Color& crRightBottom,
                                         int32_t nStyle,
                                         const CPVT_Dash& dash);
  static CFX_ByteString GenerateColorAP(const CPVT_Color& color,
                                        const FX_BOOL& bFillOrStroke);

  static CFX_ByteString GetPDFWordString(IPVT_FontMap* pFontMap,
                                         int32_t nFontIndex,
                                         uint16_t Word,
                                         uint16_t SubWord);
  static CFX_ByteString GetWordRenderString(const CFX_ByteString& strWords);
  static CFX_ByteString GetFontSetString(IPVT_FontMap* pFontMap,
                                         int32_t nFontIndex,
                                         FX_FLOAT fFontSize);
};

#endif  // CORE_FPDFDOC_CPVT_GENERATEAP_H_
