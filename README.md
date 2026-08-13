# cgit-reaper

[Original document location](https://jausoft.com/cgit/cgit-reaper.git/about/).

Cleanup [cgit](https://git.zx2c4.com/cgit/about/) cache.

## Git Repository
A canonical repository copy is hosted on [Gothel Software](https://jausoft.com/cgit/cgit-reaper.git/).

## Goals
Cleanup [cgit](https://git.zx2c4.com/cgit/about/) cache.
- Phase 1 via expiration date, either by commandline option `--ttl <minutes>` or `cgitrc` maximum `cache-*-ttl`.
- Phase 2 oldest files exceeding max-files count, either by commandline option `--files <number>` or `cgitrc` `cache-size`.

Main objective for this file reaper is to allow `cgit` to use the full range of 64-bit FNV-1a value
to reduce collisions but limiting the maximum number of cache files to a considerably lower number.
- Set `cgitrc` value `cache-size=18446744073709551615`.
- Pass `cgit-reaper` command-line argument `--files 1048575` for e.g. 1M files.

Note that certain cgit features like 64-bit FNV-1a support is currently only available
in my [cgit branch](https://jausoft.com/cgit/cgit.git/).

## Usage

### Commandline Arguments

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
`-v` or `--verbose`: Enables verbose mode.
`-n` or `--dry-run`: Perform a trial run with no changes made.
`--ttl <minutes>`  : Override ttl value for expiration.
`--files <number>` : Override maximum number of cached files.
`-h` or `--help`   : Print brief command-line arguments and exit.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

### Installation

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.sh}
touch /var/log/cgit-reaper.log
chown webrunner:webrunner /var/log/cgit-reaper.log

cp cgit-reaper /srv/www/cgit/cgit-reaper
chown webrunner:webrunner /srv/www/cgit/cgit-reaper
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

### Crontab scheduling

Adding 15 minutes crontab scheduling and limit maximum number of files to 1M,
while allowing `cgit` to use full range of 64-bit FNV-1a value to reduce collisions.
The latter can be achived by `cgitrc` config value `cache-size=18446744073709551615`.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# .---------------- minute (0 - 59)
# |  .------------- hour (0 - 23)
# |  |  .---------- day of month (1 - 31)
# |  |  |  .------- month (1 - 12) OR jan,feb,mar,apr ...
# |  |  |  |  .---- day of week (0 - 6) (Sunday=0 or 7) OR sun,mon,tue,wed,thu,fri,sat
# |  |  |  |  |
# *  *  *  *  *  user-name command to be executed
*/15 *  *  *  *  webrunner /srv/www/cgit/cgit-reaper --files 1048575 2>&1 >> /var/log/cgit-reaper.log
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

## Supported Platforms
- C++20 or better, see [jaulib C++ Minimum Requirements](https://jausoft.com/cgit/jaulib.git/about/README.md#cpp_min_req).

### Build Dependencies
- CMake >= 3.21 (2021-07-14)
- C++ compiler
  - gcc >= 11 (C++20), recommended >= 12.2.0
  - clang >= 13 (C++20), recommended >= 18.1.6
- Optional for `lint` validation
  - clang-tidy >= 18.1.6
- Optional for `eclipse` and `vscodium` integration
  - clangd >= 18.1.6
  - clang-tools >= 18.1.6
  - clang-format >= 18.1.6
- [jaulib](https://jausoft.com/cgit/jaulib.git/about/) *submodule*

#### Install on Debian or Ubuntu

Installing build dependencies for Debian >= 12 and Ubuntu >= 22:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.sh}
apt install git
apt install build-essential g++ gcc libc-dev libpthread-stubs0-dev
apt install clang-18 clang-tidy-18 clangd-18 clang-tools-18 clang-format-18
apt install libunwind8 libunwind-dev
apt install cmake cmake-extras extra-cmake-modules pkg-config
apt install doxygen graphviz
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If using optional clang toolchain,
perhaps change the clang version-suffix of above clang install line to the appropriate version.

After complete clang installation, you might want to setup the latest version as your default.
For Debian you can use this [clang alternatives setup script](https://jausoft.com/cgit/jaulib.git/tree/scripts/setup_clang_alternatives.sh).

### Build Procedure

#### Build preparations

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.sh}
git clone --recurse-submodules git://jausoft.com/srv/scm/cgit-reaper.git
cd cgit-reaper
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

<a name="cmake_presets_optional"></a>

#### CMake Build via Presets
Analog to [jaulib CMake build presets](https://jausoft.com/cgit/jaulib.git/about/README.md#cmake_presets_optional) ...

Following debug presets are defined in `CMakePresets.json`
- `debug`
  - default generator
  - default compiler
  - C++20
  - debug enabled
  - disabled `clang-tidy`
- `debug-gcc`
  - inherits from `debug`
  - compiler: `gcc`
  - disabled `clang-tidy`
- `debug-clang`
  - inherits from `debug`
  - compiler: `clang`
  - enabled `clang-tidy`
- `release`
  - inherits from `debug`
  - debug disabled
  - disabled `clang-tidy`
- `release-gcc`
  - compiler: `gcc`
  - disabled `clang-tidy`
- `release-clang`
  - compiler: `clang`
  - enabled `clang-tidy`
- `release-wasm`
  - compiler: `clang / emscripten`
  - disabled `clang-tidy`
  - needs to be run by `emcmake`

Kick-off the workflow by e.g. using preset `release-gcc` to configure, build, test, install and building documentation.
You may skip `install` and `doc` by dropping it from `--target`.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.sh}
cmake --preset release-gcc
cmake --build --preset release-gcc --parallel
cmake --build --preset release-gcc --target install
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You may utilize `scripts/build-preset.sh` for an initial build, install and test workflow.

<a name="cmake_presets_hardcoded"></a>

#### CMake Build via Hardcoded Presets
Analog to [jaulib CMake hardcoded presets](https://jausoft.com/cgit/jaulib.git/about/README.md#cmake_presets_hardcoded) ...

Besides above `CMakePresets.json` presets,
`JaulibSetup.cmake` contains hardcoded presets for *undefined variables* if
- `CMAKE_INSTALL_PREFIX` and `CMAKE_CXX_CLANG_TIDY` cmake variables are unset, or
- `JAU_CMAKE_ENFORCE_PRESETS` cmake- or environment-variable is set to `TRUE` or `ON`

The hardcoded presets resemble `debug-clang` [presets](README.md#cmake_presets_optional).

Kick-off the workflow to configure, build, test, install and building documentation.
You may skip `install` and `doc` by dropping it from `--target`.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.sh}
rm -rf build/default
cmake -B build/default
cmake --build build/default --parallel
cmake --build build/default --target install
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The install target of the last command will create the include/ and lib/ directories with a copy of
the headers and library objects respectively in your dist location.

#### CMake Variables
Our cmake configure has a number of options, *cmake-gui* or *ccmake* can show
you all the options. The interesting ones are detailed below:

See [jaulib CMake variables](https://jausoft.com/cgit/jaulib.git/about/README.md#cmake_variables) for details.

