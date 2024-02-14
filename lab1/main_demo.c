#include <stdio.h>
#include "my_malloc.h"

void print_free_list(FreeListNode fhead){

    printf("freelist: ");
    
    if (NULL == fhead){
        printf("NULL\n");
    }

    else
    {
        FreeListNode fnode = fhead;
        while (fnode) {
            printf("%lu ", fnode->size);
            fnode = fnode->flink;
        }
        printf("\n");    
    }
}





int main(int argc, char* agrv[]){


    int sizes[3] = {10, 50, 100};
    int *p[3];
    FreeListNode fhead = free_list_begin(); // fhead should be NULL here

    for (int i = 0; i < 3; i++){
        
        p[i] = (int*) my_malloc(sizeof(int) * sizes[i]); //chunk size is 4 * sizes[i]
        fhead = free_list_begin();
        print_free_list(fhead);

        my_free(p[i]);
        fhead = free_list_begin();
        print_free_list(fhead);
    }

    /*
    * If your implementation is correct, you should print the following information
    * freelist: 8144 
    * freelist: 48 8144 
    * freelist: 48 7936 
    * freelist: 48 208 7936 
    * freelist: 48 208 7528 
    * freelist: 48 208 408 7528 
    */

    return 0;
    
}