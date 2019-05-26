#pragma once

#include "bbmp_parser.h"

typedef uint8_t* PixelArrayRaw; //data type used to point to an in-memory raw binary array of pixels, rows may or may not be null padded

typedef struct Pixel_t {
    uint8_t *location;
} Pixel; 

PixelArrayRaw bbmp_get_raw_pixelarray_file(FILE *bmp_stream, struct Bmp_Info *metadata, bool strip); 
PixelArrayRaw bbmp_strip_raw_pixelarray(PixelArrayRaw pixarray, struct Bmp_Info *metadata); 
void bbmp_debug_raw_pixelarray(PixelArrayRaw pixarray, struct Bmp_Info *metadata, bool stripped); 
PixelArrayRaw bbmp_appendpadding_raw_pixelarray(PixelArrayRaw pixarray, struct Bmp_Info *metadata); 
Pixel *bbmp_parse_raw_pixelarray(PixelArrayRaw raw, struct Bmp_Info *metadata); 
void bbmp_debug_parsed_pixelarray(Pixel *parsed, struct Bmp_Info *metadata); 
