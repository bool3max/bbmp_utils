#pragma once

#include "bbmp_parser.h"

/*
 * Macros for calculating the memory space (in bytes) necesseray for storing a BMP image represented by `img`, the bbmp_Image type
 * Before using this macro, make sure that the metadata representation is up to date by calling bbmp_metacustomupdate() on the bbmp_Image instance.
*/
#define bbmp_image_calc_bytesize(img) ((14) + (40) + (img)->metadata.pixelarray_size)
/* 
 * Same as above but with raw dimensions and bits-per-pixel color depth
*/
#define bbmp_res_calc_bytesize(width, height, bpp) ((HEADER_BYTESIZE) + (BITMAPINFOHEADER_BYTESIZE) + (width * height * (bpp / 8)))

struct bbmp_Pixel {
    uint8_t r, g, b;
}; typedef struct bbmp_Pixel bbmp_Pixel;

typedef bbmp_Pixel **bbmp_PixelArray;

/*
 * A helper API structure designed to represent a full BMP image.
*/
struct bbmp_Image {
    struct bbmp_Metadata metadata; //metadata associated with the above pixelarray
    bbmp_PixelArray pixelarray; //a pixelarray in a parsed, easily consumable format
}; typedef struct bbmp_Image bbmp_Image;

bool bbmp_get_image(uint8_t *raw_bmp_data, bbmp_Image *location); 
bool bbmp_destroy_image(bbmp_Image *location);
bool bbmp_metacustomupdate(bbmp_Image *meta); 
bool bbmp_enlarge_pixelarray(bbmp_Image *img, int32_t width, int32_t height, const bbmp_Pixel *fill); 
uint8_t *bbmp_write_image(const bbmp_Image *location, uint8_t *raw_bmp_data); 
bool bbmp_debug_pixelarray(FILE *stream, const bbmp_Image *location, bool baseten); 
