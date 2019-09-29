//
// Created by kuba on 4.7.18.
//

#ifndef LIBEV3COMM_BUFFER_HPP
#define LIBEV3COMM_BUFFER_HPP

#include <type_traits>
#include <chrono>
#include "common.hpp"
#include "callback.hpp"

namespace ev3loader {
    using packet_callback = ev3loader::excallback<const bytearray &>;

    /**
     * <p>Transaction data.</p>
     *
     * <p>Transaction consists Tx-Rx packet pair with identical IDs.</p>
     */
    struct transaction {
        /**
         * <p>Create new empty transaction.</p>
         */
        transaction()
                : buffer(0), retfn(nullptr) {}

        /**
         * <p>Create new send-only transaction.</p>
         * @param pkt Packet data.
         */
        explicit transaction(bytearray buffer)
                : buffer(std::move(buffer)), retfn(nullptr), timeout(0) {}

        /**
         * <p>Create new send-only transaction.</p>
         * @param pkt Packet data.
         */
        explicit transaction(bytearray buffer, packet_callback retfn)
                : buffer(std::move(buffer)), retfn(retfn), timeout(0) {}

        /**
         * <p>Transaction transmit buffer.</p>
         *
         * <p>Packet ID is extracted from this buffer.</p>
         */
        bytearray                           buffer;
        /**
         * <p>Timeout for this transaction.</p>
         */
        std::chrono::steady_clock::duration timeout;

        packet_callback retfn;
    };

    template<typename BufferT = bytearray, typename IteratorT = typename BufferT::iterator>
    struct buffer_view {
        using buffer_type   = BufferT;
        using view_type     = buffer_view<BufferT, IteratorT>;
        using iterator_type = IteratorT;

        buffer_view() = default;

        explicit buffer_view(buffer_type &whole);

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

        size_t capacity() const;

        size_t consumed() const;

        view_type subview(size_t rq) const;

        void consume(view_type const &taken);

        iterator_type begin;
        iterator_type iter;
        iterator_type end;
    };

    using nativewindow = buffer_view<dummy, byte *>;
}

#include "buffer.template.hpp"

#endif //LIBEV3COMM_BUFFER_HPP
