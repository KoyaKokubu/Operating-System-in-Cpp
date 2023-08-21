/*
* dynamic memomry allocation for USB driver
*/

#pragma once

#include <cstddef>

namespace usb {
    /** @brief max pool size for dynamic memory allocation (byte) */
    static const size_t kMemoryPoolSize = 4096 * 32;
    /* static const size_t kMemoryPoolSize = 4096 * 64; */

    /** @brief allocate memory of given bytes and return the head pointer
    *
    *   the head pointer allocate mamory region complying with alignment.
    *   If size <= boundry, gurantee that memory region does not exceed the boundary.
    *   Typically, set 4096 for boundary in order not to exceed the page boundary.
    *
    *   @param size      : memory size to allocate (byte).
    *   @param alignment : memory region alignment. if 0, no constraint.
    *   @param boundary  : the boundary which allocated memory region cannot exceed. 
    *                      if 0, no constrint.
    *   @return nullptr if no memory region is allocate
    */
    void* AllocMem(size_t size, unsigned int alignment, unsigned int boundary);

    template <class T>
    T* AllocArray(size_t num_obj, unsigned int alignment, unsigned int boundary) {
        return reinterpret_cast<T*>(
            AllocMem(sizeof(T) * num_obj, alignment, boundary)
        );
    }

    /** @brief free up allocated memory. no guarantee for truely freeing up.*/
    void FreeMem(void* p);

    /** @brief memory allocator for standard container */
    template <class T, unsigned int Alignment = 64, unsigned int Boundary = 4096>
    class Allocator {
        public:
            using size_type = size_t;
            using pointer = T*;
            using value_type = T;

            Allocator() noexcept = default;
            Allocator(const Allocator&) noexcept = default;
            template <class U> Allocator(const Allocator<U>&) noexcept {}
            ~Allocator() noexcept = default;
            Allocator& operator=(const Allocator&) = default;
            
            pointer allocate(size_type n) {
                return AllocArray<T>(n, Alignment, Boundary);
            }

            void deallocate(pointer p, size_type num) {
                FreeMem(p);
            }
    };
}