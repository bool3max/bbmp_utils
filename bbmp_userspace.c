#include "bbmp_helper.h"
#include "bbmp_parser.h"
#include <string.h>
#include <stdlib.h>

bbmp_Image *bbmp_rot90(bbmp_Image *image, const enum clock_dir direction) {
    /* 
     * Rotate the pixelarray of the bbmp_Image pointed to by image by 90 degrees clockwise or counter-clockwise.
     * Returns NULL on failure or the pointer to the bbmp_Image on success.
    */

    if (image->metadata.pixelarray_width != image->metadata.pixelarray_height || (direction != CW && direction != CCW)) return NULL;

    const size_t r = image->metadata.pixelarray_width;

    // in-place transposition
    for(size_t n = 0; n <= r - 2; n++) {
        for(size_t m = n + 1; m <= r - 1; m++) {
            bbmp_Pixel temp = image->pixelarray[n][m];
            
            image->pixelarray[n][m] = image->pixelarray[m][n];
            image->pixelarray[m][n] = temp;
        }
    }

    if (direction == CW) {
            // reverse the columns to achieve clockwise rotation
            for(size_t i_col = 0; i_col < r; i_col++) {
                size_t r_s = 0,
                       r_e = r - 1;

                do {
                    bbmp_Pixel temp = image->pixelarray[r_s][i_col];

                    image->pixelarray[r_s][i_col] = image->pixelarray[r_e][i_col];
                    image->pixelarray[r_e][i_col] = temp;

                    r_s++; 
                    r_e--;
                } while (r_s != (r / 2));
            }
    } else if (direction == CCW) {
        // reverse the rows to achieve counter-clockwise rotation
        for(size_t i_row = 0; i_row < r; i_row++) {
            size_t c_s = 0,
                   c_e = r - 1;

            do {
                bbmp_Pixel temp = image->pixelarray[i_row][c_s];

                image->pixelarray[i_row][c_s] = image->pixelarray[i_row][c_e];
                image->pixelarray[i_row][c_e] = temp;

                c_s++;
                c_e--;

            } while (c_s != (r / 2));

        }
    }

    return image;
}
