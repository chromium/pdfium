# Copyright 2021 The PDFium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""PDFium implementation of
//build/skia_gold_common/skia_gold_session_manager.py."""

from . import pdfium_skia_gold_session
from skia_gold_common import skia_gold_session_manager as sgsm

SKIA_PDF_INSTANCE = 'pdfium'


class PDFiumSkiaGoldSessionManager(sgsm.SkiaGoldSessionManager):

  @staticmethod
  def GetSessionClass():
    return pdfium_skia_gold_session.PDFiumSkiaGoldSession

  @staticmethod
  def _GetDefaultInstance():
    return SKIA_PDF_INSTANCE
