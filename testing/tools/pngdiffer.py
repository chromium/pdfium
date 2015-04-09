#!/usr/bin/env python
# Copyright 2015 The PDFium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import subprocess
import sys

class PNGDiffer():
  ACTUAL_TEMPLATE = '.pdf.%d.png'
  EXPECTED_TEMPLATE = '_expected' + ACTUAL_TEMPLATE
  PLATFORM_EXPECTED_TEMPLATE = '_expected_%s' + ACTUAL_TEMPLATE

  def __init__(self, finder):
    self.pdfium_diff_path = finder.ExecutablePath('pdfium_diff')
    self.os_name = finder.os_name

  def HasDifferences(self, input_filename, source_dir, working_dir):
    input_root, _ = os.path.splitext(input_filename)
    actual_path_template = os.path.join(
        working_dir, input_root + self.ACTUAL_TEMPLATE)
    expected_path_template = os.path.join(
        source_dir, input_root + self.EXPECTED_TEMPLATE)
    platform_expected_path_template = os.path.join(
        source_dir, input_root + self.PLATFORM_EXPECTED_TEMPLATE)
    i = 0
    try:
      while True:
        actual_path = actual_path_template % i
        expected_path = expected_path_template % i
        platform_expected_path = (
            platform_expected_path_template % (self.os_name, i))
        if os.path.exists(platform_expected_path):
          expected_path = platform_expected_path
        elif not os.path.exists(expected_path):
          if i == 0:
            print "WARNING: no expected results files for " + input_filename
          break
        print "Checking " + actual_path
        sys.stdout.flush()
        subprocess.check_call(
            [self.pdfium_diff_path, expected_path, actual_path])
        i += 1
    except subprocess.CalledProcessError as e:
      print "FAILURE: " + input_filename + "; " + str(e)
      return True
    return False
