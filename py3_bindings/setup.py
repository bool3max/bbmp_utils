#!/usr/bin/python3

# use distutils to portably build the extension module using the host's C compiler suite 
# is only executed by meson if meson option "gen_py_bindings" is set to true (which is the default behavior)

import distutils.core as dc

from os import environ as env
from os.path import join as path_join
from sys import exit, stderr

try: 
    # these are the same as meson's project values and are passed from it as env. vars
    module_name = env["_MESON_MODULE_NAME"]
    module_version = env["_MESON_MODULE_VERSION"]
    meson_src_root = env["_MESON_SOURCE_ROOT"]
    meson_build_root = env["_MESON_BUILD_ROOT"]
except KeyError:
        # quit if the env. vars were not passed in
        print("Do not call setup.py manually! If you wish to build the python extension, set the \033[3mgen_py_bindings\033[0m meson option to \033[3mtrue\033[0m, and re-run the build system. Quitting...", file=stderr)
        exit(1)
    
module = dc.Extension(module_name, 
                      sources = [path_join(meson_src_root, "py3_bindings", "module.c")], 
                      include_dirs = [path_join(meson_src_root, "include")],
                      library_dirs = [meson_build_root],
                      runtime_library_dirs = [meson_build_root],
                      libraries = ["bbmputil"],
                      extra_link_args = ["-Wl,--no-as-needed"]) # seems to be faulty

dc.setup(name = module_name,
         version = module_version,
         description = "Python3 bindings for the bbmp_utils library",
         ext_modules = [module])
