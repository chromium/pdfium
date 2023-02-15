#!/usr/bin/env python3
# Copyright 2016 The PDFium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import argparse
from dataclasses import dataclass, field
from datetime import timedelta
from io import BytesIO
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

# Timeout (in seconds) for individual test commands.
# TODO(crbug.com/pdfium/1967): array_buffer.in is slow under MSan, so need a
# very generous 5 minute timeout for now.
TEST_TIMEOUT = timedelta(minutes=5).total_seconds()


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

    test_result.status = self._SuppressStatus(input_filename,
                                              test_result.status)
    if test_result.status == result_types.UNKNOWN:
      self.result_suppressed_cases.append(input_filename)
      self.surprises.append(test_case.input_path)
    elif test_result.status == result_types.SKIP:
      self.result_suppressed_cases.append(input_filename)
    elif not test_result.IsPass():
      self.failures.append(test_case.input_path)

    for artifact in test_result.image_artifacts:
      if artifact.skia_gold_status == result_types.PASS:
        if self.IsResultSuppressed(artifact.image_path):
          self.skia_gold_unexpected_successes.append(artifact.GetSkiaGoldId())
        else:
          self.skia_gold_successes.append(artifact.GetSkiaGoldId())
      elif artifact.skia_gold_status == result_types.FAIL:
        self.skia_gold_failures.append(artifact.GetSkiaGoldId())

    # Log test result.
    print(f'{test_result.status}: {test_result.test_id}')
    if not test_result.IsPass():
      if test_result.reason:
        print(f'Failure reason: {test_result.reason}')
      if test_result.log:
        decoded_log = bytes.decode(test_result.log, errors='backslashreplace')
        print(f'Test output:\n{decoded_log}')
      for artifact in test_result.image_artifacts:
        if artifact.skia_gold_status == result_types.FAIL:
          print(f'Failed Skia Gold: {artifact.image_path}')
        if artifact.image_diff:
          print(f'Failed image diff: {artifact.image_diff.reason}')

    # Report test result to ResultDB.
    if self.resultdb:
      only_artifacts = None
      only_failure_reason = test_result.reason
      if len(test_result.image_artifacts) == 1:
        only = test_result.image_artifacts[0]
        only_artifacts = only.GetDiffArtifacts()
        if only.GetDiffReason():
          only_failure_reason += f': {only.GetDiffReason()}'
      self.resultdb.Post(
          test_id=test_result.test_id,
          status=test_result.status,
          duration=test_result.duration_milliseconds,
          test_log=test_result.log,
          test_file=None,
          artifacts=only_artifacts,
          failure_reason=only_failure_reason)

      # Milo only supports a single diff per test, so if we have multiple pages,
      # report each page as its own "test."
      if len(test_result.image_artifacts) > 1:
        for page, artifact in enumerate(test_result.image_artifacts):
          self.resultdb.Post(
              test_id=f'{test_result.test_id}/{page}',
              status=self._SuppressArtifactStatus(test_result,
                                                  artifact.GetDiffStatus()),
              duration=None,
              test_log=None,
              test_file=None,
              artifacts=artifact.GetDiffArtifacts(),
              failure_reason=artifact.GetDiffReason())

  def _SuppressStatus(self, input_filename, status):
    if not self.IsResultSuppressed(input_filename):
      return status

    if status == result_types.PASS:
      # There isn't an actual status for succeeded-but-ignored, so use the
      # "abort" status to differentiate this from failed-but-ignored.
      #
      # Note that this appears as a preliminary failure in Gerrit.
      return result_types.UNKNOWN

    # There isn't an actual status for failed-but-ignored, so use the "skip"
    # status to differentiate this from succeeded-but-ignored.
    return result_types.SKIP

  def _SuppressArtifactStatus(self, test_result, status):
    if status != result_types.FAIL:
      return status

    if test_result.status != result_types.SKIP:
      return status

    return result_types.SKIP

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
        action='store_true',
        help='Prevents JavaScript from executing in PDF files.')

    parser.add_argument(
        '--disable-xfa',
        action='store_true',
        help='Prevents processing XFA forms.')

    parser.add_argument(
        '--render-oneshot',
        action='store_true',
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
        help='Key value pairs that are written to the top level of the JSON '
        'file that is ingested by Gold.')

    # TODO: Remove when pdfium recipe stops passing this argument
    parser.add_argument(
        '--gold_ignore_hashes',
        default='',
        help='Path to a file with MD5 hashes we wish to ignore.')

    parser.add_argument(
        '--regenerate_expected',
        action='store_true',
        help='Regenerates expected images. For each failing image diff, this '
        'will regenerate the most specific expected image file that exists. '
        'This also will suggest removals of unnecessary expected image files '
        'by renaming them with an additional ".bak" extension, although these '
        'removals should be reviewed manually. Use "git clean" to quickly deal '
        'with any ".bak" files.')

    parser.add_argument(
        '--reverse-byte-order',
        action='store_true',
        help='Run image-based tests using --reverse-byte-order.')

    parser.add_argument(
        '--ignore_errors',
        action='store_true',
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
      if self.per_process_config.test_type in TEXT_TESTS:
        test_function = _RunTextTest
      else:
        test_function = _RunPixelTest
      for result in pool.imap(test_function, self.test_cases):
        self.HandleResult(self.test_cases.GetTestCase(result.test_id), result)

    # Report test results.
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


def _RunTextTest(test_case):
  """Runs a text test case."""
  test_case_runner = _TestCaseRunner(test_case)
  with test_case_runner:
    test_case_runner.test_result = test_case_runner.GenerateAndTest(
        test_case_runner.TestText)
  return test_case_runner.test_result


def _RunPixelTest(test_case):
  """Runs a pixel test case."""
  test_case_runner = _TestCaseRunner(test_case)
  with test_case_runner:
    test_case_runner.test_result = test_case_runner.GenerateAndTest(
        test_case_runner.TestPixel)
  return test_case_runner.test_result


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
    output = subprocess.check_output([pdfium_test_path, '--show-config'],
                                     timeout=TEST_TIMEOUT)
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


class _TestCaseRunner:
  """Runner for a single test case."""

  def __init__(self, test_case):
    self.test_case = test_case
    self.test_result = None
    self.duration_start = 0

    self.source_dir, self.input_filename = os.path.split(
        self.test_case.input_path)
    self.pdf_path = os.path.join(self.working_dir, f'{self.test_id}.pdf')
    self.actual_images = None

  def __enter__(self):
    self.duration_start = time.perf_counter_ns()
    return self

  def __exit__(self, exc_type, exc_value, traceback):
    if not self.test_result:
      self.test_result = self.test_case.NewResult(
          result_types.UNKNOWN, reason='No test result recorded')
    duration = time.perf_counter_ns() - self.duration_start
    self.test_result.duration_milliseconds = duration * 1e-6

  @property
  def options(self):
    return _per_process_state.options

  @property
  def test_id(self):
    return self.test_case.test_id

  @property
  def working_dir(self):
    return _per_process_state.working_dir

  def IsResultSuppressed(self):
    return _per_process_state.test_suppressor.IsResultSuppressed(
        self.input_filename)

  def IsImageDiffSuppressed(self):
    return _per_process_state.test_suppressor.IsImageDiffSuppressed(
        self.input_filename)

  def RunCommand(self, command, stdout=None):
    """Runs a test command.

    Args:
      command: The list of command arguments.
      stdout: Optional `file`-like object to send standard output.

    Returns:
      The test result.
    """

    # Standard output and error are directed to the test log. If `stdout` was
    # provided, redirect standard output to it instead.
    if stdout:
      assert stdout != subprocess.PIPE
      try:
        stdout.fileno()
      except OSError:
        # `stdout` doesn't have a file descriptor, so it can't be passed to
        # `subprocess.run()` directly.
        original_stdout = stdout
        stdout = subprocess.PIPE
      stderr = subprocess.PIPE
    else:
      stdout = subprocess.PIPE
      stderr = subprocess.STDOUT

    test_result = self.test_case.NewResult(result_types.PASS)
    try:
      run_result = subprocess.run(
          command,
          stdout=stdout,
          stderr=stderr,
          timeout=TEST_TIMEOUT,
          check=False)
      if run_result.returncode != 0:
        test_result.status = result_types.FAIL
        test_result.reason = 'Command {} exited with code {}'.format(
            run_result.args, run_result.returncode)
    except subprocess.TimeoutExpired as timeout_expired:
      run_result = timeout_expired
      test_result.status = result_types.TIMEOUT
      test_result.reason = 'Command {} timed out'.format(run_result.cmd)

    if stdout == subprocess.PIPE and stderr == subprocess.PIPE:
      # Copy captured standard output, if any, to the original `stdout`.
      if run_result.stdout:
        original_stdout.write(run_result.stdout)

    if not test_result.IsPass():
      # On failure, report captured output to the test log.
      if stderr == subprocess.STDOUT:
        test_result.log = run_result.stdout
      else:
        test_result.log = run_result.stderr
    return test_result

  def GenerateAndTest(self, test_function):
    """Generate test input and run pdfium_test."""
    test_result = self.Generate()
    if not test_result.IsPass():
      return test_result

    return test_function()

  def _RegenerateIfNeeded(self):
    if not self.options.regenerate_expected:
      return
    if self.IsResultSuppressed() or self.IsImageDiffSuppressed():
      return
    _per_process_state.image_differ.Regenerate(self.input_filename,
                                               self.source_dir,
                                               self.working_dir)

  def Generate(self):
    input_event_path = os.path.join(self.source_dir, f'{self.test_id}.evt')
    if os.path.exists(input_event_path):
      output_event_path = f'{os.path.splitext(self.pdf_path)[0]}.evt'
      shutil.copyfile(input_event_path, output_event_path)

    template_path = os.path.join(self.source_dir, f'{self.test_id}.in')
    if not os.path.exists(template_path):
      if os.path.exists(self.test_case.input_path):
        shutil.copyfile(self.test_case.input_path, self.pdf_path)
      return self.test_case.NewResult(result_types.PASS)

    return self.RunCommand([
        sys.executable, _per_process_state.fixup_path,
        f'--output-dir={self.working_dir}', template_path
    ])

  def TestText(self):
    txt_path = os.path.join(self.working_dir, f'{self.test_id}.txt')
    with open(txt_path, 'w') as outfile:
      cmd_to_run = [
          _per_process_state.pdfium_test_path, '--send-events',
          f'--time={TEST_SEED_TIME}'
      ]

      if self.options.disable_javascript:
        cmd_to_run.append('--disable-javascript')

      if self.options.disable_xfa:
        cmd_to_run.append('--disable-xfa')

      cmd_to_run.append(self.pdf_path)
      test_result = self.RunCommand(cmd_to_run, stdout=outfile)
      if not test_result.IsPass():
        return test_result

    # If the expected file does not exist, the output is expected to be empty.
    expected_txt_path = os.path.join(self.source_dir,
                                     f'{self.test_id}_expected.txt')
    if not os.path.exists(expected_txt_path):
      return self._VerifyEmptyText(txt_path)

    # If JavaScript is disabled, the output should be empty.
    # However, if the test is suppressed and JavaScript is disabled, do not
    # verify that the text is empty so the suppressed test does not surprise.
    if self.options.disable_javascript and not self.IsResultSuppressed():
      return self._VerifyEmptyText(txt_path)

    return self.RunCommand([
        sys.executable, _per_process_state.text_diff_path, expected_txt_path,
        txt_path
    ])

  def _VerifyEmptyText(self, txt_path):
    with open(txt_path, "rb") as txt_file:
      txt_data = txt_file.read()

    if txt_data:
      return self.test_case.NewResult(
          result_types.FAIL, log=txt_data, reason=f'{txt_path} should be empty')

    return self.test_case.NewResult(result_types.PASS)

  # TODO(crbug.com/pdfium/1656): Remove when ready to fully switch over to
  # Skia Gold
  def TestPixel(self):
    # Remove any existing generated images from previous runs.
    self.actual_images = _per_process_state.image_differ.GetActualFiles(
        self.input_filename, self.source_dir, self.working_dir)
    self._CleanupPixelTest()

    # Generate images.
    cmd_to_run = [
        _per_process_state.pdfium_test_path, '--send-events', '--png', '--md5',
        f'--time={TEST_SEED_TIME}'
    ]

    if 'use_ahem' in self.source_dir or 'use_symbolneu' in self.source_dir:
      cmd_to_run.append(f'--font-dir={_per_process_state.font_dir}')
    else:
      cmd_to_run.append(f'--font-dir={_per_process_state.third_party_font_dir}')
      cmd_to_run.append('--croscore-font-names')

    if self.options.disable_javascript:
      cmd_to_run.append('--disable-javascript')

    if self.options.disable_xfa:
      cmd_to_run.append('--disable-xfa')

    if self.options.render_oneshot:
      cmd_to_run.append('--render-oneshot')

    if self.options.reverse_byte_order:
      cmd_to_run.append('--reverse-byte-order')

    cmd_to_run.append(self.pdf_path)

    with BytesIO() as command_output:
      test_result = self.RunCommand(cmd_to_run, stdout=command_output)
      if not test_result.IsPass():
        return test_result

      test_result.image_artifacts = []
      for line in command_output.getvalue().splitlines():
        # Expect this format: MD5:<path to image file>:<hexadecimal MD5 hash>
        line = bytes.decode(line).strip()
        if line.startswith('MD5:'):
          image_path, md5_hash = line[4:].rsplit(':', 1)
          test_result.image_artifacts.append(
              self._NewImageArtifact(
                  image_path=image_path.strip(), md5_hash=md5_hash.strip()))

    if self.actual_images:
      image_diffs = _per_process_state.image_differ.ComputeDifferences(
          self.input_filename, self.source_dir, self.working_dir)
      if image_diffs:
        test_result.status = result_types.FAIL
        test_result.reason = 'Images differ'

        # Merge image diffs into test result.
        diff_map = {}
        diff_log = []
        for diff in image_diffs:
          diff_map[diff.actual_path] = diff
          diff_log.append(f'{os.path.basename(diff.actual_path)} vs. ')
          if diff.expected_path:
            diff_log.append(f'{os.path.basename(diff.expected_path)}\n')
          else:
            diff_log.append('missing expected file\n')

        for artifact in test_result.image_artifacts:
          artifact.image_diff = diff_map.get(artifact.image_path)
        test_result.log = ''.join(diff_log).encode()

    elif _per_process_state.enforce_expected_images:
      if not self.IsImageDiffSuppressed():
        test_result.status = result_types.FAIL
        test_result.reason = 'Missing expected images'

    if not test_result.IsPass():
      self._RegenerateIfNeeded()
      return test_result

    if _per_process_state.delete_output_on_success:
      self._CleanupPixelTest()
    return test_result

  def _NewImageArtifact(self, *, image_path, md5_hash):
    artifact = ImageArtifact(image_path=image_path, md5_hash=md5_hash)

    if self.options.run_skia_gold:
      if _per_process_state.GetSkiaGoldTester().UploadTestResultToSkiaGold(
          artifact.GetSkiaGoldId(), artifact.image_path):
        artifact.skia_gold_status = result_types.PASS
      else:
        artifact.skia_gold_status = result_types.FAIL

    return artifact

  def _CleanupPixelTest(self):
    for image_file in self.actual_images:
      if os.path.exists(image_file):
        os.remove(image_file)


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
    log: Optional log of the test's output.
    image_artfacts: Optional list of image artifacts.
    reason: Optional reason why the test failed.
  """
  test_id: str
  status: str
  duration_milliseconds: float = None
  log: str = None
  image_artifacts: list = field(default_factory=list)
  reason: str = None

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
    image_diff: Optional image diff.
  """
  image_path: str
  md5_hash: str
  skia_gold_status: str = None
  image_diff: pngdiffer.ImageDiff = None

  def GetSkiaGoldId(self):
    # The output filename without image extension becomes the test ID. For
    # example, "/path/to/.../testing/corpus/example_005.pdf.0.png" becomes
    # "example_005.pdf.0".
    return _GetTestId(os.path.basename(self.image_path))

  def GetDiffStatus(self):
    return result_types.FAIL if self.image_diff else result_types.PASS

  def GetDiffReason(self):
    return self.image_diff.reason if self.image_diff else None

  def GetDiffArtifacts(self):
    if not self.image_diff:
      return None
    if not self.image_diff.expected_path or not self.image_diff.diff_path:
      return None
    return {
        'actual_image':
            _GetArtifactFromFilePath(self.image_path),
        'expected_image':
            _GetArtifactFromFilePath(self.image_diff.expected_path),
        'image_diff':
            _GetArtifactFromFilePath(self.image_diff.diff_path)
    }


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


def _GetArtifactFromFilePath(file_path):
  """Constructs a ResultSink artifact from a file path."""
  return {'filePath': file_path}
