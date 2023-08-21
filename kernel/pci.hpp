/**
* @file pci.hpp
*/

#pragma once

#include <cstdint>
#include <array>

#include "error.hpp"

namespace pci {
    /** @brief IO port address for CONFIG_ADDRESS register */
    const uint16_t kConfigAddress = 0x0cf8;
    /** @brief IO port address for CONFIG_DATA register */
    const uint16_t kConfigData = 0x0cfc;

    /** @brief PCI device class code */
    struct ClassCode {
        uint8_t base, sub, interface;

        /** @brief return true if equal to base class */
        bool Match(uint8_t b) { return b == base; }
        /** @brief return true if equal to base class and subclass */
        bool Match(uint8_t b, uint8_t s) {return Match(b) && s == sub; }
        /** @brief return true if equal to base class, subclass and interface */
        bool Match(uint8_t b, uint8_t s, uint8_t i) {
            return Match(b, s) && i == interface;
        }
    };

    /** @brief store basic data for manipulating PCI device.
    *   bus number, device number, function number are required to specify a device.
    *   other info is added for convinience. */
    struct Device {
        uint8_t bus, device, function, header_type;
        ClassCode class_code;
    };

    /** @brief write the address at CONFIG_ADDRESS */
    void WriteAddress(uint32_t address);
    /** @brief write data at CONFIG_DATA */
    void WriteData(uint32_t value);
    /** @brief read data at CONFIG_DATA */
    uint32_t ReadData();

    /** @brief read vendorID register (common among all header types) */
    uint16_t ReadVendorId(uint8_t bus, uint8_t device, uint8_t function);
    /** @brief read deviceID register (common among all header types) */
    uint16_t ReadDeviceId(uint8_t bus, uint8_t device, uint8_t function);
    /** @brief read header type register (common among all header types) */
    uint8_t ReadHeaderType(uint8_t bus, uint8_t device, uint8_t function);
    /** @brief read class code register (common among all header types) 
    *
    *   return the following 32 bits
    *    - 31:24 : base class
    *    - 23:16 : subcalss
    *    - 15:8  : interface
    *    -  7:0  : revision
    */
    ClassCode ReadClassCode(uint8_t bus, uint8_t device, uint8_t function);

    /** @brief inline func must be written in a header file.
    */
    inline uint16_t ReadVendorId(const Device& dev) {
        return ReadVendorId(dev.bus, dev.device, dev.function);
    }

    /** @brief read 32 bit register of the given PCI device*/
    uint32_t ReadConfReg(const Device& dev, uint8_t reg_addr);

    /** @brief write into 32 bit register of the given PCI device */
    void WriteConfReg(const Device& dev, uint8_t reg_addr, uint32_t value);

    /** @brief read bus number register (header type 1) 
    *
    *   return the following 32 bits 
    *    - 23:16 : subordinate bus number
    *    - 15:8  : secondary bus number
    *    -  7:0  : revision number
    */
    uint32_t ReadBusNumbers(uint8_t bus, uint8_t device, uint8_t function);

    /** @brief return true if single function*/
    bool IsSingleFunctionDevice(uint8_t header_type);

    /** @brief PCI device lists found via ScanAllBus() */
    inline std::array<Device, 32> devices;
    /** @brief the number of valid devices */
    inline int num_device;
    /** @brief scan all PCI devices and store at devices[] 
    *
    * scan PCI devices from bus 0 and store them from the head of devices[]. 
    * set the number of found devices into num_device 
    */
    Error ScanAllBus();

    constexpr uint8_t CalcBarAddress(unsigned int bar_index) {
        return 0x10 + 4 * bar_index;
    }

    WithError<uint64_t> ReadBar(Device& device, unsigned int bar_index);

    /** @brief common header of PCI capability register */
    union CapabilityHeader {
        uint32_t data;
        struct {
            uint32_t cap_id : 8;
            uint32_t next_ptr : 8;
            uint32_t cap : 16;
        } __attribute__((packed)) bits;
    } __attribute__((packed));

    const uint8_t kCapabilityMSI = 0x05;
    const uint8_t kCapabilityMSIX = 0x11;

    /** @brief read the given capability register og the given PCI device 
    *
    *   @param dev : PCI device to read capability
    *   @param addr : Configuration space address of capability register
    */
    CapabilityHeader ReadCapabilityHeader(const Device& dev, uint8_t addr);

    /** @brief MSI capability configuration
    *
    *   Many types of MSI capability configuration exists, such as 64 bit support ver.
    *   In this struct, members are defined with the max one to cope with all types.
    */
    struct MSICapability {
        union {
            uint32_t data;
            struct {
                uint32_t cap_id : 8;
                uint32_t next_ptr : 8;
                uint32_t msi_enable : 1;
                uint32_t multi_msg_capable : 3;
                uint32_t multi_msg_enable : 3;
                uint32_t addr_64_capable : 1;
                uint32_t per_vector_mask_capable : 1;
                uint32_t : 7;
            } __attribute__((packed)) bits;
        } __attribute__((packed)) header ;

        uint32_t msg_addr;
        uint32_t msg_upper_addr;
        uint32_t msg_data;
        uint32_t mask_bits;
        uint32_t pending_bits;
    } __attribute__((packed));

    /** @brief set MSI or MSI-X interrupt
    *
    *   @param dev : PCI device to be set
    *   @param msg_addr : address where message is written when interrupt occurs
    *   @param msg_data : value of message to be written when interrupt occurs
    *   @param num_vector_exponent : the number of vectors (set n of '2^n')
    */
    Error ConfigureMSI(const Device& dev, uint32_t msg_addr, uint32_t msg_data,
                        unsigned int num_vector_exponent);
    
    enum class MSITriggerMode {
        kEdge = 0,
        kLevel = 1
    };

    enum class MSIDeliveryMode {
        kFixed          = 0b000,
        kLowestPriority = 0b001,
        kSMI            = 0b010,
        kNMI            = 0b100,
        kINIT           = 0b101,
        kExtINT         = 0b111,
    };

    Error ConfigureMSIFixedDestination(
        const Device& dev, uint8_t apic_id,
        MSITriggerMode trigger_mode, MSIDeliveryMode delivery_mode,
        uint8_t vector, unsigned int num_vector_exponent
    );
}

void InitializePCI();