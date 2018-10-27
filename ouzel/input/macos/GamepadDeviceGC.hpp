// Copyright 2015-2018 Elviss Strazdins. All rights reserved.

#pragma once

#if defined(__OBJC__)
#include <GameController/GameController.h>
typedef GCController* GCControllerPtr;
#else
#include <objc/objc.h>
typedef id GCControllerPtr;
#endif

#include "input/macos/GamepadDeviceMacOS.hpp"
#include "input/Gamepad.hpp"

namespace ouzel
{
    namespace input
    {
        class GamepadDeviceGC final: public GamepadDeviceMacOS
        {
        public:
            GamepadDeviceGC(InputSystem& initInputSystem,
                            uint32_t initId,
                            GCControllerPtr initController);

            void setAbsoluteDPadValues(bool absoluteDPadValues);
            bool isAbsoluteDPadValues() const;

            int32_t getPlayerIndex() const;
            virtual void setPlayerIndex(int32_t playerIndex) override;

            GCControllerPtr getController() const { return controller; }

        private:
            GCControllerPtr controller;
            bool attached;
        };
    } // namespace input
} // namespace ouzel
