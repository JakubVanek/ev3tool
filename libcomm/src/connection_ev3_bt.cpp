#if 0
#include <cassert>
#include <iostream>
#include <atomic>
#include "connection_ev3_bt.hpp"
#include <unistd.h>

using namespace ev3loader;

rfcomm_link::rfcomm_link(rfcomm_address addr) : m_addr(addr) {

}

void rfcomm_link::write(nativewindow &in) {

}

void rfcomm_link::read(nativewindow &out, std::chrono::milliseconds timeout) {

}

bool rfcomm_link::nonblock_check() const {
    return false;
}

void rfcomm_link::nonblock_set(bool enabled) {

}

rfcomm_link::~rfcomm_link() {
    if (m_dev >= 0) {
        close(m_dev);
    }
    if (m_sock >= 0) {
        close(m_sock);
    }
}
#endif
