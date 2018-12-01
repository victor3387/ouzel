// Copyright 2015-2018 Elviss Strazdins. All rights reserved.

#include <algorithm>
#include "AudioDevice.hpp"
#include "math/MathUtils.hpp"

namespace ouzel
{
    namespace audio
    {
        AudioDevice::AudioDevice(Driver initDriver):
            driver(initDriver)
        {
        }

        AudioDevice::~AudioDevice()
        {
        }

        void AudioDevice::process()
        {
            Command command;
            for (;;)
            {
                std::unique_lock<std::mutex> lock(commandMutex);
                if (commandQueue.empty()) break;
                command = std::move(commandQueue.front());
                commandQueue.pop();
                lock.unlock();

                switch (command.type)
                {
                    case Command::Type::DELETE_OBJECT:
                    {
                        objects[command.objectId - 1].reset();
                        break;
                    }
                    case Command::Type::INIT_BUS:
                        break;
                    case Command::Type::ADD_LISTENER:
                        break;
                    case Command::Type::REMOVE_LISTENER:
                        break;
                    case Command::Type::ADD_FILTER:
                        break;
                    case Command::Type::REMOVE_FILTER:
                        break;
                    case Command::Type::INIT_LISTENER:
                        break;
                    case Command::Type::UPDATE_LISTENER:
                        break;
                    case Command::Type::INIT_FILTER:
                    {
                        if (command.objectId > objects.size())
                            objects.resize(command.objectId);

                        objects[command.objectId - 1] = command.createFunction();
                        break;
                    }
                    case Command::Type::UPDATE_FILTER:
                    {
                        command.updateFunction(objects[command.objectId - 1].get());
                        break;
                    }
                    case Command::Type::ADD_OUTPUT_BUS:
                    {
                        objects[command.objectId - 1]->addOutputObject(objects[command.destinationObjectId - 1].get());
                        break;
                    }
                    case Command::Type::SET_MASTER_BUS:
                    {
                        masterBus = command.objectId ? objects[command.objectId - 1].get() : nullptr;
                        break;
                    }
                    default:
                        throw std::runtime_error("Invalid command");
                }
            }
        }

        void AudioDevice::getData(uint32_t frames, std::vector<uint8_t>& result)
        {
            buffer.resize(frames * channels);
            std::fill(buffer.begin(), buffer.end(), 0.0F);

            if (masterBus)
            {
                uint16_t inputChannels = channels;
                uint32_t inputSampleRate = sampleRate;
                Vector3 inputPosition;

                masterBus->process(buffer, inputChannels, inputSampleRate, inputPosition);
            }

            for (float& f : buffer)
                f = clamp(f, -1.0F, 1.0F);

            switch (sampleFormat)
            {
                case SampleFormat::SINT16:
                {
                    result.resize(frames * channels * sizeof(int16_t));
                    int16_t* resultPtr = reinterpret_cast<int16_t*>(result.data());

                    for (uint32_t i = 0; i < buffer.size(); ++i)
                    {
                        *resultPtr = static_cast<int16_t>(buffer[i] * 32767.0F);
                        ++resultPtr;
                    }
                    break;
                }
                case SampleFormat::FLOAT32:
                {
                    result.reserve(frames * channels * sizeof(float));
                    result.assign(reinterpret_cast<uint8_t*>(buffer.data()),
                                  reinterpret_cast<uint8_t*>(buffer.data()) + buffer.size() * sizeof(float));
                    break;
                }
                default:
                    throw std::runtime_error("Invalid sample format");
            }
        }

        void AudioDevice::addCommand(const Command& command)
        {
            std::unique_lock<std::mutex> lock(commandMutex);
            commandQueue.push(command);
            lock.unlock();
            commandConditionVariable.notify_all();
        }
    } // namespace audio
} // namespace ouzel
