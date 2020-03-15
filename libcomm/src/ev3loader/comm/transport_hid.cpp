#include <iostream>
#include <atomic>
#include <ev3loader/comm/transport_hid.hpp>
#include <ev3loader/comm/transport_hid_except.hpp>

using namespace ev3loader;
using namespace ev3loader::comm;
using namespace ev3loader::utils;

#define hidx(x) hidfn(x)(m_dev.get())
#define hid(x, ...) hidfn(x)(m_dev.get(), __VA_ARGS__)
#define hidfn(x) hid_##x

void hidapi_throw(const hid_address &addr, hid_errorcode what, std::string &&where, const wchar_t *error) {
    const wchar_t *err_wide = error;
    if (err_wide == nullptr) {
#ifndef _WIN32
        std::string err_narrow = strerror_l(errno, i18n::get_locale());
#else
        std::string  err_narrow = strerror(errno);
#endif
        std::wstring err_wider = narrow2wide(err_narrow);
        err_wide = wcsdup(err_wider.c_str());
    }
    throw hidapi_error(addr, what, err_wide, std::move(where));
}

/////////////////
// HIDAPI WRAP //
/////////////////

class hidapi_enumeration {
public:
    hidapi_enumeration();

    hid_device_info *next();

private:
    using hid_deleter = void (*)(struct hid_device_info *);
    std::unique_ptr<hid_device_info, hid_deleter> list;
    hid_device_info *elem = nullptr;
};

hidapi_enumeration::hidapi_enumeration()
        : list(hid_enumerate(0, 0), &hid_free_enumeration), elem(list.get()) {}

hid_device_info *hidapi_enumeration::next() {
    hid_device_info *now = elem;
    if (now != nullptr)
        elem = elem->next;
    return now;
}

static std::atomic<bool> hidapi_initialized(false);

static void hidapi_intialize() {
    bool was = hidapi_initialized.exchange(true, std::memory_order_seq_cst);
    if (!was) {
        hid_init();
    }
}

///////////////
// DISCOVERY //
///////////////

hid_addresses discover_real() {
    hid_device_info *dev;
    hid_addresses result;

    hidapi_enumeration devices;

    while ((dev = devices.next()) != nullptr) {
        if (dev->vendor_id != EV3_VID)
            continue;
        if (dev->product_id != EV3_PID && dev->product_id != EV3_PID_FWMODE)
            continue;

        hid_address entry;
        entry.vendor_id = dev->vendor_id;
        entry.product_id = dev->product_id;
        entry.serial_no.assign(dev->serial_number);
        if (dev->product_string)
            entry.product_name.assign(dev->product_string);
        if (dev->manufacturer_string)
            entry.vendor_name.assign(dev->manufacturer_string);
        result.push_back(entry);
    }

    return result;
}

hid_addresses hidapi_link::discover() {
    hidapi_intialize();
    return discover_real();
}

//////////
// LINK //
//////////

// Ctor & Dtor

hidapi_link::hidapi_link(hid_address addr) : m_dev(nullptr), m_addr(std::move(addr)) {
    hidapi_intialize();

    const wchar_t *sn = m_addr.serial_no.empty() ? nullptr : m_addr.serial_no.c_str();

    hid_device *dev = hid_open(m_addr.vendor_id, m_addr.product_id, sn);

    if (dev == nullptr) {
        hidapi_throw(m_addr, hid_errorcode::CONNECTION_FAILED, HERE, L"HIDAPI cannot connect");
    }

    // smart pointers are great
    m_dev = devptr_t{dev, hid_deleter{}};
}

// Input/Output functions

void hidapi_link::write(nativewindow &in,
                        milliseconds timeout,
                        bool blocking) {
    if (!blocking)
        throw std::logic_error("HIDAPI does not support nonblocking writes");

    if (timeout.count() > 0)
        throw std::logic_error("HIDAPI does not support timed writes");

    while (true) {
        int len = hid(write, in.iter, in.end - in.iter);

        if (len == -1) {
            hidapi_throw(m_addr, hid_errorcode::WRITE_FAILED, HERE, hidx(error));
        } else {
            in.iter += len;
            break;
        }
    }
}

void hidapi_link::read(nativewindow &out,
                       milliseconds timeout,
                       bool blocking) {
    int time;

    if (blocking) {
        if (timeout.count() != 0)
            time = timeout.count();
        else
            time = INT32_MAX;
    } else {
        time = 0;
    }

    int len = hid(read_timeout, out.iter, out.end - out.iter, time);

    if (len == -1) {
        hidapi_throw(m_addr, hid_errorcode::READ_FAILED, HERE, hidx(error));
    } else {
        out.iter += len;
    }
}
