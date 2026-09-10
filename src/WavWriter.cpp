#include "WavWriter.h"
#include <fstream>
#include <algorithm>
#include <cstdint>

namespace
{
    void write16(
        std::ofstream& file,
        std::uint16_t value)
    {
        file.put(static_cast<char>(value & 0xFF));
        file.put(static_cast<char>((value >> 8) & 0xFF));
    }

    void write32(
        std::ofstream& file,
        std::uint32_t value)
    {
        file.put(static_cast<char>(value & 0xFF));
        file.put(static_cast<char>((value >> 8) & 0xFF));
        file.put(static_cast<char>((value >> 16) & 0xFF));
        file.put(static_cast<char>((value >> 24) & 0xFF));
    }

    void writeString(
        std::ofstream& file,
        const char* text)
    {
        while (*text)
            file.put(*text++);
    }
}

bool WavWriter::write(
    const std::string& filename,
    const EditorAudioBuffer& buffer)
{
    std::ofstream file(
        filename,
        std::ios::binary
    );

    if (!file)
        return false;

    constexpr std::uint16_t channels = 2;
    constexpr std::uint16_t bitsPerSample = 16;

    const std::uint32_t sampleRate =
        static_cast<std::uint32_t>(
            buffer.sampleRate()
        );

    const std::uint32_t bytesPerSample =
        bitsPerSample / 8;

    const std::uint32_t dataSize =
        static_cast<std::uint32_t>(
            buffer.size() *
            channels *
            bytesPerSample
        );

    const std::uint32_t fileSize =
        36 + dataSize;

    // RIFF header
    writeString(file, "RIFF");
    write32(file, fileSize);
    writeString(file, "WAVE");

    // fmt chunk
    writeString(file, "fmt ");
    write32(file, 16);       // PCM chunk size
    write16(file, 1);        // PCM
    write16(file, channels);
    write32(file, sampleRate);

    const std::uint32_t byteRate =
        sampleRate *
        channels *
        bytesPerSample;

    write32(file, byteRate);

    const std::uint16_t blockAlign =
        channels * bytesPerSample;

    write16(file, blockAlign);
    write16(file, bitsPerSample);

    // data chunk
    writeString(file, "data");
    write32(file, dataSize);

    for (std::size_t i = 0; i < buffer.size(); ++i)
    {
        float left = std::clamp(
            buffer.left()[i],
            -1.0f,
            1.0f
        );

        float right = std::clamp(
            buffer.right()[i],
            -1.0f,
            1.0f
        );

        const auto left16 =
            static_cast<std::int16_t>(
                left * 32767.0f
            );

        const auto right16 =
            static_cast<std::int16_t>(
                right * 32767.0f
            );

        write16(
            file,
            static_cast<std::uint16_t>(left16)
        );

        write16(
            file,
            static_cast<std::uint16_t>(right16)
        );
    }

    return file.good();
}
