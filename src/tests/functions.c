#include "../server/functions.h"

int main(int argc, char const *argv[])
{
    char *cmd = "=averAGE(A4,A2)";
    enum Code code;
    parseCommand(cmd, &code);

    memset(&cmd, 0, sizeof cmd);
    cmd = "=averAGE(A4:A2)";
    parseCommand(cmd, &code);

    memset(&cmd, 0, sizeof cmd);
    cmd = "=averAGE(A1:A4)";
    parseCommand(cmd, &code);

    return 0;
}
