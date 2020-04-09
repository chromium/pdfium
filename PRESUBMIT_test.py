#!/usr/bin/env python
# Copyright 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import unittest

import PRESUBMIT
from PRESUBMIT_test_mocks import MockInputApi, MockOutputApi, MockFile


class CheckChangeOnUploadTest(unittest.TestCase):

  def testCheckPNGFormat(self):
    correct_paths = [
        'test_expected.pdf.0.png',
        'test_expected_win.pdf.1.png',
        'test_expected_skia.pdf.2.png',
        'test_expected_skiapaths.pdf.3.png',
        'test_expected_skia_mac.pdf.4.png',
        'test_expected_skiapaths_win.pdf.5.png',
        'notpng.cc',  # Check will be skipped for non-PNG files
    ]
    wrong_paths = [
        'expected.pdf.0.png',  # Missing '_expected'
        'test1_expected.0.png',  # Missing '.pdf'
        'test2_expected.pdf.png',  # Missing page number
        'test3_expected.pdf.x.png',  # Wrong character for page number
        'test4_expected_mac_skia.pdf.0.png',  # Wrong order of keywords
        'test5_expected_useskia.pdf.0.png',  # Wrong keyword
    ]
    mock_input_api = MockInputApi()
    mock_output_api = MockOutputApi()
    mock_input_api.files = map(MockFile, correct_paths + wrong_paths)
    errors = map(str, PRESUBMIT._CheckPNGFormat(mock_input_api,
                                                mock_output_api))

    self.assertEqual(len(wrong_paths), len(errors))
    self.assertFalse('notpng.cc' in errors[0])
    for path, error in zip(wrong_paths, errors):
      self.assertIn(path, error)


if __name__ == '__main__':
  unittest.main()
