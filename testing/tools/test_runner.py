#!/usr/bin/env python
# Copyright 2016 The PDFium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import functools
import multiprocessing
import optparse
import os
import re
import shutil
import subprocess
import sys

# pylint: disable=relative-import
import common
import gold
import pngdiffer
import suppressor

# Arbitrary timestamp, expressed in seconds since the epoch, used to make sure
# that tests that depend on the current time are stable. Happens to be the
# timestamp of the first commit to repo, 2014/5/9 17:48:50.
TEST_SEED_TIME = "1399672130"

# List of test types that should run text tests instead of pixel tests.
TEXT_TESTS = ['javascript']


class KeyboardInterruptError(Exception):
  pass


# Nomenclature:
#   x_root - "x"
#   x_filename - "x.ext"
#   x_path - "path/to/a/b/c/x.ext"
#   c_dir - "path/to/a/b/c"


def TestOneFileParallel(this, test_case):
  """Wrapper to call GenerateAndTest() and redirect output to stdout."""
  try:
    input_filename, source_dir = test_case
    result = this.GenerateAndTest(input_filename, source_dir)
    return (result, input_filename, source_dir)
  except KeyboardInterrupt:
    raise KeyboardInterruptError()


def DeleteFiles(files):
  """Utility function to delete a list of files"""
  for f in files:
    if os.path.exists(f):
      os.remove(f)


class TestRunner:

  def __init__(self, dirname):
    # Currently the only used directories are corpus, javascript, and pixel,
    # which all correspond directly to the type for the test being run. In the
    # future if there are tests that don't have this clean correspondence, then
    # an argument for the type will need to be added.
    self.test_dir = dirname
    self.test_type = dirname
    self.delete_output_on_success = False
    self.enforce_expected_images = False
    self.oneshot_renderer = False

  # GenerateAndTest returns a tuple <success, outputfiles> where
  # success is a boolean indicating whether the tests passed comparison
  # tests and outputfiles is a list tuples:
  #          (path_to_image, md5_hash_of_pixelbuffer)
  def GenerateAndTest(self, input_filename, source_dir):
    input_root, _ = os.path.splitext(input_filename)
    pdf_path = os.path.join(self.working_dir, input_root + '.pdf')

    # Remove any existing generated images from previous runs.
    actual_images = self.image_differ.GetActualFiles(input_filename, source_dir,
                                                     self.working_dir)
    DeleteFiles(actual_images)

    sys.stdout.flush()

    raised_exception = self.Generate(source_dir, input_filename, input_root,
                                     pdf_path)

    if raised_exception is not None:
      print 'FAILURE: %s; %s' % (input_filename, raised_exception)
      return False, []

    results = []
    if self.test_type in TEXT_TESTS:
      expected_txt_path = os.path.join(source_dir, input_root + '_expected.txt')
      raised_exception = self.TestText(input_filename, input_root,
                                       expected_txt_path, pdf_path)
    else:
      use_ahem = 'use_ahem' in source_dir
      raised_exception, results = self.TestPixel(pdf_path, use_ahem)

    if raised_exception is not None:
      print 'FAILURE: %s; %s' % (input_filename, raised_exception)
      return False, results

    if actual_images:
      if self.image_differ.HasDifferences(input_filename, source_dir,
                                          self.working_dir):
        self.RegenerateIfNeeded_(input_filename, source_dir)
        return False, results
    else:
      if (self.enforce_expected_images and
          not self.test_suppressor.IsImageDiffSuppressed(input_filename)):
        self.RegenerateIfNeeded_(input_filename, source_dir)
        print 'FAILURE: %s; Missing expected images' % input_filename
        return False, results

    if self.delete_output_on_success:
      DeleteFiles(actual_images)
    return True, results

  def RegenerateIfNeeded_(self, input_filename, source_dir):
    if (not self.options.regenerate_expected or
        self.test_suppressor.IsResultSuppressed(input_filename) or
        self.test_suppressor.IsImageDiffSuppressed(input_filename)):
      return

    platform_only = (self.options.regenerate_expected == 'platform')
    self.image_differ.Regenerate(input_filename, source_dir, self.working_dir,
                                 platform_only)

  def Generate(self, source_dir, input_filename, input_root, pdf_path):
    original_path = os.path.join(source_dir, input_filename)
    input_path = os.path.join(source_dir, input_root + '.in')

    input_event_path = os.path.join(source_dir, input_root + '.evt')
    if os.path.exists(input_event_path):
      output_event_path = os.path.splitext(pdf_path)[0] + '.evt'
      shutil.copyfile(input_event_path, output_event_path)

    if not os.path.exists(input_path):
      if os.path.exists(original_path):
        shutil.copyfile(original_path, pdf_path)
      return None

    sys.stdout.flush()

    return common.RunCommand([
        sys.executable, self.fixup_path, '--output-dir=' + self.working_dir,
        input_path
    ])

  def TestText(self, input_filename, input_root, expected_txt_path, pdf_path):
    txt_path = os.path.join(self.working_dir, input_root + '.txt')

    with open(txt_path, 'w') as outfile:
      cmd_to_run = [
          self.pdfium_test_path, '--send-events', '--time=' + TEST_SEED_TIME
      ]

      if self.options.disable_javascript:
        cmd_to_run.append('--disable-javascript')

      if self.options.disable_xfa:
        cmd_to_run.append('--disable-xfa')

      cmd_to_run.append(pdf_path)
      subprocess.check_call(cmd_to_run, stdout=outfile)

    # If the expected file does not exist, the output is expected to be empty.
    if not os.path.exists(expected_txt_path):
      return self._VerifyEmptyText(txt_path)

    # If JavaScript is disabled, the output should be empty.
    # However, if the test is suppressed and JavaScript is disabled, do not
    # verify that the text is empty so the suppressed test does not surprise.
    if (self.options.disable_javascript and
        not self.test_suppressor.IsResultSuppressed(input_filename)):
      return self._VerifyEmptyText(txt_path)

    cmd = [sys.executable, self.text_diff_path, expected_txt_path, txt_path]
    return common.RunCommand(cmd)

  def _VerifyEmptyText(self, txt_path):
    try:
      with open(txt_path, "r") as txt_file:
        txt_data = txt_file.readlines()
      if not len(txt_data):
        return None
      sys.stdout.write('Unexpected output:\n')
      for line in txt_data:
        sys.stdout.write(line)
      raise Exception('%s should be empty.' % txt_path)
    except Exception as e:
      return e

  def TestPixel(self, pdf_path, use_ahem):
    cmd_to_run = [
        self.pdfium_test_path, '--send-events', '--png', '--md5',
        '--time=' + TEST_SEED_TIME
    ]

    if self.oneshot_renderer:
      cmd_to_run.append('--render-oneshot')

    if use_ahem:
      cmd_to_run.append('--font-dir=%s' % self.font_dir)

    if self.options.disable_javascript:
      cmd_to_run.append('--disable-javascript')

    if self.options.disable_xfa:
      cmd_to_run.append('--disable-xfa')

    if self.options.reverse_byte_order:
      cmd_to_run.append('--reverse-byte-order')

    cmd_to_run.append(pdf_path)
    return common.RunCommandExtractHashedFiles(cmd_to_run)

  def HandleResult(self, input_filename, input_path, result):
    success, image_paths = result

    if image_paths:
      for img_path, md5_hash in image_paths:
        # The output filename without image extension becomes the test name.
        # For example, "/path/to/.../testing/corpus/example_005.pdf.0.png"
        # becomes "example_005.pdf.0".
        test_name = os.path.splitext(os.path.split(img_path)[1])[0]

        matched = "suppressed"
        if not self.test_suppressor.IsResultSuppressed(input_filename):
          matched = self.gold_baseline.MatchLocalResult(test_name, md5_hash)
          if matched == gold.GoldBaseline.MISMATCH:
            print 'Skia Gold hash mismatch for test case: %s' % test_name
          elif matched == gold.GoldBaseline.NO_BASELINE:
            print 'No Skia Gold baseline found for test case: %s' % test_name

        if self.gold_results:
          self.gold_results.AddTestResult(test_name, md5_hash, img_path,
                                          matched)

    if self.test_suppressor.IsResultSuppressed(input_filename):
      self.result_suppressed_cases.append(input_filename)
      if success:
        self.surprises.append(input_path)
    else:
      if not success:
        self.failures.append(input_path)

  def Run(self):
    # Running a test defines a number of attributes on the fly.
    # pylint: disable=attribute-defined-outside-init

    parser = optparse.OptionParser()

    parser.add_option(
        '--build-dir',
        default=os.path.join('out', 'Debug'),
        help='relative path from the base source directory')

    parser.add_option(
        '-j',
        default=multiprocessing.cpu_count(),
        dest='num_workers',
        type='int',
        help='run NUM_WORKERS jobs in parallel')

    parser.add_option(
        '--disable-javascript',
        action="store_true",
        dest="disable_javascript",
        help='Prevents JavaScript from executing in PDF files.')

    parser.add_option(
        '--disable-xfa',
        action="store_true",
        dest="disable_xfa",
        help='Prevents processing XFA forms.')

    parser.add_option(
        '--gold_properties',
        default='',
        dest="gold_properties",
        help='Key value pairs that are written to the top level '
        'of the JSON file that is ingested by Gold.')

    parser.add_option(
        '--gold_key',
        default='',
        dest="gold_key",
        help='Key value pairs that are added to the "key" field '
        'of the JSON file that is ingested by Gold.')

    parser.add_option(
        '--gold_output_dir',
        default='',
        dest="gold_output_dir",
        help='Path of where to write the JSON output to be '
        'uploaded to Gold.')

    parser.add_option(
        '--gold_ignore_hashes',
        default='',
        dest="gold_ignore_hashes",
        help='Path to a file with MD5 hashes we wish to ignore.')

    parser.add_option(
        '--regenerate_expected',
        default='',
        dest="regenerate_expected",
        help='Regenerates expected images. Valid values are '
        '"all" to regenerate all expected pngs, and '
        '"platform" to regenerate only platform-specific '
        'expected pngs.')

    parser.add_option(
        '--reverse-byte-order',
        action='store_true',
        dest="reverse_byte_order",
        help='Run image-based tests using --reverse-byte-order.')

    parser.add_option(
        '--ignore_errors',
        action="store_true",
        dest="ignore_errors",
        help='Prevents the return value from being non-zero '
        'when image comparison fails.')

    self.options, self.args = parser.parse_args()

    if (self.options.regenerate_expected and
        self.options.regenerate_expected not in ['all', 'platform']):
      print 'FAILURE: --regenerate_expected must be "all" or "platform"'
      return 1

    finder = common.DirectoryFinder(self.options.build_dir)
    self.fixup_path = finder.ScriptPath('fixup_pdf_template.py')
    self.text_diff_path = finder.ScriptPath('text_diff.py')
    self.font_dir = os.path.join(finder.TestingDir(), 'resources', 'fonts')

    self.source_dir = finder.TestingDir()
    if self.test_dir != 'corpus':
      test_dir = finder.TestingDir(os.path.join('resources', self.test_dir))
    else:
      test_dir = finder.TestingDir(self.test_dir)

    self.pdfium_test_path = finder.ExecutablePath('pdfium_test')
    if not os.path.exists(self.pdfium_test_path):
      print "FAILURE: Can't find test executable '%s'" % self.pdfium_test_path
      print 'Use --build-dir to specify its location.'
      return 1

    self.working_dir = finder.WorkingDir(os.path.join('testing', self.test_dir))
    shutil.rmtree(self.working_dir, ignore_errors=True)
    os.makedirs(self.working_dir)

    self.features = subprocess.check_output(
        [self.pdfium_test_path, '--show-config']).strip().split(',')
    self.test_suppressor = suppressor.Suppressor(
        finder, self.features, self.options.disable_javascript,
        self.options.disable_xfa)
    self.image_differ = pngdiffer.PNGDiffer(finder,
                                            self.options.reverse_byte_order)
    error_message = self.image_differ.CheckMissingTools(
        self.options.regenerate_expected)
    if error_message:
      print "FAILURE: %s" % error_message
      return 1

    self.gold_baseline = gold.GoldBaseline(self.options.gold_properties)

    walk_from_dir = finder.TestingDir(test_dir)

    self.test_cases = []
    self.execution_suppressed_cases = []
    input_file_re = re.compile('^.+[.](in|pdf)$')
    if self.args:
      for file_name in self.args:
        file_name.replace('.pdf', '.in')
        input_path = os.path.join(walk_from_dir, file_name)
        if not os.path.isfile(input_path):
          print "Can't find test file '%s'" % file_name
          return 1

        self.test_cases.append((os.path.basename(input_path),
                                os.path.dirname(input_path)))
    else:
      for file_dir, _, filename_list in os.walk(walk_from_dir):
        for input_filename in filename_list:
          if input_file_re.match(input_filename):
            input_path = os.path.join(file_dir, input_filename)
            if self.test_suppressor.IsExecutionSuppressed(input_path):
              self.execution_suppressed_cases.append(input_path)
            else:
              if os.path.isfile(input_path):
                self.test_cases.append((input_filename, file_dir))

    self.test_cases.sort()
    self.failures = []
    self.surprises = []
    self.result_suppressed_cases = []

    # Collect Gold results if an output directory was named.
    self.gold_results = None
    if self.options.gold_output_dir:
      self.gold_results = gold.GoldResults(
          self.test_type, self.options.gold_output_dir,
          self.options.gold_properties, self.options.gold_key,
          self.options.gold_ignore_hashes)

    if self.options.num_workers > 1 and len(self.test_cases) > 1:
      try:
        pool = multiprocessing.Pool(self.options.num_workers)
        worker_func = functools.partial(TestOneFileParallel, self)

        worker_results = pool.imap(worker_func, self.test_cases)
        for worker_result in worker_results:
          result, input_filename, source_dir = worker_result
          input_path = os.path.join(source_dir, input_filename)

          self.HandleResult(input_filename, input_path, result)

      except KeyboardInterrupt:
        pool.terminate()
      finally:
        pool.close()
        pool.join()
    else:
      for test_case in self.test_cases:
        input_filename, input_file_dir = test_case
        result = self.GenerateAndTest(input_filename, input_file_dir)
        self.HandleResult(input_filename,
                          os.path.join(input_file_dir, input_filename), result)

    if self.gold_results:
      self.gold_results.WriteResults()

    if self.surprises:
      self.surprises.sort()
      print '\n\nUnexpected Successes:'
      for surprise in self.surprises:
        print surprise

    if self.failures:
      self.failures.sort()
      print '\n\nSummary of Failures:'
      for failure in self.failures:
        print failure

    self._PrintSummary()

    if self.failures:
      if not self.options.ignore_errors:
        return 1

    return 0

  def _PrintSummary(self):
    number_test_cases = len(self.test_cases)
    number_failures = len(self.failures)
    number_suppressed = len(self.result_suppressed_cases)
    number_successes = number_test_cases - number_failures - number_suppressed
    number_surprises = len(self.surprises)
    print
    print 'Test cases executed: %d' % number_test_cases
    print '  Successes: %d' % number_successes
    print '  Suppressed: %d' % number_suppressed
    print '    Surprises: %d' % number_surprises
    print '  Failures: %d' % number_failures
    print
    print 'Test cases not executed: %d' % len(self.execution_suppressed_cases)

  def SetDeleteOutputOnSuccess(self, new_value):
    """Set whether to delete generated output if the test passes."""
    self.delete_output_on_success = new_value

  def SetEnforceExpectedImages(self, new_value):
    """Set whether to enforce that each test case provide an expected image."""
    self.enforce_expected_images = new_value

  def SetOneShotRenderer(self, new_value):
    """Set whether to use the oneshot renderer. """
    self.oneshot_renderer = new_value
