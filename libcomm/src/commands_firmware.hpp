//
// Created by kuba on 26.7.18.
//

#ifndef EV3COMM_COMMANDS_FIRMWARE_HPP
#define EV3COMM_COMMANDS_FIRMWARE_HPP

#include "commands.hpp"

namespace ev3loader {
    namespace commands {
        struct fwversion {
            uint32_t hardwareVersion;
            uint32_t firmwareVersion;
        };

        struct request_checksum {
            uint32_t address;
            uint32_t length;
        };
        struct reply_checksum {
            uint32_t crc32;
        };
        struct request_fwdlbegin {
            uint32_t address;
            uint32_t length;
        };
        struct request_fwdlcontinue {
            buffer_view<> data;
        };

        struct request_fwdl {
            uint32_t  address;
            bytearray firmware;
        };

        class fwversion_command : public command<fwversion_command, fwversion> {
        public:
            using command::command;

            void send_impl();

        private:
            void on_receive(const bytearray &pkt);
        };

        class fwchecksum_command : public data_command<fwchecksum_command, request_checksum, reply_checksum> {
        public:
            using data_command::data_command;

            void send_impl();

        private:
            void on_receive(const bytearray &pkt);
        };

        class fwerase_command : public command<fwerase_command, reply_simple> {
        public:
            using command::command;

            void send_impl();

        private:
            void on_receive(const bytearray &pkt);
        };

        class fwstartapp_command : public command<fwstartapp_command, reply_simple> {
        public:
            using command::command;

            void send_impl();

        private:
            void on_receive(const bytearray &pkt);
        };

        class fwbegin_download : public data_command<fwbegin_download, request_fwdlbegin, reply_simple> {
        public:
            using data_command::data_command;

            void send_impl();

        private:
            void on_receive(const bytearray &pkt);
        };

        class fwcontinue_download : public data_command<fwcontinue_download, request_fwdlcontinue, reply_simple> {
        public:
            using data_command::data_command;

            void send_impl();

        private:
            void on_receive(const bytearray &pkt);
        };

        class fw_download : public data_command<fw_download, request_fwdl, reply_simple>,
                public simple_progress_reporter {
        public:
            fw_download(packet_layer &conn, request_fwdl in);
            fw_download(packet_layer &conn, request_fwdl in, progresscallback &&info);

            void send_impl();

        private:
            void send_next();

            void on_receive(reply_simple reply);

            buffer_view<>   m_window;
            system_commands m_state;
            size_t          m_done;
            const size_t    m_steps;
        };
    }
}

#endif //EV3COMM_COMMANDS_FIRMWARE_HPP
