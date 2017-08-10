// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PUBLIC_FPDF_ANNOT_H_
#define PUBLIC_FPDF_ANNOT_H_

// NOLINTNEXTLINE(build/include)
#include "fpdfview.h"

#include "fpdf_doc.h"
#include "fpdf_formfill.h"

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

#define FPDF_OBJECT_UNKNOWN 0
#define FPDF_OBJECT_BOOLEAN 1
#define FPDF_OBJECT_NUMBER 2
#define FPDF_OBJECT_STRING 3
#define FPDF_OBJECT_NAME 4
#define FPDF_OBJECT_ARRAY 5
#define FPDF_OBJECT_DICTIONARY 6
#define FPDF_OBJECT_STREAM 7
#define FPDF_OBJECT_NULLOBJ 8
#define FPDF_OBJECT_REFERENCE 9

// Refer to PDF Reference version 1.7 table 8.70 for field flags common to all
// interactive form field types.
#define FPDF_FORMFLAG_NONE 0
#define FPDF_FORMFLAG_READONLY (1 << 0)
#define FPDF_FORMFLAG_REQUIRED (1 << 1)
#define FPDF_FORMFLAG_NOEXPORT (1 << 2)

// Refer to PDF Reference version 1.7 table 8.77 for field flags specific to
// interactive form text fields.
#define FPDF_FORMFLAG_TEXT_MULTILINE (1 << 12)

// Refer to PDF Reference version 1.7 table 8.79 for field flags specific to
// interactive form choice fields.
#define FPDF_FORMFLAG_CHOICE_COMBO (1 << 17)
#define FPDF_FORMFLAG_CHOICE_EDIT (1 << 18)

typedef enum FPDFANNOT_COLORTYPE {
  FPDFANNOT_COLORTYPE_Color = 0,
  FPDFANNOT_COLORTYPE_InteriorColor
} FPDFANNOT_COLORTYPE;

// Experimental API.
// Check if an annotation subtype is currently supported for creation.
// Currently supported subtypes: circle, highlight, ink, popup, square,
// squiggly, stamp, strikeout, text, and underline.
//
//   subtype   - the subtype to be checked.
//
// Returns true if this subtype supported.
DLLEXPORT FPDF_BOOL STDCALL
FPDFAnnot_IsSupportedSubtype(FPDF_ANNOTATION_SUBTYPE subtype);

// Experimental API.
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

// Experimental API.
// Get the number of annotations in |page|.
//
//   page   - handle to a page.
//
// Returns the number of annotations in |page|.
DLLEXPORT int STDCALL FPDFPage_GetAnnotCount(FPDF_PAGE page);

// Experimental API.
// Get annotation in |page| at |index|. Must call FPDFPage_CloseAnnot() when the
// annotation returned by this function is no longer needed.
//
//   page  - handle to a page.
//   index - the index of the annotation.
//
// Returns a handle to the annotation object, or NULL on failure.
DLLEXPORT FPDF_ANNOTATION STDCALL FPDFPage_GetAnnot(FPDF_PAGE page, int index);

// Experimental API.
// Close an annotation. Must be called when the annotation returned by
// FPDFPage_CreateAnnot() or FPDFPage_GetAnnot() is no longer needed. This
// function does not remove the annotation from the document.
//
//   annot  - handle to an annotation.
DLLEXPORT void STDCALL FPDFPage_CloseAnnot(FPDF_ANNOTATION annot);

// Experimental API.
// Remove the annotation in |page| at |index|.
//
//   page  - handle to a page.
//   index - the index of the annotation.
//
// Returns true if successful.
DLLEXPORT FPDF_BOOL STDCALL FPDFPage_RemoveAnnot(FPDF_PAGE page, int index);

// Experimental API.
// Get the subtype of an annotation.
//
//   annot  - handle to an annotation.
//
// Returns the annotation subtype.
DLLEXPORT FPDF_ANNOTATION_SUBTYPE STDCALL
FPDFAnnot_GetSubtype(FPDF_ANNOTATION annot);

// Experimental API.
// Check if an annotation subtype is currently supported for object extraction,
// update, and removal.
// Currently supported subtypes: ink and stamp.
//
//   subtype   - the subtype to be checked.
//
// Returns true if this subtype supported.
DLLEXPORT FPDF_BOOL STDCALL
FPDFAnnot_IsObjectSupportedSubtype(FPDF_ANNOTATION_SUBTYPE subtype);

// Experimental API.
// Update |obj| in |annot|. |obj| must be in |annot| already and must have
// been retrieved by FPDFAnnot_GetObject(). Currently, only ink and stamp
// annotations are supported by this API. Also note that only path, image, and
// text objects have APIs for modification; see FPDFPath_*(), FPDFText_*(), and
// FPDFImageObj_*().
//
//   annot  - handle to an annotation.
//   obj    - handle to the object that |annot| needs to update.
//
// Return true if successful.
DLLEXPORT FPDF_BOOL STDCALL FPDFAnnot_UpdateObject(FPDF_ANNOTATION annot,
                                                   FPDF_PAGEOBJECT obj);

// Experimental API.
// Add |obj| to |annot|. |obj| must have been created by
// FPDFPageObj_CreateNew{Path|Rect}() or FPDFPageObj_New{Text|Image}Obj(), and
// will be owned by |annot|. Note that an |obj| cannot belong to more than one
// |annot|. Currently, only ink and stamp annotations are supported by this API.
// Also note that only path, image, and text objects have APIs for creation.
//
//   annot  - handle to an annotation.
//   obj    - handle to the object that is to be added to |annot|.
//
// Return true if successful.
DLLEXPORT FPDF_BOOL STDCALL FPDFAnnot_AppendObject(FPDF_ANNOTATION annot,
                                                   FPDF_PAGEOBJECT obj);

// Experimental API.
// Get the total number of objects in |annot|, including path objects, text
// objects, external objects, image objects, and shading objects.
//
//   annot  - handle to an annotation.
//
// Returns the number of objects in |annot|.
DLLEXPORT int STDCALL FPDFAnnot_GetObjectCount(FPDF_ANNOTATION annot);

// Experimental API.
// Get the object in |annot| at |index|.
//
//   annot  - handle to an annotation.
//   index  - the index of the object.
//
// Return a handle to the object, or NULL on failure.
DLLEXPORT FPDF_PAGEOBJECT STDCALL FPDFAnnot_GetObject(FPDF_ANNOTATION annot,
                                                      int index);

// Experimental API.
// Remove the object in |annot| at |index|.
//
//   annot  - handle to an annotation.
//   index  - the index of the object to be removed.
//
// Return true if successful.
DLLEXPORT FPDF_BOOL STDCALL FPDFAnnot_RemoveObject(FPDF_ANNOTATION annot,
                                                   int index);

// Experimental API.
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

// Experimental API.
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

// Experimental API.
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

// Experimental API.
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

// Experimental API.
// Get the attachment points (i.e. quadpoints) of an annotation. If the
// annotation's appearance stream is defined and this annotation is of a type
// with quadpoints, then return the bounding box it specifies instead.
//
//   annot      - handle to an annotation.
//
// Returns a quadpoints object, or an empty set of quadpoints on failure.
DLLEXPORT FS_QUADPOINTSF STDCALL
FPDFAnnot_GetAttachmentPoints(FPDF_ANNOTATION annot);

// Experimental API.
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

// Experimental API.
// Get the annotation rectangle defining the location of the annotation. If the
// annotation's appearance stream is defined and this annotation is of a type
// without quadpoints, then return the bounding box it specifies instead.
//
//   annot  - handle to an annotation.
//
// Returns a rectangle object, or an empty rectangle on failure.
DLLEXPORT FS_RECTF STDCALL FPDFAnnot_GetRect(FPDF_ANNOTATION annot);

// Experimental API.
// Check if |annot|'s dictionary has |key| as a key.
//
//   annot  - handle to an annotation.
//   key    - the key to look for.
//
// Returns true if |key| exists.
DLLEXPORT FPDF_BOOL STDCALL FPDFAnnot_HasKey(FPDF_ANNOTATION annot,
                                             FPDF_WIDESTRING key);

// Experimental API.
// Get the type of the value corresponding to |key| in |annot|'s dictionary.
//
//   annot  - handle to an annotation.
//   key    - the key to look for.
//
// Returns the type of the dictionary value.
DLLEXPORT FPDF_OBJECT_TYPE STDCALL FPDFAnnot_GetValueType(FPDF_ANNOTATION annot,
                                                          FPDF_WIDESTRING key);

// Experimental API.
// Set the string value corresponding to |key| in |annot|'s dictionary,
// overwriting the existing value if any. The value type would be
// FPDF_OBJECT_STRING after this function call succeeds.
//
//   annot  - handle to an annotation.
//   key    - the key to the dictionary entry to be set, encoded in UTF16-LE.
//   value  - the string value to be set, encoded in UTF16-LE.
//
// Returns true if successful.
DLLEXPORT FPDF_BOOL STDCALL FPDFAnnot_SetStringValue(FPDF_ANNOTATION annot,
                                                     FPDF_WIDESTRING key,
                                                     FPDF_WIDESTRING value);

// Experimental API.
// Get the string value corresponding to |key| in |annot|'s dictionary. |buffer|
// is only modified if |buflen| is longer than the length of contents. Note that
// if |key| does not exist in the dictionary or if |key|'s corresponding value
// in the dictionary is not a string (i.e. the value is not of type
// FPDF_OBJECT_STRING or FPDF_OBJECT_NAME), then an empty string would be copied
// to |buffer| and the return value would be 2. On other errors, nothing would
// be added to |buffer| and the return value would be 0.
//
//   annot  - handle to an annotation.
//   key    - the key to the requested dictionary entry.
//   buffer - buffer for holding the value string, encoded in UTF16-LE.
//   buflen - length of the buffer.
//
// Returns the length of the string value.
DLLEXPORT unsigned long STDCALL FPDFAnnot_GetStringValue(FPDF_ANNOTATION annot,
                                                         FPDF_WIDESTRING key,
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

// Experimental API.
// Get the annotation flags of |annot|, which is an interactive form
// annotation in |page|.
//
//   page     - handle to a page.
//   annot    - handle to an interactive form annotation.
//
// Returns the annotation flags specific to interactive forms.
DLLEXPORT int STDCALL FPDFAnnot_GetFormFieldFlags(FPDF_PAGE page,
                                                  FPDF_ANNOTATION annot);

// Experimental API.
// Retrieves an interactive form annotation whose rectangle contains a given
// point on a page. Must call FPDFPage_CloseAnnot() when the annotation returned
// is no longer needed.
//
//
//    hHandle     -   handle to the form fill module, returned by
//                    FPDFDOC_InitFormFillEnvironment.
//    page        -   handle to the page, returned by FPDF_LoadPage function.
//    page_x      -   X position in PDF "user space".
//    page_y      -   Y position in PDF "user space".
//
// Returns the interactive form annotation whose rectangle contains the given
// coordinates on the page. If there is no such annotation, return NULL.
DLLEXPORT FPDF_ANNOTATION STDCALL
FPDFAnnot_GetFormFieldAtPoint(FPDF_FORMHANDLE hHandle,
                              FPDF_PAGE page,
                              double page_x,
                              double page_y);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // PUBLIC_FPDF_ANNOT_H_
