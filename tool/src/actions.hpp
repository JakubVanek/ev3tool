//
// Created by kuba on 11.8.18.
//

#ifndef EV3COMM_ACTIONS_HPP
#define EV3COMM_ACTIONS_HPP

#include "cmdline_parser.hpp"

namespace ev3tool {
    namespace program {
        class help_action : public action {
        public:
            using arglist = std::vector<std::string>;

            help_action(const arglist &flags, const arglist &commands);

            void perform(context &ctx) override;

        private:
            arglist m_flags;
            arglist m_commands;
        };

        class version_action : public action {
        public:
            version_action();

            void perform(context &ctx) override;
        };

        class upload_action : public action {
        public:
            upload_action(const std::string &from, const std::string &to);

            void perform(context &ctx) override;

            uint8_t transfer(context &ctx);

            void close(context &ctx, uint8_t handle);

            void beep(context &ctx);

        private:
            std::string m_from, m_to;
        };

        class download_action : public action {
        public:
            download_action(const std::string &from, const std::string &to);

            void perform(context &ctx) override;

        private:
            std::string m_from, m_to;
        };

        class ls_action : public action {
        public:
            ls_action(const std::string &from, bool md5);

            void perform(context &ctx) override;

        private:
            std::string m_dir;
            bool m_md5;
        };

        class rm_action : public action {
        public:
            explicit rm_action(const std::string &from);

            void perform(context &ctx) override;

        private:
            std::string m_path;
        };

        class mkdir_action : public action {
        public:
            explicit mkdir_action(const std::string &to);

            void perform(context &ctx) override;

        private:
            std::string m_path;
        };

        class closehandle_action : public action {
        public:
            explicit closehandle_action();

            void perform(context &ctx) override;
        };

        class listhandle_action : public action {
        public:
            explicit listhandle_action();

            void perform(context &ctx) override;
        };

        class closehandle_single_action : public action {
        public:
            explicit closehandle_single_action(ev3loader::file_handle handle);

            void perform(context &ctx) override;
        private:
            ev3loader::file_handle m_handle;
        };

        class mailbox_action : public action {
        public:
            explicit mailbox_action(const std::string &src_file, const std::string &dst_mbox);

            void perform(context &ctx) override;
        private:
            std::string m_source_file;
            std::string m_target_mailbox;
        };

        class btpin_action : public action {
        public:
            explicit btpin_action(const std::string &client_mac, const std::string &target_pin);

            void perform(context &ctx) override;
        private:
            std::string m_client_mac;
            std::string m_target_pin;
        };

        class hid_connect : public action {
        public:
            explicit
            hid_connect(ev3loader::hid_address addr);

            void perform(context &ctx) override;

        private:
            ev3loader::hid_address find_address();

            ev3loader::hid_address m_address;
        };

        class hid_list : public action {
        public:
            hid_list();

            void perform(context &ctx) override;
        };

        class fwdl_action : public action {
        public:
            explicit fwdl_action(const std::string &from);

            void perform(context &ctx) override;

            void handle_file();

            void handle_hwversion();

            void handle_erase();

            void handle_flash();

            void handle_checksum();

        private:
            std::string m_from;
            context *m_ctx = nullptr;
            std::vector<uint8_t> m_fw = {};
            uint32_t m_fw_crc32 = 0;

            void handle_reboot();
        };

        class fwenter_action : public action {
        public:
            fwenter_action();

            void perform(context &ctx) override;
        };

        class fwversion_action : public action {
        public:
            fwversion_action();

            void perform(context &ctx) override;
        };

        class fwexit_action : public action {
        public:
            fwexit_action();

            void perform(context &ctx) override;
        };
    }
}

#endif //EV3COMM_ACTIONS_HPP
