/**
 * @file dialog.h
 * @author 
 * @brief Functions for communicating with the user
 * @version 0.1
 * @date 2021-03-24
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "../interface/code.h"
#include "../spreadsheet/spreadsheetData.h"
#include "../utils/utils.h"

#include <stdio.h>
#include <stdlib.h>

struct SheetCoord promptForCell()
{
    char *col = malloc(2 * (sizeof *col));
    char *row = malloc(2 * (sizeof row));
    int read  = 0;
    struct SheetCoord coords;

    do
    {
        printf("\nPlease enter the coordinates in cr format where c is the column letter and r is the row number (eg. E8): ");

        read = scanf("%1[a-zA-Z]%1[1-9]", col, row);

        flushStdin();

        if (read != 2)
        {
            printf("\nInvalid input. Please re-enter\n");
        }
        else
        {
            break;
        }

    } while (1);

    coords.col = col[0];
    coords.row = atoi(row);

    free(col);
    free(row);

    return coords;
}

char *promptForData()
{
    //TODO pointer check
    char *data = calloc(1, sizeof *data);
    char c;
    int i = 0;

    printf("\nPlease enter the content for the cell: ");
    while ((c = getchar()) != '\n')
    {
        data[i] = c;
        i++;
        data = realloc(data, (i + 1) * sizeof *data);
    }
    data[i] = '\0';

    return data;
}

void printErrorMsg(char *err, enum Code *code)
{
    char *codeStr = malloc(5 * (sizeof *codeStr));
    if (code != NULL)
    {
        sprintf(codeStr, " %d", *code);
    }
    else
    {
        codeStr = "";
    }

    fprintf(stderr, "\n\033[0;31m[ERROR%s]: %s\033[0m\n", codeStr, err);
}

void printInfoMsg(char *info)
{
    printf("\n\033[0;34m%s\033[0m\n", info);
}

void printWarningMsg(char *warning)
{
    printf("\n\033[0;33m%s\033[0m\n", warning);
}

void printSuccessMsg(char *msg)
{
    printf("\n\033[0;32m%s\033[0m\n", msg);
}

int promptMenu()
{
    int choice;
    int read       = 0;
    int shouldStop = 0;
    char *temp     = malloc(2 * (sizeof *temp));

    while (shouldStop != 1)
    {
        printf("Please choose:");
        printf("\n[1]\t-\tEdit Spreadsheet");
        printf("\n[2]\t-\tSave (first user only)");
        printf("\n[0]\t-\tQUIT\n> ");

        read = scanf("%1[0-2]", temp);

        flushStdin();

        if (read != 1)
        {
            printf("\nInvalid option, please try again.\n");
        }
        else
        {
            shouldStop = 1;
        }
    }
    sscanf(temp, "%d", &choice);
    free(temp);
    return choice;
}