#!/usr/bin/env python
# Copyright 2017 The PDFium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Generates a coverage report for given tests.

Requires that 'use_clang_coverage = true' is set in args.gn.
Prefers that 'is_component_build = false' is set in args.gn.
"""

import argparse
from collections import namedtuple
import fnmatch
import os
import pprint
import subprocess
import sys

# Add src dir to path to avoid having to set PYTHONPATH.
sys.path.append(
    os.path.abspath(
        os.path.join(
            os.path.dirname(__file__), os.path.pardir, os.path.pardir,
            os.path.pardir)))

from testing.tools.common import GetBooleanGnArg

# 'binary' is the file that is to be run for the test.
# 'use_test_runner' indicates if 'binary' depends on test_runner.py and thus
# requires special handling.
# 'opt_args' are optional arguments to pass to the test 'binary'.
TestSpec = namedtuple('TestSpec', 'binary, use_test_runner, opt_args')

# All of the coverage tests that the script knows how to run.
COVERAGE_TESTS = {
    'pdfium_unittests':
        TestSpec('pdfium_unittests', False, []),
    'pdfium_embeddertests':
        TestSpec('pdfium_embeddertests', False, []),
    'corpus_tests':
        TestSpec('run_corpus_tests.py', True, []),
    'corpus_tests_javascript_disabled':
        TestSpec('run_corpus_tests.py', True, ['--disable-javascript']),
    'corpus_tests_xfa_disabled':
        TestSpec('run_corpus_tests.py', True, ['--disable-xfa']),
    'javascript_tests':
        TestSpec('run_javascript_tests.py', True, []),
    'javascript_tests_javascript_disabled':
        TestSpec('run_javascript_tests.py', True, ['--disable-javascript']),
    'javascript_tests_xfa_disabled':
        TestSpec('run_javascript_tests.py', True, ['--disable-xfa']),
    'pixel_tests':
        TestSpec('run_pixel_tests.py', True, []),
    'pixel_tests_javascript_disabled':
        TestSpec('run_pixel_tests.py', True, ['--disable-javascript']),
    'pixel_tests_xfa_disabled':
        TestSpec('run_pixel_tests.py', True, ['--disable-xfa']),
}


class CoverageExecutor(object):

  def __init__(self, parser, args):
    """Initialize executor based on the current script environment

    Args:
        parser: argparse.ArgumentParser for handling improper inputs.
        args: Dictionary of arguments passed into the calling script.
    """
    self.dry_run = args['dry_run']
    self.verbose = args['verbose']

    self.source_directory = args['source_directory']
    if not os.path.isdir(self.source_directory):
      parser.error("'%s' needs to be a directory" % self.source_directory)

    self.llvm_directory = os.path.join(self.source_directory, 'third_party',
                                       'llvm-build', 'Release+Asserts', 'bin')
    if not os.path.isdir(self.llvm_directory):
      parser.error("Cannot find LLVM bin directory , expected it to be in '%s'"
                   % self.llvm_directory)

    self.build_directory = args['build_directory']
    if not os.path.isdir(self.build_directory):
      parser.error("'%s' needs to be a directory" % self.build_directory)

    (self.coverage_tests,
     self.build_targets) = self.calculate_coverage_tests(args)
    if not self.coverage_tests:
      parser.error(
          'No valid tests in set to be run. This is likely due to bad command '
          'line arguments')

    if not GetBooleanGnArg('use_clang_coverage', self.build_directory,
                           self.verbose):
      parser.error(
          'use_clang_coverage does not appear to be set to true for build, but '
          'is needed')

    self.use_goma = GetBooleanGnArg('use_goma', self.build_directory,
                                    self.verbose)

    self.output_directory = args['output_directory']
    if not os.path.exists(self.output_directory):
      if not self.dry_run:
        os.makedirs(self.output_directory)
    elif not os.path.isdir(self.output_directory):
      parser.error('%s exists, but is not a directory' % self.output_directory)
    elif len(os.listdir(self.output_directory)) > 0:
      parser.error('%s is not empty, cowardly refusing to continue' %
                   self.output_directory)

    self.prof_data = os.path.join(self.output_directory, 'pdfium.profdata')

  def check_output(self, args, dry_run=False, env=None):
    """Dry run aware wrapper of subprocess.check_output()"""
    if dry_run:
      print "Would have run '%s'" % ' '.join(args)
      return ''

    output = subprocess.check_output(args, env=env)

    if self.verbose:
      print "check_output(%s) returned '%s'" % (args, output)
    return output

  def call(self, args, dry_run=False, env=None):
    """Dry run aware wrapper of subprocess.call()"""
    if dry_run:
      print "Would have run '%s'" % ' '.join(args)
      return 0

    output = subprocess.call(args, env=env)

    if self.verbose:
      print 'call(%s) returned %s' % (args, output)
    return output

  def call_silent(self, args, dry_run=False, env=None):
    """Dry run aware wrapper of subprocess.call() that eats output from call"""
    if dry_run:
      print "Would have run '%s'" % ' '.join(args)
      return 0

    with open(os.devnull, 'w') as f:
      output = subprocess.call(args, env=env, stdout=f)

    if self.verbose:
      print 'call_silent(%s) returned %s' % (args, output)
    return output

  def calculate_coverage_tests(self, args):
    """Determine which tests should be run."""
    testing_tools_directory = os.path.join(self.source_directory, 'testing',
                                           'tools')
    tests = args['tests'] if args['tests'] else COVERAGE_TESTS.keys()
    coverage_tests = {}
    build_targets = set()
    for name in tests:
      test_spec = COVERAGE_TESTS[name]
      if test_spec.use_test_runner:
        binary_path = os.path.join(testing_tools_directory, test_spec.binary)
        build_targets.add('pdfium_diff')
        build_targets.add('pdfium_test')
      else:
        binary_path = os.path.join(self.build_directory, test_spec.binary)
        build_targets.add(name)
      coverage_tests[name] = TestSpec(binary_path, test_spec.use_test_runner,
                                      test_spec.opt_args)

    build_targets = list(build_targets)

    return coverage_tests, build_targets

  def build_binaries(self):
    """Build all the binaries that are going to be needed for coverage
    generation."""
    call_args = ['ninja']
    if self.use_goma:
      call_args += ['-j', '250']
    call_args += ['-C', self.build_directory]
    call_args += self.build_targets
    return self.call(call_args, dry_run=self.dry_run) == 0

  def generate_coverage(self, name, spec):
    """Generate the coverage data for a test

    Args:
        name: Name associated with the test to be run. This is used as a label
              in the coverage data, so should be unique across all of the tests
              being run.
        spec: Tuple containing the TestSpec.
    """
    if self.verbose:
      print "Generating coverage for test '%s', using data '%s'" % (name, spec)
    if not os.path.exists(spec.binary):
      print('Unable to generate coverage for %s, since it appears to not exist'
            ' @ %s') % (name, spec.binary)
      return False

    binary_args = [spec.binary]
    if spec.opt_args:
      binary_args.extend(spec.opt_args)
    profile_pattern_string = '%8m'
    expected_profraw_file = '%s.%s.profraw' % (name, profile_pattern_string)
    expected_profraw_path = os.path.join(self.output_directory,
                                         expected_profraw_file)

    env = {
        'LLVM_PROFILE_FILE': expected_profraw_path,
        'PATH': os.getenv('PATH') + os.pathsep + self.llvm_directory
    }

    if spec.use_test_runner:
      # Test runner performs multi-threading in the wrapper script, not the test
      # binary, so need to limit the number of instances of the binary being run
      # to the max value in LLVM_PROFILE_FILE, which is 8.
      binary_args.extend(['-j', '8', '--build-dir', self.build_directory])
    if self.call(binary_args, dry_run=self.dry_run, env=env) and self.verbose:
      print('Running %s appears to have failed, which might affect '
            'results') % spec.binary

    return True

  def merge_raw_coverage_results(self):
    """Merge raw coverage data sets into one one file for report generation."""
    llvm_profdata_bin = os.path.join(self.llvm_directory, 'llvm-profdata')

    raw_data = []
    raw_data_pattern = '*.profraw'
    for file_name in os.listdir(self.output_directory):
      if fnmatch.fnmatch(file_name, raw_data_pattern):
        raw_data.append(os.path.join(self.output_directory, file_name))

    return self.call(
        [llvm_profdata_bin, 'merge', '-o', self.prof_data, '-sparse=true'] +
        raw_data) == 0

  def generate_html_report(self):
    """Generate HTML report by calling upstream coverage.py"""
    coverage_bin = os.path.join(self.source_directory, 'tools', 'code_coverage',
                                'coverage.py')
    report_directory = os.path.join(self.output_directory, 'HTML')

    coverage_args = ['-p', self.prof_data]
    coverage_args += ['-b', self.build_directory]
    coverage_args += ['-o', report_directory]
    coverage_args += self.build_targets

    # Whitelist the directories of interest
    coverage_args += ['-f', 'core']
    coverage_args += ['-f', 'fpdfsdk']
    coverage_args += ['-f', 'fxbarcode']
    coverage_args += ['-f', 'fxjs']
    coverage_args += ['-f', 'public']
    coverage_args += ['-f', 'samples']
    coverage_args += ['-f', 'xfa']

    # Blacklist test files
    coverage_args += ['-i', '.*test.*']

    # Component view is only useful for Chromium
    coverage_args += ['--no-component-view']

    return self.call([coverage_bin] + coverage_args) == 0

  def run(self):
    """Setup environment, execute the tests and generate coverage report"""
    if not self.fetch_profiling_tools():
      print 'Unable to fetch profiling tools'
      return False

    if not self.build_binaries():
      print 'Failed to successfully build binaries'
      return False

    for name in self.coverage_tests.keys():
      if not self.generate_coverage(name, self.coverage_tests[name]):
        print 'Failed to successfully generate coverage data'
        return False

    if not self.merge_raw_coverage_results():
      print 'Failed to successfully merge raw coverage results'
      return False

    if not self.generate_html_report():
      print 'Failed to successfully generate HTML report'
      return False

    return True

  def fetch_profiling_tools(self):
    """Call coverage.py with no args to ensure profiling tools are present."""
    return self.call_silent(
        os.path.join(self.source_directory, 'tools', 'code_coverage',
                     'coverage.py')) == 0


def main():
  parser = argparse.ArgumentParser()
  parser.formatter_class = argparse.RawDescriptionHelpFormatter
  parser.description = 'Generates a coverage report for given tests.'

  parser.add_argument(
      '-s',
      '--source_directory',
      help='Location of PDFium source directory, defaults to CWD',
      default=os.getcwd())
  build_default = os.path.join('out', 'Coverage')
  parser.add_argument(
      '-b',
      '--build_directory',
      help=
      'Location of PDFium build directory with coverage enabled, defaults to '
      '%s under CWD' % build_default,
      default=os.path.join(os.getcwd(), build_default))
  output_default = 'coverage_report'
  parser.add_argument(
      '-o',
      '--output_directory',
      help='Location to write out coverage report to, defaults to %s under CWD '
      % output_default,
      default=os.path.join(os.getcwd(), output_default))
  parser.add_argument(
      '-n',
      '--dry-run',
      help='Output commands instead of executing them',
      action='store_true')
  parser.add_argument(
      '-v',
      '--verbose',
      help='Output additional diagnostic information',
      action='store_true')
  parser.add_argument(
      'tests',
      help='Tests to be run, defaults to all. Valid entries are %s' %
      COVERAGE_TESTS.keys(),
      nargs='*')

  args = vars(parser.parse_args())
  if args['verbose']:
    pprint.pprint(args)

  executor = CoverageExecutor(parser, args)
  if executor.run():
    return 0
  return 1


if __name__ == '__main__':
  sys.exit(main())
