//
// Created by kuba on 30.6.18.
//

#ifndef LIBEV3COMM_LOCALIZATION_HPP
#define LIBEV3COMM_LOCALIZATION_HPP

#include <string>
#include <memory>

#define HERE (ev3loader::i18n::format("%s:%d", __FILE__, __LINE__))
#define _n(x) x

namespace ev3loader {
    namespace i18n {
        using msgid_t   = std::string;
        using msgstr_t  = std::string;
        using locstring = msgstr_t;

        std::string vformatn(const char *format, int maxbytes, va_list va);

        __attribute__((__format__(__printf__, 1, 2)))
        std::string format (const char *format, ...);

        __attribute__((__format__(__printf__, 1, 3)))
        std::string formatn (const char *format, int maxbytes, ...);

        std::string library_path();
        std::string program_path();
        std::string localedir(const std::string &module);
        locale_t get_locale();

        namespace priv {
            const char *_(const char *literal);
        }
    }
    using i18n::format;
    using i18n::formatn;
    using i18n::locstring;
}

#endif //LIBEV3COMM_LOCALIZATION_HPP
