//
// Created by kuba on 14.01.20.
//

#ifndef EV3COMM_UPLOAD_COMMON_HPP
#define EV3COMM_UPLOAD_COMMON_HPP

#include <ev3loader/ev3proto.hpp>
#include <ev3loader/common.hpp>

#define CONTINUE_GETFILE_OVERHEAD  12
#define MAX_UP_PAYLOAD (FILE_BUF_SIZE - CONTINUE_GETFILE_OVERHEAD)

namespace ev3loader {
    namespace commands {
        namespace system {
            struct reply_upload {
                reply_upload() = default;

                system_status result = system_status::UNKNOWN_ERROR;
                size_t file_length = 0;
                file_shandle file_id = -1;
                bytearray file_payload{};
            };

            enum class upload_type {
                regular,
                datalog,
                ls
            };
        }
    }
}

#endif //EV3COMM_UPLOAD_COMMON_HPP
