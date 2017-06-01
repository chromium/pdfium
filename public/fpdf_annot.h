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

// Get the number of annotations in |page|.
//
//   page   - handle to a page.
//
// Returns the number of annotations in |page|.
DLLEXPORT int STDCALL FPDFPage_GetAnnotCount(FPDF_PAGE page);

// Get annotation in |page| at |index|.
//
//   page  - handle to a page.
//   index - the index of the annotation.
//   annot - receives the annotation
//
// Returns true if successful, false otherwise.
DLLEXPORT FPDF_BOOL STDCALL FPDFPage_GetAnnot(FPDF_PAGE page,
                                              int index,
                                              FPDF_ANNOTATION* annot);

// Get the subtype of an annotation.
//
//   annot  - handle to an annotation.
//
// Returns the annotation subtype.
DLLEXPORT FPDF_ANNOTATION_SUBTYPE STDCALL
FPDFAnnot_GetSubtype(FPDF_ANNOTATION annot);

typedef enum FPDFANNOT_COLORTYPE {
  FPDFANNOT_COLORTYPE_Color = 0,
  FPDFANNOT_COLORTYPE_InteriorColor
} FPDFANNOT_COLORTYPE;

// Get the color of an annotation. If no color is specified, default to yellow
// for highlight annotation, black for all else.
//
//   annot  - handle to an annotation.
//   type   - type of the color requested. Default to Color.
//   R, G, B  - buffer to hold the RGB value of the color. Ranges from 0 to 255.
//   A      - buffer to hold the opacity. Ranges from 0 to 255.
//
// Returns true if successful, false otherwise.
DLLEXPORT FPDF_BOOL STDCALL FPDFAnnot_GetColor(FPDF_ANNOTATION annot,
                                               FPDFANNOT_COLORTYPE type,
                                               unsigned int* R,
                                               unsigned int* G,
                                               unsigned int* B,
                                               unsigned int* A);

// Check if the annotation is of a type that has attachment points
// (i.e. quadpoints). Quadpoints are the vertices of the rectange that
// encompasses the texts affected by the annotation. They provide the
// coordinates in the page where the annotation is attached. Only markup
// annotations (i.e. highlight, strikeout, squiggly, underline, and link) have
// quadpoints.
//
//   annot  - handle to an annotation.
//
// Returns true if the annotation is of a type that has quadpoints, false
// otherwise.
DLLEXPORT FPDF_BOOL STDCALL
FPDFAnnot_HasAttachmentPoints(FPDF_ANNOTATION annot);

// Get the attachment points (i.e. quadpoints) of an annotation.
//
//   annot  - handle to an annotation.
//   quadPoints - receives the attachment points
//
// Returns true if successful, false otherwise.
DLLEXPORT FPDF_BOOL STDCALL
FPDFAnnot_GetAttachmentPoints(FPDF_ANNOTATION annot,
                              FS_QUADPOINTSF* quadPoints);

// Get the annotation rectangle defining the location of the annotation.
//
//   annot  - handle to an annotation.
//   rect   - receives the annotation rectangle
//
// Returns true if successful, false otherwise.
DLLEXPORT FPDF_BOOL STDCALL FPDFAnnot_GetRect(FPDF_ANNOTATION annot,
                                              FS_RECTF* rect);

typedef enum FPDFANNOT_TEXTTYPE {
  FPDFANNOT_TEXTTYPE_Contents = 0,
  FPDFANNOT_TEXTTYPE_Author
} FPDFANNOT_TEXTTYPE;

// Get the contents of an annotation. |buffer| is only modified if |buflen|
// is longer than the length of contents.
//
//   annot  - handle to an annotation.
//   type   - type of the text requested. Default to Contents.
//   buffer - buffer for holding the contents string, encoded in UTF16-LE.
//   buflen - length of the buffer.
//
// Returns the length of the contents.

DLLEXPORT unsigned long STDCALL FPDFAnnot_GetText(FPDF_ANNOTATION annot,
                                                  FPDFANNOT_TEXTTYPE type,
                                                  char* buffer,
                                                  unsigned long buflen);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // PUBLIC_FPDF_ANNOT_H_
