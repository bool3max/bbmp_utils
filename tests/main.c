#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "bbmp_helper.h"
#include "bbmp_parser.h"

signed int main(int argc, char **argv) {
    struct Bmp_Info maininfo = { .filesize = 33};
    bbmp_debug_bmp_metadata(&maininfo);
    return EXIT_SUCCESS;
}
