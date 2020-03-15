//
// Created by kuba on 14.01.20.
//

#ifndef EV3COMM_DOWNLOAD_COMMON_HPP
#define EV3COMM_DOWNLOAD_COMMON_HPP

#include <ev3loader/ev3proto.hpp>
#include <ev3loader/common.hpp>

#define DOWNLOAD_CONTINUE_OVERHEAD 7
#define MAX_DL_PAYLOAD (FILE_BUF_SIZE - DOWNLOAD_CONTINUE_OVERHEAD)

namespace ev3loader {
    namespace commands {
        namespace system {
            struct reply_download {
                reply_download() = default;

                system_status result = system_status::UNKNOWN_ERROR;
                file_shandle file_id = -1;
            };
        }
    }
}

#endif //EV3COMM_DOWNLOAD_COMMON_HPP
