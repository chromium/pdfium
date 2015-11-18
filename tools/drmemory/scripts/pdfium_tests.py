#!/usr/bin/env python
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

''' Runs various chrome tests through valgrind_test.py.'''

import glob
import logging
import optparse
import os
import subprocess
import sys

import logging_utils
import path_utils

import common
import valgrind_test

class TestNotFound(Exception): pass

class MultipleGTestFiltersSpecified(Exception): pass

class BuildDirNotFound(Exception): pass

class BuildDirAmbiguous(Exception): pass

class ExecutableNotFound(Exception): pass

class BadBinary(Exception): pass

class ChromeTests:
  SLOW_TOOLS = ["drmemory"]

  def __init__(self, options, args, test):
    if ':' in test:
      (self._test, self._gtest_filter) = test.split(':', 1)
    else:
      self._test = test
      self._gtest_filter = options.gtest_filter

    if self._test not in self._test_list:
      raise TestNotFound("Unknown test: %s" % test)

    if options.gtest_filter and options.gtest_filter != self._gtest_filter:
      raise MultipleGTestFiltersSpecified("Can not specify both --gtest_filter "
                                          "and --test %s" % test)

    self._options = options
    self._args = args

    script_dir = path_utils.ScriptDir()
    # Compute the top of the tree (the "source dir") from the script dir (where
    # this script lives).  We assume that the script dir is in tools/valgrind/
    # relative to the top of the tree.
    self._source_dir = os.path.dirname(os.path.dirname(script_dir))
    # since this path is used for string matching, make sure it's always
    # an absolute Unix-style path
    self._source_dir = os.path.abspath(self._source_dir).replace('\\', '/')
    valgrind_test_script = os.path.join(script_dir, "valgrind_test.py")
    self._command_preamble = ["--source-dir=%s" % (self._source_dir)]

    if not self._options.build_dir:
      dirs = [
        os.path.join(self._source_dir, "xcodebuild", "Debug"),
        os.path.join(self._source_dir, "out", "Debug"),
        os.path.join(self._source_dir, "build", "Debug"),
      ]
      build_dir = [d for d in dirs if os.path.isdir(d)]
      if len(build_dir) > 1:
        raise BuildDirAmbiguous("Found more than one suitable build dir:\n"
                                "%s\nPlease specify just one "
                                "using --build-dir" % ", ".join(build_dir))
      elif build_dir:
        self._options.build_dir = build_dir[0]
      else:
        self._options.build_dir = None

    if self._options.build_dir:
      build_dir = os.path.abspath(self._options.build_dir)
      self._command_preamble += ["--build-dir=%s" % (self._options.build_dir)]

  def _EnsureBuildDirFound(self):
    if not self._options.build_dir:
      raise BuildDirNotFound("Oops, couldn't find a build dir, please "
                             "specify it manually using --build-dir")

  def _DefaultCommand(self, tool, exe=None, valgrind_test_args=None):
    '''Generates the default command array that most tests will use.'''
    if exe and common.IsWindows():
      exe += '.exe'

    cmd = list(self._command_preamble)

    # Find all suppressions matching the following pattern:
    # tools/valgrind/TOOL/suppressions[_PLATFORM].txt
    # and list them with --suppressions= prefix.
    script_dir = path_utils.ScriptDir()
    tool_name = tool.ToolName();
    suppression_file = os.path.join(script_dir, tool_name, "suppressions.txt")
    if os.path.exists(suppression_file):
      cmd.append("--suppressions=%s" % suppression_file)
    # Platform-specific suppression
    for platform in common.PlatformNames():
      platform_suppression_file = \
          os.path.join(script_dir, tool_name, 'suppressions_%s.txt' % platform)
      if os.path.exists(platform_suppression_file):
        cmd.append("--suppressions=%s" % platform_suppression_file)

    if self._options.valgrind_tool_flags:
      cmd += self._options.valgrind_tool_flags.split(" ")
    if self._options.keep_logs:
      cmd += ["--keep_logs"]
    if valgrind_test_args != None:
      for arg in valgrind_test_args:
        cmd.append(arg)
    if exe:
      self._EnsureBuildDirFound()
      exe_path = os.path.join(self._options.build_dir, exe)
      if not os.path.exists(exe_path):
        raise ExecutableNotFound("Couldn't find '%s'" % exe_path)

      # Make sure we don't try to test ASan-built binaries
      # with other dynamic instrumentation-based tools.
      # TODO(timurrrr): also check TSan and MSan?
      # `nm` might not be available, so use try-except.
      try:
        # Do not perform this check on OS X, as 'nm' on 10.6 can't handle
        # binaries built with Clang 3.5+.
        if not common.IsMac():
          nm_output = subprocess.check_output(["nm", exe_path])
          if nm_output.find("__asan_init") != -1:
            raise BadBinary("You're trying to run an executable instrumented "
                            "with AddressSanitizer under %s. Please provide "
                            "an uninstrumented executable." % tool_name)
      except OSError:
        pass

      cmd.append(exe_path)
      # Valgrind runs tests slowly, so slow tests hurt more; show elapased time
      # so we can find the slowpokes.
      cmd.append("--gtest_print_time")
      # Built-in test launcher for gtest-based executables runs tests using
      # multiple process by default. Force the single-process mode back.
      cmd.append("--single-process-tests")
    if self._options.gtest_repeat:
      cmd.append("--gtest_repeat=%s" % self._options.gtest_repeat)
    if self._options.gtest_shuffle:
      cmd.append("--gtest_shuffle")
    if self._options.gtest_break_on_failure:
      cmd.append("--gtest_break_on_failure")
    if self._options.test_launcher_bot_mode:
      cmd.append("--test-launcher-bot-mode")
    if self._options.test_launcher_total_shards is not None:
      cmd.append("--test-launcher-total-shards=%d" % self._options.test_launcher_total_shards)
    if self._options.test_launcher_shard_index is not None:
      cmd.append("--test-launcher-shard-index=%d" % self._options.test_launcher_shard_index)
    return cmd

  def Run(self):
    ''' Runs the test specified by command-line argument --test '''
    logging.info("running test %s" % (self._test))
    return self._test_list[self._test](self)

  def _AppendGtestFilter(self, tool, name, cmd):
    '''Append an appropriate --gtest_filter flag to the googletest binary
       invocation.
       If the user passed his own filter mentioning only one test, just use it.
       Othewise, filter out tests listed in the appropriate gtest_exclude files.
    '''
    if (self._gtest_filter and
        ":" not in self._gtest_filter and
        "?" not in self._gtest_filter and
        "*" not in self._gtest_filter):
      cmd.append("--gtest_filter=%s" % self._gtest_filter)
      return

    filters = []
    gtest_files_dir = os.path.join(path_utils.ScriptDir(), "gtest_exclude")

    gtest_filter_files = [
        os.path.join(gtest_files_dir, name + ".gtest-%s.txt" % tool.ToolName())]
    # Use ".gtest.txt" files only for slow tools, as they now contain
    # Valgrind- and Dr.Memory-specific filters.
    # TODO(glider): rename the files to ".gtest_slow.txt"
    if tool.ToolName() in ChromeTests.SLOW_TOOLS:
      gtest_filter_files += [os.path.join(gtest_files_dir, name + ".gtest.txt")]
    for platform_suffix in common.PlatformNames():
      gtest_filter_files += [
        os.path.join(gtest_files_dir, name + ".gtest_%s.txt" % platform_suffix),
        os.path.join(gtest_files_dir, name + ".gtest-%s_%s.txt" % \
            (tool.ToolName(), platform_suffix))]
    logging.info("Reading gtest exclude filter files:")
    for filename in gtest_filter_files:
      # strip the leading absolute path (may be very long on the bot)
      # and the following / or \.
      readable_filename = filename.replace("\\", "/")  # '\' on Windows
      readable_filename = readable_filename.replace(self._source_dir, "")[1:]
      if not os.path.exists(filename):
        logging.info("  \"%s\" - not found" % readable_filename)
        continue
      logging.info("  \"%s\" - OK" % readable_filename)
      f = open(filename, 'r')
      for line in f.readlines():
        if line.startswith("#") or line.startswith("//") or line.isspace():
          continue
        line = line.rstrip()
        test_prefixes = ["FLAKY", "FAILS"]
        for p in test_prefixes:
          # Strip prefixes from the test names.
          line = line.replace(".%s_" % p, ".")
        # Exclude the original test name.
        filters.append(line)
        if line[-2:] != ".*":
          # List all possible prefixes if line doesn't end with ".*".
          for p in test_prefixes:
            filters.append(line.replace(".", ".%s_" % p))
    # Get rid of duplicates.
    filters = set(filters)
    gtest_filter = self._gtest_filter
    if len(filters):
      if gtest_filter:
        gtest_filter += ":"
        if gtest_filter.find("-") < 0:
          gtest_filter += "-"
      else:
        gtest_filter = "-"
      gtest_filter += ":".join(filters)
    if gtest_filter:
      cmd.append("--gtest_filter=%s" % gtest_filter)

  @staticmethod
  def ShowTests():
    test_to_names = {}
    for name, test_function in ChromeTests._test_list.iteritems():
      test_to_names.setdefault(test_function, []).append(name)

    name_to_aliases = {}
    for names in test_to_names.itervalues():
      names.sort(key=lambda name: len(name))
      name_to_aliases[names[0]] = names[1:]

    print
    print "Available tests:"
    print "----------------"
    for name, aliases in sorted(name_to_aliases.iteritems()):
      if aliases:
        print "   {} (aka {})".format(name, ', '.join(aliases))
      else:
        print "   {}".format(name)

  def SetupLdPath(self, requires_build_dir):
    if requires_build_dir:
      self._EnsureBuildDirFound()
    elif not self._options.build_dir:
      return

    # Append build_dir to LD_LIBRARY_PATH so external libraries can be loaded.
    if (os.getenv("LD_LIBRARY_PATH")):
      os.putenv("LD_LIBRARY_PATH", "%s:%s" % (os.getenv("LD_LIBRARY_PATH"),
                                              self._options.build_dir))
    else:
      os.putenv("LD_LIBRARY_PATH", self._options.build_dir)

  def SimpleTest(self, module, name, valgrind_test_args=None, cmd_args=None):
    tool = valgrind_test.CreateTool(self._options.valgrind_tool)
    cmd = self._DefaultCommand(tool, name, valgrind_test_args)
    self._AppendGtestFilter(tool, name, cmd)
    cmd.extend(['--test-tiny-timeout=1000'])
    if cmd_args:
      cmd.extend(cmd_args)

    self.SetupLdPath(True)
    return tool.Run(cmd, module)

  def RunCmdLine(self):
    tool = valgrind_test.CreateTool(self._options.valgrind_tool)
    cmd = self._DefaultCommand(tool, None, self._args)
    self.SetupLdPath(False)
    return tool.Run(cmd, None)

  def TestPDFiumUnitTests(self):
    return self.SimpleTest("pdfium_unittests", "pdfium_unittests")

  def TestPDFiumEmbedderTests(self):
    return self.SimpleTest("pdfium_embeddertests", "pdfium_embeddertests")

  # The known list of tests.
  _test_list = {
    "cmdline" : RunCmdLine,
    "pdfium_unittests": TestPDFiumUnitTests,
    "pdfium_embeddertests": TestPDFiumEmbedderTests,
  }


def _main():
  parser = optparse.OptionParser("usage: %prog -b <dir> -t <test> "
                                 "[-t <test> ...]")

  parser.add_option("--help-tests", dest="help_tests", action="store_true",
                    default=False, help="List all available tests")
  parser.add_option("-b", "--build-dir",
                    help="the location of the compiler output")
  parser.add_option("--target", help="Debug or Release")
  parser.add_option("-t", "--test", action="append", default=[],
                    help="which test to run, supports test:gtest_filter format "
                         "as well.")
  parser.add_option("--gtest_filter",
                    help="additional arguments to --gtest_filter")
  parser.add_option("--gtest_repeat", help="argument for --gtest_repeat")
  parser.add_option("--gtest_shuffle", action="store_true", default=False,
                    help="Randomize tests' orders on every iteration.")
  parser.add_option("--gtest_break_on_failure", action="store_true",
                    default=False,
                    help="Drop in to debugger on assertion failure. Also "
                         "useful for forcing tests to exit with a stack dump "
                         "on the first assertion failure when running with "
                         "--gtest_repeat=-1")
  parser.add_option("-v", "--verbose", action="store_true", default=False,
                    help="verbose output - enable debug log messages")
  parser.add_option("--tool", dest="valgrind_tool", default="drmemory_full",
                    help="specify a valgrind tool to run the tests under")
  parser.add_option("--tool_flags", dest="valgrind_tool_flags", default="",
                    help="specify custom flags for the selected valgrind tool")
  parser.add_option("--keep_logs", action="store_true", default=False,
                    help="store memory tool logs in the <tool>.logs directory "
                         "instead of /tmp.\nThis can be useful for tool "
                         "developers/maintainers.\nPlease note that the <tool>"
                         ".logs directory will be clobbered on tool startup.")
  parser.add_option("--test-launcher-bot-mode", action="store_true",
                    help="run the tests with --test-launcher-bot-mode")
  parser.add_option("--test-launcher-total-shards", type=int,
                    help="run the tests with --test-launcher-total-shards")
  parser.add_option("--test-launcher-shard-index", type=int,
                    help="run the tests with --test-launcher-shard-index")

  options, args = parser.parse_args()

  # Bake target into build_dir.
  if options.target and options.build_dir:
    assert (options.target !=
            os.path.basename(os.path.dirname(options.build_dir)))
    options.build_dir = os.path.join(os.path.abspath(options.build_dir),
                                     options.target)

  if options.verbose:
    logging_utils.config_root(logging.DEBUG)
  else:
    logging_utils.config_root()

  if options.help_tests:
    ChromeTests.ShowTests()
    return 0

  if not options.test:
    parser.error("--test not specified")

  if len(options.test) != 1 and options.gtest_filter:
    parser.error("--gtest_filter and multiple tests don't make sense together")

  for t in options.test:
    tests = ChromeTests(options, args, t)
    ret = tests.Run()
    if ret: return ret
  return 0


if __name__ == "__main__":
  sys.exit(_main())
