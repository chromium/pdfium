#!/usr/bin/env python
# Copyright 2015 The PDFium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import optparse
import os
import re
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

def generate_and_test(input_filename, source_dir, working_dir,
                      fixup_path, pdfium_test_path, image_differ,
                      drmem_wrapper):
  input_root, _ = os.path.splitext(input_filename)
  input_path = os.path.join(source_dir, input_root + '.in')
  pdf_path = os.path.join(working_dir, input_root + '.pdf')

  # Remove any existing generated images from previous runs.
  actual_images = image_differ.GetActualFiles(
      input_filename, source_dir, working_dir)
  for image in actual_images:
    if os.path.exists(image):
      os.remove(image)

  try:
    sys.stdout.flush()
    subprocess.check_call(
        [sys.executable, fixup_path, '--output-dir=' + working_dir, input_path])
    # add Dr. Memory wrapper if exist
    cmd_to_run = common.DrMemoryWrapper(drmem_wrapper, input_root)
    cmd_to_run.extend([pdfium_test_path, '--png', pdf_path])
    # run test
    subprocess.check_call(cmd_to_run)
  except subprocess.CalledProcessError as e:
    print "FAILURE: " + input_filename + "; " + str(e)
    return False
  if image_differ.HasDifferences(input_filename, source_dir, working_dir):
    print "FAILURE: " + input_filename
    return False
  return True


def main():
  parser = optparse.OptionParser()
  parser.add_option('--build-dir', default=os.path.join('out', 'Debug'),
                    help='relative path from the base source directory')
  parser.add_option('--wrapper', default='', dest="wrapper",
                    help='Dr. Memory wrapper for running test under Dr. Memory')
  options, args = parser.parse_args()
  finder = common.DirectoryFinder(options.build_dir)
  fixup_path = finder.ScriptPath('fixup_pdf_template.py')
  source_dir = finder.TestingDir(os.path.join('resources', 'pixel'))
  pdfium_test_path = finder.ExecutablePath('pdfium_test')
  if not os.path.exists(pdfium_test_path):
    print "FAILURE: Can't find test executable '%s'" % pdfium_test_path
    print "Use --build-dir to specify its location."
    return 1
  working_dir = finder.WorkingDir(os.path.join('testing', 'pixel'))
  if not os.path.exists(working_dir):
    os.makedirs(working_dir)

  test_suppressor = suppressor.Suppressor(finder)
  image_differ = pngdiffer.PNGDiffer(finder)

  input_files = []
  if len(args):
    for file_name in args:
      input_files.append(file_name.replace(".pdf", ".in"))
  else:
    input_files = os.listdir(source_dir)

  failures = []
  input_file_re = re.compile('^[a-zA-Z0-9_.]+[.]in$')
  for input_filename in input_files:
    if input_file_re.match(input_filename):
      input_path = os.path.join(source_dir, input_filename)
      if os.path.isfile(input_path):
        if test_suppressor.IsSuppressed(input_filename):
          continue
        if not generate_and_test(input_filename, source_dir, working_dir,
                                 fixup_path, pdfium_test_path, image_differ,
                                 options.wrapper):
          failures.append(input_path)

  if failures:
    failures.sort()
    print '\n\nSummary of Failures:'
    for failure in failures:
      print failure
    return 1
  return 0


if __name__ == '__main__':
  sys.exit(main())
