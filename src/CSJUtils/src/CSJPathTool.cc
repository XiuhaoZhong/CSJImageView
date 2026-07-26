#include "CSJPathTool.h"

#include <filesystem>

#if defined(__APPLE__)

#include <CoreFoundation/CoreFoundation.h>

#endif

#ifdef _WIN32

#else
#include <sys/stat.h>
#include <errno.h>
#endif

namespace csjutils {

void makePath(const char* path) {
    
}

#if defined(__APPLE__)

static fs::path getAppResourcePath() {
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    if (!mainBundle) {
        return "";
    }

    CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
    if (!resourcesURL) {
        return "";
    }

    char path[PATH_MAX];
    if (CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8 *)path, PATH_MAX)) {
        CFRelease(resourcesURL);
        return fs::path(path);
    }

    CFRelease(resourcesURL);
    return "";
}

#endif 

fs::path CSJPathTool::m_workPath = fs::path();

bool CSJPathTool::createPath(std::string& path) {
    if (path.empty()) {
        return false;
    }

    if (fileExists(path)) {
        return true;
    }

    try {
        return fs::create_directories(path);
    } catch (...) {
        return false;
    }
}

void CSJPathTool::setWorkDirectory(fs::path work_directory) {
    m_workPath = work_directory;
}

bool CSJPathTool::fileExists(std::string &file_path) {
    if (file_path.size() == 0) {
        return false;
    }

    fs::path tmp(file_path);
    return fs::exists(tmp);
}

bool CSJPathTool::fileExists(std::wstring &file_path) {
    if (file_path.size() == 0) {
        return false;
    }

    fs::path tmp(file_path);
    return fs::exists(tmp);
}

fs::path CSJPathTool::getExecuteDir() {
    return m_workPath;
}

fs::path CSJPathTool::getResourceDir() {
    return getExecuteDir().append("Resources");
}

fs::path CSJPathTool::getModelDir() {
    return getResourceDir().append("models");
}

fs::path CSJPathTool::getTextureDir() {
    return getResourceDir().append("textures");
}

fs::path CSJPathTool::getImageDir() {
#if defined(__APPLE__)
    return getAppResourcePath().append("Images");
#else
    return getResourceDir().append("images");
#endif
}

fs::path CSJPathTool::getShaderDir() {
#ifdef _WIN32
    return getResourceDir().append("DXShaders");
#else
    return getAppResourcePath().append("MetalShaders");
#endif
}

fs::path CSJPathTool::getStyleSheetDir() {
    return getResourceDir().append("StyleSheets");
}

std::string CSJPathTool::getModelFileWithName(std::string &model_file_name) {
    return getModelDir().append(model_file_name).string();
}

std::string CSJPathTool::getImageWithName(std::string &image_name) {
    return getImageDir().append(image_name).string();
}

std::string CSJPathTool::getShaderFileWithName(std::string &shader_file_name) {
    return getShaderDir().append(shader_file_name).string();
}

std::string CSJPathTool::getTextureWithName(std::string &texture_file_name) {
    return getTextureDir().append(texture_file_name).string();
}

std::string CSJPathTool::getStyleSheetWithName(std::string &styleSheetsName) {
    return getStyleSheetDir().append(styleSheetsName).string();
}

std::string CSJPathTool::getResVideoFileWithName(const std::string & fileName) {
    if (fileName.size() == 0) {
        return std::string();
    }

#if defined(__APPLE__)
    return getAppResourcePath().append("videos").append(fileName).string();
#else
    return getResourceDir().append("videos").append(fileName).string();
#endif
}

std::string CSJPathTool::getSuffix(const std::string & path) {
    return std::string();
}

std::string CSJPathTool::getFileName(const std::string & path) {
    return std::string();
}

std::string CSJPathTool::getDirName(const std::string & path) {
    return std::string();
}

bool CSJPathTool::isFileExists(const std::string & path) {
    fs::path filePath(path);

    return fs::exists(filePath);
}

bool CSJPathTool::isFileExists(const std::wstring &path) {
    fs::path filePath(path);
    return fs::exists(filePath);
}

bool CSJPathTool::isDirExists(const std::string & path) {
    return false;
}

std::string CSJPathTool::join(const std::string & dir, const std::string & name) {
    return std::string();
}

} // namespace csjutils 