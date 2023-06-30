// Copyright 2018 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SAMPLES_HELPERS_DUMP_H_
#define SAMPLES_HELPERS_DUMP_H_

#include "public/fpdfview.h"

void DumpChildStructure(FPDF_STRUCTELEMENT child, int indent);
void DumpPageInfo(FPDF_PAGE page, int page_idx);
void DumpPageStructure(FPDF_PAGE page, int page_idx);
void DumpMetaData(FPDF_DOCUMENT doc);

#endif  // SAMPLES_HELPERS_DUMP_H_
