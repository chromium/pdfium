# Copyright 2021 The PDFium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""PDFium implementation of //build/skia_gold_common/skia_gold_properties.py."""

import pdfium_root
from skia_gold_common import skia_gold_properties


class PDFiumSkiaGoldProperties(skia_gold_properties.SkiaGoldProperties):

  def _GetGitRepoDirectory(self):
    root_finder = pdfium_root.RootDirectoryFinder()
    return root_finder.pdfium_root
