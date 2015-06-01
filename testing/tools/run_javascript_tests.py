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

# Nomenclature:
#   x_root - "x"
#   x_filename - "x.ext"
#   x_path - "path/to/a/b/c/x.ext"
#   c_dir - "path/to/a/b/c"

def generate_and_test(input_filename, source_dir, working_dir,
                      fixup_path, pdfium_test_path, text_diff_path):
  input_root, _ = os.path.splitext(input_filename)
  input_path = os.path.join(source_dir, input_root + '.in')
  pdf_path = os.path.join(working_dir, input_root + '.pdf')
  txt_path = os.path.join(working_dir, input_root + '.txt')
  expected_path = os.path.join(source_dir, input_root + '_expected.txt')
  try:
    sys.stdout.flush()
    subprocess.check_call(
        [sys.executable, fixup_path, '--output-dir=' + working_dir, input_path])
    with open(txt_path, 'w') as outfile:
      subprocess.check_call([pdfium_test_path, pdf_path], stdout=outfile)
    subprocess.check_call(
        [sys.executable, text_diff_path, expected_path, txt_path])
  except subprocess.CalledProcessError as e:
    print "FAILURE: " + input_filename + "; " + str(e)
    return False
  return True

def main():
  parser = optparse.OptionParser()
  parser.add_option('--build-dir', default=os.path.join('out', 'Debug'),
                    help='relative path from the base source directory')
  options, args = parser.parse_args()
  finder = common.DirectoryFinder(options.build_dir)
  fixup_path = finder.ScriptPath('fixup_pdf_template.py')
  text_diff_path = finder.ScriptPath('text_diff.py')
  source_dir = finder.TestingDir(os.path.join('resources', 'javascript'))
  pdfium_test_path = finder.ExecutablePath('pdfium_test')
  if not os.path.exists(pdfium_test_path):
    print "FAILURE: Can't find test executable '%s'" % pdfium_test_path
    print "Use --build-dir to specify its location."
    return 1
  working_dir = finder.WorkingDir(os.path.join('testing', 'javascript'))
  if not os.path.exists(working_dir):
    os.makedirs(working_dir)

  failures = []
  input_file_re = re.compile('^[a-zA-Z0-9_.]+[.]in$')
  for input_filename in os.listdir(source_dir):
    if input_file_re.match(input_filename):
      input_path = os.path.join(source_dir, input_filename)
      if os.path.isfile(input_path):
        if not generate_and_test(input_filename, source_dir, working_dir,
                                 fixup_path, pdfium_test_path, text_diff_path):
          failures.append(input_path)

  if failures:
    print '\n\nSummary of Failures:'
    for failure in failures:
      print failure
    return 1
  return 0


if __name__ == '__main__':
  sys.exit(main())
