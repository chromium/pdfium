// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef PUBLIC_FPDF_EDIT_H_
#define PUBLIC_FPDF_EDIT_H_

#include "fpdfview.h"

// Define all types used in the SDK. Note they can be simply regarded as opaque pointers
// or long integer numbers.

#define FPDF_ARGB(a,r,g,b)      ((((FX_DWORD)(((FX_BYTE)(b)|((FX_WORD)((FX_BYTE)(g))<<8))|(((FX_DWORD)(FX_BYTE)(r))<<16)))) | (((FX_DWORD)(FX_BYTE)(a))<<24))
#define FPDF_GetBValue(argb)    ((FX_BYTE)(argb))
#define FPDF_GetGValue(argb)    ((FX_BYTE)(((FX_WORD)(argb)) >> 8))
#define FPDF_GetRValue(argb)    ((FX_BYTE)((argb)>>16))
#define FPDF_GetAValue(argb)    ((FX_BYTE)((argb)>>24))

#ifdef __cplusplus
extern "C" {
#endif

//////////////////////////////////////////////////////////////////////
//
// Document functions
//
//////////////////////////////////////////////////////////////////////

// Function: FPDF_CreateNewDocument
//          Create a new PDF document.
// Parameters:
//          None.
// Return value:
//          A handle to a document. If failed, NULL is returned.
DLLEXPORT FPDF_DOCUMENT STDCALL FPDF_CreateNewDocument();

//////////////////////////////////////////////////////////////////////
//
// Page functions
//
//////////////////////////////////////////////////////////////////////

// Function: FPDFPage_New
//          Construct an empty page.
// Parameters:
//          document    -   Handle to document. Returned by FPDF_LoadDocument and FPDF_CreateNewDocument.
//          page_index  -   The index of a page.
//          width       -   The page width.
//          height      -   The page height.
// Return value:
//          The handle to the page.
// Comments:
//          Loaded page can be deleted by FPDFPage_Delete.
DLLEXPORT FPDF_PAGE STDCALL FPDFPage_New(FPDF_DOCUMENT document, int page_index, double width, double height);

// Function: FPDFPage_Delete
//          Delete a PDF page.
// Parameters:
//          document    -   Handle to document. Returned by FPDF_LoadDocument and FPDF_CreateNewDocument.
//          page_index  -   The index of a page.
// Return value:
//          None.
DLLEXPORT void STDCALL FPDFPage_Delete(FPDF_DOCUMENT document, int page_index);

// Function: FPDFPage_GetRotation
//          Get the page rotation. One of following values will be returned: 0(0), 1(90), 2(180), 3(270).
// Parameters:
//          page        -   Handle to a page. Returned by FPDFPage_New or FPDF_LoadPage.
// Return value:
//          The PDF page rotation.
// Comment:
//          The PDF page rotation is rotated clockwise.
DLLEXPORT int STDCALL FPDFPage_GetRotation(FPDF_PAGE page);

// Function: FPDFPage_SetRotation
//          Set page rotation. One of following values will be set: 0(0), 1(90), 2(180), 3(270).
// Parameters:
//          page        -   Handle to a page. Returned by FPDFPage_New or FPDF_LoadPage.
//          rotate      -   The value of the PDF page rotation.
// Return value:
//          None.
// Comment:
//          The PDF page rotation is rotated clockwise.
//
DLLEXPORT void STDCALL FPDFPage_SetRotation(FPDF_PAGE page, int rotate);

// Function: FPDFPage_InsertObject
//          Insert an object to the page. The page object is automatically freed.
// Parameters:
//          page        -   Handle to a page. Returned by FPDFPage_New or FPDF_LoadPage.
//          page_obj    -   Handle to a page object. Returned by FPDFPageObj_NewTextObj,FPDFPageObj_NewTextObjEx and
//                          FPDFPageObj_NewPathObj.
// Return value:
//          None.
DLLEXPORT void STDCALL FPDFPage_InsertObject(FPDF_PAGE page, FPDF_PAGEOBJECT page_obj);

// Function: FPDFPage_CountObject
//          Get number of page objects inside the page.
// Parameters:
//          page        -   Handle to a page. Returned by FPDFPage_New or FPDF_LoadPage.
// Return value:
//          The number of the page object.
DLLEXPORT int STDCALL FPDFPage_CountObject(FPDF_PAGE page);

// Function: FPDFPage_GetObject
//          Get page object by index.
// Parameters:
//          page        -   Handle to a page. Returned by FPDFPage_New or FPDF_LoadPage.
//          index       -   The index of a page object.
// Return value:
//          The handle of the page object. Null for failed.
DLLEXPORT FPDF_PAGEOBJECT STDCALL FPDFPage_GetObject(FPDF_PAGE page, int index);

// Function: FPDFPage_HasTransparency
//          Check that whether the content of specified PDF page contains transparency.
// Parameters:
//          page        -   Handle to a page. Returned by FPDFPage_New or FPDF_LoadPage.
// Return value:
//          TRUE means that the PDF page does contains transparency.
//          Otherwise, returns FALSE.
DLLEXPORT FPDF_BOOL STDCALL FPDFPage_HasTransparency(FPDF_PAGE page);

// Function: FPDFPage_GenerateContent
//          Generate PDF Page content.
// Parameters:
//          page        -   Handle to a page. Returned by FPDFPage_New or FPDF_LoadPage.
// Return value:
//          True if successful, false otherwise.
// Comment:
//          Before you save the page to a file, or reload the page, you must call the FPDFPage_GenerateContent function.
//          Or the changed information will be lost.
DLLEXPORT FPDF_BOOL STDCALL FPDFPage_GenerateContent(FPDF_PAGE page);

//////////////////////////////////////////////////////////////////////
//
// Page Object functions
//
//////////////////////////////////////////////////////////////////////

// Function: FPDFPageObj_HasTransparency
//          Check that whether the specified PDF page object contains transparency.
// Parameters:
//          pageObject  -   Handle to a page object.
// Return value:
//          TRUE means that the PDF page object does contains transparency.
//          Otherwise, returns FALSE.
DLLEXPORT FPDF_BOOL STDCALL FPDFPageObj_HasTransparency(FPDF_PAGEOBJECT pageObject);

// Function: FPDFPageObj_Transform
//          Transform (scale, rotate, shear, move) page object.
// Parameters:
//          page_object -   Handle to a page object. Returned by FPDFPageObj_NewImageObj.
//          a           -   The coefficient "a" of the matrix.
//          b           -   The coefficient "b" of the matrix.
//          c           -   The coefficient "c" of the matrix.
//          d           -   The coefficient "d" of the matrix.
//          e           -   The coefficient "e" of the matrix.
//          f           -   The coefficient "f" of the matrix.
// Return value:
//          None.
DLLEXPORT void STDCALL FPDFPageObj_Transform(FPDF_PAGEOBJECT page_object,
                            double a, double b, double c, double d, double e, double f);

// Function: FPDFPage_TransformAnnots
//          Transform (scale, rotate, shear, move) all annots in a page.
// Parameters:
//          page        -   Handle to a page.
//          a           -   The coefficient "a" of the matrix.
//          b           -   The coefficient "b" of the matrix.
//          c           -   The coefficient "c" of the matrix.
//          d           -   The coefficient "d" of the matrix.
//          e           -   The coefficient "e" of the matrix.
//          f           -   The coefficient "f" of the matrix.
// Return value:
//          None.
DLLEXPORT void STDCALL FPDFPage_TransformAnnots(FPDF_PAGE page,
                                             double a, double b, double c, double d, double e, double f);

// The page object constants.
#define FPDF_PAGEOBJ_TEXT       1
#define FPDF_PAGEOBJ_PATH       2
#define FPDF_PAGEOBJ_IMAGE      3
#define FPDF_PAGEOBJ_SHADING    4
#define FPDF_PAGEOBJ_FORM       5

//////////////////////////////////////////////////////////////////////
//
// Image functions
//
//////////////////////////////////////////////////////////////////////

// Function: FPDFPageObj_NewImgeObj
//          Create a new Image Object.
// Parameters:
//          document        -   Handle to document. Returned by FPDF_LoadDocument or FPDF_CreateNewDocument function.
// Return Value:
//          Handle of image object.
DLLEXPORT FPDF_PAGEOBJECT STDCALL FPDFPageObj_NewImgeObj(FPDF_DOCUMENT document);


// Function: FPDFImageObj_LoadJpegFile
//          Load Image from a JPEG image file and then set it to an image object.
// Parameters:
//          pages           -   Pointers to the start of all loaded pages, could be NULL.
//          nCount          -   Number of pages, could be 0.
//          image_object    -   Handle of image object returned by FPDFPageObj_NewImgeObj.
//          fileAccess      -   The custom file access handler, which specifies the JPEG image file.
//  Return Value:
//          TRUE if successful, FALSE otherwise.
//  Note:
//          The image object might already has an associated image, which is shared and cached by the loaded pages, In this case, we need to clear the cache of image for all the loaded pages.
//          Pass pages and count to this API to clear the image cache.
DLLEXPORT FPDF_BOOL STDCALL FPDFImageObj_LoadJpegFile(FPDF_PAGE* pages, int nCount,FPDF_PAGEOBJECT image_object, FPDF_FILEACCESS* fileAccess);


// Function: FPDFImageObj_SetMatrix
//          Set the matrix of an image object.
// Parameters:
//          image_object    -   Handle of image object returned by FPDFPageObj_NewImgeObj.
//          a               -   The coefficient "a" of the matrix.
//          b               -   The coefficient "b" of the matrix.
//          c               -   The coefficient "c" of the matrix.
//          d               -   The coefficient "d" of the matrix.
//          e               -   The coefficient "e" of the matrix.
//          f               -   The coefficient "f" of the matrix.
// Return value:
//          TRUE if successful, FALSE otherwise.
DLLEXPORT FPDF_BOOL STDCALL FPDFImageObj_SetMatrix(FPDF_PAGEOBJECT image_object,
                                             double a, double b, double c, double d, double e, double f);

// Function: FPDFImageObj_SetBitmap
//          Set the bitmap to an image object.
// Parameters:
//          pages           -   Pointer's to the start of all loaded pages.
//          nCount          -   Number of pages.
//          image_object    -   Handle of image object returned by FPDFPageObj_NewImgeObj.
//          bitmap          -   The handle of the bitmap which you want to set it to the image object.
// Return value:
//          TRUE if successful, FALSE otherwise.
DLLEXPORT FPDF_BOOL STDCALL FPDFImageObj_SetBitmap(FPDF_PAGE* pages,int nCount,FPDF_PAGEOBJECT image_object, FPDF_BITMAP bitmap);

#ifdef __cplusplus
}
#endif

#endif  // PUBLIC_FPDF_EDIT_H_
