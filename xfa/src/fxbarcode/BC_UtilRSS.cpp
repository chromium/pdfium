// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com
// Original code is licensed as follows:
/*
 * Copyright 2009 ZXing authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "barcode.h"
#include "BC_UtilRSS.h"
CBC_UtilRSS::CBC_UtilRSS() {}
CBC_UtilRSS::~CBC_UtilRSS() {}
CFX_Int32Array* CBC_UtilRSS::GetRssWidths(int32_t val,
                                          int32_t n,
                                          int32_t elements,
                                          int32_t maxWidth,
                                          FX_BOOL noNarrow) {
  CFX_Int32Array* iTemp = new CFX_Int32Array;
  iTemp->SetSize(elements);
  CBC_AutoPtr<CFX_Int32Array> widths(iTemp);
  int32_t bar;
  int32_t narrowMask = 0;
  for (bar = 0; bar < elements - 1; bar++) {
    narrowMask |= (1 << bar);
    int32_t elmWidth = 1;
    int32_t subVal;
    while (TRUE) {
      subVal = Combins(n - elmWidth - 1, elements - bar - 2);
      if (noNarrow && (narrowMask == 0) &&
          (n - elmWidth - (elements - bar - 1) >= elements - bar - 1)) {
        subVal -= Combins(n - elmWidth - (elements - bar), elements - bar - 2);
      }
      if (elements - bar - 1 > 1) {
        int32_t lessVal = 0;
        for (int32_t mxwElement = n - elmWidth - (elements - bar - 2);
             mxwElement > maxWidth; mxwElement--) {
          lessVal += Combins(n - elmWidth - mxwElement - 1, elements - bar - 3);
        }
        subVal -= lessVal * (elements - 1 - bar);
      } else if (n - elmWidth > maxWidth) {
        subVal--;
      }
      val -= subVal;
      if (val < 0) {
        break;
      }
      elmWidth++;
      narrowMask &= ~(1 << bar);
    }
    val += subVal;
    n -= elmWidth;
    (*widths)[bar] = elmWidth;
  }
  (*widths)[bar] = n;
  return widths.release();
}
int32_t CBC_UtilRSS::GetRSSvalue(CFX_Int32Array& widths,
                                 int32_t maxWidth,
                                 FX_BOOL noNarrow) {
  int32_t elements = widths.GetSize();
  int32_t n = 0;
  for (int32_t i = 0; i < elements; i++) {
    n += widths[i];
  }
  int32_t val = 0;
  int32_t narrowMask = 0;
  for (int32_t bar = 0; bar < elements - 1; bar++) {
    int32_t elmWidth;
    for (elmWidth = 1, narrowMask |= (1 << bar); elmWidth < widths[bar];
         elmWidth++, narrowMask &= ~(1 << bar)) {
      int32_t subVal = Combins(n - elmWidth - 1, elements - bar - 2);
      if (noNarrow && (narrowMask == 0) &&
          (n - elmWidth - (elements - bar - 1) >= elements - bar - 1)) {
        subVal -= Combins(n - elmWidth - (elements - bar), elements - bar - 2);
      }
      if (elements - bar - 1 > 1) {
        int32_t lessVal = 0;
        for (int32_t mxwElement = n - elmWidth - (elements - bar - 2);
             mxwElement > maxWidth; mxwElement--) {
          lessVal += Combins(n - elmWidth - mxwElement - 1, elements - bar - 3);
        }
        subVal -= lessVal * (elements - 1 - bar);
      } else if (n - elmWidth > maxWidth) {
        subVal--;
      }
      val += subVal;
    }
    n -= elmWidth;
  }
  return val;
}
int32_t CBC_UtilRSS::Combins(int32_t n, int32_t r) {
  int32_t maxDenom;
  int32_t minDenom;
  if (n - r > r) {
    minDenom = r;
    maxDenom = n - r;
  } else {
    minDenom = n - r;
    maxDenom = r;
  }
  int32_t val = 1;
  int32_t j = 1;
  for (int32_t i = n; i > maxDenom; i--) {
    val *= i;
    if (j <= minDenom) {
      val /= j;
      j++;
    }
  }
  while (j <= minDenom) {
    val /= j;
    j++;
  }
  return val;
}
CFX_Int32Array* CBC_UtilRSS::Elements(CFX_Int32Array& eDist,
                                      int32_t N,
                                      int32_t K) {
  CFX_Int32Array* widths = new CFX_Int32Array;
  widths->SetSize(eDist.GetSize() + 2);
  int32_t twoK = K << 1;
  (*widths)[0] = 1;
  int32_t i;
  int32_t minEven = 10;
  int32_t barSum = 1;
  for (i = 1; i < twoK - 2; i += 2) {
    (*widths)[i] = eDist[i - 1] - (*widths)[i - 1];
    (*widths)[i + 1] = eDist[i] - (*widths)[i];
    barSum += (*widths)[i] + (*widths)[i + 1];
    if ((*widths)[i] < minEven) {
      minEven = (*widths)[i];
    }
  }
  (*widths)[twoK - 1] = N - barSum;
  if ((*widths)[twoK - 1] < minEven) {
    minEven = (*widths)[twoK - 1];
  }
  if (minEven > 1) {
    for (i = 0; i < twoK; i += 2) {
      (*widths)[i] += minEven - 1;
      (*widths)[i + 1] -= minEven - 1;
    }
  }
  return widths;
}
