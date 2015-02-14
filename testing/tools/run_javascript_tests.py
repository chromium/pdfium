#!/usr/bin/env python
# Copyright 2015 The PDFium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import optparse
import os
import re
import subprocess
import sys

# Nomenclature:
#   x_root - "x"
#   x_filename - "x.ext"
#   x_path - "path/to/a/b/c/x.ext"
#   c_dir - "path/to/a/b/c"

def generate_and_test(input_filename, source_dir, working_dir,
                      fixup_path, pdfium_test_path):
  input_root, _ = os.path.splitext(input_filename)
  input_path = os.path.join(source_dir, input_root + '.in')
  pdf_path = os.path.join(working_dir, input_root + '.pdf')
  txt_path = os.path.join(working_dir, input_root + '.txt')
  expected_path = os.path.join(source_dir, input_root + '_expected.txt')
  try:
    subprocess.check_call(
        [fixup_path, '--output-dir=' + working_dir, input_path])
    with open(txt_path, 'w') as outfile:
      subprocess.check_call([pdfium_test_path, pdf_path], stdout=outfile)
    subprocess.check_call(['diff', expected_path, txt_path])
  except subprocess.CalledProcessError as e:
    print "FAILURE: " + input_filename + "; " + str(e)

def main():
  parser = optparse.OptionParser()
  parser.add_option('--build-dir', default=os.path.join('out', 'Debug'),
                    help='relative path from the base source directory')
  options, args = parser.parse_args()

  # Expect |my_dir| to be .../pdfium/testing/tools.
  my_dir = os.path.dirname(os.path.realpath(__file__))
  testing_dir = os.path.dirname(my_dir)
  pdfium_dir = os.path.dirname(testing_dir)
  if (os.path.basename(my_dir) != 'tools' or
      os.path.basename(testing_dir) != 'testing'):
    print 'Confused, can not find pdfium root directory, aborting.'
    return 1

  # Other scripts are found in the same directory as this one.
  fixup_path = os.path.join(my_dir, 'fixup_pdf_template.py')

  # test files are in .../pdfium/testing/resources/javascript.
  source_dir = os.path.join(testing_dir, 'resources', 'javascript')

  # Find path to build directory.  This depends on whether this is a
  # standalone build vs. a build as part of a chromium checkout. For
  # standalone, we expect a path like .../pdfium/out/Debug, but for
  # chromium, we expect a path like .../src/out/Debug two levels
  # higher (to skip over the third_party/pdfium path component under
  # which chromium sticks pdfium).
  base_dir = pdfium_dir
  one_up_dir = os.path.dirname(base_dir)
  two_up_dir = os.path.dirname(one_up_dir)
  if (os.path.basename(two_up_dir) == 'src' and
      os.path.basename(one_up_dir) == 'third_party'):
    base_dir = two_up_dir
  build_dir = os.path.join(base_dir, options.build_dir)

  # Compiled binaries are found under the build path.
  pdfium_test_path = os.path.join(build_dir, 'pdfium_test')
  if sys.platform.startswith('win'):
    pdfium_test_path = pdfium_test_path + '.exe'
  # TODO(tsepez): Mac may require special handling here.

  # Place generated files under the build directory, not source directory.
  gen_dir = os.path.join(build_dir, 'gen', 'pdfium')
  working_dir = os.path.join(gen_dir, 'testing', 'javascript')
  if not os.path.exists(working_dir):
    os.makedirs(working_dir)

  input_file_re = re.compile('^[a-zA-Z0-9_.]+[.]in$')
  for input_filename in os.listdir(source_dir):
    if input_file_re.match(input_filename):
      input_path = os.path.join(source_dir, input_filename)
      if os.path.isfile(input_path):
        generate_and_test(input_filename, source_dir, working_dir,
                          fixup_path, pdfium_test_path)
  return 0


if __name__ == '__main__':
  sys.exit(main())
