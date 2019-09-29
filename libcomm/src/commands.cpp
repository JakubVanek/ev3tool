//
// Created by kuba on 28.7.18.
//

#include "commands.hpp"
#include "bytereader.hpp"

namespace ev3loader {
    void assert_system_reply(const reply_system_header &hdr) {
        parse_assert(hdr.type() == SYS_RECV_REPLY ||
                     hdr.type() == SYS_RECV_REPLYERROR,
                     REPLY_TYPE_MISMATCH);
    }

    void assert_system_reply_ok(const reply_system_header &hdr, system_commands out) {
        assert_system_reply(hdr);
        assert_throw<sys_remote_error>(hdr.type() == SYS_RECV_REPLY, hdr.command(), hdr.result());
        parse_assert(hdr.command() == out, REPLY_CMD_MISMATCH);
    }

    void assert_command_success(const reply_system_header &hdr, system_commands out) {
        assert_system_reply_ok(hdr, out);
        assert_throw<sys_remote_error>(hdr.result() == SUCCESS, hdr.command(), hdr.result());
    }

    simple_progress_reporter::simple_progress_reporter() : m_progresscall(nullptr) {}
    simple_progress_reporter::simple_progress_reporter(progresscallback &&info) : m_progresscall(std::move(info)) {}

    void simple_progress_reporter::set_progress_callback(progresscallback &&callback) {
        m_progresscall = std::move(callback);
    }

    void simple_progress_reporter::report_progress(size_t done, size_t total) {
        if (m_progresscall) {
            m_progresscall(done, total);
        }
    }

    abstract_progress_reporter::abstract_progress_reporter() = default;

    abstract_progress_reporter::~abstract_progress_reporter() = default;
}