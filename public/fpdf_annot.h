// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef PUBLIC_FPDF_ANNOT_H_
#define PUBLIC_FPDF_ANNOT_H_

// NOLINTNEXTLINE(build/include)
#include "fpdfview.h"

#include "public/fpdf_doc.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#define FPDF_ANNOT_UNKNOWN 0
#define FPDF_ANNOT_TEXT 1
#define FPDF_ANNOT_LINK 2
#define FPDF_ANNOT_FREETEXT 3
#define FPDF_ANNOT_LINE 4
#define FPDF_ANNOT_SQUARE 5
#define FPDF_ANNOT_CIRCLE 6
#define FPDF_ANNOT_POLYGON 7
#define FPDF_ANNOT_POLYLINE 8
#define FPDF_ANNOT_HIGHLIGHT 9
#define FPDF_ANNOT_UNDERLINE 10
#define FPDF_ANNOT_SQUIGGLY 11
#define FPDF_ANNOT_STRIKEOUT 12
#define FPDF_ANNOT_STAMP 13
#define FPDF_ANNOT_CARET 14
#define FPDF_ANNOT_INK 15
#define FPDF_ANNOT_POPUP 16
#define FPDF_ANNOT_FILEATTACHMENT 17
#define FPDF_ANNOT_SOUND 18
#define FPDF_ANNOT_MOVIE 19
#define FPDF_ANNOT_WIDGET 20
#define FPDF_ANNOT_SCREEN 21
#define FPDF_ANNOT_PRINTERMARK 22
#define FPDF_ANNOT_TRAPNET 23
#define FPDF_ANNOT_WATERMARK 24
#define FPDF_ANNOT_THREED 25
#define FPDF_ANNOT_RICHMEDIA 26
#define FPDF_ANNOT_XFAWIDGET 27

// Refer to PDF Reference (6th edition) table 8.16 for all annotation flags.
#define FPDF_ANNOT_FLAG_NONE 0
#define FPDF_ANNOT_FLAG_INVISIBLE (1 << 0)
#define FPDF_ANNOT_FLAG_HIDDEN (1 << 1)
#define FPDF_ANNOT_FLAG_PRINT (1 << 2)
#define FPDF_ANNOT_FLAG_NOZOOM (1 << 3)
#define FPDF_ANNOT_FLAG_NOROTATE (1 << 4)
#define FPDF_ANNOT_FLAG_NOVIEW (1 << 5)
#define FPDF_ANNOT_FLAG_READONLY (1 << 6)
#define FPDF_ANNOT_FLAG_LOCKED (1 << 7)
#define FPDF_ANNOT_FLAG_TOGGLENOVIEW (1 << 8)

typedef enum FPDFANNOT_COLORTYPE {
  FPDFANNOT_COLORTYPE_Color = 0,
  FPDFANNOT_COLORTYPE_InteriorColor
} FPDFANNOT_COLORTYPE;

typedef enum FPDFANNOT_TEXTTYPE {
  FPDFANNOT_TEXTTYPE_Contents = 0,
  FPDFANNOT_TEXTTYPE_Author
} FPDFANNOT_TEXTTYPE;

// Check if an annotation subtype is currently supported for creation.
// Currently supported subtypes: circle, highlight, ink, popup, square,
// squiggly, stamp, strikeout, text, and underline.
//
//   subtype   - the subtype to be checked.
//
// Returns true if this subtype supported.
DLLEXPORT FPDF_BOOL STDCALL
FPDFAnnot_IsSupportedSubtype(FPDF_ANNOTATION_SUBTYPE subtype);

// Create an annotation in |page| of the subtype |subtype|. If the specified
// subtype is illegal or unsupported, then a new annotation will not be created.
// Must call FPDFPage_CloseAnnot() when the annotation returned by this
// function is no longer needed.
//
//   page      - handle to a page.
//   subtype   - the subtype of the new annotation.
//
// Returns a handle to the new annotation object, or NULL on failure.
DLLEXPORT FPDF_ANNOTATION STDCALL
FPDFPage_CreateAnnot(FPDF_PAGE page, FPDF_ANNOTATION_SUBTYPE subtype);

// Get the number of annotations in |page|.
//
//   page   - handle to a page.
//
// Returns the number of annotations in |page|.
DLLEXPORT int STDCALL FPDFPage_GetAnnotCount(FPDF_PAGE page);

// Get annotation in |page| at |index|. Must call FPDFPage_CloseAnnot() when the
// annotation returned by this function is no longer needed.
//
//   page  - handle to a page.
//   index - the index of the annotation.
//
// Returns a handle to the annotation object, or NULL on failure.
DLLEXPORT FPDF_ANNOTATION STDCALL FPDFPage_GetAnnot(FPDF_PAGE page, int index);

// Close an annotation. Must be called when the annotation returned by
// FPDFPage_CreateAnnot() or FPDFPage_GetAnnot() is no longer needed. This
// function does not remove the annotation from the document.
//
//   annot  - handle to an annotation.
DLLEXPORT void STDCALL FPDFPage_CloseAnnot(FPDF_ANNOTATION annot);

// Remove the annotation in |page| at |index|.
//
//   page  - handle to a page.
//   index - the index of the annotation.
//
// Returns true if successful.
DLLEXPORT FPDF_BOOL STDCALL FPDFPage_RemoveAnnot(FPDF_PAGE page, int index);

// Get the subtype of an annotation.
//
//   annot  - handle to an annotation.
//
// Returns the annotation subtype.
DLLEXPORT FPDF_ANNOTATION_SUBTYPE STDCALL
FPDFAnnot_GetSubtype(FPDF_ANNOTATION annot);

// Experimental API.
// Update |path| in |annot|. |path| must be in |annot| already and must have
// been retrieved by FPDFAnnot_GetPathObject(). Only ink and stamp annotations
// are supported currently.
//
//   annot  - handle to an annotation.
//   path   - handle to the path that |annot| needs to update.
//
// Return true if successful.
DLLEXPORT FPDF_BOOL STDCALL FPDFAnnot_UpdatePathObject(FPDF_ANNOTATION annot,
                                                       FPDF_PAGEOBJECT path);

// Experimental API.
// Add |path| to |annot|. |path| must have been created by
// FPDFPageObj_CreateNewPath(), and will be owned by |annot|. Note that a |path|
// cannot belong to more than one |annot|. Only ink and stamp annotations
// are supported currently.
//
//   annot  - handle to an annotation.
//   path   - handle to the path that is to be added to |annot|.
//
// Return true if successful.
DLLEXPORT FPDF_BOOL STDCALL FPDFAnnot_AppendPathObject(FPDF_ANNOTATION annot,
                                                       FPDF_PAGEOBJECT path);

// Experimental API.
// Get the number of path objects in |annot|.
//
//   annot  - handle to an annotation.
//
// Returns the number of path objects in |annot|.
DLLEXPORT int STDCALL FPDFAnnot_GetPathObjectCount(FPDF_ANNOTATION annot);

// Experimental API.
// Get the path object in |annot| at |index|.
//
//   annot  - handle to an annotation.
//   index  - the index of the path object.
//
// Return a handle to the path object, or NULL on failure.
DLLEXPORT FPDF_PAGEOBJECT STDCALL FPDFAnnot_GetPathObject(FPDF_ANNOTATION annot,
                                                          int index);

// Set the color of an annotation. Fails when called on annotations with
// appearance streams already defined; instead use
// FPDFPath_Set{Stroke|Fill}Color().
//
//   annot    - handle to an annotation.
//   type     - type of the color to be set.
//   R, G, B  - buffer to hold the RGB value of the color. Ranges from 0 to 255.
//   A        - buffer to hold the opacity. Ranges from 0 to 255.
//
// Returns true if successful.
DLLEXPORT FPDF_BOOL STDCALL FPDFAnnot_SetColor(FPDF_ANNOTATION annot,
                                               FPDFANNOT_COLORTYPE type,
                                               unsigned int R,
                                               unsigned int G,
                                               unsigned int B,
                                               unsigned int A);

// Get the color of an annotation. If no color is specified, default to yellow
// for highlight annotation, black for all else. Fails when called on
// annotations with appearance streams already defined; instead use
// FPDFPath_Get{Stroke|Fill}Color().
//
//   annot    - handle to an annotation.
//   type     - type of the color requested.
//   R, G, B  - buffer to hold the RGB value of the color. Ranges from 0 to 255.
//   A        - buffer to hold the opacity. Ranges from 0 to 255.
//
// Returns true if successful.
DLLEXPORT FPDF_BOOL STDCALL FPDFAnnot_GetColor(FPDF_ANNOTATION annot,
                                               FPDFANNOT_COLORTYPE type,
                                               unsigned int* R,
                                               unsigned int* G,
                                               unsigned int* B,
                                               unsigned int* A);

// Check if the annotation is of a type that has attachment points
// (i.e. quadpoints). Quadpoints are the vertices of the rectange that
// encompasses the texts affected by the annotation. They provide the
// coordinates in the page where the annotation is attached. Only text markup
// annotations (i.e. highlight, strikeout, squiggly, and underline) and link
// annotations have quadpoints.
//
//   annot  - handle to an annotation.
//
// Returns true if the annotation is of a type that has quadpoints, false
// otherwise.
DLLEXPORT FPDF_BOOL STDCALL
FPDFAnnot_HasAttachmentPoints(FPDF_ANNOTATION annot);

// Set the attachment points (i.e. quadpoints) of an annotation. If the
// annotation's appearance stream is defined and this annotation is of a type
// with quadpoints, then update the bounding box too.
//
//   annot      - handle to an annotation.
//   quadPoints - the quadpoints to be set.
//
// Returns true if successful.
DLLEXPORT FPDF_BOOL STDCALL
FPDFAnnot_SetAttachmentPoints(FPDF_ANNOTATION annot,
                              const FS_QUADPOINTSF* quadPoints);

// Get the attachment points (i.e. quadpoints) of an annotation. If the
// annotation's appearance stream is defined and this annotation is of a type
// with quadpoints, then return the bounding box it specifies instead.
//
//   annot      - handle to an annotation.
//
// Returns a quadpoints object, or an empty set of quadpoints on failure.
DLLEXPORT FS_QUADPOINTSF STDCALL
FPDFAnnot_GetAttachmentPoints(FPDF_ANNOTATION annot);

// Set the annotation rectangle defining the location of the annotation. If the
// annotation's appearance stream is defined and this annotation is of a type
// without quadpoints, then update the bounding box too.
//
//   annot  - handle to an annotation.
//   rect   - the annotation rectangle to be set.
//
// Returns true if successful.
DLLEXPORT FPDF_BOOL STDCALL FPDFAnnot_SetRect(FPDF_ANNOTATION annot,
                                              const FS_RECTF* rect);

// Get the annotation rectangle defining the location of the annotation. If the
// annotation's appearance stream is defined and this annotation is of a type
// without quadpoints, then return the bounding box it specifies instead.
//
//   annot  - handle to an annotation.
//
// Returns a rectangle object, or an empty rectangle on failure.
DLLEXPORT FS_RECTF STDCALL FPDFAnnot_GetRect(FPDF_ANNOTATION annot);

// Set the contents of an annotation.
//
//   annot  - handle to an annotation.
//   type   - type of the text to be set.
//   text   - the text to be set.
//
// Returns true if successful.
DLLEXPORT FPDF_BOOL STDCALL FPDFAnnot_SetText(FPDF_ANNOTATION annot,
                                              FPDFANNOT_TEXTTYPE type,
                                              FPDF_WIDESTRING text);

// Get the contents of an annotation. |buffer| is only modified if |buflen|
// is longer than the length of contents.
//
//   annot  - handle to an annotation.
//   type   - type of the text requested.
//   buffer - buffer for holding the contents string, encoded in UTF16-LE.
//   buflen - length of the buffer.
//
// Returns the length of the contents.
DLLEXPORT unsigned long STDCALL FPDFAnnot_GetText(FPDF_ANNOTATION annot,
                                                  FPDFANNOT_TEXTTYPE type,
                                                  void* buffer,
                                                  unsigned long buflen);

// Experimental API.
// Get the annotation flags of |annot|.
//
//   annot    - handle to an annotation.
//
// Returns the annotation flags.
DLLEXPORT int STDCALL FPDFAnnot_GetFlags(FPDF_ANNOTATION annot);

// Experimental API.
// Set the |annot|'s flags to be of the value |flags|.
//
//   annot      - handle to an annotation.
//   flags      - the flag values to be set.
//
// Returns true if successful.
DLLEXPORT FPDF_BOOL STDCALL FPDFAnnot_SetFlags(FPDF_ANNOTATION annot,
                                               int flags);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // PUBLIC_FPDF_ANNOT_H_
