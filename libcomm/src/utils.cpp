#include "utils.hpp"
#include <locale>
#include <codecvt>
#include <sstream>
#include <cassert>
#include <cstring>
#include <algorithm>

std::wstring ev3loader::narrow2wide(std::string str) {
    using codecvt = std::codecvt_utf8<wchar_t>;
    using strcvt = std::wstring_convert<codecvt, wchar_t>;
    strcvt conv;

    return conv.from_bytes(str);
}

std::string ev3loader::wide2narrow(std::wstring str) {
    using codecvt = std::codecvt_utf8<wchar_t>;
    using strcvt = std::wstring_convert<codecvt, wchar_t>;
    strcvt conv;

    return conv.to_bytes(str);
}

void ev3loader::tolower_inplace(std::string &str) {
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
}

bool ev3loader::starts_with(const std::string &string, size_t pos, std::string &&prefix) {
    return string.find(prefix, pos) == pos;
}

std::string ev3loader::tolower_copy(std::string str) {
    tolower_inplace(str);
    return str;
}
