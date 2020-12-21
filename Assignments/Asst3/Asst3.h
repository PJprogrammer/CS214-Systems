/*
 * Author: Paul John and Binhong Ye
 * Purpose: Knock Knock Joke Server
 * Language:  C
 */

#pragma once

#define INIT_NUM_JOKES 5
#define INIT_TOKEN_LEN 5
#define BUFF_SIZE 256
#define FORMAT_PADDING 6
#define ERR_LEN 5

typedef char bool;
enum boolean { false, true };

typedef struct Joke {
    char* setupLine;
    char* punchLine;
} joke;

joke* getJokes(char* fname);
joke* appendJoke(joke* jokeArr, joke myJoke, int* i, int* maxLen);
joke getRandJoke(joke* jokes);

void chatWithClient(int newsockfd, joke randJoke);

char *readSocket(int sockfd, int msgNum, joke myJoke);
bool writeSocket(int sockfd, char* str, int msgNum);
char* appendToken(char* cToken, char* buff, int i, int* maxLen);

bool checkError(int socket, char* resp);
char* checkErrorCode(char *msg, int msgNum, int msgLen, joke myJoke);
char* msgFormatError(char* token, int msgNum);
void sendError(int socket, char error[]);
void fatalError(char* msg);

char* myStrdup(const char *str);
void removeNewline(char* line);