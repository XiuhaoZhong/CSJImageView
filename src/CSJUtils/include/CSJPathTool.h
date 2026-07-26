#ifndef __CSJPATHTOOL_H__
#define __CSJPATHTOOL_H__

#include <string>

#include <filesystem>
#include <memory>

#include "CSJUtils_Export.h"

namespace fs = std::filesystem;

namespace csjutils {

class CSJUTILS_API CSJPathTool {
public:
    /**
     * @brief This function can create directories recursively.
     */
    static bool createPath(std::string& path);

    static void setWorkDirectory(fs::path work_directory);
    static bool fileExists(std::string &file_path);
    static bool fileExists(std::wstring &file_path);

    static fs::path getExecuteDir();
    static fs::path getResourceDir();
    static fs::path getModelDir();
    static fs::path getTextureDir();
    static fs::path getImageDir();
    static fs::path getShaderDir();
    static fs::path getStyleSheetDir();

    static std::string getModelFileWithName(std::string& modelFileName);
    static std::string getImageWithName(std::string& imageName);
    static std::string getShaderFileWithName(std::string& shaderFileName);
    static std::string getTextureWithName(std::string& textureFileName);
    static std::string getStyleSheetWithName(std::string& styleSheetsName);
    static std::string getResVideoFileWithName(const std::string& fileName);

    static std::string getSuffix(const std::string& path);
    static std::string getFileName(const std::string& path);
    static std::string getDirName(const std::string& path);
    static std::string join(const std::string& dir, const std::string& name);
    static bool isFileExists(const std::string& path);
    static bool isFileExists(const std::wstring& path);
    static bool isDirExists(const std::string& path);

protected:
    CSJPathTool() = default;
    ~CSJPathTool() = default;

private:
    static fs::path m_workPath;
};

} // namespace csjutils

#endif // __CSJPATHTOOL_H__