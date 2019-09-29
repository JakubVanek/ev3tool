/**
 * EV3loader - the EV3 uploader
 * 
 * Copyright (C) 2017  Faculty of Electrical Engineering, CTU in Prague
 * Author: Jakub Vanek <nxc4ev3@vankovi.net>
 * 
 * 
 * EV3 communication protocol declarations.
 */

#ifndef EV3_PROTO_H
#define EV3_PROTO_H

#include <cstdint>
#include "localization.hpp"


namespace ev3loader {

    enum message_type : uint8_t {
        SYS_SEND_REPLY = 0x01, // 0b00000001
        SYS_SEND_NOREPLY = 0x81, // 0b10000001
        SYS_RECV_REPLY = 0x03, // 0b00000011
        SYS_RECV_REPLYERROR = 0x05, // 0b00000101

        DIR_SEND_REPLY = 0x00, // 0b00000000
        DIR_SEND_NOREPLY = 0x80, // 0b10000000
        DIR_RECV_REPLY = 0x02, // 0b00000010
        DIR_RECV_REPLYERROR = 0x04, // 0b00000100

        DIR_SENDBUSY_REPLY = 0x0F, // 0b00001111
        DIR_SENDBUSY_NOREPLY = 0x8F, // 0b00001111
        DAISY_SEND_REPLY = 0x0A, // 0b00001010
        DAISY_SEND_NOREPLY = 0x8A, // 0b10001010
        DAISY_RECV_REPLY = 0x08, // 0b00001000
        DAISY_RECV_REPLYERROR = 0x09, // 0b00001001
    };

    enum system_commands : uint8_t {
        COMMAND_ERROR = 0x00,
        DOWNLOAD_BEGIN = 0x92,
        DOWNLOAD_CONTINUE = 0x93,
        UPLOAD_BEGIN = 0x94,
        UPLOAD_CONTINUE = 0x95,
        GETFILE_BEGIN = 0x96,
        GETFILE_CONTINUE = 0x97,
        LIST_FILES_BEGIN = 0x99,
        LIST_FILES_CONTINUE = 0x9A,
        MKDIR = 0x9B,
        DELETE_FILE = 0x9C,
        WRITEMAILBOX = 0x9E,
        BLUETOOTHPIN = 0x9F,
        ENTERFWUPDATE = 0xA0,
        SETBUNDLEID = 0xA1,
        SETBUNDLESEEDID = 0xA2,
        LIST_OPEN_HANDLES = 0x9D,
        CLOSE_FILEHANDLE = 0x98,

        FW_DOWNLOADERASE_BEGIN = 0xF0,
        FW_DOWNLOAD_BEGIN = 0xF1,
        FW_DOWNLOAD_CONTINUE = 0xF2,
        FW_CHIPERASE = 0xF3,
        FW_STARTAPP = 0xF4,
        FW_GETCRC32 = 0xF5,
        FW_GETVERSION = 0xF6,
    };

    enum system_status : uint8_t {
        SUCCESS = 0x00,
        UNKNOWN_HANDLE = 0x01,
        HANDLE_NOT_READY = 0x02,
        CORRUPT_FILE = 0x03,
        NO_HANDLES_AVAILABLE = 0x04,
        NO_PERMISSION = 0x05,
        ILLEGAL_PATH = 0x06,
        FILE_EXITS = 0x07,
        END_OF_FILE = 0x08,
        SIZE_ERROR = 0x09,
        UNKNOWN_ERROR = 0x0A,
        ILLEGAL_FILENAME = 0x0B,
        ILLEGAL_CONNECTION = 0x0C,
    };

    enum internal_error : uint8_t {
        INTERNAL_TXFAIL              = 0x15,
        INTERNAL_UNKNOWN_ERROR       = 0x1F,
        MESSAGE_TOO_LARGE            = 0x20,
    };

    enum parse_error : uint8_t {
        NO_ERROR,
        UNKNOWN_PARSE_ERROR,

        REPLY_SIZE_MISMATCH,
        REPLY_TYPE_MISMATCH,
        REPLY_CMD_MISMATCH,
        EARLY_EOF,
        LATE_EOF,
        LS_PARSE_FAILED,
        REPLY_TOO_LARGE,
        REPLY_TOO_SMALL

    };

    enum hid_errorcode : uint8_t {
        UNKNOWN_HID_ERROR,
        CONNECTION_FAILED,
        WRITE_FAILED,
        READ_FAILED,
        INIT_FAILED,
        EXIT_FAILED
    };

    template<typename Enum, typename Int = uint8_t>
    Int e2i(Enum val) {
        return static_cast<Int>(val);
    };

    template<typename Enum, typename Int = uint8_t>
    Enum i2e(Int val) {
        return static_cast<Enum>(val);
    };

    const char *parse_error_str(parse_error pe);
    i18n::locstring parse_error_desc(parse_error pe);
    const char *system_status_str(system_status st);
    i18n::locstring system_status_desc(system_status st);
    const char * hid_error_str(hid_errorcode ue);
    i18n::locstring hid_error_desc(hid_errorcode ue);
}

#endif//EV3_PROTO_H
