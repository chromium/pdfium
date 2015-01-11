// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FPDFDOC_H_
#define _FPDFDOC_H_

#include "fpdfview.h"

// Exported Functions
#ifdef __cplusplus
extern "C" {
#endif

// Function: FPDFBookmark_GetFirstChild
//			Get the first child of a bookmark item, or the first top level bookmark item.
// Parameters:
//			document	-	Handle to the document. Returned by FPDF_LoadDocument or FPDF_LoadMemDocument.
//			bookmark	-	Handle to the current bookmark. Can be NULL if you want to get the first top level item.
// Return value:
//			Handle to the first child or top level bookmark item. NULL if no child or top level bookmark found.
//
DLLEXPORT FPDF_BOOKMARK STDCALL FPDFBookmark_GetFirstChild(FPDF_DOCUMENT document, FPDF_BOOKMARK bookmark);

// Function: FPDFBookmark_GetNextSibling
//			Get next bookmark item at the same level.
// Parameters:
//			document	-	Handle to the document. Returned by FPDF_LoadDocument or FPDF_LoadMemDocument.
//			bookmark	-	Handle to the current bookmark. Cannot be NULL.
// Return value:
//			Handle to the next bookmark item at the same level. NULL if this is the last bookmark at this level.
//
DLLEXPORT FPDF_BOOKMARK STDCALL FPDFBookmark_GetNextSibling(FPDF_DOCUMENT document, FPDF_BOOKMARK bookmark);

// Function: FPDFBookmark_GetTitle
//			Get title of a bookmark.
// Parameters:
//			bookmark	-	Handle to the bookmark.
//			buffer  	-	Buffer for the title. Can be NULL.
//          buflen      -   The length of the buffer in bytes. Can be 0.
// Return value:
//			Number of bytes the title consumes, including trailing zeros.
// Comments:
//			Regardless of the platform, the title is always in UTF-16LE encoding. That means the buffer
//          can be treated as an array of WORD (on Intel and compatible CPUs), each WORD representing the Unicode of
//          a character(some special Unicode may take 2 WORDs).The string is followed by two bytes of zero
//          indicating the end of the string.
//
//          The return value always indicates the number of bytes required for the buffer, even if no buffer is specified
//          or the buffer size is less then required. In these cases, the buffer will not be modified.
//
DLLEXPORT unsigned long STDCALL FPDFBookmark_GetTitle(FPDF_BOOKMARK bookmark, void* buffer, unsigned long buflen);

// Function: FPDFBookmark_Find
//			Find a bookmark in the document, using the bookmark title.
// Parameters: 
//			document	-	Handle to the document. Returned by FPDF_LoadDocument or FPDF_LoadMemDocument.
//			title		-	The UTF-16LE encoded Unicode string for the bookmark title to be searched. Can't be NULL.
// Return value:
//			Handle to the found bookmark item. NULL if the title can't be found.
// Comments:
//			It always returns the first found bookmark if more than one bookmarks have the same title.
//
DLLEXPORT FPDF_BOOKMARK STDCALL FPDFBookmark_Find(FPDF_DOCUMENT document, FPDF_WIDESTRING title);

// Function: FPDFBookmark_GetDest
//			Get the destination associated with a bookmark item.
// Parameters:
//			document	-	Handle to the document.
//			bookmark	-	Handle to the bookmark.
// Return value:
//			Handle to the destination data. NULL if no destination is associated with this bookmark.
//
DLLEXPORT FPDF_DEST STDCALL FPDFBookmark_GetDest(FPDF_DOCUMENT document, FPDF_BOOKMARK bookmark);

// Function: FPDFBookmark_GetAction
//			Get the action associated with a bookmark item.
// Parameters:
//			bookmark	-	Handle to the bookmark.
// Return value:
//			Handle to the action data. NULL if no action is associated with this bookmark. In this case, the 
//			application should try FPDFBookmark_GetDest.
//
DLLEXPORT FPDF_ACTION STDCALL FPDFBookmark_GetAction(FPDF_BOOKMARK bookmark);

#define PDFACTION_UNSUPPORTED		0		// Unsupported action type.
#define PDFACTION_GOTO				1		// Go to a destination within current document.
#define PDFACTION_REMOTEGOTO		2		// Go to a destination within another document.
#define PDFACTION_URI				3		// Universal Resource Identifier, including web pages and 
											// other Internet based resources.
#define PDFACTION_LAUNCH			4		// Launch an application or open a file.

// Function: FPDFAction_GetType
//			Get type of an action.
// Parameters:
//			action		-	Handle to the action.
// Return value:
//			A type number as defined above.
//
DLLEXPORT unsigned long STDCALL FPDFAction_GetType(FPDF_ACTION action);

// Function: FPDFAction_GetDest
//			Get destination of an action.
// Parameters:
//			document	-	Handle to the document.
//			action		-	Handle to the action. It must be a GOTO or REMOTEGOTO action.
// Return value:
//			Handle to the destination data.
// Comments:
//			In case of remote goto action, the application should first use FPDFAction_GetFilePath to
//			get file path, then load that particular document, and use its document handle to call this
//			function.
//
DLLEXPORT FPDF_DEST STDCALL FPDFAction_GetDest(FPDF_DOCUMENT document, FPDF_ACTION action);

// Function: FPDFAction_GetURIPath
//			Get URI path of a URI action.
// Parameters:
//			document	-	Handle to the document.
//			action		-	Handle to the action. Must be a URI action.
//			buffer		-	A buffer for output the path string. Can be NULL.
//			buflen		-	The length of the buffer, number of bytes. Can be 0.
// Return value:
//			Number of bytes the URI path consumes, including trailing zeros.
// Comments:
//			The URI path is always encoded in 7-bit ASCII.
// 
//			The return value always indicated number of bytes required for the buffer, even when there is
//			no buffer specified, or the buffer size is less then required. In this case, the buffer will not
//			be modified.
//
DLLEXPORT unsigned long STDCALL FPDFAction_GetURIPath(FPDF_DOCUMENT document, FPDF_ACTION action, 
													  void* buffer, unsigned long buflen);

// Function: FPDFDest_GetPageIndex
//			Get page index of a destination.
// Parameters:
//			document	-	Handle to the document.
//			dest		-	Handle to the destination.
// Return value:
//			The page index. Starting from 0 for the first page.
//
DLLEXPORT unsigned long STDCALL FPDFDest_GetPageIndex(FPDF_DOCUMENT document, FPDF_DEST dest);

// Function: FPDFLink_GetLinkAtPoint
//			Find a link at specified point on a document page.
// Parameters:
//			page		-	Handle to the document page.
//			x			-	The x coordinate of the point, specified in page coordinate system.
//			y			-	The y coordinate of the point, specified in page coordinate system.
// Return value:
//			Handle to the link. NULL if no link found at that point.
// Comments:
//			The point coordinates are specified in page coordinate system. You can convert coordinates 
//			from screen system to page system using FPDF_DeviceToPage functions.
//
DLLEXPORT FPDF_LINK STDCALL FPDFLink_GetLinkAtPoint(FPDF_PAGE page, double x, double y);

// Function: FPDFLink_GetDest
//			Get destination info of a link.
// Parameters:
//			document	-	Handle to the document.
//			link		-	Handle to the link. Returned by FPDFLink_GetLinkAtPoint.
// Return value:
//			Handle to the destination. NULL if there is no destination associated with the link, in this case
//			the application should try FPDFLink_GetAction.
//
DLLEXPORT FPDF_DEST STDCALL FPDFLink_GetDest(FPDF_DOCUMENT document, FPDF_LINK link);

// Function: FPDFLink_GetAction
//			Get action info of a link.
// Parameters:
//			link		-	Handle to the link.
// Return value:
//			Handle to the action. NULL if there is no action associated with the link.
//
DLLEXPORT FPDF_ACTION STDCALL FPDFLink_GetAction(FPDF_LINK link);

// Function: FPDFLink_Enumerate
//			This function would enumerate all the link annotations in a single PDF page.
// Parameters:
//			page[in]			-	Handle to the page.
//			startPos[in,out]	-	The start position to enumerate the link annotations, which should be specified to start from 
//								-	0 for the first call, and would receive the next position for enumerating to start from.
//			linkAnnot[out]		-	Receive the link handle.
// Return value:
//			TRUE if succceed, else False;
//
DLLEXPORT FPDF_BOOL STDCALL FPDFLink_Enumerate(FPDF_PAGE page, int* startPos, FPDF_LINK* linkAnnot);

// Function: FPDFLink_GetAnnotRect
//			Get the annotation rectangle. (Specified by the ¡°Rect¡± entry of annotation dictionary).
// Parameters:
//			linkAnnot[in]		-	Handle to the link annotation.
//			rect[out]			-	The annotation rect.
// Return value:
//			TRUE if succceed, else False;
//
DLLEXPORT FPDF_BOOL STDCALL FPDFLink_GetAnnotRect(FPDF_LINK linkAnnot, FS_RECTF* rect);

// Function: FPDFLink_CountQuadPoints
//			Get the count of quadrilateral points to the link annotation.
// Parameters:
//			linkAnnot[in]		-	Handle to the link annotation.
// Return value:
//			The count of quadrilateral points.
//
DLLEXPORT int STDCALL FPDFLink_CountQuadPoints(FPDF_LINK linkAnnot);

/* _FS_DEF_STRUCTURE_QUADPOINTSF_ */
#ifndef _FS_DEF_STRUCTURE_QUADPOINTSF_
#define _FS_DEF_STRUCTURE_QUADPOINTSF_
typedef struct _FS_QUADPOINTSF
{
	FS_FLOAT  x1;
	FS_FLOAT  y1;
	FS_FLOAT  x2;
	FS_FLOAT  y2;
	FS_FLOAT  x3;
	FS_FLOAT  y3;
	FS_FLOAT  x4;
	FS_FLOAT  y4;
} FS_QUADPOINTSF;
#endif /* _FS_DEF_STRUCTURE_QUADPOINTSF_ */

// Function: FPDFLink_GetQuadPoints
//			Get the quadrilateral points for the specified index in the link annotation.
// Parameters:
//			linkAnnot[in]		-	Handle to the link annotation.
//			quadIndex[in]		-	The specified quad points index.
//			quadPoints[out]		-	Receive the quadrilateral points.
// Return value:
//			True if succeed, else False.
//
DLLEXPORT FPDF_BOOL STDCALL FPDFLink_GetQuadPoints(FPDF_LINK linkAnnot, int quadIndex, FS_QUADPOINTSF* quadPoints);

// Function: FPDF_GetMetaText
//			Get a text from meta data of the document. Result is encoded in UTF-16LE.
// Parameters:
//			doc			-	Handle to a document
//			tag			-	The tag for the meta data. Currently, It can be "Title", "Author", 
//							"Subject", "Keywords", "Creator", "Producer", "CreationDate", or "ModDate".
//							For detailed explanation of these tags and their respective values,
//							please refer to PDF Reference 1.6, section 10.2.1, "Document Information Dictionary".
//			buffer		-	A buffer for output the title. Can be NULL.
//			buflen		-	The length of the buffer, number of bytes. Can be 0.
// Return value:
//			Number of bytes the title consumes, including trailing zeros.
// Comments:
//			No matter on what platform, the title is always output in UTF-16LE encoding, which means the buffer 
//			can be regarded as an array of WORD (on Intel and compatible CPUs), each WORD represent the Unicode of 
//			a character (some special Unicode may take 2 WORDs). The string is followed by two bytes of zero 
//			indicating end of the string.
//
//			The return value always indicated number of bytes required for the buffer, even when there is
//			no buffer specified, or the buffer size is less then required. In this case, the buffer will not
//			be modified.
//
DLLEXPORT unsigned long STDCALL FPDF_GetMetaText(FPDF_DOCUMENT doc, FPDF_BYTESTRING tag,
												 void* buffer, unsigned long buflen);


#ifdef __cplusplus
};
#endif

#endif	// _FPDFDOC_H_
