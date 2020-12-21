#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mymalloc.h"

#define malloc(x) mymalloc(x, __FILE__, __LINE__)
#define free(x) myfree(x, __FILE__, __LINE__)

#define SEED 91377127
#define TEST_BLOCK_SIZE 128
#define MYMALLOC_MEMORY_SIZE 4096

#define YELLOW "\033[1;33m"
#define RED "\033[1;31m"
#define GREEN "\033[1;32m"
#define RESET "\033[0m"

// Pseudorandom int between 1 and max
int random_max(unsigned int max) {
    return rand() % max + 1;
}

void fail() {
    printf("%sFAILED%s\n", RED, RESET);
    exit(EXIT_SUCCESS);
}

void succeed(char *expected_output) {
    printf("%sOK\n", GREEN, RESET);
    printf("%sCHECK EXPECTED OUPUT: %s%s\n", YELLOW, expected_output, RESET);
    exit(EXIT_SUCCESS);
}

int main (int argc, char* argv[]) {
    srand(SEED);
    if (argc != 2) {
        printf("Usage: %s case_number\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int caseno = atoi(argv[1]);
    char expected_output[100];

    printf("Running testcase #%i...\n", caseno);

    if (caseno == 1) {
        // Malloc space, set pointer values, swap data between pointers, test for values
        strcpy(expected_output, "no error messages");

        int a = 123456;
        int b = 7891011;
        int *ptr1 = (int *)malloc(sizeof(int));
        *ptr1 = a;
        int *ptr2 = (int *)malloc(sizeof(int));
        *ptr2 = b;
        if (*ptr1 != a || *ptr2 != b) fail();
        
        *ptr1 = b;
        *ptr2 = a;

        if (*ptr1 != b || *ptr2 != a) fail();

    } else if (caseno == 2) {
        // Malloc space, set pointer values, swap data between pointers, test for values, free pointers
        strcpy(expected_output, "no error messages");

        int a = 123456;
        int b = 7891011;
        int *ptr1 = (int *)malloc(sizeof(int));
        *ptr1 = a;
        int *ptr2 = (int *)malloc(sizeof(int));
        *ptr2 = b;
        if (*ptr1 != a || *ptr2 != b) fail();
        
        *ptr1 = b;
        *ptr2 = a;

        if (*ptr1 != b || *ptr2 != a) fail();

        free(ptr1);
        free(ptr2);

    } else if (caseno == 3) {
        // Store pointers to malloced space in malloced space and free them
        strcpy(expected_output, "no error messages");

        int a = 123456;
        int b = 7891011;
        int **pptr1 = malloc(sizeof(int *));
        int **pptr2 = malloc(sizeof(int *));
        *pptr1 = malloc(sizeof(int));
        *pptr2 = malloc(sizeof(int));

        free(pptr1);
        free(pptr2);

    } else if (caseno == 4) {
        // Random sequence of mallocs and frees and mallocs
        strcpy(expected_output, "no error messages");

        const int max_alloc = 5;
        const int count = 10;
        void *ptrs[count];
        for (int i = 0; i < count; ++i) {
            ptrs[i] = malloc(random_max(max_alloc));
            if (!ptrs[i]) fail();
        }

        for (int i = 0; i < count; ++i) {
            int r = random_max(count) - 1;
            while (!ptrs[r])
                r = random_max(count) - 1;
            free(ptrs[r]);
            ptrs[r] = 0;
        }
	    
        for (int i = 0; i < count; ++i) {
            ptrs[i] = malloc(random_max(max_alloc));
            if (!ptrs[i]) fail();
        }

    } else if (caseno == 5) {
        // Code does not explode on free of a pointer never allocated
        strcpy(expected_output, "invalid free");

        void *ptr = malloc(1);
        free(ptr + 1);

    } else if (caseno == 6) {
        // Code attempts to free the location of a stored pointer (&ptr)
        strcpy(expected_output, "invalid free");

        void *ptr = malloc(4);
        free(&ptr);

    } else if (caseno == 7) {
        // Code detects a free called on a pointer in the middle of an allocated block
        strcpy(expected_output, "invalid free");

        char *ptr = (char *)malloc(TEST_BLOCK_SIZE);
        free(ptr + TEST_BLOCK_SIZE / 2);

    } else if (caseno == 8) {
        // Code detects a free called on a pointer between allocated blocks
        strcpy(expected_output, "invalid free");

        void *ptr1 = malloc(TEST_BLOCK_SIZE / 4);
        void *ptr2 = malloc(TEST_BLOCK_SIZE / 4);
        void *ptr3 = malloc(TEST_BLOCK_SIZE / 4);
        void *ptr4 = malloc(TEST_BLOCK_SIZE / 4);
        free(ptr2);
        free(ptr3);

        free(ptr2);

    } else if (caseno == 9) {
        // Code detects a free called on a pointer outside of allocated space
        strcpy(expected_output, "invalid free");

        free((void *)9123);

    } else if (caseno == 10) {
        // Code detects freeing a valid pointer more than once
        strcpy(expected_output, "invalid free");

        void *ptr = malloc(10);
        free(ptr);
        free(ptr);

    } else if (caseno == 11) {
        // Code does not failure when allocation requests overflow available space
        strcpy(expected_output, "invalid malloc");

        void *ptr = malloc(10 * MYMALLOC_MEMORY_SIZE);
        
    } else if (caseno == 12) {
	// Code regularity: saturate memory with 1-byte allocations, free it all,
        // saturate again with 1-byte allocations - should result in same number of successful allocations
        strcpy(expected_output, "invalid mallocs");

        int ok1 = 0;
        void *ptrs[MYMALLOC_MEMORY_SIZE];
        for (int i = 0; i < MYMALLOC_MEMORY_SIZE; ++i) {
            ptrs[i] = malloc(1);
            if (ptrs[i]) ++ok1;
        }

        for (int i = 0; i < MYMALLOC_MEMORY_SIZE; ++i) {
            if (ptrs[i])
                free(ptrs[i]);
        }

        int ok2 = 0;
        for (int i = 0; i < MYMALLOC_MEMORY_SIZE; ++i) {
            ptrs[i] = malloc(1);
            if (ptrs[i]) ++ok2;
        }

        if (ok1 != ok2) fail();

    } else if (caseno == 13) {
        // Code operation: saturate memory with 1-byte allocations, free it all,
        // saturate again with 2-byte allocations - should result in at least 70% the number of successful allocations
        strcpy(expected_output, "invalid mallocs");

        int ok1 = 0;
        void *ptrs[MYMALLOC_MEMORY_SIZE];
        for (int i = 0; i < MYMALLOC_MEMORY_SIZE; ++i) {
            ptrs[i] = malloc(1);
            if (ptrs[i]) ++ok1;
        }

        for (int i = 0; i < MYMALLOC_MEMORY_SIZE; ++i) {
            if (ptrs[i])
                free(ptrs[i]);
        }

        int ok2 = 0;
        for (int i = 0; i < MYMALLOC_MEMORY_SIZE; ++i) {
            ptrs[i] = malloc(2);
            if (ptrs[i]) ++ok2;
        }

        if (ok2 > ok1 || ok2/(float)ok1 < 0.7) fail();
    }

    succeed(expected_output);
}
