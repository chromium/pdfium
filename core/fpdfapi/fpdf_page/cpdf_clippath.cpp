// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/fpdf_page/include/cpdf_clippath.h"

#include "core/fpdfapi/fpdf_page/include/cpdf_textobject.h"

#define FPDF_CLIPPATH_MAX_TEXTS 1024

CFX_FloatRect CPDF_ClipPath::GetClipBox() const {
  CFX_FloatRect rect;
  FX_BOOL bStarted = FALSE;
  int count = GetPathCount();
  if (count) {
    rect = GetPath(0).GetBoundingBox();
    for (int i = 1; i < count; i++) {
      CFX_FloatRect path_rect = GetPath(i).GetBoundingBox();
      rect.Intersect(path_rect);
    }
    bStarted = TRUE;
  }
  count = GetTextCount();
  if (count) {
    CFX_FloatRect layer_rect;
    FX_BOOL bLayerStarted = FALSE;
    for (int i = 0; i < count; i++) {
      CPDF_TextObject* pTextObj = GetText(i);
      if (!pTextObj) {
        if (!bStarted) {
          rect = layer_rect;
          bStarted = TRUE;
        } else {
          rect.Intersect(layer_rect);
        }
        bLayerStarted = FALSE;
      } else {
        if (!bLayerStarted) {
          layer_rect = CFX_FloatRect(pTextObj->GetBBox(nullptr));
          bLayerStarted = TRUE;
        } else {
          layer_rect.Union(CFX_FloatRect(pTextObj->GetBBox(nullptr)));
        }
      }
    }
  }
  return rect;
}

void CPDF_ClipPath::AppendPath(CPDF_Path path, int type, FX_BOOL bAutoMerge) {
  CPDF_ClipPathData* pData = GetModify();
  if (pData->m_PathCount && bAutoMerge) {
    CPDF_Path old_path = pData->m_pPathList[pData->m_PathCount - 1];
    if (old_path.IsRect()) {
      CFX_FloatRect old_rect(old_path.GetPointX(0), old_path.GetPointY(0),
                             old_path.GetPointX(2), old_path.GetPointY(2));
      CFX_FloatRect new_rect = path.GetBoundingBox();
      if (old_rect.Contains(new_rect)) {
        pData->m_PathCount--;
        pData->m_pPathList[pData->m_PathCount].SetNull();
      }
    }
  }
  if (pData->m_PathCount % 8 == 0) {
    CPDF_Path* pNewPath = new CPDF_Path[pData->m_PathCount + 8];
    for (int i = 0; i < pData->m_PathCount; i++) {
      pNewPath[i] = pData->m_pPathList[i];
    }
    delete[] pData->m_pPathList;
    uint8_t* pNewType = FX_Alloc(uint8_t, pData->m_PathCount + 8);
    FXSYS_memcpy(pNewType, pData->m_pTypeList, pData->m_PathCount);
    FX_Free(pData->m_pTypeList);
    pData->m_pPathList = pNewPath;
    pData->m_pTypeList = pNewType;
  }
  pData->m_pPathList[pData->m_PathCount] = path;
  pData->m_pTypeList[pData->m_PathCount] = (uint8_t)type;
  pData->m_PathCount++;
}

void CPDF_ClipPath::DeletePath(int index) {
  CPDF_ClipPathData* pData = GetModify();
  if (index >= pData->m_PathCount) {
    return;
  }
  pData->m_pPathList[index].SetNull();
  for (int i = index; i < pData->m_PathCount - 1; i++) {
    pData->m_pPathList[i] = pData->m_pPathList[i + 1];
  }
  pData->m_pPathList[pData->m_PathCount - 1].SetNull();
  FXSYS_memmove(pData->m_pTypeList + index, pData->m_pTypeList + index + 1,
                pData->m_PathCount - index - 1);
  pData->m_PathCount--;
}

void CPDF_ClipPath::AppendTexts(CPDF_TextObject** pTexts, int count) {
  CPDF_ClipPathData* pData = GetModify();
  if (pData->m_TextCount + count > FPDF_CLIPPATH_MAX_TEXTS) {
    for (int i = 0; i < count; i++) {
      delete pTexts[i];
    }
    return;
  }
  CPDF_TextObject** pNewList =
      FX_Alloc(CPDF_TextObject*, pData->m_TextCount + count + 1);
  if (pData->m_pTextList) {
    FXSYS_memcpy(pNewList, pData->m_pTextList,
                 pData->m_TextCount * sizeof(CPDF_TextObject*));
    FX_Free(pData->m_pTextList);
  }
  pData->m_pTextList = pNewList;
  for (int i = 0; i < count; i++) {
    pData->m_pTextList[pData->m_TextCount + i] = pTexts[i];
  }
  pData->m_pTextList[pData->m_TextCount + count] = NULL;
  pData->m_TextCount += count + 1;
}

void CPDF_ClipPath::Transform(const CFX_Matrix& matrix) {
  CPDF_ClipPathData* pData = GetModify();
  int i;
  for (i = 0; i < pData->m_PathCount; i++) {
    pData->m_pPathList[i].Transform(&matrix);
  }
  for (i = 0; i < pData->m_TextCount; i++)
    if (pData->m_pTextList[i]) {
      pData->m_pTextList[i]->Transform(matrix);
    }
}
