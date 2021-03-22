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
    BAD_SYNTAX      = 400, // The server can't understand the syntax of the request
    FORBIDDEN       = 403, // This is sent when the user requesting to save is not the first user
    COORD_NOT_FOUND = 404, // The request coordinates do not exist on the sheet
    CONFLICT        = 409, // This can occur when the board updated while the user was editing it
    IMPOSSIBLE      = 422, // The request made by the user is impossible to complete, eg. dividing by 0.
    NO_FUNCTION     = 501, // The server does not have an implementation for the requested function

    // Codes sent by the client

    REQUEST = 1, // An ordinary request to update the spreadsheet
    SAVE    = 2  // A request to save the spreadsheet

};