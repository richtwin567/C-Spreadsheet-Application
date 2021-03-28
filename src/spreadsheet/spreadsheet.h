#include "../utils/utils.h"
#include "spreadsheetData.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int COL_WIDTH = 12;

/**
 * @brief Checks if the specified coordinates exist on the spreadsheet
 * 
 * @param sheet the sheet to check the coords against
 * @param coords the coordinates to be checked
 * @return int a non-zero value to indicate true or 0 to indicate false
 */
int isOnSheet(struct Sheet sheet, struct SheetCoord coords)
{
    return (coords.row <= sheet.size && coords.row >= 1) && (toupper(coords.col) < sheet.size + 'A' && toupper(coords.col) >= 'A');
} // end function isOnSheet

/**
 * @brief Converts the column as the user enters it to an index to be used to access that specify location in the string
 * 
 * @param userColumnInput the column number
 * @return int the index of the column
 */
int getColumnIndex(char userColumnInput)
{
    // Calculate the accurate column index by expressing the column
    // as the nth value in an arithmetic series (nth = a+(n-1)d)
    return 4 + (COL_WIDTH * (toupper(userColumnInput) - 'A'));
} // end function getColumnIndex

/**
 * @brief Converts the row as the user enters it to an index to be used to access that specify location in the string
 * 
 * @param userRowInput the row number
 * @return int the index of the row
 */
int getRowIndex(int userRowInput)
{
    // calculate the accurate row index by expressing the row
    // as the nth value in an arithmetic series  (nth = a+(n-1)d)
    return 2 + (2 * (userRowInput - 1));
} // end function getRowIndex

/**
 * @brief Get the sheet row based on the index given
 * 
 * @param index the index in the string
 * @return int the sheet row
 */
int getVirtualRow(int index)
{
    // The reverse of getRowIndex
    // Uses the array index to get the row number as seen by the user
    return ((index - 2) / 2) + 1;
} //end function getVirtualRow

/**
 * @brief Get the sheet column based on the index given
 * 
 * @param index the index in the string
 * @return char the sheet column
 */
char getVirtualCol(int index)
{
    // The reverse of getColumnIndex.
    // Uses the array index to the column number as seen by the user
    return ((index - 4) / COL_WIDTH) + 'A';
}

/**
 * @brief Build the empty grid rows and column lines with the appropriate labels.
 * 
 * @param sheet the sheet to initialize
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
    for (i = 2; i < sheet->lineLength - 2; i += COL_WIDTH)
    {
        rowsep[i] = '+';
        for (int x = i + 1; x < i + COL_WIDTH; x++)
        {
            rowsep[x] = '-';
        }

        columnlines[i] = '|';
        for (int x = i + 1; x < i + COL_WIDTH; x++)
        {
            columnlines[x] = ' ';
        }

        for (int x = i; x < i + COL_WIDTH; x++)
        {
            if ((x == (i + (COL_WIDTH / 2))))
            {
                colheadings[x] = getVirtualCol(i + 2);
            }
            else
            {

                colheadings[x] = ' ';
            }
        }
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
 * @brief Calculate and set the number of characters for each row and the number of rows needed based on the sheet size.
 * 
 * @param sheet the sheet to calculate for.
 * 
 */
void calcRowsAndCols(struct Sheet *sheet)
{
    sheet->rowCount   = (sheet->size + 1) * 2;
    sheet->lineLength = (COL_WIDTH * sheet->size) + 4;
} //end function calRowsAndCols

/**
 * @brief Get the value at the specified coordinates.
 * 
 * @param sheet the sheet to get the value
 * @param coords the coordinates 
 * @return char* the value at the coordinates
 */
char *getPosition(struct Sheet sheet, struct SheetCoord coords)
{
    char *value = calloc(COL_WIDTH - 2, sizeof *value);
    strncpy(value, sheet.grid[getRowIndex(coords.row)] + getColumnIndex(coords.col), COL_WIDTH - 2);
    value[COL_WIDTH - 3] = '\0';
    return value;
} // end function getPosition

/**
 * @brief Checks if the position specified is empty
 * 
 * @param sheet the sheet to check
 * @param coords the position
 * @return int 1 if the positon is empty or 0 if it is not
 */
int isPositionEmpty(struct Sheet sheet, struct SheetCoord coords)
{
    char *value = getPosition(sheet, coords);
    int i       = 0;
    while (value[i] != '\0')
    {
        if (!isblank(value[i]))
        {
            return 0;
        }
        i++;
    }
    return 1;

} // end function isPositionEmpty

/**
 * @brief Returns a string of a number apprpriately formatted to fit into
 * the column. 
 * 
 * @param num the number to be formatted
 * @return char* the formatted number
 */
char *formatNumber(double num)
{
    int wholeDigits = countDigits((long)num);
    char *numStr    = calloc(COL_WIDTH - 2, sizeof *numStr);

    if (wholeDigits > (COL_WIDTH - 3))
    {
        snprintf(numStr, COL_WIDTH - 2, "%.3e", num);
    }
    else
    {
        if ((wholeDigits + 3) > (COL_WIDTH - 3))
        {
            snprintf(numStr, COL_WIDTH - 2, "%.3e", num);
        }
        else
        {
            snprintf(numStr, COL_WIDTH - 2, "%*.2f", COL_WIDTH - 3, num);
        }
    }
    return numStr;
} // end function formatNumber

/**
 * @brief Returns the word appropriately formatted to fit into the column width 
 * 
 * @param word the word to format
 * @return char* the formatted word
 */
char *formatWord(char *word)
{
    char *wordFormat;

    if (strlen(word) > (COL_WIDTH - 3))
    {
        wordFormat = calloc(COL_WIDTH - 2, sizeof *wordFormat);
        sprintf(wordFormat, "%.6s...", word);
    }
    else
    {
        wordFormat = word;
    }
    return wordFormat;
} // end function formatWord

/**
 * @brief Places a word at the given coordinates on the spreadsheet 
 * 
 * @param sheet the shhet to place the word on
 * @param coords the location to place the word
 * @param word the word to place
 */
void placeWord(struct Sheet *sheet, struct SheetCoord coords, char *word)
{
    char *vStr = formatWord(word);
    int row    = getRowIndex(coords.row);
    int col    = getColumnIndex(coords.col);
    int i      = 0;
    while (vStr[i] != '\0')
    {
        sheet->grid[row][col + i] = vStr[i];
        i++;
    }
}

/**
 * @brief Places a number at the given coordinates on the given spreadsheet
 * 
 * @param sheet the sheet to place on
 * @param coords the location to place
 * @param value the number to place
 */
void placeNumber(struct Sheet *sheet, struct SheetCoord coords, double value)
{
    char *vStr = formatNumber(value);
    int row    = getRowIndex(coords.row);
    int col    = getColumnIndex(coords.col);
    int i      = 0;
    while (vStr[i] != '\0')
    {
        sheet->grid[row][col + i] = vStr[i];
        i++;
    }

} // end function placeNumber

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

/**
 * @brief Creates a copy of the given sheet 
 * 
 * @param sheet the sheet to copy
 * @return struct Sheet the newly copied sheet
 */
struct Sheet copySheet(struct Sheet sheet)
{
    struct Sheet copy;
    copy.size       = sheet.size;
    copy.lineLength = sheet.lineLength;
    copy.rowCount   = sheet.rowCount;

    copy.grid = calloc(copy.rowCount, sizeof *copy.grid);
    for (int i = 0; i < copy.rowCount; i++)
    {
        copy.grid[i] = calloc(copy.lineLength, sizeof *(copy.grid[i]));
        strcpy(copy.grid[i],sheet.grid[i]);
    }

    return copy;
}// end function copySheet