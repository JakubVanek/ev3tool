#include <ev3loader/commands/system/download_begin.hpp>

using namespace ev3loader;
using namespace ev3loader::utils;
using namespace ev3loader::commands;
using namespace ev3loader::commands::system;

download_begin::download_begin(packet_layer &conn,
                               std::string path,
                               size_t file_length,
                               timeout_point due_by)
        : command(conn, due_by),
          m_path(std::move(path)),
          m_file_size(file_length) {}

reply_download download_begin::run() {
    reply_download output = {};

    bytewriter out;
    out << as_system_header(SYS_SEND_REPLY, DOWNLOAD_BEGIN)
        << as_uint32(m_file_size)
        << as_string(std::move(m_path))
        << close_small{};

    bytereader in = command::communicate(out);

    auto hdr = in.take<reply_system_header>().verify(DOWNLOAD_BEGIN);
    output.result = hdr.result();
    output.file_id = in.take_handle();
    in.close();

    return output;
}
