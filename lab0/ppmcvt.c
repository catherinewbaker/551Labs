//THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING A TUTOR, CODE WRITTEN BY OTHER STUDENTS, OR CODE DERIVED FROM AN AI TOOL- Catherine Baker
#include "pbm.h"
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>


typedef struct {
    char* mode;
    char* arg;
    char* infileName;
    char* outfileName;
} Options;

// if we need to rename a .ppm
void toPPM(char* infile, char* outfile) {
    PPMImage* imageIn = read_ppmfile(infile);
    if (imageIn == NULL) {
        fprintf(stderr, "Error: No input file specified\n");
        exit(1);
    }

    PPMImage* imageOut = new_ppmimage(imageIn->width, imageIn->height, imageIn->max);
    
    for (unsigned int i = 0; i < imageIn->height; i++) {
        for (unsigned int j = 0; j < imageIn->width; j++) {
            imageOut->pixmap[0][i][j] = imageIn->pixmap[0][i][j];
            imageOut->pixmap[1][i][j] = imageIn->pixmap[1][i][j];
            imageOut->pixmap[2][i][j] = imageIn->pixmap[2][i][j];
        }
    }

    write_ppmfile(imageOut, outfile);
    del_ppmimage(imageIn);
    del_ppmimage(imageOut);
}

// '-b' translate a .ppm file into a .pbm file
void toPBM(char* infile, char* outfile) {
    PPMImage* imageIn = read_ppmfile(infile);
    if (imageIn == NULL) {
        fprintf(stderr, "Error: No input file specified\n");
        exit(1);
    }

    PBMImage* imageOut = new_pbmimage(imageIn->width, imageIn->height);
    double average;
    for (unsigned int i = 0; i < imageIn->height; i++) {
        for (unsigned int j = 0; j < imageIn->width; j++) {
            average = (double) ((imageIn->pixmap[0][i][j] + imageIn->pixmap[1][i][j] + imageIn->pixmap[2][i][j]) / 3.0);
            if (average < (double)(imageIn->max / 2)) {
                imageOut->pixmap[i][j] = 1;
            }
            else {
                imageOut->pixmap[i][j] = 0;
            }
        }
    }

    write_pbmfile(imageOut, outfile);
    del_ppmimage(imageIn);
    del_pbmimage(imageOut);
}

void toPGM(char* infile, char* outfile, char* argu) {
    PPMImage* imageIn = read_ppmfile(infile);
    if (imageIn == NULL) {
        fprintf(stderr, "Error: No input file specified\n");
        exit(1);
    }

    long scalar = strtol(argu, NULL, 10);
    
    PGMImage* imageOut = new_pgmimage(imageIn->width, imageIn->height, scalar);

    double average;
    for (unsigned int i = 0; i < imageIn->height; i++) {
        for (unsigned int j = 0; j < imageIn->width; j++) {
            average = (double) ((imageIn->pixmap[0][i][j] + imageIn->pixmap[1][i][j] + imageIn->pixmap[2][i][j]) / 3.0);
            imageOut->pixmap[i][j] = (unsigned int) ((1.0 * average * scalar) / imageIn->max);
        }
    }
    
    write_pgmfile(imageOut, outfile);
    del_ppmimage(imageIn);
    del_pgmimage(imageOut);
}

void isolate(char* infile, char* outfile, char* argu) {
    PPMImage* imageIn = read_ppmfile(infile);
    if (imageIn == NULL) {
        fprintf(stderr, "Error: No input file specified\n");
        exit(1);
    }
    PPMImage* imageOut = new_ppmimage(imageIn->width, imageIn->height, imageIn->max);

    int colorChoice = -1;
    if (strcmp(argu, "red") == 0) {
        colorChoice = 0;
    }
    else if (strcmp(argu, "green") == 0) {
        colorChoice = 1;
    }
    else if (strcmp(argu, "blue") == 0) {
        colorChoice = 2;
    }
    
    if (colorChoice == -1) {
        fprintf(stderr, "Error: Invalid channel specification: (%s); should be 'red', 'green' or 'blue'\n", argu);
        exit(1);
    }


    for(unsigned int c = 0; c < 3; c++){
        for (unsigned int i = 0; i < imageIn->height; i++) {
            for (unsigned int j = 0; j < imageIn->width; j++) {
                if(c != colorChoice){
                    imageOut->pixmap[c][i][j] = 0;
                } else {
                    imageOut->pixmap[c][i][j] = imageIn->pixmap[c][i][j];
                }
                
            }
        }
    }

    write_ppmfile(imageOut, outfile);
    del_ppmimage(imageIn);
    del_ppmimage(imageOut);
    
}

void removeColor(char* infile, char* outfile, char* argu) {
    PPMImage* imageIn = read_ppmfile(infile);
    if (imageIn == NULL) {
        fprintf(stderr, "Error: No input file specified\n");
        exit(1);
    }

    PPMImage* imageOut = new_ppmimage(imageIn->width, imageIn->height, imageIn->max);

    int colorChoice = -1;
    if (strcmp(argu, "red") == 0) {
        colorChoice = 0;
    }
    else if (strcmp(argu, "green") == 0) {
        colorChoice = 1;
    }
    else if (strcmp(argu, "blue") == 0) {
        colorChoice = 2;
    }
    
    if (colorChoice == -1) {
        fprintf(stderr, "Error: Invalid channel specification: (%s); should be 'red', 'green' or 'blue'\n", argu);
        exit(1);
    }

    for (unsigned int i = 0; i < imageIn->height; i++) {
        for (unsigned int j = 0; j < imageIn->width; j++) {
            imageOut->pixmap[0][i][j] = imageIn->pixmap[0][i][j];
            imageOut->pixmap[1][i][j] = imageIn->pixmap[1][i][j];
            imageOut->pixmap[2][i][j] = imageIn->pixmap[2][i][j];
            imageOut->pixmap[colorChoice][i][j] = 0;
        }
    }

    write_ppmfile(imageOut, outfile);
    del_ppmimage(imageIn);
    del_ppmimage(imageOut);
}

void sepia(char* infile, char* outfile) {
    PPMImage* imageIn = read_ppmfile(infile);
    if (imageIn == NULL) {
        fprintf(stderr, "Error: No input file specified\n");
        exit(1);
    }

    PPMImage* imageOut = new_ppmimage(imageIn->width, imageIn->height, imageIn->max);

    int oldRed = 0;
    int oldGreen = 0;
    int oldBlue = 0;
    for (unsigned int i = 0; i < imageIn->height; i++) {
        for (unsigned int j = 0; j < imageIn->width; j++) {
            oldRed = imageIn->pixmap[0][i][j];
            oldGreen = imageIn->pixmap[1][i][j];
            oldBlue = imageIn->pixmap[2][i][j];
            imageOut->pixmap[0][i][j] = (unsigned int) ((0.393 * oldRed) + (0.769 * oldGreen) + (0.189 * oldBlue));
            imageOut->pixmap[1][i][j] = (unsigned int) ((0.349 * oldRed) + (0.686 * oldGreen) + (0.168 * oldBlue));
            imageOut->pixmap[2][i][j] = (unsigned int) ((0.272 * oldRed) + (0.534 * oldGreen) + (0.131 * oldBlue));
            if(imageOut->pixmap[0][i][j] > imageIn->max){
                imageOut->pixmap[0][i][j] = imageIn->max;
            }
            if(imageOut->pixmap[1][i][j] > imageIn->max){
                imageOut->pixmap[1][i][j] = imageIn->max;
            }
            if(imageOut->pixmap[1][i][j] > imageIn->max){
                imageOut->pixmap[1][i][j] = imageIn->max;
            }
        }
    }

    write_ppmfile(imageOut, outfile);
    del_ppmimage(imageIn);
    del_ppmimage(imageOut);
}

void vertMirror(char* infile, char* outfile) {
    PPMImage* imageIn = read_ppmfile(infile);
    if (imageIn == NULL) {
        fprintf(stderr, "Error: No input file specified\n");
        exit(1);
    }

    PPMImage* imageOut = new_ppmimage(imageIn->width, imageIn->height, imageIn->max);

    for (unsigned int i = 0; i < imageIn->height; i++) {
        for (unsigned int j = 0; j < imageIn->width; j++) {
            imageOut->pixmap[0][i][j] = imageIn->pixmap[0][i][j];
            imageOut->pixmap[1][i][j] = imageIn->pixmap[1][i][j];
            imageOut->pixmap[2][i][j] = imageIn->pixmap[2][i][j];
        }
    }

    int width = imageIn->width;
    int leftCol = 0;
    for (width = width - 1; width > imageIn->width / 2; width--) {
        for (int i = 0; i < imageIn->height; i++) {
            imageOut->pixmap[0][i][width] = imageIn->pixmap[0][i][leftCol];
            imageOut->pixmap[1][i][width] = imageIn->pixmap[1][i][leftCol];
            imageOut->pixmap[2][i][width] = imageIn->pixmap[2][i][leftCol];
        }
        leftCol++;
    }

    write_ppmfile(imageOut, outfile);
    del_ppmimage(imageIn);
    del_ppmimage(imageOut);
}

void reduceToThumbnail(char* infile, char* outfile, char* argu) {
    PPMImage* imageIn = read_ppmfile(infile);
    if (imageIn == NULL) {
        fprintf(stderr, "Error: No input file specified\n");
        exit(1);
    }

    int scalar = (int) strtol(argu, NULL, 10);
    if (scalar < 1 || scalar > 8) {
        fprintf(stderr, "Error: Invalid scale factor: %d; must be 1-8\n", scalar);
        exit(1);
    }

    int newHeight = (imageIn->height / scalar);
    int newWidth = (imageIn->width / scalar);
    PPMImage* imageOut = new_ppmimage(newWidth, newHeight, imageIn->max);

    for (unsigned int i = 0; i < newHeight; i++) {
        for (unsigned int j = 0; j < newWidth; j++) {
            imageOut->pixmap[0][i][j] = imageIn->pixmap[0][i * scalar][j * scalar];
            imageOut->pixmap[1][i][j] = imageIn->pixmap[1][i * scalar][j * scalar];
            imageOut->pixmap[2][i][j] = imageIn->pixmap[2][i * scalar][j * scalar];
        }
    }

    write_ppmfile(imageOut, outfile);
    del_ppmimage(imageIn);
    del_ppmimage(imageOut);
}

void tileThumbnail(char* infile, char* outfile, char* argu) {
    PPMImage* imageIn = read_ppmfile(infile);
    if (imageIn == NULL) {
        fprintf(stderr, "Error: No input file specified\n");
        exit(1);
    }

    int scalar = (int) strtol(argu, NULL, 10);
    if (scalar < 1 || scalar > 8) {
        fprintf(stderr, "Error: Invalid scale factor: %d; must be 1-8\n", scalar);
        exit(1);
    }

    int newHeight = (imageIn->height / scalar);
    int newWidth = (imageIn->width / scalar);
    PPMImage* imageOut = new_ppmimage(newWidth, newHeight, imageIn->max);
    PPMImage* imageLast = new_ppmimage(imageIn->width, imageIn->height, imageIn->max);

    for (unsigned int i = 0; i < newHeight; i++) {
        for (unsigned int j = 0; j < newWidth; j++) {
            imageOut->pixmap[0][i][j] = imageIn->pixmap[0][i * scalar][j * scalar];
            imageOut->pixmap[1][i][j] = imageIn->pixmap[1][i * scalar][j * scalar];
            imageOut->pixmap[2][i][j] = imageIn->pixmap[2][i * scalar][j * scalar];
        }
    }

   // for (int k = 0; k < scalar; k++) {
        for (unsigned int i = 0; i < imageIn->height; i++) {
            for (unsigned int j = 0; j < imageIn->width; j++) {
                imageLast->pixmap[0][i][j] = imageOut->pixmap[0][i % newHeight][j % newWidth];
                imageLast->pixmap[1][i][j] = imageOut->pixmap[1][i % newHeight][j % newWidth];
                imageLast->pixmap[2][i][j] = imageOut->pixmap[2][i % newHeight][j % newWidth];
            }
        }
    //}

    write_ppmfile(imageLast, outfile);
    del_ppmimage(imageIn);
    del_ppmimage(imageOut);
    del_ppmimage(imageLast);
}

int main(int argc, char* argv[]) {

    Options* commandLine = (Options*)malloc(sizeof(Options));
    commandLine->mode = (char*)malloc(2 * sizeof(char));
    commandLine->mode[0] = '\0';
    commandLine->mode[1] = '\0';

    char optTrack;
    int length = 0;
    while ((optTrack = getopt(argc, argv, ":bg:i:r:smt:n:o:")) != -1) {
        if(optarg){
            while (optarg[length] != '\0') {
                length++;
            }
        }

        length += 1;
        switch (optTrack) {
        case 'b':
            if((commandLine->mode[0] != '\0') && strcmp(&optTrack, "o") != 0){
                fprintf(stderr, "Error: Multiple transformations specified\n");
                exit(1);
            }
            commandLine->mode[0] = optTrack;
            printf("cline mode: %s", commandLine->mode);
            printf("\nopttr: %c\n", optTrack);
            break;
        case 'g':
            if((commandLine->mode[0] != '\0') && strcmp(&optTrack, "o") != 0){
                fprintf(stderr, "Error: Multiple transformations specified\n");
                exit(1);
            }
            commandLine->mode[0] = optTrack;
            if (optarg) {
                commandLine->arg = (char *)malloc(length * sizeof(char));
                for(int q = 0; optarg[q] != '\0'; q++){
                    commandLine->arg[q] = optarg[q];
                }
            }
            
            if(strtol(commandLine->arg, NULL, 10) >= 65536 || strtol(commandLine->arg, NULL, 10) < 1){
                fprintf(stderr, "Error: Invalid max grayscale pixel value: %s; must be less than 65,536\n", commandLine->arg);
                exit(1);
            }
            break;
        case 'i':
            if((commandLine->mode[0] != '\0') && strcmp(&optTrack, "o") != 0){
                fprintf(stderr, "Error: Multiple transformations specified\n");
                exit(1);
            }
            commandLine->mode[0] = optTrack;
            if (optarg) {
                commandLine->arg = (char *)malloc(length * sizeof(char));
                for(int e = 0; optarg[e] != '\0'; e++){
                    commandLine->arg[e] = optarg[e];
                }
            }
            if(strcmp(commandLine->arg, "red") != 0 && strcmp(commandLine->arg, "green") != 0 && strcmp(commandLine->arg, "blue") != 0 ){
                fprintf(stderr, "Error: Invalid channel specification: (%s); should be 'red', 'green' or 'blue'\n", commandLine->arg);
                exit(1);
            }
            break;
        case 'r':
            if((commandLine->mode[0] != '\0') && strcmp(&optTrack, "o") != 0){
                fprintf(stderr, "Error: Multiple transformations specified\n");
                exit(1);
            }
            commandLine->mode[0] = optTrack;
            if (optarg) {
                commandLine->arg = (char *)malloc(length * sizeof(char));
                for(int r = 0; optarg[r] != '\0'; r++){
                    commandLine->arg[r] = optarg[r];
                }
            }
            if(strcmp(commandLine->arg, "red") != 0 && strcmp(commandLine->arg, "green") != 0 && strcmp(commandLine->arg, "blue") != 0 ){
                fprintf(stderr, "Error: Invalid channel specification: (%s); should be 'red', 'green' or 'blue'\n", commandLine->arg);
                exit(1);
            }
            break;
        case 's':
            if((commandLine->mode[0] != '\0') && strcmp(&optTrack, "o") != 0){
                fprintf(stderr, "Error: Multiple transformations specified\n");
                exit(1);
            }
            commandLine->mode[0] = optTrack;
            break;
        case 'm':
            if((commandLine->mode[0] != '\0') && strcmp(&optTrack, "o") != 0){
                fprintf(stderr, "Error: Multiple transformations specified\n");
                exit(1);
            }
            commandLine->mode[0] = optTrack;
            break;
        case 't':
            if((commandLine->mode[0] != '\0') && strcmp(&optTrack, "o") != 0){
                fprintf(stderr, "Error: Multiple transformations specified\n");
                exit(1);
            }
            commandLine->mode[0] = optTrack;
            if (optarg) {
                commandLine->arg = (char *)malloc(length * sizeof(char));
                for(int t = 0; optarg[t] != '\0'; t++){
                    commandLine->arg[t] = optarg[t];
                }
            }
            if(strtol(commandLine->arg, NULL, 10) > 8 || strtol(commandLine->arg, NULL, 10) < 1){
                fprintf(stderr, "Error: Invalid scale factor: %d; must be 1-8\n", (int) strtol(commandLine->arg, NULL, 10));
                exit(1);
            }
            break;
        case 'n':
            if((commandLine->mode[0] != '\0') && strcmp(&optTrack, "o") != 0){
                fprintf(stderr, "Error: Multiple transformations specified\n");
                exit(1);
            }
            commandLine->mode[0] = optTrack;
            if (optarg) {
                commandLine->arg = (char *)malloc(length * sizeof(char));
                for(int n = 0; optarg[n] != '\0'; n++){
                    commandLine->arg[n] = optarg[n];
                }
            }
            if(strtol(commandLine->arg, NULL, 10) >= 8 || strtol(commandLine->arg, NULL, 10) < 1){
                fprintf(stderr, "Error: Invalid scale factor: %d; must be 1-8\n", (int) strtol(commandLine->arg, NULL, 10));
                exit(1);
            }
            break;
        case 'o':
            if (optarg) {
                commandLine->outfileName = (char *)malloc(length * sizeof(char));
                for(int z = 0; optarg[z] != '\0'; z++){
                    commandLine->outfileName[z] = optarg[z];
                }
            }
            // default b
            if(commandLine->mode[0] == '\0'){
                commandLine->mode[0] = 'b';
            }
            break;
        case ':':
            fprintf(stderr, "Usage: ppmcvt [-bgirsmtno] [FILE]\n");
            exit(1);
            break;
        case '?':
            fprintf(stderr, "Usage: ppmcvt [-bgirsmtno] [FILE]\n");
            exit(1);
            break;
        default:
            fprintf(stderr, "Usage: ppmcvt [-bgirsmtno] [FILE]\n");
            exit(1);
            break;
        }
    }
    
    

    if (optind < argc) {
        commandLine->infileName = (char*)malloc(sizeof(argv[optind]));
        for(int q = 0; argv[optind][q] != '\0'; q++){
            commandLine->infileName[q] = argv[optind][q];
        }
    }
    else {
        fprintf(stderr, "Error: No input file specified\n");
        exit(1);
    }

    if(strcmp(commandLine->infileName, argv[argc - 1]) != 0){
        fprintf(stderr, "Usage: ppmcvt [-bgirsmtno] [FILE]\n");
        exit(1);
    }

    if(commandLine->outfileName == NULL){
        fprintf(stderr, "Error: No output file specified\n");
        exit(1);
    }
    

    // call transformation on image
    if (commandLine->mode[0] == 'b') {
        toPBM(commandLine->infileName, commandLine->outfileName);
    }
    else if (commandLine->mode[0] == 'g')  {
        toPGM(commandLine->infileName, commandLine->outfileName, commandLine->arg);
    }
    else if (commandLine->mode[0] == 'i') {
        isolate(commandLine->infileName, commandLine->outfileName, commandLine->arg);
    }
    else if (commandLine->mode[0] == 'r') {
        removeColor(commandLine->infileName, commandLine->outfileName, commandLine->arg);
    }
    else if (commandLine->mode[0] == 's') {
        sepia(commandLine->infileName, commandLine->outfileName);
    }
    else if (commandLine->mode[0] == 'm') {
        vertMirror(commandLine->infileName, commandLine->outfileName);
    }
    else if (commandLine->mode[0] == 't') {
        reduceToThumbnail(commandLine->infileName, commandLine->outfileName, commandLine->arg);
    }
    else if (commandLine->mode[0] == 'n') {
        tileThumbnail(commandLine->infileName, commandLine->outfileName, commandLine->arg);
    }

    //free memory
    if (commandLine->mode) {
        free(commandLine->mode);
    }
    if (commandLine->arg) {
        free(commandLine->arg);
    }
    if (commandLine->outfileName) {
        free(commandLine->outfileName);
    }
    if (commandLine->infileName) {
        free(commandLine->infileName);
    }
    if(commandLine){
        free(commandLine);
    }
    

    exit(0);

}
