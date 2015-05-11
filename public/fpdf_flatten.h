// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FPDF_FLATTEN_H_
#define _FPDF_FLATTEN_H_
	
#include "fpdfview.h"

#define FLATTEN_FAIL			0	// Flatten operation failed.
#define FLATTEN_SUCCESS			1	// Flatten operation succeed.
#define FLATTEN_NOTINGTODO		2	// There is nothing can be flatten.
	
#ifdef __cplusplus
extern "C" {
#endif

#define FLAT_NORMALDISPLAY     0
#define FLAT_PRINT             1    
	//Function: FPDFPage_Flatten

	//			Flat a pdf page,annotations or form fields will become part of the page contents.
	//Parameters:

	//			page  - Handle to the page. Returned by FPDF_LoadPage function.
	//			nFlag - the flag for the use of flatten result. Zero for normal display, 1 for print.
	//Return value:
	//			The result flag of the function, See flags above ( FLATTEN_FAIL, FLATTEN_SUCCESS, FLATTEN_NOTINGTODO ).
	//
	// Comments: Current version all fails return zero. If necessary we will assign different value
	//			to indicate different fail reason.
	// 
	DLLEXPORT int STDCALL FPDFPage_Flatten( FPDF_PAGE page, int nFlag);
		
		
#ifdef __cplusplus
};
#endif

#endif //_FPDF_FLATTEN_H_
