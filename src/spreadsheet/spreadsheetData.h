/**
 * @file spreadsheetData.h
 * @author 
 * @brief 
 * @version 0.1
 * @date 2021-03-21
 * 
 * @copyright Copyright (c) 2021
 * 
 */

/**
 * @brief Represents the spreadsheet
 * 
 */
struct Sheet
{
    int size;       // The size of the board grid
    int rowCount;   // The number of array rows for a size by size grid
    int lineLength; // The number of characters in each row
    char **grid;    // The game board array (pointer implementation)
};

/**
 * @brief Represents a coordinate on a spreadsheet
 * 
 */
struct SheetCoord
{
    int row;  // The row coordinate
    char col; // the column coordinate
};

/**
 * @brief Represents a user given command to be executed by the server
 * 
 */
struct Command
{
    struct SheetCoord coords; // The coordinates for the move
    char *input;              // The letter to be placed at coords
};