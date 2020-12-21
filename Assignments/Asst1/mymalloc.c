#include <stdio.h>
#include "mymalloc.h"

bool isMemoryInitialized = false;
bool isError = false;

/**
 * Initialize memory
 */
void initializeMemory() {
    struct metaData* initMeta = (struct metaData*) &myblock[0];
    initMeta->len = SIZE-sizeof(struct metaData);
    initMeta->isFree = true;
}

/**
 * Our implementation of malloc()
 * @param size Size of bytes to allocate
 * @param fileName __FILE__
 * @param lineNum __LINE__
 * @return Returns a void* to the start of the allocated memory
 */
void* mymalloc(int size, char* fileName, int lineNum) {
    // Check if memory contains initial metadata block
    if(!isMemoryInitialized) {
        initializeMemory();
        isMemoryInitialized = true;
    }

    int i = 0;
    while(i < SIZE) {
        // Pointer to current meta data block
        struct metaData* currentMeta = (struct metaData*) &myblock[i];

        // Check if free space is available
        if(currentMeta->isFree && currentMeta->len >= size) {
            int prevLen = currentMeta->len;
            currentMeta->len = size;
            currentMeta->isFree = false;

            // Check if free space is big enough for another free block of memory
            if(prevLen >= size + sizeof(struct metaData) + MIN_BLOCK) { // Check if enough space for metadata block
                // Add meta data for left over free space
                struct metaData* newMeta = (struct  metaData*) &myblock[i+sizeof(struct metaData)+size];
                newMeta->isFree = true;
                newMeta->len = prevLen-size-sizeof(struct metaData);
            } else if(prevLen >= size) { // Update the length of metadata block to include extra padding
                currentMeta->len = prevLen;
            }

            // Return pointer to start of memory block
            return &myblock[i+sizeof(struct metaData)];
        }

        // Increment pointer to start of next meta data block
        i += sizeof(struct metaData) + currentMeta->len;
    }

    // Case D: Saturation of dynamic memory
    // No free space found, so return NULL
    printf("No free space found for memory allocation at line %d in %s", lineNum, fileName);
    return NULL;
}

/**
 * Our implementation of free()
 * @param ref pointer to memory block allocated by malloc
 * @param fileName __FILE__
 * @param lineNum __LINE__
 */
void myfree(void* ref, char* fileName, int lineNum) {
    isError = false;

    if(ref == NULL) {
        printf("Trying to free null pointer at line %d in %s", lineNum, fileName);
        isError = true;
        return;
    }

    // Compare pointer to the valid pointers in memory
    int i = 0;
    int freeBlock = 0; // Keep track of most recent free metadata block
    int containsPointer = -1; // Index of metadata block to free

    // Check if ref is valid pointer in memory
    while (i < SIZE) {
        // Pointer to current meta data block
        struct metaData* currentMeta = (struct metaData*) &myblock[i];

        if(&myblock[i+sizeof(struct metaData)] == ref) {
            containsPointer = i;
            break;
        }

        // Update start of most recent free metadata blocks
        if(currentMeta->isFree == true && freeBlock == -1) {
            freeBlock = i;
        } else if(currentMeta->isFree == false) {
            freeBlock = -1;
        }

        // Increment pointer to start of next meta data block
        i += sizeof(struct metaData) + currentMeta->len;
    }

    // Case B: Free()ing pointers that were not allocated by malloc()
    if(containsPointer == -1) {
        printf("Trying to free pointer not allocated by malloc() at line %d in %s", lineNum, fileName);
        isError = true;
        return;
    }

    struct metaData* currentMeta = (struct metaData*) &myblock[containsPointer];
    // Check if pointer has already been freed
    if(currentMeta->isFree == false) {
        currentMeta->isFree = true;

        // Check if metadata blocks after currentMeta are free
        i = containsPointer;
        do{
            i += sizeof(struct metaData) + ((struct metaData*) &myblock[i])->len;
        }
        while (i < SIZE && ((struct metaData*) &myblock[i])->isFree);

        // Combine adjacent free metadata blocks to stop memory fragmentation
        if(freeBlock != -1) { // Check if metadata blocks before currentMeta are free
            currentMeta = (struct metaData*) &myblock[freeBlock];
            currentMeta->len = i-(freeBlock+sizeof(struct metaData));
        } else {
            currentMeta->len = i-(containsPointer+sizeof(struct metaData));
        }

    } else {
        // Case C: Redundant free()ing of the same pointer
        printf("Already freed pointer at line %d in %s", lineNum, fileName);
        isError = true;
    }

}

/**
 * Checks to see if memory only contains one free metadata block
 * @return bool
 */
bool isMemCorrectlyConfigured() {
    struct metaData* initMeta = (struct metaData*) &myblock[0];
    if(initMeta->len != SIZE-sizeof(struct metaData) || initMeta->isFree != true) {
        return false;
    }

    return true;
}