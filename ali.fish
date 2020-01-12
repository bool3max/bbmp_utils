# source this file from an interactive fish session
# to expose the following aliases for easier testing
# b: build the debug build
# rt: run the default test from the debug build
# prt: run the python3 test executable with PYTHONPATH set accordingly
# vrt: run valgrind with full leack checking on the test from the debug build

abbr -ag b 'ninja -C build_dbg'
abbr -ag rt './build_dbg/tests/bbmp_utils_test'
abbr -ag prt 'env PYTHONPATH=./build_dbg python3 ./tests/mod.py'
abbr -ag vrt 'valgrind --leak-check=full ./build_dbg/tests/bbmp_utils_test'
abbr -ag g 'gdb ./build_dbg/tests/bbmp_utils_test'
