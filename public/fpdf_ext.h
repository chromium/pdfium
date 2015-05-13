// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef PUBLIC_FPDF_EXT_H_
#define PUBLIC_FPDF_EXT_H_

#include "fpdfview.h"

#ifdef __cplusplus
extern "C" {
#endif

//flags for type of unsupport object.
#define FPDF_UNSP_DOC_XFAFORM               1
#define FPDF_UNSP_DOC_PORTABLECOLLECTION    2
#define FPDF_UNSP_DOC_ATTACHMENT            3
#define FPDF_UNSP_DOC_SECURITY              4
#define FPDF_UNSP_DOC_SHAREDREVIEW          5
#define FPDF_UNSP_DOC_SHAREDFORM_ACROBAT    6
#define FPDF_UNSP_DOC_SHAREDFORM_FILESYSTEM 7
#define FPDF_UNSP_DOC_SHAREDFORM_EMAIL      8
#define FPDF_UNSP_ANNOT_3DANNOT             11
#define FPDF_UNSP_ANNOT_MOVIE               12
#define FPDF_UNSP_ANNOT_SOUND               13
#define FPDF_UNSP_ANNOT_SCREEN_MEDIA        14
#define FPDF_UNSP_ANNOT_SCREEN_RICHMEDIA    15
#define FPDF_UNSP_ANNOT_ATTACHMENT          16
#define FPDF_UNSP_ANNOT_SIG                 17

typedef struct _UNSUPPORT_INFO
{
    /**
    * Version number of the interface. Currently must be 1.
    **/
    int version;

    /**
    * Method: FSDK_UnSupport_Handler
    *            UnSupport Object process handling function.
    * Interface Version:
    *           1
    * Implementation Required:
    *           Yes
    * Parameters:
    *       pThis       -   Pointer to the interface structure itself.
    *       nType       -   The type of unsupportObject
    *   Return value:
    *       None.
    * */

    void(*FSDK_UnSupport_Handler)(struct _UNSUPPORT_INFO* pThis, int nType);
} UNSUPPORT_INFO;

/**
 * Function: FSDK_SetUnSpObjProcessHandler
 *           Setup A UnSupport Object process handler for foxit sdk.
 * Parameters:
 *          unsp_info       -   Pointer to a UNSUPPORT_INFO structure.
 * Return Value:
 *          TRUE means successful. FALSE means fails.
 **/

DLLEXPORT FPDF_BOOL STDCALL FSDK_SetUnSpObjProcessHandler(UNSUPPORT_INFO* unsp_info);

//flags for page mode.

//Unknown value
#define PAGEMODE_UNKNOWN        -1

//Neither document outline nor thumbnail images visible
#define PAGEMODE_USENONE        0

//Document outline visible
#define PAGEMODE_USEOUTLINES    1

//Thumbnial images visible
#define PAGEMODE_USETHUMBS      2

//Full-screen mode, with no menu bar, window controls, or any other window visible
#define PAGEMODE_FULLSCREEN     3

//Optional content group panel visible
#define PAGEMODE_USEOC          4

//Attachments panel visible
#define PAGEMODE_USEATTACHMENTS 5


/**
 * Function: FPDFDoc_GetPageMode
 *           Get the document's PageMode(How the document should be displayed when opened)
 * Parameters:
 *          doc     -   Handle to document. Returned by FPDF_LoadDocument function.
 * Return Value:
 *          The flags for page mode.
 **/
DLLEXPORT int FPDFDoc_GetPageMode(FPDF_DOCUMENT document);

#ifdef __cplusplus
}
#endif

#endif  // PUBLIC_FPDF_EXT_H_
