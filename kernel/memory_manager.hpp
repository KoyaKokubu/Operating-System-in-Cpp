/*
* file collecting memory manager class and some related functionalities
*/

#pragma once

#include <array>
#include <limits>

#include "error.hpp"
#include "memory_map.hpp"

namespace {
    constexpr unsigned long long operator""_KiB(unsigned long long kib) {
        return kib * 1024;
    }

    constexpr unsigned long long operator""_MiB(unsigned long long mib) {
        return mib * 1024_KiB;
    }

    constexpr unsigned long long operator""_GiB(unsigned long long gib) {
        return gib * 1024_MiB;
    }
}

/** @brief length (bytes) of one physical memory frame */
static const auto kBytesPerFrame{4_KiB};

class FrameID {
    public:
        explicit FrameID(size_t id) : id_{id} {}
        size_t ID() const { return id_; }
        void* Frame() const { return reinterpret_cast<void*>(id_ * kBytesPerFrame); }
    
    private:
        size_t id_;
};

static const FrameID kNullFrame{std::numeric_limits<size_t>::max()};

struct MemoryStat {
		size_t allocated_frames;
		size_t total_frames;
};

/** @brief class to manage memories frame by frame via bitmap array. 
*
*   manage free frame wvia bitmap (1 bit/frame).
*   Each bit in the array alloc_map is corresponded to a frame. 0: free / 1: used.
*   Pysical address corresponded to Bit m of alloc_map[n] is calculated by :
*       kFrameBytes * (n * kBitsPerMapLine + m)
*/
class BitmapMemoryManager {
    public:
        /** @brief max physical memory capacity (bytes) which can be used in this memory manager class */
        static const auto kMaxPhysicalMemoryBytes{128_GiB};
        /** @brief the number of required frames to manipulate physical memories of kMaxPhysicalMemoryBytes */
        static const auto kFrameCount{kMaxPhysicalMemoryBytes / kBytesPerFrame};

        /** @brief element type of bitmap array */
        using MapLineType = unsigned long;
        /** @brief num of frame = num of bits of one element in bitmap array */
        static const size_t kBitsPerMapLine{8 * sizeof(MapLineType)};

        /** @brief initialize instance */
        BitmapMemoryManager();

        /** @brief allocate memories of the given num of frames and return FrameID of the head */
        WithError<FrameID> Allocate(size_t num_frames);
        Error Free(FrameID start_frame, size_t num_frames);
        void MarkAllocated(FrameID start_frame, size_t num_frames);

        /** @brief set memory range that this memory manager manipulates.
        *   After calling this func, memory allocation by Allocate will be conducted within this range.
        *
        *   @param range_begin_ : beginning of the memory range
        *   @param range_end_   : end of the memory range. next frame of the last frame.
        */
        void SetMemoryRange(FrameID range_begin, FrameID range_end);

				/** @brief return the number of unused / all frames */
				MemoryStat Stat() const;

    private:
        std::array<MapLineType, kFrameCount / kBitsPerMapLine> alloc_map_;
        /** @brief beginning of memory range that this memory manager manipulates. */
        FrameID range_begin_;
        /** @brief end of memory range that this memory manager manipulates. next frame of the last frame. */
        FrameID range_end_;

        bool GetBit(FrameID frame) const;
        void SetBit(FrameID frame, bool allocated);
};

extern BitmapMemoryManager* memory_manager;
void InitializeMemoryManager(const MemoryMap& memory_map);