//
// Created by kuba on 28.7.18.
//

#ifndef EV3TOOL_LOCALIZATION_HPP
#define EV3TOOL_LOCALIZATION_HPP

#include <string>
#include <libintl.h>

namespace ev3tool {
    namespace i18n {
        const char *_(const char *literal);

        const char *_num(const char *single, const char *plural, unsigned int n);
    }
}

#endif //EV3TOOL_LOCALIZATION_HPP
