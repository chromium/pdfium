#!/usr/bin/env python
# Copyright 2015 The PDFium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function

import distutils.spawn
import itertools
import os
import shutil
import sys

# pylint: disable=relative-import
import common


class PathMode:
  """PathMode indicates the available expected results' paths.

  Attributes:
    DEFAULT:   Used for default expected paths in the format of
               'NAME_expected(_OSNAME)?.pdf.#.png'. For a test, this path always
               exists.
    SKIA:      Used when Skia or SkiaPaths is enabled, for paths in the format
               of 'NAME_expected_skia(_OSNAME)?.pdf.#.png'.
               Such paths only exist when the expected results of Skia or
               SkiaPaths are different from those of AGG.
    SKIAPATHS: Used when SkiaPaths is enabled, for path in the format of
               'NAME_expected_skiapaths(_OSNAME)?.pdf.#.png'.
               Such paths only exist when the expected results from using AGG,
               Skia and SkiaPaths are all different from each other.

  Always check PathMode in an incrementing order as the modes are listed in
  order of its matching paths' precedence.
  """

  DEFAULT = 0
  SKIA = 1
  SKIAPATHS = 2


class NotFoundError(Exception):
  """Raised when file doesn't exist"""
  pass


class PNGDiffer():

  def __init__(self, finder, features, reverse_byte_order):
    self.pdfium_diff_path = finder.ExecutablePath('pdfium_diff')
    self.os_name = finder.os_name
    self.reverse_byte_order = reverse_byte_order
    if 'SKIAPATHS' in features:
      self.max_path_mode = PathMode.SKIAPATHS
    elif 'SKIA' in features:
      self.max_path_mode = PathMode.SKIA
    else:
      self.max_path_mode = PathMode.DEFAULT

  @staticmethod
  def _GetMapFunc():
    try:
      # Only exists in Python 2.
      func = itertools.imap
    except AttributeError:
      # Python 3's map returns an iterator.
      func = map
    return func

  def CheckMissingTools(self, regenerate_expected):
    if (regenerate_expected and self.os_name == 'linux' and
        not distutils.spawn.find_executable('optipng')):
      return 'Please install "optipng" to regenerate expected images.'
    return None

  def GetActualFiles(self, input_filename, source_dir, working_dir):
    actual_paths = []
    path_templates = PathTemplates(input_filename, source_dir, working_dir,
                                   self.os_name, self.max_path_mode)

    for page in itertools.count():
      actual_path = path_templates.GetActualPath(page)
      expected_paths = path_templates.GetExpectedPaths(page)
      if any(self._GetMapFunc()(os.path.exists, expected_paths)):
        actual_paths.append(actual_path)
      else:
        break
    return actual_paths

  def _RunImageDiffCommand(self, expected_path, actual_path):
    if not os.path.exists(expected_path):
      return NotFoundError('%s does not exist.' % expected_path)

    cmd = [self.pdfium_diff_path]
    if self.reverse_byte_order:
      cmd.append('--reverse-byte-order')
    cmd.extend([expected_path, actual_path])
    return common.RunCommand(cmd)

  def HasDifferences(self, input_filename, source_dir, working_dir):
    path_templates = PathTemplates(input_filename, source_dir, working_dir,
                                   self.os_name, self.max_path_mode)
    for page in itertools.count():
      actual_path = path_templates.GetActualPath(page)
      expected_paths = path_templates.GetExpectedPaths(page)
      if not any(self._GetMapFunc()(os.path.exists, expected_paths)):
        if page == 0:
          print("WARNING: no expected results files for " + input_filename)
        if os.path.exists(actual_path):
          print('FAILURE: Missing expected result for 0-based page %d of %s' %
                (page, input_filename))
          return True
        break
      print("Checking " + actual_path)
      sys.stdout.flush()

      error = None
      for path in expected_paths:
        new_error = self._RunImageDiffCommand(path, actual_path)
        # Update error code. No need to overwrite the previous error code if
        # |path| doesn't exist.
        if not isinstance(new_error, NotFoundError):
          error = new_error
        # Found a match and proceed to next page
        if not error:
          break

      if error:
        print("FAILURE: " + input_filename + "; " + str(error))
        return True

    return False

  # TODO(crbug.com/pdfium/1508): Add support to automatically generate
  # Skia/SkiaPaths specific expected results.
  def Regenerate(self, input_filename, source_dir, working_dir, platform_only):
    path_templates = PathTemplates(input_filename, source_dir, working_dir,
                                   self.os_name, self.max_path_mode)

    for page in itertools.count():
      # Loop through the generated page images. Stop when there is a page
      # missing a png, which means the document ended.
      actual_path = path_templates.GetActualPath(page)
      if not os.path.isfile(actual_path):
        break

      platform_expected_path = path_templates.GetExpectedPathByPathMode(
          page, PathMode.DEFAULT, self.os_name)

      # If there is a platform expected png, we will overwrite it. Otherwise,
      # overwrite the generic png in "all" mode, or do nothing in "platform"
      # mode.
      if os.path.exists(platform_expected_path):
        expected_path = platform_expected_path
      elif not platform_only:
        expected_path = path_templates.GetExpectedPathByPathMode(
            page, PathMode.DEFAULT)
      else:
        continue

      shutil.copyfile(actual_path, expected_path)
      common.RunCommand(['optipng', expected_path])


ACTUAL_TEMPLATE = '.pdf.%d.png'

class PathTemplates(object):

  def __init__(self, input_filename, source_dir, working_dir, os_name,
               max_path_mode):
    assert PathMode.DEFAULT <= max_path_mode <= PathMode.SKIAPATHS, (
        'Unexpected Maximum PathMode: %d.' % max_path_mode)

    input_root, _ = os.path.splitext(input_filename)
    self.actual_path_template = os.path.join(working_dir,
                                             input_root + ACTUAL_TEMPLATE)
    self.source_dir = source_dir
    self.input_root = input_root
    self.max_path_mode = max_path_mode
    self.os_name = os_name

    # Pre-create the available templates depending on |max_path_mode|.
    self.expected_templates = []
    for mode in range(PathMode.DEFAULT, max_path_mode + 1):
      self.expected_templates.extend([
          self._GetExpectedTemplateByPathMode(mode),
          self._GetExpectedTemplateByPathMode(mode, os_name),
      ])

  def GetActualPath(self, page):
    return self.actual_path_template % page

  def _GetExpectedTemplateByPathMode(self, mode, os_name=None):
    expected_str = '_expected'
    if mode == PathMode.DEFAULT:
      pass
    elif mode == PathMode.SKIA:
      expected_str += '_skia'
    elif mode == PathMode.SKIAPATHS:
      expected_str += '_skiapaths'
    else:
      assert False, 'Unexpected PathMode: %d.' % mode

    if os_name:
      expected_str += '_' + self.os_name
    return os.path.join(self.source_dir,
                        self.input_root + expected_str + ACTUAL_TEMPLATE)

  def GetExpectedPathByPathMode(self, page, mode, os_name=None):
    return self._GetExpectedTemplateByPathMode(mode, os_name) % page

  def GetExpectedPaths(self, page):
    return [template % page for template in self.expected_templates]
