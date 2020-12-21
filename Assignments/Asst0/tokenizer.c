#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "tokenizer.h"

#define INIT_TOKEN_LEN 5
#define KEYWORD_MAX_SIZE 8

int current_index = 0; // Pointer for current position in each element from argument
int rest_index = 0; // Pointer for checked position in each element from argument
int cTokenMaxLen = INIT_TOKEN_LEN; // Maximum length of current token
int current_token_length; // Length of current token

int main(int argc, char *argv[]) {
    if (argc < 2) { // Check if single string argument is present
        printf("Error");
        return -1;
    }

    char *cToken = (char *) malloc(cTokenMaxLen * sizeof(char)); // Current token
    // Initialize memory
    for (int i = 0; i < cTokenMaxLen; ++i) {
        cToken[i] = '\0';
    }

    int i = 0; // index
    int cTokenI = 0; // current token index
    int prevMatch = -1; // previous matched token
    int cMatch = -1; // current matched token
    while (argv[1][i] != '\0') {
        // Check for C keywords at beginning of string
        if (i == 0) {
            i = checkKeywordPresent(argv[1], i);
        }

        // Skip delimiters & comments in the string; Print C keywords between delimiters
        while (checkDelimiter(argv[1][i]) || checkComment(argv[1][i], argv[1][i + 1])) {
            // Skip delimiter & Check for C keywords
            if (checkDelimiter(argv[1][i])) {
                i++;
                i = checkKeywordPresent(argv[1], i);
            }
                // Skip comments in the form of '/* Comment *\'
            else if (checkComment(argv[1][i], argv[1][i + 1]) == 1) {
                i = i + 2; // Move past '/*'
                while (argv[1][i] != '*' && argv[1][i + 1] != '\\') { // Check for '*\'
                    i++;
                }
                i = i + 2; // Move past '*\'
            }
                // Skip comments in the form of '// Comment \n'
            else if (checkComment(argv[1][i], argv[1][i + 1]) == 2) {
                i = i + 2; // Move past '//'
                while (argv[1][i] != '\n') { // Check for '\n'
                    i++;
                }
                i = i + 1; // Move past '\n'
            }

            rest_index = 0;
        }

        // Iterate and append to current token until delimiter or comment
        while (!checkDelimiter(argv[1][i]) && !checkComment(argv[1][i], argv[1][i + 1]) && argv[1][i] != '\0') {
            if (strlen(cToken) + 1 < cTokenMaxLen) { // Check if enough memory
                strncat(cToken, &argv[1][i], 1);
            } else { // Reallocate memory
                cToken = (char *) realloc(cToken, cTokenMaxLen * 2 * sizeof(char));
                cTokenMaxLen = cTokenMaxLen * 2;
                strncat(cToken, &argv[1][i], 1);
            }
            i++;
        }

        current_token_length = strlen(cToken);
        int startIndex = 0;
        while (current_index + 1 <= current_token_length) {
            cMatch = checkTokenType(cToken, rest_index);  // Find token type for current token
            if (current_index + 1 == current_token_length) { // Matched entire element as token
                int subTokenfullSize =
                        current_index - startIndex; // Narrow down the range on each element for recognized token
                char subTokenFull[subTokenfullSize +
                                  2]; // Char array store the matched token; extra space for index and null terminator
                memcpy(subTokenFull, &cToken[startIndex], subTokenfullSize + 1); // Copy the part of token from element
                subTokenFull[subTokenfullSize + 1] = '\0'; // Set null terminator
                printf("%s: \"%s\"\n", getTokenName(cMatch), subTokenFull); // Print
                startIndex = current_index + 1; // Move starting index for rest of element
                rest_index = rest_index + strlen(subTokenFull); // Updating rest searching position by add token length
                break;
            } else if (current_index + 1 < current_token_length) { // Matched partial token
                int subTokenSize =
                        current_index - startIndex; // Narrow down the range on each element for recognized token
                char subToken[subTokenSize +
                              2]; // Char array store the matched token; extra space for index and null terminator
                memcpy(subToken, &cToken[startIndex], subTokenSize + 1); // Copy the part of token from element
                subToken[subTokenSize + 1] = '\0'; // Set null terminator
                printf("%s: \"%s\"\n", getTokenName(cMatch), subToken); // Print
                current_index++; // Moving scanning position to next;
                startIndex = current_index; // Matching start position
                rest_index = rest_index + strlen(subToken); // Updating rest searching position by add token length
            }
        }

        current_token_length = 0; // Reset length of token for future using
        cToken[0] = '\0'; // Reset token for future using
        current_index = 0; // Reset searching position for future using

    }

    free(cToken); // Let freedom run
    return 0;
}


/**
 * Checks if a character is a delimiter
 * @param character
 * @return boolean
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
 * Checks to see if the characters are the symbol to represent the start of a comment
 * @param char1
 * @param char2
 * @return 0 = false, 1 = '/*', 2 = '//'
 */
int checkComment(char char1, char char2) {
    if (char1 == '/' && char2 == '*') {
        return 1;
    } else if (char1 == '/' && char2 == '/') {
        return 2;
    } else {
        return 0;
    }
}

/**
 * Checks to see if the string is a C keyword
 * @param keyword
 * @return boolean
 */
int isKeyword(char *keyword) {
    if (strcmp(keyword, "auto") == 0) {
        printf("C Keyword: \"%s\"\n", "auto");
        return 1;
    } else if (strcmp(keyword, "const") == 0) {
        printf("C Keyword: \"%s\"\n", "const");
        return 1;
    } else if (strcmp(keyword, "double") == 0) {
        printf("C Keyword: \"%s\"\n", "double");
        return 1;
    } else if (strcmp(keyword, "float") == 0) {
        printf("C Keyword: \"%s\"\n", "float");
        return 1;
    } else if (strcmp(keyword, "int") == 0) {
        printf("C Keyword: \"%s\"\n", "int");
        return 1;
    } else if (strcmp(keyword, "short") == 0) {
        printf("C Keyword: \"%s\"\n", "short");
        return 1;
    } else if (strcmp(keyword, "struct") == 0) {
        printf("C Keyword: \"%s\"\n", "struct");
        return 1;
    } else if (strcmp(keyword, "unsigned") == 0) {
        printf("C Keyword: \"%s\"\n", "unsigned");
        return 1;
    } else if (strcmp(keyword, "break") == 0) {
        printf("C Keyword: \"%s\"\n", "break");
        return 1;
    } else if (strcmp(keyword, "continue") == 0) {
        printf("C Keyword: \"%s\"\n", "continue");
        return 1;
    } else if (strcmp(keyword, "else") == 0) {
        printf("C Keyword: \"%s\"\n", "else");
        return 1;
    } else if (strcmp(keyword, "for") == 0) {
        printf("C Keyword: \"%s\"\n", "for");
        return 1;
    } else if (strcmp(keyword, "long") == 0) {
        printf("C Keyword: \"%s\"\n", "long");
        return 1;
    } else if (strcmp(keyword, "signed") == 0) {
        printf("C Keyword: \"%s\"\n", "signed");
        return 1;
    } else if (strcmp(keyword, "switch") == 0) {
        printf("C Keyword: \"%s\"\n", "switch");
        return 1;
    } else if (strcmp(keyword, "void") == 0) {
        printf("C Keyword: \"%s\"\n", "void");
        return 1;
    } else if (strcmp(keyword, "case") == 0) {
        printf("C Keyword: \"%s\"\n", "case");
        return 1;
    } else if (strcmp(keyword, "default") == 0) {
        printf("C Keyword: \"%s\"\n", "default");
        return 1;
    } else if (strcmp(keyword, "enum") == 0) {
        printf("C Keyword: \"%s\"\n", "enum");
        return 1;
    } else if (strcmp(keyword, "goto") == 0) {
        printf("C Keyword: \"%s\"\n", "goto");
        return 1;
    } else if (strcmp(keyword, "register") == 0) {
        printf("C Keyword: \"%s\"\n", "register");
        return 1;
    } else if (strcmp(keyword, "sizeof") == 0) {
        printf("C Keyword: \"%s\"\n", "sizeof");
        return 1;
    } else if (strcmp(keyword, "typedef") == 0) {
        printf("C Keyword: \"%s\"\n", "typedef");
        return 1;
    } else if (strcmp(keyword, "volatile") == 0) {
        printf("C Keyword: \"%s\"\n", "volatile");
        return 1;
    } else if (strcmp(keyword, "char") == 0) {
        printf("C Keyword: \"%s\"\n", "char");
        return 1;
    } else if (strcmp(keyword, "do") == 0) {
        printf("C Keyword: \"%s\"\n", "do");
        return 1;
    } else if (strcmp(keyword, "extern") == 0) {
        printf("C Keyword: \"%s\"\n", "extern");
        return 1;
    } else if (strcmp(keyword, "if") == 0) {
        printf("C Keyword: \"%s\"\n", "if");
        return 1;
    } else if (strcmp(keyword, "return") == 0) {
        printf("C Keyword: \"%s\"\n", "return");
        return 1;
    } else if (strcmp(keyword, "static") == 0) {
        printf("C Keyword: \"%s\"\n", "static");
        return 1;
    } else if (strcmp(keyword, "union") == 0) {
        printf("C Keyword: \"%s\"\n", "union");
        return 1;
    } else if (strcmp(keyword, "while") == 0) {
        printf("C Keyword: \"%s\"\n", "while");
        return 1;
    } else {
        return 0;
    }
}

/**
 * Checks for C keyword in a 9-character string segment
 * @param input
 * @param i
 * @return boolean
 */
int checkKeywordPresent(char *input, int i) {
    int keywordCounter = i;

    char keyword[9];
    for (int j = 0; j < 9; ++j) {
        keyword[j] = '\0';
    }

    // Append each character to keyword until maximum size of C keyword or delimiter
    while (input[keywordCounter] != '\0' && keywordCounter - i <= KEYWORD_MAX_SIZE) {
        // Append character
        keyword[keywordCounter - i] = input[keywordCounter];

        // Check to see if character is a delimiter
        if (checkDelimiter(input[keywordCounter])) {
            keyword[keywordCounter - i] = '\0'; // Remove delimiter from keyword token
            if (isKeyword(keyword)) { // Check to see if keyword is a C keyword
                i = keywordCounter; // Move index of the input string
            }
            break;
        }
        keywordCounter++;
    }

    // If C keyword not found, check keyword again
    if (i != keywordCounter && isKeyword(keyword)) {
        i = keywordCounter;
    }

    return i;
}

/**
 * Returns the name of the token given a token number
 * @param tokenNum
 * @return Name of the token
 */
char *getTokenName(int tokenNum) {
    char *tokenName = NULL;
    switch (tokenNum) {
        case 0  :
            tokenName = "word";
            break;
        case 1  :
            tokenName = "decimal integer";
            break;
        case 2  :
            tokenName = "octal integer";
            break;
        case 3  :
            tokenName = "hexadecimal integer";
            break;
        case 4  :
            tokenName = "floating point";
            break;
        case 5  :
            tokenName = "left parenthesis";
            break;
        case 6  :
            tokenName = "right parenthesis";
            break;
        case 7  :
            tokenName = "left bracket";
            break;
        case 8  :
            tokenName = "right bracket";
            break;
        case 9  :
            tokenName = "structure member";
            break;
        case 10 :
            tokenName = "structure pointer";
            break;
        case 11  :
            tokenName = "sizeof";
            break;
        case 12 :
            tokenName = "comma";
            break;
        case 13  :
            tokenName = "negate";
            break;
        case 14 :
            tokenName = "1s complement";
            break;
        case 15  :
            tokenName = "shift right";
            break;
        case 16 :
            tokenName = "shift left";
            break;
        case 17  :
            tokenName = "bitwise XOR";
            break;
        case 18 :
            tokenName = "bitwise OR";
            break;
        case 19  :
            tokenName = "increment";
            break;
        case 20 :
            tokenName = "decrement";
            break;
        case 21  :
            tokenName = "addition";
            break;
        case 22 :
            tokenName = "division";
            break;
        case 23  :
            tokenName = "logical OR";
            break;
        case 24 :
            tokenName = "logical AND";
            break;
        case 25  :
            tokenName = "conditional true";
            break;
        case 26 :
            tokenName = "conditional false";
            break;
        case 27 :
            tokenName = "equality test";
            break;
        case 28 :
            tokenName = "inequality test";
            break;
        case 29 :
            tokenName = "less than test";
            break;
        case 30 :
            tokenName = "greater than test";
            break;
        case 31 :
            tokenName = "less than or equal test";
            break;
        case 32 :
            tokenName = "greater than or equal test";
            break;
        case 33 :
            tokenName = "assignment";
            break;
        case 34 :
            tokenName = "plus equals";
            break;
        case 35 :
            tokenName = "minus equals";
            break;
        case 36 :
            tokenName = "times equals";
            break;
        case 37 :
            tokenName = "divide equals";
            break;
        case 38 :
            tokenName = "mod equals";
            break;
        case 39 :
            tokenName = "shift right equals";
            break;
        case 40 :
            tokenName = "shift left equals";
            break;
        case 41 :
            tokenName = "bitwise AND equals";
            break;
        case 42 :
            tokenName = "bitwise XOR equals";
            break;
        case 43 :
            tokenName = "bitwise OR equals";
            break;
        case 44 :
            tokenName = "AND/address operator";
            break;
        case 45 :
            tokenName = "minus/subtract operator";
            break;
        case 46 :
            tokenName = "multiply/dereference operator";
            break;
    }
    return tokenName;
}

/**
 * Returns the matched token type by defined label number
 * @param token, index
 * @return tokenNum which list above
 */

int checkTokenType(char *token, int index) {
    int position = rest_index; // Starting from scanned position
    int element_length = strlen(token); // Length of current element
    int tokenNum = -1; // Default for token type
    char *hexadecimal = "1234567890abcdefABCDEF"; // Collection of hexadecimal
    char *octal = "01234567"; // Collection of octal
    char *decimal = "0123456789"; // Collection of decimal
    char *outside_octal = "89"; // Collection of numbers appear on hexadecimal without appear on octal
    char *sign = "~`!@#$%^&*()_-+=|\[]{};':,.<>/?"; // Collection of sign

    // Finite-state machine:
    // Iterating entre element
    while (position < element_length) {

        // Matched alphabet for 'word'
        if (position == rest_index && isalpha(token[position])) {

            // Matching for last letter on element
            if (position + 1 == element_length) {
                tokenNum = 0;
                break;
            }

            position++;
            int w_check = 0;

            // Only alphabet and digit are allowed in 'word'
            while (isalpha(token[position]) || isdigit(token[position])) {
                position++;
                w_check = 1;
            }

            // Precaution from failed matching but overridden position above
            if (w_check == 1) {
                position--;
            }
            current_index = position;

            // Returning type
            if (position + 1 == element_length) {
                tokenNum = 0;
                break;
            } else if (position - index > 0) {
                tokenNum = 0;
                break;
            } else {
                break;
            }
        }

        // Matched '0' as initial character, so it could end with: hexadecimal, octal, floating point, decimal
        if (position == rest_index && token[position] == '0') {
            position++;

            // Matched 'x' or 'X' as second character, so it could end with hexadecimal
            if (position == rest_index + 1 && (token[position] == 'x' || token[position] == 'X')) {
                int h_check = 0; // Precaution from failed matching but overridden position below
                position++;

                // Catching possible character follow the 'x'
                while (strchr(hexadecimal, token[position]) != NULL && position < element_length) {
                    h_check = 1;
                    position++;
                }
                position--;
                current_index = position;

                // Return matched type
                if (position + 1 == element_length && position + 1 > 1) {
                    tokenNum = 3;
                    break;
                } else if (h_check == 1) {
                    tokenNum = 3;
                    break;
                } else {
                    break;
                }
            }


            // Matched 0-7, so could end with: octal, floating point, decimal
            if (position == rest_index + 1 && strchr(octal, token[position]) != NULL && element_length > 1) {

                // Integer boolean for avoiding missed matched type
                int short_o = 0;
                int short_d = 0;
                int o_check = 0;
                int d_check = 0;
                int f_check = 0;
                int s_check = 0;
                int f_s_follow_check = 0;

                // Handle 2 characters length special case
                if (element_length == 2) {
                    if (strchr(outside_octal, token[position]) != NULL) {
                        short_d = 1;
                    } else {
                        short_o = 1;
                    }
                }

                // Only iterate when there are characters left over
                if (position + 1 < element_length) {
                    position++;
                }

                // Matching for octal
                while (strchr(octal, token[position]) != NULL && position < element_length) {
                    position++;
                    o_check = 1;
                }

                // Precaution from failed matching but overridden position above
                if (o_check == 1) {
                    position--;
                }

                // Return matched type
                if (position + 1 == element_length && o_check == 1) {
                    tokenNum = 2;
                    current_index = position;
                    break;
                }

                // Return as octal when it mismatched from letter
                if (isalpha(token[position + 1])) {
                    current_index = position;
                    tokenNum = 2;
                    break;
                }

                // Return as octal when it mismatched from sign
                if (strchr(sign, token[position + 1]) != NULL) {
                    current_index = position;
                    tokenNum = 2;
                    break;
                }

                // Matching for decimal number
                while (strchr(decimal, token[position]) != NULL && position < element_length) {
                    position++;
                    d_check = 1;
                }

                // Precaution from failed matching but overridden position above
                if (d_check == 1) {
                    position--;
                }
                if (position + 1 == element_length) {
                    tokenNum = 1;
                    current_index = position;
                    break;
                }
                current_index = position;

                // Matching for floating point
                while (token[position] == '.' && strchr(decimal, token[position + 1]) != NULL) {
                    position++;
                    tokenNum = 4;
                    f_check = 1;

                    // Matching after '.'
                    while (strchr(decimal, token[position]) != NULL && position < element_length) {
                        position++;

                        // Matching for scientific notation
                        if (token[position] == 'e') {
                            if (s_check == 0) {
                                if ((token[position + 1] == '-' && strchr(decimal, token[position + 2]) != NULL) ||
                                    strchr(decimal, token[position + 1]) != NULL) {
                                    s_check = 1;
                                    position = position + 2;
                                    tokenNum = 4;
                                    while (strchr(decimal, token[position]) != NULL && position < element_length) {
                                        position++;
                                        f_s_follow_check = 1;
                                    }

                                    // Precaution from failed matching but overridden position above
                                    if (f_s_follow_check == 1) {
                                        position--;
                                    }
                                    break;
                                } else {
                                    break;
                                    f_check = 0;
                                }
                            } else { break; }
                        }
                    }
                    if (s_check == 0) { position--; }
                    current_index = position;
                }

                // Return type for token
                if (position + 1 == element_length && element_length > 1) {
                    if (f_check == 1) {
                        tokenNum = 4;
                    } else if (d_check == 1 && short_o == 0) {
                        tokenNum = 1;
                    } else if (short_d == 0) {
                        tokenNum = 2;
                    }
                    break;
                } else if (f_check == 1) {
                    tokenNum = 4;
                    break;
                } else if (d_check == 1) {
                    tokenNum = 1;
                    break;
                } else if (o_check == 1) {
                    tokenNum = 2;
                    break;
                } else if (short_o == 1) {
                    tokenNum = 2;
                    break;
                } else if (short_d == 1) {
                    tokenNum = 1;
                    break;
                } else {
                    break;
                }


            }

            // Matching decimal
            if (position == rest_index + 1 && strchr(decimal, token[position]) != NULL) {
                int d_check = 0;

                if (position + 1 < element_length) { position++; }

                // Iterating for digit
                while (strchr(decimal, token[position]) != NULL && position < element_length) {
                    position++;
                    d_check = 1;
                }

                // Precaution from failed matching but overridden position above
                if (d_check == 1) {
                    position--;
                }
                current_index = position;

                // Return token type
                if (position + 1 == element_length) {
                    tokenNum = 1;
                    break;
                } else if (d_check == 1) {
                    tokenNum = 1;
                    break;
                }

            }


            // Matching for floating number
            if (token[position] == '.' && strchr(decimal, token[position + 1]) != NULL) {
                int checked = 0;
                position++;
                tokenNum = 4;
                while (strchr(decimal, token[position]) != NULL && position < element_length) {
                    position++;

                    // Matching for scientific notation
                    if (token[position] == 'e') {
                        if (checked == 0 && position + 1 < element_length) {
                            if ((token[position + 1] == '-' && strchr(decimal, token[position + 2]) != NULL) ||
                                strchr(decimal, token[position + 1]) != NULL) {
                                checked = 1;
                                position = position + 2;
                                tokenNum = 4;
                                while (strchr(decimal, token[position]) != NULL && position < element_length) {
                                    position++;
                                }
                                position--;
                                break;
                            } else { break; }
                        } else { break; }
                    }
                }
                if (checked == 0) { position--; }
                current_index = position;

                // Return toke type
                if (position + 1 == element_length) {
                    tokenNum = 4;
                    break;
                } else {
                    break;
                }
            }

            // Special case for 1 digit as decimal integer
            if (element_length == 1) {
                tokenNum = 1;
                current_index = 0;
                break;
            }

        }

        // Matching decimal and floating
        if (position == rest_index && strchr(decimal, token[position]) != NULL && token[position] != '0') {
            int decimal_check = 0;
            position++;
            while (strchr(decimal, token[position]) != NULL && position < element_length) {
                decimal_check = 1;
                position++;
            }

            // Precaution from failed matching but overridden position above
            if (decimal_check == 1) { position--; }
            if (position >= element_length) { position--; }

            // Return as octal when it mismatched from letter
            if (isalpha(token[position])) {
                current_index = --position;
                tokenNum = 1;
                break;
            }

            // Return as octal when it mismatched from letter when searching position passed
            if (isalpha(token[position + 1])) {
                current_index = position;
                tokenNum = 1;
                break;
            }

            current_index = position;

            // Return token type
            if (position + 1 == element_length) {
                tokenNum = 1;
                break;
            } else if (decimal_check == 1 && token[position + 1] != '.') {
                tokenNum = 1;
                break;
            } else if (token[position + 1] == '.' && token[position] != '.') {
                position++;
            }

            // Matching for possible floating number
            if (token[position] == '.' && strchr(decimal, token[position + 1]) != NULL &&
                position + 1 < element_length) {
                int checked = 0;
                position++;
                tokenNum = 4;
                while (strchr(decimal, token[position]) != NULL && position < element_length) {
                    position++;

                    // Matching for scientific notation
                    if (token[position] == 'e') {
                        if (checked == 0 && position + 1 < element_length) {
                            if ((token[position + 1] == '-' && strchr(decimal, token[position + 2]) != NULL) ||
                                strchr(decimal, token[position + 1]) != NULL) {
                                checked = 1;
                                position = position + 2;
                                tokenNum = 4;
                                while (strchr(decimal, token[position]) != NULL && position < element_length) {
                                    position++;
                                }
                                position--;
                                break;
                            } else { break; }
                        } else { break; }
                    }
                }
                if (checked == 0) { position--; }
                current_index = position;

                // Return token type
                if (position + 1 == element_length) {
                    tokenNum = 4;
                    break;
                } else {
                    break;
                }
            } else if (token[position] == '.') {
                position--;
                current_index = position;
                tokenNum = 1;
                break;
            }
        }
        break;
    }


    while (1) {
        // Matching 3 space C operator
        if (token[current_index] == '<' && token[current_index + 1] == '<' && token[current_index + 2] == '=') {
            tokenNum = 40;
            current_index = current_index + 2;
            break;
        }

        if (token[current_index] == '>' && token[current_index + 1] == '>' && token[current_index + 2] == '=') {
            tokenNum = 39;
            current_index = current_index + 2;
            break;
        }

        // Matching 2 space C operator
        if (token[current_index] == '+' && token[current_index + 1] == '=') {
            tokenNum = 34;
            current_index++;
            break;
        }

        if (token[current_index] == '-' && token[current_index + 1] == '=') {
            tokenNum = 35;
            current_index++;
            break;
        }

        if (token[current_index] == '*' && token[current_index + 1] == '=') {
            tokenNum = 36;
            current_index++;
            break;
        }

        if (token[current_index] == '/' && token[current_index + 1] == '=') {
            tokenNum = 37;
            current_index++;
            break;
        }

        if (token[current_index] == '%' && token[current_index + 1] == '=') {
            tokenNum = 38;
            current_index++;
            break;
        }

        if (token[current_index] == '<' && token[current_index + 1] == '=') {
            tokenNum = 31;
            current_index++;
            break;
        }

        if (token[current_index] == '>' && token[current_index + 1] == '=') {
            tokenNum = 32;
            current_index++;
            break;
        }
        if (token[current_index] == '=' && token[current_index + 1] == '=') {
            tokenNum = 27;
            current_index++;
            break;
        }

        if (token[current_index] == '!' && token[current_index + 1] == '=') {
            tokenNum = 28;
            current_index++;
            break;
        }

        if (token[current_index] == '|' && token[current_index + 1] == '|') {
            tokenNum = 23;
            current_index++;
            break;
        }

        if (token[current_index] == '&' && token[current_index + 1] == '&') {
            tokenNum = 24;
            current_index++;
            break;
        }
        if (token[current_index] == '+' && token[current_index + 1] == '+') {
            tokenNum = 19;
            current_index++;
            break;
        }

        if (token[current_index] == '-' && token[current_index + 1] == '-') {
            tokenNum = 20;
            current_index++;
            break;
        }

        if (token[current_index] == '>' && token[current_index + 1] == '>') {
            tokenNum = 15;
            current_index++;
            break;
        }

        if (token[current_index] == '<' && token[current_index + 1] == '<') {
            tokenNum = 16;
            current_index++;
            break;
        }

        if (token[current_index] == '-' && token[current_index + 1] == '>') {
            tokenNum = 10;
            current_index++;
            break;
        }

        if (token[current_index] == '&' && token[current_index + 1] == '=') {
            tokenNum = 41;
            current_index++;
            break;
        }

        if (token[current_index] == '^' && token[current_index + 1] == '=') {
            tokenNum = 42;
            current_index++;
            break;
        }

        if (token[current_index] == '|' && token[current_index + 1] == '=') {
            tokenNum = 43;
            current_index++;
            break;
        }

        // Matching 1 space C operator
        if (token[current_index] == '(') {
            tokenNum = 5;
            break;
        }

        if (token[current_index] == ')') {
            tokenNum = 6;
            break;
        }

        if (token[current_index] == '[') {
            tokenNum = 7;
            break;
        }

        if (token[current_index] == ']') {
            tokenNum = 8;
            break;
        }

        if (token[current_index] == '.') {
            tokenNum = 9;
            break;
        }

        // Matching 'sizeof'
        if (element_length == 6 && strcmp(token, "sizeof") == 0) {
            tokenNum = 11;
            break;
        }

        if (token[current_index] == ',') {
            tokenNum = 12;
            break;
        }

        if (token[current_index] == '!') {
            tokenNum = 13;
            break;
        }

        if (token[current_index] == '~') {
            tokenNum = 14;
            break;
        }

        if (token[current_index] == '^') {
            tokenNum = 17;
            break;
        }

        if (token[current_index] == '|') {
            tokenNum = 18;
            break;
        }

        if (token[current_index] == '+') {
            tokenNum = 21;
            break;
        }

        if (token[current_index] == '/') {
            tokenNum = 22;
            break;
        }

        if (token[current_index] == '?') {
            tokenNum = 25;
            break;
        }

        if (token[current_index] == ':') {
            tokenNum = 26;
            break;
        }

        if (token[current_index] == '<') {
            tokenNum = 29;
            break;
        }

        if (token[current_index] == '>') {
            tokenNum = 30;
            break;
        }

        if (token[current_index] == '=') {
            tokenNum = 33;
            break;
        }

        if (token[current_index] == '&') {
            tokenNum = 44;
            break;
        }

        if (token[current_index] == '-') {
            tokenNum = 45;
            break;
        }

        if (token[current_index] == '*') {
            tokenNum = 46;
            break;
        }
        break; // Precaution from mismatch
    }

    return tokenNum;
}
