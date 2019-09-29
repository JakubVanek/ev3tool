//
// Created by kuba on 1.7.18.
//

#ifndef EV3TOOL_CONFIG_HPP
#define EV3TOOL_CONFIG_HPP

#define TOOL_NAME          "ev3tool"
#define TOOL_VERSION_MAJOR  ${TOOL_VERSION_MAJOR}
#define TOOL_VERSION_MINOR  ${TOOL_VERSION_MINOR}
#define TOOL_VERSION_PATCH  ${TOOL_VERSION_PATCH}
#define TOOL_VERSION       "${TOOL_VERSION}"
#define LOCALEDIR         "${LOCALEDIR}"

#define EV3_VID       0x0694
#define EV3_PID       0x0005

#cmakedefine SINGLEDIR
#cmakedefine USE_GETTEXT

#endif //EV3TOOL_CONFIG_HPP
