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
        
        stbi_write_png("test.png", search.x, search.y, channels, search.data, result.x*channels);
        
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

int main(int argc, const char * argv[]) {
    
    init_clock_frequency();
    
    const char *searchNames[256] = {"images//license.png", "images//input.png"};
    const char *templateNames[256] = {"images//template2.png", "images//template.png"};
    
    runTemplateMatch(searchNames, templateNames, 2, 3);
    
    system("PWD");


    return 0;
}
