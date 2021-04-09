#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdio.h>
#include <ctype.h>
#include <string.h>

/**
 * @brief Count the number of digits in a number   
 * 
 * @param number the number to count
 * @return int the number of digits in number
 */
int countDigits(long number)
{
    if (number == 0)
    {
        return 1;
    }

    int count = 0;
    while (number > 0)
    {
        number = number / 10;
        count++;
    }
    return count;
}

/**
 * @brief A function to flush the stdin stream since fflush(stdin) has undefined behaviour.
 * 
 */
void flushStdin()
{
    while (getchar() != '\n');
}

/**
 * @brief Trims the whitespaces on the left side of a string. The string must be memory allocated with malloc, realloc etc to avoid a segfault. 
 * 
 * @param string the string to trim
 */
void ltrim (char* string)
{
    char * start = string;
    while(isspace(*start)){
        start++;
    }
    memmove(string, start, strlen(start)+1);
}

/**
 * @brief Trims the whitespaces on the right side of a string. The string must be memory allocated with malloc, realloc etc to avoid a segfault. 
 * 
 * @param string the string to trim
 */
void rtrim (char* string)
{
    int len = strlen(string);

    char *end = string +len-1;
    while (isspace(*end))
    {
        end--;
    }
    *(end+1)='\0';    
}

/**
 * @brief Trims the whitespaces on both sides of a string. The string must be memory allocated with malloc, realloc etc to avoid a segfault. 
 * 
 * @param string the string to trim
 */
void trim (char* string)
{
    ltrim(string);
    rtrim(string);
}

#endif
