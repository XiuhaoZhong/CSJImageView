#include "CSJStringUtils.h"

#include <locale>
#include <codecvt>

namespace csjutils {

std::wstring CSJStringUtil::char2wstring(const char* origin_str) {
    std::string origin(origin_str);

    return string2wstring(origin);
}

std::wstring CSJStringUtil::string2wstring(const std::string& origin_string) {
    if (origin_string.size() == 0) {
        return std::wstring();
    }

    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    try {
        return converter.from_bytes(origin_string);
    } catch (const std::range_error& e) {
        return std::wstring();
    }
}

std::string CSJStringUtil::wstring2string(const std::wstring &origin_string) {
    if (origin_string.size() == 0) {
        std::string();
    }

    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    try {
        return converter.to_bytes(origin_string);
    } catch (const std::range_error& e) {
        return std::string();
    }
}

} // namespace csjutils
