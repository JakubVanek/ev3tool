#include <utility>

//
// Created by kuba on 30.9.17.
//

#include <cstring>
#include <cassert>
#include <functional>
#include "connection_ev3.hpp"
#include "config.hpp"

using namespace ev3loader;
using namespace ev3loader::i18n;
using namespace ev3loader::i18n::priv;
using namespace std::placeholders;

/////////////
// PACKETS //
/////////////

// Ctor & Dtor
ev3_connection::ev3_connection(std::unique_ptr<channel> link)
        : m_conn(std::move(link)),
          m_reg(m_output),
          m_output(m_reg, *m_conn),
          m_input(m_reg, *m_conn) {}

ev3_connection::~ev3_connection() = default;


// Managed mode

pcnt_t ev3_connection::push_packet(transaction &tx) {
    msg_size(tx.buffer, tx.buffer.size());
    pcnt_t id = m_reg.register_transaction(tx);

    m_output.push(tx.buffer);
    return id;
}

void ev3_connection::default_callback(packet_callback on_receive) {
    m_reg.set_default_callback(std::move(on_receive));
}

bookkeeper::record::record()
        : retfn(nullptr),
          time_last(),
          timeout(0),
          sent(false) {}

bookkeeper::record::record(const transaction &tsx)
        : retfn(tsx.retfn),
          time_last(clock::now()),
          timeout(std::chrono::duration_cast<duration>(tsx.timeout)),
          sent(false) {}

void bookkeeper::record::trigger() {
    time_last = clock::now();
}

bool bookkeeper::record::timed_out() const {
    return timeout.count() > 0 && (clock::now() - time_last) > timeout;
}

void bookkeeper::record::mark_sent() {
    sent = true;
}

bool bookkeeper::record::was_sent() const {
    return sent;
}

pcnt_t bookkeeper::register_transaction(transaction &tsx) {
    std::lock_guard<std::mutex> guard(m_maplock);

    pcnt_t id = m_counter++;
    msg_id(tsx, id);
    m_bookings[id] = record{tsx};
    return id;
}

void bookkeeper::trigger(pcnt_t id) {
    std::lock_guard<std::mutex> guard(m_maplock);

    auto it = m_bookings.find(id);
    if (it != m_bookings.end()) {
        it->second.trigger();
    }
}

packet_callback bookkeeper::get_call(pcnt_t id) {
    packet_callback             call;
    std::lock_guard<std::mutex> guard(m_maplock);
    auto                        it = m_bookings.find(id);
    if (it != m_bookings.end()) {
        call = it->second.retfn;
        m_bookings.erase(it);
        m_txqueue->wake_on_rx();
    } else {
        call = m_retfn_dfl;
    }
    return call;
}

void bookkeeper::finish(const bytearray &pkt) {
    pcnt_t id = msg_id(pkt);
    get_call(id)(pkt);
}

void bookkeeper::fail(packet_callback::failure_type ret) {
    intptr_t id = -1;
    try {
        std::rethrow_exception(ret);
    } catch (communication_error &ex) {
        if (ex.getExtraPurpose() == ExtraPurpose::PacketId) {
            id = ex.getExtra();
        }
    } catch (...) {}

    packet_callback call;
    if (id != -1) {
        auto cast = static_cast<pcnt_t>(id);

        call = get_call(cast);
    } else {
        call = m_retfn_dfl;
    }
    call(std::move(ret));
}

void bookkeeper::check_timeouts() {
    std::unique_lock<std::mutex> sleeper(m_sleeplock);
    m_sleeper.wait_for(sleeper, std::chrono::milliseconds(TIMEOUT_PERIOD_MS), [this] { return !m_thr.run(); });
    if (!m_thr.run())
        return;

    std::vector<std::pair<pcnt_t, packet_callback>> errors;

    {
        std::lock_guard<std::mutex> guard(m_maplock);

        auto it  = m_bookings.begin();
        auto end = m_bookings.end();
        for (; it != end; ++it) {
            if (it->second.timed_out()) {
                errors.emplace_back(it->first, it->second.retfn);
                m_bookings.erase(it);
            }
        }
    }

    for (auto entry : errors) {
        timeout_error err(_("Packet timed out."), HERE, entry.first, ExtraPurpose::PacketId);
        entry.second(std::move(err));
    }
}

void bookkeeper::set_default_callback(packet_callback call) {
    this->m_retfn_dfl = std::move(call);
}

bookkeeper::bookkeeper(txqueue &tx)
        : m_txqueue(&tx),
          m_retfn_dfl(nullptr),
          m_bookings(),
          m_counter(0),
          m_maplock(),
          m_sleeper(), m_sleeplock(),
          m_thr(std::bind(&bookkeeper::check_timeouts, this),
                [this] { m_sleeper.notify_all(); }) {}

void bookkeeper::mark_sent(pcnt_t id) {
    std::lock_guard<std::mutex> guard(m_maplock);

    auto it = m_bookings.find(id);
    if (it != m_bookings.end()) {
        it->second.mark_sent();
    }
}

bool bookkeeper::has_unfinished_read() {
    std::lock_guard<std::mutex> guard(m_maplock);

    auto it  = m_bookings.begin();
    auto end = m_bookings.end();
    for (; it != end; ++it) {
        if (it->second.was_sent()) {
            return true;
        }
    }
    return false;
}

rxqueue::rxqueue(bookkeeper &reg, channel &link)
        : m_rxbuf(EV3_RXBUF),
          m_rxwin(m_rxbuf.data(), m_rxbuf.data() + m_rxbuf.size()),
          m_rasbuf(0),
          m_raswin(m_rasbuf.begin(), m_rasbuf.end()),
          m_reg(&reg), m_link(&link),
          m_thr(std::bind(&rxqueue::dowork, this), nullptr) {
    m_link->nonblock_set(false);
}

rxqueue::~rxqueue() = default;

void rxqueue::dowork() {
    try {
        m_link->read(m_rxwin, std::chrono::milliseconds(READ_TIMEOUT_MS));
    } catch (communication_error &ex) {
        if (!m_raswin.empty()) {
            ex.setExtra(msg_id(m_rasbuf), ExtraPurpose::PacketId);
        }
        m_reg->fail(ex);
        return;
    } catch (...) {
        m_reg->fail(std::current_exception());
        return;
    }
    if (m_rxwin.consumed() != 0) {
        buffer_view<> consumed(m_rxbuf.begin(), m_rxbuf.begin() + m_rxwin.consumed());
        push(consumed);
        m_rxwin.reset();
    }
}

void rxqueue::push(buffer_view<> input) {
    if (m_raswin.empty()) {
        psize_t size = msg_size(input.iter, input.end);
        m_rasbuf.assign(size, 0);
        m_raswin = buffer_view<>{m_rasbuf};
    }

    m_raswin.copy_in(input);
    m_reg->trigger(msg_id(m_rasbuf));

    if (m_raswin.empty()) {
        m_reg->finish(m_rasbuf);
    }
}


txqueue::txqueue(bookkeeper &reg, channel &link)
        : m_sendlock(), m_sendqueue(), // send lock and queue
          m_notify(), // notify conditional variable
          m_reg(&reg), m_link(&link), // input/output
          m_thr(std::bind(&txqueue::dowork, this),
                [this] { m_notify.notify_all(); }) /* thread itself */ {}

txqueue::~txqueue() = default;

// lock[add data to queue] and then notify worker
void txqueue::push(bytearray pkt) {
    {
        std::lock_guard<std::mutex> lock(m_sendlock);
        m_sendqueue.emplace(pkt);
    }
    m_notify.notify_all();
}

// lock the lock, check for queue empty and wait for element/stop
txqueue::task *txqueue::current() {
    std::unique_lock<std::mutex> lock(m_sendlock);

    bool run;
    while ((run = m_thr.run()) && (m_sendqueue.empty() || m_reg->has_unfinished_read())) {
        m_notify.wait_for(lock, std::chrono::seconds(1));
    }
    return run ? &m_sendqueue.front() : nullptr;
}

// lock[pop queue front]
void txqueue::pop() {
    std::lock_guard<std::mutex> lock(m_sendlock);
    m_sendqueue.pop();
}

void txqueue::dowork() {
    task *io = current();
    if (io == nullptr) {
        return;
    }
    pcnt_t id = msg_id(io->m_sndbuf);
    try {
        while (!io->m_sndwindow.empty()) {
            m_link->write(io->m_sndwindow);
        }
        m_reg->mark_sent(id);
        m_reg->trigger(id);
    } catch (communication_error &ex) {
        ex.setExtra(id, ExtraPurpose::PacketId);
        m_reg->fail(ex);
    } catch (...) {
        m_reg->fail(std::current_exception());
    }
    pop();
}

void txqueue::wake_on_rx() {
    m_notify.notify_all();
}


txqueue::task::task(const bytearray &sndbuf)
        : m_sndbuf(sndbuf),
          m_sndwindow() {
    m_sndbuf.resize(std::max(m_sndbuf.size(), (size_t) EV3_RXBUF), 0);
    m_sndwindow = nativewindow(m_sndbuf.data(), m_sndbuf.data() + m_sndbuf.size());
}

txqueue::task::task()
        : m_sndbuf(0),
          m_sndwindow(m_sndbuf.data(),
                      m_sndbuf.data()) {}
