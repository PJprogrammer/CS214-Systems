/*
 * Author: Paul John and Binhong Ye
 * Purpose: Knock Knock Joke Server
 * Language:  C
 */

#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>
#include "Asst3.h"

int numJokes;


int main(int argc, char* argv[]) {
    int portno = atoi(argv[1]); // port number

    if(argc < 3) { // Check number of arguments
        fprintf(stderr, "Error: Too few arguments");
        exit(1);
    } else if(portno == 0 || portno >= 65536 || portno <= 5000) { // Check if port is in valid range
        fprintf(stderr, "Error: Invalid Port");
        exit(1);
    }

    joke* jokes = getJokes(argv[2]); // Get all available jokes from text file provided

    int sockfd;
    int newsockfd;
    int clilen;

    struct sockaddr_in serverAddressInfo;
    struct sockaddr_in clientAddressInfo;

    clilen = sizeof(clientAddressInfo);

    // Create server socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    // Assign IP, Port
    bzero((char*) &serverAddressInfo, sizeof(serverAddressInfo));
    serverAddressInfo.sin_port = htons(portno);
    serverAddressInfo.sin_family = AF_INET;
    serverAddressInfo.sin_addr.s_addr = INADDR_ANY;

    // Bind newly created server socket to ip:port
    if(bind(sockfd, (struct sockaddr*) &serverAddressInfo, sizeof(serverAddressInfo)) < 0) {
        fatalError("Error: on binding");
    }

    listen(sockfd, 0); // Listen on server port

    printf("Knock-knock Joke Server Initialized...\n");

    bool shouldRun = true;

    while (shouldRun) {
        // Wait for client to connect to server socket
        newsockfd = accept(sockfd, (struct sockaddr*) &clientAddressInfo, (socklen_t*) &clilen);
        if(newsockfd < 0) {
            shouldRun = false;
            fatalError("Error: on accept");
        }

        // Find IP address of client
        char clntIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, (struct inaddr *)&clientAddressInfo.sin_addr, clntIP, sizeof(clntIP));
        printf("\nClient IP is %s\n", clntIP);

        joke randJoke = getRandJoke(jokes); // Get a random joke from the jokes array

        chatWithClient(newsockfd, randJoke); // Do Knock-Knock Joke with client
    }

    return 0;
}

/**
 * Put all jokes from the user provided file into an array
 * @param fname file name
 * @return array of jokes
 */
joke* getJokes(char* fname) {
    char* line = NULL;
    size_t len = 0;
    FILE* fp = fopen(fname, "r");

    if (fp == NULL) { // Check if valid file
        fprintf(stderr, "Error: Can not open %s", fname);
        exit(1);
    }

    joke* jokes;
    int jokesSize = INIT_NUM_JOKES;
    jokes = calloc(jokesSize, sizeof(joke)); // Allocate memory to store jokes
    bool isJokeStarted = false; // Indicator for the joke's setup line

    int i = 0;
    joke currentJoke;
    while (getline(&line, &len, fp) != -1) {
        removeNewline(line); // Remove trailing '\n' or '\r\n'

        if(strlen(line) > 0) { // Check that line is not empty
            if(isJokeStarted == true) { // Check if we saw a joke's setup line
                currentJoke.punchLine = myStrdup(line);
                jokes = appendJoke(jokes, currentJoke, &i, &jokesSize); // append joke to array
                isJokeStarted = false; // Reset joke indicator
            } else {
                currentJoke.setupLine = myStrdup(line);
                isJokeStarted = true;
            }
        }
    }

    if(isJokeStarted == true) { // Check to make sure there are not an odd number of non-blank lines in file
        fprintf(stderr, "Error: Odd number of lines in jokes input file");

        // Gracefully exit
        free(currentJoke.setupLine);
        for (int j = 0; j < i; ++j) {
            free(jokes[j].setupLine);
            free(jokes[j].punchLine);
        }
        free(jokes);
        free(line);
        exit(1);
    }

    free(line);
    numJokes = i; // Set number of jokes encountered

    return jokes;
}

/**
 * Append a joke to the jokes array
 * @param jokeArr jokes array
 * @param myJoke  joke to be appended to array
 * @param i current index of jokes array
 * @param maxLen memory allocated for array
 * @return jokes array
 */
joke* appendJoke(joke* jokeArr, joke myJoke, int* i, int* maxLen) {
    if(*i+1 < *maxLen) { // Check if there is enough space before appending joke
        jokeArr[*i] = myJoke;
    } else { // Otherwise, allocate more space and then append joke
        jokeArr = realloc(jokeArr, *maxLen*2*sizeof(joke));
        *maxLen *= 2;
        jokeArr[*i] = myJoke;
    }

    *i+=1;
    return jokeArr;
}

/**
 * Get random joke from the jokes array
 * @param jokes
 * @return
 */
joke getRandJoke(joke* jokes) {
    // Generate random number within valid range
    time_t t;
    srand((unsigned) time(&t));
    int randNum = rand() % numJokes;

    joke myJoke = jokes[randNum];
    return myJoke;
}

void chatWithClient(int newsockfd, joke randJoke) {

    // Write Message 0 - "Knock, knock."
    bool resp0 = writeSocket(newsockfd, "Knock, knock.", 0);
    if(resp0 == false) return;

    // Read Message  1 - "Who's there?"
    char* resp1 = readSocket(newsockfd, 1, randJoke);
    if(resp1 == NULL || checkError(newsockfd, resp1) == true) {
        free(resp1);
        close(newsockfd);
        return;
    }
    printf("Read msg 1:  %s\n", resp1);
    free(resp1);

    // Write Message 2 - "<setup line>."
    bool resp2 = writeSocket(newsockfd, randJoke.setupLine, 2);
    if(resp2 == false) return;

    // Read Message  3 - "<setup line>, who?"
    char* resp3 = readSocket(newsockfd, 3, randJoke);
    if(resp3 == NULL || checkError(newsockfd, resp3) == true) {
        free(resp3);
        close(newsockfd);
        return;
    }
    printf("Read msg 3:  %s\n", resp3);
    free(resp3);

    // Write Message 4 - "<punch line><punctuation>"
    bool resp4 = writeSocket(newsockfd, randJoke.punchLine, 4);
    if(resp4 == false) return;

    // Read Message  5 - "<expression of A/D/S><punctuation>"
    char* resp5 = readSocket(newsockfd, 5, randJoke);
    if(resp5 == NULL || checkError(newsockfd, resp5) == true) {
        free(resp5);
        close(newsockfd);
        return;
    }
    printf("Read msg 5:  %s\n", resp5);
    free(resp5);

    close(newsockfd);
}

/**
 * Read message from socket (with error handling)
 * @param sockfd socket file descriptor
 * @param msgNum message number
 * @param myJoke knock knock joke
 * @return message
 */
char *readSocket(int sockfd, int msgNum, joke myJoke) {
    // Allocate memory to hold current token being processed
    int cTokenMaxLen = INIT_TOKEN_LEN;
    char *cToken = (char *) calloc(cTokenMaxLen , sizeof(char));

    int tokenNum = 1;
    char status[4]; // message status (REG or ERR)
    int messageLen = -1;

    char *next;
    int n;
    char buffer[BUFF_SIZE];

    while(tokenNum < 4) {
        bzero(buffer, BUFF_SIZE);
        n = read(sockfd, buffer, BUFF_SIZE-1);
        if(n <= 0) { // Check for errors when try to read from socket
            if(n < 0) fprintf(stderr, "Error: reading from socket.\n");
            else if(n == 0) fprintf(stderr, "Error: Client socket closed.\n");
            free(cToken);
            return NULL;
        }

        // Iterate over all the characters read into the buffer
        for (int i = 0; i < n; ++i) {
            if(buffer[i] == '|') { // Check if current token has come to an end
                if(tokenNum == 1) {
                    // Find status of message (REG or ERR)
                    if(strcmp(cToken, "REG") == 0 || strcmp(cToken, "ERR") == 0) {
                        bzero(status, 4);
                        bcopy(cToken, status, 3);
                    } else {
                        return msgFormatError(cToken, msgNum);
                    }
                    bzero(cToken, cTokenMaxLen);
                } else if(tokenNum == 2) {
                    // Find length of message (int)
                    if(strcmp(status, "REG") == 0) {
                        messageLen = strtol(cToken, &next, 10);
                        if ((next == cToken) || (*next != '\0')) { // Check if string is a valid integer
                            return msgFormatError(cToken, msgNum);
                        }
                    } else if(strcmp(status, "ERR") == 0) {
                        return cToken;
                    }
                    bzero(cToken, cTokenMaxLen);
                } else if(tokenNum == 3) {
                    if(i != n-1) { // Check if there is more data to be read
                        return msgFormatError(cToken, msgNum);
                    }

                    // Check message for length and content errors
                    char* errMsg = checkErrorCode(cToken, msgNum, messageLen, myJoke);
                    if(errMsg != NULL) {
                        free(cToken);
                        return errMsg;
                    }
                }

                tokenNum++;
            } else { // Append character to current token
                cToken = appendToken(cToken, buffer, i, &cTokenMaxLen);
            }
        }
    }

    return cToken;
}

/**
 * Write message to socket using the appropriate messaging format
 * @param sockfd socket file descriptor
 * @param str message string
 * @param msgNum message number
 * @return Successful write indicator
 */
bool writeSocket(int sockfd, char* str, int msgNum) {
    // Determine amount of memory to allocate for message
    int strLen = strlen(str);
    int intWidth = snprintf(NULL, 0, "%d", strLen);
    int msgLen = intWidth + strlen(str) + FORMAT_PADDING + 1;

    // Write message to character array in correct format
    char msg[msgLen];
    bzero(msg, msgLen);
    snprintf(msg, msgLen, "REG|%d|%s|", strLen, str);

    // Write message to socket and check for errors
    int n = write(sockfd, msg, msgLen - 1);
    if(n < 0) {
        fprintf(stderr, "Error: writing message to socket.\n");
        return false;
    } else if(n == 0) {
        fprintf(stderr, "Error: Client socket closed.\n");
        return false;
    }

    printf("Write msg %d: REG|%d|%s|\n", msgNum, strLen, str);
    return true;
}

/**
 * Append character to character array
 * @param cToken character array
 * @param buff character to be appended
 * @param i index
 * @param maxLen max allocated size for character array
 * @return character array
 */
char* appendToken(char* cToken, char* buff, int i, int* maxLen) {
    if (strlen(cToken) + 1 < *maxLen) { // Check if enough memory
        strncat(cToken, &buff[i], 1);
    } else { // Reallocate memory
        cToken = (char *) realloc(cToken, *maxLen * 2 * sizeof(char));
        *maxLen = *maxLen * 2;
        strncat(cToken, &buff[i], 1);
    }

    return cToken;
}

/**
 * Check if string is a predefined error code
 * @param socket socket file descriptor
 * @param resp string
 * @return error indicator
 */
bool checkError(int socket, char* resp) {
    bool isError = false;

    // Check if string matches one of the error codes (and print/send error to client accordingly)
    if(strcmp(resp, "M0CT") == 0) {
        fprintf(stderr, "message 0 content was not correct\n");
        isError = true;
    } else if(strcmp(resp, "M0LN") == 0) {
        fprintf(stderr, "message 0 length value was incorrect\n");
        isError = true;
    } else if(strcmp(resp, "M0FT") == 0) {
        fprintf(stderr, "message 0 format was broken\n");
        isError = true;
    } else if(strcmp(resp, "M1CT") == 0) {
        fprintf(stderr, "message 1 content was not correct\n");
        sendError(socket, resp);
        isError = true;
    } else if(strcmp(resp, "M1LN") == 0) {
        fprintf(stderr, "message 1 length value was incorrect\n");
        sendError(socket, resp);
        isError = true;
    } else if(strcmp(resp, "M1FT") == 0) {
        fprintf(stderr, "message 1 format was broken\n");
        sendError(socket, resp);
        isError = true;
    } else if(strcmp(resp, "M2CT") == 0) {
        fprintf(stderr, "message 2 content was not correct\n");
        isError = true;
    } else if(strcmp(resp, "M2LN") == 0) {
        fprintf(stderr, "message 2 length value was incorrect\n");
        isError = true;
    } else if(strcmp(resp, "M2FT") == 0) {
        fprintf(stderr, "message 2 format was broken\n");
        isError = true;
    } else if(strcmp(resp, "M3CT") == 0) {
        fprintf(stderr, "message 3 content was not correct\n");
        sendError(socket, resp);
        isError = true;
    } else if(strcmp(resp, "M3LN") == 0) {
        fprintf(stderr, "message 3 length value was incorrect\n");
        sendError(socket, resp);
        isError = true;
    } else if(strcmp(resp, "M3FT") == 0) {
        fprintf(stderr, "message 3 format was broken\n");
        sendError(socket, resp);
        isError = true;
    } else if(strcmp(resp, "M4CT") == 0) {
        fprintf(stderr, "message 4 content was not correct\n");
        isError = true;
    } else if(strcmp(resp, "M4LN") == 0) {
        fprintf(stderr, "message 4 length value was incorrect\n");
        isError = true;
    } else if(strcmp(resp, "M4FT") == 0) {
        fprintf(stderr, "message 4 format was broken\n");
        isError = true;
    } else if(strcmp(resp, "M5CT") == 0) {
        fprintf(stderr, "message 5 content was not correct\n");
        sendError(socket, resp);
        isError = true;
    } else if(strcmp(resp, "M5LN") == 0) {
        fprintf(stderr, "message 5 length value was incorrect\n");
        sendError(socket, resp);
        isError = true;
    } else if(strcmp(resp, "M5FT") == 0) {
        fprintf(stderr, "message 5 format was broken\n");
        sendError(socket, resp);
        isError = true;
    }

    return isError;
}

/**
 * Check if message send from client has length or content error
 * @param msg
 * @param msgNum
 * @param msgLen
 * @param myJoke
 * @return
 */
char* checkErrorCode(char *msg, int msgNum, int msgLen, joke myJoke) {
    // Allocate space for constructing the correct message 3 content
    int msg3Len = strlen(myJoke.setupLine)-1 + FORMAT_PADDING;
    char msg3[msg3Len + 1];

    if(msgNum == 3) {
        // Construct the correct message 3 content
        bzero(msg3, msg3Len + 1);
        char punc = myJoke.setupLine[strlen(myJoke.setupLine)-1];
        myJoke.setupLine[strlen(myJoke.setupLine)-1] = '\0';
        bzero(msg3, msg3Len + 1);
        snprintf(msg3, msg3Len+1, "%s, who?", myJoke.setupLine);
        myJoke.setupLine[strlen(myJoke.setupLine)] = punc;
    }

    char* status = calloc(ERR_LEN, sizeof(char));

    if((strlen(msg) != msgLen) ||   // Check for length errors based on message number
       (msgNum == 1 && strlen(msg) != 12) ||
       (msgNum == 3 && strlen(msg) != msg3Len)) {

        snprintf(status, ERR_LEN, "M%dLN", msgNum);
    } else if((msgNum == 1 && strcmp(msg, "Who's there?") != 0) || // Check for content errors based on message number
              (msgNum == 3 && strcmp(msg, msg3) != 0) ||
              (msgNum == 5 && ispunct(msg[msgLen-1]) == 0)) {

        snprintf(status, ERR_LEN, "M%dCT", msgNum);
    } else {
        free(status);
        return NULL;
    }

    return status;
}

/**
 * Create a message format error string
 * @param token
 * @param msgNum
 * @return
 */
char* msgFormatError(char* token, int msgNum) {
    free(token);

    char* status = calloc(ERR_LEN, sizeof(char));
    snprintf(status, ERR_LEN, "M%dFT", msgNum);
    return status;
}

/**
 * Send error message (to client)
 * @param socket
 * @param errorCode
 */
void sendError(int socket, char errorCode[]) {
    // Put error message code in correct format
    char msg[10];
    bzero(msg, 10);
    snprintf(msg, 10, "ERR|%s|\n", errorCode);

    // Write to socket and handle any errors
    int n = write(socket, msg, 9);
    if(n < 0) {
        fprintf(stderr, "Error: writing error msg to socket.\n");
    }
}

/**
 * Exit program on error
 * @param msg
 */
void fatalError(char* msg) {
    perror(msg);
    exit(1);
}

/**
 * My implementation of strdup() function in C
 * @param str
 * @return
 */
char* myStrdup(const char *str) {
    char *newStr = malloc(strlen (str) + 1);
    if (newStr == NULL) return NULL;
    strcpy(newStr, str);
    return newStr;
}

/**
 * Remove trailing '\n' or '\r\n' from string
 * @param line
 */
void removeNewline(char* line) {
    int len = strlen(line);
    if(len > 1 && line[len-1] == '\n' && line[len-2] == '\r') { // Check for '\r\n'
        line[len-1] = '\0';
        line[len-2] = '\0';
    } else if (len > 0 && line[len-1] == '\n') { // Check for '\n'
        line[len-1] = '\0';
    }
}
