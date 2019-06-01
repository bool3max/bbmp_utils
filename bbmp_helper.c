#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "bbmp_parser.h"
#include "bbmp_helper.h"

typedef uint8_t *bbmp_PixelArray_Raw;

inline static bbmp_PixelArray_Raw bbmp_get_pixelarray_raw(uint8_t *raw_bmp_data, const struct Bmp_Info *metadata, void *dest);
static bbmp_PixelArray_Raw bbmp_get_pixelarray_raw_file(FILE *bmp_stream, const struct Bmp_Info *metadata, void *dest); 
static void bbmp_debug_pixelarray_raw(FILE *stream, bbmp_PixelArray_Raw pixarray_raw, const struct Bmp_Info *metadata);

static void bbmp_debug_pixelarray_raw(FILE *stream, bbmp_PixelArray_Raw pixarray_raw, const struct Bmp_Info *metadata) {
    /* 
     * Print the entire raw pixelarray, byte by byte, to stdout.
    */

    for (uint8_t *bp = pixarray_raw; bp < pixarray_raw + metadata->pixelarray_size; bp++) {
        fprintf(stream, "%.2hhX - ", *bp);
    }

    fputc('\n', stream);
}

inline static bbmp_PixelArray_Raw bbmp_get_pixelarray_raw(uint8_t *raw_bmp_data, const struct Bmp_Info *metadata, void *dest) {
    /*
     * Copy the raw pixel array from "raw_bmp_data" to "dest". 
     * Its size is equal to metadata->pixelarray_size.
     * Returns a NULL pointer on failure.
    */

    if(!raw_bmp_data || !metadata || !dest) return NULL;

    return memcpy(dest, raw_bmp_data + metadata->pixelarray_off, metadata->pixelarray_size);
}

static bbmp_PixelArray_Raw bbmp_get_pixelarray_raw_file(FILE *bmp_stream, const struct Bmp_Info *metadata, void *dest) {
    /*
     * A stream-based interface for bbmp_get_pixelarray_raw.
     * The function does not modify the offset of "bmp_stream".
     * Returns a NULL pointer on failure.
    */

    if(!bmp_stream || !metadata || !dest) return NULL;

    //save the current offset of the stream
    long off_before = ftell(bmp_stream);
    if(off_before == -1) {
        perror("bmp_helper: Error retrieving stream offset: ");
        return NULL;
    }

    //seek stream to start of pixelarray data 
    if(fseek(bmp_stream, metadata->pixelarray_off, SEEK_SET) == -1) {
        perror("bmp_helper: Failed setting stream offset: ");
        return NULL;
    }

    //read raw pixelarray data into dest
    if(fread(dest, metadata->pixelarray_size, 1, bmp_stream) < 1) {
        perror("bmp_helper: Failed reading pixelarray data from stream: ");
        fseek(bmp_stream, off_before, SEEK_SET);
        return NULL;
    }
    
    //rewind stream
    fseek(bmp_stream, off_before, SEEK_SET);
    return dest;
}

bbmp_Pixel *bbmp_get_pixelarray(uint8_t *raw_bmp_data, const struct Bmp_Info *metadata) {
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

bool bbmp_debug_pixelarray(FILE *stream, bbmp_Pixel *pixarray, const struct Bmp_Info *metadata, bool baseten) {
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
