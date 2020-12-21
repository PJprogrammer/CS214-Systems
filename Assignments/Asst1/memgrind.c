#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <assert.h>

#include "mymalloc.h"

long caseA();
long caseB();
long caseC();
long caseD();
long caseE();

int main() {

    // tests time counter
    long workA = 0;
    long workB = 0;
    long workC = 0;
    long workD = 0;
    long workE = 0;

    // workload run 50 times, accumulate runtime
    long runCount = 50;
    for (int i = 0; i < runCount; ++i) {
        workA += caseA();
        workB += caseB();
        workC += caseC();
        workD += caseD();
        workE += caseE();
    }

    // print mean run time for each workload
    printf("Workload A Mean Run Time (microsecond): %ld\n",workA/runCount);
    printf("Workload B Mean Run Time (microsecond): %ld\n",workB/runCount);
    printf("Workload C Mean Run Time (microsecond): %ld\n",workC/runCount);
    printf("Workload D Mean Run Time (microsecond): %ld\n",workD/runCount);
    printf("Workload E Mean Run Time (microsecond): %ld\n",workE/runCount);

    return 0;
}

// Test Cases:

// Test A: malloc() 1 byte and immediately free it - do this 120 times:
long caseA() {

    // Runtime counter
    struct timeval start, end;
    gettimeofday(&start, NULL); // Starting clock

    for (int i = 0; i < 120; i++) {
        char* a = malloc(1);
        free(a);
    }

    assert(isMemCorrectlyConfigured());

    gettimeofday(&end, NULL); // Ending clock
    return ((end.tv_usec+end.tv_sec*1000000) - (start.tv_usec+start.tv_sec*1000000));
}

// Test B: malloc() 1 byte, store the pointer in an array - do this 120 times.
// Once you've malloc()ed 120 byte chunks, then free() the 120 1 byte pointers one by one.
long caseB() {
    struct timeval start, end;
    gettimeofday(&start, NULL);
    char* arrB[120]; // initial array for 120 times malloc()

    for (int i = 0; i < 120; i++) {
        arrB[i] = malloc(1);
    }
    for (int j = 0; j < 120; j++)
    {
        free(arrB[j]);
    }

    assert(isMemCorrectlyConfigured());

    gettimeofday(&end, NULL);
    return ((end.tv_usec+end.tv_sec*1000000) - (start.tv_usec+start.tv_sec*1000000));
}



// Test C: 240 times, randomly choose between a 1 byte malloc() or free()ing one of the malloc()ed pointers
//- Keep track of each operation so that you eventually malloc() 120 bytes, in total
//- Keep track of each operation so that you eventually free() all pointers
//(don't allow a free() if you have no pointers to free())

long caseC() {
    struct timeval start, end;
    gettimeofday(&start, NULL);
    int malloc_count = 0; // count malloc()ed space, so it will not call free() when there is no malloc()ed
    int malloc_limit = 0; // boundary for 120 malloc()
    char* arrC[120];

    for (int i = 0; i < 240; i++) {
        int pseudorandom = rand(); // random generator for malloc() or free() in ideal situation
        if (pseudorandom % 2 == 0) { // call malloc()
            if (malloc_limit < 120) { // stop any malloc() if already malloc()ed 120 times
                arrC[malloc_count] = malloc(1);
                malloc_count++; // allow free() working on malloc()ed space
                malloc_limit++; // counter malloc()ed
            } else {
                i--; // skip this iteration if malloc()ed 120 times
            }
        } else { // call free()
            if (malloc_count > 0) { // stop free() on non-malloc()ed space
                free(arrC[malloc_count-1]);
                malloc_count--; // count available free() space
            } else {
                i--; // ignore this iteration for boundary, nothing to be free()
            }
        }
    }

    assert(isMemCorrectlyConfigured());

    gettimeofday(&end, NULL);
    return ((end.tv_usec+end.tv_sec*1000000) - (start.tv_usec+start.tv_sec*1000000));
}

// Test D: malloc() 300 bytes one by one, free() backward one by one
long caseD() {
    struct timeval start, end;
    gettimeofday(&start, NULL);
    char* arrD[300];

    // malloc() entire arrD for 300 times
    for (int i = 0; i < 300; ++i) {
        arrD[i] = malloc(1);
    }

    // free() backward in arrD from the end to the beginning
    for (int j = 299; j >= 0; j--) {
        free(arrD[j]);
    }

    assert(isMemCorrectlyConfigured());

    gettimeofday(&end, NULL);
    return ((end.tv_usec+end.tv_sec*1000000) - (start.tv_usec+start.tv_sec*1000000));
}

// Test E: malloc() 30 * 30 matrix, then free() it
long caseE() {
    struct timeval start, end;
    gettimeofday(&start, NULL);

    // malloc() int-matrix
    int n = 30; // size of matrix
    int** matrix;
    matrix = (int**)malloc(sizeof(int*)*n); // malloc() n's pointers for row
    for (int i = 0; i < n; ++i) {
        matrix[i] = (int*)malloc(sizeof(int)*n); // malloc() n times for column in each row
    }

    // free() matrix
    for (int j = 0; j < n; ++j) {
        int* ptr = matrix[j];
        free(ptr); // free() column in each row
    }
    free(matrix); // free() row pointer

    assert(isMemCorrectlyConfigured());

    gettimeofday(&end, NULL);
    return ((end.tv_usec+end.tv_sec*1000000) - (start.tv_usec+start.tv_sec*1000000));
}