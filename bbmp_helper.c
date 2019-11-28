#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
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

bool bbmp_get_image(uint8_t *raw_bmp_data, bbmp_Image *location) {
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

bbmp_Image *bbmp_create_image(int32_t pixelarray_width, int32_t pixelarray_height, uint16_t bpp, const bbmp_Pixel *fill, bbmp_Image *location) {
    /*
     * Create a "canvas" or a blank instance of a BMP image and save it to *location, which must be a valid pointer to a bbmp_Image structure.
     * The instance is created using basic parameters passed as parameters: pixelarray_width, pixelarray_height, bpp (color depth). 
     * The bbmp_Metadata structure and all of its members are calculated based off of these basic parameters. 
     * If "fill" is not a null pointer, the canvas is filled with pixels based off of this reference pixel. Otherwise, the pixelarray memory is left uninitialized.
     * (it is *allocated*, but is left uninitialized)
    */

    if(!location) return false;
    
    // save pixelarray dimensions and color depth to metadata
    location->metadata.pixelarray_width = pixelarray_width;
    location->metadata.pixelarray_height = pixelarray_height;
    location->metadata.bpp = bpp;

    // update metadata properties that bbmp_metacustomupdate() doesn't touch
    memcpy(location->metadata.header_iden, BITMAPINFOHEADER_STRING, 3);
    location->metadata.res1 = 0;
    location->metadata.res2 = 0;
    location->metadata.pixelarray_off = HEADER_BYTESIZE + BITMAPINFOHEADER_BYTESIZE;
    location->metadata.dib_size = BITMAPINFOHEADER_BYTESIZE;
    location->metadata.panes_num = 1;
    location->metadata.compression_method = 0;
    location->metadata.ppm_horiz = 0; 
    location->metadata.ppm_vert = 0;
    location->metadata.colors_num = 0;
    location->metadata.colors_important_num = 0;
    location->metadata.Bpp = bpp / 8;

    // updates Bpr, Bpr_np, padding, resolution, pixelarray_size_np, pixelarray_size and filesize metadata properties
    bbmp_metacustomupdate(location);

    if(fill) {
        // allocate and fill pixelarray memory     
        location->pixelarray = malloc(location->metadata.pixelarray_height * sizeof(bbmp_Pixel *));
        if(!location->pixelarray) {
            perror("bbmp_helper: Failed allocating memory\n");
            return NULL;
        }

        for(bbmp_PixelArray bp = location->pixelarray; bp < location->pixelarray + location->metadata.pixelarray_height; bp++) {
            *bp = malloc(location->metadata.pixelarray_width * sizeof(bbmp_Pixel));

            if(!(*bp)) {
                perror("bbmp_helper: Failed allocating memory\n");
                free(location->pixelarray);
                return NULL;
            }

            for(bbmp_Pixel *bp_nest = *bp; bp_nest < (*bp) + location->metadata.pixelarray_width; bp_nest++) {
                bp_nest->r = fill->r;
                bp_nest->g = fill->g;
                bp_nest->b = fill->b;
            }
        }
    }

    return location;
}

bool bbmp_destroy_image(bbmp_Image *location) {
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
     * The buffer must be at least metadata->pixelarray_size bytes large.
     * On success, it returns a pointer to the destination buffer, and on failure it returns a null pointer.
    */

    if(!parsed || !metadata || !buffer) return NULL;

    uint8_t *bp_raw = buffer;
    for(bbmp_PixelArray bp = parsed; bp < parsed + metadata->pixelarray_height; bp++) {
        uint8_t *bp_raw_nest = bp_raw;

        for(bbmp_Pixel *bp_nest = *bp; bp_nest < (*bp) + metadata->pixelarray_width; bp_nest++) {
            bp_raw_nest[0] = bp_nest->b; 
            bp_raw_nest[1] = bp_nest->g;
            bp_raw_nest[2] = bp_nest->r;

            bp_raw_nest += metadata->Bpp;
        }

        //append padding
        memset(bp_raw_nest, 0x0, metadata->padding);

        bp_raw += metadata->Bpr;
    }

    return buffer;
}

bool bbmp_metacustomupdate(bbmp_Image *img) {
    /* 
     * Updates properties of the bbmp_Metadata structure based on the pixelarray_width, pixelarray_height, and bpp properties.
     * The purpose of this function is to be called by the user in order to calculate custom metadata such as the padding (per row).
     * If the API consumer modifies the height/width of the pixelarray associated with this metadata, they should manually call this function before
     * attempting to write the said pixelarray to a buffer using bbmp_write_imge.
    */

    if(!img) return false;

    // not all fields are updated, since some of them are constant (e.g. .bpp and .Bpp)
    
    img->metadata.Bpr = ceil(( (double) img->metadata.bpp * img->metadata.pixelarray_width) / 32) * 4;
    img->metadata.Bpr_np = (img->metadata.pixelarray_width * img->metadata.Bpp);
    img->metadata.padding = img->metadata.Bpr - (img->metadata.pixelarray_width * img->metadata.Bpp);
    img->metadata.resolution = img->metadata.pixelarray_height * img->metadata.pixelarray_width;
    img->metadata.pixelarray_size_np = img->metadata.resolution * img->metadata.Bpp;

    img->metadata.pixelarray_size = img->metadata.pixelarray_size_np + (img->metadata.pixelarray_height * img->metadata.padding);
    img->metadata.filesize = HEADER_BYTESIZE + BITMAPINFOHEADER_BYTESIZE + img->metadata.pixelarray_size;

    return true;
}

bool bbmp_enlarge_pixelarray(bbmp_Image *img, int32_t width, int32_t height, const bbmp_Pixel *fill) {
    /* 
     * Dynamically update the size of the pixelarray of the associated bbmp_Image instance pointed to by `img`. 
     * If either of the passed dimensions is lower than that of the current pixelarray, the function returns false. 
     * If either is larger, the pixelarray is resized and reallocated, and the metadata is updated using bbmp_metacustomupdate.
     * When either passed dimension is larger, the new blank rows/columns are appended to the top/right of the image respectively, and their pixels values
     * are initialized to that of the "fill" reference pixel. 
     * Returns true on success.
    */
    
    if((!img || !fill) || height < img->metadata.pixelarray_height || width < img->metadata.pixelarray_width) return false;

    int32_t prev_width = img->metadata.pixelarray_width,
            prev_height = img->metadata.pixelarray_height;

    //update metadata with new dimensions
    img->metadata.pixelarray_width = width;
    img->metadata.pixelarray_height = height;
    
    //update rest of the metadata based on the new dimension(s)
    bbmp_metacustomupdate(img);
    
    if(height > prev_height) {
        img->pixelarray = realloc(img->pixelarray, img->metadata.pixelarray_height * sizeof(bbmp_Pixel *));
        if(!img->pixelarray) { 
            perror("bbmp_helper: Error enlarging HEIGHT dimension of pixelarray: ");
            return false;
        }

        // initialize new pointers with more rows and fill those rows with the reference pixel
        for(bbmp_PixelArray bp = img->pixelarray + prev_height; bp < img->pixelarray + height; bp++) {
            *bp = malloc(img->metadata.pixelarray_width * sizeof(bbmp_Pixel));
            if(!(*bp)) {
                perror("bbmp_helper: Error enlarging HEIGHT dimension of pixelarray: ");
                return false;
            }

            for(bbmp_Pixel *bp_nest = *bp; bp_nest < *bp + img->metadata.pixelarray_width; bp_nest++) {
                bp_nest->b = fill->b;
                bp_nest->g = fill->g;
                bp_nest->r = fill->r;
            }
        }
    }

    if(width > prev_width) {
        for(bbmp_PixelArray bp = img->pixelarray; bp < img->pixelarray + img->metadata.pixelarray_height; bp++) {
            *bp = realloc(*bp, img->metadata.pixelarray_width * sizeof(bbmp_Pixel));
            if(!(*bp)) {
                perror("bbmp_helper: Error enlarging WIDTH dimension of pixelarray: ");
                return false;
            }

            for(bbmp_Pixel *bp_nest = *bp + prev_width; bp_nest < *bp + img->metadata.pixelarray_width; bp_nest++) {
                bp_nest->b = fill->b;
                bp_nest->g = fill->g;
                bp_nest->r = fill->r;
            }
        }
    }
    
    return true;
}

uint8_t *bbmp_write_image(const bbmp_Image *location, uint8_t *raw_bmp_data) {
    /* 
     * Write the BMP image pointed to by location to the raw_bmp_data pointed to by buffer.
     * The size of the raw_bmp_data should, at a minimum, be equal to 14 + 40 + metadata->pixelarray_size
     * If the size of the raw_bmp_data doesn't meet the size requirements, the behavior is undefined.
     * Before calling this function, the API consumer should call bbmp_metacustomupdate on the associated bbmp_Image structure, in order to update
     * the fields that this function utilizies.
    */
    
    if(!location || !raw_bmp_data) return NULL;

    #define meta location->metadata

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
    * (int32_t *) (raw_bmp_data + BSP_OFF_DIB_PPM_VERT) = meta.ppm_vert;
    * (uint32_t *) (raw_bmp_data + BSP_OFF_DIB_COLORSNUM) = meta.colors_num;
    * (uint32_t *) (raw_bmp_data + BSP_OFF_DIB_IMPORTANTCOLORSNUM) = meta.colors_important_num;

    //convert the parsed pixelarray and save it to the offset to the start of the raw pixelarray in the raw bmp imge data
    if(bbmp_convert_pixelarray(location->pixelarray, &(location->metadata), raw_bmp_data + meta.pixelarray_off) == NULL) {
        fprintf(stderr, "bbmp_helper: Error converting parsed pixelarray.");
        return NULL;
    }

    #undef meta
    
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

    // pointer arithmetic based indexing

    for(bbmp_PixelArray bp = location->pixelarray; bp < location->pixelarray + location->metadata.pixelarray_height; bp++) {
        for (bbmp_Pixel *bp_nest = *bp; bp_nest < *bp + location->metadata.pixelarray_width; bp_nest++) {
            fprintf(stream, format, bp_nest->r, bp_nest->g, bp_nest->b);
        }

        fputc('\n', stream);
    }

    fputc('\n', stream);

    return true;
}
