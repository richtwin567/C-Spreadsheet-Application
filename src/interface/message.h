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
#include "../spreadsheet/spreadsheet.h"
#include "code.h"

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
    enum Code code;   // The code associated with the message
    int sheetVersion; // The latest version of the sheet that the sender has
    int senderId;     // The sender's ID number
};

/**
 * @brief Represents a client message
 * 
 */
struct ClientMessage
{
    struct Header header; // The packet header

    // payload
    struct Command *command; // The command sent by the client. This may be null if the client is requesting a save
};

/**
 * @brief Represents a server message
 * 
 */
struct ServerMessage
{
    struct Header header; // The packet header

    // payload
    struct Sheet sheet; // The latest version of the spreadsheet
    char *message;      // Any additional message the server wants to send to the client. This may be null.
};

/**
 * @brief Get the Payload Length from the header
 * 
 * @param headerStr the header
 * @return int the payload length
 */
int getPayloadLength(char *headerStr)
{
    int length=0;
    sscanf(headerStr, "%*d:%*d:%*d:%d", &length);
    return length;
}

/**
 * @brief Initializes a ServerMessage struct's pointers to null
 * 
 * @param msg the server message
 */
void initServerMessage(struct ServerMessage *msg)
{
    msg->message    = NULL;
    msg->sheet.grid = NULL;
}

/**
 * @brief Initializes a ClientMessage struct's pointers to null
 * 
 * @param msg the client message
 */
void initClientMessage(struct ClientMessage *msg)
{
    msg->command = NULL;
}

/**
 * @brief Converts a client message to a string to be sent over a socket
 * 
 * @param msg the message to be converted
 * @param packet a pointer to the packet to write the serialized data to
 * @return int the packet length
 */
int serializeClientMsg(struct ClientMessage msg, char **packet)
{
    int payloadLength = 0; // the length of the message not including the header
    int fullLength    = 0; // the length of the payload and the header
    char *temp        = calloc(HEADER_SIZE, sizeof(char));

    // add length of command and coords and a space for the null terminator
    payloadLength += msg.command == NULL ? 11 : countDigits(msg.command->coords.row) + 4 + strlen(msg.command->input);

    // initialize packet memory if null
    if (*packet == NULL)
    {
        *packet = malloc(1);
    }

    // zero out structure
    memset(*packet, 0, sizeof *packet);

    sprintf(temp, "%d:%d:%d:%d@", msg.header.code, msg.header.sheetVersion, msg.header.senderId, payloadLength);

    fullLength += payloadLength + HEADER_SIZE;

    *packet = realloc(*packet, fullLength);

    // pad to length of header
    sprintf(*packet, "%*s%d:%c:%s", HEADER_SIZE, temp,
            msg.command == NULL ? -1 : msg.command->coords.row,
            msg.command == NULL ? '!' : msg.command->coords.col,
            msg.command == NULL ? "None" : msg.command->input);

    // free used pointers
    free(temp);

    return fullLength;
} // end function serializeClientMsg

/**
 * @brief Converts a server message to a string
 * 
 * @param msg the message to convert
 * @param packet a pointer to the packet to write the serialized data to
 * @return int the packet length
 */
int serializeServerMsg(struct ServerMessage msg, char **packet)
{
    int payloadLength = 0;                   // the length of the message not including the header
    int fullLength    = 0;                   // the length of the payload and the header
    char *temp        = malloc(HEADER_SIZE); // a temporary header to avoid undefined behaviour if sprintf locations overlap on line 151

    // add length of grid
    payloadLength = (msg.sheet.rowCount * msg.sheet.lineLength);

    // add length of grid stats
    payloadLength += countDigits(msg.sheet.size) + countDigits(msg.sheet.rowCount) + countDigits(msg.sheet.lineLength);

    // add length of message if it exists
    payloadLength += msg.message == NULL ? 4 : strlen(msg.message);

    // add space for semicolon separators
    payloadLength += 4;

    // add space for null terminator
    payloadLength++;

    // initialize packet memory if null
    if (*packet == NULL)
    {
        *packet = malloc(1);
    }

    // zero out structure
    memset(*packet, 0, sizeof *packet);

    sprintf(temp, "%d:%d:%d:%d@", msg.header.code, msg.header.sheetVersion, msg.header.senderId, payloadLength);

    fullLength += payloadLength + HEADER_SIZE;

    *packet = realloc(*packet, fullLength);

    // pad to length of header
    sprintf(*packet, "%*s%d:%d:%d:", HEADER_SIZE, temp, msg.sheet.size, msg.sheet.rowCount, msg.sheet.lineLength);

    // add grid
    for (int i = 0; i < msg.sheet.rowCount; i++)
    {
        strcat(*packet, msg.sheet.grid[i]);
        strcat(*packet, "\n");
    }

    // add message
    strcat(*packet, ":");
    strcat(*packet, msg.message == NULL ? "None" : msg.message);

    // free used pointers
    free(temp);

    return fullLength;

} // end function serializeServerMsg

/**
 * @brief Converts a string into a server message
 * 
 * @param msg the string sent throught the socket
 * @param parsedMsg the message
 */
void parseServerMsg(char *msg, struct ServerMessage *parsedMsg)
{
    int payloadLength = 0;
    int messageLength = 0;
    int gridLength    = 0;
    int read          = 0;
    int size          = 0;
    int rowCount      = 0;
    int lineLength    = 0;
    int code          = 0;
    int i             = 0;

    read = sscanf(msg, "%d:%*d:%*d:%d@%d:%d:%d:", &code, &payloadLength, &size, &rowCount, &lineLength);

    if (read != 5)
    {
        fprintf(stderr, "\nParsing the server message failed\n");
        // TODO maybe exit?
    }

    payloadLength -= countDigits(size) + countDigits(rowCount) + countDigits(lineLength) + 3;

    // move the pointer down to the start of the grid
    char *gridStart = strchr(msg, '@') + 1;
    gridStart       = strchr(gridStart, ':') + 1;
    gridStart       = strchr(gridStart, ':') + 1;
    gridStart       = strchr(gridStart, ':') + 1;

    // move pointer down to start of message
    char *messageStart = strchr(gridStart, ':') + 1;

    gridLength = messageStart - gridStart;

    messageLength = payloadLength - gridLength;

    // allocate/reallocate memory for message
    if (parsedMsg->message == NULL)
    {
        parsedMsg->message = malloc(messageLength);
    }
    else
    {
        parsedMsg->message = realloc(parsedMsg, messageLength);
    }

    // allocate/reallocate memory for spreadsheet grid
    if (parsedMsg->sheet.grid == NULL)
    {
        parsedMsg->sheet.grid = calloc(rowCount, sizeof *parsedMsg->sheet.grid);
        for (int i = 0; i < rowCount; i++)
        {
            parsedMsg->sheet.grid[i] = calloc(lineLength, sizeof *(parsedMsg->sheet.grid[i]));
        }
    }
    else
    {
        parsedMsg->sheet.grid = realloc(parsedMsg->sheet.grid, rowCount * (sizeof *parsedMsg->sheet.grid));
        for (int i = 0; i < rowCount; i++)
        {
            parsedMsg->sheet.grid[i] = realloc(parsedMsg->sheet.grid[i], lineLength * (sizeof *(parsedMsg->sheet.grid[i])));
            memset(parsedMsg->sheet.grid[i], 0, sizeof parsedMsg->sheet.grid[i]);
        }  }

    // clear memory
    memset(parsedMsg->message, 0, sizeof parsedMsg->message);
    memset(&parsedMsg->sheet, 0, sizeof &parsedMsg->sheet);
    memset(&parsedMsg->header, 0, sizeof &parsedMsg->header);
    memset(parsedMsg, 0, sizeof parsedMsg);

    parsedMsg->header.code      = code;
    parsedMsg->sheet.rowCount   = rowCount;
    parsedMsg->sheet.lineLength = lineLength;
    parsedMsg->sheet.size       = size;

    read = sscanf(msg, "%*d:%d:%d:", &parsedMsg->header.sheetVersion, &parsedMsg->header.senderId);

    if (read != 2)
    {
        fprintf(stderr, "\nParsing the server message failed\n");
        // TODO maybe exit?
    }

    // fill grid
    i     = 0;
    int x = 0;
    while (*gridStart != ':')
    {
        if (*gridStart == '\n')
        {
            i++;
            x = 0;
        }
        else
        {
            parsedMsg->sheet.grid[i][x] = *gridStart;
            x++;
        }
        gridStart++;
    }

    // get message
    i = 0;
    while (*messageStart != '\0')
    {
        parsedMsg->message[i] = *messageStart;
        messageStart++;
        i++;
        parsedMsg->message = realloc(parsedMsg->message, i + 1);
    }
    parsedMsg->message[i] = '\0';

    if (strcmp(parsedMsg->message, "None") == 0)
    {
        free(parsedMsg->message);
        parsedMsg->message = NULL;
    }

} // end function parseServerMsg

/**
 * @brief Converts a string into a client message
 * 
 * @param msg the message to convert
 * @param parsedMsg the message
 */
void parseClientMsg(char *msg, struct ClientMessage *parsedMsg)
{
    int payloadLength, read, code, row;

    read = sscanf(msg,
                  "%d:%*d:%*d:%d@%d:", &code, &payloadLength, &row);

    if (read != 3)
    {
        fprintf(stderr, "\nParsing the client message failed\n");
        // TODO maybe exit?
    }

    payloadLength -= (countDigits(row) + 3);

    if (parsedMsg->command == NULL)
    {
        parsedMsg->command        = malloc(sizeof *parsedMsg->command);
        parsedMsg->command->input = malloc(payloadLength);
    }
    else
    {
        parsedMsg->command        = realloc(parsedMsg->command, sizeof *parsedMsg->command);
        parsedMsg->command->input = realloc(parsedMsg->command->input, payloadLength);
    }

    // clear memory
    memset(parsedMsg->command->input, 0, payloadLength);
    memset(parsedMsg->command, 0, sizeof parsedMsg->command);
    memset(parsedMsg, 0, sizeof parsedMsg);

    parsedMsg->header.code         = code;
    parsedMsg->command->coords.row = row;

    read = sscanf(msg,
                  "%*d:%d:%d:%*d@%*d:%c:%s",
                  &parsedMsg->header.sheetVersion,
                  &parsedMsg->header.senderId,
                  &parsedMsg->command->coords.col,
                  parsedMsg->command->input);

    if (read != 4)
    {
        fprintf(stderr, "\nParsing the client message failed\n");
        // TODO maybe exit?
    }

    if (parsedMsg->command->coords.col == '!' && parsedMsg->command->coords.row == -1 && strcmp(parsedMsg->command->input, "None") == 0)
    {
        free(parsedMsg->command);
        parsedMsg->command = NULL;
    }

} // end function parseClientMsg
