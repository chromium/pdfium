#!/usr/bin/env python
# Copyright 2021 The PDFium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import sys


def AddDirToPathIfNeeded(*path_parts):
  path = os.path.abspath(os.path.join(*path_parts))
  if os.path.isdir(path) and path not in sys.path:
    sys.path.append(path)


def GetPDFiumDir():
  if not GetPDFiumDir.pdfium_dir:
    # Expect |skia_gold_dir| to be .../pdfium/testing/tools/skia_gold.
    skia_gold_dir = os.path.dirname(os.path.realpath(__file__))
    tools_dir = os.path.dirname(skia_gold_dir)
    testing_dir = os.path.dirname(tools_dir)
    if (os.path.basename(tools_dir) != 'tools' or
        os.path.basename(testing_dir) != 'testing'):
      raise RuntimeError(
          'Confused, can not find pdfium root directory, aborting.')
    GetPDFiumDir.pdfium_dir = os.path.dirname(testing_dir)
  return GetPDFiumDir.pdfium_dir


GetPDFiumDir.pdfium_dir = None
