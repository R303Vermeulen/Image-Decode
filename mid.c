#include<stdio.h>
#include<stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

typedef unsigned char byte;
typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef unsigned int LONG;

typedef struct tagBITMAPFILEHEADER 
{ 
    WORD bfType; //specifies the file type 
    DWORD bfSize; //specifies the size in bytes of the bitmap file 
    WORD bfReserved1; //reserved; must be 0 
    WORD bfReserved2; //reserved; must be 0 
    DWORD bfOffBits; //species the offset in bytes from the bitmapfileheader to the bitmap bits 
} tagBITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER 
{
    DWORD biSize; //specifies the number of bytes required by the struct
    LONG biWidth; //specifies width in pixels 
    LONG biHeight; //species height in pixels 
    WORD biPlanes; //specifies the number of color planes, must be 1 
    WORD biBitCount; //specifies the number of bit per pixel 
    DWORD biCompression;//spcifies the type of compression 
    DWORD biSizeImage; //size of image in bytes 
    LONG biXPelsPerMeter; //number of pixels per meter in x axis 
    LONG biYPelsPerMeter; //number of pixels per meter in y axis
    DWORD biClrUsed; //number of colors used by th ebitmap 
    DWORD biClrImportant; //number of colors that are important 
} tagBITMAPINFOHEADER;

typedef struct col
{
    int r, g, b;
} col;

typedef struct pixel
{
    byte pred;
    byte pgre;
    byte pblu;
}pixel;

typedef struct chunk
{
    byte color_index;
    short count;
} chunk;

int main()
{

    clock_t startt;
    startt = clock();
    int dowefork = 1;

    
    FILE *inf = fopen("compressed.bin", "rb");
    if(inf == NULL)
    {
        fprintf(stderr, "Input File Invalid\n");
        return 0;
    }
    
    int w, h;
    int rowbyte_quarter[4];
    int palettecolors;
   

    fread(&w, sizeof(w), 1, inf);
    fread(&h, sizeof(h), 1, inf);
    fread(&rowbyte_quarter, sizeof(rowbyte_quarter), 1, inf);
    fread(&palettecolors, sizeof(palettecolors), 1, inf);
  
    
    col colors[palettecolors];

    fread(&colors, sizeof(colors), 1, inf);
 

    struct tagBITMAPFILEHEADER fh;
    struct tagBITMAPINFOHEADER ih;

    fh.bfType = 19778;
    fh.bfSize = 4320054;
    fh.bfReserved1 = 0;
    fh.bfReserved2 = 0;
    fh.bfOffBits = 54;

    ih.biSize = 40;
    ih.biWidth = 1200;
    ih.biHeight = 1200;
    ih.biPlanes = 1;
    ih.biBitCount = 24;
    ih.biCompression = 0;
    ih.biSizeImage = 4320000;
    ih.biXPelsPerMeter = 3780;
    ih.biYPelsPerMeter = 3780;
    ih.biClrUsed = 0;
    ih.biClrImportant = 0;

    int datac = 0;
    

    
    
    byte *data = (byte *) mmap(NULL, ih.biSizeImage, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    chunk *chunks = (chunk *) mmap(NULL, ih.biSizeImage, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    
    int filec = 0;
    int chunkc = 0;
    int chunksizer = 0;

    while(filec < rowbyte_quarter[3])
    {
        chunk chun;
        fread(&(chun.color_index), sizeof(chun.color_index), 1, inf);
        fread(&(chun.count), sizeof(chun.count), 1, inf);
        chunks[chunkc].color_index = (byte) chun.color_index;
        chunks[chunkc].count = (short) chun.count;   

        chunkc += 1;
        chunksizer = sizeof(chun.color_index) + sizeof(chun.count);
        filec += chunksizer;
    }
    fclose(inf);

    int chunkmax = chunkc;
    
    while(datac<(ih.biSizeImage) && dowefork == 0)
    {
        chunkc = 0;
        datac = 0;
        while(chunkc < chunkmax)
        {
           
            chunk *chun = &(chunks[chunkc]);
            byte indx = (byte) chun->color_index;
            col cool = (col) colors[indx];

            short count = (short) chun->count;

            for(int i = 0; i < count; i++)
            {    
                data[datac] = (byte) cool.b;
                data[datac+1] = (byte) cool.g;
                data[datac+2] = (byte) cool.r;
                datac += 3;
            }
            chunkc += 1;
        }
    }

    
    if(dowefork == 1)
    {
        if(fork() == 0)
        {
            if(fork() == 0)
            {
                chunkc = 0;
                datac = 0;
                chunkmax = rowbyte_quarter[0]/(chunksizer);
                while(chunkc < chunkmax)
                {
           
                    chunk *chun = &(chunks[chunkc]);
                    byte indx = (byte) chun->color_index;
                    col cool = (col) colors[indx];

                    short count = (short) chun->count;

                    for(int i = 0; i < count; i++)
                    {    
                        data[datac] = (byte) cool.b;
                        data[datac+1] = (byte) cool.g;
                        data[datac+2] = (byte) cool.r;
                        datac += 3;
                    }
                    chunkc += 1;
                }
                return 0;
            }
            else
            {
                chunkc = rowbyte_quarter[0]/(chunksizer);
                chunkmax = rowbyte_quarter[1]/(chunksizer);
                datac = (4320000/4);
                while(chunkc < chunkmax)
                {
           
                    chunk *chun = &(chunks[chunkc]);
                    byte indx = (byte) chun->color_index;
                    col cool = (col) colors[indx];

                    short count = (short) chun->count;

                    for(int i = 0; i < count; i++)
                    {    
                        data[datac] = (byte) cool.b;
                        data[datac+1] = (byte) cool.g;
                        data[datac+2] = (byte) cool.r;
                        datac += 3;
                    }
                    chunkc += 1;
                }
            }
            wait();
            return 0;
        }
        else
        {
            if(fork()==0)
            {
                chunkc = rowbyte_quarter[1]/(chunksizer);
                chunkmax = rowbyte_quarter[2]/(chunksizer);
                datac = 2*(4320000/4);
                while(chunkc < chunkmax)
                {
           
                    chunk *chun = &(chunks[chunkc]);
                    byte indx = (byte) chun->color_index;
                    col cool = (col) colors[indx];

                    short count = (short) chun->count;

                    for(int i = 0; i < count; i++)
                    {    
                        data[datac] = (byte) cool.b;
                        data[datac+1] = (byte) cool.g;
                        data[datac+2] = (byte) cool.r;
                        datac += 3;
                    }
                    chunkc += 1;
                }
                return 0;
            }
            else
            {
                chunkc = rowbyte_quarter[2]/(chunksizer);
                chunkmax = rowbyte_quarter[3]/(chunksizer);
                datac = 3*(4320000/4);
                while(chunkc < chunkmax)
                {
           
                    chunk *chun = &(chunks[chunkc]);
                    byte indx = (byte) chun->color_index;
                    col cool = (col) colors[indx];

                    short count = (short) chun->count;

                    for(int i = 0; i < count; i++)
                    {    
                        data[datac] = (byte) cool.b;
                        data[datac+1] = (byte) cool.g;
                        data[datac+2] = (byte) cool.r;
                        datac += 3;
                    }
                    chunkc += 1;
                }
            }
            wait();
        }
        wait();

        

    } 
    

    FILE *outf = fopen("out.bmp", "wb");
    if(outf == NULL)
    {
        fprintf(stderr,"Output File Invalid\n");
        return 0;
    }

    fwrite(&(fh.bfType),sizeof(fh.bfType), 1, outf);
    fwrite(&(fh.bfSize),sizeof(fh.bfSize), 1, outf);
    fwrite(&(fh.bfReserved1),sizeof(fh.bfReserved1), 1, outf);
    fwrite(&(fh.bfReserved2),sizeof(fh.bfReserved2), 1, outf);
    fwrite(&(fh.bfOffBits),sizeof(fh.bfOffBits), 1, outf);

    fwrite(&ih, sizeof(tagBITMAPINFOHEADER), 1, outf);

    for(int i = 0; i < ih.biSizeImage; i+=3)
    {
        fwrite(&(data[i]), sizeof(byte), 1, outf);
        fwrite(&(data[i+1]), sizeof(byte), 1, outf);
        fwrite(&(data[i+2]), sizeof(byte), 1, outf);
    }

    
    fclose(outf);




    // fwrite(&w, sizeof(w), 1, outf);
    // fwrite(&h, sizeof(h), 1, outf);
    // fwrite(&rowbyte_quarter, sizeof(rowbyte_quarter), 1, outf);
    // fwrite(&palettecolors, sizeof(palettecolors), 1, outf);
    // fwrite(&colors, sizeof(colors), 1, outf);

    clock_t endt = clock();
    clock_t elapsed = (endt-startt);///CLOCKS_PER_SEC;

    printf("Time Elapsed: %ld \n\n", elapsed);


    return 0;
}