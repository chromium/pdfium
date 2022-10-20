#!/usr/bin/env python3
# Copyright 2016 The PDFium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division

import argparse
from dataclasses import dataclass
import multiprocessing
import os
import re
import shutil
import subprocess
import sys
import time

import common
import pdfium_root
import pngdiffer
from skia_gold import skia_gold
import suppressor

pdfium_root.add_source_directory_to_import_path(os.path.join('build', 'util'))
from lib.results import result_sink, result_types


# Arbitrary timestamp, expressed in seconds since the epoch, used to make sure
# that tests that depend on the current time are stable. Happens to be the
# timestamp of the first commit to repo, 2014/5/9 17:48:50.
TEST_SEED_TIME = "1399672130"

# List of test types that should run text tests instead of pixel tests.
TEXT_TESTS = ['javascript']


class KeyboardInterruptError(Exception):
  """Custom exception used to wrap `KeyboardInterrupt` exceptions."""


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
    self.per_process_config = _PerProcessConfig(
        test_dir=dirname, test_type=dirname)

  @property
  def options(self):
    return self.per_process_config.options

  def IsSkiaGoldEnabled(self):
    return (self.options.run_skia_gold and
            not self.per_process_config.test_type in TEXT_TESTS)

  def IsExecutionSuppressed(self, input_path):
    return self.per_process_state.test_suppressor.IsExecutionSuppressed(
        input_path)

  def IsResultSuppressed(self, input_filename):
    return self.per_process_state.test_suppressor.IsResultSuppressed(
        input_filename)

  def HandleResult(self, test_case, test_result):
    input_filename = os.path.basename(test_case.input_path)

    if self.IsResultSuppressed(input_filename):
      self.result_suppressed_cases.append(input_filename)
      if test_result.IsPass():
        self.surprises.append(test_case.input_path)

        # There isn't an actual status for succeeded-but-ignored, so use the
        # "abort" status to differentiate this from failed-but-ignored.
        #
        # Note that this appears as a preliminary failure in Gerrit.
        result_status = result_types.UNKNOWN
      else:
        # There isn't an actual status for failed-but-ignored, so use the
        # "skip" status to differentiate this from succeeded-but-ignored.
        result_status = result_types.SKIP
    else:
      if test_result.IsPass():
        result_status = result_types.PASS
      else:
        self.failures.append(test_case.input_path)
        result_status = result_types.FAIL

    if test_result.image_artifacts:
      for artifact in test_result.image_artifacts:
        if artifact.skia_gold_status == result_types.PASS:
          if self.IsResultSuppressed(artifact.image_path):
            self.skia_gold_unexpected_successes.append(artifact.GetSkiaGoldId())
          else:
            self.skia_gold_successes.append(artifact.GetSkiaGoldId())
        elif artifact.skia_gold_status == result_types.FAIL:
          self.skia_gold_failures.append(artifact.GetSkiaGoldId())

    if self.resultdb:
      # TODO(crbug.com/pdfium/1916): Populate more ResultDB fields.
      self.resultdb.Post(
          test_id=test_result.test_id,
          status=result_status,
          duration=test_result.duration_milliseconds,
          test_log=None,
          test_file=None)

  def Run(self):
    # Running a test defines a number of attributes on the fly.
    # pylint: disable=attribute-defined-outside-init

    relative_test_dir = self.per_process_config.test_dir
    if relative_test_dir != 'corpus':
      relative_test_dir = os.path.join('resources', relative_test_dir)

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

    parser.add_argument(
        'inputted_file_paths',
        nargs='*',
        help='Path to test files to run, relative to '
        f'testing/{relative_test_dir}. If omitted, runs all test files under '
        f'testing/{relative_test_dir}.',
        metavar='relative/test/path')

    skia_gold.add_skia_gold_args(parser)

    self.per_process_config.options = parser.parse_args()

    if (self.options.regenerate_expected and
        self.options.regenerate_expected not in ['all', 'platform']):
      print('FAILURE: --regenerate_expected must be "all" or "platform"')
      return 1

    finder = self.per_process_config.NewFinder()
    pdfium_test_path = self.per_process_config.GetPdfiumTestPath(finder)
    if not os.path.exists(pdfium_test_path):
      print(f"FAILURE: Can't find test executable '{pdfium_test_path}'")
      print('Use --build-dir to specify its location.')
      return 1
    self.per_process_config.InitializeFeatures(pdfium_test_path)

    self.per_process_state = _PerProcessState(self.per_process_config)
    shutil.rmtree(self.per_process_state.working_dir, ignore_errors=True)
    os.makedirs(self.per_process_state.working_dir)

    error_message = self.per_process_state.image_differ.CheckMissingTools(
        self.options.regenerate_expected)
    if error_message:
      print('FAILURE:', error_message)
      return 1

    self.resultdb = result_sink.TryInitClient()
    if self.resultdb:
      print('Detected ResultSink environment')

    # Collect test cases.
    walk_from_dir = finder.TestingDir(relative_test_dir)

    self.test_cases = TestCaseManager()
    self.execution_suppressed_cases = []
    input_file_re = re.compile('^.+[.](in|pdf)$')
    if self.options.inputted_file_paths:
      for file_name in self.options.inputted_file_paths:
        input_path = os.path.join(walk_from_dir, file_name)
        if not os.path.isfile(input_path):
          print(f"Can't find test file '{file_name}'")
          return 1

        self.test_cases.NewTestCase(input_path)
    else:
      for file_dir, _, filename_list in os.walk(walk_from_dir):
        for input_filename in filename_list:
          if input_file_re.match(input_filename):
            input_path = os.path.join(file_dir, input_filename)
            if self.IsExecutionSuppressed(input_path):
              self.execution_suppressed_cases.append(input_path)
              continue
            if not os.path.isfile(input_path):
              continue

            self.test_cases.NewTestCase(input_path)

    # Execute test cases.
    self.failures = []
    self.surprises = []
    self.skia_gold_successes = []
    self.skia_gold_unexpected_successes = []
    self.skia_gold_failures = []
    self.result_suppressed_cases = []

    if self.IsSkiaGoldEnabled():
      assert self.options.gold_output_dir
      # Clear out and create top level gold output directory before starting
      skia_gold.clear_gold_output_dir(self.options.gold_output_dir)

    with multiprocessing.Pool(
        processes=self.options.num_workers,
        initializer=_InitializePerProcessState,
        initargs=[self.per_process_config]) as pool:
      for result in pool.imap(_RunPdfiumTest, self.test_cases):
        self.HandleResult(self.test_cases.GetTestCase(result.test_id), result)

    # Report test results.
    #
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
    if self.IsSkiaGoldEnabled():
      number_gold_failures = len(self.skia_gold_failures)
      number_gold_successes = len(self.skia_gold_successes)
      number_gold_surprises = len(self.skia_gold_unexpected_successes)
      number_total_gold_tests = sum(
          [number_gold_failures, number_gold_successes, number_gold_surprises])
      print('\nSkia Gold Test cases executed:', number_total_gold_tests)
      print('  Skia Gold Successes:', number_gold_successes)
      print('  Skia Gold Surprises:', number_gold_surprises)
      print('  Skia Gold Failures:', number_gold_failures)
      skia_tester = self.per_process_state.GetSkiaGoldTester()
      if self.skia_gold_failures and skia_tester.IsTryjobRun():
        cl_triage_link = skia_tester.GetCLTriageLink()
        print('  Triage link for CL:', cl_triage_link)
        skia_tester.WriteCLTriageLink(cl_triage_link)
    print()
    print('Test cases not executed:', len(self.execution_suppressed_cases))

  def SetDeleteOutputOnSuccess(self, new_value):
    """Set whether to delete generated output if the test passes."""
    self.per_process_config.delete_output_on_success = new_value

  def SetEnforceExpectedImages(self, new_value):
    """Set whether to enforce that each test case provide an expected image."""
    self.per_process_config.enforce_expected_images = new_value


def _RunPdfiumTest(test_case):
  """Runs a PDFium test case."""
  try:
    with _TestTimer() as timer:
      timer.result = _per_process_state.GenerateAndTest(test_case)
      return timer.result
  except KeyboardInterrupt as exc:
    raise KeyboardInterruptError() from exc


# `_PerProcessState` singleton. This is initialized when creating the
# `multiprocessing.Pool()`. `TestRunner.Run()` creates its own separate
# instance of `_PerProcessState` as well.
_per_process_state = None


def _InitializePerProcessState(config):
  """Initializes the `_per_process_state` singleton."""
  global _per_process_state
  assert not _per_process_state
  _per_process_state = _PerProcessState(config)


@dataclass
class _PerProcessConfig:
  """Configuration for initializing `_PerProcessState`.

  Attributes:
    test_dir: The name of the test directory.
    test_type: The test type.
    delete_output_on_success: Whether to delete output on success.
    enforce_expected_images: Whether to enforce expected images.
    options: The dictionary of command line options.
    features: The list of features supported by `pdfium_test`.
  """
  test_dir: str
  test_type: str
  delete_output_on_success: bool = False
  enforce_expected_images: bool = False
  options: dict = None
  features: list = None

  def NewFinder(self):
    return common.DirectoryFinder(self.options.build_dir)

  def GetPdfiumTestPath(self, finder):
    return finder.ExecutablePath('pdfium_test')

  def InitializeFeatures(self, pdfium_test_path):
    output = subprocess.check_output([pdfium_test_path, '--show-config'])
    self.features = output.decode('utf-8').strip().split(',')


class _PerProcessState:
  """State defined per process."""

  def __init__(self, config):
    self.test_dir = config.test_dir
    self.test_type = config.test_type
    self.delete_output_on_success = config.delete_output_on_success
    self.enforce_expected_images = config.enforce_expected_images
    self.options = config.options
    self.features = config.features

    finder = config.NewFinder()
    self.pdfium_test_path = config.GetPdfiumTestPath(finder)
    self.fixup_path = finder.ScriptPath('fixup_pdf_template.py')
    self.text_diff_path = finder.ScriptPath('text_diff.py')
    self.font_dir = os.path.join(finder.TestingDir(), 'resources', 'fonts')
    self.third_party_font_dir = finder.ThirdPartyFontsDir()

    self.source_dir = finder.TestingDir()
    self.working_dir = finder.WorkingDir(os.path.join('testing', self.test_dir))

    self.test_suppressor = suppressor.Suppressor(
        finder, self.features, self.options.disable_javascript,
        self.options.disable_xfa)
    self.image_differ = pngdiffer.PNGDiffer(finder, self.features,
                                            self.options.reverse_byte_order)

    self.process_name = multiprocessing.current_process().name
    self.skia_tester = None

  def __getstate__(self):
    raise RuntimeError('Cannot pickle per-process state')

  def GetSkiaGoldTester(self):
    """Gets the `SkiaGoldTester` singleton for this worker."""
    if not self.skia_tester:
      self.skia_tester = skia_gold.SkiaGoldTester(
          source_type=self.test_type,
          skia_gold_args=self.options,
          process_name=self.process_name)
    return self.skia_tester

  def GenerateAndTest(self, test_case):
    """Generate test input and run pdfium_test."""
    source_dir, input_filename = os.path.split(test_case.input_path)
    pdf_path = os.path.join(self.working_dir, f'{test_case.test_id}.pdf')

    # Remove any existing generated images from previous runs.
    actual_images = self.image_differ.GetActualFiles(input_filename, source_dir,
                                                     self.working_dir)
    DeleteFiles(actual_images)

    sys.stdout.flush()

    raised_exception = self.Generate(source_dir, input_filename,
                                     test_case.test_id, pdf_path)

    if raised_exception is not None:
      print('FAILURE: {}; {}'.format(input_filename, raised_exception))
      return test_case.NewResult(result_types.FAIL)

    image_artifacts = None
    if self.test_type in TEXT_TESTS:
      expected_txt_path = os.path.join(source_dir,
                                       f'{test_case.test_id}_expected.txt')
      raised_exception = self.TestText(input_filename, test_case.test_id,
                                       expected_txt_path, pdf_path)
    else:
      raised_exception, image_artifacts = self.TestPixel(pdf_path, source_dir)

    if raised_exception is not None:
      print('FAILURE: {}; {}'.format(input_filename, raised_exception))
      return test_case.NewResult(
          result_types.FAIL, image_artifacts=image_artifacts)

    if actual_images:
      if self.image_differ.HasDifferences(input_filename, source_dir,
                                          self.working_dir):
        self.RegenerateIfNeeded_(input_filename, source_dir)
        return test_case.NewResult(
            result_types.FAIL, image_artifacts=image_artifacts)
    else:
      if (self.enforce_expected_images and
          not self.test_suppressor.IsImageDiffSuppressed(input_filename)):
        self.RegenerateIfNeeded_(input_filename, source_dir)
        print('FAILURE: {}; Missing expected images'.format(input_filename))
        return test_case.NewResult(
            result_types.FAIL, image_artifacts=image_artifacts)

    if self.delete_output_on_success:
      DeleteFiles(actual_images)
    return test_case.NewResult(
        result_types.PASS, image_artifacts=image_artifacts)

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
  def TestPixel(self, pdf_path, source_dir):
    cmd_to_run = [
        self.pdfium_test_path, '--send-events', '--png', '--md5',
        '--time=' + TEST_SEED_TIME
    ]

    if 'use_ahem' in source_dir or 'use_symbolneu' in source_dir:
      cmd_to_run.append('--font-dir=%s' % self.font_dir)
    else:
      cmd_to_run.append('--font-dir=%s' % self.third_party_font_dir)
      cmd_to_run.append('--croscore-font-names')

    if self.options.disable_javascript:
      cmd_to_run.append('--disable-javascript')

    if self.options.disable_xfa:
      cmd_to_run.append('--disable-xfa')

    if self.options.render_oneshot:
      cmd_to_run.append('--render-oneshot')

    if self.options.reverse_byte_order:
      cmd_to_run.append('--reverse-byte-order')

    cmd_to_run.append(pdf_path)

    raised_exception, results = common.RunCommandExtractHashedFiles(cmd_to_run)
    if results:
      results = [
          self._NewImageArtifact(image_path=image_path, md5_hash=md5_hash)
          for image_path, md5_hash in results
      ]
    return raised_exception, results

  def _NewImageArtifact(self, *, image_path, md5_hash):
    artifact = ImageArtifact(image_path=image_path, md5_hash=md5_hash)

    if self.options.run_skia_gold:
      if self.GetSkiaGoldTester().UploadTestResultToSkiaGold(
          artifact.GetSkiaGoldId(), artifact.image_path):
        artifact.skia_gold_status = result_types.PASS
      else:
        artifact.skia_gold_status = result_types.FAIL

    return artifact


@dataclass
class TestCase:
  """Description of a test case to run.

  Attributes:
    test_id: A unique identifier for the test.
    input_path: The absolute path to the test file.
  """
  test_id: str
  input_path: str

  def NewResult(self, status, **kwargs):
    """Derives a new test result corresponding to this test case."""
    return TestResult(test_id=self.test_id, status=status, **kwargs)


@dataclass
class TestResult:
  """Results from running a test case.

  Attributes:
    test_id: The corresponding test case ID.
    status: The overall `result_types` status.
    duration_milliseconds: Test time in milliseconds.
    image_artfacts: Optional list of image artifacts.
  """
  test_id: str
  status: str
  duration_milliseconds: float = None
  image_artifacts: list = None

  def IsPass(self):
    """Whether the test passed."""
    return self.status == result_types.PASS


@dataclass
class ImageArtifact:
  """Image artifact for a test result.

  Attributes:
    image_path: The absolute path to the image file.
    md5_hash: The MD5 hash of the pixel buffer.
    skia_gold_status: Optional Skia Gold status.
  """
  image_path: str
  md5_hash: str
  skia_gold_status: str = None

  def GetSkiaGoldId(self):
    # The output filename without image extension becomes the test ID. For
    # example, "/path/to/.../testing/corpus/example_005.pdf.0.png" becomes
    # "example_005.pdf.0".
    return _GetTestId(os.path.basename(self.image_path))


class TestCaseManager:
  """Manages a collection of test cases."""

  def __init__(self):
    self.test_cases = {}

  def __len__(self):
    return len(self.test_cases)

  def __iter__(self):
    return iter(self.test_cases.values())

  def NewTestCase(self, input_path, **kwargs):
    """Creates and registers a new test case."""
    input_basename = os.path.basename(input_path)
    test_id = _GetTestId(input_basename)
    if test_id in self.test_cases:
      raise ValueError(
          f'Test ID "{test_id}" derived from "{input_basename}" must be unique')

    test_case = TestCase(test_id=test_id, input_path=input_path, **kwargs)
    self.test_cases[test_id] = test_case
    return test_case

  def GetTestCase(self, test_id):
    """Looks up a test case previously registered by `NewTestCase()`."""
    return self.test_cases[test_id]


def _GetTestId(input_basename):
  """Constructs a test ID by stripping the last extension from the basename."""
  return os.path.splitext(input_basename)[0]


class _TestTimer:
  """Context manager for measuring the duration of a test."""

  def __init__(self):
    self.duration_start = 0
    self.result = None

  def __enter__(self):
    self.duration_start = time.perf_counter_ns()
    return self

  def __exit__(self, exc_type, exc_value, traceback):
    if not self.result:
      return
    duration = time.perf_counter_ns() - self.duration_start
    self.result.duration_milliseconds = duration * 1e-6
