// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef PUBLIC_FPDF_TRANSFORMPAGE_H_
#define PUBLIC_FPDF_TRANSFORMPAGE_H_

// NOLINTNEXTLINE(build/include)
#include "fpdfview.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void* FPDF_PAGEARCSAVER;
typedef void* FPDF_PAGEARCLOADER;

/**
 * Set "MediaBox" entry to the page dictionary.
 *
 * page   - Handle to a page.
 * left   - The left of the rectangle.
 * bottom - The bottom of the rectangle.
 * right  - The right of the rectangle.
 * top    - The top of the rectangle.
 */
FPDF_EXPORT void FPDF_CALLCONV FPDFPage_SetMediaBox(FPDF_PAGE page,
                                                    float left,
                                                    float bottom,
                                                    float right,
                                                    float top);

/**
 * Set "CropBox" entry to the page dictionary.
 *
 * page   - Handle to a page.
 * left   - The left of the rectangle.
 * bottom - The bottom of the rectangle.
 * right  - The right of the rectangle.
 * top    - The top of the rectangle.
 */
FPDF_EXPORT void FPDF_CALLCONV FPDFPage_SetCropBox(FPDF_PAGE page,
                                                   float left,
                                                   float bottom,
                                                   float right,
                                                   float top);

/**
 * Get "MediaBox" entry from the page dictionary.
 *
 * page   - Handle to a page.
 * left   - Pointer to a float value receiving the left of the rectangle.
 * bottom - Pointer to a float value receiving the bottom of the rectangle.
 * right  - Pointer to a float value receiving the right of the rectangle.
 * top    - Pointer to a float value receiving the top of the rectangle.
 *
 * On success, return true and write to the out parameters. Otherwise return
 * false and leave the out parameters unmodified.
 */
FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV FPDFPage_GetMediaBox(FPDF_PAGE page,
                                                         float* left,
                                                         float* bottom,
                                                         float* right,
                                                         float* top);

/**
 * Get "CropBox" entry from the page dictionary.
 *
 * page   - Handle to a page.
 * left   - Pointer to a float value receiving the left of the rectangle.
 * bottom - Pointer to a float value receiving the bottom of the rectangle.
 * right  - Pointer to a float value receiving the right of the rectangle.
 * top    - Pointer to a float value receiving the top of the rectangle.
 *
 * On success, return true and write to the out parameters. Otherwise return
 * false and leave the out parameters unmodified.
 */
FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV FPDFPage_GetCropBox(FPDF_PAGE page,
                                                        float* left,
                                                        float* bottom,
                                                        float* right,
                                                        float* top);

/**
 * Apply transforms to |page|.
 *
 * If |matrix| is provided it will be applied to transform the page.
 * If |clipRect| is provided it will be used to clip the resulting page.
 * If neither |matrix| or |clipRect| are provided this method returns |false|.
 * Returns |true| if transforms are applied.
 *
 * This function will transform the whole page, and would take effect to all the
 * objects in the page.
 *
 * page        - Page handle.
 * matrix      - Transform matrix.
 * clipRect    - Clipping rectangle.
 */
FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDFPage_TransFormWithClip(FPDF_PAGE page,
                           FS_MATRIX* matrix,
                           FS_RECTF* clipRect);

/**
 * Transform (scale, rotate, shear, move) the clip path of page object.
 * page_object - Handle to a page object. Returned by
 * FPDFPageObj_NewImageObj().
 *
 * a  - The coefficient "a" of the matrix.
 * b  - The coefficient "b" of the matrix.
 * c  - The coefficient "c" of the matrix.
 * d  - The coefficient "d" of the matrix.
 * e  - The coefficient "e" of the matrix.
 * f  - The coefficient "f" of the matrix.
 */
FPDF_EXPORT void FPDF_CALLCONV
FPDFPageObj_TransformClipPath(FPDF_PAGEOBJECT page_object,
                              double a,
                              double b,
                              double c,
                              double d,
                              double e,
                              double f);

/**
 * Create a new clip path, with a rectangle inserted.
 *
 * left   - The left of the clip box.
 * bottom - The bottom of the clip box.
 * right  - The right of the clip box.
 * top    - The top of the clip box.
 */
FPDF_EXPORT FPDF_CLIPPATH FPDF_CALLCONV FPDF_CreateClipPath(float left,
                                                            float bottom,
                                                            float right,
                                                            float top);

/**
 * Destroy the clip path.
 *
 * clipPath - A handle to the clip path.
 */
FPDF_EXPORT void FPDF_CALLCONV FPDF_DestroyClipPath(FPDF_CLIPPATH clipPath);

/**
 * Clip the page content, the page content that outside the clipping region
 * become invisible.
 *
 * A clip path will be inserted before the page content stream or content array.
 * In this way, the page content will be clipped by this clip path.
 *
 * page        - A page handle.
 * clipPath    - A handle to the clip path.
 */
FPDF_EXPORT void FPDF_CALLCONV FPDFPage_InsertClipPath(FPDF_PAGE page,
                                                       FPDF_CLIPPATH clipPath);

#ifdef __cplusplus
}
#endif

#endif  // PUBLIC_FPDF_TRANSFORMPAGE_H_
