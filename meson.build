project('bbmp_utils', 'c', license: 'MIT', default_options : ['c_std=c11'])
add_global_arguments('-Wno-unused', language: 'c')

ccompiler = meson.get_compiler('c')

sources = ['bbmp_parser.c', 'bbmp_helper.c']
incdir = include_directories('include')

mainlib = static_library('bbmputil', sources, include_directories : incdir, install: true)
install_headers(['include/bbmp_parser.h', 'include/bbmp_helper.h'], subdir: 'bbmp_utils')

if get_option('build_test')
  subdir('tests') 
endif
