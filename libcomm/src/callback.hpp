//
// Created by kuba on 22.6.18.
//

#ifndef LIBEV3COMM_CALLBACK_HPP
#define LIBEV3COMM_CALLBACK_HPP

#include <functional>
#include "localization.hpp"

namespace ev3loader {
    template<typename SuccessT,
            typename FailureT,
            typename ReturnT = void,
            typename SuccessFunctorT = std::function<ReturnT(SuccessT)>,
            typename FailureFunctorT = std::function<ReturnT(FailureT)> >
    class callback {
    public:
        using success_type = SuccessT;
        using failure_type = FailureT;
        using return_type = ReturnT;
        using success_functor_type = SuccessFunctorT;
        using failure_functor_type = FailureFunctorT;

        callback() : m_success(nullptr), m_failure(nullptr) {}

        callback(std::nullptr_t) : m_success(nullptr), m_failure(nullptr) {}

        template<typename A, typename B>
        callback(A &&success, B &&failure)
                : m_success(std::forward<A>(success)),
                  m_failure(std::forward<B>(failure)) {}

        ReturnT operator()(SuccessT args) const {
            if ((bool)m_success) {
                return m_success(std::forward<SuccessT>(args));
            }
        }

        ReturnT operator()(FailureT args) const {
            if ((bool)m_failure) {
                return m_failure(std::forward<FailureT>(args));
            }
        }

        explicit operator bool() const {
            return (bool) m_success && (bool) m_failure;
        }

    private:
        success_functor_type m_success;
        failure_functor_type m_failure;
    };

    template<typename SuccessT>
    using excallback = callback<SuccessT, std::exception_ptr, void>;
    using progresscallback = std::function<void(size_t, size_t)>;
}

#endif //LIBEV3COMM_CALLBACK_HPP
