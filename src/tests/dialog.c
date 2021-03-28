#include "../client/dialog.h"

int main(int argc, char const *argv[])
{
    printErrorMsg("This is an error message", NULL);

    enum Code code = CONFLICT;
    printErrorMsg("This is an error message", &code);
    printInfoMsg("This is an info message");
    printWarningMsg("This is a warning message");
    printSuccessMsg("This is a success message");

    int choice = promptMenu();

    printf("\nReturned: %d", choice);

    struct SheetCoord coords = promptForCell();

    printf("\nReturned: col: %c, row: %d", coords.col, coords.row);

    char *data = promptForData();

    printf("\nReturned: %s\n", data);

    return 0;
}
