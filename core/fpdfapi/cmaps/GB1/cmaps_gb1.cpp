// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/cmaps/GB1/cmaps_gb1.h"

#include <iterator>

const FXCMAP_CMap kFXCMAP_GB1_cmaps[] = {
    {"GB-EUC-H", kFXCMAP_GB_EUC_H_0, nullptr, 90, 0, FXCMAP_CMap::Range, 0},
    {"GB-EUC-V", kFXCMAP_GB_EUC_V_0, nullptr, 20, 0, FXCMAP_CMap::Range, -1},
    {"GBpc-EUC-H", kFXCMAP_GBpc_EUC_H_0, nullptr, 91, 0, FXCMAP_CMap::Range, 0},
    {"GBpc-EUC-V", kFXCMAP_GBpc_EUC_V_0, nullptr, 20, 0, FXCMAP_CMap::Range,
     -1},
    {"GBK-EUC-H", kFXCMAP_GBK_EUC_H_2, nullptr, 4071, 0, FXCMAP_CMap::Range, 0},
    {"GBK-EUC-V", kFXCMAP_GBK_EUC_V_2, nullptr, 20, 0, FXCMAP_CMap::Range, -1},
    {"GBKp-EUC-H", kFXCMAP_GBKp_EUC_H_2, nullptr, 4070, 0, FXCMAP_CMap::Range,
     -2},
    {"GBKp-EUC-V", kFXCMAP_GBKp_EUC_V_2, nullptr, 20, 0, FXCMAP_CMap::Range,
     -1},
    {"GBK2K-H", kFXCMAP_GBK2K_H_5, kFXCMAP_GBK2K_H_5_DWord, 4071, 1017,
     FXCMAP_CMap::Range, -4},
    {"GBK2K-V", kFXCMAP_GBK2K_V_5, nullptr, 41, 0, FXCMAP_CMap::Range, -1},
    {"UniGB-UCS2-H", kFXCMAP_UniGB_UCS2_H_4, nullptr, 13825, 0,
     FXCMAP_CMap::Range, 0},
    {"UniGB-UCS2-V", kFXCMAP_UniGB_UCS2_V_4, nullptr, 24, 0, FXCMAP_CMap::Range,
     -1},
    {"UniGB-UTF16-H", kFXCMAP_UniGB_UCS2_H_4, nullptr, 13825, 0,
     FXCMAP_CMap::Range, 0},
    {"UniGB-UTF16-V", kFXCMAP_UniGB_UCS2_V_4, nullptr, 24, 0,
     FXCMAP_CMap::Range, -1},
};

const size_t kFXCMAP_GB1_cmaps_size = std::size(kFXCMAP_GB1_cmaps);
