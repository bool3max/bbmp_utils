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
        return EXIT_FAILURE;
    }

    if(fclose(f) != 0) {
        perror("closing fd failed: ");
        return EXIT_FAILURE;
    }

    bbmp_Image represent;
    bool res = bbmp_get_image(bmp_raw_data, &represent); 
    if(!res) { perror("error occured: "); free(bmp_raw_data); }

    bbmp_debug_pixelarray(stdout, &represent, true);
    (represent.pixelarray)[0][1].r = 255;

    fputs("Changed [0][1]'s RED value to 255 baseten\n", stdout);
    /* (represent.pixelarray)[0][1].g = 255; */   
    /* (represent.pixelarray)[0][1].b = 255; */   
    bbmp_debug_pixelarray(stdout, &represent, true);

    printf("[0][0] red pixel: %hhu\n", represent.pixelarray[0][0].r);
    printf("[0][1] red pixel: %hhu\n", represent.pixelarray[0][1].r);
    printf("[1][0] red pixel: %hhu\n", represent.pixelarray[1][0].r);
    printf("[1][1] red pixel: %hhu\n", represent.pixelarray[1][1].r);

    bbmp_write_image(&represent, bmp_raw_data);

    FILE *fp = fopen(IMG, "rw");
    fwrite(bmp_raw_data, tempstat.st_size, 1, fp);
    fclose(fp);

    free(bmp_raw_data);

    bbmp_destroy_image(&represent);

    return EXIT_SUCCESS;
}
