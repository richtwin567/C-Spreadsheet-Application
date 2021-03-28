#include "../interface/message.h"

// interface unit test
int main(int argc, char const *argv[])
{
    struct ClientMessage test;
    struct ClientMessage test2;
    test.header.code         = SAVE;
    test.header.sheetVersion = 1;
    test.header.senderId     = 567;
    test.command             = malloc(sizeof *test.command);
    test.command->coords.row = 4;
    test.command->coords.col = 'E';
    test.command->input      = "=AVERAGE(43,54)";

    char *p = serializeClientMsg(test);
    printf("%s\n", p);
    test2 = parseClientMsg(p);
    printf("%d\n%d\n%d\n%c\n%s\n", test2.header.code, test2.header.sheetVersion, test2.command->coords.row, test2.command->coords.col, test2.command->input);

    struct ServerMessage t3;
    struct ServerMessage t4;
    t3.header.code         = OK;
    t3.header.sheetVersion = 2;
    t3.header.senderId     = 0;
    t3.message             = NULL;
    t3.sheet.size          = 9;
    t3.sheet.rowCount      = 20;
    t3.sheet.lineLength    = 40;

    t3.sheet.grid = calloc(t3.sheet.rowCount, sizeof *t3.sheet.grid);
    for (int i = 0; i < t3.sheet.rowCount; i++)
    {
        t3.sheet.grid[i] = calloc(t3.sheet.lineLength, sizeof *(t3.sheet.grid[i]));
    }
    t3.sheet.grid[0]  = "    1   2   3   4   5   6   7   8   9  ";
    t3.sheet.grid[1]  = "  +---+---+---+---+---+---+---+---+---+";
    t3.sheet.grid[2]  = "1 | n | r |   |   |   |   | j |   |   |";
    t3.sheet.grid[3]  = "  +---+---+---+---+---+---+---+---+---+";
    t3.sheet.grid[4]  = "2 |   |   |   |   |   |   |   |   | q |";
    t3.sheet.grid[5]  = "  +---+---+---+---+---+---+---+---+---+";
    t3.sheet.grid[6]  = "3 |   |   |   |   | i |   |   |   | f |";
    t3.sheet.grid[7]  = "  +---+---+---+---+---+---+---+---+---+";
    t3.sheet.grid[8]  = "4 |   |   |   |   |   |   |   |   |   |";
    t3.sheet.grid[9]  = "  +---+---+---+---+---+---+---+---+---+";
    t3.sheet.grid[10] = "5 |   |   |   |   |   |   |   |   |   |";
    t3.sheet.grid[11] = "  +---+---+---+---+---+---+---+---+---+";
    t3.sheet.grid[12] = "6 |   |   |   | p |   |   |   |   |   |";
    t3.sheet.grid[13] = "  +---+---+---+---+---+---+---+---+---+";
    t3.sheet.grid[14] = "7 |   | y |   |   |   |   |   |   |   |";
    t3.sheet.grid[15] = "  +---+---+---+---+---+---+---+---+---+";
    t3.sheet.grid[16] = "8 |   | b |   |   |   |   |   |   |   |";
    t3.sheet.grid[17] = "  +---+---+---+---+---+---+---+---+---+";
    t3.sheet.grid[18] = "9 |   |   |   |   | k |   |   |   |   |";
    t3.sheet.grid[19] = "  +---+---+---+---+---+---+---+---+---+";

    p = serializeServerMsg(t3);
    printf("%s\n", p);
    t4 = parseServerMsg(p);
    printf("%d\n%d\n%s\n%d\n%d\n%d\n", t4.header.code, t4.header.sheetVersion, t4.message, t4.sheet.size, t4.sheet.rowCount, t4.sheet.lineLength);
    for (int i = 0; i < t4.sheet.rowCount; i++)
    {
        printf("%s\n", t4.sheet.grid[i]);
    }
    return 0;
}