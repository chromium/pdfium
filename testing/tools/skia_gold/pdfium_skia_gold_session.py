# Copyright 2021 The PDFium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""PDFium implementation of //build/skia_gold_common/skia_gold_session.py."""

from skia_gold_common import output_managerless_skia_gold_session as omsgs


# ComparisonResults nested inside the SkiaGoldSession causes issues with
# multiprocessing and pickling, so it was moved out here.
class PDFiumComparisonResults:
  """Struct-like object for storing results of an image comparison."""

  def __init__(self):
    self.public_triage_link = None
    self.internal_triage_link = None
    self.triage_link_omission_reason = None
    self.local_diff_given_image = None
    self.local_diff_closest_image = None
    self.local_diff_diff_image = None


class PDFiumSkiaGoldSession(omsgs.OutputManagerlessSkiaGoldSession):

  def _GetDiffGoldInstance(self):
    return str(self._instance)

  def ComparisonResults(self):
    return PDFiumComparisonResults()
