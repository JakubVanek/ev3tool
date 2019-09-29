//
// Created by kuba on 30.9.17.
//

#ifndef LIBEV3COMM_FILE_COMMANDS_HPP
#define LIBEV3COMM_FILE_COMMANDS_HPP

#include "commands.hpp"
#include "ev3proto.hpp"
#include "connection_ev3.hpp"

namespace ev3loader {
    namespace commands {

        // DOWNLOAD
        struct request_download_begin {
            request_download_begin() = default;

            system_commands cmd;
            size_t          file_length;
            std::string     file_name;
            buffer_view<>   file_payload;
        };

        struct request_download_continue {
            request_download_continue() = default;

            system_commands cmd;
            file_handle     file_id;
            buffer_view<>   file_payload;
        };

        struct reply_download {
            reply_download() = default;

            system_status result;
            file_shandle  file_id;
        };

        // UPLOAD
        struct request_upload_begin {
            request_upload_begin() = default;

            system_commands cmd;
            std::string     file_name;
            uint16_t        request_length;
        };

        struct request_upload_continue {
            request_upload_continue() = default;

            system_commands cmd;
            file_handle     file_id;
            size_t          file_length;
            uint16_t        request_length;
        };

        struct reply_upload {
            reply_upload() = default;

            system_status result;
            size_t        file_length;
            file_shandle  file_id;
            bytearray     file_payload;
        };

        // MKDIR/UNLINK
        enum class fileop_type {
            mkdir,
            unlink
        };
        struct request_fileop {
            std::string path;
            fileop_type what;
        };

        // HIGHLEVEL DOWNLOAD/UPLOAD
        enum class upload_type {
            regular,
            datalog,
            ls
        };
        struct request_download {
            std::string path;
            bytearray   contents;
        };

        struct request_upload {
            std::string path;
            upload_type op;
        };

        // HANDLE CLOSE
        struct request_handle_close {
            file_handle fd;
        };
        struct reply_handle_close {
            file_handle fd;
        };

        struct request_mailbox {
            std::string name;
            bytearray   payload;
        };

        enum class file_type {
            file,
            directory
        };

        struct request_ls {
            std::string path;
        };
        struct dir_entry {
            std::string             name;
            file_type               what;
            size_t                  size;
            std::array<uint8_t, 16> md5;
        };
        struct reply_ls {
            std::vector<dir_entry> listing;
            system_status          result;
        };
        struct request_setpin {
            uint8_t  clientMac[6];
            uint32_t brickPin;
        };
        struct reply_setpin {
            uint8_t  brickMac[6];
            uint32_t brickPin;
        };

        // COMMANDS
        class download_begin final
                : public data_command<download_begin, request_download_begin, reply_download> {
        public:
            download_begin() = default;

            explicit download_begin(packet_layer &conn,
                                    input_type const &rq)
                    : data_command(conn, rq) {}

            void send_impl();

        private:
            void on_receive(const bytearray &result);
        };

        class download_continue final
                : public data_command<download_continue, request_download_continue, reply_download> {
        public:
            download_continue() = default;

            explicit download_continue(packet_layer &conn,
                                       input_type const &rq)
                    : data_command(conn, rq) {}

            void send_impl();

        private:
            void on_receive(const bytearray &pkt);
        };

        class upload_begin final
                : public data_command<upload_begin, request_upload_begin, reply_upload> {
        public:
            upload_begin() = default;

            explicit upload_begin(packet_layer &conn,
                                  input_type const &rq)
                    : data_command(conn, rq) {}

            void send_impl();

        private:
            void on_receive(const bytearray &pkt);
        };

        class upload_continue final
                : public data_command<upload_continue, request_upload_continue, reply_upload> {
        public:
            upload_continue() = default;

            explicit upload_continue(packet_layer &conn,
                                     input_type const &rq)
                    : data_command(conn, rq) {}

            void send_impl();

        private:
            void on_receive(const bytearray &pkt);
        };

        // HIGHLEVEL COMMANDS
        class download_command final
                : public data_command<download_command, request_download, reply_download>,
                  public simple_progress_reporter {
        public:
            download_command() = default;

            explicit download_command(packet_layer &conn, input_type const &rq);
            explicit download_command(packet_layer &conn, input_type const &rq, progresscallback &&info);

            void send_impl();

        private:
            void on_receive(reply_download rpl);

            void send_next();

            system_commands m_state = system_commands::COMMAND_ERROR;
            buffer_view<>   m_window;
            size_t m_done;
            size_t m_steps;
        };

        class upload_command final
                : public data_command<upload_command, request_upload, reply_upload>,
                  public simple_progress_reporter {
        public:
            upload_command() = default;

            explicit upload_command(packet_layer &conn, input_type const &rq);
            explicit upload_command(packet_layer &conn, input_type const &rq, progresscallback &&info);

            void send_impl();

        private:
            void send_next();

            void receive_first(reply_upload rpl);

            void receive_later(reply_upload rpl);

            void refresh_length(reply_upload &rpl);

            void progress_update();

            system_commands m_state = system_commands::COMMAND_ERROR;
            buffer_view<>   m_window;
            size_t m_done;
            size_t m_steps;
        };

        class fileop_command final
                : public data_command<fileop_command, request_fileop, reply_simple> {
        public:
            fileop_command() = default;

            explicit fileop_command(packet_layer &conn, input_type const &rq)
                    : data_command(conn, rq) {}

            void send_impl();

        private:
            void on_receive(const bytearray &pkt);
        };

        class ls_command final : public data_command<ls_command, request_ls, reply_ls> {
        public:
            ls_command() = default;

            explicit ls_command(packet_layer &conn, input_type const &rq)
                    : data_command(conn, rq) {}

            void send_impl();

        private:
            void on_receive(reply_upload rpl);
        };

        class fwupdate_command final
                : public command<fwupdate_command, dummy> {
        public:
            fwupdate_command() = default;

            explicit fwupdate_command(packet_layer &conn) : command(conn) {}

            void send_impl();
        };

        class closehandle_command final
                : public data_command<closehandle_command, request_handle_close, reply_handle_close> {
        public:
            closehandle_command() = default;

            explicit closehandle_command(packet_layer &conn, input_type const &rq)
                    : data_command(conn, rq) {}

            void send_impl();

        private:
            void on_receive(const bytearray &pkt);
        };

        class write_mailbox_command final
                : public data_command<write_mailbox_command, request_mailbox, dummy> {
        public:
            write_mailbox_command() = default;

            explicit write_mailbox_command(packet_layer &conn, input_type const &rq)
                    : data_command(conn, rq) {}

            void send_impl();
        };

        class bluetooth_pin_command final
                : public data_command<bluetooth_pin_command, request_setpin, reply_setpin> {
        public:
            bluetooth_pin_command() = default;

            explicit bluetooth_pin_command(packet_layer &conn, input_type const &rq)
                    : data_command(conn, rq) {}

            void send_impl();

        private:
            void on_receive(const bytearray &pkt);
        };
    };
}

#endif //LIBEV3COMM_FILE_COMMANDS_HPP
