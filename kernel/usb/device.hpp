/**
* class describing USB device, and related functionalities.
*/

#pragma once

#include <array>

#include "error.hpp"
#include "usb/setupdata.hpp"
#include "usb/endpoint.hpp"
#include "usb/arraymap.hpp"

namespace usb {
    class ClassDriver;

    class Device {
        public:
            virtual ~Device();
            virtual Error ControlIn(EndpointID ep_id, SetupData setup_data,
                                    void* buf, int len, ClassDriver* issuer);
            virtual Error ControlOut(EndpointID ep_id, SetupData setup_data,
                                     const void* buf, int len, ClassDriver* issuer);
            virtual Error InterruptIn(EndpointID ep_id, void* buf, int len);
            virtual Error InterruptOut(EndpointID ep_id, void* buf, int len);

            Error StartInitialize();
            bool IsInitialized() { return is_initialized_; }
            EndpointConfig* EndpointConfigs() { return ep_configs_.data(); }
            int NumEndpointConfigs() { return num_ep_configs_; }
            Error OnEndpointsConfigured();

            uint8_t* Buffer() { return buf_.data(); }

        protected:
            Error OnControlCompleted(EndpointID ep_id, SetupData setup_data,
                                     const void* buf, int len);
            Error OnInterruptCompleted(EndpointID ep_id, const void* buf, int len);
        
        private:
            /** @brief class drivers which are already allocated to an endpoint.
            *
            *   index : endpoint number (0 ~ 15)
            *   index 0 : always unused because it's never used by any class driver.
            */
            std::array<ClassDriver*, 16> class_drivers_{};

            /* PCI configuration space ? 
            *  Or doorbell register ?
            *  -> buffer for device descriptor
            */
            std::array<uint8_t, 256> buf_{};

            // following fields are used during initialization
            uint8_t num_configurations_;
            uint8_t config_index_;

            Error OnDeviceDescriptorReceived(const uint8_t* buf, int len);
            Error OnConfigurationDescriptorreceived(const uint8_t* buf, int len);
            Error OnSetConfigurationCompleted(uint8_t config_value);

            bool is_initialized_ = false;
            int initialize_phase_ = 0;
            std::array<EndpointConfig, 16> ep_configs_;
            int num_ep_configs_;
            Error InitializePhase1(const uint8_t* buf, int len);
            Error InitializePhase2(const uint8_t* buf, int len);
            Error InitializePhase3(uint8_t config_value);
            Error InitializePhase4();

            /** Map structure to specify the event waiter in OnControlCompleted.
            *   Register the event waiter when calling ControlOut or ControlIn.
            */
            ArrayMap<SetupData, ClassDriver*, 4> event_waiters_{};
    };

    Error GetDescriptor(Device& dev, EndpointID ep_id,
                        uint8_t desc_type, uint8_t desc_index,
                        void* buf, int len, bool debug = false);
    Error SetConfiguration(Device& dev, EndpointID ep_id,
                           uint8_t config_value, bool debug = false);
}