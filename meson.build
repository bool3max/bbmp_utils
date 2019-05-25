project('bbmp_utils', 'c')

sources = ['bbmp_parser.c', 'bbmp_helper.c']

static_library('bbmputil', sources, include_directories : include_directories('include'), install: true)

install_headers('include/bbmp_parser.h', 'include/bbmp_helper.h', subdir: 'bbmp_utils')
