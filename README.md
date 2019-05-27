# bbmp_utils

`bbmp_utils` is a native library for working with BMP files.

It is written in POSIX C11 and its only dependency it the C stdlib. It has only been tested on `amd64` and linux 5.1, though I see no reason why it shouldn't work on other architectures and/or POSIX systems (cross-compilation isn't yet supported by the build system but host-compilation should work).

---

Right now it includes:

* functionality for parsing metadata out of and writing it to BMP files
* a helper API for working with raw BMP image data

**TODO**: 

* [ ] optimize the helper (internal representations of image data are horrendeously inefficient and memory hungry -- fix that)
* [ ] document the library API
* [ ] support for DIB headers other than `BITMAPINFOHEADER` (`"BM"`)
* [x] ~~extend the parser with funtionality for easily modifying metadata of existing BMP files~~
* [x] ~~extend the build system to support more edge/use cases and installation~~

---

## Development

Development is done using `meson` with the `ninja` backend.

1. Run `$ meson setup build -Dbuild-type=debug` (replace `debug` with `release` if you wish to build a release build w/o debugging symbols and w/ optimizations)
2. Run `$ ninja -C build` to build the project
3. Run `# ninja -C build install` to install the static library and the header files in the default location.

If you also wish to build the sample application used for the library and the API, also pass `-Dbuild_test=true` to the initial `meson setup ...` command, or run `$ meson configure build -Dbuild_test=true && ninja -C build` if you've already run the setup.
The test application is not guaranteed to do anything meaningful and is not to be used as an actual test environment.

Note that on linux, standard practice is to never install libraries w/ debugging symbols, so that should be avoided. (e.g. never install a build generated with `meson setup build -Dbuild-type=debug`)

---

The BMP file format is thoroughly documented [**here**](https://en.wikipedia.org/wiki/BMP_file_format).
