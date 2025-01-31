#include "CSJApplication.h"

#ifdef __WIN32
int WinMain() {
#elif __APPLE__
int main(int argc, char* avgv[]) {
#endif 
    CSJApplication app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}