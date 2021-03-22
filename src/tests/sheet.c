#include "../spreadsheet/spreadsheet.h"

int main(int argc, char const *argv[])
{
    struct Sheet s;
    s.size=9;
    getBlankSheet(&s);
    printSheet(s);
    

    return 0;
}
