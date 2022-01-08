#!/usr/bin/env python3
# Copyright 2016 The PDFium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division

import argparse
import functools
import multiprocessing
import os
import re
import shutil
import subprocess
import sys

import common
import pngdiffer
import suppressor
from skia_gold import skia_gold


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
    # TODO(https://crbug.com/pdfium/1674): Re-enable this check after try bots
    # can run with Python3.
    # pylint: disable=raise-missing-from
    raise KeyboardInterruptError()


def RunSkiaWrapper(this, input_chunk):
  """Wrapper to call RunSkia() and redirect output to stdout"""
  try:
    results = []
    for img_path_input_filename in input_chunk:
      img_path, input_filename = img_path_input_filename
      multiprocessing_name = multiprocessing.current_process().name

      test_name, skia_success = this.RunSkia(img_path, multiprocessing_name)
      results.append((test_name, skia_success, input_filename))
    return results
  except KeyboardInterrupt:
    # TODO(https://crbug.com/pdfium/1674): Re-enable this check after try bots
    # can run with Python3.
    # pylint: disable=raise-missing-from
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
    self.skia_tester = None

  def GetSkiaGoldTester(self, process_name=None):
    if not self.skia_tester:
      self.skia_tester = skia_gold.SkiaGoldTester(
          source_type=self.test_type,
          skia_gold_args=self.options,
          process_name=process_name)
    return self.skia_tester

  def RunSkia(self, img_path, process_name=None):
    skia_tester = self.GetSkiaGoldTester(process_name=process_name)
    # The output filename without image extension becomes the test name.
    # For example, "/path/to/.../testing/corpus/example_005.pdf.0.png"
    # becomes "example_005.pdf.0".
    test_name = os.path.splitext(os.path.split(img_path)[1])[0]
    skia_success = skia_tester.UploadTestResultToSkiaGold(test_name, img_path)
    sys.stdout.flush()
    return test_name, skia_success

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
      print('FAILURE: {}; {}'.format(input_filename, raised_exception))
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
      print('FAILURE: {}; {}'.format(input_filename, raised_exception))
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
        print('FAILURE: {}; Missing expected images'.format(input_filename))
        return False, results

    if self.delete_output_on_success:
      DeleteFiles(actual_images)
    return True, results

  # TODO(crbug.com/pdfium/1508): Add support for an option to automatically
  # generate Skia/SkiaPaths specific expected results.
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
      try:
        subprocess.check_call(cmd_to_run, stdout=outfile)
      except subprocess.CalledProcessError as e:
        return e

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
      if not txt_data:
        return None
      sys.stdout.write('Unexpected output:\n')
      for line in txt_data:
        sys.stdout.write(line)
      raise Exception('%s should be empty.' % txt_path)
    except Exception as e:
      return e

  # TODO(crbug.com/pdfium/1656): Remove when ready to fully switch over to
  # Skia Gold
  def TestPixel(self, pdf_path, use_ahem):
    cmd_to_run = [
        self.pdfium_test_path, '--send-events', '--png', '--md5',
        '--time=' + TEST_SEED_TIME
    ]

    if use_ahem:
      cmd_to_run.append('--font-dir=%s' % self.font_dir)

    if self.options.disable_javascript:
      cmd_to_run.append('--disable-javascript')

    if self.options.disable_xfa:
      cmd_to_run.append('--disable-xfa')

    if self.options.render_oneshot:
      cmd_to_run.append('--render-oneshot')

    if self.options.reverse_byte_order:
      cmd_to_run.append('--reverse-byte-order')

    cmd_to_run.append(pdf_path)
    return common.RunCommandExtractHashedFiles(cmd_to_run)

  def HandleResult(self, input_filename, input_path, result):
    success, _ = result

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

    parser = argparse.ArgumentParser()

    parser.add_argument(
        '--build-dir',
        default=os.path.join('out', 'Debug'),
        help='relative path from the base source directory')

    parser.add_argument(
        '-j',
        default=multiprocessing.cpu_count(),
        dest='num_workers',
        type=int,
        help='run NUM_WORKERS jobs in parallel')

    parser.add_argument(
        '--disable-javascript',
        action="store_true",
        dest="disable_javascript",
        help='Prevents JavaScript from executing in PDF files.')

    parser.add_argument(
        '--disable-xfa',
        action="store_true",
        dest="disable_xfa",
        help='Prevents processing XFA forms.')

    parser.add_argument(
        '--render-oneshot',
        action="store_true",
        dest="render_oneshot",
        help='Sets whether to use the oneshot renderer.')

    parser.add_argument(
        '--run-skia-gold',
        action='store_true',
        default=False,
        help='When flag is on, skia gold tests will be run.')

    # TODO: Remove when pdfium recipe stops passing this argument
    parser.add_argument(
        '--gold_properties',
        default='',
        dest="gold_properties",
        help='Key value pairs that are written to the top level '
        'of the JSON file that is ingested by Gold.')

    # TODO: Remove when pdfium recipe stops passing this argument
    parser.add_argument(
        '--gold_ignore_hashes',
        default='',
        dest="gold_ignore_hashes",
        help='Path to a file with MD5 hashes we wish to ignore.')

    parser.add_argument(
        '--regenerate_expected',
        default='',
        dest="regenerate_expected",
        help='Regenerates expected images. Valid values are '
        '"all" to regenerate all expected pngs, and '
        '"platform" to regenerate only platform-specific '
        'expected pngs.')

    parser.add_argument(
        '--reverse-byte-order',
        action='store_true',
        dest="reverse_byte_order",
        help='Run image-based tests using --reverse-byte-order.')

    parser.add_argument(
        '--ignore_errors',
        action="store_true",
        dest="ignore_errors",
        help='Prevents the return value from being non-zero '
        'when image comparison fails.')

    skia_gold.add_skia_gold_args(parser)

    self.options, self.inputted_file_paths = parser.parse_known_args()

    if (self.options.regenerate_expected and
        self.options.regenerate_expected not in ['all', 'platform']):
      print('FAILURE: --regenerate_expected must be "all" or "platform"')
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
      print("FAILURE: Can't find test executable '{}'".format(
          self.pdfium_test_path))
      print('Use --build-dir to specify its location.')
      return 1

    self.working_dir = finder.WorkingDir(os.path.join('testing', self.test_dir))
    shutil.rmtree(self.working_dir, ignore_errors=True)
    os.makedirs(self.working_dir)

    self.features = subprocess.check_output(
        [self.pdfium_test_path,
         '--show-config']).decode('utf-8').strip().split(',')
    self.test_suppressor = suppressor.Suppressor(
        finder, self.features, self.options.disable_javascript,
        self.options.disable_xfa)
    self.image_differ = pngdiffer.PNGDiffer(finder, self.features,
                                            self.options.reverse_byte_order)
    error_message = self.image_differ.CheckMissingTools(
        self.options.regenerate_expected)
    if error_message:
      print('FAILURE:', error_message)
      return 1


    walk_from_dir = finder.TestingDir(test_dir)

    self.test_cases = []
    self.execution_suppressed_cases = []
    input_file_re = re.compile('^.+[.](in|pdf)$')
    if self.inputted_file_paths:
      for file_name in self.inputted_file_paths:
        file_name.replace('.pdf', '.in')
        input_path = os.path.join(walk_from_dir, file_name)
        if not os.path.isfile(input_path):
          print("Can't find test file '{}'".format(file_name))
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
    self.skia_gold_successes = []
    self.skia_gold_unexpected_successes = []
    self.skia_gold_failures = []
    self.result_suppressed_cases = []

    gold_results = []
    if self.test_type not in TEXT_TESTS and self.options.run_skia_gold:
      assert self.options.gold_output_dir
      # Clear out and create top level gold output directory before starting
      skia_gold.clear_gold_output_dir(self.options.gold_output_dir)

    if self.options.num_workers > 1 and len(self.test_cases) > 1:
      skia_gold_parallel_inputs = []
      try:
        pool = multiprocessing.Pool(self.options.num_workers)
        worker_func = functools.partial(TestOneFileParallel, self)

        worker_results = pool.imap(worker_func, self.test_cases)
        for worker_result in worker_results:
          result, input_filename, source_dir = worker_result
          input_path = os.path.join(source_dir, input_filename)

          self.HandleResult(input_filename, input_path, result)

          if self.test_type not in TEXT_TESTS and self.options.run_skia_gold:
            _, image_paths = result
            if image_paths:
              path_filename_tuples = [
                  (path, input_filename) for path, _ in image_paths
              ]
              skia_gold_parallel_inputs.extend(path_filename_tuples)

      except KeyboardInterrupt:
        pool.terminate()
      finally:
        pool.close()
        pool.join()

      if skia_gold_parallel_inputs and self.test_type not in TEXT_TESTS:
        try:
          pool = multiprocessing.Pool(self.options.num_workers)
          gold_worker_func = functools.partial(RunSkiaWrapper, self)

          def chunk_input(whole_list):
            chunked = []
            size = len(whole_list) // self.options.num_workers
            for i in range(0, len(whole_list), size):
              chunked.append(whole_list[i:i + size])
            return chunked

          chunked_input = chunk_input(skia_gold_parallel_inputs)
          pool_results = pool.imap(gold_worker_func, chunked_input)
          for r in pool_results:
            gold_results.extend(r)
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

        _, image_paths = result
        if image_paths and self.test_type not in TEXT_TESTS and \
            self.options.run_skia_gold:
          for img_path, _ in image_paths:
            test_name, skia_success = self.RunSkia(img_path)
            gold_results.append((test_name, skia_success, input_filename))

    for r in gold_results:
      test_name, skia_success, input_filename = r
      if skia_success:
        if self.test_suppressor.IsResultSuppressed(input_filename):
          self.skia_gold_unexpected_successes.append(test_name)
        else:
          self.skia_gold_successes.append(test_name)
      else:
        self.skia_gold_failures.append(test_name)

    # For some reason, summary will be cut off from stdout on windows if
    # _PrintSummary() is called at the end
    # TODO(crbug.com/pdfium/1657): Once resolved, move _PrintSummary() back
    # down to the end
    self._PrintSummary()

    if self.surprises:
      self.surprises.sort()
      print('\nUnexpected Successes:')
      for surprise in self.surprises:
        print(surprise)

    if self.failures:
      self.failures.sort()
      print('\nSummary of Failures:')
      for failure in self.failures:
        print(failure)

    if self.skia_gold_unexpected_successes:
      self.skia_gold_unexpected_successes.sort()
      print('\nUnexpected Skia Gold Successes:')
      for surprise in self.skia_gold_unexpected_successes:
        print(surprise)

    if self.skia_gold_failures:
      self.skia_gold_failures.sort()
      print('\nSummary of Skia Gold Failures:')
      for failure in self.skia_gold_failures:
        print(failure)

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
    print('\nTest cases executed:', number_test_cases)
    print('  Successes:', number_successes)
    print('  Suppressed:', number_suppressed)
    print('  Surprises:', number_surprises)
    print('  Failures:', number_failures)
    if self.test_type not in TEXT_TESTS and self.options.run_skia_gold:
      number_gold_failures = len(self.skia_gold_failures)
      number_gold_successes = len(self.skia_gold_successes)
      number_gold_surprises = len(self.skia_gold_unexpected_successes)
      number_total_gold_tests = sum(
          [number_gold_failures, number_gold_successes, number_gold_surprises])
      print('\nSkia Gold Test cases executed:', number_total_gold_tests)
      print('  Skia Gold Successes:', number_gold_successes)
      print('  Skia Gold Surprises:', number_gold_surprises)
      print('  Skia Gold Failures:', number_gold_failures)
      skia_tester = self.GetSkiaGoldTester()
      if self.skia_gold_failures and skia_tester.IsTryjobRun():
        cl_triage_link = skia_tester.GetCLTriageLink()
        print('  Triage link for CL:', cl_triage_link)
        skia_tester.WriteCLTriageLink(cl_triage_link)
    print()
    print('Test cases not executed:', len(self.execution_suppressed_cases))

  def SetDeleteOutputOnSuccess(self, new_value):
    """Set whether to delete generated output if the test passes."""
    self.delete_output_on_success = new_value

  def SetEnforceExpectedImages(self, new_value):
    """Set whether to enforce that each test case provide an expected image."""
    self.enforce_expected_images = new_value
