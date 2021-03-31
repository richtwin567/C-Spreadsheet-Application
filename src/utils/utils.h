#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdio.h>

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

#endif
