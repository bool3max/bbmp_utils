#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "bbmp_parser.h"
#include "bbmp_helper.h"

typedef uint8_t *bbmp_PixelArray_Raw;

inline static bbmp_PixelArray_Raw bbmp_get_pixelarray_raw(uint8_t *raw_bmp_data, const struct bbmp_Metadata *metadata, void *dest);
static bbmp_PixelArray bbmp_get_pixelarray(uint8_t *raw_bmp_data, const struct bbmp_Metadata *metadata); 
static bbmp_PixelArray_Raw bbmp_convert_pixelarray(const bbmp_PixelArray parsed, const bbmp_Metadata *metadata, bbmp_PixelArray_Raw buffer); 
static void bbmp_debug_pixelarray_raw(FILE *stream, bbmp_PixelArray_Raw pixarray_raw, const struct bbmp_Metadata *metadata);

static void bbmp_debug_pixelarray_raw(FILE *stream, bbmp_PixelArray_Raw pixarray_raw, const struct bbmp_Metadata *metadata) {
    /* 
     * Print the entire raw pixelarray, byte by byte, to the specified stream.
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
     * Namely the bbmp_PixelArray as it lives on the heap, as the metadata is a struct on the stack
    */

    if(!location) return false;

    for(bbmp_PixelArray bp = location->pixelarray; bp < location->pixelarray + (location->metadata).pixelarray_height; bp++) {
        free(*bp);
    }

    free(location->pixelarray);

    return true;
}

static bbmp_PixelArray bbmp_get_pixelarray(uint8_t *raw_bmp_data, const struct bbmp_Metadata *metadata) {
    /* 
     * Parse raw BMP data pointed to by "raw_bmp_data" and return a 2D array of bbmp_Pixel structs.
     * The array's 'dimensions' are equal to [metadata.pixelarray_height]*[metadata->pixelarray_width];
     * The memory allocated by this function must be freed manually, although this is usually done by the API consumer using
     * bbmp_destroy_image on a bbmp_Image struct.
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

    // allocate space for HEIGHT pointers to pixels
    bbmp_PixelArray pixelarray_parsed = malloc(metadata->pixelarray_height * sizeof(bbmp_Pixel*));
    if(!pixelarray_parsed) {
        perror("bbmp_helper: Failed allocating memory: ");
        free(pixelarray_raw);
        return NULL;
    }
     
    //raw row pointer
    bbmp_PixelArray_Raw bp_raw = pixelarray_raw;

    // initialize each HEIGHT row
    for (bbmp_PixelArray bp = pixelarray_parsed; bp < pixelarray_parsed + metadata->pixelarray_height; bp++) {
        // make each pointer point to a memory location large enough to hold WIDTH instances of bbmp_Pixel
        *bp = malloc(metadata->pixelarray_width * sizeof(bbmp_Pixel));

        bbmp_PixelArray_Raw bp_raw_nest = bp_raw;

        // fill each allocated row 
        for(bbmp_Pixel *bp_nest = *bp; bp_nest < (*bp) + metadata->pixelarray_width; bp_nest++) {
            bp_nest->b = bp_raw_nest[0];
            bp_nest->g = bp_raw_nest[1];
            bp_nest->r = bp_raw_nest[2];

            bp_raw_nest += metadata->Bpp;
        }

        bp_raw += metadata->Bpr;
    }
    
    free(pixelarray_raw);
    return pixelarray_parsed;
}

static bbmp_PixelArray_Raw bbmp_convert_pixelarray(const bbmp_PixelArray parsed, const bbmp_Metadata *metadata, bbmp_PixelArray_Raw buffer) {
    /* 
     * Convert the parsed pixelarray pointed to by "parsed" to a raw pixelarray, and save it to the buffer pointed to by "buffer".
     * The buffer must be at least (metadata->Bpp * metadata->resolution) bytes large.
     * On success, it returns a pointer to the destination buffer, and on failure it returns a null pointer.
    */

    if(!parsed || !metadata || !buffer) return NULL;

    uint8_t *bp_raw = buffer;
    for(bbmp_PixelArray bp = parsed; bp < parsed + metadata->pixelarray_height; bp++) {
        for(bbmp_Pixel *bp_nest = *bp; bp_nest < (*bp) + metadata->pixelarray_width; bp_nest++) {
            buffer[0] = bp_nest->b; 
            buffer[1] = bp_nest->g;
            buffer[2] = bp_nest->r;

            bp_raw += metadata->Bpp;
        }
    }

    return buffer;
}

uint8_t *bbmp_write_image(const bbmp_Image *location, uint8_t *raw_bmp_data) {
    /* 
     * Write the BMP image pointed to by location to the raw_bmp_data pointed to by buffer.
     * The size of the raw_bmp_data should, at a minimum, be equal to location->metadata.Bpp * location->metadata.resolution + 14 + 40
     * (bytes per single pixel * number of pixels + the size of the BMP header + the size of the DIB bitmapinfo header)
     * If the size of the raw_bmp_data doesn't meet the size requirements, the behavior is undefined.
    */
    
    if(!location || !raw_bmp_data) return NULL;

    bbmp_Metadata meta = location->metadata;

    //write the header to the raw_bmp_data

    memcpy(raw_bmp_data + BSP_OFF_DIB_IDEN, meta.header_iden, 2);
    * (uint32_t *) (raw_bmp_data + BSP_OFF_FILESIZE) = meta.filesize;
    * (uint16_t *) (raw_bmp_data + BSP_OFF_RES1) = meta.res1;
    * (uint16_t *) (raw_bmp_data + BSP_OFF_RES2) = meta.res2;
    * (uint32_t *) (raw_bmp_data + BSP_OFF_PIXELARRAY_START) = meta.pixelarray_off;

    // write the DIB (BITMAPINFOHEADER, 'BM') header to the raw_bmp_data

    * (uint32_t *) (raw_bmp_data + BSP_OFF_DIB_SIZE) = meta.dib_size;
    * (int32_t *) (raw_bmp_data + BSP_OFF_DIB_IMGWIDTH) = meta.pixelarray_width;
    * (int32_t *) (raw_bmp_data + BSP_OFF_DIB_IMGHEIGHT) = meta.pixelarray_height;
    * (uint16_t *) (raw_bmp_data + BSP_OFF_DIB_PLANESNUM) = meta.panes_num;
    * (uint16_t *) (raw_bmp_data + BSP_OFF_DIB_BPP) = meta.bpp;
    * (uint32_t *) (raw_bmp_data + BSP_OFF_DIB_COMPRESSION) = meta.compression_method;
    * (uint32_t *) (raw_bmp_data + BSP_OFF_DIB_IMGSIZE) = meta.pixelarray_size;
    * (int32_t *) (raw_bmp_data + BSP_OFF_DIB_PPM_HORIZ) = meta.ppm_horiz;
    * (int32_t *) (raw_bmp_data + BSP_OFF_DIB_PPM_VERT ) = meta.ppm_vert;
    * (uint32_t *) (raw_bmp_data + BSP_OFF_DIB_COLORSNUM) = meta.colors_num;
    * (uint32_t *) (raw_bmp_data + BSP_OFF_DIB_IMPORTANTCOLORSNUM) = meta.colors_important_num;

    //convert the parsed pixelarray and save it to the offset to the start of the raw pixelarray in the raw bmp imge data
    if(bbmp_convert_pixelarray(location->pixelarray, &(location->metadata), raw_bmp_data + meta.pixelarray_off) == NULL) {
        fprintf(stderr, "bbmp_helper: Error converting parsed pixelarray.");
        return NULL;
    }
    
    return raw_bmp_data;
}


bool bbmp_debug_pixelarray(FILE *stream, const bbmp_Image *location, bool baseten) {
    /* 
     * Print RBG values to "stream" for each pixel in the parsed pixel array pointed to by "pixarray". 
     * If baseten is "true", print all RBG values in base 10 (decimal) instead of base 16 (hex)
     * Useful only when debugging relatively low-res images.
    */
    if(!stream || !location) return false;

    const char *format;

    if (baseten) {
        format = "\e[1m[[  \e[0m\e[31mR\e[0m:%3hhu - \e[32mG\e[0m:%3hhu - \e[34mB\e[0m:%3hhu\e[1m  ]]\e[0m ";
    } else {
        format = "\e[1m[[  \e[0m\e[31mR\e[0m:%.2hhX - \e[32mG\e[0m:%.2hhX - \e[34mB\e[0m:%.2hhX\e[1m  ]]\e[0m ";
    }

    for(bbmp_Pixel *bp = *(location->pixelarray); bp < (*location->pixelarray) + location->metadata.pixelarray_height; bp++) {
        for (bbmp_Pixel *bp_nest = bp; bp_nest < bp + location->metadata.pixelarray_width; bp_nest++) {
            fprintf(stream, format, bp->r, bp->g, bp->b);
        }

        fputc('\n', stream);
    }

    fputc('\n', stream);

    return true;
}
