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

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image.h"
#include "stb_image_write.h"

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
    
    sol solution;
    
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
    memcpy(&temp.data, &search.data, sizeof *search.data);
    
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

int main(int argc, const char * argv[]) {
    
    int channels = 3;
    const char *searchNames[256] = {"images//license.png", "images//input.png"};
    const char *templateNames[256] = {"images//template2.png", "images//template.png"};
    for (int i = 0; i < 2; i++) {
        image search, template, result;
        search.data = stbi_load(searchNames[i], &search.x, &search.y, &search.n, channels);
        template.data = stbi_load(templateNames[i], &template.x, &template.y, &template.n, channels);
        
        printf("Template Size: %i, %i\n", template.x, template.y);
        printf("Search Size: %i, %i\n", search.x, search.y);

        
        sol solution = templateMatch(search, template);
        
        printf("Best Match at: x:%u, y:%u, SSD:%i\n\n", solution.x, solution.y, solution.colorD.sum);
        
        
        result = drawBox(search, template, solution);
        char outputName[256];
        snprintf(outputName, sizeof outputName, "output%i.png",i);
        stbi_write_png(outputName, result.x, result.y, channels, result.data, result.x*channels);
    
    }
    return 0;
}
