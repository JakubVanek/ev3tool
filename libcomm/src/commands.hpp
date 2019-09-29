//
// Created by kuba on 30.9.17.
//

#ifndef LIBEV3COMM_COMMANDS_HPP
#define LIBEV3COMM_COMMANDS_HPP

#include <future>
#include <exception>
#include <iostream>
#include "connection.hpp"
#include "config.hpp"
#include "ev3proto.hpp"

using std::placeholders::_1;

namespace ev3loader {


    template<typename ThisT, typename RetT, typename ArgT>
    std::function<RetT(ArgT)> bind_member(ThisT *dthis, RetT (ThisT::*mfn)(ArgT)) {
        return std::bind(mfn, dthis, _1);
    }

    template<typename DerivedT, typename... Args>
    DerivedT *make_freestanding(Args &&...args);


    template<typename OwnerT>
    class byte_sender {
    public:
        byte_sender() = default;

        explicit byte_sender(packet_layer &conn, OwnerT &parent) : m_conn(&conn), m_parent(&parent) {}

        void buffer_send(bytearray &payload) {
            transaction tx(payload, nullptr);
            m_conn->push_packet(tx);
        }

        void buffer_send(bytearray &payload, std::function<void(const bytearray &)> &&fn) {
            auto wrap_fn = m_parent->make_exception_wrapper(std::move(fn));

            transaction tx(payload, std::move(wrap_fn));
            m_conn->push_packet(tx);
        }

        void buffer_send(bytearray &m_txbuf, void (OwnerT::*mfn)(const bytearray &)) {
            auto bind_fn = bind_member<>(m_parent, mfn);
            buffer_send(m_txbuf, std::move(bind_fn));
        }

        packet_layer &connection() {
            return *m_conn;
        }

    protected:
        OwnerT *m_parent;
        packet_layer *m_conn = nullptr;
    };

    template<typename OutT>
    class promise_holder {
    public:
        using output_type  = OutT;
        using exception_type  = std::exception_ptr;
        using outcall_type = ev3loader::callback<output_type, exception_type>;
        using promise_type = std::promise<output_type>;
        using promise_ptr  = std::unique_ptr<promise_type>;
        using future_type  = std::future<output_type>;

        promise_holder() : m_wrapper{bind_member(this, &promise_holder::on_success),
                                     bind_member(this, &promise_holder::on_failure)},
                           m_promise{nullptr} {}

        future_type make_promise() {
            reset();
            m_promise = std::make_unique<promise_type>();
            return m_promise->get_future();
        }

        outcall_type get_callback() {
            return m_wrapper;
        }

        bool promiseMade() {
            return m_promise != nullptr;
        }

        void reset() {
            if (promiseMade()) {
                m_promise->set_exception(abort_error{i18n::priv::_("Promise was cancelled"), HERE});
                m_promise.reset();
            }
        }

    private:
        void on_success(typename outcall_type::success_type arg) {
            if (!m_promise) {
                std::cerr << i18n::priv::_("WARNING: c++ promise bridge called twice") << std::endl;
                return;
            }
            try {
                m_promise->set_value(std::forward<typename outcall_type::success_type>(arg));
                m_promise.reset();
            } catch (std::future_error &err) {
                std::cerr << i18n::priv::_("WARNING: c++ promise error: ") << err.what() << std::endl;
            }
        }

        void on_failure(typename outcall_type::failure_type arg) {
            if (!m_promise) {
                std::cerr << i18n::priv::_("WARNING: c++ promise bridge called twice") << std::endl;
                return;
            }
            try {
                m_promise->set_exception(arg);
                m_promise.reset();
            } catch (std::future_error &err) {
                std::cerr << i18n::priv::_("WARNING: c++ promise error: ") << err.what() << std::endl;
            }
        }

        outcall_type m_wrapper;
        promise_ptr m_promise;
    };

    template<typename DerivedT, typename OutT>
    class pusher {
    public:
        using output_type    = OutT;
        using derived_type   = DerivedT;
        using exception_type = std::exception_ptr;
        using outcall_type   = ev3loader::callback<output_type, exception_type>;
        using future_type    = std::future<output_type>;
        using promise_type   = std::promise<output_type>;
        using promise_ptr    = std::shared_ptr<promise_type>;

        pusher() = default;

        virtual ~pusher() = default;

        void set_callback(outcall_type &&fn) {
            m_promise.reset();
            m_callback = std::move(fn);
        }

        template<typename DerivedT2, typename... Args>
        friend DerivedT2 *make_freestanding(Args &&...args);

        future_type
        make_promise() {
            m_callback = m_promise.get_callback();
            return m_promise.make_promise();
        }

    protected:
        void async_success(output_type &&out) noexcept(true) {
            try {
                m_callback(std::move(out));
            } catch (...) {
                std::cerr << i18n::priv::_("WARNING: c++ success callback error") << std::endl;
            }
            memory_release();
        }

        void async_fail(exception_type err) noexcept(true) {
            try {
                m_callback(std::move(err));
            } catch (...) {
                std::cerr << i18n::priv::_("WARNING: c++ failure callback error") << std::endl;
            }
            memory_release();
        }

        template<typename ReplyT>
        excallback<ReplyT>
        make_exception_wrapper(std::function<void(ReplyT)> &&onSuccess) {
            using CallT = excallback<ReplyT>;
            auto dthis = static_cast<DerivedT *>(this);

            typename CallT::failure_functor_type
                    onfail = [dthis](typename CallT::failure_type arg) -> void {
                dthis->async_fail(arg);
            };

            typename CallT::success_functor_type
                    successWrap = [dthis, real{std::move(onSuccess)}](typename CallT::success_type arg) -> void {
                try {
                    real(std::forward<typename CallT::success_type>(arg));
                } catch (...) {
                    dthis->async_fail(std::current_exception());
                }
            };
            return CallT{std::move(successWrap), std::move(onfail)};
        }

        template<typename ReplyT>
        excallback<ReplyT> make_exception_wrapper(void (DerivedT::*mfn)(ReplyT)) {
            auto dthis = static_cast<DerivedT *>(this);
            auto bound = bind_member(static_cast<DerivedT *>(dthis), mfn);
            return make_exception_wrapper < ReplyT > (std::move(bound));
        }

    private:
        void memory_claim() {
            m_selfref.reset(static_cast<DerivedT *>(this));
        }

        void memory_release() {
            m_selfref.reset();
        }

        promise_holder<OutT> m_promise;
        std::unique_ptr<derived_type> m_selfref{};
        outcall_type m_callback{};
    };

    template<typename DerivedT, typename... Args>
    DerivedT *make_freestanding(Args &&... args) {
        auto *raw = new DerivedT(std::forward<Args>(args)...);
        raw->memory_claim();
        return raw;
    }

    template<typename DerivedT, typename... Args>
    typename DerivedT::future_type async_command(Args &&... args) {
        auto raw = make_freestanding<DerivedT, Args...>(std::forward<Args>(args)...);
        auto future = raw->make_promise();
        raw->send();
        return future;
    }

    /**
     * Device command.
     *
     * Command is composed of one or more packets.
     */
    template<typename DerivedT, typename OutT>
    class command : public pusher<DerivedT, OutT> {
    public:
        command() = default;

        command(packet_layer &conn) : m_tx(conn, *static_cast<DerivedT *>(this)) {}

        friend class byte_sender<DerivedT>;

        /**
         * Start processing the command through a connection.
         * @param conn Connection to interact with.
         * @return Whether the operation has succeeded.
         */
        void send() noexcept(true) {
            auto *dthis = (DerivedT *) this;
            try {
                dthis->send_impl();
            } catch (...) {
                this->async_fail(std::current_exception());
            }
        }

        template<typename SubCmdT, typename ...Args>
        void send_subcommand(
                void (DerivedT::*mfn)(typename SubCmdT::output_type),
                Args &&...args) noexcept(true) {
            SubCmdT *cmd = make_freestanding<SubCmdT>(m_tx.connection(), std::forward<Args>(args)...);
            cmd->set_callback(this->make_exception_wrapper(mfn));
            cmd->send();
        }

    protected:
        byte_sender<DerivedT> m_tx;
    };

    template<typename DerivedT, typename DataIn, typename DataOut>
    class data_command : public command<DerivedT, DataOut> {
    public:
        typedef DataIn input_type;

        data_command()
                : command<DerivedT, DataOut>(),
                  m_dataIn(),
                  m_dataOut() {
        }

        explicit data_command(packet_layer &conn, const input_type &in)
                : command<DerivedT, DataOut>(conn),
                  m_dataIn(in),
                  m_dataOut() {
        }

    protected:
        void async_success() noexcept(true) {
            pusher<DerivedT, DataOut>::async_success(std::move(m_dataOut));
        }

        DataIn m_dataIn;
        DataOut m_dataOut;
    };

    inline bool buffer_single_ok(size_t size) {
        return size <= FILE_BUF_SIZE;
    }

    inline void assert_buffer_single_ok(size_t size) {
        if (!buffer_single_ok(size)) {
            throw std::out_of_range("Message buffer too big");
        }
    }

    inline bool buffer_large_ok(size_t size) {
        return size <= UINT16_MAX;
    }

    inline void assert_buffer_large_ok(size_t size) {
        if (!buffer_large_ok(size)) {
            throw std::out_of_range("Message buffer too big");
        }
    }


    template<typename ExceptionT, typename ...Args>
    void assert_throw(bool cond, Args &&...args) {
        if (!cond) {
            throw ExceptionT{std::forward<Args>(args)...};
        }
    };

    class reply_system_header;

    inline void parse_assert(bool cond, parse_error err) {
        assert_throw<parse_failed>(cond, err);
    }

    void assert_system_reply(const reply_system_header &hdr);

    void assert_system_reply_ok(const reply_system_header &hdr, system_commands out);

    void assert_command_success(const reply_system_header &hdr, system_commands out);

    struct reply_simple {
        system_status result;
    };

    class abstract_progress_reporter {
    public:
        abstract_progress_reporter();

        virtual ~abstract_progress_reporter();

        virtual void set_progress_callback(progresscallback &&callback) = 0;
    };

    class simple_progress_reporter : public abstract_progress_reporter {
    public:
        simple_progress_reporter();

        simple_progress_reporter(progresscallback &&info);

        void set_progress_callback(progresscallback &&callback) override;

    protected:
        void report_progress(size_t done, size_t total);

        progresscallback m_progresscall;
    };
}

#endif //LIBEV3COMM_COMMANDS_HPP
