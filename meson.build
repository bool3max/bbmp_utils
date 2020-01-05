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

  py = import('python') # meson python module
  py_ins = py.find_installation('python3') # python3 installation object

  extension = py_ins.extension_module(meson.project_name(), 
                     'py3_bindings/module.c',
                     include_directories: incdir,
                     dependencies: py_ins.dependency(),
                     link_with: [mainlib],
                     install: true)
endif

if get_option('gen_test')
  subdir('tests')
endif
