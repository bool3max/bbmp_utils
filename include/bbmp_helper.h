#pragma once

#include "bbmp_parser.h"

struct bbmp_Pixel {
    uint8_t r, g, b;
}; typedef struct bbmp_Pixel bbmp_Pixel;

bbmp_Pixel *bbmp_get_pixelarray(uint8_t *raw_bmp_data, const struct Bmp_Info *metadata); 
bool bbmp_debug_pixelarray(FILE *stream, bbmp_Pixel *pixarray, const struct Bmp_Info *metadata, bool baseten); 
