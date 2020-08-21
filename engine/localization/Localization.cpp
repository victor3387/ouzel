// Copyright 2015-2020 Elviss Strazdins. All rights reserved.

#include <stdexcept>
#include <string>
#include <string_view>
#include "Localization.hpp"

namespace ouzel
{
    Language::Language(const std::vector<std::byte>& data)
    {
        if (data.size() < 5 * sizeof(std::uint32_t))
            throw std::runtime_error("Not enough data");

        const auto magic = static_cast<std::uint32_t>(data[0]) |
            (static_cast<std::uint32_t>(data[1]) << 8) |
            (static_cast<std::uint32_t>(data[2]) << 16) |
            (static_cast<std::uint32_t>(data[3]) << 24);

        const auto decodeUInt32 = [magic]() -> std::function<std::uint32_t(const std::byte*)> {
            constexpr std::uint32_t magicBig = 0xDE120495U;
            constexpr std::uint32_t magicLittle = 0x950412DEU;

            if (magic == magicBig)
                return [](const std::byte* bytes) noexcept {
                    return static_cast<std::uint32_t>(bytes[3]) |
                        (static_cast<std::uint32_t>(bytes[2]) << 8) |
                        (static_cast<std::uint32_t>(bytes[1]) << 16) |
                        (static_cast<std::uint32_t>(bytes[0]) << 24);
                };
            else if (magic == magicLittle)
                return [](const std::byte* bytes) noexcept {
                    return static_cast<std::uint32_t>(bytes[0]) |
                        (static_cast<std::uint32_t>(bytes[1]) << 8) |
                        (static_cast<std::uint32_t>(bytes[2]) << 16) |
                        (static_cast<std::uint32_t>(bytes[3]) << 24);
                };
            else
                throw std::runtime_error("Wrong magic " + std::to_string(magic));
        }();

        const std::uint32_t revisionOffset = sizeof(magic);
        const std::uint32_t revision = decodeUInt32(data.data() + revisionOffset);

        if (revision != 0)
            throw std::runtime_error("Unsupported revision " + std::to_string(revision));

        const std::uint32_t stringCountOffset = revisionOffset + sizeof(revision);
        const std::uint32_t stringCount = decodeUInt32(data.data() + stringCountOffset);

        struct TranslationInfo final
        {
            std::uint32_t stringLength = 0;
            std::uint32_t stringOffset = 0;

            std::uint32_t translationLength = 0;
            std::uint32_t translationOffset = 0;
        };
        std::vector<TranslationInfo> translations(stringCount);

        const std::uint32_t stringsOffsetOffset = stringCountOffset + sizeof(stringCount);
        const std::uint32_t stringsOffset = decodeUInt32(data.data() + stringsOffsetOffset);

        const std::uint32_t translationsOffsetOffset = stringsOffsetOffset + sizeof(stringsOffset);
        const std::uint32_t translationsOffset = decodeUInt32(data.data() + translationsOffsetOffset);

        if (data.size() < stringsOffset + 2 * sizeof(std::uint32_t) * stringCount)
            throw std::runtime_error("Not enough data");

        std::uint32_t stringOffset = stringsOffset;
        for (std::uint32_t i = 0; i < stringCount; ++i)
        {
            translations[i].stringLength = decodeUInt32(data.data() + stringOffset);
            stringOffset += sizeof(translations[i].stringLength);

            translations[i].stringOffset = decodeUInt32(data.data() + stringOffset);
            stringOffset += sizeof(translations[i].stringOffset);
        }

        if (data.size() < translationsOffset + 2 * sizeof(std::uint32_t) * stringCount)
            throw std::runtime_error("Not enough data");

        std::uint32_t translationOffset = translationsOffset;
        for (std::uint32_t i = 0; i < stringCount; ++i)
        {
            translations[i].translationLength = decodeUInt32(data.data() + translationOffset);
            translationOffset += sizeof(translations[i].translationLength);

            translations[i].translationOffset = decodeUInt32(data.data() + translationOffset);
            translationOffset += sizeof(translations[i].translationOffset);
        }

        for (std::uint32_t i = 0; i < stringCount; ++i)
        {
            if (data.size() < translations[i].stringOffset + translations[i].stringLength ||
                data.size() < translations[i].translationOffset + translations[i].translationLength)
                throw std::runtime_error("Not enough data");

            const std::string str(reinterpret_cast<const char*>(data.data() + translations[i].stringOffset),
                                  translations[i].stringLength);
            const std::string_view translation(reinterpret_cast<const char*>(data.data() + translations[i].translationOffset),
                                               translations[i].translationLength);

            strings[str] = translation;
        }
    }

    std::string Language::getString(const std::string& str) const
    {
        const auto i = strings.find(str);

        if (i != strings.end())
            return i->second;
        else
            return str;
    }

    void Localization::addLanguage(const std::string& name, const std::vector<std::byte>& data)
    {
        const auto i = languages.find(name);

        if (i != languages.end())
            i->second = Language(data);
        else
            languages.insert(std::make_pair(name, Language(data)));
    }

    void Localization::removeLanguage(const std::string& name)
    {
        const auto i = languages.find(name);

        if (i != languages.end())
        {
            if (currentLanguage == i)
                currentLanguage = languages.end();

            languages.erase(i);
        }
    }

    void Localization::setLanguage(const std::string& name)
    {
        const auto i = languages.find(name);

        if (i != languages.end())
            currentLanguage = i;
        else
            currentLanguage = languages.end();
    }

    std::string Localization::getString(const std::string& str) const
    {
        if (currentLanguage != languages.end())
            return currentLanguage->second.getString(str);
        else
            return str;
    }
}
