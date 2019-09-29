//
// Created by kuba on 23.7.18.
//

#include "commands_direct.hpp"
#include "bytewriter.hpp"
#include "bytereader.hpp"

namespace ev3loader {
    namespace commands {
        direct_command::direct_command(packet_layer &conn, const request_direct &in)
                : data_command(conn, in) {

        }

        void direct_command::send_impl() {
            if (m_dataIn.locals > 0x3F)
                throw std::logic_error("Too many locals in a direct command!");
            if (m_dataIn.globals > 0x3FF)
                throw std::logic_error("Too many globals in a direct command!");

            bytewriter out;

            out << as_size(0);
            out << as_id(0);
            switch (m_dataIn.type) {
                case direct_command_type::WITH_REPLY:
                    out << as_type(DIR_SEND_REPLY);
                    break;
                case direct_command_type::WITHOUT_REPLY:
                    out << as_type(DIR_SEND_NOREPLY);
                    break;
            }
            auto first  = static_cast<uint8_t>(m_dataIn.globals);
            auto second = static_cast<uint8_t>((m_dataIn.globals >> 8) & 0x03);
            second |= m_dataIn.locals << 2;
            out << as_uint8(first);
            out << as_uint8(second);
            out << as_bytes(m_dataIn.bytecode);
            assert_buffer_large_ok(out.size());
            m_tx.buffer_send(out.buffer(), &direct_command::on_receive);

            if (m_dataIn.type == direct_command_type::WITHOUT_REPLY) {
                async_success();
                return;
            }
        }

        void direct_command::on_receive(const bytearray &pkt) {
            bytereader in(pkt);

            auto size = in.take_size();
            auto id   = in.take_id();
            m_dataOut.result  = in.take_type() == DIR_RECV_REPLY ?
                                direct_command_result::SUCCESS :
                                direct_command_result::FAILURE;
            m_dataOut.globals = in.take_buffer();
            in.finalize();

            async_success();
        }
    }
}
