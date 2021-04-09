#include "../server/functions.h"

int main(int argc, char const *argv[])
{
    struct Command cmd;
    cmd.input = "=averAGE(A4,A2)";
    struct CommandInfo cinfo;
    enum Code code;
    cmd.coords.col='b';
    cmd.coords.row=3;

    struct Sheet s;
    s.size = 9;
    getBlankSheet(&s);
    struct SheetCoord coords;
    coords.col = 'A';
    coords.row = 1;
    placeNumber(&s, coords, 50665655);
    coords.col = 'A';
    coords.row = 2;
    placeNumber(&s, coords, 555455493);
    coords.col = 'A';
    coords.row = 3;
    placeNumber(&s, coords, 18878882);
    coords.col = 'A';
    coords.row = 4;
    placeNumber(&s, coords, 7989887776);
    printSheet(s);

    parseCommand(&cmd, &code, &s);
    printSheet(s);
    printf("%d\n", code);

    cmd.input = "=averAGE(A4:A2)";

    cmd.coords.col='b';
    cmd.coords.row=7;
    parseCommand(&cmd, &code, &s);
    printSheet(s);
    printf("%d\n", code);

    cmd.input = "=averAGE(A1:A4)";
    parseCommand(&cmd, &code, &s);
    printSheet(s);
    printf("%d\n", code);

    cmd.input = "=sum(A1:A4, B5, C3:C5)";
    parseCommand(&cmd, &code, &s);
    printSheet(s);
    printf("%d\n", code);

    return 0;
}
