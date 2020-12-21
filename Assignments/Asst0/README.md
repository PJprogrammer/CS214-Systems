# CS214 Assignment 0
## Description
Our program takes a string from the command line arguments and tokenizes it. The several types of tokens that the program
recognizes are word, decimal integer, octal integer, hexadecimal integer, floating point, C keywords, and C operators. Additionally, the program will ignore C style comments and use delimiters such as ' ', '\t' and '\n' to separate tokens.

### Efficiency
Time Complexity: O(n)<br/>
Space Complexity (Worst Case): O(n)

The worst case for space complexity occurs when the entire string of size n is a single type of token.

### Process
The first implementation of this program we created had a running time of O(n^2) since we did not have any mechanism of keeping track of characters we had seen previously. However, in the second implmentation of this program, we modeled our program after a finite state machine. By using this type of logic, we were able to create an algorithm that successfully tokenizes a string in O(n).

## Running
```
# Run program
C> gcc tokenizer.c "123abc 3.1"
C> ./a.out
decimal integer: "123"
word: "abc"
floating point: "3.1"
```

Find a full list of test cases in 'testcases.txt'
