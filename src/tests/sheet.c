#include "../spreadsheet/spreadsheet.h"

int main(int argc, char const *argv[])
{
    struct Sheet s;
    s.size = 9;
    getBlankSheet(&s);
    printSheet(s);

    // test formatting of words and numbers
    double f = 89;
    char *res = formatNumber(f);
    printf("   %s\n", res);

    f = 389895537.8665;
    res = formatNumber(f);
    printf("   %s\n", res);

    f = 89.8665;
    res = formatNumber(f);
    printf("   %s\n", res);

    f = 895537.8665;
    res = formatNumber(f);
    printf("   %s\n", res);

    f = 8955378665;
    res = formatNumber(f);
    printf("   %s\n", res);

    f = 748955378665;
    res = formatNumber(f);
    printf("   %s\n", res);

    char *w = "Header";
    res = formatWord(w);
    printf("   %s\n", res);

    w = "Head Too Long I think";
    res = formatWord(w);
    printf("   %s\n", res);

    w = "Header On";
    res = formatWord(w);
    printf("   %s\n", res);

    // test placing words and numbers
    struct SheetCoord coords;
    coords.row = 2;
    coords.col = 'B';
    placeNumber(&s, coords, 789);
    printSheet(s);

    coords.row = 1;
    coords.col = 'G';
    placeNumber(&s, coords, 24145789);
    printSheet(s);

    coords.row = 9;
    coords.col = 'E';
    placeNumber(&s, coords, 25789);
    printSheet(s);

    coords.row = 1;
    coords.col = 'A';
    placeWord(&s, coords, "Balance");
    printSheet(s);

    coords.row = 1;
    coords.col = 'I';
    placeWord(&s, coords, "Cumulative Credits");
    printSheet(s);

    // test getting values from the spreadsheet;
    double val;
    w = getPosition(s, coords);
    printf("\n%s", w);

    coords.row = 1;
    coords.col = 'G';
    w = getPosition(s, coords);
    printf("\n%s", w);
    sscanf(w, "%lf", &val);
    printf("\n%lf", val);

    coords.row = 9;
    coords.col = 'E';
    printf("\n%d", isPositionEmpty(s, coords));

    coords.row = 1;
    coords.col = 'A';
    printf("\n%d", isPositionEmpty(s, coords));

    coords.row = 5;
    coords.col = 'E';
    printf("\n%d", isPositionEmpty(s, coords));

    coords.row = 5;
    coords.col = 'C';
    printf("\n%d", isOnSheet(s, coords));

    coords.row = 8;
    coords.col = 'I';
    printf("\n%d", isOnSheet(s, coords));

    coords.row = 8;
    coords.col = 'J';
    printf("\n%d", isOnSheet(s, coords));
    coords.row = 10;
    coords.col = 'J';
    printf("\n%d", isOnSheet(s, coords));

    coords.row = 8;
    coords.col = 'A' - 1;
    printf("\n%d", isOnSheet(s, coords));
    return 0;
}
