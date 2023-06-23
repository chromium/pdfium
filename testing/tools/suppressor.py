#!/usr/bin/env python3
# Copyright 2015 The PDFium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os

import common
import pngdiffer


class Suppressor:

  def __init__(self, finder, features, js_disabled, xfa_disabled,
               rendering_option):
    self.has_v8 = not js_disabled and 'V8' in features
    self.has_xfa = not js_disabled and not xfa_disabled and 'XFA' in features
    self.rendering_option = rendering_option
    self.suppression_set = self._LoadSuppressedSet('SUPPRESSIONS', finder)
    self.image_suppression_set = self._LoadSuppressedSet(
        'SUPPRESSIONS_IMAGE_DIFF', finder)
    self.exact_matching_suppression_set = self._LoadSuppressedSet(
        'SUPPRESSIONS_EXACT_MATCHING', finder)

  def _LoadSuppressedSet(self, suppressions_filename, finder):
    v8_option = "v8" if self.has_v8 else "nov8"
    xfa_option = "xfa" if self.has_xfa else "noxfa"
    with open(os.path.join(finder.TestingDir(), suppressions_filename)) as f:
      return set(
          self._FilterSuppressions(common.os_name(), v8_option, xfa_option,
                                   self.rendering_option,
                                   self._ExtractSuppressions(f)))

  def _ExtractSuppressions(self, f):
    return [
        y.split(' ') for y in [x.split('#')[0].strip()
                               for x in f.readlines()] if y
    ]

  def _FilterSuppressions(self, os_name, js, xfa, rendering_option,
                          unfiltered_list):
    return [
        x[0]
        for x in unfiltered_list
        if self._MatchSuppression(x, os_name, js, xfa, rendering_option)
    ]

  def _MatchSuppression(self, item, os_name, js, xfa, rendering_option):
    os_column = item[1].split(",")
    js_column = item[2].split(",")
    xfa_column = item[3].split(",")
    rendering_option_column = item[4].split(",")
    return (('*' in os_column or os_name in os_column) and
            ('*' in js_column or js in js_column) and
            ('*' in xfa_column or xfa in xfa_column) and
            ('*' in rendering_option_column or
             rendering_option in rendering_option_column))

  def IsResultSuppressed(self, input_filename):
    if input_filename in self.suppression_set:
      print("%s result is suppressed" % input_filename)
      return True
    return False

  def IsExecutionSuppressed(self, input_filepath):
    if "xfa_specific" in input_filepath and not self.has_xfa:
      print("%s execution is suppressed" % input_filepath)
      return True
    return False

  def IsImageDiffSuppressed(self, input_filename):
    if input_filename in self.image_suppression_set:
      print("%s image diff comparison is suppressed" % input_filename)
      return True
    return False

  def GetImageMatchingAlgorithm(self, input_filename):
    if input_filename in self.exact_matching_suppression_set:
      print(f"{input_filename} image diff comparison is fuzzy")
      return pngdiffer.FUZZY_MATCHING
    return pngdiffer.EXACT_MATCHING
