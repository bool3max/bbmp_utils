#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define HEADER_BYTESIZE (14) //file header size, constant 14 bytes
#define BITMAPINFOHEADER_BYTESIZE (40) //hardcoded because we only support this particular (BM) DIB structure
#define BITMAPINFOHEADER_IDENTIFIER (0x4D42) //hardcoded for same reason, only support BM structure
#define BITMAPINFOHEADER_STRING "BM"

struct bbmp_Metadata {
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
    int32_t ppm_horiz; //horizontal resolution of the image in pixels per meter
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
}; typedef struct bbmp_Metadata bbmp_Metadata;

enum BSP_OFFSET {
    //Bitmap File Header (always 14 bytes)
    BSP_OFF_DIB_IDEN = 0x00,
    BSP_OFF_FILESIZE = 0x02,
    BSP_OFF_RES1 = 0x06,
    BSP_OFF_RES2 = 0x08,
    BSP_OFF_PIXELARRAY_START = 0x0A,
    //DIB (BITMAPINFOHEADER, `BM`), (40 bytes) -- only DIB supported for now
    BSP_OFF_DIB_SIZE = 0x0E,
    BSP_OFF_DIB_IMGWIDTH = 0x12,
    BSP_OFF_DIB_IMGHEIGHT = 0x16,
    BSP_OFF_DIB_PLANESNUM = 0x1A,
    BSP_OFF_DIB_BPP = 0x1C,
    BSP_OFF_DIB_COMPRESSION = 0x1E,
    BSP_OFF_DIB_IMGSIZE = 0x22,
    BSP_OFF_DIB_PPM_HORIZ = 0x26,
    BSP_OFF_DIB_PPM_VERT = 0x2A,
    BSP_OFF_DIB_COLORSNUM = 0x2E,
    BSP_OFF_DIB_IMPORTANTCOLORSNUM = 0x32
};

void bbmp_parse_bmp_metadata(unsigned char *raw_bmp_data, bbmp_Metadata *location); 
void bbmp_debug_bmp_metadata(const bbmp_Metadata *dbgtemp); 
