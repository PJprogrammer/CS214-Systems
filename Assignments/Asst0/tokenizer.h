#pragma once

int checkTokenType(char* token, int index);
char* getTokenName(int tokenNum);
int checkDelimiter(char character);
int checkComment(char char1, char char2);
int checkKeywordPresent(char* input, int i);
int isKeyword(char* keyword);