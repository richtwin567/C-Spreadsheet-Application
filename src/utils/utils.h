
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

