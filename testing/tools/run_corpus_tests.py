#!/usr/bin/env python
# Copyright 2015 The PDFium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import optparse
import os
import re
import shutil
import subprocess
import sys

import common
import pngdiffer
import suppressor

# Nomenclature:
#   x_root - "x"
#   x_filename - "x.ext"
#   x_path - "path/to/a/b/c/x.ext"
#   c_dir - "path/to/a/b/c"

def test_one_file(input_filename, source_dir, working_dir,
                  pdfium_test_path, image_differ):
  input_path = os.path.join(source_dir, input_filename)
  pdf_path = os.path.join(working_dir, input_filename)
  try:
    shutil.copyfile(input_path, pdf_path)
    sys.stdout.flush()
    subprocess.check_call([pdfium_test_path, '--png', pdf_path])
  except subprocess.CalledProcessError as e:
    print "FAILURE: " + input_filename + "; " + str(e)
    return False
  if image_differ.HasDifferences(input_filename, source_dir, working_dir):
    return False
  return True

def main():
  parser = optparse.OptionParser()
  parser.add_option('--build-dir', default=os.path.join('out', 'Debug'),
                    help='relative path from the base source directory')
  options, args = parser.parse_args()
  finder = common.DirectoryFinder(options.build_dir)
  pdfium_test_path = finder.ExecutablePath('pdfium_test')
  if not os.path.exists(pdfium_test_path):
    print "FAILURE: Can't find test executable '%s'" % pdfium_test_path
    print "Use --build-dir to specify its location."
    return 1
  working_dir = finder.WorkingDir(os.path.join('testing', 'corpus'))
  if not os.path.exists(working_dir):
    os.makedirs(working_dir)

  test_suppressor = suppressor.Suppressor(finder)
  image_differ = pngdiffer.PNGDiffer(finder)

  # test files are under .../pdfium/testing/corpus.
  failures = []
  walk_from_dir = finder.TestingDir('corpus');
  input_file_re = re.compile('^[a-zA-Z0-9_.]+[.]pdf$')
  for source_dir, _, filename_list in os.walk(walk_from_dir):
    for input_filename in filename_list:
      if input_file_re.match(input_filename):
         input_path = os.path.join(source_dir, input_filename)
         if os.path.isfile(input_path):
           if test_suppressor.IsSuppressed(input_filename):
             continue
         if not test_one_file(input_filename, source_dir, working_dir,
                              pdfium_test_path, image_differ):
             failures.append(input_path)

  if failures:
    print '\n\nSummary of Failures:'
    for failure in failures:
      print failure
    return 1

  return 0


if __name__ == '__main__':
  sys.exit(main())
