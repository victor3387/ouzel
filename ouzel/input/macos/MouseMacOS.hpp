// Copyright 2015-2018 Elviss Strazdins. All rights reserved.

#pragma once

#include "input/InputDevice.hpp"

namespace ouzel
{
    namespace input
    {
        class InputSystemMacOS;

        class MouseMacOS: public InputDevice
        {
        public:
            MouseMacOS(InputSystemMacOS& initInputSystemMacOS,
                       uint32_t initDeviceId):
                InputDevice(initDeviceId),
                inputSystemMacOS(initInputSystemMacOS)
            {
            }

            virtual ~MouseMacOS() {}

        private:
            InputSystemMacOS& inputSystemMacOS;
        };
    } // namespace input
} // namespace ouzel
