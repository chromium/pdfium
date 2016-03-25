// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/fpdf_page/pageint.h"

#include <algorithm>

#include "core/fpdfapi/fpdf_page/cpdf_meshstream.h"
#include "core/fpdfapi/fpdf_page/cpdf_shadingpattern.h"
#include "core/fpdfapi/fpdf_page/include/cpdf_form.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_array.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_dictionary.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_document.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_stream.h"

namespace {

const int kSingleCoordinatePair = 1;
const int kTensorCoordinatePairs = 16;
const int kCoonsCoordinatePairs = 12;

const int kSingleColorPerPatch = 1;
const int kQuadColorsPerPatch = 4;

}  // namespace



CFX_FloatRect GetShadingBBox(CPDF_Stream* pStream,
                             ShadingType type,
                             const CFX_Matrix* pMatrix,
                             CPDF_Function** pFuncs,
                             int nFuncs,
                             CPDF_ColorSpace* pCS) {
  if (!pStream || !pStream->IsStream() || !pFuncs || !pCS)
    return CFX_FloatRect(0, 0, 0, 0);

  CPDF_MeshStream stream;
  if (!stream.Load(pStream, pFuncs, nFuncs, pCS))
    return CFX_FloatRect(0, 0, 0, 0);

  CFX_FloatRect rect;
  bool bStarted = false;
  bool bGouraud = type == kFreeFormGouraudTriangleMeshShading ||
                  type == kLatticeFormGouraudTriangleMeshShading;

  int point_count = kSingleCoordinatePair;
  if (type == kTensorProductPatchMeshShading)
    point_count = kTensorCoordinatePairs;
  else if (type == kCoonsPatchMeshShading)
    point_count = kCoonsCoordinatePairs;

  int color_count = kSingleColorPerPatch;
  if (type == kCoonsPatchMeshShading || type == kTensorProductPatchMeshShading)
    color_count = kQuadColorsPerPatch;

  while (!stream.m_BitStream.IsEOF()) {
    uint32_t flag = 0;
    if (type != kLatticeFormGouraudTriangleMeshShading)
      flag = stream.GetFlag();

    if (!bGouraud && flag) {
      point_count -= 4;
      color_count -= 2;
    }

    for (int i = 0; i < point_count; i++) {
      FX_FLOAT x, y;
      stream.GetCoords(x, y);
      if (bStarted) {
        rect.UpdateRect(x, y);
      } else {
        rect.InitRect(x, y);
        bStarted = TRUE;
      }
    }
    stream.m_BitStream.SkipBits(stream.m_nComps * stream.m_nCompBits *
                                color_count);
    if (bGouraud)
      stream.m_BitStream.ByteAlign();
  }
  rect.Transform(pMatrix);
  return rect;
}
