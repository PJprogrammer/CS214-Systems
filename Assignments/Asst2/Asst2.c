/*
 * Author: Paul John and Binhong Ye
 * Purpose: Plagiarism Checker
 * Language:  C
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <pthread.h>
#include <dirent.h>
#include <unistd.h>
#include "Asst2.h"
#include "colors.h"


int main(int argc, char* argv[]) {
    // Create 2d linked list
    List* list = malloc(sizeof(List));
    initList(list);

    // Allocate memory for starting directory
    char* dirName = (char*) calloc(strlen(argv[1])+1, sizeof(char));
    strcat(dirName, argv[1]);
    if(dirName[strlen(dirName)-1] == '/') { // Remove trailing slash if present
        dirName[strlen(dirName)-1] = '\0';
    }

    // Initialize mutex
    pthread_mutexattr_t mutexattr;
    pthread_mutexattr_init(&mutexattr);
    pthread_mutex_t lock;
    pthread_mutex_init(&lock, &mutexattr);

    // Initialize pthread attribute struct
    pthread_attr_t pthreadAttr;
    pthread_attr_init(&pthreadAttr);

    // Initialize argument passed to thread
    DirHandler* dirThreadArg = malloc(sizeof(DirHandler));
    dirThreadArg->list = list;
    dirThreadArg->name = dirName;
    dirThreadArg->lock = &lock;

    // Create linked list to store thread created
    PthreadNode* pthreadNode = malloc(sizeof(PthreadNode));
    pthreadNode->next = NULL;

    // Start thread & Wait for thread to finish
    pthread_create(&pthreadNode->pthread, &pthreadAttr, dirHandler, dirThreadArg);
    pthread_join(pthreadNode->pthread, NULL);

    // Clean up resources for the thread
    free(pthreadNode);
    pthread_mutex_destroy(&lock);
    pthread_mutexattr_destroy(&mutexattr);
    pthread_attr_destroy(&pthreadAttr);

    //printList(list);

    // Compute results using the 2d linked list
    outputResults(list);

    // Free 2d linked list
    freeList(list);
    free(list);

    return 0;
}

/**
 * Initialize 2d linked list
 * @param list
 */
void initList(List* list) {
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}

/**
 * Debugging purposes only: Print 2d linked list
 * @param list
 */
void printList(List* list) {
    Node* temp = list->head;
    while (temp != NULL) {
        printf("path: %s, num: %.1f\t->\t", temp->name, temp->val);

        Node* tempToken = temp->child;
        while (tempToken != NULL) {
            printf("token: %s, num: %.1f\t->\t", tempToken->name, tempToken->val);

            tempToken = tempToken->child;
        }
        printf("NULL\n\t↓\n");

        temp = temp->next;
    }
    printf("NULL\n\n\n");
}

/**
 * Append new linked list (representing a file's tokens) to the 2d linked list
 * @param list
 * @param node
 */
void appendNode(List* list, Node* node) {
    if (list->head == NULL && list->tail == NULL) {  // Check if 2d linked list is empty
        list->head = node;
        list->tail = node;
    } else { // Append new linked list
        list->tail->next = node;
        list->tail = node;
    }

    list->size++;
}

/**
 * Append new node (representing a token in a file) to the file's linked list
 * @param head
 * @param node
 */
void insertToken(Node* head, Node* node) {
    Node* temp = head;
    if(temp->child == NULL) { // Check if linked list of file tokens is empty
        temp->child = node;
    } else {
        while (temp->child != NULL) {
            if(strcmp(node->name, temp->child->name) < 0) {
                // Append node in correct alphabetical order
                Node* tempNode = temp->child;
                temp->child = node;
                node->child = tempNode;
                return;
            } else if(strcmp(node->name, temp->child->name) == 0) {
                // Increase count for token already seen
                temp->child->val += 1;

                // Free unused node parameter
                free(node->name);
                free(node);
                return;
            }

            temp = temp->child;
        }

        // Append node to end of list (this token is alphabetically greater than all tokens in the list)
        temp->child = node;
    }
}

/**
 * Debugging purposes only: Print a file's token linked list
 * @param head
 */
void printTokenList(Node* head) {
    printf("\n%s\n", head->name);
    head = head->child;

    while (head != NULL) {
        printf("%s\n", head->name);
        head = head->child;
    }
}

/**
 * Free entire 2d linked list
 * @param list
 */
void freeList(List* list) {
    Node* head = list->head;

    while (head != NULL) {
        Node* tempNode = head;
        head = head->next;

        while (tempNode != NULL) {
            Node* tempChild = tempNode;
            tempNode = tempNode->child;

            // Free token/file name & node
            free(tempChild->name);
            free(tempChild);
        }
    }
}

/**
 * Parse tokens for a file and add to 2d linked list
 * @param arg
 * @return
 */
void* fileHandler(void* arg) {
    Node* head = (Node*) arg;

    char* line = NULL;
    size_t len = 0;

    FILE* fp = fopen(head->name, "r"); // Open file

    int numTokens = 0;  // Count number of total tokens in file

    // Allocate memory to hold token currently being parse
    int cTokenMaxLen = INIT_TOKEN_LEN;
    char *cToken = (char *) calloc(cTokenMaxLen , sizeof(char));
    int cTokenIndex = -1;

    while (getline(&line, &len, fp) != -1) { // Read file line by line

        // Parse one line
        for (int i = 0; i < strlen(line); ++i) {
            if(checkDelimiter(line[i]) == 1) { // Check to see if current character is a delimiter
                if(cTokenIndex != -1) { // Check if token is currently being parsed
                    // Append current token to linked list
                    Node* token = (Node*) malloc(sizeof(Node));
                    token->next = NULL;
                    token->child = NULL;
                    token->val = 1;
                    token->name = strdup(cToken);

                    insertToken(head, token);

                    numTokens++;

                    // Reset current token
                    cTokenIndex = -1;
                    for (int j = 0; j < strlen(cToken); ++j) {
                        cToken[j] = '\0';
                    }
                }
            } else if(isalpha(line[i]) != 0 || line[i] == '-') { // Check to see if character is a letter or hyphen
                // Convert all uppercase letters to lowercase
                if(isalpha(line[i]) != 0) {
                    line[i] = tolower(line[i]);
                }

                if(cTokenIndex == -1) { // Check if token is currently being parsed
                    cTokenIndex = 0; // Parse a new token
                }

                // Check to see if enough space to append character to current token
                if (strlen(cToken) + 1 < cTokenMaxLen) { // Check if enough memory
                    strncat(cToken, &line[i], 1);
                } else { // Reallocate memory
                    cToken = (char *) realloc(cToken, cTokenMaxLen * 2 * sizeof(char));
                    cTokenMaxLen = cTokenMaxLen * 2;
                    strncat(cToken, &line[i], 1);
                }
                cTokenIndex++;
            }
        }

        // Handle valid token at end of line
        if(cTokenIndex != -1) { // Check if token is currently being parsed
            // Append current token to linked list
            Node* token = (Node*) malloc(sizeof(Node));
            token->next = NULL;
            token->child = NULL;
            token->val = 1;
            token->name = strdup(cToken);

            insertToken(head, token);

            numTokens++;

            // Reset current token
            cTokenIndex = -1;
            for (int j = 0; j < strlen(cToken); ++j) {
                cToken[j] = '\0';
            }
        }
    }

    // Total tokens in a file
    head->val = numTokens;

    //printTokenList(head);

    // Free resources
    free(cToken);
    free(line);
    fclose(fp);

    pthread_exit((void*) 0);
}

/**
 * Iterate over all files/sub-directories and add each file to 2d linked list
 * @param arg
 * @return
 */
void* dirHandler(void* arg) {
    DirHandler* dirHandlerArg = (DirHandler*) arg;

    struct dirent *dirEntry;
    DIR *dir = opendir(dirHandlerArg->name); // Open directory

    // Initialize thread linked list & thread attribute struct
    PthreadNode* head = NULL;
    pthread_attr_t pthreadAttr;
    pthread_attr_init(&pthreadAttr);

    if(dir == NULL) { // Check if unable to access directory
        fprintf(stderr, "Error opening directory: %s\n", dirHandlerArg->name);

        // Free resources & gracefully return
        free(dirHandlerArg->name);
        free(dirHandlerArg);

        pthread_exit((void*) -1);
    } else {
        // Loop through all files/sub-directories
        while ((dirEntry = readdir(dir)) != NULL) {
            if(dirEntry->d_type == DT_DIR && checkValidDir(dirEntry->d_name) == 1) { // Check if directory
                // Create the new thread's (handle sub-directory recursively) argument
                DirHandler* dirThreadArg = malloc(sizeof(DirHandler));
                dirThreadArg->lock = dirHandlerArg->lock;
                dirThreadArg->list = dirHandlerArg->list;
                // Append directory name to path
                dirThreadArg->name = (char*) calloc(strlen(dirHandlerArg->name)+strlen(dirEntry->d_name)+2, sizeof(char));
                strcat(dirThreadArg->name, dirHandlerArg->name);
                strcat(dirThreadArg->name, "/");
                strcat(dirThreadArg->name, dirEntry->d_name);

                // Create new thread to handle sub-directory recursively
                PthreadNode* pthreadNode = malloc(sizeof(PthreadNode));
                pthreadNode->next = NULL;
                head = appendPthreadNode(head, pthreadNode);
                pthread_create(&pthreadNode->pthread, &pthreadAttr, dirHandler, dirThreadArg);
            } else if(dirEntry->d_type == DT_REG) { // Check if regular file
                // Create new file linked list
                Node* token = (Node*) malloc(sizeof(Node));
                token->next = NULL;
                token->child = NULL;
                token->val = 0;
                // Append file name to path
                token->name = (char*) calloc(strlen(dirHandlerArg->name)+strlen(dirEntry->d_name)+2, sizeof(char));
                strcat(token->name, dirHandlerArg->name);
                strcat(token->name, "/");
                strcat(token->name, dirEntry->d_name);

                // Check if file can be opened with read permissions
                if(fopen(token->name, "r") == NULL) {
                    free(token->name);
                    free(token);
                    continue;
                }

                // Append file's linked list to 2d linked list (use mutex to ensure mutual exclusion)
                pthread_mutex_lock(dirHandlerArg->lock);
                appendNode(dirHandlerArg->list, token);
                pthread_mutex_unlock(dirHandlerArg->lock);

                // Create new thread to parse file's tokens and add them to the linked list
                PthreadNode* pthreadNode = malloc(sizeof(PthreadNode));
                pthreadNode->next = NULL;
                head = appendPthreadNode(head, pthreadNode);
                pthread_create(&pthreadNode->pthread, &pthreadAttr, fileHandler, token);
            }
        }
    }

    // Join all threads created
    PthreadNode* temp = head;
    while (temp != NULL) {
        pthread_join(temp->pthread, NULL);
        temp = temp->next;
    }

    // Free thread linked list
    while (head != NULL) {
        temp = head;
        head = head->next;
        free(temp);
    }

    // Free additional resources
    free(dirHandlerArg->name);
    free(dirHandlerArg);
    pthread_attr_destroy(&pthreadAttr);

    free(dir);

    pthread_exit((void*) 0);
}

/**
 * Append node to thread linked list
 * @param head
 * @param node
 * @return
 */
PthreadNode* appendPthreadNode(PthreadNode* head, PthreadNode* node) {
    if(head == NULL) {
        head = node;
        return head;
    }

    // Iterate to end of linked list
    PthreadNode* temp = head;
    while (temp->next != NULL) {
        temp = temp->next;
    }

    // Append Node at end of linked list
    temp->next = node;

    return head;
}

/**
 * Parse 2d linked list and output results
 * @param list
 */
void outputResults(List* list) {
    Node* head = list->head;

    // Check for errors
    if (head == NULL) { // Check to see if 2d linked list is empty
        write(STDERR_FILENO, "Error: No files found\n", 22);
    } else if(head == list->tail) { // Check to see if only one file in 2d linked list
        write(STDERR_FILENO, "Error: Only 1 file found\n", 25);
    }

    int totalPairs = combination(list->size, 2); // Find total number of file pairs
    NodePair nodePairs[totalPairs];

    // Fill array with all possible file pair combinations
    int j = 0;
    while (head != NULL) {
        // Use two node pointers
        Node* first = head;
        Node* second = first->next;

        // Keep first node in-place and iterate second node until the end of the list
        while (first != NULL && second != NULL) {
            // Add node pair to array
            NodePair nodePair;
            nodePair.node1 = first;
            nodePair.node2 = second;
            nodePair.weight = first->val + second->val; // Total number of combined tokens (used to sort the file pairs)
            nodePairs[j] = nodePair;

            second = second->next;
            j++;
        }

        head = head->next;
    }

    // Sort nodePairs array
    qsort(nodePairs, totalPairs, sizeof(NodePair), compareFunc);

    // Calculate the frequency of each token in the token list for each file
    calculateProbabilities(list);

    // Calculate & print out results for each pair of files
    for (int i = 0; i < totalPairs; ++i) {
        double result = calculateJensenShannonDistance(nodePairs[i].node1, nodePairs[i].node2);
        printResult(result, nodePairs[i].node1->name, nodePairs[i].node2->name);
    }
}

/**
 * Calculate the frequency of each token in the token list for each file
 * @param list
 */
void calculateProbabilities(List* list) {
    Node* head = list->head;

    // Iterate over each file in 2d linked list
    while (head != NULL) {
        // File linked list
        Node* fileHead = head;
        double totalTokens = fileHead->val;
        fileHead = fileHead->child;

        head = head->next;

        // Divide each token amount by total number of tokens in file
        while (fileHead != NULL) {
            fileHead->val = fileHead->val / totalTokens;
            fileHead = fileHead->child;
        }
    }
}

/**
 * Calculate Jensen Shannon Distance for a pair of files
 * @param node1
 * @param node2
 * @return
 */
double calculateJensenShannonDistance(Node* node1, Node* node2) {
    Node* meanHead = createMeanList(node1->child, node2->child);
    //printMeanList(meanHead);

    double kldNode1 = calculateKullbeckLeiblerDivergence(node1->child, meanHead);
    double kldNode2 = calculateKullbeckLeiblerDivergence(node2->child, meanHead);

    freeMeanList(meanHead);

    return (kldNode1+kldNode2)/2;
}

/**
 * Calculate Kullbeck Leibler Divergence for a file
 * @param head
 * @param meanHead
 * @return
 */
double calculateKullbeckLeiblerDivergence(Node* head, Node* meanHead) {
    double kldSum = 0;

    // Iterate over each token in mean linked list
    while (meanHead != NULL && head != NULL) {
        if(strcmp(meanHead->name, head->name) == 0) { // Check if mean linked list token exists in file
            kldSum += head->val * log10(head->val/meanHead->val); // Calculate token value

            head = head->child;
        }
        meanHead = meanHead->child;
    }

    // Return final summation calculation
    return kldSum;
}

/**
 * Print output for a file pair
 * @param res
 * @param fname1
 * @param fname2
 */
void printResult(double res, char* fname1, char* fname2) {
    // Determine range of result and output in the respective color
    if(res >= 0 && res <= 0.1) {
        printf(RED "%.3f" reset "\t\"%s\" and \"%s\"\n", res, fname1, fname2);
    } else if(res > 0.1 && res <= 0.15) {
        printf(YEL "%.3f" reset "\t\"%s\" and \"%s\"\n", res, fname1, fname2);
    } else if(res > 0.15 && res <= 0.2) {
        printf(GRN "%.3f" reset "\t\"%s\" and \"%s\"\n", res, fname1, fname2);
    } else if(res > 0.2 && res <= 0.25) {
        printf(CYN "%.3f" reset "\t\"%s\" and \"%s\"\n", res, fname1, fname2);
    } else if(res > 0.25 && res <= 0.3) {
        printf(BLU "%.3f" reset "\t\"%s\" and \"%s\"\n", res, fname1, fname2);
    } else if(res > 0.3) {
        printf(WHT "%.3f" reset "\t\"%s\" and \"%s\"\n", res, fname1, fname2);
    } else{
        write(STDERR_FILENO, "Error: Invalid Results\n", 23);
    }
}

/**
 * Create mean list for a pair of files
 * @param node1
 * @param node2
 * @return
 */
Node* createMeanList(Node* node1, Node* node2) {
    Node* head = NULL;

    // Iterate through the tokens in both files
    while (node1 != NULL || node2 != NULL) {
        // Insert token into mean list while there are still tokens left to parse
        if(node1 != NULL) {
            head = insertMeanNode(head, node1);
            node1 = node1->child;
        }
        if(node2 != NULL) {
            head = insertMeanNode(head, node2);
            node2 = node2->child;
        }
    }

    // Halve each value in the mean list
    Node* temp = head;
    while (temp != NULL) {
        temp->val /= 2;
        temp = temp->child;
    }

    return head;
}

/**
 * Debugging purposes only: Print mean linked list
 * @param list
 */
void printMeanList(Node* head) {
    while (head != NULL) {
        printf("token: %s, num: %.1f\t->\t", head->name, head->val);
        head = head->child;
    }
    printf("\n\n\n");
}

/**
 * Insert a Node into mean linked list
 * @param head
 * @param node
 * @return
 */
Node* insertMeanNode(Node* head, Node* node) {
    // Duplicate node to insert (so as to not modify the original reference)
    Node* copyNode = (Node*) malloc(sizeof(Node));
    copyNode->name = strdup(node->name);
    copyNode->val = node->val;
    copyNode->child = NULL;
    copyNode->next = NULL;

    Node* temp = head;
    if(temp == NULL) { // Check if list is empty
        temp = copyNode;
        return temp;
    } else if(strcmp(copyNode->name, temp->name) < 0) { // Check if token comes before the head token alphabetically
        copyNode->child = temp;
        return copyNode;
    } else if(strcmp(copyNode->name, temp->name) == 0) { // Check if tokens are equal
        // Add token probability value to existing token in mean list
        temp->val += copyNode->val;

        // Free unused node parameter
        free(copyNode->name);
        free(copyNode);
        return head;
    } else {
        while (temp->child != NULL) {
            if(strcmp(copyNode->name, temp->child->name) < 0) { // Check if token comes before alphabetically
                // Insert node in alphabetical order
                Node* tempNode = temp->child;
                temp->child = copyNode;
                copyNode->child = tempNode;

                return head;
            } else if(strcmp(node->name, temp->child->name) == 0) { // Check if tokens are equal
                // Add token probability value to existing token in mean list
                temp->child->val += copyNode->val;

                // Free unused node parameter
                free(copyNode->name);
                free(copyNode);
                return head;
            }

            temp = temp->child;
        }

        temp->child = copyNode;
    }

    return head;
}

/**
 * Free mean linked list
 * @param head
 * @return
 */
Node* freeMeanList(Node* head) {
    // Iterate over each node in linked list
    while (head != NULL) {
        Node* temp = head;
        head = head->child;

        // Free node resources
        free(temp->name);
        free(temp);
    }
}

/**
 * Check if a character is a delimiter
 * @param character
 * @return
 */
int checkDelimiter(char character) {
    switch (character) {
        case ' ':
            break;
        case '\t':
            break;
        case '\v':
            break;
        case '\f':
            break;
        case '\n':
            break;
        case '\r':
            break;
        default:
            return 0;
    }

    return 1;
}

/**
 * Make sure directory is not the special hidden directories (. and ..)
 * @param dirName
 * @return
 */
int checkValidDir(char* dirName) {
    if(strlen(dirName) > 0 && dirName[0] == '.') { // Check for . hidden directory
        return 0;
    } else if(strlen(dirName) > 1 && dirName[0] == '.' && dirName[1] == '.') { // Check for .. hidden directory
        return 0;
    }

    return 1;
}

/**
 * Calculate Combinations: nCr
 * @param n
 * @param r
 * @return
 */
int combination(int n, int r) {
    return factorial(n) / (factorial(r) * factorial(n-r));
}

/**
 * Find factorial of a number
 * @param n
 * @return
 */
unsigned long long factorial(int n) {
    unsigned long long res = 1;

    for (int i = 1; i <= n; ++i) {
        res *= i;
    }

    return res;
}

/**
 * Comparator for qsort function
 * @param a
 * @param b
 * @return
 */
int compareFunc (const void * a, const void * b) {
    return (int) ( ((NodePair*)a)->weight - ((NodePair*)b)->weight );
}