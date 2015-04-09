#!/usr/bin/env python
# Copyright 2015 The PDFium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os

import common

class Suppressor:
  SUPPRESSIONS_FILENAME = 'SUPPRESSIONS'
  PLATFORM_SUPPRESSIONS_FILENAME = 'SUPPRESSIONS_%s' % common.os_name()

  def __init__(self, finder):
    testing_dir = finder.TestingDir()
    self.suppression_list = self._ExtractSuppressions(
      os.path.join(testing_dir, self.SUPPRESSIONS_FILENAME))
    self.platform_suppression_list = self._ExtractSuppressions(
      os.path.join(testing_dir, self.PLATFORM_SUPPRESSIONS_FILENAME))

  def _ExtractSuppressions(self, suppressions_filename):
    with open(suppressions_filename) as f:
      return [y for y in [x.split('#')[0].strip() for x in f.readlines()] if y]

  def IsSuppressed(self, input_filename):
    if input_filename in self.suppression_list:
      print ("Not running %s, found in %s file" %
             (input_filename, self.SUPPRESSIONS_FILENAME))
      return True
    if input_filename in self.platform_suppression_list:
      print ("Not running %s, found in %s file" %
             (input_filename, self.PLATFORM_SUPPRESSIONS_FILENAME))
      return True
    return False
