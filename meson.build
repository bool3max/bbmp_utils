project('bbmp_utils', 'c', license: 'MIT', version: '0.1.1', default_options : ['c_std=c11']) 
add_global_arguments('-Wno-unused', '-pedantic', language: 'c')

ccompiler = meson.get_compiler('c')
math = ccompiler.find_library('m', required: true)

lib_sources = ['bbmp_parser.c', 'bbmp_helper.c', 'bbmp_userspace.c']
incdir = include_directories('include')

mainlib = library('bbmputil', lib_sources, include_directories : incdir, dependencies: [math], install: true)
install_headers(['include/bbmp_parser.h', 'include/bbmp_helper.h'], subdir: 'bbmp_utils') # only called on the "install" operation

if get_option('gen_py_bindings')
  # build the provided python extension module

  run_command('touch', [meson.build_root() + '/_fake'])

  # let python's build system handle it (install: false because the setup script handles installation if told to do so)
  custom_target('py_bindings',
                command: ['env', '_MESON_MODULE_NAME=' + meson.project_name(), '_MESON_MODULE_VERSION=' + meson.project_version(), '_MESON_SOURCE_ROOT=' + meson.source_root(), '_MESON_BUILD_ROOT=' + meson.build_root(), files('py3_bindings/setup.py'), 'build'],
                depends: [mainlib], 
                depend_files: ['py3_bindings/module.c'],
                install: false,
                output: '_fake',
                build_by_default: true)
endif

if get_option('gen_test')
  subdir('tests')
endif
