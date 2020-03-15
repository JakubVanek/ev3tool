#include <locale>
#include <codecvt>
#include <cstring>
#include <algorithm>
#include <ev3loader/utils/utils.hpp>


std::wstring ev3loader::utils::narrow2wide(const std::string& str) {
    using codecvt = std::codecvt_utf8<wchar_t>;
    using strcvt = std::wstring_convert<codecvt, wchar_t>;
    strcvt conv;

    return conv.from_bytes(str);
}

std::string ev3loader::utils::wide2narrow(const std::wstring& str) {
    using codecvt = std::codecvt_utf8<wchar_t>;
    using strcvt = std::wstring_convert<codecvt, wchar_t>;
    strcvt conv;

    return conv.to_bytes(str);
}

void ev3loader::utils::tolower_inplace(std::string &str) {
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
}

bool ev3loader::utils::starts_with(const std::string &string, size_t pos, std::string &&prefix) {
    return string.find(prefix, pos) == pos;
}
