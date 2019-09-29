//
// Created by kuba on 1.7.18.
//

#include "localization.hpp"
#include "config.hpp"
#include "whereami.hpp"
#include <string>
#include <experimental/filesystem>
#include <clocale>
#include <cstdarg>
#include <iostream>

namespace fs = std::experimental::filesystem;

fs::path getLibraryPath();

fs::path getProgramPath();

fs::path getLocalePath(const fs::path &module);


std::string ev3loader::i18n::vformatn(const char *format, int maxbytes, va_list va) {
    va_list args1, args2;
    va_copy(args1, va);
    va_copy(args2, va);

    int bytes = std::vsnprintf(nullptr, 0, format, args1) + 1;
    if (bytes < 0) {
        throw std::invalid_argument("std::vsnprintf returned negative value");
    }
    va_end(args1);

    bytes = std::min(bytes, maxbytes);
    std::string result(bytes, 0);
    std::vsnprintf(&result.front(), result.size(), format, args2);
    result.erase(result.size() - 1);
    va_end(args2);

    return result;
}

std::string ev3loader::i18n::formatn(const char *format, int maxbytes, ...) {
    va_list va;
    va_start(va, maxbytes);
    auto result = vformatn(format, maxbytes, va);
    va_end(va);
    return result;
}

std::string ev3loader::i18n::format(const char * format, ...) {
    va_list va;
    va_start(va, format);
    auto result = vformatn(format, INT32_MAX, va);
    va_end(va);
    return result;
}


fs::path getLocalePath(const fs::path &module) {
#if (defined(_WIN32) || defined(__CYGWIN__)) || defined(SINGLEDIR)
    return module / fs::u8path("../share/locale");
#else
    return fs::u8path(LOCALEDIR);
#endif
}

fs::path getProgramPath() {
    int dirname_length = 0;

    int chars = wai_getExecutablePath(nullptr, 0, nullptr);
    if (chars == -1)
        throw std::runtime_error("Cannot get executable path!");

    std::string str(chars, 0);
    wai_getExecutablePath(&str.front(), str.size(), &dirname_length);
    return fs::u8path(str).parent_path();
}

fs::path getLibraryPath() {
    int dirname_length = 0;

    int chars = wai_getModulePath(nullptr, 0, nullptr);
    if (chars == -1)
        throw std::runtime_error("Cannot get executable path!");

    std::string str(chars, 0);
    wai_getModulePath(&str.front(), str.size(), &dirname_length);
    return fs::u8path(str).parent_path();
}

std::string ev3loader::i18n::program_path() {
    return getProgramPath().generic_string();
}

std::string ev3loader::i18n::localedir(const std::string &module) {
    return getLocalePath(fs::u8path(module)).generic_string();
}

std::string ev3loader::i18n::library_path() {
    return getLibraryPath().generic_string();
}

static void locale_deleter(locale_t locale) {
    if (locale != nullptr) {
        freelocale(locale);
    }
}

static std::unique_ptr<std::remove_pointer<locale_t>::type, void (*)(locale_t)>
        locale_holder(nullptr, &locale_deleter);

locale_t ev3loader::i18n::get_locale() {
    if (locale_holder == nullptr) {
        locale_holder.reset(newlocale(LC_ALL_MASK, "", nullptr));
        if (locale_holder == nullptr) {
            throw std::runtime_error("Cannot allocate new locale");
        }
    }
    return locale_holder.get();
}

#ifdef USE_GETTEXT

#include <libintl.h>
#include <atomic>

std::atomic<bool> gettext_initialized(false);
void gettext_init() {
    bool was = gettext_initialized.exchange(true, std::memory_order_seq_cst);
    if (!was) {
        try {
            bindtextdomain(LIB_NAME, getLocalePath(getLibraryPath()).c_str());
        } catch (fs::filesystem_error &err) {
            std::cerr << "Localization support load failed." << std::endl;
        }
    }
}

const char * ev3loader::i18n::priv::_(const char *literal) {
    gettext_init();
    return dgettext(LIB_NAME, literal);
}

#else
void gettext_init() {
}

const char *ev3loader::i18n::priv::_(const char *literal) {
    return literal;
}

#endif
