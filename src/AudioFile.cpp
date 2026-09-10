#include "AudioFile.h"
#include <cstdio>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
    std::string quotePath(const std::string& path)
    {
        std::string result = "\"";

        for (char c : path)
        {
            if (c == '"')
                result += "\\\"";
            else
                result += c;
        }

        result += "\"";

        return result;
    }
}

EditorAudioBuffer AudioFile::load(
    const std::string& filename,
    int sampleRate)
{
    if (sampleRate <= 0)
        throw std::runtime_error(
            "Invalid sample rate."
        );

    /*
        FFmpeg outputs:

        stereo
        48 kHz
        32-bit floating point
        raw PCM

        The resulting stream is:

        L R L R L R ...
    */

    const std::string command =
        "ffmpeg -v error "
        "-i " + quotePath(filename) + " "
        "-f f32le "
        "-ac 2 "
        "-ar " + std::to_string(sampleRate) + " "
        "pipe:1";

    FILE* pipe =
        _popen(command.c_str(), "rb");

    if (!pipe)
    {
        throw std::runtime_error(
            "Could not start FFmpeg."
        );
    }

    std::vector<float> samples;

    constexpr std::size_t chunkSamples =
        4096;

    float chunk[chunkSamples];

    while (true)
    {
        const std::size_t count =
            std::fread(
                chunk,
                sizeof(float),
                chunkSamples,
                pipe
            );

        if (count == 0)
            break;

        samples.insert(
            samples.end(),
            chunk,
            chunk + count
        );
    }

    const int result =
        _pclose(pipe);

    if (result != 0)
    {
        throw std::runtime_error(
            "FFmpeg could not decode: " +
            filename
        );
    }

    if (samples.empty())
    {
        throw std::runtime_error(
            "No audio data found in: " +
            filename
        );
    }

    /*
        Two channels:

        samples[0] = left
        samples[1] = right
        samples[2] = left
        samples[3] = right
        ...
    */

    const std::size_t frames =
        samples.size() / 2;

    EditorAudioBuffer buffer(
        frames,
        sampleRate
    );

    for (std::size_t i = 0; i < frames; ++i)
    {
        buffer.left()[i] =
            samples[i * 2];

        buffer.right()[i] =
            samples[i * 2 + 1];
    }

    return buffer;
}
