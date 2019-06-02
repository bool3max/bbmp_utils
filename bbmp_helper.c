#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "bbmp_parser.h"
#include "bbmp_helper.h"

typedef uint8_t *bbmp_PixelArray_Raw;

inline static bbmp_PixelArray_Raw bbmp_get_pixelarray_raw(uint8_t *raw_bmp_data, const struct bbmp_Metadata *metadata, void *dest);
static void bbmp_debug_pixelarray_raw(FILE *stream, bbmp_PixelArray_Raw pixarray_raw, const struct bbmp_Metadata *metadata);

static void bbmp_debug_pixelarray_raw(FILE *stream, bbmp_PixelArray_Raw pixarray_raw, const struct bbmp_Metadata *metadata) {
    /* 
     * Print the entire raw pixelarray, byte by byte, to stdout.
    */

    for (uint8_t *bp = pixarray_raw; bp < pixarray_raw + metadata->pixelarray_size; bp++) {
        fprintf(stream, "%.2hhX - ", *bp);
    }

    fputc('\n', stream);
}

inline static bbmp_PixelArray_Raw bbmp_get_pixelarray_raw(uint8_t *raw_bmp_data, const struct bbmp_Metadata *metadata, void *dest) {
    /*
     * Copy the raw pixel array from "raw_bmp_data" to "dest". 
     * Its size is equal to metadata->pixelarray_size.
     * Returns a NULL pointer on failure.
    */

    if(!raw_bmp_data || !metadata || !dest) return NULL;

    return memcpy(dest, raw_bmp_data + metadata->pixelarray_off, metadata->pixelarray_size);
}

bool bbmp_get_image(uint8_t *raw_bmp_data, struct bbmp_Image *location) {
    /* 
     * Assuming that "raw_bmp_data" is a pointer to a memory location containing the entire BMP file data, 
     * parse its metadata and save it to location->metadata and parse its pixelarray and save it to location->pixelarray.
     * After the data is no longer needed, the API consumer must call bbmp_destroy_image() on the bbmp_Image structure to free all the
     * required resources.
    */

    // parse the metadata and save it to the struct
    if(!bbmp_parse_bmp_metadata(raw_bmp_data, &(location->metadata) )) return false;

    if((location->pixelarray = bbmp_get_pixelarray(raw_bmp_data, &(location->metadata))) == NULL) return false;

    return true;
}

bool bbmp_destroy_image(struct bbmp_Image *location) {
    /*
     * Free all resources allocated by the internal bbmp_Image representation
    */

    if(!location) return false;

    free(location->pixelarray);

    return true;
}

bbmp_Pixel *bbmp_get_pixelarray(uint8_t *raw_bmp_data, const struct bbmp_Metadata *metadata) {
    /* 
     * Returns a pointer to an array of "struct bbmp_Pixel" objects associated with a certain BMP image. 
     * Pixels are stored from the bottom left -> top right
     * The length of the array is equal to metadata->resolution.
     * The caller is responsible for freeing the memory allocated and returned by this function.
     * Returns NULL on failure.
    */

    if(!raw_bmp_data || !metadata) return NULL;

    //allocate space for the raw pixelarray
    bbmp_PixelArray_Raw pixelarray_raw = malloc(metadata->pixelarray_size);
    if(!pixelarray_raw) {
        perror("bbmp_helper: Failed allocating memory: ");
        return NULL;
    }

    //store the raw pixelarray in the temporary buffer
    bbmp_get_pixelarray_raw(raw_bmp_data, metadata, pixelarray_raw);
    
    bbmp_Pixel *pixelarray_parsed = malloc(metadata->resolution * sizeof(struct bbmp_Pixel));
    if(!pixelarray_parsed) {
        perror("bbmp_helper: Failed allocating memory: ");
        free(pixelarray_raw);
        return NULL;
    }

    // parse raw pixelarray data into a parsed, user-consumable format
    bbmp_Pixel *pixelarray_parsed_bp = pixelarray_parsed;
    for(bbmp_PixelArray_Raw bp = pixelarray_raw; bp < pixelarray_raw + metadata->pixelarray_size; bp += metadata->Bpr) {
        // per row
        for(bbmp_PixelArray_Raw bp_nest = bp; bp_nest < bp + metadata->Bpr_np; bp_nest += metadata->Bpp) {
            // per pixel -- could use memcpy here instead of copying each byte, but it probably gets optimized anyways
            pixelarray_parsed_bp->b = bp_nest[0];
            pixelarray_parsed_bp->g = bp_nest[1];
            pixelarray_parsed_bp->r = bp_nest[2];

            pixelarray_parsed_bp++;
        }
    }

    free(pixelarray_raw); //no longer needed
    return pixelarray_parsed;
}


bool bbmp_debug_pixelarray(FILE *stream, bbmp_Pixel *pixarray, const struct bbmp_Metadata *metadata, bool baseten) {
    /* 
     * Print RBG values to "stream" for each pixel in the parsed pixel array pointed to by "pixarray". 
     * If baseten is "true" (0...), print all RBG values in base 10 (decimal) instead of base 16 (hex)
     * Useful only when debugging relatively low-res images.
    */
    if(!stream || !pixarray || !metadata) return false;

    const char *format;

    if (baseten) {
        format = "\e[1m[[  \e[0m\e[31mR\e[0m:%.2hhX - \e[32mG\e[0m:%.2hhX - \e[34mB\e[0m:%.2hhX\e[1m  ]]\e[0m ";
    } else {
        format = "\e[1m[[  \e[0m\e[31mR\e[0m:%3hhu - \e[32mG\e[0m:%3hhu - \e[34mB\e[0m:%3hhu\e[1m  ]]\e[0m ";
    }

    size_t count = 0;
    for(bbmp_Pixel *bp = pixarray; bp < pixarray + metadata->resolution; bp++) {
        if(count % metadata->pixelarray_width == 0) fputc('\n', stream);        
        fprintf(stream, format, bp->r, bp->g, bp->b);
        count++;
    }

    fputc('\n', stream);

    return true;
}
