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
    LOGICAL,    // Commands that require logical processing eg, IF, OR and AND functions
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
char *OTHER_FUNC[]      = {"RANGE", "COUNT", "MAX", "MIN"};
char *LOGICAL_FUNC[]    = {"IF", "OR", "AND", "NOT"};

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
    return read == 2;
}

int tryParseArthimetic(char *funcName, char *input, struct CommandInfo *cmd)
{
    char *argStart = NULL;
    char *argEnd   = NULL;
    char *arg      = malloc(1);
    int arglen     = 0;
    char *operator;
    input++;
    argStart       = input;
    cmd->args.expr = malloc(1);

    if (funcName == NULL)
    {
    }

    if (strcmp(funcName, "SUM") == 0 || strcmp(funcName, "AVERAGE") == 0)
    {
        operator= "+";
    }

    while (*input != '\0')
    {
        if (*input == ',')
        {
            argEnd = input;
            arglen = argEnd - argStart;

            if (arglen < 2)
            {
                return 0;
            }

            arg = realloc(arg, arglen + 1);
            strncpy(arg, argStart, arglen);
            arg[arglen] = '\0';

            trim(arg);

            if (!isValidArg(arg))
            {
                return 0;
            }

            cmd->args.expr = realloc(cmd->args.expr, 4 + strlen(cmd->args.expr));
            if (strlen(cmd->args.expr) > 0)
            {
                strcat(cmd->args.expr, operator);
            }
            strcat(cmd->args.expr, arg);
            argStart = ++argEnd;
        }
        else if (*input == ':')
        {
            argEnd = input;
            arglen = argEnd - argStart;

            if (arglen < 2)
            {
                return 0;
            }

            arg = realloc(arg, arglen + 1);
            strncpy(arg, argStart, arglen);
            arg[arglen] = '\0';

            trim(arg);

            if (!isValidArg(arg))
            {
                return 0;
            }

            char *rangeEnd;
            char *rangeStart = ++input;
            int rangelen;
            char *range;
            while (*input != '\0' && *input != ')' && *input != ',')
            {
                input++;
            }

            if (*input == '\0')
            {
                return 0;
            }
            rangeEnd = input;
            rangelen = rangeEnd - rangeStart;

            if (rangelen < 2)
            {
                return 0;
            }

            range = malloc(1);
            range = realloc(range, rangelen + 1);
            strncpy(range, rangeStart, rangelen);
            range[rangelen] = '\0';

            trim(range);
            if (!isValidArg(range))
            {
                return 0;
            }

            if (arg[0] == range[0] && arg[1] < range[1])
            {

                while (strcmp(arg, range) != 0 && *arg != '\0')
                {
                    cmd->args.expr = realloc(cmd->args.expr, 4 + strlen(cmd->args.expr));

                    if (strlen(cmd->args.expr) > 0)
                    {
                        strcat(cmd->args.expr, operator);
                    }
                    strcat(cmd->args.expr, arg);
                    arg[1] = arg[1] + 1;
                }
                argStart= rangeStart;
            }
            else if (arg[0] < range[0] && arg[1] == range[1])
            {
                while (strcmp(arg, range) != 0 && *arg != '\0')
                {
                    cmd->args.expr = realloc(cmd->args.expr, 4 + strlen(cmd->args.expr));

                    if (strlen(cmd->args.expr) > 0)
                    {
                        strcat(cmd->args.expr, operator);
                    }
                    strcat(cmd->args.expr, arg);
                    arg[0] = arg[0] + 1;
                }
                argStart= rangeStart;
            }
            else
            {
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

        if (arglen < 2)
        {
            return 0;
        }

        arg = realloc(arg, arglen + 1);
        strncpy(arg, argStart, arglen);
        arg[arglen] = '\0';

        trim(arg);

        if (!isValidArg(arg))
        {
            return 0;
        }

        cmd->args.expr = realloc(cmd->args.expr, 4 + strlen(cmd->args.expr));
        if (strlen(cmd->args.expr) > 0)
        {
            strcat(cmd->args.expr, operator);
        }
        strcat(cmd->args.expr, arg);
    }
    else
    {
        return 0;
    }

    if (strcmp(funcName, "AVERAGE") == 0)
    {
        int count  = 1;
        char *next = strchr(cmd->args.expr, '+');
        while (next != NULL)
        {
            next = strchr(next+1, '+');
            count++;
        }
        cmd->args.expr = realloc(cmd->args.expr, 3 + countDigits(count) + strlen(cmd->args.expr));
        memmove(cmd->args.expr+1, cmd->args.expr, strlen(cmd->args.expr));
        cmd->args.expr[0]='(';
        cmd->args.expr[strlen(cmd->args.expr)]=')';
        strcat(cmd->args.expr, "/");
        char* countStr = malloc(countDigits(count));
        sprintf(countStr, "%d", count);
        strcat(cmd->args.expr, countStr);
        //sprintf(cmd->args.expr, "(%s)/%d", cmd->args.expr, count);
    }
    return 1;
}

/**
 * @brief Parses a command sent by the client into an arithmetic expression
 * 
 * @param expr The string to store the parsed command in if successful
 * @param code Set to OK if successful or BAD_SYNTAX, NO_FUNCTION or IMPOSSIBLE depending on the error
 */
struct CommandInfo parseCommand(char *expr, enum Code *code)
{
    struct CommandInfo cmd;

    if (*expr == '=')
    {
        expr++;
        char *commandStart = expr;
        char *commandEnd   = NULL;

        // try to determine if a fucntion is being called by the presence of a (
        while (*expr != '\0')
        {
            if (*expr == '(')
            {
                commandEnd = expr;
                break;
            }
            expr++;
        }

        printf("%s\n", commandEnd);

        if (commandEnd == NULL)
        {
            // try to process with regular operators +, /, -, * or >=, <=, >, <, ==
        }
        else
        {
            // try to find out what type of function

            int len    = commandEnd - commandStart;
            char *name = malloc(len + 1);
            strncpy(name, commandStart, len);
            name[len] = '\0';

            toCaps(name);

            int isImplemented = 0;
            int len_arith     = sizeof(ARITHMETIC_FUNC) / sizeof(ARITHMETIC_FUNC[0]);
            int len_logical   = sizeof(LOGICAL_FUNC) / sizeof(LOGICAL_FUNC[0]);
            int len_other     = sizeof(OTHER_FUNC) / sizeof(OTHER_FUNC[0]);

            // check if it is arithmetic
            for (int i = 0; i < len_arith; i++)
            {
                if (strcmp(ARITHMETIC_FUNC[i], name) == 0)
                {
                    isImplemented = 1;
                    cmd.funcName  = ARITHMETIC_FUNC[i];
                    break;
                }
            }

            for (int i = 0; i < len_logical; i++)
            {
                if (strcmp(LOGICAL_FUNC[i], name) == 0)
                {
                    isImplemented = 2;
                    cmd.funcName  = LOGICAL_FUNC[i];
                    break;
                }
            }

            for (int i = 0; i < len_other; i++)
            {
                if (strcmp(OTHER_FUNC[i], name) == 0)
                {
                    isImplemented = 3;
                    cmd.funcName  = OTHER_FUNC[i];
                    break;
                }
            }

            if (!isImplemented)
            {
                memset(&cmd, 0, sizeof(cmd));
                *code = NO_FUNCTION;
                return cmd;
            }

            switch (isImplemented)
            {
                case 1:
                    if (!tryParseArthimetic(name, expr, &cmd))
                    {
                        *code = BAD_SYNTAX;
                        memset(&cmd, 0, sizeof(cmd));
                        return cmd;
                    }
                    else
                    {
                        *code = OK;
                        return cmd;
                    }

                    break;
                case 2:

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
        cmd.type = PLACEMENT;
    }
}

#endif
