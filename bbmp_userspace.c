#include "bbmp_helper.h"
#include "bbmp_parser.h"
#include <string.h>
#include <stdlib.h>

bbmp_Image *bbmp_rot90(bbmp_Image *image, const enum clock_dir direction) {
    /* 
     * Rotate the pixelarray of the bbmp_Image pointed to by image by 90 degrees clockwise or counter-clockwise.
     * Returns NULL on failure or the pointer to the bbmp_Image on success.
    */

    if (image->metadata.pixelarray_width != image->metadata.pixelarray_height) return NULL;

    const size_t row_bytecount = image->metadata.pixelarray_width * sizeof(bbmp_Pixel); // bytes required per one pixelarray row

    if (direction == CW) {
        // NOTE: always temp backup the NEXT operating row
        // this could be further optimized by only backing up the pixels necessary for the next operation, but I'm going the
        // lazy route for now

        size_t r_col_index = 0;

        bbmp_Pixel *source_row = malloc(image->metadata.pixelarray_width * sizeof(bbmp_Pixel)); 
        if(!source_row) {
            perror("bbmp_helper: Failed allocating memory\n");
            return NULL;
        }
        
        memcpy(source_row, *(image->pixelarray), row_bytecount); // at start, point to a copy of the first row of pixels

        for(size_t row_index = 0; row_index < image->metadata.pixelarray_height; row_index++) {
            // per-row
            size_t r_row_index = image->metadata.pixelarray_height - 1;
            
            for(size_t col_index = 0; col_index < image->metadata.pixelarray_width; col_index++) {
                // per-pixel
                image->pixelarray[r_row_index][r_col_index] = source_row[col_index];

                r_row_index--;
                memcpy(source_row, *(image->pixelarray + row_index), row_bytecount);
            }

            r_col_index++;
        }

        free(source_row);
    } else if (direction == CCW) {
        // not implemented yet
        return NULL;
    } else return NULL;

    return image;        
}
