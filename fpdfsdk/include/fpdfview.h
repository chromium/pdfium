// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com


#ifndef _FPDFVIEW_H_
#define _FPDFVIEW_H_

#if defined(_WIN32) && !defined(__WINDOWS__)
#include <windows.h>
#endif

// Data types
typedef void*	FPDF_MODULEMGR;

// PDF types
typedef void*	FPDF_DOCUMENT;		
typedef void*	FPDF_PAGE;			
typedef void*	FPDF_PAGEOBJECT;	// Page object(text, path, etc)
typedef void*	FPDF_PATH;
typedef void*	FPDF_CLIPPATH;	
typedef void*	FPDF_BITMAP;	
typedef void*	FPDF_FONT;			

typedef void*	FPDF_TEXTPAGE;
typedef void*	FPDF_SCHHANDLE;
typedef void*	FPDF_PAGELINK;
typedef void*	FPDF_HMODULE;
typedef void*	FPDF_DOCSCHHANDLE;

typedef void*	FPDF_BOOKMARK;
typedef void*	FPDF_DEST;
typedef void*	FPDF_ACTION;
typedef void*	FPDF_LINK;
typedef void*   FPDF_PAGERANGE;

// Basic data types
typedef int				FPDF_BOOL;
typedef int				FPDF_ERROR;	
typedef unsigned long	FPDF_DWORD;

typedef	float			FS_FLOAT;

// Duplex types
typedef enum _FPDF_DUPLEXTYPE_ {
    DuplexUndefined = 0,
    Simplex,
    DuplexFlipShortEdge,
    DuplexFlipLongEdge
} FPDF_DUPLEXTYPE;

// String types
typedef unsigned short			FPDF_WCHAR;
typedef unsigned char const*	FPDF_LPCBYTE;

// FPDFSDK may use three types of strings: byte string, wide string (UTF-16LE encoded), and platform dependent string
typedef const char*				FPDF_BYTESTRING;

typedef const unsigned short*	FPDF_WIDESTRING;		// Foxit PDF SDK always use UTF-16LE encoding wide string,
														// each character use 2 bytes (except surrogation), with low byte first.

// For Windows programmers: for most case it's OK to treat FPDF_WIDESTRING as Windows unicode string,
//		 however, special care needs to be taken if you expect to process Unicode larger than 0xffff.
// For Linux/Unix programmers: most compiler/library environment uses 4 bytes for a Unicode character,
//		you have to convert between FPDF_WIDESTRING and system wide string by yourself.

#ifdef _WIN32_WCE
typedef const unsigned short* FPDF_STRING;
#else
typedef const char* FPDF_STRING;
#endif

#ifndef _FS_DEF_MATRIX_
#define _FS_DEF_MATRIX_
/** @brief Matrix for transformation. */
typedef struct _FS_MATRIX_
{
	float	a;	/**< @brief Coefficient a.*/
	float	b;	/**< @brief Coefficient b.*/
	float	c;	/**< @brief Coefficient c.*/
	float	d;	/**< @brief Coefficient d.*/
	float	e;	/**< @brief Coefficient e.*/
	float	f;	/**< @brief Coefficient f.*/
} FS_MATRIX;
#endif

#ifndef _FS_DEF_RECTF_
#define _FS_DEF_RECTF_
/** @brief Rectangle area(float) in device or page coordination system. */
typedef struct _FS_RECTF_
{
	/**@{*/
	/** @brief The x-coordinate of the left-top corner. */
	float	left;
	/** @brief The y-coordinate of the left-top corner. */
	float	top;
	/** @brief The x-coordinate of the right-bottom corner. */
	float	right;
	/** @brief The y-coordinate of the right-bottom corner. */
	float	bottom;
	/**@}*/
}* FS_LPRECTF, FS_RECTF;
/** @brief Const Pointer to ::FS_RECTF structure.*/
typedef const FS_RECTF*	FS_LPCRECTF;
#endif

#if defined(_WIN32) && defined(FPDFSDK_EXPORTS)
// On Windows system, functions are exported in a DLL
#define DLLEXPORT __declspec( dllexport )
#define STDCALL __stdcall
#else
#define DLLEXPORT
#define STDCALL
#endif

extern const char g_ExpireDate[];
extern const char g_ModuleCodes[];

// Exported Functions
#ifdef __cplusplus
extern "C" {
#endif

// Function: FPDF_InitLibrary
//			Initialize the FPDFSDK library 
// Parameters:
//			None
// Return value:
//			None.
// Comments:
//			You have to call this function before you can call any PDF processing functions.

DLLEXPORT void STDCALL FPDF_InitLibrary();


// Function: FPDF_DestroyLibary
//			Release all resources allocated by the FPDFSDK library.
// Parameters:
//			None.
// Return value:
//			None.
// Comments:
//			You can call this function to release all memory blocks allocated by the library. 
//			After this function called, you should not call any PDF processing functions.
DLLEXPORT void STDCALL FPDF_DestroyLibrary();

//Policy for accessing the local machine time.
#define FPDF_POLICY_MACHINETIME_ACCESS	0

// Function: FPDF_SetSandBoxPolicy
//			Set the policy for the sandbox environment.
// Parameters:	
//			policy		-	The specified policy for setting, for example:FPDF_POLICY_MACHINETIME_ACCESS.
//			enable		-	True for enable, False for disable the policy.
// Return value:
//			None.
DLLEXPORT void	STDCALL FPDF_SetSandBoxPolicy(FPDF_DWORD policy, FPDF_BOOL enable);

/**
* Open and load a PDF document.
* @param[in] file_path	-	Path to the PDF file (including extension).
* @param[in] password	-	A string used as the password for PDF file. 
*							If no password needed, empty or NULL can be used.
* @note		Loaded document can be closed by FPDF_CloseDocument.
*			If this function fails, you can use FPDF_GetLastError() to retrieve
*			the reason why it fails.
* @retval	A handle to the loaded document. If failed, NULL is returned.
*/
DLLEXPORT FPDF_DOCUMENT	STDCALL FPDF_LoadDocument(FPDF_STRING file_path, 
	FPDF_BYTESTRING password);

// Function: FPDF_LoadMemDocument
//			Open and load a PDF document from memory.
// Parameters: 
//			data_buf	-	Pointer to a buffer containing the PDF document.
//			size		-	Number of bytes in the PDF document.
//			password	-	A string used as the password for PDF file. 
//							If no password needed, empty or NULL can be used.
// Return value:
//			A handle to the loaded document. If failed, NULL is returned.
// Comments:
//			The memory buffer must remain valid when the document is open.
//			Loaded document can be closed by FPDF_CloseDocument.
//			If this function fails, you can use FPDF_GetLastError() to retrieve
//			the reason why it fails.
//
DLLEXPORT FPDF_DOCUMENT	STDCALL FPDF_LoadMemDocument(const void* data_buf, 
											int size, FPDF_BYTESTRING password);

// Structure for custom file access.
typedef struct {
	// File length, in bytes.
	unsigned long	m_FileLen;

	// A function pointer for getting a block of data from specific position.
	// Position is specified by byte offset from beginning of the file.
	// The position and size will never go out range of file length.
	// It may be possible for FPDFSDK to call this function multiple times for same position.
	// Return value: should be non-zero if successful, zero for error.
	int				(*m_GetBlock)(void* param, unsigned long position, unsigned char* pBuf, unsigned long size);

	// A custom pointer for all implementation specific data.
	// This pointer will be used as the first parameter to m_GetBlock callback.
	void*			m_Param;
} FPDF_FILEACCESS;

// Function: FPDF_LoadCustomDocument
//			Load PDF document from a custom access descriptor.
// Parameters:
//			pFileAccess	-	A structure for access the file.
//			password	-	Optional password for decrypting the PDF file.
// Return value:
//			A handle to the loaded document. If failed, NULL is returned.
// Comments:
//			The application should maintain the file resources being valid until the PDF document close.
//			Loaded document can be closed by FPDF_CloseDocument.
DLLEXPORT FPDF_DOCUMENT STDCALL FPDF_LoadCustomDocument(FPDF_FILEACCESS* pFileAccess, 
														FPDF_BYTESTRING password);

// Function: FPDF_GetFileVersion
//			Get the file version of the specific PDF document.
// Parameters:
//			doc			-	Handle to document.
//			fileVersion	-	The PDF file version. File version: 14 for 1.4, 15 for 1.5, ...
// Return value:
//			TRUE if this call succeed, If failed, FALSE is returned.
// Comments:
//			If the document is created by function ::FPDF_CreateNewDocument, then this function would always fail.
DLLEXPORT FPDF_BOOL STDCALL FPDF_GetFileVersion(FPDF_DOCUMENT doc, int* fileVersion);

#define FPDF_ERR_SUCCESS		0		// No error.
#define FPDF_ERR_UNKNOWN		1		// Unknown error.
#define FPDF_ERR_FILE			2		// File not found or could not be opened.
#define FPDF_ERR_FORMAT			3		// File not in PDF format or corrupted.
#define FPDF_ERR_PASSWORD		4		// Password required or incorrect password.
#define FPDF_ERR_SECURITY		5		// Unsupported security scheme.
#define FPDF_ERR_PAGE			6		// Page not found or content error.

// Function: FPDF_GetLastError
//			Get last error code when an SDK function failed.
// Parameters: 
//			None.
// Return value:
//			A 32-bit integer indicating error codes (defined above).
// Comments:
//			If the previous SDK call succeeded, the return value of this function
//			is not defined.
//
DLLEXPORT unsigned long	STDCALL FPDF_GetLastError();

// Function: FPDF_GetDocPermission
//			Get file permission flags of the document.
// Parameters: 
//			document	-	Handle to document. Returned by FPDF_LoadDocument function.
// Return value:
//			A 32-bit integer indicating permission flags. Please refer to PDF Reference for
//			detailed description. If the document is not protected, 0xffffffff will be returned.
//
DLLEXPORT unsigned long	STDCALL FPDF_GetDocPermissions(FPDF_DOCUMENT document);

// Function: FPDF_GetSecurityHandlerRevision
//			Get the revision for security handler.
// Parameters:
//			document	-	Handle to document. Returned by FPDF_LoadDocument function.
// Return value:
//			The security handler revision number. Please refer to PDF Reference for
//			detailed description. If the document is not protected, -1 will be returned.
//
DLLEXPORT int STDCALL FPDF_GetSecurityHandlerRevision(FPDF_DOCUMENT document);

// Function: FPDF_GetPageCount
//			Get total number of pages in a document.
// Parameters: 
//			document	-	Handle to document. Returned by FPDF_LoadDocument function.
// Return value:
//			Total number of pages in the document.
//
DLLEXPORT int STDCALL FPDF_GetPageCount(FPDF_DOCUMENT document);

// Function: FPDF_LoadPage
//			Load a page inside a document.
// Parameters: 
//			document	-	Handle to document. Returned by FPDF_LoadDocument function.
//			page_index	-	Index number of the page. 0 for the first page.
// Return value:
//			A handle to the loaded page. If failed, NULL is returned.
// Comments:
//			Loaded page can be rendered to devices using FPDF_RenderPage function.
//			Loaded page can be closed by FPDF_ClosePage.
//
DLLEXPORT FPDF_PAGE	STDCALL FPDF_LoadPage(FPDF_DOCUMENT document, int page_index);

// Function: FPDF_GetPageWidth
//			Get page width.
// Parameters:
//			page		-	Handle to the page. Returned by FPDF_LoadPage function.
// Return value:
//			Page width (excluding non-displayable area) measured in points.
//			One point is 1/72 inch (around 0.3528 mm).
//
DLLEXPORT double STDCALL FPDF_GetPageWidth(FPDF_PAGE page);

// Function: FPDF_GetPageHeight
//			Get page height.
// Parameters:
//			page		-	Handle to the page. Returned by FPDF_LoadPage function.
// Return value:
//			Page height (excluding non-displayable area) measured in points.
//			One point is 1/72 inch (around 0.3528 mm)
//
DLLEXPORT double STDCALL FPDF_GetPageHeight(FPDF_PAGE page);

// Function: FPDF_GetPageSizeByIndex
//			Get the size of a page by index.
// Parameters:
//			document	-	Handle to document. Returned by FPDF_LoadDocument function.
//			page_index	-	Page index, zero for the first page.
//			width		-	Pointer to a double value receiving the page width (in points).
//			height		-	Pointer to a double value receiving the page height (in points).
// Return value:
//			Non-zero for success. 0 for error (document or page not found).
//
DLLEXPORT int STDCALL FPDF_GetPageSizeByIndex(FPDF_DOCUMENT document, int page_index, double* width, double* height);


// Page rendering flags. They can be combined with bit OR.
#define FPDF_ANNOT			0x01		// Set if annotations are to be rendered.
#define FPDF_LCD_TEXT		0x02		// Set if using text rendering optimized for LCD display.
#define FPDF_NO_NATIVETEXT	0x04		// Don't use the native text output available on some platforms
#define FPDF_GRAYSCALE		0x08		// Grayscale output.
#define FPDF_DEBUG_INFO		0x80		// Set if you want to get some debug info. 
										// Please discuss with Foxit first if you need to collect debug info.
#define FPDF_NO_CATCH		0x100		// Set if you don't want to catch exception.
#define FPDF_RENDER_LIMITEDIMAGECACHE	0x200	// Limit image cache size. 
#define FPDF_RENDER_FORCEHALFTONE		0x400	// Always use halftone for image stretching.
#define FPDF_PRINTING		0x800	// Render for printing.
#define FPDF_REVERSE_BYTE_ORDER		0x10		//set whether render in a reverse Byte order, this flag only 
												//enable when render to a bitmap.
#ifdef _WIN32
// Function: FPDF_RenderPage
//			Render contents in a page to a device (screen, bitmap, or printer).
//			This function is only supported on Windows system.
// Parameters: 
//			dc			-	Handle to device context.
//			page		-	Handle to the page. Returned by FPDF_LoadPage function.
//			start_x		-	Left pixel position of the display area in the device coordinate.
//			start_y		-	Top pixel position of the display area in the device coordinate.
//			size_x		-	Horizontal size (in pixels) for displaying the page.
//			size_y		-	Vertical size (in pixels) for displaying the page.
//			rotate		-	Page orientation: 0 (normal), 1 (rotated 90 degrees clockwise),
//								2 (rotated 180 degrees), 3 (rotated 90 degrees counter-clockwise).
//			flags		-	0 for normal display, or combination of flags defined above.
// Return value:
//			None.
//
DLLEXPORT void STDCALL FPDF_RenderPage(HDC dc, FPDF_PAGE page, int start_x, int start_y, int size_x, int size_y,
						int rotate, int flags);
#endif

// Function: FPDF_RenderPageBitmap
//			Render contents in a page to a device independent bitmap
// Parameters: 
//			bitmap		-	Handle to the device independent bitmap (as the output buffer).
//							Bitmap handle can be created by FPDFBitmap_Create function.
//			page		-	Handle to the page. Returned by FPDF_LoadPage function.
//			start_x		-	Left pixel position of the display area in the bitmap coordinate.
//			start_y		-	Top pixel position of the display area in the bitmap coordinate.
//			size_x		-	Horizontal size (in pixels) for displaying the page.
//			size_y		-	Vertical size (in pixels) for displaying the page.
//			rotate		-	Page orientation: 0 (normal), 1 (rotated 90 degrees clockwise),
//								2 (rotated 180 degrees), 3 (rotated 90 degrees counter-clockwise).
//			flags		-	0 for normal display, or combination of flags defined above.
// Return value:
//			None.
//
DLLEXPORT void STDCALL FPDF_RenderPageBitmap(FPDF_BITMAP bitmap, FPDF_PAGE page, int start_x, int start_y, 
						int size_x, int size_y, int rotate, int flags);

// Function: FPDF_ClosePage
//			Close a loaded PDF page.
// Parameters: 
//			page		-	Handle to the loaded page.
// Return value:
//			None.
//
DLLEXPORT void STDCALL FPDF_ClosePage(FPDF_PAGE page);

// Function: FPDF_CloseDocument
//			Close a loaded PDF document.
// Parameters: 
//			document	-	Handle to the loaded document.
// Return value:
//			None.
//
DLLEXPORT void STDCALL FPDF_CloseDocument(FPDF_DOCUMENT document);

// Function: FPDF_DeviceToPage
//			Convert the screen coordinate of a point to page coordinate.
// Parameters:
//			page		-	Handle to the page. Returned by FPDF_LoadPage function.
//			start_x		-	Left pixel position of the display area in the device coordinate.
//			start_y		-	Top pixel position of the display area in the device coordinate.
//			size_x		-	Horizontal size (in pixels) for displaying the page.
//			size_y		-	Vertical size (in pixels) for displaying the page.
//			rotate		-	Page orientation: 0 (normal), 1 (rotated 90 degrees clockwise),
//								2 (rotated 180 degrees), 3 (rotated 90 degrees counter-clockwise).
//			device_x	-	X value in device coordinate, for the point to be converted.
//			device_y	-	Y value in device coordinate, for the point to be converted.
//			page_x		-	A Pointer to a double receiving the converted X value in page coordinate.
//			page_y		-	A Pointer to a double receiving the converted Y value in page coordinate.
// Return value:
//			None.
// Comments:
//			The page coordinate system has its origin at left-bottom corner of the page, with X axis goes along
//			the bottom side to the right, and Y axis goes along the left side upward. NOTE: this coordinate system 
//			can be altered when you zoom, scroll, or rotate a page, however, a point on the page should always have 
//			the same coordinate values in the page coordinate system. 
//
//			The device coordinate system is device dependent. For screen device, its origin is at left-top
//			corner of the window. However this origin can be altered by Windows coordinate transformation
//			utilities. You must make sure the start_x, start_y, size_x, size_y and rotate parameters have exactly
//			same values as you used in FPDF_RenderPage() function call.
//
DLLEXPORT void STDCALL FPDF_DeviceToPage(FPDF_PAGE page, int start_x, int start_y, int size_x, int size_y,
						int rotate, int device_x, int device_y, double* page_x, double* page_y);

// Function: FPDF_PageToDevice
//			Convert the page coordinate of a point to screen coordinate.
// Parameters:
//			page		-	Handle to the page. Returned by FPDF_LoadPage function.
//			start_x		-	Left pixel position of the display area in the device coordinate.
//			start_y		-	Top pixel position of the display area in the device coordinate.
//			size_x		-	Horizontal size (in pixels) for displaying the page.
//			size_y		-	Vertical size (in pixels) for displaying the page.
//			rotate		-	Page orientation: 0 (normal), 1 (rotated 90 degrees clockwise),
//								2 (rotated 180 degrees), 3 (rotated 90 degrees counter-clockwise).
//			page_x		-	X value in page coordinate, for the point to be converted.
//			page_y		-	Y value in page coordinate, for the point to be converted.
//			device_x	-	A pointer to an integer receiving the result X value in device coordinate.
//			device_y	-	A pointer to an integer receiving the result Y value in device coordinate.
// Return value:
//			None.
// Comments:
//			See comments of FPDF_DeviceToPage() function.
//
DLLEXPORT void STDCALL FPDF_PageToDevice(FPDF_PAGE page, int start_x, int start_y, int size_x, int size_y,
						int rotate, double page_x, double page_y, int* device_x, int* device_y);

// Function: FPDFBitmap_Create
//			Create a Foxit Device Independent Bitmap (FXDIB).
// Parameters:
//			width		-	Number of pixels in a horizontal line of the bitmap. Must be greater than 0.
//			height		-	Number of pixels in a vertical line of the bitmap. Must be greater than 0.
//			alpha		-	A flag indicating whether alpha channel is used. Non-zero for using alpha, zero for not using.
// Return value:
//			The created bitmap handle, or NULL if parameter error or out of memory.
// Comments:
//			An FXDIB always use 4 byte per pixel. The first byte of a pixel is always double word aligned.
//			Each pixel contains red (R), green (G), blue (B) and optionally alpha (A) values.
//			The byte order is BGRx (the last byte unused if no alpha channel) or BGRA.
//			
//			The pixels in a horizontal line (also called scan line) are stored side by side, with left most
//			pixel stored first (with lower memory address). Each scan line uses width*4 bytes.
//
//			Scan lines are stored one after another, with top most scan line stored first. There is no gap
//			between adjacent scan lines.
//
//			This function allocates enough memory for holding all pixels in the bitmap, but it doesn't 
//			initialize the buffer. Applications can use FPDFBitmap_FillRect to fill the bitmap using any color.
DLLEXPORT FPDF_BITMAP STDCALL FPDFBitmap_Create(int width, int height, int alpha);

// More DIB formats
#define FPDFBitmap_Gray		1		// Gray scale bitmap, one byte per pixel.
#define FPDFBitmap_BGR		2		// 3 bytes per pixel, byte order: blue, green, red.
#define FPDFBitmap_BGRx		3		// 4 bytes per pixel, byte order: blue, green, red, unused.
#define FPDFBitmap_BGRA		4		// 4 bytes per pixel, byte order: blue, green, red, alpha.

// Function: FPDFBitmap_CreateEx
//			Create a Foxit Device Independent Bitmap (FXDIB)
// Parameters:
//			width		-	Number of pixels in a horizontal line of the bitmap. Must be greater than 0.
//			height		-	Number of pixels in a vertical line of the bitmap. Must be greater than 0.
//			format		-	A number indicating for bitmap format, as defined above.
//			first_scan	-	A pointer to the first byte of first scan line, for external buffer
//							only. If this parameter is NULL, then the SDK will create its own buffer.
//			stride		-	Number of bytes for each scan line, for external buffer only..
// Return value:
//			The created bitmap handle, or NULL if parameter error or out of memory.
// Comments:
//			Similar to FPDFBitmap_Create function, with more formats and external buffer supported. 
//			Bitmap created by this function can be used in any place that a FPDF_BITMAP handle is 
//			required. 
//
//			If external scanline buffer is used, then the application should destroy the buffer
//			by itself. FPDFBitmap_Destroy function will not destroy the buffer.
//
DLLEXPORT FPDF_BITMAP STDCALL FPDFBitmap_CreateEx(int width, int height, int format, void* first_scan, int stride);

// Function: FPDFBitmap_FillRect
//			Fill a rectangle area in an FXDIB.
// Parameters:
//			bitmap		-	The handle to the bitmap. Returned by FPDFBitmap_Create function.
//			left		-	The left side position. Starting from 0 at the left-most pixel.
//			top			-	The top side position. Starting from 0 at the top-most scan line.
//			width		-	Number of pixels to be filled in each scan line.
//			height		-	Number of scan lines to be filled.
//			color		-	A 32-bit value specifing the color, in 8888 ARGB format.
// Return value:
//			None.
// Comments:
//			This function set the color and (optionally) alpha value in specified region of the bitmap.
//			NOTE: If alpha channel is used, this function does NOT composite the background with the source color,
//			instead the background will be replaced by the source color and alpha.
//			If alpha channel is not used, the "alpha" parameter is ignored.
//
DLLEXPORT void STDCALL FPDFBitmap_FillRect(FPDF_BITMAP bitmap, int left, int top, int width, int height, FPDF_DWORD color);

// Function: FPDFBitmap_GetBuffer
//			Get data buffer of an FXDIB
// Parameters:
//			bitmap		-	Handle to the bitmap. Returned by FPDFBitmap_Create function.
// Return value:
//			The pointer to the first byte of the bitmap buffer.
// Comments:
//			The stride may be more than width * number of bytes per pixel
//			Applications can use this function to get the bitmap buffer pointer, then manipulate any color
//			and/or alpha values for any pixels in the bitmap.
DLLEXPORT void* STDCALL FPDFBitmap_GetBuffer(FPDF_BITMAP bitmap);

// Function: FPDFBitmap_GetWidth
//			Get width of an FXDIB.
// Parameters:
//			bitmap		-	Handle to the bitmap. Returned by FPDFBitmap_Create function.
// Return value:
//			The number of pixels in a horizontal line of the bitmap.
DLLEXPORT int STDCALL FPDFBitmap_GetWidth(FPDF_BITMAP bitmap);

// Function: FPDFBitmap_GetHeight
//			Get height of an FXDIB.
// Parameters:
//			bitmap		-	Handle to the bitmap. Returned by FPDFBitmap_Create function.
// Return value:
//			The number of pixels in a vertical line of the bitmap.
DLLEXPORT int STDCALL FPDFBitmap_GetHeight(FPDF_BITMAP bitmap);

// Function: FPDFBitmap_GetStride
//			Get number of bytes for each scan line in the bitmap buffer.
// Parameters:
//			bitmap		-	Handle to the bitmap. Returned by FPDFBitmap_Create function.
// Return value:
//			The number of bytes for each scan line in the bitmap buffer.
// Comments:
//			The stride may be more than width * number of bytes per pixel
DLLEXPORT int STDCALL FPDFBitmap_GetStride(FPDF_BITMAP bitmap);

// Function: FPDFBitmap_Destroy
//			Destroy an FXDIB and release all related buffers. 
// Parameters:
//			bitmap		-	Handle to the bitmap. Returned by FPDFBitmap_Create function.
// Return value:
//			None.
// Comments:
//			This function will not destroy any external buffer.
//
DLLEXPORT void STDCALL FPDFBitmap_Destroy(FPDF_BITMAP bitmap);

// Function: FPDF_VIEWERREF_GetPrintScaling
//			Whether the PDF document prefers to be scaled or not.
// Parameters: 
//			document	-	Handle to the loaded document.
// Return value:
//			None.
//
DLLEXPORT FPDF_BOOL STDCALL FPDF_VIEWERREF_GetPrintScaling(FPDF_DOCUMENT document);

// Function: FPDF_VIEWERREF_GetNumCopies
//			Returns the number of copies to be printed.
// Parameters:
//			document	-	Handle to the loaded document.
// Return value:
//          The number of copies to be printed.
//
DLLEXPORT int STDCALL FPDF_VIEWERREF_GetNumCopies(FPDF_DOCUMENT document);

// Function: FPDF_VIEWERREF_GetPrintPageRange
//			Page numbers to initialize print dialog box when file is printed.
// Parameters:
//			document	-	Handle to the loaded document.
// Return value:
//          The print page range to be used for printing.
//
DLLEXPORT FPDF_PAGERANGE STDCALL FPDF_VIEWERREF_GetPrintPageRange(FPDF_DOCUMENT document);

// Function: FPDF_VIEWERREF_GetDuplex
//			Returns the paper handling option to be used when printing from print dialog.
// Parameters:
//			document	-	Handle to the loaded document.
// Return value:
//          The paper handling option to be used when printing.
//
DLLEXPORT FPDF_DUPLEXTYPE STDCALL FPDF_VIEWERREF_GetDuplex(FPDF_DOCUMENT document);

// Function: FPDF_CountNamedDests
//			Get the count of named destinations in the PDF document.
// Parameters:
//			document	-	Handle to a document
// Return value:
//			The count of named destinations.
DLLEXPORT FPDF_DWORD STDCALL FPDF_CountNamedDests(FPDF_DOCUMENT document);

// Function: FPDF_GetNamedDestByName
//			get a special dest handle by the index.
// Parameters: 
//			document	-	Handle to the loaded document.
//			name		-	The name of a special named dest.
// Return value:
//			The handle of the dest.
//
DLLEXPORT FPDF_DEST STDCALL FPDF_GetNamedDestByName(FPDF_DOCUMENT document,FPDF_BYTESTRING name);

// Function: FPDF_GetNamedDest
//			Get the specified named destinations of the PDF document by index.
// Parameters:
//			document        -	Handle to a document
//			index           -	The index of named destination.
//			buffer          -	The buffer to obtain destination name, used as wchar_t*.
//			buflen [in/out]	-	Size of the buffer in bytes on input, length of the result in bytes on output or -1 if the buffer is too small.
// Return value:
//			The destination handle of a named destination, or NULL if no named destination corresponding to |index|.
// Comments:
//			Call this function twice to get the name of the named destination:
//			1) First time pass in |buffer| as NULL and get buflen.
//			2) Second time pass in allocated |buffer| and buflen to retrieve |buffer|, which should be used as wchar_t*.
//			   If buflen is not sufficiently large, it will be set to -1 upon return.
//
DLLEXPORT FPDF_DEST STDCALL FPDF_GetNamedDest(FPDF_DOCUMENT document, int index, void* buffer, long& buflen);


#ifdef __cplusplus
};
#endif

#endif // _FPDFVIEW_H_
