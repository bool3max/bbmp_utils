project('bbmp_utils', 'c', license: 'MIT', version: '0.1.1', default_options : ['c_std=c11']) 
add_global_arguments('-Wno-unused', language: 'c')

ccompiler = meson.get_compiler('c')

sources = ['bbmp_parser.c', 'bbmp_helper.c', 'bbmp_userspace.c']
incdir = include_directories('include')

mainlib = static_library('bbmputil', sources, include_directories : incdir, install: true)
install_headers(['include/bbmp_parser.h', 'include/bbmp_helper.h'], subdir: 'bbmp_utils') # only called on the "install" operation

if get_option('gen_test')
  subdir('tests')
endif
