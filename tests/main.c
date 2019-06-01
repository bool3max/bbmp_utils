#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "bbmp_parser.h"
#include "bbmp_helper.h"

#define IMG "/home/bogdan/Downloads/img.bmp"

signed int main(int argc, char **argv) {
    struct stat tempstat;
    if(stat(IMG, &tempstat) == -1 ) { 
        perror("stating failed: ");
        return EXIT_FAILURE;
    }

    void *bmp_raw_data = malloc(tempstat.st_size);
    FILE *f = fopen(IMG, "r");
    if(fread(bmp_raw_data, tempstat.st_size, 1, f) != 1) {
        perror("reading file into buffer failed: ");
        return EXIT_FAILURE;
    }

    if(fclose(f) != 0) {
        perror("closing fd failed: ");
        return EXIT_FAILURE;
    }

    struct Bmp_Info maininfo;
    if(!bbmp_parse_bmp_metadata(bmp_raw_data, &maininfo)) {
        free(bmp_raw_data); fclose(f);
        return EXIT_FAILURE;
    }

    bbmp_Pixel *pixarray = bbmp_get_pixelarray(bmp_raw_data, &maininfo); //now accessible in an array-like manner
    free(bmp_raw_data); //not needed anymore, all the info we need is in memory

    //do actual work with the parsed pixel array
    bbmp_debug_pixelarray(stdout, pixarray, &maininfo, false);

    free(pixarray);
    return EXIT_SUCCESS;
}
