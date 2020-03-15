//
// Created by kuba on 14.01.20.
//

#ifndef EV3COMM_TRANSPORT_HID_EXCEPT_HPP
#define EV3COMM_TRANSPORT_HID_EXCEPT_HPP

#include <ev3loader/exceptions.hpp>
#include <ev3loader/comm/transport_hid_types.hpp>

namespace ev3loader {
    namespace comm {
        class hidapi_error : public communication_error {
        public:
            hidapi_error()
                    : addr(), code(hid_errorcode::UNKNOWN_HID_ERROR), errstr(nullptr) {}

            explicit hidapi_error(hid_address addr,
                                  hid_errorcode code,
                                  const wchar_t *errstr, std::string &&where)
                    : communication_error(hid_error_desc(code), where),
                      addr(std::move(addr)),
                      code(code),
                      errstr(errstr) {}

            const hid_address &get_address() const {
                return addr;
            }

            const hid_errorcode &get_code() const {
                return code;
            }

            const char *what() const noexcept override {
                try {
                    std::string str = format(i18n::priv::_("Device %s - %s (%S) at %s"),
                                             ((std::string) addr).c_str(),
                                             getDescription().c_str(),
                                             errstr,
                                             getLocation().c_str());
                    return strdup(str.c_str());
                } catch (...) {
                    return "FATAL EXCEPTION ERROR";
                }
            }

            operator std::exception_ptr() const override {
                return std::make_exception_ptr(*this);
            }

        private:
            hid_address addr;
            hid_errorcode code;
            const wchar_t *errstr;
        };
    }
}

#endif //EV3COMM_TRANSPORT_HID_EXCEPT_HPP
