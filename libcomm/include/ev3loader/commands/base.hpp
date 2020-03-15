//
// Created by kuba on 30.9.17.
//

#ifndef LIBEV3COMM_COMMANDS_HPP
#define LIBEV3COMM_COMMANDS_HPP

#include <future>
#include <exception>
#include <iostream>
#include <ev3loader/config.hpp>
#include <ev3loader/ev3proto.hpp>
#include <ev3loader/utils/bytereader.hpp>
#include <ev3loader/utils/bytewriter.hpp>
#include <ev3loader/comm/base.hpp>
#include <ev3loader/utils/buffer.hpp>

using std::placeholders::_1;

namespace ev3loader {
    namespace commands {
        using ev3loader::comm::packet_layer;
        using ev3loader::comm::timeout_point;
        using std::chrono::milliseconds;
        using ev3loader::utils::bytewriter;
        using ev3loader::utils::bytereader;
        using ev3loader::utils::buffer_view;
        using ev3loader::utils::nativewindow;
        using ev3loader::utils::reply_system_header;
        using ev3loader::bytearray;

        template<typename DataOut>
        class command {
        public:
            using output_type = DataOut;

        protected:
            explicit command(packet_layer &conn)
                    : pConn(&conn),
                      due_by(std::chrono::steady_clock::now() + std::chrono::seconds(3)) {}

        public:
            command(packet_layer &conn, timeout_point due_by)
                    : pConn(&conn),
                      due_by(due_by) {}

            std::future<output_type> async() {
                return std::async(&command::run, this);
            }

            output_type sync() {
                return run();
            }

        protected:
            virtual output_type run() = 0;

            bytereader communicate(bytewriter &source) {
                bytearray data = source.finalize();
                return bytereader(pConn->exchange(std::move(data), due_by));
            }

            void just_send(bytewriter &source) {
                bytearray data = source.finalize();
                return pConn->write_only(std::move(data));
            }

            packet_layer *pConn;
            timeout_point due_by;
        };

        struct reply_simple {
            reply_simple() = default;

            system_status result;
        };

        namespace system {

        }

        namespace firmware {

        }

        namespace direct {

        }
    }
}

#endif //LIBEV3COMM_COMMANDS_HPP
