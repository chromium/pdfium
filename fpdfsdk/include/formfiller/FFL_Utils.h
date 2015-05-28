// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_FORMFILLER_FFL_UTILS_H_
#define FPDFSDK_INCLUDE_FORMFILLER_FFL_UTILS_H_

#include "../../../core/include/fpdfapi/fpdf_parser.h"
#include "../../../core/include/fxcrt/fx_memory.h"

#define FFL_BASE_USERUNIT			(1.0f / 72.0f)

class CFFL_Utils
{
public:
	static CPDF_Rect				MaxRect(const CPDF_Rect & rect1,const CPDF_Rect & rect2);
	static CPDF_Rect				InflateRect(const CPDF_Rect & crRect, const FX_FLOAT & fSize);
	static CPDF_Rect				DeflateRect(const CPDF_Rect & crRect, const FX_FLOAT & fSize);
	static FX_BOOL					TraceObject(CPDF_Object* pObj);
};

#endif  // FPDFSDK_INCLUDE_FORMFILLER_FFL_UTILS_H_
