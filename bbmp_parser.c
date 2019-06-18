#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "bbmp_parser.h"

/*  
 *  A header-only file composed of a few functions for parsing data and metadata from BMP4.0 files.
 *  Only supports the `BM` BITMAPINFOHEADER DIB structure.
*/

#define HEADER_BYTESIZE (14) //file header size, constant 14 bytes
#define BITMAPINFOHEADER_BYTESIZE (40) //hardcoded because we only support this particular (BM) DIB structure
#define BITMAPINFOHEADER_IDENTIFIER (0x4D42) //hardcoded for same reason, only support BM structure

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

bool bbmp_parse_bmp_metadata(unsigned char *raw_bmp_data, struct bbmp_Metadata *metadata) {
    /* 
        Reads BMP file metadata from the memory pointed to by raw_bmp_data. raw_bmp_data must be at least 54 (bound to change?) bytes wide. Saves all metadata in the structure pointed to by metadata.
        Returns true on success, false on failure. If memloc is < 54 bytes wide, behaviour is undefined
    */
    if(!raw_bmp_data || !metadata) return false;

    if(* (uint16_t *) (raw_bmp_data + BSP_OFF_DIB_IDEN) != BITMAPINFOHEADER_IDENTIFIER) {
        fprintf(stderr, "bmpparser: Only the BM DIB header structure is supported. Quitting...\n");
        return false;
    }

    //--------- begin setting each property of the struct one by one
    metadata->header_iden[0] = *(raw_bmp_data + BSP_OFF_DIB_IDEN);
    metadata->header_iden[1] = *(raw_bmp_data + BSP_OFF_DIB_IDEN + 1);
    metadata->header_iden[2] = '\0';

    metadata->filesize = * (uint32_t*) (raw_bmp_data + BSP_OFF_FILESIZE);

    metadata->res1 = * (uint16_t *) (raw_bmp_data + BSP_OFF_RES1);
    metadata->res2 = * (uint16_t *) (raw_bmp_data + BSP_OFF_RES2);

    metadata->pixelarray_off = * (uint32_t *) (raw_bmp_data + BSP_OFF_PIXELARRAY_START);

    metadata->dib_size = * (uint32_t *) (raw_bmp_data + BSP_OFF_DIB_SIZE);

    metadata->pixelarray_width = * (int32_t *) (raw_bmp_data + BSP_OFF_DIB_IMGWIDTH);
    metadata->pixelarray_height = * (int32_t *) (raw_bmp_data + BSP_OFF_DIB_IMGHEIGHT);

    metadata->panes_num = * (uint16_t *) (raw_bmp_data + BSP_OFF_DIB_PLANESNUM);
    
    metadata->bpp = * (uint16_t *) (raw_bmp_data + BSP_OFF_DIB_BPP);

    metadata->compression_method = * (uint32_t *) (raw_bmp_data + BSP_OFF_DIB_COMPRESSION);

    metadata->pixelarray_size = * (uint32_t *) (raw_bmp_data + BSP_OFF_DIB_IMGSIZE);

    metadata->ppm_horiz = * (int32_t *) (raw_bmp_data + BSP_OFF_DIB_PPM_HORIZ);
    metadata->ppm_vert = * (int32_t *) (raw_bmp_data + BSP_OFF_DIB_PPM_VERT);

    metadata->colors_num = * (uint32_t *) (raw_bmp_data + BSP_OFF_DIB_COLORSNUM);
    metadata->colors_important_num = * (uint32_t *) (raw_bmp_data + BSP_OFF_DIB_IMPORTANTCOLORSNUM);
    
    //CUSTOM FIELDS (not in the spec or in the file, provided for ease of use)
    metadata->Bpp = metadata->bpp / 8;
    metadata->Bpr = ceil(( (double) metadata->bpp * metadata->pixelarray_width) / 32) * 4;
    metadata->Bpr_np = (metadata->pixelarray_width * metadata->Bpp);
    metadata->padding = metadata->Bpr - (metadata->pixelarray_width * metadata->Bpp);
    metadata->resolution = metadata->pixelarray_height * metadata->pixelarray_width;
    metadata->pixelarray_size_np = metadata->resolution * metadata->Bpp;

    return true;
}

bool bbmp_write_bmp_metadata(unsigned char *raw_bmp_data, struct bbmp_Metadata *metadata) {
    /* 
        Copy all properties (fields) of the bbmp_Metadata metadata struct pointed to by "location" to their respective locations inside of the raw metadata memory pointed to by "raw_bmp_data"
    */

    // BMP file header

    if(!raw_bmp_data || !metadata) return false;

    *(raw_bmp_data + BSP_OFF_DIB_IDEN) = metadata->header_iden[0];
    *(raw_bmp_data + BSP_OFF_DIB_IDEN + 1) = metadata->header_iden[1];

    *(raw_bmp_data + BSP_OFF_FILESIZE) = metadata->filesize;
    *(raw_bmp_data + BSP_OFF_RES1) = metadata->res1;
    *(raw_bmp_data + BSP_OFF_RES2) = metadata->res2;
    *(raw_bmp_data + BSP_OFF_PIXELARRAY_START) = metadata->pixelarray_off;

    // DIB header metadata

    *(raw_bmp_data + BSP_OFF_DIB_SIZE) = metadata->dib_size;
    *(raw_bmp_data + BSP_OFF_DIB_IMGWIDTH) = metadata->pixelarray_width;
    *(raw_bmp_data + BSP_OFF_DIB_IMGHEIGHT) = metadata->pixelarray_height;
    *(raw_bmp_data + BSP_OFF_DIB_PLANESNUM) = metadata->panes_num;
    *(raw_bmp_data + BSP_OFF_DIB_BPP) = metadata->bpp;
    *(raw_bmp_data + BSP_OFF_DIB_COMPRESSION) = metadata->compression_method;
    *(raw_bmp_data + BSP_OFF_DIB_IMGSIZE) = metadata->pixelarray_size;
    *(raw_bmp_data + BSP_OFF_DIB_PPM_HORIZ) = metadata->ppm_horiz;
    *(raw_bmp_data + BSP_OFF_DIB_PPM_VERT) = metadata->ppm_vert;
    *(raw_bmp_data + BSP_OFF_DIB_COLORSNUM) = metadata->colors_num;
    *(raw_bmp_data + BSP_OFF_DIB_IMPORTANTCOLORSNUM) = metadata->colors_important_num;

    return true;
}


void bbmp_debug_bmp_metadata(const struct bbmp_Metadata *dbgtemp) {
    /* 
     * Prints all fields of the metadata storage struct to stdout. Useful for debugging
     * All printed in base 10.
    */

    //TODO: left justify the fields and prettify the output
    printf("bmpparser: header_iden -> %s\n", dbgtemp->header_iden); //is null terminated so %s works...
    printf("bmpparser: filesize -> %u\n", dbgtemp->filesize);
    printf("bmpparser: res1 -> %hu, res2 -> %hu\n", dbgtemp->res1, dbgtemp->res2);
    printf("bmpparser: pixelarray_off -> %u\n", dbgtemp->pixelarray_off);
    printf("bmpparser: dib_size -> %u\n", dbgtemp->dib_size);
    printf("bmpparser: pixelarray_width -> %d, pixelarray_height -> %d\n", dbgtemp->pixelarray_width, dbgtemp->pixelarray_height);
    printf("bmpparser: panes_num -> %hu\n", dbgtemp->panes_num);
    printf("bmpparser: bpp -> %hu\n", dbgtemp->bpp);
    printf("bmpparser: compression_method -> %u\n", dbgtemp->compression_method);
    printf("bmpparser: pixelarray_size -> %u\n", dbgtemp->pixelarray_size);
    printf("bmpparser: ppm_horiz -> %d, ppm_vert -> %d\n", dbgtemp->ppm_horiz, dbgtemp->ppm_vert);
    printf("bmpparser: colors_num -> %u, colors_important_num -> %u\n", dbgtemp->colors_num, dbgtemp->colors_important_num);
    
    //custom fields
    printf("bmpparser: Bpr -> %u\n", dbgtemp->Bpr);
    printf("bmpparser: Bpr_np -> %u\n", dbgtemp->Bpr_np);
    printf("bmpparser: Bpp -> %hu\n", dbgtemp->Bpp);
    printf("bmpparser: resolution -> %u\n", dbgtemp->resolution);
    printf("bmpparser: padding -> %u\n", dbgtemp->padding);
    printf("bmpparser: pixelarray_size_np -> %u\n", dbgtemp->pixelarray_size_np);
}
