# PDFium

## Prerequisites

PDFium uses the same build tooling as Chromium. See the platform-specific
Chromium build instructions to get started, but replace Chromium's
"Get the code" instructions with [PDFium's](#get-the-code).

*   [Chromium Linux build instructions](https://chromium.googlesource.com/chromium/src/+/main/docs/linux/build_instructions.md)
*   [Chromium Mac build instructions](https://chromium.googlesource.com/chromium/src/+/main/docs/mac_build_instructions.md)
*   [Chromium Windows build instructions](https://chromium.googlesource.com/chromium/src/+/main/docs/windows_build_instructions.md)

### CPU Architectures supported

The default architecture for Windows, Linux, and Mac is "`x64`". On Windows,
"`x86`" is also supported. GN parameter "`target_cpu = "x86"`" can be used to
override the default value. If you specify Android build, the default CPU
architecture will be "`arm`".

It is expected that there are still some places lurking in the code which will
not function properly on big-endian architectures. Bugs and/or patches are
welcome, however providing this support is **not** a priority at this time.

#### Google employees

Run: `download_from_google_storage --config` and follow the
authentication instructions. **Note that you must authenticate with your
@google.com credentials**. Enter "0" if asked for a project-id.

Once you've done this, the toolchain will be installed automatically for
you in the [Generate the build files](#generate-the-build-files) step below.

The toolchain will be in `depot_tools\win_toolchain\vs_files\<hash>`, and
windbg can be found in
`depot_tools\win_toolchain\vs_files\<hash>\win_sdk\Debuggers`.

If you want the IDE for debugging and editing, you will need to install
it separately, but this is optional and not needed for building PDFium.

## Get the code

The name of the top-level directory does not matter. In the following example,
the directory name is "repo". This directory must not have been used before by
`gclient config` as each directory can only house a single gclient
configuration.

```
mkdir repo
cd repo
gclient config --unmanaged https://pdfium.googlesource.com/pdfium.git
gclient sync
cd pdfium
```

On Linux, additional build dependencies need to be installed by running the
following from the `pdfium` directory.

```
./build/install-build-deps.sh
```

## Generate the build files

PDFium uses GN to generate the build files and [Ninja](https://ninja-build.org/)
to execute the build files.  Both of these are included with the
depot\_tools checkout.

### Selecting build configuration

PDFium may be built either with or without JavaScript support, and with
or without XFA forms support.  Both of these features are enabled by
default. Also note that the XFA feature requires JavaScript.

Configuration is done by executing `gn args <directory>` to configure the build.
This will launch an editor in which you can set the following arguments.
By convention, `<directory>` should be named `out/foo`, and some tools / test
support code only works if one follows this convention.
A typical `<directory>` name is `out/Debug`.

```
use_goma = false  # Googlers only. Ensure goma is installed and running first.
is_debug = true  # Enable debugging features.

# Set true to enable experimental Skia backend.
pdf_use_skia = false

pdf_enable_xfa = true  # Set false to remove XFA support (implies JS support).
pdf_enable_v8 = true  # Set false to remove Javascript support.
pdf_is_standalone = true  # Set for a non-embedded build.
is_component_build = false # Disable component build (Though it should work)
```

For sample applications like `pdfium_test` to build, one must set
`pdf_is_standalone = true`.

By default, the entire project builds with C++20.

By default, PDFium expects to build with a clang compiler that provides
additional chrome plugins. To build against a vanilla one lacking these,
one must set
`clang_use_chrome_plugins = false`.

When complete the arguments will be stored in `<directory>/args.gn`, and
GN will automatically use the new arguments to generate build files.
Should your files fail to generate, please double-check that you have set
use\_sysroot as indicated above.

## Building the code

You can build the sample program by running: `ninja -C <directory> pdfium_test`
You can build the entire product (which includes a few unit tests) by running:
`ninja -C <directory> pdfium_all`.

## Running the sample program

The pdfium\_test program supports reading, parsing, and rasterizing the pages of
a .pdf file to .ppm or .png output image files (Windows supports two other
formats). For example: `<directory>/pdfium_test --ppm path/to/myfile.pdf`. Note
that this will write output images to `path/to/myfile.pdf.<n>.ppm`.
Run `pdfium_test --help` to see all the options.

## Testing

There are currently several test suites that can be run:

 * pdfium\_unittests
 * pdfium\_embeddertests
 * testing/tools/run\_corpus\_tests.py
 * testing/tools/run\_javascript\_tests.py
 * testing/tools/run\_pixel\_tests.py

It is possible the tests in the `testing` directory can fail due to font
differences on the various platforms. These tests are reliable on the bots. If
you see failures, it can be a good idea to run the tests on the tip-of-tree
checkout to see if the same failures appear.

### Pixel Tests

If your change affects rendering, a pixel test should be added. Simply add a
`.in` or `.pdf` file in `testing/resources/pixel` and the pixel runner will
pick it up at the next run.

Make sure that your test case doesn't have any copyright issues. It should also
be a minimal test case focusing on the bug that renders the same way in many
PDF viewers. Try to avoid binary data in streams by using the `ASCIIHexDecode`
simply because it makes the PDF more readable in a text editor.

To try out your new test, you can call the `run_pixel_tests.py` script:

```bash
$ ./testing/tools/run_pixel_tests.py your_new_file.in
```

To generate the expected image, you can use the `make_expected.sh` script:

```bash
$ ./testing/tools/make_expected.sh your_new_file.pdf
```

Please make sure to have `optipng` installed which optimized the file size of
the resulting png.

### `.in` files

`.in` files are PDF template files. PDF files contain many byte offsets that
have to be kept correct or the file won't be valid. The template makes this
easier by replacing the byte offsets with certain keywords.

This saves space and also allows an easy way to reduce the test case to the
essentials as you can simply remove everything that is not necessary.

A simple example can be found [here](https://pdfium.googlesource.com/pdfium/+/refs/heads/main/testing/resources/rectangles.in).

To transform this into a PDF, you can use the `fixup_pdf_template.py` tool:

```bash
$ ./testing/tools/fixup_pdf_template.py your_file.in
```

This will create a `your_file.pdf` in the same directory as `your_file.in`.

There is no official style guide for the .in file, but a consistent style is
preferred simply to help with readability. If possible, object numbers should
be consecutive and `/Type` and `/SubType` should be on top of a dictionary to
make object identification easier.

## Embedding PDFium in your own projects

The public/ directory contains header files for the APIs available for use by
embedders of PDFium. The PDFium project endeavors to keep these as stable as
possible.

Outside of the public/ directory, code may change at any time, and embedders
should not directly call these routines.

## Code Coverage

Code coverage reports for PDFium can be generated in Linux development
environments. Details can be found [here](/docs/code-coverage.md).

Chromium provides code coverage reports for PDFium
[here](https://chromium-coverage.appspot.com/). PDFium is located in
`third_party/pdfium` in Chromium's source code.
This includes code coverage from PDFium's fuzzers.

## Waterfall

The current health of the source tree can be found
[here](https://ci.chromium.org/p/pdfium/g/main/console).

## Community

There are several mailing lists that are setup:

 * [PDFium](https://groups.google.com/forum/#!forum/pdfium)
 * [PDFium Reviews](https://groups.google.com/forum/#!forum/pdfium-reviews)
 * [PDFium Bugs](https://groups.google.com/forum/#!forum/pdfium-bugs)

Note, the Reviews and Bugs lists are typically read-only.

## Bugs

PDFium uses this [bug tracker](https://bugs.chromium.org/p/pdfium/issues/list),
but for security bugs, please use
[Chromium's security bug template](https://bugs.chromium.org/p/chromium/issues/entry?template=Security%20Bug)
and add the "Cr-Internals-Plugins-PDF" label.

## Contributing code

See the [CONTRIBUTING](CONTRIBUTING.md) document for more information on
contributing to the PDFium project.
