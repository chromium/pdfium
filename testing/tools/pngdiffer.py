#!/usr/bin/env python3
# Copyright 2015 The PDFium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from dataclasses import dataclass
import distutils.spawn
import itertools
import os
import shutil
import sys

import common


class PathMode:
  """PathMode indicates the available expected results' paths.

  Attributes:
    DEFAULT:   Used for default expected paths in the format of
               'NAME_expected(_OSNAME)?.pdf.#.png'. For a test, this path always
               exists.
    SKIA:      Used when Skia is enabled, for paths in the format of
               'NAME_expected_skia(_OSNAME)?.pdf.#.png'.
               Such paths only exist when the expected results of Skia are
               different from those of AGG.

  Always check PathMode in an incrementing order as the modes are listed in
  order of its matching paths' precedence.
  """

  DEFAULT = 0
  SKIA = 1


@dataclass
class ImageDiff:
  """Details about an image diff.

  Attributes:
    actual_path: Path to the actual image file.
    expected_path: Path to the expected image file, or `None` if no matches.
    diff_path: Path to the diff image file, or `None` if no diff.
    reason: Optional reason for the diff.
  """
  actual_path: str
  expected_path: str = None
  diff_path: str = None
  reason: str = None


class PNGDiffer():

  def __init__(self, finder, features, reverse_byte_order):
    self.pdfium_diff_path = finder.ExecutablePath('pdfium_diff')
    self.os_name = finder.os_name
    self.reverse_byte_order = reverse_byte_order
    if 'SKIA' in features:
      self.max_path_mode = PathMode.SKIA
    else:
      self.max_path_mode = PathMode.DEFAULT

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
      if any(map(os.path.exists, expected_paths)):
        actual_paths.append(actual_path)
      else:
        break
    return actual_paths

  def _RunImageCompareCommand(self, image_diff):
    cmd = [self.pdfium_diff_path]
    if self.reverse_byte_order:
      cmd.append('--reverse-byte-order')
    cmd.extend([image_diff.actual_path, image_diff.expected_path])
    return common.RunCommand(cmd)

  def _RunImageDiffCommand(self, image_diff):
    # TODO(crbug.com/pdfium/1925): Diff mode ignores --reverse-byte-order.
    return common.RunCommand([
        self.pdfium_diff_path, '--subtract', image_diff.actual_path,
        image_diff.expected_path, image_diff.diff_path
    ])

  def ComputeDifferences(self, input_filename, source_dir, working_dir):
    """Computes differences between actual and expected image files.

    Returns:
      A list of `ImageDiff` instances, one per differing page.
    """
    image_diffs = []

    path_templates = PathTemplates(input_filename, source_dir, working_dir,
                                   self.os_name, self.max_path_mode)
    for page in itertools.count():
      page_diff = ImageDiff(actual_path=path_templates.GetActualPath(page))
      if not os.path.exists(page_diff.actual_path):
        break

      last_not_found_expected_path = None
      compare_error = None
      for expected_path in path_templates.GetExpectedPaths(page):
        if not os.path.exists(expected_path):
          last_not_found_expected_path = expected_path
          continue
        page_diff.expected_path = expected_path

        compare_error = self._RunImageCompareCommand(page_diff)
        if not compare_error:
          # Found a match.
          break

      if page_diff.expected_path:
        if not compare_error:
          # Proceed to next page
          continue
        page_diff.reason = str(compare_error)

        # TODO(crbug.com/pdfium/1925): Compare and diff in a single invocation.
        page_diff.diff_path = path_templates.GetDiffPath(page)
        if not self._RunImageDiffCommand(page_diff):
          print(f'WARNING: No diff for {page_diff.actual_path}')
          page_diff.diff_path = None
      else:
        if page == 0:
          print(f'WARNING: no expected results files for {input_filename}')
        if last_not_found_expected_path:
          page_diff.reason = f'{last_not_found_expected_path} does not exist'
        else:
          page_diff.reason = f'Missing expected result for 0-based page {page}'

      image_diffs.append(page_diff)

    return image_diffs

  # TODO(crbug.com/pdfium/1508): Add support to automatically generate Skia
  # specific expected results.
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
DIFF_TEMPLATE = '.pdf.%d.diff.png'


class PathTemplates:

  def __init__(self, input_filename, source_dir, working_dir, os_name,
               max_path_mode):
    assert PathMode.DEFAULT <= max_path_mode <= PathMode.SKIA, (
        'Unexpected Maximum PathMode: %d.' % max_path_mode)

    input_root, _ = os.path.splitext(input_filename)
    self.actual_path_template = os.path.join(working_dir,
                                             input_root + ACTUAL_TEMPLATE)
    self.diff_path_template = os.path.join(working_dir,
                                           input_root + DIFF_TEMPLATE)
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

  def GetDiffPath(self, page):
    return self.diff_path_template % page

  def _GetExpectedTemplateByPathMode(self, mode, os_name=None):
    expected_str = '_expected'
    if mode == PathMode.DEFAULT:
      pass
    elif mode == PathMode.SKIA:
      expected_str += '_skia'
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
