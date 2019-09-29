#include <cassert>
#include <iostream>
#include <atomic>
#include "connection_ev3_hid.hpp"

using namespace ev3loader;

#define hidx(x) hidfn(x)(m_dev.get())
#define hid(x, ...) hidfn(x)(m_dev.get(), __VA_ARGS__)
#define hidfn(x) hid_##x

void hidapi_throw(const hid_address &addr, hid_errorcode what, std::string &&where, const wchar_t *error) {
    const wchar_t *err_wide = error;
    if (err_wide == nullptr) {
        std::string  err_narrow = strerror_l(errno, i18n::get_locale());
        std::wstring err_wider  = narrow2wide(err_narrow);
        err_wide = wcsdup(err_wider.c_str());
    }
    throw hidapi_error(addr, what, err_wide, std::move(where));
}

/////////////////
// HIDAPI WRAP //
/////////////////

class hidapi_enumeration {
public:
    hidapi_enumeration(uint16_t vid, uint16_t pid);

    hid_device_info *next();

private:
    using hid_deleter = void (*)(struct hid_device_info *);
    std::unique_ptr<hid_device_info, hid_deleter> list;
    hid_device_info                               *elem = nullptr;
};

hidapi_enumeration::hidapi_enumeration(uint16_t vid, uint16_t pid)
        : list(hid_enumerate(vid, pid), &hid_free_enumeration), elem(list.get()) {}

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

// Ctor & Dtor

hidapi_discovery::hidapi_discovery() {
    hidapi_intialize();
}

hidapi_discovery::~hidapi_discovery() = default;

// Functions

hidapi_discovery::device_list hidapi_discovery::discover() {
    hid_address addr(EV3_VID, EV3_PID);
    return discover(addr);
}

hidapi_discovery::device_list hidapi_discovery::discover(const hid_address &addr) {
    hid_device_info *dev;
    device_list     result;

    hidapi_enumeration devices(addr.vendor_id, addr.product_id);

    while ((dev = devices.next()) != nullptr) {
        hid_address entry;
        entry.vendor_id  = dev->vendor_id;
        entry.product_id = dev->product_id;
        entry.serial_no.assign(dev->serial_number);
        if (dev->product_string)
            entry.product_name.assign(dev->product_string);
        if (dev->manufacturer_string)
            entry.vendor_name.assign(dev->manufacturer_string);

        if (addr.serial_no.empty() || entry.serial_no == addr.serial_no) {
            result.push_back(entry);
        }
    }

    return result;
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

void hidapi_link::write(nativewindow &in) {
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

void hidapi_link::read(nativewindow &out, std::chrono::milliseconds timeout) {
    int time = static_cast<int>(timeout.count());
    int len  = hid(read_timeout, out.iter, out.end - out.iter, time);

    if (len == -1) {
        hidapi_throw(m_addr, hid_errorcode::READ_FAILED, HERE, hidx(error));
    } else {
        out.iter += len;
    }
}

// Input/Output blocking

bool hidapi_link::nonblock_check() const {
    return m_nonblock;
}

void hidapi_link::nonblock_set(bool enabled) {
    m_nonblock = enabled;
    hid(set_nonblocking, m_nonblock ? 1 : 0);
}