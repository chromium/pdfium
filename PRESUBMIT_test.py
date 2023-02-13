#!/usr/bin/env python3
# Copyright 2020 The PDFium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import unittest

import PRESUBMIT
from PRESUBMIT_test_mocks import MockInputApi, MockOutputApi, MockFile


class BannedTypeCheckTest(unittest.TestCase):

  def testBannedCppFunctions(self):
    input_api = MockInputApi()
    input_api.files = [
        MockFile('some/cpp/problematic/file.cc', ['using namespace std;']),
        MockFile('third_party/some/cpp/problematic/file.cc',
                 ['using namespace std;']),
        MockFile('some/cpp/ok/file.cc', ['using std::string;']),
        MockFile('some/cpp/nocheck/file.cc',
                 ['using namespace std;  // nocheck']),
        MockFile('some/cpp/comment/file.cc',
                 ['  // A comment about `using namespace std;`']),
        MockFile('some/cpp/v8/get-current.cc', ['v8::Isolate::GetCurrent()']),
        MockFile('some/cpp/v8/try-get-current.cc',
                 ['v8::Isolate::TryGetCurrent()']),
    ]

    results = PRESUBMIT._CheckNoBannedFunctions(input_api, MockOutputApi())

    # There are no warnings to test, so add an empty warning to keep the test
    # extendable for the future. This block can be removed once warnings are
    # added.
    self.assertEqual(1, len(results))
    results.insert(0, MockOutputApi().PresubmitPromptWarning(''))

    # warnings are results[0], errors are results[1]
    self.assertEqual(2, len(results))
    self.assertTrue('some/cpp/problematic/file.cc' in results[1].message)
    self.assertFalse(
        'third_party/some/cpp/problematic/file.cc' in results[1].message)
    self.assertFalse('some/cpp/ok/file.cc' in results[1].message)
    self.assertFalse('some/cpp/nocheck/file.cc' in results[0].message)
    self.assertFalse('some/cpp/nocheck/file.cc' in results[1].message)
    self.assertFalse('some/cpp/comment/file.cc' in results[0].message)
    self.assertFalse('some/cpp/comment/file.cc' in results[1].message)
    self.assertTrue('some/cpp/v8/get-current.cc' in results[1].message)
    self.assertTrue('some/cpp/v8/try-get-current.cc' in results[1].message)


class CheckChangeOnUploadTest(unittest.TestCase):

  def testCheckPngNames(self):
    correct_paths = [
        'test_expected.pdf.0.png',
        'test_expected_win.pdf.1.png',
        'test_expected_agg.pdf.3.png',
        'test_expected_agg_linux.pdf.3.png',
        'test_expected_skia.pdf.2.png',
        'test_expected_skia_mac.pdf.4.png',
        'notpng.cc',  # Check will be skipped for non-PNG files
    ]
    wrong_paths = [
        'expected.pdf.0.png',  # Missing '_expected'
        'test1_expected.0.png',  # Missing '.pdf'
        'test2_expected.pdf.png',  # Missing page number
        'test3_expected.pdf.x.png',  # Wrong character for page number
        'test4_expected_linux_agg.pdf.0.png',  # Wrong order of keywords
        'test4_expected_mac_skia.pdf.0.png',  # Wrong order of keywords
        'test5_expected_useskia.pdf.0.png',  # Wrong keyword
    ]
    mock_input_api = MockInputApi()
    mock_output_api = MockOutputApi()
    mock_input_api.files = map(MockFile, correct_paths + wrong_paths)
    errors = list(
        map(str, PRESUBMIT._CheckPngNames(mock_input_api, mock_output_api)))

    self.assertEqual(len(wrong_paths), len(errors))
    self.assertFalse('notpng.cc' in errors[0])
    for path, error in zip(wrong_paths, errors):
      self.assertIn(path, error)


if __name__ == '__main__':
  unittest.main()
