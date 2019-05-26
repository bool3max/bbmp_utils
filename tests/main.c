#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "bbmp_helper.h"
#include "bbmp_parser.h"

signed int main(int argc, char **argv) {
    struct Bmp_Info main_info;
    FILE *f = fopen("/home/bogdan/Downloads/img.bmp", "r+");
    if (f == NULL) {
        return EXIT_FAILURE;
    }

    if (!bbmp_parse_bmp_metadata_file(f, &main_info)) {
        fclose(f);
        return EXIT_FAILURE;
    }

    // read metadata from file and save it to the struct
    bbmp_debug_bmp_metadata(&main_info); 
    //modify the struct
    main_info.pixelarray_width = 30; main_info.pixelarray_height = 31; 
    // write modified data back to file
    bbmp_write_bmp_metadata_file(f, &main_info);

    // cleanup but no error check, unsafe
    fclose(f);

    return EXIT_SUCCESS;

    return 0;
}
