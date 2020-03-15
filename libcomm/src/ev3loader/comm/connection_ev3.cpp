//
// Created by kuba on 15.01.20.
//

#include <ev3loader/comm/connection_ev3.hpp>

using namespace ev3loader;
using namespace ev3loader::comm;
using namespace ev3loader::utils;
using namespace ev3loader::i18n;
using namespace ev3loader::i18n::priv;
using namespace std::chrono;

ev3_connection::ev3_connection(std::unique_ptr<channel> link)
        : m_link(std::move(link)),
          m_lock(),
          m_msgid(0) {}

bytearray ev3_connection::read_only(timeout_point due_by) {
    std::lock_guard<std::mutex> guard(m_lock);
    return do_receive(due_by);
}

void ev3_connection::write_only(bytearray request) {
    std::lock_guard<std::mutex> guard(m_lock);
    do_push(std::move(request));
}

bytearray ev3_connection::exchange(bytearray request, timeout_point due_by) {
    std::lock_guard<std::mutex> guard(m_lock);

    // this operation has to be separate with the mutex locked all the time
    // - because the reply follows directly after the request, without permissible interleaved reads/writes
    pcnt_t reqid = do_push(std::move(request));
    bytearray reply = do_receive(due_by);

    if (msg_id(reply) == reqid)
        return reply;
    else
        throw communication_error(format(_("Mismatch between sent and received message ids"
                                           " (sent %d, received %d)"),
                                         reqid, msg_id(reply)), HERE);
}

pcnt_t ev3_connection::do_push(bytearray msg) {
    pcnt_t id = prepare_for_send(msg);
    nativewindow win(msg.data(), msg.data() + msg.size());

    while (win.in_progress())
        m_link->write(win, milliseconds::zero(), true);

    return id;
}

pcnt_t ev3_connection::prepare_for_send(bytearray &request) {
    pcnt_t id = m_msgid++;
    msg_size(request, request.size());
    msg_id(request, id);
    return id;
}

bytearray ev3_connection::do_receive(timeout_point due_by) {
    bytearray rxbuf(1024);
    nativewindow rxwin(rxbuf.data(), rxbuf.data() + rxbuf.size());

    while (rxwin.consumed() < 2)
        m_link->read(rxwin, prepare_timeout(due_by), true);

    bytearray message(msg_size(rxbuf));
    buffer_view<> raswin(message);
    raswin.copy_in(buffer_view<>(rxbuf.begin(), rxbuf.begin() + rxwin.consumed()));

    while (raswin.in_progress()) {
        rxwin.reset();
        m_link->read(rxwin, prepare_timeout(due_by), true);
        raswin.copy_in(buffer_view<>(rxbuf.begin(), rxbuf.begin() + rxwin.consumed()));
    }

    return message;
}

milliseconds ev3_connection::prepare_timeout(timeout_point due_by) const {
    auto now = steady_clock::now();
    auto remaining = due_by - now;
    auto millis = duration_cast<milliseconds>(remaining);

    if (millis > milliseconds::zero())
        return millis;
    else
        throw timeout_error("read has timed out", HERE);
}

