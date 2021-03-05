#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <math.h>
#include <errno.h>  
#include <tiffio.h>
#include <stdint.h>

#define HPIXELS 2048            // number of horizontal pixels in sensor
#define VPIXELS 2048            // number of vertical pixels in sensor

int main(int argc, char **argv) {

    int color;
    if(argc > 1)
        color = atoi(argv[1]);
    else
        color = 128;

    printf("value to be set on memory: %d\n", color);

    static const short CFARepeatPatternDim[] = { 2,2 }; // 2x2 CFA

    static const float cam_xyz[] = { 
        2.005, -0.771, -0.269, 
        -0.752, 1.688, 0.064, 
        -0.149, 0.283, 0.745 }; // xyz

    static const double sRGB[] = {
        3.6156, -0.8100, -0.0837,
        -0.3094, 1.5500, -0.5439,
        0.0967, -0.4700, 1.9805 }; // sRGB profile

    static const float neutral[] = { 0.807133, 1.0, 0.913289 };

    const int samples_per_pixel = 1;
    const char* fname = "/home/pvbecker/workspace/tests/libtifftest/output.dng";

    //white level, obtained from the Android API
    long white = 1023;

    struct stat st;
    struct tm tm;
    char datetime[64];

    FILE *ifp;
    TIFF *tif;

    // ============================================================================
    stat(fname, &st);
    gmtime_r(&st.st_mtime, &tm);
    sprintf(datetime, "%04d:%02d:%02d %02d:%02d:%02d",
        tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

    if (!(tif = TIFFOpen(fname, "w"))) {
        printf("error on opening file, abort");
        fclose(ifp);
        exit(-1);
    }

    TIFFSetField(tif, TIFFTAG_SUBFILETYPE, 1);          // image
    TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, HPIXELS);     // in pixels
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH, VPIXELS);    // in pixels
    TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 16);       // int
    TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, samples_per_pixel);
    TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(tif, TIFFTAG_CFAREPEATPATTERNDIM, CFARepeatPatternDim);
    TIFFSetField(tif, TIFFTAG_CFAPATTERN, "\001\000\001\002"); 
    TIFFSetField(tif, TIFFTAG_WHITELEVEL, 1, &white);
    TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
    TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, 32803);  //DNG CFA value. 
    TIFFSetField(tif, TIFFTAG_MAKE, "MyDNG_Writer");
    TIFFSetField(tif, TIFFTAG_MODEL, "MyDNG_Writer model");
    TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
    TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 1);
    TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(tif, TIFFTAG_SOFTWARE, "MyDNG_Writer");
    TIFFSetField(tif, TIFFTAG_DATETIME, datetime);
//    TIFFSetField(tif, TIFFTAG_SUBIFD, 1, &sub_offset);
    TIFFSetField(tif, TIFFTAG_DNGVERSION, "\001\001\0\0");
    TIFFSetField(tif, TIFFTAG_DNGBACKWARDVERSION, "\001\0\0\0");
    TIFFSetField(tif, TIFFTAG_UNIQUECAMERAMODEL, "random camera sensor");
    TIFFSetField(tif, TIFFTAG_COLORMATRIX1, 9, cam_xyz);
    TIFFSetField(tif, TIFFTAG_ASSHOTNEUTRAL, 3, neutral);
    TIFFSetField(tif, TIFFTAG_CALIBRATIONILLUMINANT1, 21);
    fprintf(stderr, "Processing RAW data...\n");

    // bit depth
    //this is a pointer of unsigned char that will have the whole line of data from the input array
    //One thing to note is that the bitspersample tag will impact the way a given data reader will
    //interpret this data, expecting more or less data on a single line. Since we're setting this
    //particular example to 16 bits per sample, the line buffer will have it's size as HPIXEL * 16
    //and cast as unsigned char type so that the libtiff api accepts it.
    uint16_t *pLine = (uint16_t *)malloc(sizeof(uint16_t) * HPIXELS);
//    uint8_t *pLine = (uint8_t *)malloc(sizeof(uint8_t) * HPIXELS * 2);
    //sets some value in buffer to see if color changes
    memset(pLine, color, sizeof(uint16_t) * HPIXELS);
    for (int row = 0; row < VPIXELS; row++)
    {
        TIFFWriteScanline(tif, pLine, row, 0); // this writes a single complete row
    }
    free(pLine);
    TIFFClose(tif);
    return 0;
}
