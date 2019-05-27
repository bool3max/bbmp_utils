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

bool bbmp_parse_bmp_metadata(unsigned char *metadata_buffer, struct Bmp_Info *metadata) {
    /* 
        Reads BMP file metadata from the memory pointed to by metadata_buffer. metadata_buffer must be at least 54 (bound to change?) bytes wide. Saves all metadata in the structure pointed to by metadata.
        Returns true on success, false on failure. If memloc is < 54 bytes wide, behaviour is undefined
    */
    if(!metadata_buffer || !metadata) return false;

    #define BITMAPINFOHEADER_IDENTIFIER (0x4D42) //hardcoded for same reason, only support BM structure

    if(* (uint16_t *) (metadata_buffer + BSP_OFF_DIB_IDEN) != BITMAPINFOHEADER_IDENTIFIER) {
        printf("bmpparser: Only the BM DIB header structure is supported. Quitting...\n");
        return false;
    }

    //--------- begin setting each property of the struct one by one
    metadata->header_iden[0] = *(metadata_buffer + BSP_OFF_DIB_IDEN);
    metadata->header_iden[1] = *(metadata_buffer + BSP_OFF_DIB_IDEN + 1);
    metadata->header_iden[2] = '\0';

    metadata->filesize = * (uint32_t*) (metadata_buffer + BSP_OFF_FILESIZE);

    metadata->res1 = * (uint16_t *) (metadata_buffer + BSP_OFF_RES1);
    metadata->res2 = * (uint16_t *) (metadata_buffer + BSP_OFF_RES2);

    metadata->pixelarray_off = * (uint32_t *) (metadata_buffer + BSP_OFF_PIXELARRAY_START);

    metadata->dib_size = * (uint32_t *) (metadata_buffer + BSP_OFF_DIB_SIZE);

    metadata->pixelarray_width = * (int32_t *) (metadata_buffer + BSP_OFF_DIB_IMGWIDTH);
    metadata->pixelarray_height = * (int32_t *) (metadata_buffer + BSP_OFF_DIB_IMGHEIGHT);

    metadata->panes_num = * (uint16_t *) (metadata_buffer + BSP_OFF_DIB_PLANESNUM);
    
    metadata->bpp = * (uint16_t *) (metadata_buffer + BSP_OFF_DIB_BPP);

    metadata->compression_method = * (uint32_t *) (metadata_buffer + BSP_OFF_DIB_COMPRESSION);

    metadata->pixelarray_size = * (uint32_t *) (metadata_buffer + BSP_OFF_DIB_IMGSIZE);

    metadata->ppm_horiz = * (int32_t *) (metadata_buffer + BSP_OFF_DIB_PPM_HORIZ);
    metadata->ppm_vert = * (int32_t *) (metadata_buffer + BSP_OFF_DIB_PPM_VERT);

    metadata->colors_num = * (uint32_t *) (metadata_buffer + BSP_OFF_DIB_COLORSNUM);
    metadata->colors_important_num = * (uint32_t *) (metadata_buffer + BSP_OFF_DIB_IMPORTANTCOLORSNUM);
    
    //CUSTOM FIELDS (not in the spec or in the file, provided for ease of use)
    metadata->Bpr = ceil(( (double) metadata->bpp * metadata->pixelarray_width) / 32) * 4;
    metadata->Bpp = metadata->bpp / 8;
    metadata->Bpr_np = (metadata->pixelarray_width * metadata->Bpp);
    metadata->padding = metadata->Bpr - (metadata->pixelarray_width * metadata->Bpp);
    metadata->resolution = metadata->pixelarray_height * metadata->pixelarray_width;
    metadata->pixelarray_size_np = metadata->resolution * metadata->Bpp;

    return true;
}

bool bbmp_parse_bmp_metadata_file(FILE *bmp_stream, struct Bmp_Info *metadata)  {
    /* Reads BMP file metadata from the stream pointed to by bmp_stream. Saves all metadata in the structure pointed to by metadata. Returns true on success, false on failure */

    if(!bmp_stream || !metadata) return false;
    
    //save previous stream offset
    long int fileoff_before;
    if((fileoff_before = ftell(bmp_stream)) == -1) {
        perror("bmpparser: Error retrieving current stream offset: ");
        return false;
    }

    //set offset to beginning for reading purposes
    if(fseek(bmp_stream, 0, SEEK_SET) == -1) {
        perror("bmpparser: Error seeking stream: ");
        return false;
    }

    // allocate space for metadata
    char *metadata_buffer = malloc(HEADER_BYTESIZE + BITMAPINFOHEADER_BYTESIZE);
    if(!metadata_buffer) {
        perror("bmpparser: Error allocating memory: ");
        fseek(bmp_stream, fileoff_before, SEEK_SET);
        return false;
    }

    //read all 54 bytes of metadata
    if(fread(metadata_buffer, HEADER_BYTESIZE + BITMAPINFOHEADER_BYTESIZE, 1, bmp_stream) != 1) {
        perror("bmpparser: Error reading metadata from stream: ");
        free(metadata_buffer);
        fseek(bmp_stream, fileoff_before, SEEK_SET);
        return false;
    }

    if(!bbmp_parse_bmp_metadata((unsigned char *) metadata_buffer, metadata)) {
        fseek(bmp_stream, fileoff_before, SEEK_SET);
        free(metadata_buffer);
        return false;
    }
    
    //cleanup
    free(metadata_buffer);
    fseek(bmp_stream, fileoff_before, SEEK_SET);

    return true;
}

bool bbmp_write_bmp_metadata(unsigned char *metadata_buffer, struct Bmp_Info *metadata) {
    /* 
        Copy all properties (fields) of the Bmp_Info metadata struct pointed to by "location" to their respective locations inside of the raw metadata memory pointed to by "metadata_buffer"
    */

    // BMP file header

    if(!metadata_buffer || !metadata) return false;

    *(metadata_buffer + BSP_OFF_DIB_IDEN) = metadata->header_iden[0];
    *(metadata_buffer + BSP_OFF_DIB_IDEN + 1) = metadata->header_iden[1];

    *(metadata_buffer + BSP_OFF_FILESIZE) = metadata->filesize;
    *(metadata_buffer + BSP_OFF_RES1) = metadata->res1;
    *(metadata_buffer + BSP_OFF_RES2) = metadata->res2;
    *(metadata_buffer + BSP_OFF_PIXELARRAY_START) = metadata->pixelarray_off;

    // DIB header metadata

    *(metadata_buffer + BSP_OFF_DIB_SIZE) = metadata->dib_size;
    *(metadata_buffer + BSP_OFF_DIB_IMGWIDTH) = metadata->pixelarray_width;
    *(metadata_buffer + BSP_OFF_DIB_IMGHEIGHT) = metadata->pixelarray_height;
    *(metadata_buffer + BSP_OFF_DIB_PLANESNUM) = metadata->panes_num;
    *(metadata_buffer + BSP_OFF_DIB_BPP) = metadata->bpp;
    *(metadata_buffer + BSP_OFF_DIB_COMPRESSION) = metadata->compression_method;
    *(metadata_buffer + BSP_OFF_DIB_IMGSIZE) = metadata->pixelarray_size;
    *(metadata_buffer + BSP_OFF_DIB_PPM_HORIZ) = metadata->ppm_horiz;
    *(metadata_buffer + BSP_OFF_DIB_PPM_VERT) = metadata->ppm_vert;
    *(metadata_buffer + BSP_OFF_DIB_COLORSNUM) = metadata->colors_num;
    *(metadata_buffer + BSP_OFF_DIB_IMPORTANTCOLORSNUM) = metadata->colors_important_num;

    return true;
}

bool bbmp_write_bmp_metadata_file(FILE *bmp_stream, struct Bmp_Info *metadata) {
    /* 
     * Read BMP metadata from the struct pointed to by "metadata" and save it to the FILE stream pointed to by
     * "bmp_stream". The file offset of "bmp_stream" is untouched.
     * "bmp_stream" must represent a file stream of a valid BMP file.
    */

    if(!bmp_stream || !metadata) return false;

    long int fileoff_before;
    if((fileoff_before = ftell(bmp_stream)) == -1) {
        perror("bmpparser: Error retrieving current stream offset: ");
        return false;
    }

    if(fseek(bmp_stream, 0, SEEK_SET) == -1) {
        perror("bmpparser: Error seeking stream: ");
        return false;
    }

    // allocate space for metadata
    char *metadata_buffer = malloc(HEADER_BYTESIZE + BITMAPINFOHEADER_BYTESIZE);
    if(!metadata_buffer) {
        perror("bmpparser: Error allocating memory: ");
        fseek(bmp_stream, fileoff_before, SEEK_SET);
        return false;
    }

    // read all 54 bytes of metadata
    if(fread(metadata_buffer, HEADER_BYTESIZE + BITMAPINFOHEADER_BYTESIZE, 1, bmp_stream) != 1) {
        perror("bmpparser: Error reading metadata from stream: ");
        free(metadata_buffer);
        fseek(bmp_stream, fileoff_before, SEEK_SET);
        return false;
    }

    // write new metadata to the temporary buffer
    if(!bbmp_write_bmp_metadata((unsigned char *) metadata_buffer, metadata)) {
        perror("bmpparser: Error modifying metadata: "); 
        fseek(bmp_stream, fileoff_before, SEEK_SET);
        free(metadata_buffer);
        return false;
    }

    //write the (now modified) buffer back to the file
    fseek(bmp_stream, 0, SEEK_SET); //rewind stream back to start
    if(fwrite(metadata_buffer, HEADER_BYTESIZE + BITMAPINFOHEADER_BYTESIZE, 1, bmp_stream) != 1) {
        perror("bmpparser: Error writing metadata buffer back to file: ");
        free(metadata_buffer);
        fseek(bmp_stream, fileoff_before, SEEK_SET);
        return false;
    }

    free(metadata_buffer);
    fseek(bmp_stream, fileoff_before, SEEK_SET);
    
    return true;
}


void bbmp_debug_bmp_metadata(const struct Bmp_Info *dbgtemp) {
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
