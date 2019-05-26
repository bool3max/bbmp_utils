#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

struct Bmp_Info {
    //bitmap file header
    char header_iden[2 + 1]; //the 2-letter (2byte) ascii representation of the DIB header used in the file
    uint32_t filesize; //the filesize (in bytes) of the actual .BMP file on disk
    uint16_t res1; //reserved
    uint16_t res2; //reserved
    uint32_t pixelarray_off; //the offset to the start of the actual pixelarray (imagedata)
    //DIB header metadata
    uint32_t dib_size; //the size of the DIB header in bytes
    int32_t pixelarray_width; //the width of the pixelarray (actual image), in pixels
    int32_t pixelarray_height; //the height of the pixelarray (actual image), in pixels
    uint16_t panes_num; //always 1
    uint16_t bpp; //bits per single pixel (usually 24 or 32)
    uint32_t compression_method; //the compression method
    uint32_t pixelarray_size; //the size (in bytes) of the pixelarray (actual image data)
    int32_t ppm_horiz; //horizontal resolution if image in pixels per meter
    int32_t ppm_vert; //vertical ---
    uint32_t colors_num; //the number of colors in the color palette
    uint32_t colors_important_num; //generally ignored
    //custom fields
    uint32_t Bpr; //bytes per image row, including null padding bytes
    uint16_t Bpp; //bytes per single pixel (usually 3 or 4)
    uint32_t Bpr_np; //bytes per image row, excluind null padding bytes
    uint32_t resolution; //the total number of pixels in the pixelarray (width * height)
    uint32_t padding; //the number of 0x00 padding bytes at the end of each row, may be zero
    uint32_t pixelarray_size_np; //the total size of the pixelarray in bytes, excluding all null padding bytes
};


bool bbmp_parse_bmp_metadata(unsigned char *metadata_buffer, struct Bmp_Info *location); 
bool bbmp_parse_bmp_metadata_file(FILE *bmp_stream, struct Bmp_Info *location);
bool bbmp_write_bmp_metadata(unsigned char *metadata_buffer, struct Bmp_Info *metadata);
bool bbmp_write_bmp_metadata_file(FILE *bmp_steam, struct Bmp_Info *metadata);
void bbmp_debug_bmp_metadata(const struct Bmp_Info *dbgtemp); 
