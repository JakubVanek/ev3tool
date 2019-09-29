//
// Created by kuba on 30.7.18.
//

#include "threading.hpp"

namespace ev3loader {

    loop_thread::loop_thread(handler_function work, handler_function wakeup)
            : m_dowork(std::move(work)), m_wakeup(std::move(wakeup)),
              m_run(true), m_thr(&loop_thread::loop, this) {
        if (!m_dowork) {
            throw std::logic_error("Some work is required for loop thread");
        }
    }

    loop_thread::~loop_thread() {
        stop();
    }

    void loop_thread::stop() {
        m_run.store(false, std::memory_order_release);
        if (m_wakeup) {
            m_wakeup();
        }
        if (m_thr.joinable()) {
            m_thr.join();
        }
    }

    void loop_thread::loop() {
        while (run()) {
            m_dowork();
        }
    }

    bool loop_thread::run() {
        return m_run.load(std::memory_order::memory_order_acquire);
    }
}