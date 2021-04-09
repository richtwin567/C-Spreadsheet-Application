#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

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
 * @brief Trims the whitespaces on the left side of a string.
 * 
 * @param string the string to trim
 * @return char* the trimmed string
 */
char* ltrim (char* string)
{
    char * start = string;
    char* newString;
    while(isspace(*start)){
        start++;
    }
    int len = start- string ;
    newString=malloc(strlen(string)-len+1);
    strcpy(newString,start);
    memset(string, 0, sizeof string);
    free(string);
    return newString;
}

/**
 * @brief Trims the whitespaces on the right side of a string. 
 * 
 * @param string the string to trim
 * @return char* the trimmed string
 */
char* rtrim (char* string)
{
    int len = strlen(string);

    char *end = string +len-1;
    char* newString;
    while (isspace(*end))
    {
        end--;
    }
    end++;
    len = end- string;
    newString = malloc(len+1);
    strncpy(newString, string, len);
    newString[len]='\0';
    memset(string, 0, sizeof string);
    free(string);
    return newString;
}

/**
 * @brief Trims the whitespaces on both sides of a string.  
 * 
 * @param string the string to trim
 * @return char* the trimmed string
 */
char* trim (char* string)
{
    return ltrim(rtrim(string));
}

#endif
