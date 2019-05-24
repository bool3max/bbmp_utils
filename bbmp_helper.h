#pragma once

typedef uint8_t* PixelArrayRaw; //data type used to point to an in-memory raw binary array of pixels, rows may or may not be null padded

typedef struct Pixel_t {
    uint8_t *location;
} Pixel; 

PixelArrayRaw GetRawBmpPixelArray(FILE *bmp_stream, struct Bmp_Info *metadata, bool strip); 
PixelArrayRaw StripRawPixelArray(PixelArrayRaw pixarray, struct Bmp_Info *metadata); 
void DebugRawPixelArray(PixelArrayRaw pixarray, struct Bmp_Info *metadata, bool stripped); 
PixelArrayRaw AppendNullRawPixelArray(PixelArrayRaw pixarray, struct Bmp_Info *metadata); 
Pixel *ParseRawPixelArray(PixelArrayRaw raw, struct Bmp_Info *metadata); 
void DebugParsedPixelArray(Pixel *parsed, struct Bmp_Info *metadata); 
