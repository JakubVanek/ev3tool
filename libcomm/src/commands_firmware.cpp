//
// Created by kuba on 26.7.18.
//

#include "commands_firmware.hpp"
#include "bytewriter.hpp"
#include "bytereader.hpp"

#define FWDL_OVERHEAD 6
#define MAX_FW_PAYLOAD (FILE_BUF_SIZE - FWDL_OVERHEAD)

namespace ev3loader {
    namespace commands {

        //////////////////////
        // FIRMWARE VERSION //
        //////////////////////
        void fwversion_command::send_impl() {
            bytewriter out;
            out << as_system_header(SYS_SEND_REPLY, FW_GETVERSION);

            m_tx.buffer_send(out.buffer(), &fwversion_command::on_receive);
        }

        void fwversion_command::on_receive(const bytearray &pkt) {
            fwversion m_dataOut{};
            bytereader in(pkt);

            auto hdr = in.take<reply_system_header>();
            m_dataOut.hardwareVersion = in.take_uint32();
            m_dataOut.firmwareVersion = in.take_uint32();
            in.finalize();

            assert_command_success(hdr, FW_GETVERSION);
            async_success(std::move(m_dataOut));
        }

        ///////////////////
        // DATA CHECKSUM //
        ///////////////////
        void fwchecksum_command::send_impl() {
            bytewriter out;
            out << as_system_header(SYS_SEND_REPLY, FW_GETCRC32)
                << as_uint32(m_dataIn.address)
                << as_uint32(m_dataIn.length);

            m_tx.buffer_send(out.buffer(), &fwchecksum_command::on_receive);
        }

        void fwchecksum_command::on_receive(const bytearray &pkt) {
            bytereader in(pkt);

            auto hdr = in.take<reply_system_header>();
            auto crc32 = in.take_uint32();
            in.finalize();

            assert_system_reply_ok(hdr, FW_GETCRC32);

            m_dataOut.crc32 = crc32;
            async_success();
        }


        /////////////////
        // FLASH ERASE //
        /////////////////
        void fwerase_command::send_impl() {
            bytewriter out;
            out << as_system_header(SYS_SEND_REPLY, FW_CHIPERASE);

            m_tx.buffer_send(out.buffer(), &fwerase_command::on_receive);
        }

        void fwerase_command::on_receive(const bytearray &pkt) {
            bytereader in(pkt);

            auto hdr = in.take<reply_system_header>();
            in.finalize();

            assert_system_reply_ok(hdr, FW_CHIPERASE);
            async_success(reply_simple{hdr.result()});
        }

        ///////////////////
        // SYSTEM REBOOT //
        ///////////////////
        void fwstartapp_command::send_impl() {
            bytewriter out;
            out << as_system_header(SYS_SEND_REPLY, FW_STARTAPP);

            m_tx.buffer_send(out.buffer(), &fwstartapp_command::on_receive);
        }

        void fwstartapp_command::on_receive(const bytearray &pkt) {
            bytereader in(pkt);

            auto hdr = in.take<reply_system_header>();
            in.finalize();

            assert_system_reply_ok(hdr, FW_STARTAPP);
            async_success(reply_simple{hdr.result()});
        }

        /////////////////////////////
        // FIRMWARE DOWNLOAD BEGIN //
        /////////////////////////////
        void fwbegin_download::send_impl() {
            bytewriter out;
            out << as_system_header(SYS_SEND_REPLY, FW_DOWNLOAD_BEGIN)
                << as_uint32(m_dataIn.address)
                << as_uint32(m_dataIn.length);

            m_tx.buffer_send(out.buffer(), &fwbegin_download::on_receive);
        }

        void fwbegin_download::on_receive(const bytearray &pkt) {
            bytereader in(pkt);

            auto hdr = in.take<reply_system_header>();
            m_dataOut.result = hdr.result();
            in.finalize();

            assert_system_reply_ok(hdr, FW_DOWNLOAD_BEGIN);
            async_success();
        }

        ////////////////////////////
        // FIRMWARE DOWNLOAD NEXT //
        ////////////////////////////
        void fwcontinue_download::send_impl() {
            bytewriter out;
            out << as_system_header(SYS_SEND_REPLY, FW_DOWNLOAD_CONTINUE)
                << as_bytes(m_dataIn.data);

            m_tx.buffer_send(out.buffer(), &fwcontinue_download::on_receive);
        }

        void fwcontinue_download::on_receive(const bytearray &pkt) {
            bytereader in(pkt);

            auto hdr = in.take<reply_system_header>();
            m_dataOut.result = hdr.result();
            in.finalize();

            assert_system_reply_ok(hdr, FW_DOWNLOAD_CONTINUE);
            async_success();
        }

        ///////////////////////
        // FIRMWARE DOWNLOAD //
        ///////////////////////
        fw_download::fw_download(packet_layer &conn, request_fwdl in)
                : fw_download(conn, std::move(in), nullptr) {
        }

        fw_download::fw_download(packet_layer &conn, request_fwdl in, progresscallback &&info)
                : simple_progress_reporter(std::move(info)),
                  data_command(conn, in), m_window(m_dataIn.firmware),
                  m_done(0), m_steps(1 + (m_dataIn.firmware.size() + MAX_FW_PAYLOAD - 1) / MAX_FW_PAYLOAD),
                  m_state(system_commands::FW_DOWNLOAD_BEGIN) {
        }

        void fw_download::send_impl() {
            m_state = system_commands::FW_DOWNLOAD_BEGIN;

            request_fwdlbegin payload = {};
            payload.address = m_dataIn.address;
            payload.length = static_cast<uint32_t>(m_dataIn.firmware.size());

            send_subcommand<fwbegin_download>(&fw_download::on_receive, payload);
        }

        void fw_download::send_next() {
            m_state = system_commands::FW_DOWNLOAD_CONTINUE;

            request_fwdlcontinue payload;
            payload.data = m_window.subview(MAX_FW_PAYLOAD);

            send_subcommand<fwcontinue_download>(&fw_download::on_receive, payload);
            m_window.consume(payload.data);
        }

        void fw_download::on_receive(reply_simple reply) {
            report_progress(++m_done, m_steps);
            switch (reply.result) {
                case SUCCESS:
                case END_OF_FILE:
                    if (m_window.empty()) {
                        m_dataOut.result = reply.result;
                        async_success();
                    } else {
                        send_next();
                    }
                    return;
                default:
                    throw sys_remote_error(m_state, reply.result);
            }
        }
    }
}