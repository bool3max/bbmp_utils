# bbmp_utils

`bbmp_utils` is a native library for working with BMP files.

It is written in POSIX C11 and its only dependency it the C stdlib. It has only been tested on `amd64`, `linux 5.1` and `gcc 9.0`, though I see no reason why it shouldn't work on other architectures and/or POSIX systems (cross-compilation isn't yet supported by the build system but host-compilation should work).

---

As of now it includes:

* functionality for parsing metadata out of and writing it to BMP files
* a helper API for working with raw BMP image data
* userspace functions for editing raw bitmaps (e.g. a `rot90` function for rotating a pixelarray by 90 degrees in either direction)
* optional python3 extension module for interacting with the library from within python

---

## Development

Development is done using `meson` with the `ninja` backend.

1. Run `$ meson setup build_dbg -Dbuildtype=debug` (replace `debug` with `release` if you wish to build a release build w/o debugging symbols and w/ optimizations)
2. Run `$ ninja -C build_dbg` to build the project
3. Run `# ninja -C build_dbg install` to install the library and the header files in the default location.

If you wish to *not* build the python extension module, pass `-Dgen_py_bindings=false` to the initial `meson setup` command.

### `ali.fish`

If you're using the `fish` shell, sourcing this file (e.g. `$ source ./ali.fish`) will expose a few aliases for more convenient developing inside the shell: 

* `b`: build the debug version of the project
* `rt`: run the default test application in the debug build
* `vrt`: run valgrind on the default test application in the debug build
* `g`: run GDB on the default test applicaiton in the debug build

The aliases are global (not universal) and live only in the current shell session.

---

Note that on linux, standard practice is to never install libraries w/ debugging symbols, so that should be avoided. (e.g. never install a build generated with `meson setup build_* -Dbuildtype=debug`)

---

**TODO**: 

* [ ] documentation
* [x] ~~optimize the helper (internal representations of image data are horrendeously inefficient and memory hungry -- fix that)~~
* [x] ~~extend the parser with funtionality for easily modifying metadata of existing BMP files~~
* [x] ~~extend the build system to support more edge/use cases and installation~~

---

The BMP file format is thoroughly documented [**here**](https://en.wikipedia.org/wiki/BMP_file_format).
