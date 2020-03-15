//
// Created by kuba on 4.7.18.
//

#ifndef LIBEV3COMM_BUFFER_HPP
#define LIBEV3COMM_BUFFER_HPP

#include <type_traits>
#include <chrono>
#include <future>
#include <ev3loader/common.hpp>

namespace ev3loader {
    namespace utils {
        template<typename BufferT = bytearray,
                typename IteratorT = typename BufferT::iterator>
        struct buffer_view {
            using buffer_type   = BufferT;
            using view_type     = buffer_view<BufferT, IteratorT>;
            using iterator_type = IteratorT;

            buffer_view() = default;

            template<typename T = BufferT>
            explicit
            buffer_view(typename std::enable_if_t<!std::is_same<T, void>::value, T> &whole)
                    : begin(whole.begin()), iter(whole.begin()), end(whole.end()) {}

            explicit buffer_view(iterator_type begin, iterator_type end);

            size_t common_length(view_type const &other);

            size_t common_window(view_type const &other,
                                 view_type &thisWin, view_type &otherWin);

            size_t copy_in(view_type const &other);

            size_t copy_out(view_type const &other);

            void skip(size_t count);

            bool in_progress() const;

            bool empty() const;

            void reset();

            size_t total_size() const;

            size_t available() const;

            size_t consumed() const;

            view_type subview(size_t rq) const;

            void consume(view_type const &taken);

            iterator_type begin;
            iterator_type iter;
            iterator_type end;
        };

        using nativewindow = buffer_view<void, byte *>;
    }
}

#include "buffer.template.hpp"

#endif //LIBEV3COMM_BUFFER_HPP
