// Copyright 2015-2018 Elviss Strazdins. All rights reserved.

#ifndef OUZEL_AUDIO_SOUND_HPP
#define OUZEL_AUDIO_SOUND_HPP

#include <memory>
#include "audio/Stream.hpp"
#include "math/Vector3.hpp"

namespace ouzel
{
    namespace audio
    {
        class Audio;
        class Mix;
        class SoundData;

        class Sound final: public Stream::EventListener
        {
            friend Mix;
        public:
            explicit Sound(Audio& initAudio);
            Sound(Audio& initAudio, const std::shared_ptr<SoundData>& initSoundData);
            ~Sound();

            Sound(const Sound&) = delete;
            Sound& operator=(const Sound&) = delete;
            Sound(Sound&&) = delete;
            Sound& operator=(Sound&&) = delete;

            inline const std::shared_ptr<SoundData>& getSoundData() const { return soundData; }

            void play(bool repeatSound = false);
            void pause();
            void stop();

            bool isPlaying() const { return playing; }
            bool isRepeating() const { return repeating; }

            void onReset() override;
            void onStop() override;

            void setOutput(Mix* newOutput);
            
        private:
            Audio& audio;
            uintptr_t sourceId;

            std::shared_ptr<SoundData> soundData;
            std::shared_ptr<Stream> stream;

            bool playing = false;
            bool repeating = false;

            Mix* output = nullptr;
        };
    } // namespace audio
} // namespace ouzel

#endif // OUZEL_AUDIO_SOUND_HPP
