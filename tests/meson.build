math = ccompiler.find_library('m', required: true)
sdl2 = ccompiler.find_library('SDL2')

executable('bbmp_utils_test', 'main.c', include_directories: incdir, link_with: mainlib, dependencies: [math, sdl2], install: false)
