/* A helper API for abstracting work with BMP image data. */

#include "bbmp_parser.h"
#include "bbmp_helper.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

PixelArrayRaw bbmp_get_raw_pixelarray_file(FILE *bmp_stream, struct Bmp_Info *metadata, bool strip) {
    /* 
        Allocates enough memory on the heap for the entire raw pixelarray of the bmp file, fills it, and returns a
        pointer to the memory. Size allocated will be equal to metadata.pixelarray_size. Does not write to 
        stream. Does not offset stream. The caller is responsible for manipulating and freeing the memory
        allocated and returned by this function. Returns a NULL ptr on failure.
    */

    if(bmp_stream == NULL || metadata == NULL) {
        return NULL;
    }

    long fileoff_before;
    if((fileoff_before = ftell(bmp_stream)) == -1) {
        perror("bmpparser: Error retrieving current stream offset: ");
        return NULL;
    }

    //seek stream to start of pixel array data
    if(fseek(bmp_stream, metadata->pixelarray_off, SEEK_SET) == -1) {
        perror("bmpparser: Error setting stream offset: ");
        return NULL;
    }

    //allocate heap memory for entirety of the pixel array
    PixelArrayRaw pixelarray_buffer = (PixelArrayRaw) malloc((size_t) metadata->pixelarray_size);

    //read entire pixelarray into the buffer
    if(fread(pixelarray_buffer, metadata->pixelarray_size, 1, bmp_stream) != 1) {
        perror("bmpparser: Error reading stream: ");
        free(pixelarray_buffer);
        return NULL;
    }

    if(strip) {
        pixelarray_buffer = bbmp_strip_raw_pixelarray(pixelarray_buffer, metadata);
    }

    //seek stream back to original position
    fseek(bmp_stream, fileoff_before, SEEK_SET);
    return pixelarray_buffer;
}

PixelArrayRaw bbmp_strip_raw_pixelarray(PixelArrayRaw pixarray, struct Bmp_Info *metadata) {
    /* Strip all null bytes from the pixelarray pointed to by pixarray, allowing for much easier parsing */
    size_t newsize = metadata->pixelarray_size_np;

    PixelArrayRaw temp = malloc(newsize); //enough space needed, without null padding bytes

    uint8_t *dest = temp;
    for(uint8_t *src = pixarray; src < pixarray + metadata->pixelarray_size; src += metadata->Bpr) {
        memcpy((void *) dest, src, metadata->Bpr_np);
        dest += metadata->Bpr_np;
    }

    pixarray = realloc(pixarray, newsize);
    memcpy(pixarray, temp, newsize);

    free(temp);
    return pixarray;
}

PixelArrayRaw bbmp_appendpadding_raw_pixelarray(PixelArrayRaw pixarray, struct Bmp_Info *metadata) {
    /* Append null padding bytes to end of each row of `pixarray`. Dynamically calculates number of needed null padding bytes
    based on width and depth of the image, (so that I can use it later when I make a text BMP export format).
    Assumes that pixarray is already stripped of all null padding bytes.
    */

    //calculate the amount of null padd bytes needed
    size_t nbytes;
    uint32_t t;
    for(t = (metadata->pixelarray_width * metadata->Bpp); (t % 4) != 0; t++) {;}
    nbytes = t - (metadata->pixelarray_width * metadata->Bpp);

    size_t newsize = (metadata->resolution * metadata->Bpp) + (metadata->pixelarray_height * nbytes);

    PixelArrayRaw temp = malloc(newsize);

    uint8_t *dest = temp;
    for(uint8_t *src = pixarray; src < pixarray + (metadata->resolution * metadata->Bpp); src += (metadata->pixelarray_width * metadata->Bpp)) {
        memcpy(dest, src, metadata->pixelarray_width * metadata->Bpp);
        for(size_t ins = 1; ins <= nbytes; ins++) {
            *(dest + (metadata->pixelarray_width * metadata->Bpp) + ins - 1) = 0x00;
        }

        dest += (metadata->pixelarray_width * metadata->Bpp) + nbytes;
    }

    pixarray = realloc(pixarray, newsize);
    memcpy(pixarray, temp, newsize);

    free(temp);

    //update metadata with new sizes
    //NOTE: doing this to future-proof the function...
    metadata->pixelarray_size = newsize;
    metadata->padding = nbytes;
    
    return pixarray;
}

void bbmp_debug_raw_pixelarray(PixelArrayRaw pixarray, struct Bmp_Info *metadata, bool stripped) {
    /* Print bytes of pixelarray pointed to by pixarray byte by byte, in groups of 3, to stdout.
    Also prints null padding bytes, if there are any.
    */

    printf("bmphelper: ");
    size_t iter = 1;
    for(uint8_t *byte = pixarray; byte < pixarray + (stripped ? metadata->pixelarray_size_np : metadata->pixelarray_size); byte++) {
        printf(iter % 3 == 0 ? "%3.2X -" : "%3.2X", *byte);
        iter++;
    }

    printf("\n");
}

Pixel *bbmp_parse_raw_pixelarray(PixelArrayRaw raw, struct Bmp_Info *metadata) {
    /* Parse a raw (STRIPPED) pixelarray, and return an array (a ptr..) of `Pixel` structs. Does not modify the raw pixel array in any way.
    The caller is responsible for freeing the allocated memory of the returned array (pointer..). The function assumes that the 
    passed `raw` PixelArray is already null-stripped. If it isn't, the function is likely to return an incorrect representation of the 
    pixels in the array. Always strip the array before calling this function. Providing the size of the returned array isn't needed
    as it will be equal to metadata->pixelarray_size_np (after stripping), which the caller has access to */

    size_t struct_size = sizeof(struct Pixel_t);
    Pixel *parsed = malloc(metadata->resolution * struct_size);

    Pixel *dest = parsed;
    for(uint8_t *src = raw; src < raw + metadata->pixelarray_size_np; src += metadata->Bpp) {
        //Parse every individual pixel from the raw pixelarray
        dest->location = src;
        
        dest++;
    }

    return parsed;
}

void bbmp_debug_parsed_pixelarray(Pixel *parsed, struct Bmp_Info *metadata) {
    /* Print a parsed pixelarray byte by byte. */

    printf("bmphelper: ");
    for(Pixel *pixel = parsed; pixel < parsed + metadata->resolution; pixel++) {
        if(metadata->bpp == 24) {
            printf("%3.2X%3.2X%3.2X -", *(pixel->location), *(pixel->location + 1), *(pixel->location + 2));
        } else if(metadata->bpp == 32) {
            printf("%3.2X%3.2X%3.2X%3.2X -", *(pixel->location), *(pixel->location + 1), *(pixel->location + 2), *(pixel->location + 3));
        }
    }

    printf("\n");
}
