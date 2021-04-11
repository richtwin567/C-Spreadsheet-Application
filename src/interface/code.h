#ifndef _CODE_H_
#define _CODE_H_

/**
 * @file code.h
 * @author 
 * @brief 
 * @version 0.1
 * @date 2021-03-22
 * 
 * @copyright Copyright (c) 2021
 * 
 */

/**
 * @brief Represents the codes exchanged in the messages between the client and the server
 * 
 */
enum Code
{
    // Codes sent by the server

    OK              = 200, // Indicates that the request was completed successfully
    ACKNOWLEDGED    = 202, // Sent by the server when a client first connects along with the client's assigned ID
    BAD_SYNTAX      = 400, // The server can't understand the syntax of the request
    FORBIDDEN       = 403, // This is sent when the user requesting to save is not the first user.
    COORD_NOT_FOUND = 404, // The request coordinates do not exist on the sheet
    CONFLICT        = 409, // This can occur when the board updated while the user was editing it or conflicting commands were received fom different users.
    IMPOSSIBLE      = 422, // The request made by the user is impossible to complete, eg. dividing by 0 or performing arithmetic between a number and a word.
    NO_FUNCTION     = 501, // The syntax is valid but the server does not have an implementation for the requested function
    CONN_REJECTED   = 503, // The server has already reached max connections at the moment

    // Codes sent by the client

    REQUEST = 1, // An ordinary request to update the spreadsheet
    SAVE    = 2,  // A request to save the spreadsheet

    // codes sent by both
    DISCONNECTED    = 410, // The server has disconnected the client most likely due to the first user disconnecting or the client has disconnected from the server
};


#endif
