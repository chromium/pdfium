#!/usr/bin/env python3
# Copyright 2015 The PDFium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import datetime
import glob
import os
import re
import subprocess
import sys

import pdfium_root


def os_name():
  if sys.platform.startswith('linux'):
    return 'linux'
  if sys.platform.startswith('win'):
    return 'win'
  if sys.platform.startswith('darwin'):
    return 'mac'
  raise Exception('Confused, can not determine OS, aborting.')


def RunCommandPropagateErr(cmd,
                           stdout_has_errors=False,
                           exit_status_on_error=None):
  """Run a command as a subprocess.

  Errors in that subprocess are printed out if it returns an error exit code.

  Args:
    cmd: Command to run as a list of strings.
    stdout_has_errors: Whether to print stdout instead of stderr on an error
        exit.
    exit_status_on_error: If specified, upon an error in the subprocess the
        caller script exits immediately with the given status.
  """
  p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
  output, err = p.communicate()

  if p.returncode:
    PrintErr('\nError when invoking "%s"' % ' '.join(cmd))
    if stdout_has_errors:
      PrintErr(output)

    PrintErr(err)

    if exit_status_on_error is not None:
      sys.exit(exit_status_on_error)

    return None

  return output


class DirectoryFinder:
  '''A class for finding directories and paths under either a standalone
  checkout or a chromium checkout of PDFium.'''

  def __init__(self, build_location):
    # `build_location` is typically "out/Debug" or "out/Release".
    root_finder = pdfium_root.RootDirectoryFinder()
    self.testing_dir = os.path.join(root_finder.pdfium_root, 'testing')
    self.my_dir = os.path.join(self.testing_dir, 'tools')
    self.pdfium_dir = root_finder.pdfium_root
    self.base_dir = root_finder.source_root
    self.build_dir = os.path.join(self.base_dir, build_location)
    self.os_name = os_name()

  def ExecutablePath(self, name):
    '''Finds compiled binaries under the build path.'''
    result = os.path.join(self.build_dir, name)
    if self.os_name == 'win':
      result = result + '.exe'
    return result

  def ScriptPath(self, name):
    '''Finds other scripts in the same directory as this one.'''
    return os.path.join(self.my_dir, name)

  def WorkingDir(self, other_components=''):
    '''Places generated files under the build directory, not source dir.'''
    result = os.path.join(self.build_dir, 'gen', 'pdfium')
    if other_components:
      result = os.path.join(result, other_components)
    return result

  def TestingDir(self, other_components=''):
    '''Finds test files somewhere under the testing directory.'''
    result = self.testing_dir
    if other_components:
      result = os.path.join(result, other_components)
    return result

  def ThirdPartyFontsDir(self):
    '''Finds directory with the test fonts.'''
    return os.path.join(self.base_dir, 'third_party', 'test_fonts')


def GetBooleanGnArg(arg_name, build_dir, verbose=False):
  '''Extract the value of a boolean flag in args.gn'''
  cwd = os.getcwd()
  os.chdir(build_dir)
  gn_args_output = subprocess.check_output(
      ['gn', 'args', '.', '--list=%s' % arg_name, '--short']).decode('utf8')
  os.chdir(cwd)
  arg_match_output = re.search('%s = (.*)' % arg_name, gn_args_output).group(1)
  if verbose:
    print(
        "Found '%s' for value of %s" % (arg_match_output, arg_name),
        file=sys.stderr)
  return arg_match_output == 'true'


def PrintWithTime(s):
  """Prints s prepended by a timestamp."""
  print('[%s] %s' % (datetime.datetime.now().strftime("%Y%m%d %H:%M:%S"), s))


def PrintErr(s):
  """Prints s to stderr."""
  print(s, file=sys.stderr)
