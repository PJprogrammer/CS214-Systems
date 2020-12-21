/*
 * Author: Paul John and Binhong Ye
 * Purpose: Plagiarism Checker - Header File
 * Language:  C
 */

#pragma once

#include <sys/param.h>

#define INIT_TOKEN_LEN 5

// 2d Linked List Node
typedef struct Node {
    char* name;
    double val;
    struct Node *next;
    struct Node *child;
} Node;

// 2d Linked List
typedef struct List {
    struct Node* head;
    struct Node* tail;
    int size;
} List;

// Directory Handler Thread Argument
typedef struct DirHandler {
    pthread_mutex_t* lock;
    List* list;
    char* name;
} DirHandler;

// Thread Linked List
typedef struct PthreadNode {
    pthread_t pthread;
    struct PthreadNode* next;
} PthreadNode;

// File Pair Linked List
typedef struct NodePair {
    struct Node* node1;
    struct Node* node2;
    double weight;
} NodePair;

// 2d Linked List Methods
void initList(List* list);
void printList(List* list);
void appendNode(List* list, Node* node);
void insertToken(Node* head, Node* node);
void printTokenList(Node* head);
void freeList(List* list);

// File & Directory Handler Methods
void* fileHandler(void* arg);
void* dirHandler(void* arg);
PthreadNode* appendPthreadNode(PthreadNode* head, PthreadNode* node);

// Jensen Shannon Distance Calculation Methods
void outputResults(List* list);
void calculateProbabilities(List* list);
double calculateJensenShannonDistance(Node* node1, Node* node2);
double calculateKullbeckLeiblerDivergence(Node* head, Node* meanHead);
void printResult(double res, char* fname1, char* fname2);

// Mean Linked List Methods
Node* createMeanList(Node* node1, Node* node2);
void printMeanList(Node* head);
Node* insertMeanNode(Node* head, Node* node);
Node* freeMeanList(Node* head);

// Helper Methods
int checkDelimiter(char character);
int checkValidDir(char* dirName);
int combination(int n, int r);
unsigned long long factorial(int n);
int compareFunc (const void * a, const void * b);