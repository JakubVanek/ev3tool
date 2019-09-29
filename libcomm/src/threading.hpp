//
// Created by kuba on 30.7.18.
//

#ifndef EV3COMM_THREADING_HPP
#define EV3COMM_THREADING_HPP

#include <thread>
#include <atomic>
#include <functional>

namespace ev3loader {

    class loop_thread {
    public:
        using handler_function = std::function<void()>;

        loop_thread(handler_function work, handler_function wakeup);

        virtual ~loop_thread();

        void stop();

        bool run();

    private:
        void loop();

        handler_function  m_dowork;
        handler_function  m_wakeup;
        std::atomic<bool> m_run;
        std::thread       m_thr;
    };
}

#endif //EV3COMM_THREADING_HPP
