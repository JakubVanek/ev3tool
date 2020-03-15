//
// Created by kuba on 30.7.18.
//

#include <atomic>
#include <iostream>
#include <experimental/filesystem>

#include <ev3loader/i18n/localization.hpp>

#include "localization_tool.hpp"
#include "config.hpp"

namespace ev3tool {
    namespace i18n {

#ifdef USE_GETTEXT
        static std::atomic<bool> gettext_initialized(false);

        void i18n_init() {
            bool was = gettext_initialized.exchange(true, std::memory_order_seq_cst);
            if (!was) {
                try {
                    auto prgPath = ev3loader::i18n::program_path();
                    auto locPath = ev3loader::i18n::localedir(prgPath);
                    setlocale(LC_ALL, "");
                    bindtextdomain(TOOL_NAME, locPath.c_str());
                    textdomain(TOOL_NAME);
                } catch (...) {
                    std::cerr << "Localization support load failed." << std::endl;
                }
            }
        }

        const char *_(const char *literal) {
            i18n_init();
            return gettext(literal);
        }

        const char *_num(const char *single, const char *plural, unsigned int n) {
            i18n_init();
            return ngettext(single, plural, n);
        }

#else
        const char *_(const char *literal) {
            return literal;
        }

        const char * _num(const char *single, const char *plural, unsigned int n) {
            if (n > 1) {
                return single;
            } else {
                return plural;
            }
        }
#endif

    }
}
