#pragma once

#define SIZE 4096
#define MIN_BLOCK 8

#define malloc( x ) mymalloc( x, __FILE__, __LINE__ )
#define free( x ) myfree( x, __FILE__, __LINE__ )

typedef char bool;
enum boolean { false, true };

static char myblock[SIZE];
typedef struct metaData {
    int len;
    bool isFree;
} metaData;

void initializeMemory();
void* mymalloc(int size, char* fileName, int lineNum);
void myfree(void* ref, char* fileName, int lineNum);
bool isMemCorrectlyConfigured();