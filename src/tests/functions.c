#include "../server/functions.h"

int main(int argc, char const *argv[])
{
    struct Command cmd;
    cmd.input = "=averAGE(A4,A2)";
    struct CommandInfo cinfo;
    enum Code code;
    cmd.coords.col = 'b';
    cmd.coords.row = 3;

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
    placeWord(&s, coords, "Word");
    printSheet(s);

    parseCommand(&cmd, &code, &s);
    printSheet(s);
    printf("%d\n", code);

    cmd.input = "=averAGE(A4:A2)";

    cmd.coords.col = 'b';
    cmd.coords.row = 7;
    parseCommand(&cmd, &code, &s);
    printSheet(s);
    printf("%d\n", code);

    cmd.input = "=averAGE(A1:A5)";

    parseCommand(&cmd, &code, &s);
    printSheet(s);
    printf("%d\n", code);

    cmd.input      = "=sum(C3,C5)";
    cmd.coords.col = 'E';
    cmd.coords.row = 9;
    parseCommand(&cmd, &code, &s);
    printSheet(s);
    printf("%d\n", code);

    cmd.input      = "=range(a1,a3)";
    cmd.coords.col = 'd';
    cmd.coords.row = 4;
    parseCommand(&cmd, &code, &s);
    printSheet(s);
    printf("%d\n", code);

    cmd.input      = "=range(f1,i1)";
    cmd.coords.col = 'h';
    cmd.coords.row = 2;
    parseCommand(&cmd, &code, &s);
    printSheet(s);
    printf("%d\n", code);
    return 0;
}
