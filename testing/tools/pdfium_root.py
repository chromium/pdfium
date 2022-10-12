# Copyright 2022 The PDFium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Utilities for working with the PDFium tree's root directory."""

import os
import sys


class RootDirectoryFinder:
  """Finds the PDFium tree's root directories.

  The implementation expects that either:
  1. PDFium is a standalone checkout.
  2. PDFium is part of another tree within a "third_party/pdfium" directory.

  Attributes:
      pdfium_root: The path to the root of the PDFium tree.
      source_root: The path to the root of the source tree. May differ from
        `pdfium_root` if PDFium is a third-party dependency in another tree.
  """

  def __init__(self):
    # Expect `self_dir` to be ".../testing/tools".
    self_dir = os.path.dirname(os.path.realpath(__file__))

    self.pdfium_root = _remove_path_suffix(self_dir, ('testing', 'tools'))
    if not self.pdfium_root:
      raise Exception('Cannot find testing/tools within PDFium root directory')

    # In a Chromium checkout, expect `self.pdfium_root` to be
    # ".../third_party/pdfium".
    self.source_root = _remove_path_suffix(self.pdfium_root,
                                           ('third_party', 'pdfium'))
    if not self.source_root:
      self.source_root = self.pdfium_root


def _remove_path_suffix(path, expected_suffix):
  for expected_part in reversed(expected_suffix):
    if os.path.basename(path) != expected_part:
      return None
    path = os.path.dirname(path)
  return path


def add_source_directory_to_import_path(source_directory_path):
  """Adds a source root-relative directory to end of the import path."""
  root_finder = RootDirectoryFinder()
  path = os.path.realpath(
      os.path.join(root_finder.source_root, source_directory_path))
  if path not in sys.path:
    sys.path.append(path)
