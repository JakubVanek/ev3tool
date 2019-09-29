//
// Created by kuba on 1.7.18.
//

#ifndef LIBEV3COMM_CONFIG_HPP
#define LIBEV3COMM_CONFIG_HPP

#define LIB_NAME          "libev3comm"
#define LIB_VERSION_MAJOR  ${LIB_VERSION_MAJOR}
#define LIB_VERSION_MINOR  ${LIB_VERSION_MINOR}
#define LIB_VERSION_PATCH  ${LIB_VERSION_PATCH}
#define LIB_VERSION       "${LIB_VERSION}"
#define LOCALEDIR         "${LOCALEDIR}"

#define EV3_RXBUF      1024
#define FILE_BUF_SIZE  1024
#define EV3_VID        0x0694
#define EV3_PID        0x0005
#define EV3_PID_FWMODE 0x0006
#define READ_TIMEOUT_MS 200
#define TIMEOUT_PERIOD_MS 200

#cmakedefine SINGLEDIR
#cmakedefine USE_GETTEXT

#endif //LIBEV3COMM_CONFIG_HPP
