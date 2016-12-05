//
//
//  TemplateMatching
//
//  Created by Michael on 11/30/16.
//  Ugly but it does the do.
//
//

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>

#include <mach/mach_time.h>
#include <stdint.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

#define PLATFORM_OSX




typedef struct SumSquareD {
    int SSDr, SSDg, SSDb, sum;
} SSD;

typedef struct imageContainer {
    int x,y,n;
    unsigned char *data;
} image;

typedef struct solutionLocation {
    int x,y;
    SSD colorD;
} sol;

#ifdef PLATFORM_OSX
static uint64_t freq_num   = 0;
static uint64_t freq_denom = 0;

void init_clock_frequency ()
{
    mach_timebase_info_data_t tb;
    
    if (mach_timebase_info (&tb) == KERN_SUCCESS && tb.denom != 0) {
        freq_num   = (uint64_t) tb.numer;
        freq_denom = (uint64_t) tb.denom;
    }
}
#endif


int calculateCoord(int x, int y, int imageWidth) {
    //offset = (row * NUMCOLS) + column
    return (3*((y*imageWidth)+x));
}
int calculateCoordXY(int x, int y, int imageWidth, int imageHeight) {
    //offset = (row * NUMCOLS) + column
    if ((x*y) > (imageWidth*imageHeight)) return -1;
    if (x > imageWidth || x < 0) return -1;
    if (y > imageHeight || y < 0) return -1;
    
    return (3*((y*imageWidth)+x));
}

SSD newSSD(void) {
    SSD new;
    new.SSDb=0;
    new.SSDg=0;
    new.SSDr=0;
    new.sum=0;
    
    return new;
}

sol templateMatch(image search, image template) {
    int lastSSD = INT_MAX;
    SSD colorD;
    
    sol solution = {};
    
    //loop through search image
    for (int sx = 0; sx <= search.x - template.x; sx++) {
        for (int sy = 0; sy <= search.y - template.y; sy++ ) {
            colorD = newSSD();
            //loop through template starting position
            for (int tx = 0; tx < template.x; tx++) {
                for (int ty = 0; ty < template.y; ty++) {
                    
                    int soffset = calculateCoord(sx+tx, sy+ty, search.x);
                    int toffset = calculateCoord(tx, ty, template.x);
                    
                    unsigned char *searchPixel = &search.data[soffset];
                    unsigned char *templatePixel = &template.data[toffset];
                    
                    colorD.SSDb += pow((*searchPixel - *templatePixel), 2);
                    colorD.SSDr += pow((*(searchPixel+1) - *(templatePixel+1)), 2);
                    colorD.SSDg += pow((*(searchPixel+2) - *(templatePixel+2)), 2);

                }

            }
            
            colorD.sum = colorD.SSDb + colorD.SSDg + colorD.SSDr;

            if (lastSSD > colorD.sum) {
                lastSSD = colorD.sum;
                solution.x = sx;
                solution.y = sy;
                solution.colorD = colorD;
                
            }
            
            //if (sx%10==0 && sy == 0) printf("%i %i\n", sx, sy);
        }

    }
    return solution;
}
image drawBox(image search, image template, sol solution) {
    
    image temp = search;
    temp.data = (unsigned char *)malloc(sizeof search.data);
    memcpy(&temp.data, &search.data, sizeof search.data);
    
    for (int x = 0; x < temp.x; x++) {
        for (int y = 0; y < temp.y; y++) {
            int offset = calculateCoord(x, y, temp.x);
            //Draws a box with the middle section cut out
            if ((y+5 > solution.y && y-5 < solution.y+template.y)
                && !(y > solution.y && y < solution.y+template.y)
                && (x+5>solution.x && x-5<solution.x+template.x)) {

                temp.data[offset] = 0;
            }
            
        }
    }
    
    return temp;
}
#ifdef PLATFORM_OSX
uint64_t tickTimeDiff(uint64_t before, uint64_t after) {
    uint64_t value_diff =  after-before;
    value_diff /= 1000000;
    value_diff *= freq_num;
    value_diff /= freq_denom;
    
    return value_diff;
}
#endif

void runTemplateMatch(const char ** searchFiles, const char ** templateFiles, int numFiles, int channels) {
    //int channels = 3;
    //int numFiles = 2;
    
    for (int i = 0; i < numFiles; i++) {
        
        uint64_t tick_before = mach_absolute_time();
        
        image search, template, result;
        search.data = stbi_load(searchFiles[i], &search.x, &search.y, &search.n, channels);
        template.data = stbi_load(templateFiles[i], &template.x, &template.y, &template.n, channels);
        
        
        printf("Template Size: %i, %i\n", template.x, template.y);
        printf("Search Size: %i, %i\n", search.x, search.y);
        
        
        sol solution = templateMatch(search, template);
        
        printf("Best Match at: x:%u, y:%u, SSD:%i\n\n", solution.x, solution.y, solution.colorD.sum);
        
        
        result = drawBox(search, template, solution);
        char outputName[256];
        snprintf(outputName, sizeof outputName, "output%i.png",i);
        stbi_write_png(outputName, result.x, result.y, channels, result.data, result.x*channels);
        
        uint64_t tick_after = mach_absolute_time();
        
        printf("Took %llu ms\n", tickTimeDiff(tick_before, tick_after));
        
    }
    
}

void copyImage(image * src, image * dest) {
    dest->x = src->x;
    dest->y = src->y;
    dest->n = src->n;
    dest->data = malloc(sizeof (UINT8_MAX) * src->x * src->y* src->n);
    memcpy(dest->data, src->data, sizeof src);
}

void writeImage(image img, const char *filename) {
    stbi_write_png(filename, img.x, img.y, 3, img.data, 0);
}

void rotateImage(image * src, image * dest, double angle) {
    double radians = angle * M_PI/180;
    double cosR = cos(radians);
    double sinR = sin(radians);
    
    copyImage(src, dest);
    
    int centerx = src->x/2;
    int centery= src->y/2;
    
    for (int x = 0; x < src->x; x++) {
        int m = x - centerx;
        for (int y = 0; y < src->y; y++) {
            int destIndex = calculateCoordXY(x, y, src->x, src->y);
            int n = y - centery;
            int newx = (int) (m * cosR + n * sinR) + centerx;
            int newy = (int) (n * cosR - m * sinR) + centery;
            int sourceIndex = calculateCoordXY(newx, newy, src->x, src->y);
            
            if (sourceIndex == -1) {
                dest->data[destIndex] = 0;
                dest->data[destIndex+1] = 0;
                dest->data[destIndex+2] = 0;
                
            } else {
                dest->data[destIndex] = src->data[sourceIndex];
                dest->data[destIndex+1] = src->data[sourceIndex+1];
                dest->data[destIndex+2] = src->data[sourceIndex+2];

            }
        }
    }
    
}

void resizeImage(image * img) {
    
    copyImage(img, testImage);

    
}

int main(int argc, const char * argv[]) {
    
    init_clock_frequency();
    
    image test, rotate;
    test.data = stbi_load("images//license.png", &test.x, &test.y, &test.n, 3);

    rotateImage(&test, &rotate, 25);
    writeImage(rotate, "test.png");
    
    const char *searchNames[256] = {"images//license.png", "images//input.png"};
    const char *templateNames[256] = {"images//template2.png", "images//template.png"};
    
    //runTemplateMatch(searchNames, templateNames, 2, 3);
    


    return 0;
}
