/**
 * @file message.h
 * @author 
 * @brief 
 * @version 0.1
 * @date 2021-03-21
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "code.h"
#include "../spreadsheet/spreadsheetData.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int HEADER_SIZE = 20;

/**
 * @brief Represents the packet header
 * 
 */
struct Header
{
    enum Code code;         // The code associated with the message
    int       sheetVersion; // The latest version of the sheet that the sender has
};

/**
 * @brief Represents a client message
 * 
 */
struct ClientMessage
{
    struct Header header; // The packet header

    // payload
    struct Command command; // The command sent by the client. This may be null if the client is requesting a save
};

/**
 * @brief Represents a server message
 * 
 */
struct ServerMessage
{
    struct Header header; // The packet header

    // payload
    struct Sheet sheet;   // The latest version of the spreadsheet
    char *       message; // Any additional message the server wants to send to the client. This may be null.
};

/**
 * @brief Converts a client message to a string to be sent over a socket
 * 
 * @param msg the message to be converted
 * @return char* the message as a string
 */
char *serializeClientMsg(struct ClientMessage msg)
{
    char *payload = calloc(4 + strlen(msg.command.input), sizeof(char)); //initialize message to size of command and coordinates.
    char *header  = calloc(HEADER_SIZE, sizeof(char));
    char *temp    = calloc(HEADER_SIZE, sizeof(char));
    
    sprintf(payload, "%d:%c:%s", msg.command.coords.x, msg.command.coords.y, msg.command.input);
    sprintf(temp, "%d:%d:%ld@", msg.header.code, msg.header.sheetVersion, strlen(payload));
    // pad to length of header
    sprintf(header, "%*s", HEADER_SIZE, temp);

    int   fullSize = HEADER_SIZE + strlen(payload);
    char *packet   = calloc(fullSize, sizeof(char));
    sprintf(packet, "%s%s", header, payload);

    // free used pointers
    free(temp);
    free(header);
    free(payload);

    return packet;
}

/**
 * @brief Count the number of digits in a number   
 * 
 * @param number the number to count
 * @return int the number of digits in number
 */
int countDigits(int number)
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
 * @brief Converts a server message to a string
 * 
 * @param msg the message to convert
 * @return char* the message a string
 */
char *serializeServerMsg(struct ServerMessage msg)
{
    int   length = (msg.sheet.rowCount * msg.sheet.lineLength) + msg.sheet.rowCount;
    char *grid   = calloc(length, sizeof(char));
    for (int i = 0; i < msg.sheet.rowCount; i++)
    {
        strcat(grid, msg.sheet.grid[i]);
        strcat(grid, "\n");
    }

    length += countDigits(msg.sheet.size) + countDigits(msg.sheet.rowCount) + countDigits(msg.sheet.lineLength);
    char *payload;
    if (msg.message != NULL)
    {
        length += strlen(msg.message);
        payload = calloc(length, sizeof(char)); //initialize message to size of command and coordinates.
        sprintf(payload, "%d:%d:%d:%s:%s", msg.sheet.size, msg.sheet.rowCount, msg.sheet.lineLength, grid, msg.message);
    }
    else
    {
        payload = calloc(length, sizeof(char)); //initialize message to size of command and coordinates.
        sprintf(payload, "%d:%d:%d:%s:None", msg.sheet.size, msg.sheet.rowCount, msg.sheet.lineLength, grid);
    }

    char *header = calloc(HEADER_SIZE, sizeof(char));
    char *temp   = calloc(HEADER_SIZE, sizeof(char));
    sprintf(temp, "%d:%d:%ld@", msg.header.code, msg.header.sheetVersion, strlen(payload));
    // pad to length of header
    sprintf(header, "%*s", HEADER_SIZE, temp);

    int   fullSize = HEADER_SIZE + strlen(payload);
    char *packet   = calloc(fullSize, sizeof(char));
    sprintf(packet, "%s%s", header, payload);

    // free used pointers
    free(temp);
    free(grid);
    free(header);
    free(payload);

    return packet;
}

/**
 * @brief Converts a string into a server message
 * 
 * @param msg the string sent throught the socket
 * @return struct ServerMessage the message obtained from the string
 */
struct ServerMessage parseServerMsg(char *msg)
{
    struct ServerMessage parsedMsg;

    int length;
    int read;
    int code;
    int i = 0;

    read = sscanf(msg, "%d:%d:%d@%d:%d:%d:", &code, &parsedMsg.header.sheetVersion, &length, &parsedMsg.sheet.size, &parsedMsg.sheet.rowCount, &parsedMsg.sheet.lineLength);

    parsedMsg.header.code = code;

    if (read != 6)
    {
        fprintf(stderr, "\nParsing the server message failed\n");
        // TODO maybe exit?
    }

    // move the pointer down to the start of the grid
    char *payloadSectionStart = strchr(msg, '@') + 1;
    payloadSectionStart       = strchr(payloadSectionStart, ':') + 1;
    payloadSectionStart       = strchr(payloadSectionStart, ':') + 1;
    payloadSectionStart       = strchr(payloadSectionStart, ':') + 1;

    // allocate memory for spreadsheet grid
    parsedMsg.sheet.grid = calloc(parsedMsg.sheet.rowCount, sizeof *parsedMsg.sheet.grid);
    for (int i = 0; i < parsedMsg.sheet.rowCount; i++)
    {
        parsedMsg.sheet.grid[i] = calloc(parsedMsg.sheet.lineLength, sizeof *(parsedMsg.sheet.grid[i]));
    }

    // fill grid
    i     = 0;
    int x = 0;
    payloadSectionStart++;
    while (*payloadSectionStart != ':' && *payloadSectionStart != '\0')
    {
        if (*payloadSectionStart == '\n')
        {
            i++;
            x = 0;
        }
        else
        {
            parsedMsg.sheet.grid[i][x] = *payloadSectionStart;
            x++;
        }
        payloadSectionStart++;
    }

    // get message
    i = 0;
    payloadSectionStart++;
    parsedMsg.message = calloc(1, sizeof(char));
    while (*payloadSectionStart != '\0')
    {
        parsedMsg.message[i] = *payloadSectionStart;
        payloadSectionStart++;
        i++;
        parsedMsg.message = realloc(parsedMsg.message, i + 1);
    }
    parsedMsg.message[i] = '\0';

    return parsedMsg;
}

/**
 * @brief Converts a string into a client message
 * 
 * @param msg the message to convert
 * @return struct ClientMessage 
 */
struct ClientMessage parseClientMsg(char *msg)
{
    struct ClientMessage parsedMsg;

    int payloadLength;
    int read;
    int code;

    read = sscanf(msg,
                  "%d:%d:%d@%d:%c", &code, &parsedMsg.header.sheetVersion, &payloadLength, &parsedMsg.command.coords.x, &parsedMsg.command.coords.y);

    if (read != 5)
    {
        fprintf(stderr, "\nParsing the client message failed\n");
        // TODO maybe exit?
    }

    parsedMsg.header.code = code;
    payloadLength -= 4;
    parsedMsg.command.input = calloc(payloadLength, sizeof(char));

    read = sscanf(msg, "%*[^@]@%*[^:]:%*[^:]:%s", parsedMsg.command.input);
    if (read != 1)
    {
        fprintf(stderr, "\nParsing the client message failed\n");
        // TODO maybe exit?
    }
    return parsedMsg;
}
