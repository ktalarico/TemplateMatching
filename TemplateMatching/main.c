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


typedef struct imageContainer {
    int x,y,n;
    unsigned char *data;
} image;

typedef struct solutionLocation {
    int x,y,SAD;
} sol;

int calculateCoord(int x, int y, int imageWidth) {
    //offset = (row * NUMCOLS) + column
    return (y*imageWidth)+x;
}

sol templateMatch(image search, image template) {
    int minSAD = INT_MAX;
    int SAD;
    
    sol solution;
    
    //loop through search image
    for (int sx = 0; sx <= search.x - template.x; sx++) {
        for (int sy = 0; sy <= search.y - template.y; sy++ ) {
            SAD = 0;
            //loop through template starting position
            for (int tx = 0; tx < template.x; tx++) {
                for (int ty = 0; ty < template.y; ty++) {
                    
                    int soffset = calculateCoord(sx+tx, sy+ty, search.x);
                    int toffset = calculateCoord(tx, ty, template.x);
                    
                    unsigned char searchPixel = search.data[soffset];
                    unsigned char templatePixel = template.data[toffset];
                    
                    SAD += abs(searchPixel - templatePixel);
                    
                }
            }
            
            if (minSAD > SAD) {
                minSAD = SAD;
                solution.x = sx;
                solution.y = sy;
                solution.SAD = SAD;
                
            }
            
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
    
    
    image search, template, result;
    search.data = stbi_load("input.png", &search.x, &search.y, &search.n, 1);
    template.data = stbi_load("template.png", &template.x, &template.y, &template.n, 1);
    
    printf("Template Size: %i, %i\n", template.x, template.y);
    printf("Search Size: %i, %i\n", search.x, search.y);

    
    sol solution = templateMatch(search, template);
    printf("Best Match at: x:%u, y:%u, SAD:%i\n", solution.x, solution.y, solution.SAD);
    
    
    result = drawBox(search, template, solution);
    stbi_write_png("output.png", result.x, result.y, 1, result.data, result.x*1);
    
    
    return 0;
}
