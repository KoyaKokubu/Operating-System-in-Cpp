/*
* functionalities for setting endpoints
*/

#pragma once

#include "error.hpp"

namespace usb {
    enum class EndpointType {
        kControl = 0,
        kIsochronous = 1,
        kBulk = 2,
        kInterrupt = 3,
    };

    class EndpointID {
        public:
            constexpr EndpointID() : addr_{0} {}
            constexpr EndpointID(const EndpointID& ep_id) : addr_{ep_id.addr_} {}
            explicit constexpr EndpointID(int addr) : addr_{addr} {}

            /** compose ID with endpoind number and I/O direction.
            *
            *   ep_num : integer (0 ~ 15)
            *   dir_in : must be true at Control endpoint
            */
            constexpr EndpointID(int ep_num, bool dir_in) : addr_{ep_num << 1 | dir_in} {}

            EndpointID& operator =(const EndpointID& rhs) {
                addr_ = rhs.addr_;
                return *this;
            }

            /** endpoint address (0 ~ 31) */
            int Address() const { return addr_; }

            /** endpoint number (0 ~ 15) */
            int Number() const { return addr_ >> 1; }

            /** I/O direction. Control endpoint=true */
            bool IsIn() const { return addr_ & 1; }

        private:
            int addr_;
    };

    constexpr EndpointID kDefaultControlPipeID{0, true};

    struct EndpointConfig {
        /** endpointID */
        EndpointID ep_id;

        /** type of this endpoint */
        EndpointType ep_type;

        /** the max packet size of this endpoint (bytes) */
        int max_packet_size;

        /** control interval of this endpoint (125*2^(interval-1) micro sec) */
        int interval;
    };
}