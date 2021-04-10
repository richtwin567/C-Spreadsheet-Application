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
 * @brief A word to Capitalize
 * 
 * @param word 
 */
void toCaps(char *word)
{
    while (*word != '\0')
    {
        *word = toupper(*word);
        word++;
    }
}

int isValidArg(char *arg)
{
    char *col = malloc(2 * (sizeof *col));
    char *row = malloc(2 * (sizeof row));
    int read  = sscanf(arg, "%1[a-zA-Z]%1[1-9]", col, row);
    free(col);
    free(row);
    return read == 2;
}

int tryParseArthimetic(struct Command *cmd, struct CommandInfo *cmdi, enum Code *code, struct Sheet *sheet)
{
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
    cmdi->args.expr  = calloc(1,1);
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

    if (strcmp(funcName, "SUM") == 0 || strcmp(funcName, "AVERAGE") == 0)
    {
        operator= "+";
    }

    while (*input != '\0')
    {
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
            valStr = trim(valStr);
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
        if (*input == ',' || *input==':')
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
            valStr = trim(valStr);
            read   = sscanf(valStr, "%lf", &val);

            if (read != 1)
            {
                *code = IMPOSSIBLE;
                return 0;
            }
            free(valStr); */

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
            valStr = trim(valStr);
            read   = sscanf(valStr, "%lf", &val);

            if (read != 1)
            {
                *code = IMPOSSIBLE;
                return 0;
            }

            free(valStr); */

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
                    valStr = trim(valStr);
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
                    valStr = trim(valStr);
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

    if (range!=NULL)
    {
        /* code */
    }
    

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

        printf("%s\n", funcEnd);

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
                    {
                        memset(&cmdi, 0, sizeof(cmdi));
                    }
                    return;

                    break;
                case 3:

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
