/*
* Default definition of Protocol Speed ID.
* This is valid only when PSIC == 0
*/

#pragma once

namespace usb::xhci {
    const int kFullSpeed = 1;
    const int kLowSpeed = 2;
    const int kHighSpeed = 3;
    const int kSuperSpeed = 4;
    const int kSuperSpeedPlus = 5;
}