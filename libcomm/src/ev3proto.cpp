//
// Created by kuba on 30.9.17.
//

#include "ev3proto.hpp"
#include "localization.hpp"

using namespace ev3loader;
using namespace ev3loader::i18n::priv;

const char *ev3loader::system_status_str(system_status st) {
    switch (st) {
        case system_status::SUCCESS:
            return "SUCCESS";
        case system_status::UNKNOWN_HANDLE:
            return "UNKNOWN_HANDLE";
        case system_status::HANDLE_NOT_READY:
            return "HANDLE_NOT_READY";
        case system_status::CORRUPT_FILE:
            return "CORRUPT_FILE";
        case system_status::NO_HANDLES_AVAILABLE:
            return "NO_HANDLES_AVAILABLE";
        case system_status::NO_PERMISSION:
            return "NO_PERMISSION";
        case system_status::ILLEGAL_PATH:
            return "ILLEGAL_PATH";
        case system_status::FILE_EXITS:
            return "FILE_EXISTS";
        case system_status::END_OF_FILE:
            return "END_OF_FILE";
        case system_status::SIZE_ERROR:
            return "SIZE_ERROR";
        case system_status::UNKNOWN_ERROR:
            return "UNKNOWN_ERROR";
        case system_status::ILLEGAL_FILENAME:
            return "ILLEGAL_FILENAME";
        case system_status::ILLEGAL_CONNECTION:
            return "ILLEGAL_CONNECTION";
        default:
            return "NOT_DESCRIBED";
    }
}

ev3loader::i18n::locstring ev3loader::system_status_desc(system_status st) {
    switch (st) {
        case system_status::SUCCESS:
            return _("Success");
        case system_status::UNKNOWN_HANDLE:
            return _("Unknown handle");
        case system_status::HANDLE_NOT_READY:
            return _("Handle not ready");
        case system_status::CORRUPT_FILE:
            return _("File is corrupt");
        case system_status::NO_HANDLES_AVAILABLE:
            return _("No handles available");
        case system_status::NO_PERMISSION:
            return _("Operation not permitted");
        case system_status::ILLEGAL_PATH:
            return _("Illegal path");
        case system_status::FILE_EXITS:
            return _("File exists");
        case system_status::END_OF_FILE:
            return _("End of file reached");
        case system_status::SIZE_ERROR:
            return _("Size error");
        case system_status::UNKNOWN_ERROR:
            return _("Unknown error");
        case system_status::ILLEGAL_FILENAME:
            return _("Illegal filename");
        case system_status::ILLEGAL_CONNECTION:
            return _("Illegal connection");
        default:
            return _("Error not described in source");
    }
}

const char *ev3loader::parse_error_str(parse_error pe) {
    switch (pe) {
        case parse_error::NO_ERROR:
            return "NO_ERROR";
        case parse_error::UNKNOWN_PARSE_ERROR:
            return "UNKNOWN_ERROR";
        case parse_error::REPLY_SIZE_MISMATCH:
            return "REPLY_SIZE_MISMATCH";
        case parse_error::REPLY_TYPE_MISMATCH:
            return "REPLY_TYPE_MISMATCH";
        case parse_error::REPLY_CMD_MISMATCH:
            return "REPLY_CMD_MISMATCH";
        case parse_error::EARLY_EOF:
            return "EARLY_EOF";
        case parse_error::LATE_EOF:
            return "LATE_EOF";
        case parse_error::LS_PARSE_FAILED:
            return "LS_PARSE_FAILED";
        case REPLY_TOO_LARGE:
            return "REPLY_TOO_LARGE";
        case REPLY_TOO_SMALL:
            return "REPLY_TOO_SMALL";
        default:
            return "NOT_DESCRIBED";
    }
}

ev3loader::i18n::locstring ev3loader::parse_error_desc(parse_error pe) {
    switch (pe) {
        case parse_error::NO_ERROR:
            return _("No error");
        case parse_error::UNKNOWN_PARSE_ERROR:
            return _("Unknown error");
        case parse_error::REPLY_SIZE_MISMATCH:
            return _("Reply size mismatch");
        case parse_error::REPLY_TYPE_MISMATCH:
            return _("Reply type mismatch");
        case parse_error::REPLY_CMD_MISMATCH:
            return _("Reply command mismatch");
        case parse_error::EARLY_EOF:
            return _("Unexpected EOF");
        case parse_error::LATE_EOF:
            return _("EOF expected");
        case parse_error::LS_PARSE_FAILED:
            return _("Directory listing parse failed");
        case REPLY_TOO_LARGE:
            return _("Reply too large");
        case REPLY_TOO_SMALL:
            return _("Reply too small");
        default:
            return _("Error not described in source");
    }
}

const char *ev3loader::hid_error_str(hid_errorcode ue) {
    switch (ue) {
        case hid_errorcode::UNKNOWN_HID_ERROR:
            return "UNKNOWN_ERROR";
        case hid_errorcode::CONNECTION_FAILED:
            return "CONNECTION_FAILED";
        case hid_errorcode::WRITE_FAILED:
            return "WRITE_FAILED";
        case hid_errorcode::READ_FAILED:
            return "READ_FAILED";
        case hid_errorcode::INIT_FAILED:
            return "INIT_FAILED";
        case hid_errorcode::EXIT_FAILED:
            return "EXIT_FAILED";
        default:
            return "NOT_DESCRIBED";
    }
}

ev3loader::i18n::locstring ev3loader::hid_error_desc(hid_errorcode ue) {
    switch (ue) {
        case hid_errorcode::UNKNOWN_HID_ERROR:
            return _("Unknown error");
        case hid_errorcode::CONNECTION_FAILED:
            return _("Connection failed");
        case hid_errorcode::WRITE_FAILED:
            return _("Write failed");
        case hid_errorcode::READ_FAILED:
            return _("Read failed");
        case hid_errorcode::INIT_FAILED:
            return _("HIDAPI initialization failed");
        case hid_errorcode::EXIT_FAILED:
            return _("HIDAPI deinitialization failed");
        default:
            return _("Error not described in source");
    }
}
