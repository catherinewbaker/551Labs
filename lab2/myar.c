//THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING A TUTOR, CODE WRITTEN //BY OTHER STUDENTS, OR CODE DERIVED FROM AN AI TOOL- Catherine W Baker

#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <unistd.h>
#include <utime.h>

#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <time.h>

#include "ar.h"

// meta struct outline
struct meta {
    char name[16]; //room for null
    int mode;
    int size;
    time_t mtime; // a time_t is a long
} meta;

int fill_ar_hdr(char *filename, struct ar_hdr *hdr){
    struct stat fileStat;
    // Get the file metadata
    if (stat(filename, &fileStat) == -1) {
        perror("Failed to fetch file metadata\n");
        return 1;
    }

    // Clear the header to fill with spaces
    memset(hdr, ' ', sizeof(struct ar_hdr));

    // manually assign filename
    for(int i = 0; i < 16; i++){
        if(strlen(filename) - 1 >= i){
            hdr->ar_name[i] = filename[i];
        } else if (strlen(filename) == i){
            hdr->ar_name[i] = '/';
        } else {
            hdr->ar_name[i] = ' ';
        }
        
    }

    // Convert and fill the ar_hdr struct
    snprintf(hdr->ar_date, sizeof(hdr->ar_date), "%-11ld", (long) fileStat.st_mtime); // Date
    snprintf(hdr->ar_uid, sizeof(hdr->ar_uid), "%-5d", (int) fileStat.st_uid); // UID
    snprintf(hdr->ar_gid, sizeof(hdr->ar_gid), "%-5d", (int) fileStat.st_gid); // GID
    snprintf(hdr->ar_mode, sizeof(hdr->ar_mode), "%-7o", (int) (fileStat.st_mode)); // Mode
    snprintf(hdr->ar_size, sizeof(hdr->ar_size), "%-9d", (int) fileStat.st_size); // Size
    strncpy(hdr->ar_fmag, ARFMAG, sizeof(hdr->ar_fmag)); // ARFMAG

    // for loops removing all null charcters and replacing them with spaces
    for(int j = 0; j < 12; j++){
        if(hdr->ar_date[j] == '\0'){
            hdr->ar_date[j] = ' ';
        }
    }
    for(int h = 0; h < 6; h++){
        if(hdr->ar_uid[h] == '\0'){
            hdr->ar_uid[h] = ' ';
        } 
        if(hdr->ar_gid[h] == '\0'){
            hdr->ar_gid[h] = ' ';
        } 
    }
    for(int g = 0; g < 8; g++){
        if(hdr->ar_mode[g] == '\0'){
            hdr->ar_mode[g] = ' ';
        } 
    }
    for(int f = 0; f < 10; f++){
        if(hdr->ar_size[f] == '\0'){
            hdr->ar_size[f] = ' ';
        } 
    }

    return 0;  // Success
}


int fill_meta( struct ar_hdr hdr, struct meta *meta){
    // Assign file name
    strncpy(meta->name, hdr.ar_name, sizeof(meta->name));
    // Ensure name string is null-terminated
    if (meta->name[sizeof(meta->name) - 1] != '\0') {
        meta->name[sizeof(meta->name) - 1] = '\0';
    }

    // rest of meta features
    meta->mtime = strtol(hdr.ar_date, NULL, 10); // Modification time
    meta->mode = strtol(hdr.ar_mode, NULL, 8); // File mode (permissions)
    meta->size = (int) strtol(hdr.ar_size, NULL, 10); // File size
    // meta has no uid or gid so ignore those

    return 0;  // Success
}

char* formatMode(int mode) {
    // setup the char displaying our permissions
    static char modeStr[10];
    strcpy(modeStr, "---------");
    if (mode & S_hy // if the file is executable by the owner
    if (mode & S_IRGRP) modeStr[3] = 'r'; // if the file is readable by the group
    if (mode & S_IWGRP) modeStr[4] = 'w'; // if the file is writable by the group
    if (mode & S_IXGRP) modeStr[5] = 'x'; // if the file is executable by the group
    if (mode & S_IROTH) modeStr[6] = 'r'; // if the file is readable by  others
    if (mode & S_IWOTH) modeStr[7] = 'w'; // if the file is writable by others
    if (mode & S_IXOTH) modeStr[8] = 'x'; // if the file is executable by others
    return modeStr;
}

int addQ(char* archiveFile, char* allFiles[], int num){
    int flag; // flag for error checking
    // open the file in append mode (or create if necessary) with write-only permissions
    int archive = open(archiveFile, O_WRONLY | O_CREAT | O_APPEND, 0666);
    // if there was an issue opening it then print an error and return
    if (archive == -1) {
        perror("add\n");
        return 1;
    }
    
    // Check if the archive is newly created (empty), if so, write ARMAG string
    if (lseek(archive, 0, SEEK_END) == 0) {
        flag = write(archive, ARMAG, SARMAG);
        if(flag != SARMAG) {
            perror("Error writing ARMAG to archive\n");
            close(archive);
            return 1;
        }
    }

    // cycle through the files to add
    for (int i = 1; i < num; i++) {
        struct ar_hdr head;
        if (fill_ar_hdr(allFiles[i], &head) != 0) {
            continue;  // Error handling: If there's an issue with filling header, skip this file
        }
        // Write the file header
        write(archive, &head, sizeof(head));
        
        // Open the file to read its content
        int file = open(allFiles[i], O_RDONLY);
        if (file == -1) {
            perror("No such file or directory\n");
            continue;
        }
        // setup sizing for writing
        struct stat fileInfo;
        if (stat(allFiles[i], &fileInfo) != 0) {
            // Handle error, for example:
            perror("stat");
            return 1;
        }
        char* buffer = malloc(fileInfo.st_size);
        int bytesRead;
        // while there's more to read in our current file
        while ((bytesRead = read(file, buffer, sizeof(buffer))) > 0) {
            if (bytesRead == -1) {
                perror("Failed to read from archive\n");
                close(archive);
                return 1;
            }
            // write it to our output Archive
            flag = write(archive, buffer, bytesRead);
            if(flag == -1){
                perror("Failed to write file to archive\n");
                return 1;
            }
        }
        // free memory and close
        free(buffer);
        close(file);
        
        // If the file size is odd, append a newline for byte alignment
        if (atoi(head.ar_size) % 2) {
            write(archive, "\n", 1);
        }
    }
    close(archive);
    return 0;
}

int extractXO(char* archiveFile, char* allFiles[], int num, int flag){
    // open the file with read only permissions
    int archive = open(archiveFile, O_RDONLY);
    // return an error if this failed
    if (archive == -1) {
        perror("extract\n");
        return 1;
    }

    // Read the ARMAG and check if it's a valid archive
    char validity[SARMAG];
    read(archive, validity, SARMAG);
    // check if we are at the start of the file appropriately by checking if the first 8 chars match ARMAG
    if (strncmp(validity, ARMAG, SARMAG) != 0) {
        perror("Invalid archive file\n");
        close(archive);
        return 1;
    }

    // now we prepare empty header and metadata for the extracted file
    struct ar_hdr head;
    struct meta data;
    int i = 1;

    // while we're not at the end of the archive file loop through it's file contents
    while (read(archive, &head, sizeof(head)) == sizeof(head)) {
        // convert head to a meta struct so we can access the file data
        fill_meta(head, &data);
        // store the file size from meta data
        int fileSize = data.size;

        int extract = 0;
        // save just the file name
        char metaFileName[16];
        for(int i = 0; i < 16; i++){
            if(data.name[i] == '/'){
                metaFileName[i] = '\0';
                break;
            } else {
               metaFileName[i] = data.name[i];
            }
        }
        // for all files we want to extract
        for (int i = 0; i < num; i++) {
            // if our current file equals any of the files we want to extract
            if(strcmp(allFiles[i], metaFileName) == 0){
                // then make note of it
                extract = 1;
            }
        }

        if(extract) {
            // then extract the file
            int outFile = open(metaFileName, O_WRONLY | O_CREAT | O_TRUNC, 0666);
            // if the open failed then print an error
            if (outFile == -1) {
                perror("extract\n");
                lseek(archive, fileSize, SEEK_CUR);  // Skip the file content in the archive
                continue;
            }

            // now create a buffer sized with stat()
            struct stat fileInfo;
            if (stat(metaFileName, &fileInfo) != 0) {
                // Handle error, for example:
                perror("stat");
                return 1;
            }
            char* buffer = malloc(fileInfo.st_size);
            ssize_t bytes = fileInfo.st_size;
            // while there are bytes to read in file
            while (bytes > 0) {
                // read the next line
                int bytesRead = read(archive, buffer, (bytes > sizeof(buffer)) ? sizeof(buffer) : bytes);
                if (bytesRead == -1) {
                    perror("Failed to read from archive\n");
                    close(archive);
                    return 1;
                }
                // write those bytes to the archive file
                if (write(outFile, buffer, bytesRead) != bytesRead) {
                    perror("Error writing to extracted file\n");
                    close(outFile);
                    close(archive);
                    return 1;
                }
                // subtract the bytes we've already read from our total
                bytes -= bytesRead;
            }
            free(buffer);

            close(outFile);

            if(flag){
                struct utimbuf newTimes;
                newTimes.actime = data.mtime;  // Setting access time
                newTimes.modtime = data.mtime;  // Setting modification time
                flag = utime(metaFileName, &newTimes);    // Update the file's access and modification times
                if (flag != 0) {
                    perror("Error setting file modification time\n");
                }
                flag = chmod(metaFileName, data.mode);  // Restore the file's permissions
                if (flag != 0) {
                    perror("Error setting file modification time\n");
                }
            }
            break;
        } else {
            // Skip the file content in the archive if it's not the one we want
            lseek(archive, fileSize, SEEK_CUR);
        }

        // if the fileSize is odd then skip to the next header
        if(fileSize % 2){
            lseek(archive, 1, SEEK_CUR);
        }
    }
    close(archive);

    return 0; // success
}

int listTV(char* archiveFile, int flag){
    // open the file with read only permissions
    int archive = open(archiveFile, O_RDONLY);
    // return an error if this failed
    if (archive == -1) {
        perror("list\n");
        return 1;
    }

    // Read the ARMAG and check if it's a valid archive
    char validity[SARMAG];
    read(archive, validity, SARMAG);
    // check if we are at the start of the file appropriately by checking if the first 8 chars match ARMAG
    if (strncmp(validity, ARMAG, SARMAG) != 0) {
        fprintf(stderr, "Invalid archive file\n");
        close(archive);
        return 1;
    }

    struct ar_hdr head;
    struct meta data;

    // while we're not at the end of the archive file loop through it's file contents
    while (read(archive, &head, sizeof(head)) == sizeof(head)) {
        // Convert the ar_hdr struct to meta struct
        fill_meta(head, &data);

        // Check if the `-v` option is specified
        if (flag) {
            // Print in verbose mode, showing details (permissions, modification time, etc.)
            // first print mode formatted as ar does
            printf("%s ", formatMode(data.mode));
            // next print the uid
            for(int j = 0; j < 6; j++){
                if(head.ar_uid[j] == ' '){
                    break;
                } else {
                    printf("%c", head.ar_uid[j]);
                }
            }
            // now print a '/'
            printf("%c", '/');
            // and the gid
            for(int h = 0; h < 6; h++){
                printf("%c", head.ar_gid[h]);
            }
            // then the size
            printf("%d ", data.size);

            // now prep for date
            char date_str[80];
            time_t mtime = data.mtime;  // Assuming data.mtime has the st_mtime value
            struct tm *tm_info = localtime(&mtime);
            // now time data is in local strucure, print it in prefered format
            strftime(date_str, sizeof(date_str), "%b %d %H:%M %Y", tm_info);
            printf("%s ", date_str);
            // then print the letters in the name until we reach the '/'
            for(int i = 0; i < 16; i++){
                if(data.name[i] == '/'){
                    break;
                } else {
                    printf("%c", data.name[i]);
                }
            }
            // then print an empty line
            printf("\n");
        } else {
            // Print just the file name
            for(int i = 0; i < 16; i++){
                if(data.name[i] == '/'){
                    break;
                } else {
                    printf("%c", data.name[i]);
                }
            }
            printf("\n");
        }

        int fileSize = data.size;

        // Skip the file content in the archive
        lseek(archive, fileSize, SEEK_CUR);

        // If the file's size is odd in the archive, skip the padding byte
        if (fileSize % 2) {
            lseek(archive, 1, SEEK_CUR);
        }
    }

    close(archive);
    return 0;
}

int deleteD(char* archiveFile, char* allFiles[], int num){
    int flag;
    // Open the original archive file in read-only mode
    int archive = open(archiveFile, O_RDONLY);
    // print an error if that failed
    if(archive == -1){
        perror("Failed to open archive for writing");
        return 1;
    }

    unlink(archiveFile); // can unlink now

    // Create a temporary file for the new archive
    char* tempFile = archiveFile;  // Temporary file name is old archives name
    int tempArchive = open(tempFile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    // return an error if our file creation failed
    if (tempArchive == -1) {
        perror("Failed to create temporary archive");
        close(archive);
        return 1;
    }

    // Copy ARMAG to the temporary file
    char hdr[SARMAG];
    flag = read(archive, hdr, SARMAG);
    if (flag == -1) {
        perror("Error reading extracted file\n");
        close(tempArchive);
        close(archive);
        return 1;
    }
    flag = write(tempArchive, hdr, SARMAG);
    if (flag == -1) {
        perror("Error writing to extracted file\n");
        close(tempArchive);
        close(archive);
        return 1;
    }

    // setup empty meta and head
    struct ar_hdr head;
    struct meta data;

    int shouldDelete = 0;
     // Loop through the original archive content
    while (read(archive, &head, sizeof(head)) == sizeof(head)) {
        // Convert the ar_hdr struct to meta struct
        fill_meta(head, &data);

        // set the name
        char metaFileName[16];
            for(int i = 0; i < 16; i++){
                if(data.name[i] == '/'){
                    metaFileName[i] = '\0';
                    break;
                } else {
                metaFileName[i] = data.name[i];
                }
            }

        // check if the file we are on is one we want to delete
        if(shouldDelete == 0){
            for (int i = 1; i < num; i++) {
                if (strcmp(allFiles[i], metaFileName) == 0) {
                    shouldDelete = 1; // flag -> need to delete this file
                    break;
                }
            }
        }
        
        // store the file size from meta data
        int fileSize = data.size;

        // if yes, skip the contents of this file
        if (shouldDelete == 1) {
            // Skip the file content in the original archive
            lseek(archive, fileSize, SEEK_CUR);
            // If the file's size is odd in the archive, skip the padding byte
            if (fileSize % 2) {
                lseek(archive, 1, SEEK_CUR);
            }
            shouldDelete = 2;
        // otherwise write the file to tempArchive
        } else {
            // Write the ar_hdr and to the temporary file
            flag = write(tempArchive, &head, sizeof(head));
            if (flag == -1) {
                perror("Error writing to extracted file\n");
                close(tempArchive);
                close(archive);
                return 1;
            }
            
            // and write the file's contents to tempArchive as well
            struct stat buffer;
            stat(tempFile, &buffer);
            int bytes = fileSize;
            while (bytes > 0) {
                int bytesRead = read(archive, &buffer, (bytes > sizeof(buffer)) ? sizeof(buffer) : bytes);
                flag = write(tempArchive, &buffer, bytesRead);
                if (flag == -1) {
                    perror("Error writing to extracted file\n");
                    close(tempArchive);
                    close(archive);
                    return 1;
                }
                bytes -= bytesRead;
            }

            // If the file's size was odd, add a padding byte to the temporary file
            if (fileSize % 2) {
                char padding = '\n';  // Padding byte
                flag = write(tempArchive, &padding, 1);
            }
        }
    }
    close(archive);
    close(tempArchive);
   // rename(tempFile, archiveFile);

    return 0;
}

int appendA(char* archiveFile, int val){
    DIR *dir; // our directory
    struct dirent *entry; // file
    struct stat fileStat; // buffer
    time_t currentTime; // time of file's meta data
    int flag;

    // Get the current time
    time(&currentTime);

    // Open the current directory
    dir = opendir(".");
    if (!dir) {
        perror("Failed to open current directory\n");
        return 1;
    }
    
    // Open the archive file in append mode (or create if necessary) with read and write permissions
    int archive = open(archiveFile, O_RDWR | O_APPEND | O_CREAT, 0666);
    if (archive == -1) {
        perror("Failed to open archive for appending\n");
        closedir(dir);
        return 1;
    }

    // Read the ARMAG and check if it's a valid archive
    char validity[SARMAG];
    read(archive, validity, SARMAG);
    // check if we are at the start of the file appropriately by checking if the first 8 chars match ARMAG
    if (strncmp(validity, ARMAG, SARMAG) != 0) {
        perror("Invalid archive file\n");
        close(archive);
        return 1;
    }

    // Iterate over each file in the directory
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {  // Check that it's a regular file
            // Get file metadata
            if (stat(entry->d_name, &fileStat) == 0) {
                double daysOld = difftime(currentTime, fileStat.st_mtime) / (60 * 60 * 24);  // Calculate file age in days
                if (daysOld > val) {
                    // Add the file to the archive like in addQ()
                    struct ar_hdr head;
                    if (fill_ar_hdr(entry->d_name, &head) == 0) {
                        flag = write(archive, &head, sizeof(head));  // Write header to archive
                        if (flag == -1) {
                            perror("Failed to write to archive\n");
                            close(archive);
                            return 1;
                        }

                        // Open the file and append its content to the archive
                        int file = open(entry->d_name, O_RDONLY);
                        if (file != -1) {
                            struct stat fileInfo;
                            if (stat(entry->d_name, &fileInfo) != 0) {
                                // Handle error, for example:
                                perror("stat");
                                return 1;
                            }
                            char* buffer = malloc(fileInfo.st_size);
                            ssize_t bytesRead;
                            while ((bytesRead = read(file, buffer, sizeof(buffer))) > 0) {
                                if (bytesRead == -1) {
                                    perror("Failed to read from archive\n");
                                    close(archive);
                                    return 1;
                                }
                                flag = write(archive, buffer, bytesRead);
                                if (flag == -1) {
                                    perror("Failed to write to archive\n");
                                    close(archive);
                                    return 1;
                                }
                            }
                            free(buffer);
                            close(file);
                        } else {
                            perror("extract\n");
                            close(archive);
                            return 1;
                        }
                    }
                }
            }
        }
    }
    close(archive);
    closedir(dir);

    return 0;
}

int main(int argc, char* argv[]) {
    
    if (argc < 3) {
        fprintf(stderr, "Usage: %s [qxotvdA:] archive-file [file1 ..... ]\n", argv[0]);
        exit(1);
    }

    int numFiles = 0; // how many files we have
    int opt; // track what parameters we flagged
    char arg[10] = ""; // if A has an argument we track it here

    // Flags for each option
    int qFlag = 0, xFlag = 0, oFlag = 0, tFlag = 0, vFlag = 0, dFlag = 0, AFlag = 0;

    // Parse options using getopt
    while ((opt = getopt(argc, argv, "qxotvdA:")) != -1) {
        switch (opt) {
        case 'q':
            qFlag = 1;
            break;
        case 'x':
            xFlag = 1;
            break;
        case 'o':
            oFlag = 1;
            break;
        case 't':
            tFlag = 1;
            break;
        case 'v':
            vFlag = 1;
            break;
        case 'd':
            dFlag = 1;
            break;
        case 'A':
            AFlag = 1;
            if (optarg) {
                strcpy(arg, optarg);
            }
            break;
        default: // '?'
            fprintf(stderr, "Usage: %s [-qxotvdA:] archive-file [file1 ...]\n", argv[0]);
            return 1;
        }
    }

    char* infiles[argc - optind];

    // grab file names
    for (int q = optind; q < argc; q++) {
        infiles[numFiles] = argv[q];
        numFiles += 1;
    }

    // call methods
    if (qFlag) {
        addQ(infiles[0], infiles, numFiles);
    }
    if (xFlag) {
        extractXO(infiles[0], infiles, numFiles, oFlag);
    }
    if (tFlag) {
        listTV(infiles[0], vFlag);
    }
    if (dFlag) {
        deleteD(infiles[0], infiles, numFiles);
    }
    if (AFlag) {
        appendA(infiles[0], (int) strtol(arg, NULL, 10));
    }

    return 0;
}
