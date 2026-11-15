#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <iostream>

#include "util/status.h"
#include "web/server.h"

int main() {
    ::SetConsoleOutputCP(CP_UTF8);

    nc::Status st = nc::web::run_hello_server(8787);
    if (!st) {
        std::cerr << st.msg << "\n";
        return 1;
    }
    return 0;
}
