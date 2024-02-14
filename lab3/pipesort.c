//THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING A TUTOR, CODE WRITTEN
//BY OTHER STUDENTS, OR CODE DERIVED FROM AN AI TOOL- Catherine W Baker
#include <getopt.h>
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>

// method to handle counting words and sending them to stdout
int counting(int nFlag, int lFlag, int sFlag, FILE **streams) {
    // initial variables
    int wordCount = 0; // counts of prevWord
    char currWord[nFlag][lFlag + 1]; // array of words sent by stream, +1 for null terminator
    char prevWord[lFlag + 1]; // current word we're counting for
    int flag = 0; // flag tells us when to quit the while loop
    int flags[nFlag]; // flags[nFlag] tracks if our streams go NULL

    // initilize flags as safe
    for (int h = 0; h < nFlag; h++) {
        flags[h] = 0;
    }
    // pull a word from each stream into our array of words
    for (int w = 0; w < nFlag; w++) {
        fgets(currWord[w], lFlag + 1, streams[w]);
    }
    
    // count our words and print them
    while(flag != 1) {
        flag = 1; // initialize temp flag as unsafe
        // for all the streams
        for (int d = 0; d < nFlag; d++) {
            // if its a good stream then
            if (flags[d] == 0){
                // compare our prev and curr words
                while ((strcmp(currWord[d], prevWord) == 0)) { // if they're equal then
                    wordCount += 1; // increase word count

                    // pull the the next word
                    if(fgets(currWord[d], lFlag+1, streams[d]) != NULL) { // check for null -> means finished/bad stream
                        // if it was just a new line char then skip to the next word
                        if(strcmp(currWord[d], "\n") == 0){
                            // note stream won't be null if it's just a new line char - a word must come after
                            fgets(currWord[d], lFlag+1, streams[d]);
                        }
                    } else { // otherwise if our stream was null
                        flags[d] = 1; // mark this stream as bad/done
                        break; // and move out to the next stream
                    }
                    // otherwise then null terminate the word
                    if (currWord[d][strlen(currWord[d]) - 1] == '\n') {
                        currWord[d][strlen(currWord[d]) - 1] = '\0';
                    }
                }
            }
        }
        // after we've gotten our word count, print the word with it's count
        if (strlen(prevWord) > 0 && prevWord[0] != '\n'){
            if(strlen(prevWord) > sFlag){
                fprintf(stdout, "%-10i", wordCount); // digit left aligned in a 10 character space
                fprintf(stdout, "%s\n", prevWord); // word
            }
        }

        wordCount = 0; // reset wordCount
        prevWord[0] = '~'; // reset prevWord
        for (int a = 0; a < nFlag; a++) {
            // if this stream is not done
            if (flags[a] == 0){
                // and we see a word in this stream that is alphabetically before our prevWord
                if (strcmp(currWord[a], prevWord) < 0){ 
                    strcpy(prevWord, currWord[a]); // then set prevWord to what currWord[] was
                }
            }
        }

        // now make sure all our streams are done
        for (int n = 0; n < nFlag; n++) {
            // if any are still open
            if (flags[n] == 0){
                flag = 0; // then note we should continue
            }
        }
    }
    return 0;
}

// method to handle stdin words and send them to input
int parsing(int nFlag, int lFlag, FILE **streams) {
    int pipeIndex = 0; // this tracks pipe index for round-robin distribution
    int index = 0; // this will keep track of the length of the current word
    int c; // this will hold the current character
    // Read input char by char
    while ((c = fgetc(stdin)) != EOF) { // read a single character
        // Check if the character is an alphabet character
        if (isalpha(c)) {
            // Make sure it's lowercase
            c = tolower(c);
            if (index < lFlag) { // if it's not too big...
                // send it to the stream
                fputc(c, streams[pipeIndex]);
                index++; // and increase the current index of our word to keep track of length
            }
        } else { // if it's not an alphabet char then
            fputc('\n', streams[pipeIndex]); // send the newline to mark the end of each word and help our counter
            pipeIndex = (pipeIndex + 1) % nFlag; // Rotate the stream index round-robin style
            index = 0; // Reset the word index for the next word
        }
    }
    return 0;
}

int main(int argc, char *argv[]) { 
    // Pull command line arguments with getopt
    char *hold;
    int opt;
    int nFlag = -1, sFlag = -1, lFlag = -1;
    while ((opt = getopt(argc, argv, "n:s:l:")) != -1) {
        switch (opt) {
        case 'n':
            nFlag = strtol(optarg, &hold, 10);
            break;
        case 's':
            sFlag = strtol(optarg, &hold, 10);
            break;
        case 'l':
            lFlag = strtol(optarg, &hold, 10);
            break;
        default: // '?'
            fprintf(stderr, "Usage: pipesort [n:s:l:]\n");
            exit(1);
        }
    }
    // check we have apropriate arguments to work with
    if (nFlag < 0 || sFlag < 0 || lFlag < 0) {
        fprintf(stderr, "Usage: [n:s:l:] option must be greater than 0\n");
        exit(1);
    }

    // initialize our 2 main pipes
    int input[nFlag][2]; // each pipe has 2 rows and we have as many pipes as sorters
    int output[nFlag][2];
    for (int i = 0; i < nFlag; i++) {
        // initialize pipes with pipe()
        if (pipe(input[i]) == -1 || pipe(output[i]) == -1) {
            perror("Pipe between parser and sorter");
            exit(1);
        }
    }

    pid_t count; // fork to count words output and send to stdout
    pid_t parse; // fork to parse words from stin and send to input
    // first fork for parsing
    // code in main() will handle all file opening and closing
    // code in methods will handle child processes (for sort this is the exec call)
    if ((parse = fork()) == -1) { // check for error return
        perror("fork parse");
        exit(1);
    }
    // continue to parse if we have a child pid
    if (parse == 0) {
        // establish organization for [nFlag] filestreams
        FILE * streams[nFlag];
        for (int f = 0; f < nFlag; f++) {
            close(input[f][0]); // close read before opening write
            // parsing file streams write to input[][] pipe so open it
            if ((streams[f] = fdopen(input[f][1], "w")) == NULL) { // error check
                perror("fdopen in to parse");
                exit(1);
            }
            // now close output[][] since we don't need it for parsing
            close(output[f][0]);
            close(output[f][1]);
        }
        // do parsing
        parsing(nFlag, lFlag, streams);
        // close streams
        for (int t = 0; t < nFlag; t++) {
            fclose(streams[t]);
        }
        // all children should exit upon completion
        exit(0);
    // else if parsePID != -1 or 0 then fork count process
    } else if ((count = fork()) == -1) { // check for error
        perror("fork count");
        exit(1);
    } else if(count == 0){
        // setup file streams
        FILE * streams[nFlag]; // stream organizer
        for (int o = 0; o < nFlag; o++) {
            close(output[o][1]); // close write end of ouput before opening read end
            // open read end out output in a file stream
            if ((streams[o] = fdopen(output[o][0], "r")) == NULL) { // check for bad stream
                perror("fdopen count to out");
                exit(1);
            }
            // now close input[][] since we're not using it
            close(input[o][0]);
            close(input[o][1]);
        }
        // and call our count method
        counting(nFlag, lFlag, sFlag, streams);
        // finish by closing up our streams
        for (int j = 0; j < nFlag; j++) {
            fclose(streams[j]);
        }
        // exit 0 for success
        exit(0);
    }
    // now handle sort calls based on nFlag
    // earlier we forked once for parse and count but since sort is a bunch of 
    // subprocesses based on nFlag, we want to fork() as many times as nFlag says
    pid_t sort;
    for (int g = 0; g < nFlag; g++) {
        // fork
        if ((sort=fork()) == -1) {  // and check for error
            perror("Fork sorter process");
            exit(1);
        }
        // if we are ready for the child
        if (sort == 0) {
            // taking words from input[][], sorting them, and writing them to ouput[][]
            for (int j = 0; j < nFlag; j++) {
                // close input write end and output read end since we aren't using those
                close(input[j][1]);
                close(output[j][0]);
            }
            // dup stdin to input and stdout to output so sort exec call uses our pipes
            dup2(input[g][0], 0);
            dup2(output[g][1], 1);
            // and perform our final pipe closures
            for (int x = 0; x < nFlag; x++) {
                // close input write end and output read end since we aren't using those
                close(input[x][0]);
                close(output[x][1]);
            }
            // call sort with no arguments
            execl("/usr/bin/sort", "sort", (char*) NULL);
            // if we ever return to this code then our sorting function had an error
            perror("exec sort:");
            exit(1); // so we return with error
        }
    }
    // lastly we need to clean up
    // close all ends of input and output after opening them with pipe()
    for (int r = 0; r < nFlag; r++) {
        for(int e = 0; e < 2; e++){
            close(input[r][e]);
            close(output[r][e]);
        }
    }
    // and perform all waits necessary
    int status;
    pid_t pid;
    // wait for 1 child parser + nFlag child sort processes + 1 child counter process = nFlag + 2 wait calls
    for (int s = 0; s < nFlag + 2; s++) {
        if ((pid = wait(&status)) == -1) { // check for bad wait calls
            perror("Main wait");
            fprintf(stderr, "s: %i", s);
            exit(1);
        } else if (status != 0) { // check for non-good wait calls
            fprintf(stderr, "Error: child %i with index %i exited with status %i", pid, s, status); // and print helpful info in this case
            exit(1);
        }
    }
    return 0;
}