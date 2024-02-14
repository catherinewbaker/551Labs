//THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING A TUTOR, CODE WRITTEN BY OTHER STUDENTS, OR CODE DERIVED FROM AN AI TOOL- Catherine Baker
#include <stddef.h>
#include <unistd.h>
#include <stdio.h>
#include "my_malloc.h"

// one global variable to track head and our error variable as allotted
FreeListNode head = NULL;
MyErrorNo my_errno = MYNOERROR; // MYNOERROR = no error

// updates the chunk header for each new node created
void setChunkHead(FreeListNode chunk, size_t size){
    int *header = (int*)chunk; // ints use only 4 bytes, also stored as a pointer to header so we actually change the info there
    header[0] = size; // first 4 bytes hold size
    header[1] = 1; // second 4 bytes tells us that we've set aside space for this one already (not free)
}

//my_malloc: returns a pointer to a chunk of heap allocated memory
void* my_malloc(size_t size) {
    // initialize locals
    FreeListNode current = head; // track the head of the list so we can traverse it
    FreeListNode previous = NULL;

    // 1. Calculate total size
    //      A. add 8 bytes to request for the header
    size += CHUNKHEADERSIZE; 

    //      B. add 1 byte to size until it is a multiple of 8 and >= 16
    if (size % 8 != 0 || size < 16) { 
        size += (8 - (size % 8));
    }


    // 2. traverse free list for usable node
    while (current != NULL) { // while we have a node to look at...
        if (current->size >= size) { // if this node has enough size for our memory request...
            // A. check if it's big enough to split
            if (current->size >= size + 16) {
                // (1) if yes then split it -> newChunk is the free end of the node
                FreeListNode newChunk = (FreeListNode)((char*)current + size); // starting address of free chunk = current address + size requested (including header)
                newChunk->size = current->size - size ; // unused chunk size is that of our current node - the storage we need
                newChunk->flink = current->flink; // this smaller chunk points to what the original larger chunk pointed to
                // (2) adjust the now smaller current chunk size (this one is sent for memory request)
                current->size = size; // we've saved size amount of space
                // (3) contingencies left based on our list position
                if(current == head || previous == NULL){ // if we are at the head of the list or if there is an empty list or current was the only node in the list...
                    head = newChunk; // then newChunk becomes the new head of the list
                }
                else { // otherwise if we had a populated list and we weren't at the head...
                    previous->flink = newChunk; // then previous is now pointing to the smaller chunk we just split off (instead of current)
                }
            } // otherwise...
            else { // if we didn't have a chunk big enough to split in two...
                // (1) only check the contingency cases to reset flink connections (we already have current->size)
                if (previous) { // if previous exists
                    previous->flink = current->flink; // then it links to what current linked to
                }
                else { // otherwise
                    head = current->flink; // we have a new head (or if we took the last node on the list, then head == NULL)
                }
            }

            // B. set chunk header for this new chunk of memory
            setChunkHead(current, current->size);

            // C. check that we have an actual pointer to a node
            if(!current){ // otherwise...
                my_errno = MYENOMEM; // return an error
                fprintf(stderr, "%d", my_errno);
                return NULL;
            }

            // D. return pointer to memory
            return (void*)((char*)current + CHUNKHEADERSIZE); // return pointer to current node + the chunk header size
        }
        // skip to next node while tracking previous node if this node didn't have enough space for our request
        previous = current;
        current = current->flink;
    }

    // 3. if we didn't find a suitable chunk in free list then we continue to this line of code
    if (size <= 8192) { // if size request is smaller than 8192...
        // A. Call sbrk(8192)
        current = (FreeListNode)sbrk(8192);
        // B. check if that request failed
        if((void*)current == (void*)-1){
            my_errno = MYENOMEM; // and return an error if yes
            fprintf(stderr, "%d", my_errno);
            return NULL;
        }
        // C. Otherwise set the correct size of the node
        current->size = 8192;
        // D. check if it's too big
        if (current->size >= size + 16) {
            // (1) if yes then split it - newChunk is the free part
            FreeListNode newChunk = (FreeListNode)((char*)current + size); // starting address of free chunk
            newChunk->size = current->size - size; // unused chunk size is that of our current node - the storage we need
            newChunk->flink = current->flink; // this smaller chunk points to what the original larger chunk pointed to
            // (2) adjust the now smaller current chunk size (the one we are returning)
            current->size = size; // we've saved size amount of space
            // (3) now the contingencies
            if (current == head || previous == NULL) { // if we're at the head, have an empty list, or current is the only node in the list
                head = newChunk; // then newChunk is our head
            }
            else { // otherwise...
                previous->flink = newChunk; // previous is pointing to the smaller chunk we just split off
            }
        }
        // E. if our chunk wasn't big enough to split
        else {
            // (1) then check our contingencious and setup current->flink based on our list population (as done before)
            if (previous) {
                previous->flink = current->flink;
            }
            else {
                head = current->flink;
            }
        }
    }
    // 4. or, if we need more size than 8192...
    else {
        // (1) create a stack of memory the exact size we need
        current = (FreeListNode)sbrk(size);
        // (2) do the error check
        if((void*)current == (void*)-1){
            my_errno = MYENOMEM;
            fprintf(stderr, "%d", my_errno);
            return NULL;
        }
        current->size = size; // confirm our current->size = size
    }
    // 5. update chunk header for this new chunk of memory
    setChunkHead(current, current->size);


    // 6. final error check before returning possible empty node
    if(!current){
        my_errno = MYENOMEM;
        fprintf(stderr, "%d", my_errno);
        return NULL;
    }
    // 7. return final pointer to the block of memory after the bookkeeping header
    return (void*)((char*)current + CHUNKHEADERSIZE);
}


//my_free: reclaims the previously allocated chunk referenced by ptr
void my_free(void* ptr) {
    // 1. Initialize local variables
        // A. pointer to the header data for the received pointer 
    int *header = (int*)(ptr - CHUNKHEADERSIZE); // '-8' because all returned my_malloc pointers point to data after header
    // 2. error return if ptr is NULL
    if (ptr == NULL || header[1] != 1) {
        my_errno = MYBADFREEPTR;
        fprintf(stderr, "%d", my_errno);
        return;
    }

    // 3. find the chunk ptr references
    FreeListNode chunk = (FreeListNode)(header);
    chunk->size = header[0];
    chunk->flink = NULL;
    //     A. set flink for new free chunk
    header[1] = 0; // reset header pointer to identify a free node

    // 4. insert chunk into free list (sort by address)
        // B. setup our initial traversing variables
    FreeListNode current = head;

    //     C. If the list is empty or the freed block's address is less than the head's address
    if (!head || chunk < head) {
        chunk->flink = head;
        head = chunk;
        return;
    }

        // D. traverse node list until we are at the address of a node that is equal to or greater than the node we're on
    while(current->flink && current->flink < chunk){
        current = current->flink;
    }

        // E. Now that we've found the spot the node goes, change the flinks on contingency
    chunk->flink = current->flink;
    current->flink = chunk;
}

//free_list_begin(): returns pointer to first chunk in free list
FreeListNode free_list_begin(void) {
    // since we are keeping track of the head of the list as a global variable, this list should only return head
    // if head == NULL, this function returns NULL
    return head;
}

//coalesce_free_list(): merge adjacent chunks on the free list
void coalesce_free_list(void) {
    // 1. Initialize one traversal variable this time
    FreeListNode current = head;
    FreeListNode placeholder = NULL;
    //2. traverse free list for chunks w memory addresses right next to each other
    while (current != NULL && current->flink != NULL) {
        // A. if currentAddress + currentSize == nextAddress then...
        if (((char*)current + current->size) == (char*)current->flink) { // since current pointer is after current chunk header, adding current->size skips to the next pointer after it's chunk header
            // (1) we have two adjacent nodes! combine then
            current->size = current->size + current->flink->size; // currentSize = currentSize + nextSize + headerSizeOfNextNode
            placeholder = current->flink; // current now points to the node after the free adjacent node
            current->flink = placeholder->flink;
            continue;  // Continue to recheck the merged node with its new next node
        }
        // B. otherwise if they aren't adjacent then skip to the next node
        current = current->flink; // because if 3 nodes are adjacent then we'd want to
        // double check there's no more adjacent nodes again after combining the first two
    }
}
