#ifndef _DIALOG_H_
#define _DIALOG_H_

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
#include "../interface/message.h"

#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Prompts the user to enter a set of coordinates on the spreadsheet 
 * 
 * @return struct SheetCoord the coordinates entered
 */
struct SheetCoord promptForCell(int *shouldReturnToMenu)
{
    char *col   = malloc(2);
    char *row   = malloc(2);
    char *input = malloc(5);
    int read    = 0;
    struct SheetCoord coords;

    do
    {
        printf("\nPlease enter the coordinates in cr format where c is the column letter and r is the row number (eg. E8) or enter 'menu' to return the menu: ");

        scanf("%s", input);
        flushStdin();

        toCaps(input);

        if (strcmp(input, "MENU") == 0)
        {
            coords.col          = 'A';
            coords.row          = -1;
            *shouldReturnToMenu = 1;
            free(col);
            free(row);
            free(input);
            return coords;
        }
        else
        {
            *shouldReturnToMenu = 0;
        }

        read = sscanf(input, "%1[a-zA-Z]%1[1-9]", col, row);

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
    free(input);

    return coords;
}

/**
 * @brief Prompt the user for data to enter into a cell 
 * 
 * @return char* the data entered
 */
char *promptForData(int *shouldReturnToMenu)
{
    //TODO pointer check
    char *data = calloc(1, sizeof *data);
    char *input;
    char c;
    int i = 0;

    printf("\nPlease enter the content for the cell or 'menu' to return to the menu: ");
    while ((c = getchar()) != '\n')
    {
        data[i] = c;
        i++;
        data = realloc(data, (i + 1) * sizeof *data);
    }
    data[i] = '\0';

    input = malloc(i + 1);
    strcpy(input, data);

    toCaps(input);

    if (strcmp(input, "MENU") == 0)
    {
        *shouldReturnToMenu = 1;
        free(input);
        free(data);
        return NULL;
    }
    else
    {
        *shouldReturnToMenu = 0;
    }

    return data;
}

/**
 * @brief Prints an error message with the code if given
 * 
 * @param err the error message
 * @param code the error code
 */
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

/**
 * @brief Prints an informative message 
 * 
 * @param info the message to print
 */
void printInfoMsg(char *info)
{
    printf("\n\033[0;34m%s\033[0m\n", info);
}

/**
 * @brief Prints a warning message
 * 
 * @param warning the warning to print
 */
void printWarningMsg(char *warning)
{
    printf("\n\033[0;33m%s\033[0m\n", warning);
}

/**
 * @brief Prints a message indicating that something was completed succesfully
 * 
 * @param msg the message to print
 */
void printSuccessMsg(char *msg)
{
    printf("\n\033[0;32m%s\033[0m\n", msg);
}

/**
 * @brief Displays the menu and prompts the user to choose
 * 
 * @return int the choice the user made
 */
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

/**
 * @brief Print a message from the server in a colour based on the code
 * 
 * @param msg the server msg
 */
void printMsgFromCode(struct ServerMessage msg)
{
    switch (msg.header.code)
    {
        case ACKNOWLEDGED:
            break;

        case OK:
            printSuccessMsg(msg.message == NULL ? "Spreadsheet updated successfully." : msg.message);
            break;
        case BAD_SYNTAX:
            printErrorMsg(msg.message == NULL ? "There is a syntax error in your input. Please check it and try again." : msg.message, &msg.header.code);
            break;
        case FORBIDDEN:
            printErrorMsg(msg.message == NULL ? "You are not authorized to execute this task." : msg.message, &msg.header.code);
            break;
        case COORD_NOT_FOUND:
            printErrorMsg(msg.message == NULL ? "One or more of the requested coordinates do not exist on the sheet." : msg.message, &msg.header.code);
            break;
        case CONFLICT:
            printWarningMsg(msg.message == NULL ? "The board has been changed while you were working on it" : msg.message);
            break;
        case DISCONNECTED:
            printWarningMsg(msg.message == NULL ? "You have been disconnected from the server." : msg.message);
            break;
        case IMPOSSIBLE:
            printErrorMsg(msg.message == NULL ? "The server could not completet the calculation. Check your input cells." : msg.message, &msg.header.code);
            break;
        case NO_FUNCTION:
            printErrorMsg(msg.message == NULL ? "The server does not support the requested function." : msg.message, &msg.header.code);
            break;
        case CONN_REJECTED:
            printErrorMsg(msg.message == NULL ? "The server has rejected the connection due to it being at max connection capacity." : msg.message, &msg.header.code);
            break;

        default:
            printInfoMsg(msg.message);
            break;
    }
}
#endif
