//
// Created by kuba on 30.7.18.
//

#ifndef EV3COMM_CMDLINE_PARSER_HPP
#define EV3COMM_CMDLINE_PARSER_HPP

#include <connection.hpp>
#include <memory>
#include <list>
#include <unordered_map>

namespace ev3tool {
    namespace program {
        using argument_storage = std::unordered_map<std::string, std::string>;
        using argument_list = std::vector<std::string>;

        enum output_level {
            OUTPUT_VERBOSE,
            OUTPUT_NORMAL,
            OUTPUT_QUIET
        };

// connection
        struct context {
            std::unique_ptr<ev3loader::packet_layer> connection;
            output_level                             verbosity;
        };

        class invalid_argument_exception : public std::exception {
        public:
            invalid_argument_exception();

            explicit invalid_argument_exception(std::string &&reason);

            const char *what() const noexcept override;

        private:
            std::string m_reason;
        };

// operation
        class action {
        public:
            virtual ~action();

            virtual void perform(context &ctx) = 0;
        };

        class bootstrap {
        public:
            virtual ~bootstrap();

            virtual void process(int argc, char **argv) = 0;

            virtual std::unique_ptr<action> take_action() = 0;

            virtual output_level get_verbosity() = 0;
        };

        class composite_action : public action {
        public:
            composite_action();

            ~composite_action() override;

            void append(std::unique_ptr<action> action);

            void clear();

            void perform(context &ctx) override;

        private:
            std::list<std::unique_ptr<action>> m_list;
        };

        class flag {
        public:
            flag(const std::string &key);

            virtual ~flag();

            virtual bool process(const argument_list &args, int &idx, argument_storage &stor) = 0;

        protected:
            std::string m_key;
        };


        class counter_flag : public flag {
        public:
            counter_flag(const std::string &key, const std::string &shrt, const std::string &lng, int change);

            bool process(const argument_list &args, int &idx, argument_storage &stor) override;

        protected:
            int         m_change;
            std::string m_short;
            std::string m_long;
        };

        class string_flag : public flag {
        public:
            string_flag(const std::string &key, const std::string &shrt, const std::string &lng);

            bool process(const argument_list &args, int &idx, argument_storage &stor) override;

        protected:
            std::string m_short;
            std::string m_long;
        };

        class command_flag : public flag {
        public:
            command_flag(const std::string &key, const std::string &name);

            bool process(const argument_list &args, int &idx, argument_storage &stor) override;

        protected:
            std::string m_name;
        };


        class ev3tool_bootstrap : public bootstrap {
        public:
            ev3tool_bootstrap();

            void process(int argc, char **argv) override;

            std::unique_ptr<action> take_action() override;

            output_level get_verbosity() override;

        private:
            enum loop_state {
                loop_continue,
                loop_break
            };
            void add_counter_arg(const std::string &key, const std::string &shrt, const std::string &lng, int change,
                                 const std::string &desc);

            void add_string_arg(const std::string &key, const std::string &shrt, const std::string &lng,
                                const std::string &desc);

            void add_syntesizer(const std::string &key, std::function<loop_state(argument_storage &)> &&fn);

            void add_cmd_arg(const std::string &key, const std::string &name, const std::string &desc);

            void push_normal_help(std::string shrt, std::string lng, const std::string &desc, bool value);

            void push_cmd_help(const std::string &name, const std::string &desc);

            argument_list stringize(int argc, char **argv);

            argument_storage analyze(argument_list &args);

            void synthesize(argument_storage &args);

            void setup_connect(argument_storage &args, int vid = -1, int pid = -1);

            void append_composite(std::unique_ptr<action> act);

            std::unordered_map<std::string, std::function<loop_state(argument_storage &)>> m_synths;
            std::vector<std::string>           m_flag_help;
            std::vector<std::string>           m_cmd_help;
            std::vector<std::unique_ptr<flag>> m_flags;
            std::unique_ptr<action>            m_root;
            output_level                       m_lvl;
        };
    }
}

#endif //EV3COMM_CMDLINE_PARSER_HPP
