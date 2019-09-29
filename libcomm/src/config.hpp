//
// Created by kuba on 1.7.18.
//

#ifndef LIBEV3COMM_CONFIG_HPP
#define LIBEV3COMM_CONFIG_HPP

#define LIB_NAME          "libev3comm"
#define LIB_VERSION_MAJOR  0
#define LIB_VERSION_MINOR  2
#define LIB_VERSION_PATCH  0
#define LIB_VERSION       "0.2.0"
#define LOCALEDIR         "/home/kuba/ev3tool/share/locale"

#define EV3_RXBUF      1024
#define FILE_BUF_SIZE  1024
#define EV3_VID        0x0694
#define EV3_PID        0x0005
#define EV3_PID_FWMODE 0x0006
#define READ_TIMEOUT_MS 200
#define TIMEOUT_PERIOD_MS 200

#define SINGLEDIR
#define USE_GETTEXT

#endif //LIBEV3COMM_CONFIG_HPP
