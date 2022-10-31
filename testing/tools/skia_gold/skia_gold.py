# Copyright 2021 The PDFium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import logging
import shlex
import shutil

from . import pdfium_skia_gold_properties
from . import pdfium_skia_gold_session_manager

GS_BUCKET = 'skia-pdfium-gm'


def _ParseKeyValuePairs(kv_str):
  """
  Parses a string of the type 'key1 value1 key2 value2' into a dict.
  """
  kv_pairs = shlex.split(kv_str)
  if len(kv_pairs) % 2:
    raise ValueError('Uneven number of key/value pairs. Got %s' % kv_str)
  return {kv_pairs[i]: kv_pairs[i + 1] for i in range(0, len(kv_pairs), 2)}


def add_skia_gold_args(parser):
  group = parser.add_argument_group('Skia Gold Arguments')
  group.add_argument(
      '--git-revision', help='Revision being tested.', default=None)
  group.add_argument(
      '--gerrit-issue',
      help='For Skia Gold integration. Gerrit issue ID.',
      default='')
  group.add_argument(
      '--gerrit-patchset',
      help='For Skia Gold integration. Gerrit patch set number.',
      default='')
  group.add_argument(
      '--buildbucket-id',
      help='For Skia Gold integration. Buildbucket build ID.',
      default='')
  group.add_argument(
      '--bypass-skia-gold-functionality',
      action='store_true',
      default=False,
      help='Bypass all interaction with Skia Gold, effectively disabling the '
      'image comparison portion of any tests that use Gold. Only meant to '
      'be used in case a Gold outage occurs and cannot be fixed quickly.')
  local_group = group.add_mutually_exclusive_group()
  local_group.add_argument(
      '--local-pixel-tests',
      action='store_true',
      default=None,
      help='Specifies to run the test harness in local run mode or not. When '
      'run in local mode, uploading to Gold is disabled and links to '
      'help with local debugging are output. Running in local mode also '
      'implies --no-luci-auth. If both this and --no-local-pixel-tests are '
      'left unset, the test harness will attempt to detect whether it is '
      'running on a workstation or not and set this option accordingly.')
  local_group.add_argument(
      '--no-local-pixel-tests',
      action='store_false',
      dest='local_pixel_tests',
      help='Specifies to run the test harness in non-local (bot) mode. When '
      'run in this mode, data is actually uploaded to Gold and triage links '
      'arge generated. If both this and --local-pixel-tests are left unset, '
      'the test harness will attempt to detect whether it is running on a '
      'workstation or not and set this option accordingly.')
  group.add_argument(
      '--no-luci-auth',
      action='store_true',
      default=False,
      help='Don\'t use the service account provided by LUCI for '
      'authentication for Skia Gold, instead relying on gsutil to be '
      'pre-authenticated. Meant for testing locally instead of on the bots.')

  group.add_argument(
      '--gold_key',
      default='',
      dest="gold_key",
      help='Key value pairs of config data such like the hardware/software '
      'configuration the image was produced on.')
  group.add_argument(
      '--gold_output_dir',
      default='',
      dest="gold_output_dir",
      help='Path to the dir where diff output image files are saved, '
      'if running locally. If this is a tryjob run, will contain link to skia '
      'gold CL triage link. Required with --run-skia-gold.')


def clear_gold_output_dir(output_dir):
  # make sure the output directory exists and is empty.
  if os.path.exists(output_dir):
    shutil.rmtree(output_dir, ignore_errors=True)
  os.makedirs(output_dir)


class SkiaGoldTester:

  def __init__(self, source_type, skia_gold_args, process_name=None):
    """
    source_type: source_type (=corpus) field used for all results.
    skia_gold_args: Parsed arguments from argparse.ArgumentParser.
    process_name: Unique name of current process, if multiprocessing is on.
    """
    self._source_type = source_type
    self._output_dir = skia_gold_args.gold_output_dir
    # goldctl is not thread safe, so each process needs its own directory
    if process_name:
      self._output_dir = os.path.join(skia_gold_args.gold_output_dir,
                                      process_name)
      clear_gold_output_dir(self._output_dir)
    self._keys = _ParseKeyValuePairs(skia_gold_args.gold_key)
    self._old_gold_props = _ParseKeyValuePairs(skia_gold_args.gold_properties)
    self._skia_gold_args = skia_gold_args
    self._skia_gold_session_manager = None
    self._skia_gold_properties = None

  def WriteCLTriageLink(self, link):
    # pdfium recipe will read from this file and display the link in the step
    # presentation
    assert isinstance(link, str)
    output_file_name = os.path.join(self._output_dir, 'cl_triage_link.txt')
    if os.path.exists(output_file_name):
      os.remove(output_file_name)
    with open(output_file_name, 'wb') as outfile:
      outfile.write(link.encode('utf8'))

  def GetSkiaGoldProperties(self):
    if not self._skia_gold_properties:
      if self._old_gold_props:
        self._skia_gold_args.git_revision = self._old_gold_props['gitHash']
        self._skia_gold_args.gerrit_issue = self._old_gold_props['issue']
        self._skia_gold_args.gerrit_patchset = self._old_gold_props['patchset']
        self._skia_gold_args.buildbucket_id = \
            self._old_gold_props['buildbucket_build_id']

      if self._skia_gold_args.local_pixel_tests is None:
        self._skia_gold_args.local_pixel_tests = 'SWARMING_SERVER' \
            not in os.environ

      self._skia_gold_properties = pdfium_skia_gold_properties\
          .PDFiumSkiaGoldProperties(self._skia_gold_args)
    return self._skia_gold_properties

  def GetSkiaGoldSessionManager(self):
    if not self._skia_gold_session_manager:
      self._skia_gold_session_manager = pdfium_skia_gold_session_manager\
          .PDFiumSkiaGoldSessionManager(self._output_dir,
                                        self.GetSkiaGoldProperties())
    return self._skia_gold_session_manager

  def IsTryjobRun(self):
    return self.GetSkiaGoldProperties().IsTryjobRun()

  def GetCLTriageLink(self):
    return 'https://pdfium-gold.skia.org/search?issue={issue}&crs=gerrit&'\
    'corpus={source_type}'.format(
        issue=self.GetSkiaGoldProperties().issue, source_type=self._source_type)

  def UploadTestResultToSkiaGold(self, image_name, image_path):
    gold_properties = self.GetSkiaGoldProperties()
    use_luci = not (gold_properties.local_pixel_tests or
                    gold_properties.no_luci_auth)
    gold_session = self.GetSkiaGoldSessionManager()\
        .GetSkiaGoldSession(self._keys, corpus=self._source_type,
                            bucket=GS_BUCKET)

    status, error = gold_session.RunComparison(
        name=image_name, png_file=image_path, use_luci=use_luci)

    status_codes =\
        self.GetSkiaGoldSessionManager().GetSessionClass().StatusCodes
    if status == status_codes.SUCCESS:
      return True
    if status == status_codes.AUTH_FAILURE:
      logging.error('Gold authentication failed with output %s', error)
    elif status == status_codes.INIT_FAILURE:
      logging.error('Gold initialization failed with output %s', error)
    elif status == status_codes.COMPARISON_FAILURE_REMOTE:
      logging.error('Remote comparison failed. See outputted triage links.')
    elif status == status_codes.COMPARISON_FAILURE_LOCAL:
      logging.error('Local comparison failed. Local diff files:')
      _OutputLocalDiffFiles(gold_session, image_name)
      print()
    elif status == status_codes.LOCAL_DIFF_FAILURE:
      logging.error(
          'Local comparison failed and an error occurred during diff '
          'generation: %s', error)
      # There might be some files, so try outputting them.
      logging.error('Local diff files:')
      _OutputLocalDiffFiles(gold_session, image_name)
      print()
    else:
      logging.error(
          'Given unhandled SkiaGoldSession StatusCode %s with error %s', status,
          error)

    return False


def _OutputLocalDiffFiles(gold_session, image_name):
  """Logs the local diff image files from the given SkiaGoldSession.

  Args:
    gold_session: A skia_gold_session.SkiaGoldSession instance to pull files
        from.
    image_name: A string containing the name of the image/test that was
        compared.
  """
  given_file = gold_session.GetGivenImageLink(image_name)
  closest_file = gold_session.GetClosestImageLink(image_name)
  diff_file = gold_session.GetDiffImageLink(image_name)
  failure_message = 'Unable to retrieve link'
  logging.error('Generated image for %s: %s', image_name, given_file or
                failure_message)
  logging.error('Closest image for %s: %s', image_name, closest_file or
                failure_message)
  logging.error('Diff image for %s: %s', image_name, diff_file or
                failure_message)
