/**
 * @file functions.h
 * @author 
 * @brief Contains definitions for all the spreadsheet functions the server supports
 * @version 0.1
 * @date 2021-03-22
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef _FUNCTIONS_H_
#define _FUNCTIONS_H_
#include "../interface/code.h"
#include "../spreadsheet/spreadsheet.h"
#include "../utils/utils.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief The different types of commands the client may send
 * 
 */
enum CommandType
{
    ARITHMETIC, // Commands that can be written as an arithmetic expression eg. (A5+A6)/B4
    PLACEMENT,  // A command to simply place a value in the cell ( no calculation )
    OTHER       // A command that does not fit into the other categories . It may be combination of logical and arithmetic or neither. eg. RANGE
};

struct CommandInfo
{
    enum CommandType type; // the type of command
    // the arguments for the command
    union args
    {
        char *expr;      // Used for LOGICAL, ARITHMETIC and PLACEMENT
        char **argsList; // Used for OTHER
    } args;
    char *funcName; // the name of the function
};

// implemented functions
char *ARITHMETIC_FUNC[] = {"AVERAGE", "SUM"};
char *OTHER_FUNC[]      = {"RANGE"};

/**
 * @brief Finds the minimum and maximum of the given array
 * 
 * @param arr The array
 * @param min To be assigned the minimum number
 * @param max to be assigned the maximum number
 * @param length the number of items in the array
 */
void minMax(double *arr, double *min, double *max, int length)
{
    if (length > 0)
    {
        *max = arr[0];
        *min = arr[0];

        int i;

        for (i = 0; i < length; i++)
        {
            if (arr[i] > *max)
            {
                *max = arr[i];
            }
            if (arr[i] < *min)
            {
                *min = arr[i];
            }
        }
    }
    else
    {
        *min = 0;
        *max = 0;
    }
}


/**
 * @brief Checks if the argument has a valid cell format. 
 * 
 * @param arg the arg to check
 * @return int 1 if the arg is valid and 0 otherwise
 */
int isValidArg(char *arg)
{
    char *col = malloc(2 * (sizeof *col));
    char *row = malloc(2 * (sizeof *row));
    int read  = sscanf(arg, "%1[a-zA-Z]%1[1-9]", col, row);
    free(col);
    free(row);
    return read == 2;
}

/**
 * @brief Try to parse and calculate the SUM and AVERAGE functions    
 * 
 * @param cmd the command from the client
 * @param cmdi information about the command
 * @param code the return code
 * @param sheet the sheet to modify
 * @return int 1 if successful and 0 otherwise
 */
int tryParseArthimetic(struct Command *cmd, struct CommandInfo *cmdi, enum Code *code, struct Sheet *sheet)
{
    int arglen       = 0;
    int rangelen     = 0;
    char *valStr     = NULL;
    char *arg        = malloc(1);
    char *argStart   = NULL;
    char *argEnd     = NULL;
    char *range      = malloc(1);
    char *rangeEnd   = NULL;
    char *rangeStart = NULL;
    char *operator   = "+";
    char *next       = NULL;
    char *input      = cmd->input;
    char *funcName   = cmdi->funcName;
    cmdi->args.expr  = calloc(1, 1);
    struct SheetCoord coords;
    double val;
    double res = 0;
    int read;
    input++;
    argStart = input;

    if (funcName == NULL)
    {
        *code = NO_FUNCTION;
        return 0;
    }

    while (*input != '\0')
    {
        // commented out an implementation that distinguished between , and : in the function args

        /* if (*input == ',')
        {
            argEnd = input;
            arglen = argEnd - argStart;

            if (arglen < 2)
            {
                *code = BAD_SYNTAX;
                return 0;
            }

            arg = realloc(arg, arglen + 1);
            strncpy(arg, argStart, arglen);
            arg[arglen] = '\0';

            arg = trim(arg);

            if (!isValidArg(arg))
            {
                *code = BAD_SYNTAX;
                return 0;
            }

            coords.col = arg[0];
            coords.row = arg[1] - '0';

            if (!isOnSheet(*sheet, coords))
            {
                *code = COORD_NOT_FOUND;
                return 0;
            }

            if (isPositionEmpty(*sheet, coords))
            {
                input++;
                continue;
            }

            valStr = getPosition(*sheet, coords);
            read   = sscanf(valStr, "%lf", &val);

            if (read != 1)
            {
                input++;
                continue;
            }

            res += val;

            cmdi->args.expr = realloc(cmdi->args.expr, 1 + strlen(valStr) + strlen(cmdi->args.expr));
            if (strlen(cmdi->args.expr) > 0)
            {
                strcat(cmdi->args.expr, operator);
            }
            strcat(cmdi->args.expr, arg);
            argStart = ++argEnd;
        } 
        else*/
        if (*input == ',' || *input == ':')
        {
            argEnd = input;
            arglen = argEnd - argStart;

            if (arglen < 2)
            {
                *code = BAD_SYNTAX;
                return 0;
            }

            arg = realloc(arg, arglen + 1);
            memset(arg, 0, sizeof arg);
            strncpy(arg, argStart, arglen);
            arg[arglen] = '\0';

            arg = trim(arg);

            if (!isValidArg(arg))
            {
                *code = BAD_SYNTAX;
                return 0;
            }
            coords.col = arg[0];
            coords.row = arg[1] - '0';

            /* if (!isOnSheet(*sheet, coords))
            {
                *code = COORD_NOT_FOUND;
                return 0;
            } */

            /*if (isPositionEmpty(*sheet, coords))
            {
                *code = IMPOSSIBLE;
                return 0;
            }

            valStr = getPosition(*sheet, coords);
            read   = sscanf(valStr, "%lf", &val);

            if (read != 1)
            {
                *code = IMPOSSIBLE;
                return 0;
            }
            free(valStr); */
            
            // find where the range ends
            rangeStart = ++input;

            while (*input != '\0' && *input != ')' && *input != ',')
            {
                input++;
            }

            if (*input == '\0')
            {
                *code = BAD_SYNTAX;
                return 0;
            }
            rangeEnd = input;
            rangelen = rangeEnd - rangeStart;

            if (rangelen < 2)
            {
                *code = BAD_SYNTAX;
                return 0;
            }

            range = realloc(range, rangelen + 1);
            strncpy(range, rangeStart, rangelen);
            range[rangelen] = '\0';

            range = trim(range);
            if (!isValidArg(range))
            {
                *code = BAD_SYNTAX;
                return 0;
            }

            /* if (isPositionEmpty(*sheet, coords))
            {
                *code = IMPOSSIBLE;
                return 0;
            }

            valStr = getPosition(*sheet, coords);
            read   = sscanf(valStr, "%lf", &val);

            if (read != 1)
            {
                *code = IMPOSSIBLE;
                return 0;
            }

            free(valStr); */

            if (arg[0] == range[0] && arg[1] < range[1])
            {
                // add the values where applicable as long as the coordinate exists on the sheet
                while (arg[1] <= range[1])
                {
                    coords.col = arg[0];
                    coords.row = arg[1] - '0';

                    if (!isOnSheet(*sheet, coords))
                    {
                        *code = COORD_NOT_FOUND;
                        return 0;
                    }

                    if (isPositionEmpty(*sheet, coords))
                    {
                        arg[1] = arg[1] + 1;
                        continue;
                    }
                    valStr = getPosition(*sheet, coords);
                    read   = sscanf(valStr, "%lf", &val);

                    if (read != 1)
                    {
                        arg[1] = arg[1] + 1;
                        continue;
                    }
                    res += val;

                    cmdi->args.expr = realloc(cmdi->args.expr, 1 + strlen(valStr) + strlen(cmdi->args.expr));

                    if (strlen(cmdi->args.expr) > 0)
                    {
                        strcat(cmdi->args.expr, operator);
                    }
                    strcat(cmdi->args.expr, arg);
                    arg[1] = arg[1] + 1;
                }
                argStart = rangeEnd + 1;
            }
            else if (arg[0] < range[0] && arg[1] == range[1])
            {
                while (arg[0] <= range[0])
                {
                    coords.col = arg[0];
                    coords.row = arg[1] - '0';

                    if (!isOnSheet(*sheet, coords))
                    {
                        *code = COORD_NOT_FOUND;
                        return 0;
                    }

                    if (isPositionEmpty(*sheet, coords))
                    {
                        arg[0] = arg[0] + 1;
                        continue;
                    }
                    valStr = getPosition(*sheet, coords);
                    read   = sscanf(valStr, "%lf", &val);

                    if (read != 1)
                    {
                        arg[0] = arg[0] + 1;
                        continue;
                    }

                    res += val;
                    cmdi->args.expr = realloc(cmdi->args.expr, 1 + strlen(valStr) + strlen(cmdi->args.expr));

                    if (strlen(cmdi->args.expr) > 0)
                    {
                        strcat(cmdi->args.expr, operator);
                    }
                    strcat(cmdi->args.expr, arg);
                    arg[0] = arg[0] + 1;
                }
                argStart = rangeEnd + 1;
            }
            else
            {
                // only increasing ranges supported
                *code = BAD_SYNTAX;
                return 0;
            }
        }

        input++;
    }

    input--;
    //ensure the end of the function was closed by a bracket otherwise BAD_SYNTAX
    if (*input == ')')
    {
        argEnd = input;
        arglen = argEnd - argStart;

        // double check to ensure there were no unread args
        if (arglen >= 2)
        {

            arg = realloc(arg, arglen + 1);
            strncpy(arg, argStart, arglen);
            arg[arglen] = '\0';

            arg = trim(arg);

            if (!isValidArg(arg))
            {
                *code = BAD_SYNTAX;
                return 0;
            }
            coords.col = arg[0];
            coords.row = arg[1] - '0';

            if (!isOnSheet(*sheet, coords))
            {
                *code = COORD_NOT_FOUND;
                return 0;
            }

            if (!isPositionEmpty(*sheet, coords))
            {

                valStr = getPosition(*sheet, coords);
                valStr = trim(valStr);
                read   = sscanf(valStr, "%lf", &val);

                if (read != 1)
                {
                    *code = IMPOSSIBLE;
                    return 0;
                }

                res += val;
                cmdi->args.expr = realloc(cmdi->args.expr, 1 + strlen(valStr) + strlen(cmdi->args.expr));
                if (strlen(cmdi->args.expr) > 0)
                {
                    strcat(cmdi->args.expr, operator);
                }
                strcat(cmdi->args.expr, arg);
                free(valStr);
            }
        }
    }
    else
    {
        *code = BAD_SYNTAX;
        return 0;
    }

    // divide py the number of oprands if average was the requested function
    if (strcmp(funcName, "AVERAGE") == 0)
    {
        int count = 1;
        next      = strchr(cmdi->args.expr, '+');
        while (next != NULL)
        {
            next = strchr(next + 1, '+');
            count++;
        }
        res /= count;
    }
    *code = OK;

    if (arg != NULL)
    {
        free(arg);
    }

    if (range != NULL)
    {
        free(range);
    }

    // once no errors have been encountered, update the sheet
    placeNumber(sheet, cmd->coords, res);
    return 1;
}

/**
 * @brief Calculates the range of the given cells
 * 
 * @param cmd the command
 * @param cmdi the command information
 * @param code the code
 * @param sheet the sheet to be updated
 * @return int 1 if successful 0 otherwise
 */
int tryParseRange(struct Command *cmd, struct CommandInfo *cmdi, enum Code *code, struct Sheet *sheet)
{
    double *rangearr;
    rangearr         = malloc(sizeof(double) * 100);
    int index        = 0;
    char *argStart   = NULL;
    char *argEnd     = NULL;
    char *arg        = malloc(1);
    char *valStr     = NULL;
    int arglen       = 0;
    int rangelen     = 0;
    char *range      = malloc(1);
    char *rangeEnd   = NULL;
    char *rangeStart = NULL;
    char *operator   = NULL;
    char *countStr   = NULL;
    char *next       = NULL;
    char *input      = cmd->input;
    char *funcName   = cmdi->funcName;
    cmdi->args.expr  = calloc(1, 1);
    struct SheetCoord coords;
    double val;
    double res;
    int read;
    input++;
    argStart = input;

    while (*input != '\0')
    {
        if (*input == ',' || *input == ':')
        {
            argEnd = input;
            arglen = argEnd - argStart;

            if (arglen < 2)
            {
                *code = BAD_SYNTAX;
                return 0;
            }

            arg = realloc(arg, arglen + 1);
            memset(arg, 0, sizeof arg);
            strncpy(arg, argStart, arglen);
            arg[arglen] = '\0';

            arg = trim(arg);

            if (!isValidArg(arg))
            {
                *code = BAD_SYNTAX;
                return 0;
            }
            coords.col = arg[0];
            coords.row = arg[1] - '0';

            rangeStart = ++input;

            while (*input != '\0' && *input != ')' && *input != ',')
            {
                input++;
            }

            if (*input == '\0')
            {
                *code = BAD_SYNTAX;
                return 0;
            }
            rangeEnd = input;
            rangelen = rangeEnd - rangeStart;

            if (rangelen < 2)
            {
                *code = BAD_SYNTAX;
                return 0;
            }

            range = realloc(range, rangelen + 1);
            strncpy(range, rangeStart, rangelen);
            range[rangelen] = '\0';

            range = trim(range);
            if (!isValidArg(range))
            {
                *code = BAD_SYNTAX;
                return 0;
            }

            if (arg[0] == range[0] && arg[1] < range[1])
            {

                while (arg[1] <= range[1])
                {
                    coords.col = arg[0];
                    coords.row = arg[1] - '0';

                    if (!isOnSheet(*sheet, coords))
                    {
                        *code = COORD_NOT_FOUND;
                        return 0;
                    }

                    if (isPositionEmpty(*sheet, coords))
                    {
                        arg[1] = arg[1] + 1;
                        continue;
                    }
                    valStr = getPosition(*sheet, coords);
                    read   = sscanf(valStr, "%lf", &val);

                    if (read != 1)
                    {
                        arg[1] = arg[1] + 1;
                        continue;
                    }

                    rangearr[index] = val;
                    index++;
                    arg[1] = arg[1] + 1;
                }
                argStart = rangeEnd + 1;
            }
            else if (arg[0] < range[0] && arg[1] == range[1])
            {
                while (arg[0] <= range[0])
                {
                    coords.col = arg[0];
                    coords.row = arg[1] - '0';

                    if (!isOnSheet(*sheet, coords))
                    {
                        *code = COORD_NOT_FOUND;
                        return 0;
                    }

                    if (isPositionEmpty(*sheet, coords))
                    {
                        arg[0] = arg[0] + 1;
                        continue;
                    }
                    valStr = getPosition(*sheet, coords);
                    read   = sscanf(valStr, "%lf", &val);

                    if (read != 1)
                    {
                        arg[0] = arg[0] + 1;
                        continue;
                    }
                    rangearr[index] = val;
                    index++;
                    arg[0] = arg[0] + 1;
                }
                argStart = rangeEnd + 1;
            }
            else
            {
                *code = BAD_SYNTAX;
                return 0;
            }
        }
        input++;
    }

    input--;
    if (*input == ')')
    {
        argEnd = input;
        arglen = argEnd - argStart;

        if (arglen >= 2)
        {

            arg = realloc(arg, arglen + 1);
            strncpy(arg, argStart, arglen);
            arg[arglen] = '\0';

            arg = trim(arg);

            if (!isValidArg(arg))
            {
                *code = BAD_SYNTAX;
                return 0;
            }
            coords.col = arg[0];
            coords.row = arg[1] - '0';

            if (!isOnSheet(*sheet, coords))
            {
                *code = COORD_NOT_FOUND;
                return 0;
            }

            if (!isPositionEmpty(*sheet, coords))
            {

                valStr = getPosition(*sheet, coords);
                read   = sscanf(valStr, "%lf", &val);

                if (read != 1)
                {
                    *code = IMPOSSIBLE;
                    return 0;
                }

                rangearr[index] = val;
                index++;
            }
        }
    }
    else
    {
        *code = BAD_SYNTAX;
        return 0;
    }

    if (strcmp(funcName, "RANGE") == 0)
    {
        double min, max;
        minMax(rangearr, &min, &max, index);
        res = max - min;
    }
    *code = OK;

    if (arg != NULL)
    {
        free(arg);
    }

    if (range != NULL)
    {
        free(range);
    }

    free(rangearr);
    placeNumber(sheet, cmd->coords, res);
    return 1;
}

/**
 * @brief Parses a command sent by the client into an arithmetic expression
 * 
 * @param expr The string to store the parsed command in if successful
 * @param code Set to OK if successful or BAD_SYNTAX, NO_FUNCTION or IMPOSSIBLE depending on the error
 */
void parseCommand(struct Command *cmd, enum Code *code, struct Sheet *sheet)
{
    struct CommandInfo cmdi;
    int read;
    double val;

    if (!isOnSheet(*sheet, cmd->coords))
    {
        *code = COORD_NOT_FOUND;
        return;
    }

    if (*cmd->input == '=')
    {

        cmd->input++;
        char *funcStart = cmd->input;
        char *funcEnd   = NULL;

        // try to determine if a function is being called by the presence of a (
        while (*cmd->input != '\0')
        {
            if (*cmd->input == '(')
            {
                funcEnd = cmd->input;
                break;
            }
            cmd->input++;
        }


        if (funcEnd == NULL)
        {
            *code = NO_FUNCTION;
            return;
        }
        else
        {
            // try to find out what type of function

            int len    = funcEnd - funcStart;
            char *name = malloc(len + 1);
            strncpy(name, funcStart, len);
            name[len] = '\0';

            toCaps(name);

            int isImplemented = 0;
            int len_arith     = sizeof(ARITHMETIC_FUNC) / sizeof(ARITHMETIC_FUNC[0]);
            int len_other     = sizeof(OTHER_FUNC) / sizeof(OTHER_FUNC[0]);

            // check if it is arithmetic
            for (int i = 0; i < len_arith; i++)
            {
                if (strcmp(ARITHMETIC_FUNC[i], name) == 0)
                {
                    isImplemented = 1;
                    cmdi.type     = ARITHMETIC;
                    cmdi.funcName = ARITHMETIC_FUNC[i];
                    break;
                }
            }

            if (!isImplemented)
            {
                for (int i = 0; i < len_other; i++)
                {
                    if (strcmp(OTHER_FUNC[i], name) == 0)
                    {
                        isImplemented = 3;
                        cmdi.type     = OTHER;
                        cmdi.funcName = OTHER_FUNC[i];
                        break;
                    }
                }
            }

            free(name);

            if (!isImplemented)
            {
                memset(&cmdi, 0, sizeof(cmdi));
                *code = NO_FUNCTION;
                return;
            }

            switch (isImplemented)
            {
                case 1:
                    if (!tryParseArthimetic(cmd, &cmdi, code, sheet))

                        memset(&cmdi, 0, sizeof(cmdi));
                    return;

                case 3:
                    tryParseRange(cmd, &cmdi, code, sheet);
                    break;

                default:
                    break;
            }
        }
    }
    else
    {
        cmdi.type = PLACEMENT;
        read      = sscanf(cmd->input, "%lf", &val);
        if (read == 1)
        {
            placeNumber(sheet, cmd->coords, val);
        }
        else
        {
            placeWord(sheet, cmd->coords, trim(cmd->input));
        }
        *code = OK;
        return;
    }
}

#endif
