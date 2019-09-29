//
// Created by kuba on 29.12.17.
//

#ifndef LIBEV3COMM_HANDLE_MANAGER_H
#define LIBEV3COMM_HANDLE_MANAGER_H


#include <cstdint>
#include <string>

namespace ev3loader {
    typedef uint8_t handle_t;
    enum {
        handle_read,
        handle_write
    };

    class handle_manager {
    public:
        virtual void register_handle(handle_t id, std::string path, std::string desc) = 0;
        virtual void unregister_handle(handle_t id) = 0;
    private:
    };
};

#endif //LIBEV3COMM_HANDLE_MANAGER_H
