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
    if(stat(IMG, &tempstat) == -1) { 
        perror("stating failed: ");
        return EXIT_FAILURE;
    }

    void *bmp_raw_data = malloc(tempstat.st_size);
    FILE *f = fopen(IMG, "r");
    if(fread(bmp_raw_data, tempstat.st_size, 1, f) != 1) {
        perror("reading file into buffer failed: ");
        free(bmp_raw_data); fclose(f);
        return EXIT_FAILURE;
    }

    if(fclose(f) != 0) {
        perror("closing fd failed: ");
        free(bmp_raw_data);
        return EXIT_FAILURE;
    }

    bbmp_Image represent;
    bool res = bbmp_get_image(bmp_raw_data, &represent); 
    if(!res) { perror("error occured: "); free(bmp_raw_data); }

    bbmp_debug_bmp_metadata(&represent.metadata);

    bbmp_debug_pixelarray(stdout, &represent, true);

    fputs("modified [0][1] to all-green\n", stdout);
    (represent.pixelarray)[0][1] = (bbmp_Pixel) {0, 255, 0}; //rgb
    (represent.pixelarray)[1][1] = (bbmp_Pixel) {255, 255, 1}; 

    bbmp_debug_pixelarray(stdout, &represent, true);

    bbmp_metacustomupdate(&represent); //not *necessary* here, but JIC

    bbmp_write_image(&represent, bmp_raw_data);

    FILE *fp = fopen(IMG, "r+");
    rewind(fp);
    fwrite(bmp_raw_data, tempstat.st_size, 1, fp);
    fclose(fp);

    free(bmp_raw_data);

    bbmp_destroy_image(&represent);

    return EXIT_SUCCESS;
}
