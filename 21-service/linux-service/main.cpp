#include "server.h"

int main(int argc, char *argv[])
{
    server s("127.0.0.1","6666");
    s.run();
    return 0;
}