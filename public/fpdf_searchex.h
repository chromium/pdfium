// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FPDF_SEARCH_EX_H
#define _FPDF_SEARCH_EX_H
	
#ifndef _FPDFVIEW_H_
#include "fpdfview.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Function: FPDFText_GetCharIndexFromTextIndex
//		Get the actually char index in text_page's internal char list.
// Parameters:
//			text_page	- 	Handle to a text page information structure. Returned by FPDFText_LoadPage function.
//			nTextIndex	-	The index of the text in the string get from FPDFText_GetText.
//	Return value:
//			The index of the character in internal charlist. -1 for error.
DLLEXPORT int STDCALL  FPDFText_GetCharIndexFromTextIndex(FPDF_TEXTPAGE text_page, int nTextIndex);

#ifdef __cplusplus
};
#endif


#endif

