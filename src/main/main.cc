#include "CSJApplication.h"

#include "CSJPathTool.h"
#include "CSJLogger.h"

using namespace csjutils;

void init_log(std::string& log_path);
void uninit_log();

#ifdef _WIN32
//int WinMain() {
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
#elif __APPLE__
int main(int argc, char* avgv[]) {
#endif 
    CSJApplication app;

    fs::path current_path = CSJPathTool::getWorkingPath().remove_filename();
    CSJPathTool::setWorkDirectory(fs::canonical(current_path));

    fs::path logPath = current_path.append("Log").append("CSJVirtualScene.log");

    init_log(logPath.string());
    LOG_Info("CSJImageView started!");

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        uninit_log();
        return EXIT_FAILURE;
    }

    uninit_log();
    return EXIT_SUCCESS;
}

void init_log(std::string& log_path) {
    if (log_path.size() == 0) {
        std::runtime_error("Intialize log path failed!");
    }

    size_t pos = log_path.find_last_of("/\\");
    if (pos == std::string::npos)
        return;

    std::string dir = log_path.substr(0, pos);
    if (CSJPathTool::createPath(dir)) {
        CSJLog_Init(log_path.c_str());
    }
}

void uninit_log() {
    CSJLog_Uninit();
}