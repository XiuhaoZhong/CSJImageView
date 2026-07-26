#include "CSJApplication.h"

#ifdef _WIN32
//int WinMain() {
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
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