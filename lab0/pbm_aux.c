//THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING A TUTOR, CODE WRITTEN BY OTHER STUDENTS, OR CODE DERIVED FROM AN AI TOOL- Catherine Baker
#include "pbm.h"
#include <stdlib.h>

PPMImage * new_ppmimage( unsigned int w, unsigned int h, unsigned int m )
{
    //Define PBMImage pointer and allocate storage for it
    PPMImage *image = (PPMImage *)malloc(sizeof(PPMImage));
    // Initialize PBMImage struct height and width
    image->width = w;
    image->height = h;
    image->max = m;

    // Initialize and allocate the memory for remaining struct variables (pixmap)
    for(unsigned int i = 0; i < 3; i++) {
        image->pixmap[i] = (unsigned int **)malloc(h * sizeof(unsigned int *));

        for(unsigned int j = 0; j < h; j++) {
            image->pixmap[i][j] = (unsigned int *)malloc(w * sizeof(unsigned int));
        }
    }
    
    return image;
}

PBMImage * new_pbmimage( unsigned int w, unsigned int h )
{
    //Define PBMImage pointer and allocate storage for it
    PBMImage *image = (PBMImage *)malloc(sizeof(PBMImage));
    // Initialize PBMImage struct height and width
    image->width = w;
    image->height = h;

    // Initialize and allocate the memory for remaining struct variables (pixmap)
    image->pixmap = (unsigned int **)malloc(h * sizeof(unsigned int *)); // pixmap now points to array of pointers that is h long
    for(unsigned int i = 0; i < h; i++){
        image->pixmap[i] = (unsigned int *)malloc(w * sizeof(unsigned int)); // each point points to array of unsigned ints
    }

    return image;
}

PGMImage * new_pgmimage( unsigned int w, unsigned int h, unsigned int m )
{
    //Define PBMImage pointer and allocate storage for it
    PGMImage *image = (PGMImage *)malloc(sizeof(PGMImage));
    // Initialize PBMImage struct height and width
    image->width = w;
    image->height = h;
    image->max = m;

    // Initialize and allocate the memory for remaining struct variables (pixmap)
    image->pixmap = (unsigned int **)malloc(h * sizeof(unsigned int *)); // pixmap now points to array of pointers that is h long
    for(unsigned int i = 0; i < h; i++){
        image->pixmap[i] = (unsigned int *)malloc(w * sizeof(unsigned int)); // each point points to array of unsigned ints
    }

    return image;
}

void del_ppmimage( PPMImage * p )
{
    // no need to do work if the image already does not exist
    if(p == NULL){
        return;
    }

    // free pixmap allocated memory per row per color
    for(int j = 0; j < 3; j++){
        for(unsigned int i = 0; i < p->height; i++){
            free(p->pixmap[j][i]);
        }
    }

    //free initial color array
    free(p->pixmap[0]);
    free(p->pixmap[1]);
    free(p->pixmap[2]);

    // free the image
    free(p);

}

void del_pgmimage( PGMImage * p )
{
    // no need to do work if the image already does not exist
    if(p == NULL){
        return;
    }

    // free pixmap allocated memory per row
    for(unsigned int i = 0; i < p->height; i++){
        free(p->pixmap[i]);
    }

    //free initial h array
    free(p->pixmap);

    // free the image
    free(p);
}

void del_pbmimage( PBMImage * p )
{
    // no need to do work if the image already does not exist
    if(p == NULL){
        return;
    }

    // free pixmap allocated memory per row
    for(unsigned int i = 0; i < p->height; i++){
        free(p->pixmap[i]);
    }

    //free initial h array
    free(p->pixmap);

    // free the image
    free(p);
}

