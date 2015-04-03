#!/usr/bin/env python
# Copyright 2015 The PDFium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import sys

def os_name():
  if sys.platform.startswith('linux'):
    return 'linux'
  if sys.platform.startswith('win'):
    return 'win'
  if sys.platform.startswith('darwin'):
    return 'mac'
  raise Exception('Confused, can not determine OS, aborting.')


class DirectoryFinder:
  '''A class for finding directories and paths under either a standalone
  checkout or a chromium checkout of PDFium.'''

  def __init__(self, build_location):
    # |build_location| is typically "out/Debug" or "out/Release".
    # Expect |my_dir| to be .../pdfium/testing/tools.
    self.my_dir = os.path.dirname(os.path.realpath(__file__))
    self.testing_dir = os.path.dirname(self.my_dir)
    if (os.path.basename(self.my_dir) != 'tools' or
        os.path.basename(self.testing_dir) != 'testing'):
      raise Exception('Confused, can not find pdfium root directory, aborting.')
    self.pdfium_dir = os.path.dirname(self.testing_dir)
    # Find path to build directory.  This depends on whether this is a
    # standalone build vs. a build as part of a chromium checkout. For
    # standalone, we expect a path like .../pdfium/out/Debug, but for
    # chromium, we expect a path like .../src/out/Debug two levels
    # higher (to skip over the third_party/pdfium path component under
    # which chromium sticks pdfium).
    self.base_dir = self.pdfium_dir
    one_up_dir = os.path.dirname(self.base_dir)
    two_up_dir = os.path.dirname(one_up_dir)
    if (os.path.basename(two_up_dir) == 'src' and
        os.path.basename(one_up_dir) == 'third_party'):
      self.base_dir = two_up_dir
    self.build_dir = os.path.join(self.base_dir, build_location)
    self.os_name = os_name()

  def ExecutablePath(self, name):
    '''Finds compiled binaries under the build path.'''
    result = os.path.join(self.build_dir, name)
    if self.os_name == 'win':
      result = result + '.exe'
    return result

  def ScriptPath(self, name):
    '''Finds other scripts in the same directory as this one.'''
    return os.path.join(self.my_dir, name)

  def WorkingDir(self, other_components=''):
    '''Places generated files under the build directory, not source dir.'''
    result = os.path.join(self.build_dir, 'gen', 'pdfium')
    if other_components:
      result = os.path.join(result, other_components)
    return result

  def TestingDir(self, other_components=''):
    '''Finds test files somewhere under the testing directory.'''
    result = self.testing_dir
    if other_components:
      result = os.path.join(result, other_components)
    return result
