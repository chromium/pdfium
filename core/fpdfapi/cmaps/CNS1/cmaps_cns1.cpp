// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/cmaps/CNS1/cmaps_cns1.h"

#include "third_party/base/cxx17_backports.h"

const FXCMAP_CMap kFXCMAP_CNS1_cmaps[] = {
    {"B5pc-H", kFXCMAP_B5pc_H_0, nullptr, 247, 0, FXCMAP_CMap::Range, 0},
    {"B5pc-V", kFXCMAP_B5pc_V_0, nullptr, 12, 0, FXCMAP_CMap::Range, -1},
    {"HKscs-B5-H", kFXCMAP_HKscs_B5_H_5, nullptr, 1210, 0, FXCMAP_CMap::Range,
     0},
    {"HKscs-B5-V", kFXCMAP_HKscs_B5_V_5, nullptr, 13, 0, FXCMAP_CMap::Range,
     -1},
    {"ETen-B5-H", kFXCMAP_ETen_B5_H_0, nullptr, 254, 0, FXCMAP_CMap::Range, 0},
    {"ETen-B5-V", kFXCMAP_ETen_B5_V_0, nullptr, 13, 0, FXCMAP_CMap::Range, -1},
    {"ETenms-B5-H", kFXCMAP_ETenms_B5_H_0, nullptr, 1, 0, FXCMAP_CMap::Range,
     -2},
    {"ETenms-B5-V", kFXCMAP_ETenms_B5_V_0, nullptr, 18, 0, FXCMAP_CMap::Range,
     -1},
    {"CNS-EUC-H", kFXCMAP_CNS_EUC_H_0, kFXCMAP_CNS_EUC_H_0_DWord, 157, 238,
     FXCMAP_CMap::Range, 0},
    {"CNS-EUC-V", kFXCMAP_CNS_EUC_V_0, kFXCMAP_CNS_EUC_V_0_DWord, 180, 261,
     FXCMAP_CMap::Range, 0},
    {"UniCNS-UCS2-H", kFXCMAP_UniCNS_UCS2_H_3, nullptr, 16418, 0,
     FXCMAP_CMap::Range, 0},
    {"UniCNS-UCS2-V", kFXCMAP_UniCNS_UCS2_V_3, nullptr, 13, 0,
     FXCMAP_CMap::Range, -1},
    {"UniCNS-UTF16-H", kFXCMAP_UniCNS_UTF16_H_0, nullptr, 14557, 0,
     FXCMAP_CMap::Single, 0},
    {"UniCNS-UTF16-V", kFXCMAP_UniCNS_UCS2_V_3, nullptr, 13, 0,
     FXCMAP_CMap::Range, -1},
};

const size_t kFXCMAP_CNS1_cmaps_size = pdfium::size(kFXCMAP_CNS1_cmaps);
