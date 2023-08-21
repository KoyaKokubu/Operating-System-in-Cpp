/**
* classes and functions for Event Ring, Command Ring, Transfer Ring
*/

#pragma once

#include <cstdint>
#include <vector>

#include "error.hpp"
#include "usb/memory.hpp"
#include "usb/xhci/registers.hpp"
#include "usb/xhci/trb.hpp"

namespace usb::xhci {
    /** @brief class about Command/Transfer Ring */
    class Ring {
        public:
            Ring() = default;
            Ring(const Ring&) = delete;
            ~Ring();
            Ring& operator=(const Ring&) = delete;

            /** @brief allocate memory for Ring and initialize member */
            Error Initialize(size_t buf_size);

            /** @brief set cycle bit for TRB and add it to the end of the Ring.
            *
            *   @return pointer to the added TRB in the Ring
            */
            template <typename TRBType>
            TRB* Push(const TRBType& trb) {
                return Push(trb.data);
            }

            TRB* Buffer() const { return buf_; }

        private:
            TRB* buf_ = nullptr;
            size_t buf_size_ = 0;

            /** @brief bit indicating the producer cycle state */
            bool cycle_bit_;
            /** @brief index of Ring where the element will be written next time */
            size_t write_index_;

            /** @brief set cycle bit for TRB and write it to the end of the Ring
            *
            *   no change for write_index_
            */
            void CopyToLast(const std::array<uint32_t, 4>& data);

            /** @brief set cycle bit for TRB and add it to the end of the Ring.
            *
            *   increment write_index_ and if it achieve the end of the Ring,
            *   then allocate LinkTRB properly, get write_index_ back to 0, 
            *   and flip cycle bit.
            *
            *   @return pointer of TRB added to Ring.
            */
            TRB* Push(const std::array<uint32_t, 4>& data);
    };

    union EventRingSegmentTableEntry {
        std::array<uint32_t, 4> data;
        struct {
            uint64_t ring_segment_base_address; // 64 bytes alignment

            uint32_t ring_segment_size : 16;
            uint32_t : 16;

            uint32_t : 32;
        } __attribute__((packed)) bits;
    };

    class EventRing {
        public:
            Error Initialize(size_t buf_size, InterrupterRegisterSet* interrupter);

            TRB* ReadDequeuePointer() const {
                return reinterpret_cast<TRB*>(interrupter_->ERDP.Read().Pointer());
            }

            void WriteDequeuePointer(TRB* p);

            bool HasFront() const {
                return Front()->bits.cycle_bit == cycle_bit_;
            }

            TRB* Front() const {
                return ReadDequeuePointer();
            }

            void Pop();

        private:
            TRB* buf_;
            size_t buf_size_;

            bool cycle_bit_;
            EventRingSegmentTableEntry* erst_;
            InterrupterRegisterSet* interrupter_;
    };
}