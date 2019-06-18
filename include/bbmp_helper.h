#pragma once

#include "bbmp_parser.h"

struct bbmp_Pixel {
    uint8_t r, g, b;
}; 

typedef struct bbmp_Pixel bbmp_Pixel;
typedef bbmp_Pixel **bbmp_PixelArray;

/*
 * A helper API structure designed to represent a full BMP image.
*/
struct bbmp_Image {
    struct bbmp_Metadata metadata; //metadata associated with the above pixelarray
    bbmp_PixelArray pixelarray; //a pixelarray in a parsed, easily consumable format: this is not a FAM, it's a pointer
}; typedef struct bbmp_Image bbmp_Image;

bool bbmp_get_image(uint8_t *raw_bmp_data, struct bbmp_Image *location); 
bool bbmp_destroy_image(struct bbmp_Image *location);
bool bbmp_debug_pixelarray(FILE *stream, bbmp_PixelArray pixarray, const struct bbmp_Metadata *metadata, bool baseten); 

bbmp_PixelArray bbmp_get_pixelarray(uint8_t *raw_bmp_data, const struct bbmp_Metadata *metadata); 
