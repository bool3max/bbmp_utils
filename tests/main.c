#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "bbmp_parser.h"
#include "bbmp_helper.h"

#define IMG_PATH "/home/bogdan/Downloads/img.bmp"

signed int main(int argc, char **argv) {
    bbmp_Image img; // image structure that also holds the actual image pixelarray data 
    img.metadata = (bbmp_Metadata) {0}; //stop valgrind from complaining about uninitialized values

    bbmp_create_image(2560, 1080, 24, & (const bbmp_Pixel) {.r = 182, .g =  255, .b = 22}, &img); // create a new 2x2 image instance 
    bbmp_debug_bmp_metadata(&img.metadata);

    const size_t raw_bytesize = bbmp_image_calc_bytesize(&img);
    fprintf(stdout, "raw_bytesize: %lu\n", raw_bytesize);

    void *imgdata = malloc(raw_bytesize);
    // allocate enough space for the entire image, including all the metadata
    if (!imgdata) {
        perror("error: ");
        bbmp_destroy_image(&img);
        return EXIT_FAILURE;
    }

    if(!bbmp_write_image(&img, imgdata)) {
        fprintf(stderr, "Failed writing image to raw buffer");
        bbmp_destroy_image(&img);
        free(imgdata);
        return EXIT_FAILURE;
    }

    // save the raw imaage data to a .bmp file, for viewing

    FILE *file = fopen(IMG_PATH, "w+");
    if (!file) {
        // failed opening file, do cleanup
        perror("Failed opening file");
        bbmp_destroy_image(&img);
        free(imgdata);
        return EXIT_FAILURE;
    }

    size_t it = fwrite(imgdata, raw_bytesize, 1u, file);
    if(it != 1) {
        fprintf(stderr, "Failed writing raw image to file: short item count from fwrite.\n");        
        bbmp_destroy_image(&img);
        free(imgdata);
        fclose(file);
        return EXIT_FAILURE;
    }

    bbmp_destroy_image(&img);
    fclose(file);
    free(imgdata);

    return EXIT_SUCCESS;
}
