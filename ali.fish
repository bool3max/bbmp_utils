# source this file from an interactive fish session
# to expose the following aliases for easier testing
# b: build the debug build
# rt: run the default test from the debug build
# vrt: run valgrind with full leack checking on the test from the debug build

abbr -ag b 'ninja -C build_dbg'
abbr -ag rt './build_dbg/tests/bbmp_utils_test'
abbr -ag vrt 'valgrind --leak-check=full ./build_dbg/tests/bbmp_utils_test'
