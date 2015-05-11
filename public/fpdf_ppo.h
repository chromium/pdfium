// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FPDFPPO_H_
#define _FPDFPPO_H_

#include "fpdfview.h"

#ifdef __cplusplus
extern "C" {
#endif

// Function: FPDF_ImportPages
//			Import some pages to a PDF document.
// Parameters:	
//			dest_doc	-	The destination document which add the pages.
//			src_doc		-	A document to be imported.
//			pagerange	-	A page range string, Such as "1,3,5-7". 
//							If this parameter is NULL, it would import all pages in src_doc.
//			index		-	The page index wanted to insert from.	
// Return value:
//			TRUE for succeed, FALSE for Failed.	
DLLEXPORT FPDF_BOOL STDCALL FPDF_ImportPages(FPDF_DOCUMENT dest_doc,FPDF_DOCUMENT src_doc, FPDF_BYTESTRING pagerange, int index);


// Function: FPDF_CopyViewerPreferences
//			Copy the viewer preferences from one PDF document to another.#endif
// Parameters:	
//			dest_doc	-	Handle to document to write the viewer preferences to.
//			src_doc		-	Handle to document with the viewer preferences.
// Return value:
//			TRUE for success, FALSE for failure.
DLLEXPORT FPDF_BOOL STDCALL FPDF_CopyViewerPreferences(FPDF_DOCUMENT dest_doc, FPDF_DOCUMENT src_doc);

#ifdef __cplusplus
};
#endif

#endif //_FPDFPPO_H_

