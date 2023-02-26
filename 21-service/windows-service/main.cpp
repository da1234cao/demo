#include "service.hpp"

int main(int argc, char* argv[]) {
    service::start();
    WriteLog("process end");
    return 0;
}