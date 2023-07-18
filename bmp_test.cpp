#include <fstream>
#include <stdlib.h>
#include <iostream>

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int DWORD;

#pragma pack(1)
typedef struct {
    WORD  Type;
    DWORD Size;
    WORD  Reserved1;
    WORD  Reserved2;
    DWORD OffBits;
} BitMapFileHeader_t;

typedef struct {
    DWORD Size;
    DWORD Width;
    DWORD Height;
    WORD  Planes;
    WORD  BitCount;
    DWORD Compression;
    DWORD SizeImage;
    DWORD XPixPerMeter;
    DWORD YPixPerMeter;
    DWORD ClrUsed;
    DWORD ClrImportant;
} BitMapInfoHeader_t;

typedef struct {
    BitMapFileHeader_t File;
    BitMapInfoHeader_t Info;
} BitMap_t;
#pragma pack()

void save(unsigned char *img, char *filename, int width, int height){
    FILE *fp;
    BitMap_t bitmap;

    bitmap.File.Type = 19778;
    bitmap.File.Size = 14 + 40 + width*height*3;
    std::cout << "bitmap.File.Size = " << bitmap.File.Size << std::endl;

    bitmap.File.Reserved1 = 0;
    bitmap.File.Reserved2 = 0;
    bitmap.File.OffBits = 14 + 40;

    bitmap.Info.Size = 40;
    bitmap.Info.Width = width;
    bitmap.Info.Height = height;
    bitmap.Info.Planes = 1;
    bitmap.Info.BitCount = 24;
    bitmap.Info.Compression = 0;
    bitmap.Info.SizeImage = 0;
    bitmap.Info.XPixPerMeter = 0;
    bitmap.Info.YPixPerMeter = 0;
    bitmap.Info.ClrUsed = 0;
    bitmap.Info.ClrImportant = 0;

    fp = fopen(filename, "wb");
    fwrite(&bitmap, sizeof(bitmap), 1, fp);
    fwrite(img, sizeof(*img)*width*height*3, 1, fp);
    fclose(fp);
};

int main(){
    int width = 300;
    int height = 300;

    unsigned char *img;
    img = (unsigned char*)malloc(width*height*3*sizeof(unsigned char));

    for (int i = 0; i < width * height*3; i+=3) {
        img[i+0] = 0;  // Blue
        img[i+1] = i;  // Green
        img[i+2] = 0;  // Red
    }

    save(img, "output.bmp", width, height);

    return 0;
}