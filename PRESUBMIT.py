# Copyright 2015 The PDFium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Presubmit script for pdfium.

See http://dev.chromium.org/developers/how-tos/depottools/presubmit-scripts
for more details about the presubmit API built into depot_tools.
"""

PRESUBMIT_VERSION = '2.0.0'

USE_PYTHON3 = True

LINT_FILTERS = [
  # Rvalue ref checks are unreliable.
  '-build/c++11',
  # Need to fix header names not matching cpp names.
  '-build/include_order',
  # Too many to fix at the moment.
  '-readability/casting',
  # Need to refactor large methods to fix.
  '-readability/fn_size',
  # Lots of usage to fix first.
  '-runtime/int',
  # Lots of non-const references need to be fixed
  '-runtime/references',
  # We are not thread safe, so this will never pass.
  '-runtime/threadsafe_fn',
  # Figure out how to deal with #defines that git cl format creates.
  '-whitespace/indent',
]


_INCLUDE_ORDER_WARNING = (
    'Your #include order seems to be broken. Remember to use the right '
    'collation (LC_COLLATE=C) and check\nhttps://google.github.io/styleguide/'
    'cppguide.html#Names_and_Order_of_Includes')


# Bypass the AUTHORS check for these accounts.
_KNOWN_ROBOTS = set() | set(
    '%s@skia-public.iam.gserviceaccount.com' % s for s in ('pdfium-autoroll',))

_THIRD_PARTY = 'third_party/'

# Format: Sequence of tuples containing:
# * String pattern or, if starting with a slash, a regular expression.
# * Sequence of strings to show when the pattern matches.
# * Error flag. True if a match is a presubmit error, otherwise it's a warning.
# * Sequence of paths to *not* check (regexps).
_BANNED_CPP_FUNCTIONS = (
    (
        r'/\busing namespace ',
        (
            'Using directives ("using namespace x") are banned by the Google',
            'Style Guide (',
            'https://google.github.io/styleguide/cppguide.html#Namespaces ).',
            'Explicitly qualify symbols or use using declarations ("using',
            'x::foo").',
        ),
        True,
        [_THIRD_PARTY],
    ),
    (
        r'/v8::Isolate::(?:|Try)GetCurrent\(\)',
        (
            'v8::Isolate::GetCurrent() and v8::Isolate::TryGetCurrent() are',
            'banned. Hold a pointer to the v8::Isolate that was entered. Use',
            'v8::Isolate::IsCurrent() to check whether a given v8::Isolate is',
            'entered.',
        ),
        True,
        (),
    ),
    (
        r'/\bmemcpy\(',
        ('Use FXSYS_memcpy() in place of memcpy().',),
        True,
        [_THIRD_PARTY],
    ),
    (
        r'/\bmemmove\(',
        ('Use FXSYS_memmove() in place of memmove().',),
        True,
        [_THIRD_PARTY],
    ),
    (
        r'/\bmemset\(',
        ('Use FXSYS_memset() in place of memset().',),
        True,
        [_THIRD_PARTY],
    ),
    (
        r'/\bmemclr\(',
        ('Use FXSYS_memclr() in place of memclr().',),
        True,
        [_THIRD_PARTY],
    ),
)


def _CheckNoBannedFunctions(input_api, output_api):
  """Makes sure that banned functions are not used."""
  warnings = []
  errors = []

  def _GetMessageForMatchingType(input_api, affected_file, line_number, line,
                                 type_name, message):
    """Returns an string composed of the name of the file, the line number where
    the match has been found and the additional text passed as `message` in case
    the target type name matches the text inside the line passed as parameter.
    """
    result = []

    if input_api.re.search(r"^ *//",
                           line):  # Ignore comments about banned types.
      return result
    if line.endswith(
        " nocheck"):  # A // nocheck comment will bypass this error.
      return result

    matched = False
    if type_name[0:1] == '/':
      regex = type_name[1:]
      if input_api.re.search(regex, line):
        matched = True
    elif type_name in line:
      matched = True

    if matched:
      result.append('    %s:%d:' % (affected_file.LocalPath(), line_number))
      for message_line in message:
        result.append('      %s' % message_line)

    return result

  def IsExcludedFile(affected_file, excluded_paths):
    local_path = affected_file.LocalPath()
    for item in excluded_paths:
      if input_api.re.match(item, local_path):
        return True
    return False

  def CheckForMatch(affected_file, line_num, line, func_name, message, error):
    problems = _GetMessageForMatchingType(input_api, f, line_num, line,
                                          func_name, message)
    if problems:
      if error:
        errors.extend(problems)
      else:
        warnings.extend(problems)

  file_filter = lambda f: f.LocalPath().endswith(('.cc', '.cpp', '.h'))
  for f in input_api.AffectedFiles(file_filter=file_filter):
    for line_num, line in f.ChangedContents():
      for func_name, message, error, excluded_paths in _BANNED_CPP_FUNCTIONS:
        if IsExcludedFile(f, excluded_paths):
          continue
        CheckForMatch(f, line_num, line, func_name, message, error)

  result = []
  if (warnings):
    result.append(
        output_api.PresubmitPromptWarning('Banned functions were used.\n' +
                                          '\n'.join(warnings)))
  if (errors):
    result.append(
        output_api.PresubmitError('Banned functions were used.\n' +
                                  '\n'.join(errors)))
  return result


def _CheckUnwantedDependencies(input_api, output_api):
  """Runs checkdeps on #include statements added in this
  change. Breaking - rules is an error, breaking ! rules is a
  warning.
  """
  import sys
  # We need to wait until we have an input_api object and use this
  # roundabout construct to import checkdeps because this file is
  # eval-ed and thus doesn't have __file__.
  original_sys_path = sys.path
  try:
    def GenerateCheckdepsPath(base_path):
      return input_api.os_path.join(base_path, 'buildtools', 'checkdeps')

    presubmit_path = input_api.PresubmitLocalPath()
    presubmit_parent_path = input_api.os_path.dirname(presubmit_path)
    not_standalone_pdfium = \
        input_api.os_path.basename(presubmit_parent_path) == "third_party" and \
        input_api.os_path.basename(presubmit_path) == "pdfium"

    sys.path.append(GenerateCheckdepsPath(presubmit_path))
    if not_standalone_pdfium:
      presubmit_grandparent_path = input_api.os_path.dirname(
          presubmit_parent_path)
      sys.path.append(GenerateCheckdepsPath(presubmit_grandparent_path))

    import checkdeps
    from cpp_checker import CppChecker
    from rules import Rule
  except ImportError:
    return [output_api.PresubmitError(
        'Unable to run checkdeps, does pdfium/buildtools/checkdeps exist?')]
  finally:
    # Restore sys.path to what it was before.
    sys.path = original_sys_path

  added_includes = []
  for f in input_api.AffectedFiles():
    if not CppChecker.IsCppFile(f.LocalPath()):
      continue

    changed_lines = [line for line_num, line in f.ChangedContents()]
    added_includes.append([f.LocalPath(), changed_lines])

  deps_checker = checkdeps.DepsChecker(input_api.PresubmitLocalPath())

  error_descriptions = []
  warning_descriptions = []
  for path, rule_type, rule_description in deps_checker.CheckAddedCppIncludes(
      added_includes):
    description_with_path = '%s\n    %s' % (path, rule_description)
    if rule_type == Rule.DISALLOW:
      error_descriptions.append(description_with_path)
    else:
      warning_descriptions.append(description_with_path)

  results = []
  if error_descriptions:
    results.append(output_api.PresubmitError(
        'You added one or more #includes that violate checkdeps rules.',
        error_descriptions))
  if warning_descriptions:
    results.append(output_api.PresubmitPromptOrNotify(
        'You added one or more #includes of files that are temporarily\n'
        'allowed but being removed. Can you avoid introducing the\n'
        '#include? See relevant DEPS file(s) for details and contacts.',
        warning_descriptions))
  return results


def _CheckIncludeOrderForScope(scope, input_api, file_path, changed_linenums):
  """Checks that the lines in scope occur in the right order.

  1. C system files in alphabetical order
  2. C++ system files in alphabetical order
  3. Project's .h files
  """

  c_system_include_pattern = input_api.re.compile(r'\s*#include <.*\.h>')
  cpp_system_include_pattern = input_api.re.compile(r'\s*#include <.*>')
  custom_include_pattern = input_api.re.compile(r'\s*#include ".*')

  C_SYSTEM_INCLUDES, CPP_SYSTEM_INCLUDES, CUSTOM_INCLUDES = range(3)

  state = C_SYSTEM_INCLUDES

  previous_line = ''
  previous_line_num = 0
  problem_linenums = []
  out_of_order = " - line belongs before previous line"
  for line_num, line in scope:
    if c_system_include_pattern.match(line):
      if state != C_SYSTEM_INCLUDES:
        problem_linenums.append((line_num, previous_line_num,
            " - C system include file in wrong block"))
      elif previous_line and previous_line > line:
        problem_linenums.append((line_num, previous_line_num,
            out_of_order))
    elif cpp_system_include_pattern.match(line):
      if state == C_SYSTEM_INCLUDES:
        state = CPP_SYSTEM_INCLUDES
      elif state == CUSTOM_INCLUDES:
        problem_linenums.append((line_num, previous_line_num,
            " - c++ system include file in wrong block"))
      elif previous_line and previous_line > line:
        problem_linenums.append((line_num, previous_line_num, out_of_order))
    elif custom_include_pattern.match(line):
      if state != CUSTOM_INCLUDES:
        state = CUSTOM_INCLUDES
      elif previous_line and previous_line > line:
        problem_linenums.append((line_num, previous_line_num, out_of_order))
    else:
      problem_linenums.append((line_num, previous_line_num,
          "Unknown include type"))
    previous_line = line
    previous_line_num = line_num

  warnings = []
  for (line_num, previous_line_num, failure_type) in problem_linenums:
    if line_num in changed_linenums or previous_line_num in changed_linenums:
      warnings.append('    %s:%d:%s' % (file_path, line_num, failure_type))
  return warnings


def _CheckIncludeOrderInFile(input_api, f, changed_linenums):
  """Checks the #include order for the given file f."""

  system_include_pattern = input_api.re.compile(r'\s*#include \<.*')
  # Exclude the following includes from the check:
  # 1) #include <.../...>, e.g., <sys/...> includes often need to appear in a
  # specific order.
  # 2) <atlbase.h>, "build/build_config.h"
  excluded_include_pattern = input_api.re.compile(
      r'\s*#include (\<.*/.*|\<atlbase\.h\>|"build/build_config.h")')
  custom_include_pattern = input_api.re.compile(r'\s*#include "(?P<FILE>.*)"')
  # Match the final or penultimate token if it is xxxtest so we can ignore it
  # when considering the special first include.
  test_file_tag_pattern = input_api.re.compile(
    r'_[a-z]+test(?=(_[a-zA-Z0-9]+)?\.)')
  if_pattern = input_api.re.compile(
      r'\s*#\s*(if|elif|else|endif|define|undef).*')
  # Some files need specialized order of includes; exclude such files from this
  # check.
  uncheckable_includes_pattern = input_api.re.compile(
      r'\s*#include '
      '("ipc/.*macros\.h"|<windows\.h>|".*gl.*autogen.h")\s*')

  contents = f.NewContents()
  warnings = []
  line_num = 0

  # Handle the special first include. If the first include file is
  # some/path/file.h, the corresponding including file can be some/path/file.cc,
  # some/other/path/file.cc, some/path/file_platform.cc, some/path/file-suffix.h
  # etc. It's also possible that no special first include exists.
  # If the included file is some/path/file_platform.h the including file could
  # also be some/path/file_xxxtest_platform.h.
  including_file_base_name = test_file_tag_pattern.sub(
    '', input_api.os_path.basename(f.LocalPath()))

  for line in contents:
    line_num += 1
    if system_include_pattern.match(line):
      # No special first include -> process the line again along with normal
      # includes.
      line_num -= 1
      break
    match = custom_include_pattern.match(line)
    if match:
      match_dict = match.groupdict()
      header_basename = test_file_tag_pattern.sub(
        '', input_api.os_path.basename(match_dict['FILE'])).replace('.h', '')

      if header_basename not in including_file_base_name:
        # No special first include -> process the line again along with normal
        # includes.
        line_num -= 1
      break

  # Split into scopes: Each region between #if and #endif is its own scope.
  scopes = []
  current_scope = []
  for line in contents[line_num:]:
    line_num += 1
    if uncheckable_includes_pattern.match(line):
      continue
    if if_pattern.match(line):
      scopes.append(current_scope)
      current_scope = []
    elif ((system_include_pattern.match(line) or
           custom_include_pattern.match(line)) and
          not excluded_include_pattern.match(line)):
      current_scope.append((line_num, line))
  scopes.append(current_scope)

  for scope in scopes:
    warnings.extend(_CheckIncludeOrderForScope(scope, input_api, f.LocalPath(),
                                               changed_linenums))
  return warnings


def _CheckIncludeOrder(input_api, output_api):
  """Checks that the #include order is correct.

  1. The corresponding header for source files.
  2. C system files in alphabetical order
  3. C++ system files in alphabetical order
  4. Project's .h files in alphabetical order

  Each region separated by #if, #elif, #else, #endif, #define and #undef follows
  these rules separately.
  """
  warnings = []
  for f in input_api.AffectedFiles(file_filter=input_api.FilterSourceFile):
    if f.LocalPath().endswith(('.cc', '.cpp', '.h', '.mm')):
      changed_linenums = set(line_num for line_num, _ in f.ChangedContents())
      warnings.extend(_CheckIncludeOrderInFile(input_api, f, changed_linenums))

  results = []
  if warnings:
    results.append(output_api.PresubmitPromptOrNotify(_INCLUDE_ORDER_WARNING,
                                                      warnings))
  return results


def _CheckLibcxxRevision(input_api, output_api):
  """Makes sure that libcxx_revision is set correctly."""
  if 'DEPS' not in [f.LocalPath() for f in input_api.AffectedFiles()]:
    return []

  script_path = input_api.os_path.join('testing', 'tools', 'libcxx_check.py')
  buildtools_deps_path = input_api.os_path.join('buildtools',
                                                'deps_revisions.gni')

  try:
    errors = input_api.subprocess.check_output(
        [script_path, 'DEPS', buildtools_deps_path])
  except input_api.subprocess.CalledProcessError as error:
    msg = 'libcxx_check.py failed:'
    long_text = error.output.decode('utf-8', 'ignore')
    return [output_api.PresubmitError(msg, long_text=long_text)]

  if errors:
    return [output_api.PresubmitError(errors)]
  return []


def _CheckTestDuplicates(input_api, output_api):
  """Checks that pixel and javascript tests don't contain duplicates.
  We use .in and .pdf files, having both can cause race conditions on the bots,
  which run the tests in parallel.
  """
  tests_added = []
  results = []
  for f in input_api.AffectedFiles():
    if f.Action() == 'D':
      continue
    if not f.LocalPath().startswith(('testing/resources/pixel/',
        'testing/resources/javascript/')):
      continue
    end_len = 0
    if f.LocalPath().endswith('.in'):
      end_len = 3
    elif f.LocalPath().endswith('.pdf'):
      end_len = 4
    else:
      continue
    path = f.LocalPath()[:-end_len]
    if path in tests_added:
      results.append(output_api.PresubmitError(
          'Remove %s to prevent shadowing %s' % (path + '.pdf',
            path + '.in')))
    else:
      tests_added.append(path)
  return results


def _CheckPngNames(input_api, output_api):
  """Checks that .png files have the right file name format, which must be in
  the form:

  NAME_expected(_gdi)?(_(agg|skia))?(_(linux|mac|win))?.pdf.\d+.png

  This must be the same format as the one in testing/corpus's PRESUBMIT.py.
  """
  expected_pattern = input_api.re.compile(
      r'.+_expected(_gdi)?(_(agg|skia))?(_(linux|mac|win))?\.pdf\.\d+.png')
  results = []
  for f in input_api.AffectedFiles(include_deletes=False):
    if not f.LocalPath().endswith('.png'):
      continue
    if expected_pattern.match(f.LocalPath()):
      continue
    results.append(
        output_api.PresubmitError(
            'PNG file %s does not have the correct format' % f.LocalPath()))
  return results


def _CheckUselessForwardDeclarations(input_api, output_api):
  """Checks that added or removed lines in non third party affected
     header files do not lead to new useless class or struct forward
     declaration.
  """
  results = []
  class_pattern = input_api.re.compile(r'^class\s+(\w+);$',
                                       input_api.re.MULTILINE)
  struct_pattern = input_api.re.compile(r'^struct\s+(\w+);$',
                                        input_api.re.MULTILINE)
  for f in input_api.AffectedFiles(include_deletes=False):
    if f.LocalPath().startswith('third_party'):
      continue

    if not f.LocalPath().endswith('.h'):
      continue

    contents = input_api.ReadFile(f)
    fwd_decls = input_api.re.findall(class_pattern, contents)
    fwd_decls.extend(input_api.re.findall(struct_pattern, contents))

    useless_fwd_decls = []
    for decl in fwd_decls:
      count = sum(
          1
          for _ in input_api.re.finditer(r'\b%s\b' %
                                         input_api.re.escape(decl), contents))
      if count == 1:
        useless_fwd_decls.append(decl)

    if not useless_fwd_decls:
      continue

    for line in f.GenerateScmDiff().splitlines():
      if (line.startswith('-') and not line.startswith('--') or
          line.startswith('+') and not line.startswith('++')):
        for decl in useless_fwd_decls:
          if input_api.re.search(r'\b%s\b' % decl, line[1:]):
            results.append(
                output_api.PresubmitPromptWarning(
                    '%s: %s forward declaration is no longer needed' %
                    (f.LocalPath(), decl)))
            useless_fwd_decls.remove(decl)

  return results


def ChecksCommon(input_api, output_api):
  results = []

  results.extend(
      input_api.canned_checks.PanProjectChecks(
          input_api, output_api, project_name='PDFium'))
  results.extend(_CheckUnwantedDependencies(input_api, output_api))

  # PanProjectChecks() doesn't consider .gn/.gni files, so check those, too.
  files_to_check = (
      r'.*\.gn$',
      r'.*\.gni$',
  )
  results.extend(
      input_api.canned_checks.CheckLicense(
          input_api,
          output_api,
          project_name='PDFium',
          source_file_filter=lambda x: input_api.FilterSourceFile(
              x, files_to_check=files_to_check)))
  results.extend(
      input_api.canned_checks.CheckInclusiveLanguage(input_api, output_api))

  return results


def CheckChangeOnUpload(input_api, output_api):
  results = []
  results.extend(_CheckNoBannedFunctions(input_api, output_api))
  results.extend(
      input_api.canned_checks.CheckPatchFormatted(input_api, output_api))
  results.extend(
      input_api.canned_checks.CheckChangeLintsClean(
          input_api, output_api, lint_filters=LINT_FILTERS))
  results.extend(_CheckIncludeOrder(input_api, output_api))
  results.extend(_CheckLibcxxRevision(input_api, output_api))
  results.extend(_CheckTestDuplicates(input_api, output_api))
  results.extend(_CheckPngNames(input_api, output_api))
  results.extend(_CheckUselessForwardDeclarations(input_api, output_api))

  author = input_api.change.author_email
  if author and author not in _KNOWN_ROBOTS:
    results.extend(
        input_api.canned_checks.CheckAuthorizedAuthor(input_api, output_api))

  for f in input_api.AffectedFiles():
    path, name = input_api.os_path.split(f.LocalPath())
    if name == 'PRESUBMIT.py':
      full_path = input_api.os_path.join(input_api.PresubmitLocalPath(), path)
      test_file = input_api.os_path.join(path, 'PRESUBMIT_test.py')
      if f.Action() != 'D' and input_api.os_path.exists(test_file):
        # The PRESUBMIT.py file (and the directory containing it) might
        # have been affected by being moved or removed, so only try to
        # run the tests if they still exist.
        results.extend(
            input_api.canned_checks.RunUnitTestsInDirectory(
                input_api,
                output_api,
                full_path,
                files_to_check=[r'^PRESUBMIT_test\.py$'],
                run_on_python2=not USE_PYTHON3,
                run_on_python3=USE_PYTHON3,
                skip_shebang_check=True))

  return results
