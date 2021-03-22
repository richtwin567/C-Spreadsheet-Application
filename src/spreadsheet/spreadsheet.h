#include "spreadsheetData.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Checks if the specified coordinates exist on the spreadsheet
 * 
 * @param row the row to be checked
 * @param col the column to be checked
 * @return int a non-zero value to indicate true or 0 to indicate false
 */
int isOnSheet(struct Sheet sheet, int row, int col)
{
    return (row <= sheet.size && row >= 1) && (col <= sheet.size && col >= 1);
} // end function isOnSheet

/**
 * @brief Converts the column as the user enters it to an index to be used to access that specify location in the string
 * 
 * @param userColumnInput the column number (1-9)
 * @return int the index of the column
 */
int getColumnIndex(int userColumnInput)
{
    // Calculate the accurate column index by expressing the column
    // as the nth value in an arithmetic series (nth = a+(n-1)d)
    return 4 + (4 * (userColumnInput - 1));
} // end function getColumnIndex

/**
 * @brief Converts the row as the user enters it to an index to be used to access that specify location in the string
 * 
 * @param userRowInput the row number (1-8)
 * @return int the index of the row
 */
int getRowIndex(int userRowInput)
{
    // calculate the accurate row index by expressing the row
    // as the nth value in an arithmetic series  (nth = a+(n-1)d)
    return 2 + (2 * (userRowInput - 1));
} // end function getRowIndex

/**
 * @brief Get the board row based on the index given
 * 
 * @param index the index in the string
 * @return int the board row (1-8)
 */
int getVirtualRow(int index)
{
    // The reverse of getRowIndex
    // Uses the array index to get the row number as seen by the user
    return ((index - 2) / 2) + 1;
} //end function getVirtualRow

/**
 * @brief Get the board column based on the index given
 * 
 * @param index the index in the string
 * @return int the board column (1-8)
 */
int getVirtualCol(int index)
{
    // The reverse of getVirtualCol.
    // Uses the array index to the column number as seen by the user
    return ((index - 4) / 10) + 1;
}

/**
 * @brief Build the empty grid rows and column lines with the appropriate number labels.
 * 
 */
void initGrid(struct Sheet *sheet)
{
    char rowsep[sheet->lineLength];
    char columnlines[sheet->lineLength];
    char colheadings[sheet->lineLength];

    // build row separator string
    // build column lines
    // build column headers

    rowsep[0]      = ' ';
    rowsep[1]      = ' ';
    columnlines[0] = ' ';
    columnlines[1] = ' ';
    colheadings[0] = ' ';
    colheadings[1] = ' ';

    int i;
    for (i = 2; i < sheet->lineLength - 2; i += 10)
    {
        rowsep[i]          = '+';
        rowsep[i + 1]      = '-';
        rowsep[i + 2]      = '-';
        rowsep[i + 3]      = '-';
        rowsep[i + 4]      = '-';
        rowsep[i + 5]      = '-';
        rowsep[i + 6]      = '-';
        rowsep[i + 7]      = '-';
        rowsep[i + 8]      = '-';
        rowsep[i + 9]      = '-';

        columnlines[i]     = '|';
        columnlines[i + 1] = ' ';
        columnlines[i + 2] = ' ';
        columnlines[i + 3] = ' ';
        columnlines[i + 4] = ' ';
        columnlines[i + 5] = ' ';
        columnlines[i + 6] = ' ';
        columnlines[i + 7] = ' ';
        columnlines[i + 8] = ' ';
        columnlines[i + 9] = ' ';

        colheadings[i]     = ' ';
        colheadings[i + 1] = ' ';
        colheadings[i + 2] = ' ';
        colheadings[i + 3] = ' ';
        colheadings[i + 4] = ' ';
        colheadings[i + 5] = getVirtualCol(i + 2) + '0';
        colheadings[i + 6] = ' ';
        colheadings[i + 7] = ' ';
        colheadings[i + 8] = ' ';
        colheadings[i + 9] = ' ';
    }
    rowsep[i]          = '+';
    rowsep[i + 1]      = '\0';
    columnlines[i]     = '|';
    columnlines[i + 1] = '\0';
    colheadings[i]     = ' ';
    colheadings[i + 1] = '\0';

    // Copy the empty grid lines and labels into the grid
    strcpy(sheet->grid[0], colheadings);
    for (i = 1; i < sheet->rowCount - 1; i += 2)
    {
        strcpy(sheet->grid[i], rowsep);
        columnlines[0] = getVirtualRow(i + 1) + '0';
        strcpy(sheet->grid[i + 1], columnlines);
    }
    strcpy(sheet->grid[i], rowsep);

    return;
} // end function initGrid

/**
 * @brief calculate the number of characters for each row and the number of rows needed based on the grid size.
 * 
 */
void calcRowsAndCols(struct Sheet *sheet)
{
    sheet->rowCount   = (sheet->size + 1) * 2;
    sheet->lineLength = 9* (sheet->size + 1);
} //end function calRowsAndCols

/**
 * @brief Get the character at the specified coordinates.
 * 
 * @param coords the coordinates 
 * @return char the character at the coordinates
 */
char getPosition(struct Sheet sheet, struct SheetCoord coords)
{
    return sheet.grid[getRowIndex(coords.x)][getColumnIndex(coords.y)];
} // end function getPosition

/**
     * @brief Checks if the position specified is empty
     * 
     * @param coords the position
     * @return int 1 if the positon is empty or 0 if it is not
     */
int isPositionEmpty(struct Sheet sheet, struct SheetCoord coords)
{

    return isblank(getPosition(sheet, coords));
} // end function isPositionEmpty

/**
 * @brief Initializes a blank new sheet
 * 
 * @param sheet the sheet to initialize
 */
void getBlankSheet(struct Sheet *sheet)
{
    calcRowsAndCols(sheet);

    // allocate the necessary memory
    sheet->grid = calloc(sheet->rowCount, sizeof *sheet->grid);
    for (int i = 0; i < sheet->rowCount; i++)
    {
        sheet->grid[i] = calloc(sheet->lineLength, sizeof *(sheet->grid[i]));
    }
    initGrid(sheet);

} //EndFunction getBlankSheet

/**
 * @brief This function prints out the sheet that was passed.
 * 
 * @param sheet the sheet to print
 */
void printSheet(struct Sheet sheet)
{
    for (int i = 0; i < sheet.rowCount; i++)
    {
        printf("%s\n", sheet.grid[i]);
    }
    return;
} //EndFunction printSheet