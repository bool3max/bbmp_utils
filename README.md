# bbmp_utils

`bbmp_utils` is a native library for working with BMP files.

It is written in POSIX C11 and has only been tested on Linux.

---

Right now it includes:

    * functionality for parsing metadata out of BMP files
    * a helper API for working with raw BMP image data

**TODO**: 

    * extender the parser with funtionality for easily modifying metadata of existing BMP files
    * optimize the helper (internal representations of image data are horrendeously inefficient and memory hungry -- fix that)

---

## Development

The build system is `meson` with the `ninja` backend. I'm tired of using Make and writing Makefiles.

---

The BMP file format is thoroughly documented [**here**](https://en.wikipedia.org/wiki/BMP_file_format).
