#include "../interface/message.h"

// interface unit test
int main(int argc, char const *argv[])
{
    struct ClientMessage test;
    struct ClientMessage test2;

    initClientMessage(&test);
    initClientMessage(&test2);

    // test serialization and parsing of regular request
    test.header.code         = REQUEST;
    test.header.sheetVersion = 1;
    test.header.senderId     = 567;
    test.command             = malloc(sizeof *test.command);
    test.command->coords.row = 4;
    test.command->coords.col = 'E';
    test.command->input      = malloc(16);
    strcpy(test.command->input, "=AVERAGE(43,54)");

    char *p = NULL;
    serializeClientMsg(test, &p);
    printf("%s\n", p);
    parseClientMsg(p, &test2);
    printf("%d\n%d\n%d\n%c\n%s\n", test2.header.code, test2.header.sheetVersion, test2.command->coords.row, test2.command->coords.col, test2.command->input);

    free(test.command->input);
    free(test.command);

    // test serialization and parsing of save request
    initClientMessage(&test);
    initClientMessage(&test2);
    test.header.code         = SAVE;
    test.header.sheetVersion = 1;
    test.header.senderId     = 567;

    serializeClientMsg(test, &p);
    printf("%s\n", p);
    parseClientMsg(p, &test2);
    printf("%d\n%d\n%d\n%c\n%s\n", test2.header.code, test2.header.sheetVersion,test.command==NULL? 0: test2.command->coords.row, test.command==NULL? '!':test2.command->coords.col,test.command==NULL? "NONE": test2.command->input);

    struct ServerMessage t3;
    struct ServerMessage t4;
    initServerMessage(&t4);
    initServerMessage(&t3);
    t3.header.code         = OK;
    t3.header.sheetVersion = 2;
    t3.header.senderId     = 0;
    t3.sheet.size          = 9;
    t3.sheet.rowCount      = 20;
    t3.sheet.lineLength    = 112;

    t3.sheet.grid = calloc(t3.sheet.rowCount, sizeof *t3.sheet.grid);
    for (int i = 0; i < t3.sheet.rowCount; i++)
    {
        t3.sheet.grid[i] = calloc(t3.sheet.lineLength, sizeof *(t3.sheet.grid[i]));
    }
    t3.sheet.grid[0]  = "        A           B           C           D           E           F           G           H           I      ";
    t3.sheet.grid[1]  = "  +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+";
    t3.sheet.grid[2]  = "1 | Balance   |           |           |           |           |           | 2.415e+07 |           | Cumula... |";
    t3.sheet.grid[3]  = "  +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+";
    t3.sheet.grid[4]  = "2 |           |    789.00 |           |           |           |           |           |           |           |";
    t3.sheet.grid[5]  = "  +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+";
    t3.sheet.grid[6]  = "3 |           |           |           |           |           |           |           |           |           |";
    t3.sheet.grid[7]  = "  +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+";
    t3.sheet.grid[8]  = "4 |           |           |           |           |           |           |           |           |           |";
    t3.sheet.grid[9]  = "  +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+";
    t3.sheet.grid[10] = "5 |           |           |           |           |           |           |           |           |           |";
    t3.sheet.grid[11] = "  +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+";
    t3.sheet.grid[12] = "6 |           |           |           |           |           |           |           |           |           |";
    t3.sheet.grid[13] = "  +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+";
    t3.sheet.grid[14] = "7 |           |           |           |           |           |           |           |           |           |";
    t3.sheet.grid[15] = "  +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+";
    t3.sheet.grid[16] = "8 |           |           |           |           |           |           |           |           |           |";
    t3.sheet.grid[17] = "  +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+";
    t3.sheet.grid[18] = "9 |           |           |           |           |  25789.00 |           |           |           |           |";
    t3.sheet.grid[19] = "  +-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+";

    serializeServerMsg(t3, &p);
    serializeServerMsg(t3, &p);
    serializeServerMsg(t3, &p);
    printf("%s\n", p);
    parseServerMsg(p,&t4);
    parseServerMsg(p,&t4);
    parseServerMsg(p,&t4);
    printf("%d\n%d\n%s\n%d\n%d\n%d\n", t4.header.code, t4.header.sheetVersion, t4.message, t4.sheet.size, t4.sheet.rowCount, t4.sheet.lineLength);
    for (int i = 0; i < t4.sheet.rowCount; i++)
    {
        printf("%s\n", t4.sheet.grid[i]);
    }

    for (int i = 0; i < t4.sheet.rowCount; i++)
    {
        free(t4.sheet.grid[i]);
    }

    free(p);
    return 0;
}