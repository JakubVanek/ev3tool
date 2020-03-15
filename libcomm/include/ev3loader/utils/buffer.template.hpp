//
// Created by kuba on 5.7.18.
//

#include "buffer.hpp"

template<typename A, typename B>
ev3loader::utils::buffer_view<A, B>::buffer_view(iterator_type begin,
                                          iterator_type end)
        : begin(begin), iter(begin), end(end) {}

template<typename A, typename B>
size_t ev3loader::utils::buffer_view<A, B>::common_length(const view_type &other) {
    size_t oLen = other.available();
    size_t tLen = this->available();
    return std::min(tLen, oLen);
}

template<typename A, typename B>
size_t ev3loader::utils::buffer_view<A, B>::common_window(const view_type &other,
                                                   view_type &thisWin,
                                                   view_type &otherWin) {
    size_t len = common_length(other);
    thisWin  = this->subview(len);
    otherWin = other.subview(len);
    return len;
}

template<typename A, typename B>
size_t ev3loader::utils::buffer_view<A, B>::copy_in(const view_type &other) {
    view_type thisWindow, otherWindow;
    size_t    len = common_window(other, thisWindow, otherWindow);

    std::copy(otherWindow.iter, otherWindow.end, thisWindow.iter);
    consume(thisWindow);

    return len;
}

template<typename A, typename B>
size_t ev3loader::utils::buffer_view<A, B>::copy_out(const view_type &other) {
    view_type thisWindow, otherWindow;
    size_t    len = common_window(other, thisWindow, otherWindow);

    std::copy(thisWindow.iter, thisWindow.end, otherWindow.iter);
    consume(thisWindow);

    return len;
}

template<typename A, typename B>
void ev3loader::utils::buffer_view<A, B>::skip(size_t count) {
    iter += count;
}

template<typename A, typename B>
bool ev3loader::utils::buffer_view<A, B>::in_progress() const {
    return !empty();
}

template<typename A, typename B>
bool ev3loader::utils::buffer_view<A, B>::empty() const {
    return iter >= end;
}

template<typename A, typename B>
size_t ev3loader::utils::buffer_view<A, B>::available() const {
    return static_cast<size_t>(empty() ? 0 : end - iter);
}


template<typename BufferT, typename IteratorT>
size_t ev3loader::utils::buffer_view<BufferT, IteratorT>::total_size() const {
    return end - begin;
}


template<typename A, typename B>
size_t ev3loader::utils::buffer_view<A, B>::consumed() const {
    return iter - begin;
}

template<typename A, typename B>
typename ev3loader::utils::buffer_view<A, B>::view_type ev3loader::utils::buffer_view<A, B>::subview(size_t rq) const {
    if (available() > rq) {
        return view_type{iter, iter + rq};
    } else {
        return view_type{iter, end};
    }
}

template<typename A, typename B>
void ev3loader::utils::buffer_view<A, B>::consume(const view_type &taken) {
    iter = taken.end;
}

template<typename A, typename B>
void ev3loader::utils::buffer_view<A, B>::reset() {
    iter = begin;
}
