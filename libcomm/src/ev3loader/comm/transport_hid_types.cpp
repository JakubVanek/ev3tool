//
// Created by kuba on 22.7.18.
//

#include <sstream>
#include <iomanip>
#include <ev3loader/comm/transport_hid_types.hpp>
#include <ev3loader/utils/utils.hpp>
#include <ev3loader/i18n/localization.hpp>

/////////////////
// HID ADDRESS //
/////////////////

using namespace ev3loader;
using namespace ev3loader::comm;
using namespace ev3loader::utils;
using namespace ev3loader::i18n::priv;

static void take_prefix(const std::string &str, size_t &idx) {
    if (!starts_with(str, idx, "hid:")) {
        throw std::invalid_argument(_("HID address does not contain valid prefix"));
    }
    idx += 4;
}

static uint16_t take_id(const std::string &str, size_t &idx) {
    size_t subidx = 0;
    uint16_t id = 0x0000;
    try {
        id = static_cast<uint16_t>(std::stoi(str.substr(idx), &subidx, 16));
    } catch (std::invalid_argument &ex) {
        throw std::invalid_argument(_("HID ID is invalid"));
    }
    idx += subidx;
    return id;
}

static void take_colon(const std::string &str, size_t &idx) {
    if (!starts_with(str, idx, ":")) {
        throw std::invalid_argument(_("HID address separator is invalid"));
    }
    idx += 1;
}

static std::wstring take_serialno(const std::string &str, size_t &idx) {
    std::string sn = str.substr(idx);
    idx += sn.size();
    return narrow2wide(sn);
}

static void take_end(const std::string &str, size_t &idx) {
    if (str.size() != idx) {
        throw std::invalid_argument(_("Extra data after HID address end"));
    }
}

hid_address::hid_address(std::string addr) {
    size_t idx = 0;
    std::string sn;

    tolower_inplace(addr);
    take_prefix(addr, idx);
    this->vendor_id = take_id(addr, idx);
    take_colon(addr, idx);
    this->product_id = take_id(addr, idx);
    if (starts_with(addr, idx, ":")) {
        take_colon(addr, idx);
        this->serial_no = take_serialno(addr, idx);
    }
    take_end(addr, idx);
}

hid_address::operator std::string() const {
    std::ostringstream str;

    str << "HID:" << std::hex << std::setw(4) << std::setfill('0') << vendor_id
        << ':' << std::hex << std::setw(4) << std::setfill('0') << product_id;

    if (!serial_no.empty()) {
        str << ":" << wide2narrow(serial_no);
    }
    return str.str();
}
