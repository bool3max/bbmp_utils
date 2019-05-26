# bbmp_utils

`bbmp_utils` is a native library for working with BMP files.

It is written in POSIX C11 and has only been tested on `amd64` and linux 5.1, though I see no reason why it shouldn't work on other architectures and/or POSIX systems (cross-compilation isn't yet supported by the build system but host-compilation should work).

---

Right now it includes:

* functionality for parsing metadata out of BMP files
* a helper API for working with raw BMP image data

**TODO**: 

* [ ] extend the parser with funtionality for easily modifying metadata of existing BMP files
* [ ] optimize the helper (internal representations of image data are horrendeously inefficient and memory hungry -- fix that)
* [ ] extend the build system to support more edge/use cases and installation
* [ ] document the library API
* [ ] bindings for lua?

---

## Development

Development is done using `meson` with the `ninja` backend.

1. Run `$ meson setup build_dbg --build-type debug` or `meson setup build_release --build-type release` if you wish to build a release build w/o optimizations or debug symbols
2. Run `$ ninja -C build_TYPE` to produce a build of a static library in the `build_...` directory
3. Run `$ ninja -C build_TYPE install` to install the static library and the header files (default prefix is `/usr/local/`)

You could also combine steps 2. and 3. to build and install at once: `$ ninja -C build_TYPE install`.

---

The BMP file format is thoroughly documented [**here**](https://en.wikipedia.org/wiki/BMP_file_format).
