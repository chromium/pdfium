# Code Coverage Support for PDFium

[TOC]

This guide explains how to generate code coverage information for the PDFium
library on a local computer.

## Prerequisites

You will need the PDFium source code on your computer. You can see
the [README](/README.md) for instructions on checking out PDFium's source.

The tools used for code coverage are known to work on Ubuntu and Debian. They
should work correctly on newer versions of Ubuntu, Debian and related Linux
distros. They have not been tested on Windows and Mac.

Previously, the code coverage scripts required specific versions of `lcov` and
`llvm-cov` to be present. This is no longer true, so if you have no other need
for them they can be removed. All of the required tools will be pulled into the
Clang build tools directory by the script.

## Generating Code Coverage

### Setup

This step assumes that you have already checked out the PDFium source code. If
you have not, please consult the Prerequisites section above.

Before generating code coverage information, you will need to have a build
directory with coverage enabled. This can be done by running the `gn args`
command and adding `use_clang_coverage = true` in the editor that is opened.

If not using the default directory, `out/Coverage`, then replace it with the
correct location in the following command.

```shell
gn args out/Coverage
```

If you already have a build directory, you can append the coverage flag to the
existing `args.gn` as follows. If not using the default directory,
`out/Coverage`, then replace it with the correct location in the following
command.

```shell
echo "use_clang_coverage = true" >> out/Coverage/args.gn
```

Previous versions of code coverage used **use_coverage = true** in args.gn; this
needs to be changed to **use_clang_coverage = true**

### Usage

Generating code coverage information is done via the
`testing/tools/coverage/coverage_report.py` script. This script will download
the Clang coverage tools if needed, build any binaries that it needs, perform
test runs, collect coverage data, and generate a HTML coverage report. It is
based on the Chromium coverage scripts, so will generate the same style of
report.

Running the script with no arguments, as below, it will assume that you are
currently at the root of your PDFium checkout, the build directory to use is
`./out/Coverage/` and that HTML should be outputted to `./coverage_report/`.

```shell
testing/tools/coverage/coverage_report.py
```

If the current working directory is not the root of your PDFium checkout, then
you will need to pass in `--source-directory` with the appropriate directory. If
you are using a different build directory, then `--build-directory` will need to
be passed in. Finally, if you want the HTML report in a different location then
you will need to pass in `--output-directory`.

An example of all these flags being used:

```shell
testing/tools/coverage/coverage_report.py \
    --source-directory ~/pdfium/pdfium \
    --build-directory ~/pdfium/pdfium/out/Debug_with_Coverage \
    --output-directory ~/Documents/PDFium_coverage
```

To run different tests than the default set, including running just a single
test, you can specify the test names on the command line. The list of supported
tests can be found by running the script with `--help`.

For example, running all of the tests that don't use pdfium_test:

```shell
testing/tools/coverage/coverage_report.py pdfium_unittests pdfium_embeddertests
```

NOTE: At the present time, there is no mechanism for combining data from
different invocations of `coverage_report.py`. Instead, you must specify all of
the tests to be included in the report in a single invocation. Alternatively,
you can collect the profiling data that is generated from each run and manually
invoke `tools/code_coverage/coverage.py` to generate a combined report.

There are additional developer debugging flags available, `--dry-run` and
`--verbose`. `--dry-run` will output a trace of commands that would have been
run, but doesn't actually execute them. `--verbose` turns on outputting
additional logging information.

### Viewing

Once the script has run, the output directory should contain a set of HTML files
containing the coverage report.

These files are static HTML, so you can point your browser at them directly on
your local file system and they should render fine. You can also serve them via a
web server if you want, but how to achieve that is beyond the scope of this
documentation.

## Issues

For help with using the code coverage tools please contact the PDFium
maintainers via the PDFium
mailing [list](https://groups.google.com/forum/#!forum/pdfium).

Please file bugs against the code coverage
support [here](https://bugs.chromium.org/p/pdfium/issues/list).
